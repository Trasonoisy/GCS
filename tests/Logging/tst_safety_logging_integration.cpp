#include <QDateTime>
#include <QtTest/QtTest>
#include <memory>

#include "Firmware/PX4FirmwarePlugin.h"
#include "Logging/EventLogger.h"
#include "Logging/LogEvent.h"
#include "Logging/MemoryLogSink.h"
#include "Manual/ManualControlManager.h"
#include "Manual/ManualControlState.h"
#include "Manual/MockJoystickBackend.h"
#include "Safety/SafetyGate.h"
#include "Simulation/MockVehicle.h"
#include "Vehicle/Vehicle.h"
#include "Vehicle/VehicleStateStore.h"

using gcs::firmware::PX4FirmwarePlugin;
using gcs::logging::EventLogger;
using gcs::logging::LogEvent;
using gcs::logging::MemoryLogSink;
using gcs::manual::ManualControlManager;
using gcs::manual::ManualControlState;
using gcs::manual::manualControlStateName;
using gcs::manual::MockJoystickBackend;
using gcs::safety::SafetyGate;
using gcs::simulation::MockVehicle;
using gcs::vehicle::Vehicle;

// Bridges the manual-control state machine into the EventLogger the same
// way main.cpp does. Lets us assert that safety-relevant transitions land
// in the log under the right category.
namespace {
void wireSafetyLogging(ManualControlManager* mgr, EventLogger* log)
{
    QObject::connect(mgr, &ManualControlManager::stateChanged, log,
        [log, mgr](ManualControlState s) {
            const QString name = QString::fromLatin1(manualControlStateName(s));
            const QString reason = mgr->blockedReason();
            const QString sev = (s == ManualControlState::Failsafe
                                 || s == ManualControlState::Blocked)
                                ? QStringLiteral("warn") : QStringLiteral("info");
            QString msg = QStringLiteral("Manual control state → %1").arg(name);
            if (!reason.isEmpty()) msg += QStringLiteral(" (%1)").arg(reason);
            log->log(QStringLiteral("Safety"), sev, msg,
                {{QStringLiteral("state"), name},
                 {QStringLiteral("reason"), reason}});
        });
}
} // namespace

class TestSafetyLoggingIntegration : public QObject
{
    Q_OBJECT
private slots:
    void manualEnableEmitsSafetyLogEntries();
    void staleHeartbeatLogsFailsafeWithReason();
};

void TestSafetyLoggingIntegration::manualEnableEmitsSafetyLogEntries()
{
    EventLogger lg;
    auto mem = std::make_shared<MemoryLogSink>();
    lg.addSink(mem);

    PX4FirmwarePlugin fw;
    Vehicle veh(200, 1, &fw);
    MockVehicle mock(&veh);
    MockJoystickBackend joystick;
    SafetyGate gate;
    ManualControlManager mgr(&joystick, &gate);
    mgr.setActiveVehicle(&veh);
    mgr.setSink(&mock);
    wireSafetyLogging(&mgr, &lg);

    // Push a fresh heartbeat + connect joystick + enable.
    mock.tickHeartbeatNow();
    joystick.setConnected(true);
    mgr.enable();
    QCOMPARE(mgr.state(), ManualControlState::Active);

    QVERIFY(mem->size() >= 1);
    bool sawActive = false;
    for (const auto& e : mem->events()) {
        if (e.category == "Safety" && e.message.contains("Active")) sawActive = true;
    }
    QVERIFY(sawActive);
}

void TestSafetyLoggingIntegration::staleHeartbeatLogsFailsafeWithReason()
{
    EventLogger lg;
    auto mem = std::make_shared<MemoryLogSink>();
    lg.addSink(mem);

    PX4FirmwarePlugin fw;
    Vehicle veh(200, 1, &fw);
    MockVehicle mock(&veh);
    MockJoystickBackend joystick;
    SafetyGate gate;
    ManualControlManager mgr(&joystick, &gate);
    mgr.setActiveVehicle(&veh);
    mgr.setSink(&mock);
    wireSafetyLogging(&mgr, &lg);

    mock.tickHeartbeatNow();
    joystick.setConnected(true);
    mgr.enable();
    QCOMPARE(mgr.state(), ManualControlState::Active);

    // Backdate the heartbeat so the gate trips on the next state-store update.
    veh.stateStore()->updateHeartbeat(
        veh.systemId(), veh.componentId(),
        QStringLiteral("quadrotor"), QStringLiteral("PX4"),
        QDateTime::currentMSecsSinceEpoch() - 10'000);
    QCOMPARE(mgr.state(), ManualControlState::Failsafe);

    bool sawFailsafeWithReason = false;
    for (const auto& e : mem->events()) {
        if (e.category == "Safety"
            && e.severity == "warn"
            && e.message.contains("Failsafe")
            && !e.metadata.value("reason").toString().isEmpty()) {
            sawFailsafeWithReason = true;
        }
    }
    QVERIFY(sawFailsafeWithReason);
}

QTEST_MAIN(TestSafetyLoggingIntegration)
#include "tst_safety_logging_integration.moc"
