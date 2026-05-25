#include "ManualControlManager.h"

#include <QDateTime>

#include "AxisMapper.h"
#include "IManualControlSink.h"
#include "JoystickBackend.h"
#include "JoystickState.h"
#include "Safety/SafetyDecision.h"
#include "Safety/SafetyGate.h"
#include "Vehicle/Vehicle.h"
#include "Vehicle/VehicleStateStore.h"

namespace gcs::manual {

using gcs::safety::SafetyDecision;
using gcs::safety::SafetyGate;
using gcs::vehicle::LinkStatus;
using gcs::vehicle::Vehicle;

ManualControlManager::ManualControlManager(JoystickBackend* joystick,
                                           const SafetyGate* gate,
                                           QObject* parent)
    : QObject(parent), m_joystick(joystick), m_gate(gate)
{
    Q_ASSERT(joystick);
    Q_ASSERT(gate);
    connect(joystick, &JoystickBackend::stateChanged,
            this, &ManualControlManager::onJoystickChanged);

    m_sendTimer.setInterval(1000 / m_sendRateHz);
    m_sendTimer.setSingleShot(false);
    connect(&m_sendTimer, &QTimer::timeout,
            this, &ManualControlManager::onTick);

    rebuildChecklist();
}

ManualControlManager::~ManualControlManager() = default;

void ManualControlManager::setActiveVehicle(Vehicle* vehicle)
{
    if (m_vehicle == vehicle) return;

    // Disconnect old vehicle's state-store signal if any.
    if (m_vehicle) {
        disconnect(m_vehicle->stateStore(), nullptr, this, nullptr);
        // SAFETY: active vehicle changed mid-session - tear down immediately.
        if (m_state == ManualControlState::Active) {
            leaveActive(QStringLiteral("Active vehicle changed"),
                        ManualControlState::Failsafe);
        }
        // Force operator opt-in to require an explicit re-enable on the new
        // vehicle. Phase 4 brief: "active vehicle changes unexpectedly" must
        // block manual control.
        m_operatorOptedIn = false;
    }

    m_vehicle = vehicle;

    if (m_vehicle) {
        connect(m_vehicle->stateStore(),
                &gcs::vehicle::VehicleStateStore::stateChanged,
                this, &ManualControlManager::onVehicleStateChanged);
    }
    requestEvaluate();
}

void ManualControlManager::setSink(IManualControlSink* sink)
{
    m_sink = sink;
    requestEvaluate();
}

void ManualControlManager::setSendRateHz(int hz)
{
    if (hz <= 0 || hz > 100) return; // sanity clamp
    m_sendRateHz = hz;
    m_sendTimer.setInterval(1000 / hz);
}

void ManualControlManager::enable()
{
    if (m_operatorOptedIn) return;
    m_operatorOptedIn = true;
    requestEvaluate();
}

void ManualControlManager::disable()
{
    const bool wasOpted = m_operatorOptedIn;
    m_operatorOptedIn = false;
    if (m_state == ManualControlState::Active) {
        leaveActive(QStringLiteral("Operator disabled manual control"),
                    ManualControlState::Disabled);
    } else if (wasOpted) {
        setState(ManualControlState::Disabled);
    }
    setBlockedReason(QString());
    rebuildChecklist();
}

void ManualControlManager::requestEvaluate()
{
    rebuildChecklist();

    if (!m_operatorOptedIn) {
        if (m_state == ManualControlState::Active) {
            leaveActive(QStringLiteral("Operator disabled manual control"),
                        ManualControlState::Disabled);
        } else {
            setState(ManualControlState::Disabled);
        }
        return;
    }

    if (!m_joystick->isConnected()) {
        if (m_state == ManualControlState::Active) {
            leaveActive(QStringLiteral("Joystick disconnected"),
                        ManualControlState::WaitingForJoystick);
        } else {
            setState(ManualControlState::WaitingForJoystick);
        }
        return;
    }

    enterActiveIfPossible();
}

void ManualControlManager::enterActiveIfPossible()
{
    if (!m_vehicle) {
        setBlockedReason(QStringLiteral("No active vehicle"));
        setState(ManualControlState::Blocked);
        return;
    }
    const auto& vs = m_vehicle->stateStore()->state();
    const SafetyDecision d = m_gate->canStartManualControl(
        vs, m_joystick->state(), m_operatorOptedIn);
    if (!d.allowed) {
        setBlockedReason(d.reason);
        setState(ManualControlState::Blocked);
        return;
    }
    setBlockedReason(QString());
    if (m_state != ManualControlState::Active) {
        setState(ManualControlState::Ready);
        if (m_sink) {
            setState(ManualControlState::Active);
            m_sendTimer.start();
            m_vehicle->appendEvent(QStringLiteral(
                "[MANUAL] Send loop started - sink=%1 (%2)")
                .arg(m_sink->sinkName(),
                     m_sink->isSimulated() ? QStringLiteral("simulated")
                                           : QStringLiteral("real")));
        } else {
            setBlockedReason(QStringLiteral("No manual-control sink wired"));
            setState(ManualControlState::Blocked);
        }
    }
}

void ManualControlManager::leaveActive(const QString& reason,
                                       ManualControlState newState)
{
    m_sendTimer.stop();
    setBlockedReason(reason);
    setState(newState);
    if (m_vehicle) {
        m_vehicle->appendEvent(QStringLiteral(
            "[MANUAL] Send loop stopped - %1").arg(reason));
    }
}

void ManualControlManager::onJoystickChanged()
{
    requestEvaluate();
}

void ManualControlManager::onVehicleStateChanged()
{
    // Active path: run the continue check; on failure, demote.
    if (m_state == ManualControlState::Active && m_vehicle) {
        const auto& vs = m_vehicle->stateStore()->state();
        const SafetyDecision d = m_gate->canContinueManualControl(
            vs, m_joystick->state());
        if (!d.allowed) {
            leaveActive(d.reason, ManualControlState::Failsafe);
            rebuildChecklist();
            return;
        }
    }
    // Non-active path: re-evaluate so e.g. heartbeat arrival promotes us
    // out of Blocked.
    if (m_state == ManualControlState::Blocked
        || m_state == ManualControlState::Ready
        || m_state == ManualControlState::Failsafe) {
        // Failsafe stays sticky until operator disables and re-enables; only
        // reset here for Blocked/Ready cycling.
        if (m_state != ManualControlState::Failsafe) {
            requestEvaluate();
        } else {
            rebuildChecklist();
        }
    }
}

void ManualControlManager::onTick()
{
    if (m_state != ManualControlState::Active) {
        m_sendTimer.stop();
        return;
    }
    if (!m_vehicle || !m_sink) {
        leaveActive(QStringLiteral("Sink or vehicle vanished"),
                    ManualControlState::Failsafe);
        return;
    }

    const auto& vs = m_vehicle->stateStore()->state();
    const SafetyDecision d = m_gate->canContinueManualControl(
        vs, m_joystick->state());
    if (!d.allowed) {
        leaveActive(d.reason, ManualControlState::Failsafe);
        rebuildChecklist();
        return;
    }

    const auto& js = m_joystick->state();
    m_lastX = axis::packAxis(js.pitch);
    m_lastY = axis::packAxis(js.roll);
    m_lastZ = axis::packThrottle(js.throttle);
    m_lastR = axis::packAxis(js.yaw);
    ++m_samplesSent;

    m_sink->onManualControlSample(static_cast<int16_t>(m_lastX),
                                  static_cast<int16_t>(m_lastY),
                                  static_cast<int16_t>(m_lastZ),
                                  static_cast<int16_t>(m_lastR),
                                  js.buttons);
    emit sampleSent(m_lastX, m_lastY, m_lastZ, m_lastR);
}

void ManualControlManager::rebuildChecklist()
{
    QStringList c;
    auto push = [&c](bool ok, const QString& label) {
        c.append(QStringLiteral("%1 %2").arg(ok ? "[OK]" : "[BLOCKED]", label));
    };

    push(m_operatorOptedIn, QStringLiteral("Operator enabled"));
    push(m_joystick->isConnected(), QStringLiteral("Joystick connected"));

    bool vehicleOk = false;
    bool linkOk    = false;
    bool simOrSitl = false;
    if (m_vehicle) {
        const auto& vs = m_vehicle->stateStore()->state();
        vehicleOk = vs.systemId > 0;
        linkOk    = vs.linkStatus == LinkStatus::Connected
                    && vs.lastHeartbeatUtcMs > 0
                    && (QDateTime::currentMSecsSinceEpoch() - vs.lastHeartbeatUtcMs)
                       <= gcs::safety::SafetyGate::kFreshHeartbeatMs;
        const bool sitlTransport =
            (vs.linkKind == gcs::comms::LinkKind::Udp
             || vs.linkKind == gcs::comms::LinkKind::Tcp);
        const bool knownSitlAutopilot =
            vs.autopilotType.compare(QStringLiteral("PX4"), Qt::CaseInsensitive) == 0
            || vs.autopilotType.compare(QStringLiteral("ArduPilot"), Qt::CaseInsensitive) == 0;
        simOrSitl = vs.simulated
                    || vs.linkKind == gcs::comms::LinkKind::Mock
                    || (sitlTransport && knownSitlAutopilot);
    }
    push(vehicleOk, QStringLiteral("Active vehicle"));
    push(linkOk,    QStringLiteral("Heartbeat fresh"));
    push(simOrSitl, QStringLiteral("Simulation or SITL"));
    push(m_sink != nullptr, QStringLiteral("Sink wired"));

    if (m_checklist != c) {
        m_checklist = c;
        emit checklistChanged();
    }
}

void ManualControlManager::setState(ManualControlState s)
{
    if (m_state == s) return;
    m_state = s;
    emit stateChanged(s);
    rebuildChecklist();
}

void ManualControlManager::setBlockedReason(const QString& r)
{
    if (m_blockedReason == r) return;
    m_blockedReason = r;
    emit blockedReasonChanged();
}

} // namespace gcs::manual
