#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include "Comms/LinkManager.h"
#include "Comms/SerialLink.h"
#include "Comms/UdpLink.h"
#include "Firmware/FirmwarePluginManager.h"
#include "Firmware/PX4FirmwarePlugin.h"
#include "Logging/EventLogger.h"
#include "Logging/FileLogSink.h"
#include "Logging/LogEvent.h"
#include "Logging/MemoryLogSink.h"
#include "Logging/OperatorActionLogger.h"
#include "Manual/IManualControlSink.h"
#include "Manual/ManualControlManager.h"
#include "Manual/MockJoystickBackend.h"
#include "Manual/SitlStubManualControlSink.h"
#include "Mission/MavlinkMissionLink.h"
#include "Mission/MissionManager.h"
#include "Protocol/MAVLinkMessageRouter.h"
#include "Protocol/MAVLinkProtocol.h"
#include "Safety/SafetyGate.h"
#include "Simulation/MockMissionLink.h"
#include "Simulation/MockVehicle.h"
#include "Vehicle/MultiVehicleManager.h"
#include "Vehicle/Vehicle.h"
#include "ViewModels/LinkViewModel.h"
#include "ViewModels/LogViewModel.h"
#include "ViewModels/ManualControlViewModel.h"
#include "ViewModels/MissionViewModel.h"
#include "ViewModels/VehicleViewModel.h"

