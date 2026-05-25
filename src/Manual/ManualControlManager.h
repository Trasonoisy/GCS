#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>

#include "ManualControlState.h"

namespace gcs::safety  { class SafetyGate; struct SafetyDecision; }
namespace gcs::vehicle { class Vehicle; }

namespace gcs::manual {

class IManualControlSink;
class JoystickBackend;

// Owns the manual-control state machine and the send loop. One instance
// drives the currently active Vehicle; on active-vehicle change the manager
// is rebound to the new vehicle and its state resets to Disabled.
//
// SAFETY: this is the only object that calls IManualControlSink. It runs
// SafetyGate::canStartManualControl before transitioning to Active and
// SafetyGate::canContinueManualControl before every send. On any failure
// the send loop stops and state becomes Failsafe/Blocked.
class ManualControlManager : public QObject
{
    Q_OBJECT
public:
    ManualControlManager(JoystickBackend* joystick,
                         const gcs::safety::SafetyGate* gate,
                         QObject* parent = nullptr);
    ~ManualControlManager() override;

    void setActiveVehicle(gcs::vehicle::Vehicle* vehicle);
    void setSink(IManualControlSink* sink);
    void setSendRateHz(int hz);

    gcs::vehicle::Vehicle* activeVehicle() const { return m_vehicle; }
    IManualControlSink*    sink()          const { return m_sink; }
    JoystickBackend*       joystick()      const { return m_joystick; }

    ManualControlState state()         const { return m_state; }
    QString            blockedReason() const { return m_blockedReason; }
    QStringList        checklist()     const { return m_checklist; }

    // Latest packed MAVLink values that would be sent (or were last sent).
    int lastX() const { return m_lastX; }
    int lastY() const { return m_lastY; }
    int lastZ() const { return m_lastZ; }
    int lastR() const { return m_lastR; }

    int totalSamplesSent() const { return m_samplesSent; }

public slots:
    void enable();
    void disable();
    void requestEvaluate(); // recompute checklist + state (idempotent)

signals:
    void stateChanged(gcs::manual::ManualControlState state);
    void checklistChanged();
    void blockedReasonChanged();
    void sampleSent(int x, int y, int z, int r);

private slots:
    void onJoystickChanged();
    void onVehicleStateChanged();
    void onTick();

private:
    void rebuildChecklist();
    void setState(ManualControlState s);
    void setBlockedReason(const QString& r);
    void enterActiveIfPossible();
    void leaveActive(const QString& reason, ManualControlState newState);

    JoystickBackend*               m_joystick;
    const gcs::safety::SafetyGate* m_gate;
    IManualControlSink*            m_sink     = nullptr;
    gcs::vehicle::Vehicle*         m_vehicle  = nullptr;

    bool m_operatorOptedIn = false;
    ManualControlState m_state = ManualControlState::Disabled;
    QString     m_blockedReason;
    QStringList m_checklist;

    QTimer m_sendTimer;
    int    m_sendRateHz = 10;
    int    m_samplesSent = 0;
    int    m_lastX = 0, m_lastY = 0, m_lastZ = 500, m_lastR = 0;
};

} // namespace gcs::manual