namespace {

// Helper: convert a LinkStatus to a short human-readable label.
QString linkStatusName(gcs::vehicle::LinkStatus s)
{
    switch (s) {
        case gcs::vehicle::LinkStatus::Connected:    return QStringLiteral("Connected");
        case gcs::vehicle::LinkStatus::Stale:        return QStringLiteral("Stale");
        case gcs::vehicle::LinkStatus::Disconnected: return QStringLiteral("Disconnected");
    }
    return QStringLiteral("Unknown");
}

// Hooks a fresh Vehicle into the EventLogger. Called for the mock vehicle
// at boot and again for every SITL vehicle the router spawns.
void wireVehicleLogging(gcs::vehicle::Vehicle* v, gcs::logging::EventLogger* log)
{
    if (!v || !log) return;
    using gcs::logging::category::Vehicle;
    using gcs::logging::category::Safety;

    QObject::connect(v, &gcs::vehicle::Vehicle::heartbeatReceivedFirstTime, log,
        [log, v] {
            const auto& s = v->stateStore()->state();
            log->info(Vehicle,
                QStringLiteral("Heartbeat received — vehicle %1 (sysid=%2, autopilot=%3, type=%4)")
                    .arg(s.vehicleType).arg(s.systemId).arg(s.autopilotType).arg(s.vehicleType),
                {{QStringLiteral("sysid"),    s.systemId},
                 {QStringLiteral("compid"),   s.componentId},
                 {QStringLiteral("autopilot"), s.autopilotType}});
        });

    QObject::connect(v, &gcs::vehicle::Vehicle::linkStateChanged, log,
        [log, v](gcs::vehicle::LinkStatus oldS, gcs::vehicle::LinkStatus newS, qint64 ageMs) {
            const QString sev = (newS == gcs::vehicle::LinkStatus::Connected)
                                ? QString::fromLatin1(gcs::logging::severity::Info)
                                : QString::fromLatin1(gcs::logging::severity::Warn);
            log->log(Vehicle, sev,
                QStringLiteral("Link %1 → %2 (sysid=%3, age=%4 ms)")
                    .arg(linkStatusName(oldS), linkStatusName(newS))
                    .arg(v->systemId()).arg(ageMs),
                {{QStringLiteral("sysid"), v->systemId()},
                 {QStringLiteral("from"),  linkStatusName(oldS)},
                 {QStringLiteral("to"),    linkStatusName(newS)},
                 {QStringLiteral("ageMs"), ageMs}});
        });

    QObject::connect(v, &gcs::vehicle::Vehicle::statustextReceived, log,
        [log, v](int severity, const QString& text, bool isPrearm) {
            // Severity 0..3 → error/warn; 4..7 → info. ArduPilot's PreArm
            // messages can be warnings; flag them under Safety either way.
            const QString sev = (severity <= 3)
                                ? QString::fromLatin1(gcs::logging::severity::Error)
                                : (severity <= 5
                                   ? QString::fromLatin1(gcs::logging::severity::Warn)
                                   : QString::fromLatin1(gcs::logging::severity::Info));
            log->log(isPrearm ? Safety : Vehicle, sev,
                QStringLiteral("%1 sysid=%2: %3")
                    .arg(isPrearm ? "Pre-arm" : "STATUSTEXT")
                    .arg(v->systemId()).arg(text),
                {{QStringLiteral("sysid"),    v->systemId()},
                 {QStringLiteral("severity"), severity},
                 {QStringLiteral("prearm"),   isPrearm}});
        });
}

} // namespace

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName(QStringLiteral("LabGCS"));
    QGuiApplication::setOrganizationName(QStringLiteral("UAV Lab"));
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    // ============================================================
    // Logging (Phase 6)
    // ============================================================
    //
    // SAFETY: This is a pure observer. The EventLogger does not influence
    // any control path. Disable the file sink without changing app behavior.

    auto* logger    = new gcs::logging::EventLogger(&app);
    auto  memorySink = std::make_shared<gcs::logging::MemoryLogSink>(/*capacity*/ 500);
    auto  fileSink   = std::make_shared<gcs::logging::FileLogSink>();
    const QString logPath = gcs::logging::FileLogSink::defaultLogPathForNow();
    if (!logPath.isEmpty() && fileSink->open(logPath)) {
        logger->addSink(fileSink);
    }
    logger->addSink(memorySink);
    gcs::logging::OperatorActionLogger ops(logger);

    using gcs::logging::category::App;
    using gcs::logging::category::Link;
    using gcs::logging::category::Mission;
    using gcs::logging::category::Manual;
    using gcs::logging::category::Vehicle;
    using gcs::logging::category::Safety;

    logger->info(App,
        QStringLiteral("LabGCS starting (Phase 6 — logging enabled, replay NOT implemented)"),
        {{QStringLiteral("logFile"), logPath}});

    QObject::connect(&app, &QGuiApplication::aboutToQuit, &app,
        [logger] { logger->info(App, QStringLiteral("LabGCS shutting down")); });

    // ============================================================
    // Backend wiring
    // ============================================================

    auto* multi  = new gcs::vehicle::MultiVehicleManager(&app);
    auto* mockFw = new gcs::firmware::PX4FirmwarePlugin(&app);

    auto* mockVehicle = new gcs::simulation::MockVehicle(
        new gcs::vehicle::Vehicle(/*sysid*/ 200, /*compid*/ 1, mockFw, multi), multi);
    auto* mockVeh = mockVehicle->vehicle();
    multi->addVehicle(mockVeh);
    wireVehicleLogging(mockVeh, logger);

    auto* missionMgr = new gcs::mission::MissionManager(
        mockFw, mockVehicle->missionLink(), &app);
    mockVeh->setMissionManager(missionMgr);
    mockVehicle->start();

    // Mission logging (operator-driven and result events).
    QObject::connect(missionMgr, &gcs::mission::MissionManager::uploadStarted, logger,
        [logger](int total) {
            logger->info(Mission,
                QStringLiteral("Mock mission upload started (%1 items)").arg(total),
                {{QStringLiteral("totalItems"), total}});
        });
    QObject::connect(missionMgr, &gcs::mission::MissionManager::uploadCompleted, logger,
        [logger](bool ok, const QString& msg) {
            logger->log(Mission, ok ? "info" : "warn",
                QStringLiteral("Mock mission upload %1: %2").arg(ok ? "OK" : "FAILED", msg),
                {{QStringLiteral("ok"), ok}});
        });
    QObject::connect(missionMgr, &gcs::mission::MissionManager::downloadStarted, logger,
        [logger] { logger->info(Mission, QStringLiteral("Mock mission download started")); });
    QObject::connect(missionMgr, &gcs::mission::MissionManager::downloadCompleted, logger,
        [logger](bool ok, const QString& msg, const gcs::mission::MissionPlan&) {
            logger->log(Mission, ok ? "info" : "warn",
                QStringLiteral("Mock mission download %1: %2").arg(ok ? "OK" : "FAILED", msg));
        });
    QObject::connect(missionMgr, &gcs::mission::MissionManager::rejected, logger,
        [logger](const QString& reason) {
            logger->warn(Mission, QStringLiteral("Mission transfer rejected: %1").arg(reason));
        });

    // --- MAVLink protocol + router for PX4 / ArduPilot SITL ---------------
    auto* protocol = new gcs::protocol::MAVLinkProtocol(&app);
    auto* router   = new gcs::protocol::MAVLinkMessageRouter(&app);
    router->setMultiVehicleManager(multi);
    router->setProtocol(protocol);
    router->setVehicleFactory(
        [multi, &app, logger, protocol](int sysid, int compid,
                                        int autopilot, int mavType,
                                        gcs::comms::LinkKind linkKind,
                                        gcs::comms::LinkInterface* sourceLink)
                -> gcs::vehicle::Vehicle* {
            auto* fw = gcs::firmware::FirmwarePluginManager::createForHeartbeat(
                static_cast<uint8_t>(autopilot),
                static_cast<uint8_t>(mavType),
                &app);
            auto* v  = new gcs::vehicle::Vehicle(sysid, compid, fw, multi);
            // Phase 7: stamp the transport kind so SafetyGate can branch on
            // hardware vs SITL even when the autopilot identity is the same.
            v->stateStore()->setLinkKind(linkKind);
            multi->addVehicle(v);
            wireVehicleLogging(v, logger);

            // Phase 8: install a real MAVLink Mission Protocol seam for
            // SITL vehicles only. We deliberately gate on LinkKind, not on
            // autopilot identity — a real Pixhawk over UDP would be a misuse
            // of this branch, but the lab posture is that UDP/TCP == SITL.
            // Hardware (LinkKind::Serial / Replay / Unknown) gets NO mission
            // manager, so QML's upload button stays disabled and SafetyGate
            // blocks the path defensively as a second line of defence.
            const bool sitlTransport =
                (linkKind == gcs::comms::LinkKind::Udp ||
                 linkKind == gcs::comms::LinkKind::Tcp);
            if (sitlTransport && sourceLink) {
                auto* mlink = new gcs::mission::MavlinkMissionLink(
                    sysid, compid, protocol, sourceLink, /*gcsSysid*/255,
                    /*gcsCompid*/190, v);
                auto* mm = new gcs::mission::MissionManager(fw, mlink, v);
                v->setMissionManager(mm);

                QObject::connect(mm, &gcs::mission::MissionManager::uploadStarted, logger,
                    [logger, sysid](int total) {
                        logger->info(Mission,
                            QStringLiteral("SITL mission upload started sysid=%1 (%2 items)")
                                .arg(sysid).arg(total),
                            {{QStringLiteral("sysid"), sysid},
                             {QStringLiteral("totalItems"), total}});
                    });
                QObject::connect(mm, &gcs::mission::MissionManager::uploadCompleted, logger,
                    [logger, sysid](bool ok, const QString& msg) {
                        logger->log(Mission, ok ? "info" : "warn",
                            QStringLiteral("SITL mission upload %1 sysid=%2: %3")
                                .arg(ok ? "OK" : "FAILED").arg(sysid).arg(msg),
                            {{QStringLiteral("sysid"), sysid},
                             {QStringLiteral("ok"), ok}});
                    });
                QObject::connect(mm, &gcs::mission::MissionManager::downloadStarted, logger,
                    [logger, sysid] {
                        logger->info(Mission,
                            QStringLiteral("SITL mission download started sysid=%1").arg(sysid));
                    });
                QObject::connect(mm, &gcs::mission::MissionManager::downloadCompleted, logger,
                    [logger, sysid](bool ok, const QString& msg,
                                    const gcs::mission::MissionPlan&) {
                        logger->log(Mission, ok ? "info" : "warn",
                            QStringLiteral("SITL mission download %1 sysid=%2: %3")
                                .arg(ok ? "OK" : "FAILED").arg(sysid).arg(msg));
                    });
                QObject::connect(mm, &gcs::mission::MissionManager::rejected, logger,
                    [logger, sysid](const QString& reason) {
                        logger->warn(Mission,
                            QStringLiteral("SITL mission transfer rejected sysid=%1: %2")
                                .arg(sysid).arg(reason));
                    });
            }
            return v;
        });

    QObject::connect(router, &gcs::protocol::MAVLinkMessageRouter::vehicleSpawned, logger,
        [logger](int sysid, int compid, int autopilot) {
            logger->info(Vehicle,
                QStringLiteral("Vehicle detected sysid=%1 compid=%2 autopilot=%3")
                    .arg(sysid).arg(compid).arg(autopilot),
                {{QStringLiteral("sysid"), sysid},
                 {QStringLiteral("compid"), compid},
                 {QStringLiteral("autopilot"), autopilot}});
        });

    QObject::connect(multi, &gcs::vehicle::MultiVehicleManager::activeVehicleChanged, logger,
        [logger](gcs::vehicle::Vehicle* v) {
            if (!v) {
                logger->info(Vehicle, QStringLiteral("Active vehicle: (none)"));
                return;
            }
            const auto& s = v->stateStore()->state();
            logger->info(Vehicle,
                QStringLiteral("Active vehicle changed → sysid=%1 type=%2 autopilot=%3 (simulated=%4)")
                    .arg(s.systemId).arg(s.vehicleType).arg(s.autopilotType)
                    .arg(s.simulated ? "yes" : "no"),
                {{QStringLiteral("sysid"), s.systemId},
                 {QStringLiteral("simulated"), s.simulated}});
        });

    auto* linkMgr = new gcs::comms::LinkManager(protocol, &app);

    QObject::connect(linkMgr, &gcs::comms::LinkManager::udpStateChanged, logger,
        [logger, linkMgr] {
            if (auto* udp = linkMgr->udpLink()) {
                const QString listenLabel = QStringLiteral("%1:%2")
                    .arg(udp->listenAddress().toString()).arg(udp->listenPort());
                if (udp->isConnected()) {
                    logger->info(Link,
                        QStringLiteral("UDP listener started on %1").arg(listenLabel),
                        {{QStringLiteral("port"), udp->listenPort()}});
                } else {
                    logger->info(Link,
                        QStringLiteral("UDP listener stopped (was %1)").arg(listenLabel));
                }
            }
        });
    QObject::connect(linkMgr, &gcs::comms::LinkManager::udpError, logger,
        [logger](const QString& msg) { logger->error(Link, msg); });

    // Phase 7: serial link logging. The SAFETY note in the log makes the
    // hardware-read-only posture obvious in post-incident audit.
    QObject::connect(linkMgr, &gcs::comms::LinkManager::serialStateChanged, logger,
        [logger, linkMgr] {
            if (auto* s = linkMgr->serialLink()) {
                if (s->isConnected()) {
                    logger->warn(Link,
                        QStringLiteral("Serial connected (HARDWARE READ-ONLY) on %1 @ %2 — commands disabled")
                            .arg(s->portName()).arg(s->baudRate()),
                        {{QStringLiteral("port"), s->portName()},
                         {QStringLiteral("baud"), qint64(s->baudRate())},
                         {QStringLiteral("readOnly"), true}});
                } else {
                    logger->info(Link, QStringLiteral("Serial disconnected"));
                }
            }
        });
    QObject::connect(linkMgr, &gcs::comms::LinkManager::serialError, logger,
        [logger](const QString& msg) { logger->error(Link, msg); });

    // --- Manual control + SafetyGate (Phase 4) ---------------------------
    static gcs::safety::SafetyGate gate;
    auto* joystick = new gcs::manual::MockJoystickBackend(&app);
    auto* manual   = new gcs::manual::ManualControlManager(joystick, &gate, &app);

    auto rebindManual =
        [manual, mockVehicle](gcs::vehicle::Vehicle* v) {
            manual->setActiveVehicle(v);
            if (!v) { manual->setSink(nullptr); return; }
            if (v == mockVehicle->vehicle()) {
                manual->setSink(mockVehicle);
            } else {
                auto* stub = new gcs::manual::SitlStubManualControlSink(v, v);
                manual->setSink(stub);
            }
        };
    QObject::connect(multi, &gcs::vehicle::MultiVehicleManager::activeVehicleChanged,
                     &app, rebindManual);
    rebindManual(multi->activeVehicle());

    QObject::connect(manual, &gcs::manual::ManualControlManager::stateChanged, logger,
        [logger, manual](gcs::manual::ManualControlState s) {
            const QString name = QString::fromLatin1(gcs::manual::manualControlStateName(s));
            const QString reason = manual->blockedReason();
            const QString sev = (s == gcs::manual::ManualControlState::Failsafe
                                 || s == gcs::manual::ManualControlState::Blocked)
                                ? QString::fromLatin1(gcs::logging::severity::Warn)
                                : QString::fromLatin1(gcs::logging::severity::Info);
            // SAFETY: blocked/failsafe transitions always carry a reason.
            QString msg = QStringLiteral("Manual control state → %1").arg(name);
            if (!reason.isEmpty()) msg += QStringLiteral(" (%1)").arg(reason);
            logger->log(Safety, sev, msg,
                {{QStringLiteral("state"), name},
                 {QStringLiteral("reason"), reason}});
        });

    // --- ViewModels --------------------------------------------------------
    auto* vehicleVm = new gcs::viewmodels::VehicleViewModel(multi, &app);
    auto* missionVm = new gcs::viewmodels::MissionViewModel(multi, &app);
    auto* linkVm    = new gcs::viewmodels::LinkViewModel(linkMgr, &app);
    auto* manualVm  = new gcs::viewmodels::ManualControlViewModel(manual, joystick, multi, &app);
    auto* logVm     = new gcs::viewmodels::LogViewModel(logger, memorySink, &app);
    logVm->setCurrentLogPath(logPath);

    // Operator-action and validation logging tied to the QML-facing actions.
    QObject::connect(missionVm, &gcs::viewmodels::MissionViewModel::validationChanged, logger,
        [logger, missionVm] {
            if (!missionVm->validationRun()) return;
            const bool ok = missionVm->valid();
            const int errs = missionVm->validationErrors().size();
            const int warns = missionVm->validationWarnings().size();
            logger->log(Mission, ok ? "info" : "warn",
                QStringLiteral("Mission validation: %1 (errors=%2 warnings=%3)")
                    .arg(ok ? "OK" : "FAILED").arg(errs).arg(warns),
                {{QStringLiteral("errors"), errs},
                 {QStringLiteral("warnings"), warns}});
        });
    QObject::connect(missionVm, &gcs::viewmodels::MissionViewModel::infoMessage, logger,
        [&ops](const QString& msg) {
            // Save / load surface through infoMessage; treat as operator audit.
            ops.recordAction(QStringLiteral("Mission file"), msg);
        });

    // SAFETY: enable/disable transitions are already covered by the
    // ManualControlManager::stateChanged hook above under Safety. We
    // additionally surface them under OperatorAction so audits can filter
    // by category=OperatorAction.
    QObject::connect(manualVm, &gcs::viewmodels::ManualControlViewModel::stateChanged, &app,
        [manualVm, &ops] {
            static bool wasEnabled = false;
            const bool now = manualVm->operatorEnabled();
            if (now != wasEnabled) {
                ops.recordAction(
                    now ? QStringLiteral("Enable manual control")
                        : QStringLiteral("Disable manual control"));
                wasEnabled = now;
            }
        });

    // --- QML engine --------------------------------------------------------
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("vehicleVm"), vehicleVm);
    engine.rootContext()->setContextProperty(QStringLiteral("missionVm"), missionVm);
    engine.rootContext()->setContextProperty(QStringLiteral("linkVm"),    linkVm);
    engine.rootContext()->setContextProperty(QStringLiteral("manualVm"),  manualVm);
    engine.rootContext()->setContextProperty(QStringLiteral("logVm"),     logVm);

    engine.loadFromModule(QStringLiteral("LabGCS"), QStringLiteral("Main"));

    if (engine.rootObjects().isEmpty()) {
        logger->error(App, QStringLiteral("QML engine produced no root objects"));
        return -1;
    }
    return app.exec();
}
