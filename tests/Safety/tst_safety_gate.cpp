#include <QDateTime>
#include <QtTest/QtTest>

#include "Manual/JoystickState.h"
#include "Safety/SafetyGate.h"
#include "Vehicle/VehicleState.h"

using gcs::manual::JoystickState;
using gcs::safety::SafetyDecision;
using gcs::safety::SafetyGate;
using gcs::vehicle::LinkStatus;
using gcs::vehicle::VehicleState;

class TestSafetyGate : public QObject
{
    Q_OBJECT
private slots:
    void manualBlockedWithoutOptIn();
    void manualBlockedWithoutVehicle();
    void manualBlockedWhenLinkDisconnected();
    void manualBlockedWhenHeartbeatStale();
    void manualBlockedWithoutJoystick();
    void manualBlockedForRealHardware();
    void manualAllowedInSimulation();
    void manualAllowedInPx4Sitl();
    void continueBlockedOnStaleHeartbeat();
    void otherActionsAreBlockedInThisPhase();
};

namespace {
VehicleState liveSimVehicle()
{
    VehicleState s;
    s.systemId           = 200;
    s.componentId        = 1;
    s.simulated          = true;
    s.vehicleType        = "quadrotor";
    s.autopilotType      = "PX4";
    s.linkStatus         = LinkStatus::Connected;
    s.lastHeartbeatUtcMs = QDateTime::currentMSecsSinceEpoch();
    return s;
}
JoystickState connectedStick()
{
    JoystickState j;
    j.connected = true;
    j.name      = "MockJoystick";
    return j;
}
} // namespace

void TestSafetyGate::manualBlockedWithoutOptIn()
{
    SafetyGate g;
    const auto d = g.canStartManualControl(liveSimVehicle(), connectedStick(),
                                           /*operatorOptedIn*/ false);
    QVERIFY(!d.allowed);
    QVERIFY(d.reason.contains("Operator"));
}

void TestSafetyGate::manualBlockedWithoutVehicle()
{
    SafetyGate g;
    VehicleState s; // sysid = 0
    const auto d = g.canStartManualControl(s, connectedStick(), true);
    QVERIFY(!d.allowed);
}

void TestSafetyGate::manualBlockedWhenLinkDisconnected()
{
    SafetyGate g;
    auto s = liveSimVehicle();
    s.linkStatus = LinkStatus::Disconnected;
    const auto d = g.canStartManualControl(s, connectedStick(), true);
    QVERIFY(!d.allowed);
    QVERIFY(d.reason.contains("Link") || d.reason.contains("heartbeat"));
}

void TestSafetyGate::manualBlockedWhenHeartbeatStale()
{
    SafetyGate g;
    auto s = liveSimVehicle();
    s.lastHeartbeatUtcMs -= 5'000; // 5 s old, gate threshold is 2.5 s
    const auto d = g.canStartManualControl(s, connectedStick(), true);
    QVERIFY(!d.allowed);
}

void TestSafetyGate::manualBlockedWithoutJoystick()
{
    SafetyGate g;
    JoystickState j; j.connected = false;
    const auto d = g.canStartManualControl(liveSimVehicle(), j, true);
    QVERIFY(!d.allowed);
    QVERIFY(d.reason.contains("Joystick"));
}

void TestSafetyGate::manualBlockedForRealHardware()
{
    SafetyGate g;
    auto s = liveSimVehicle();
    s.simulated = false;
    s.autopilotType = "Generic"; // not PX4 -> real hardware in our model
    const auto d = g.canStartManualControl(s, connectedStick(), true);
    QVERIFY(!d.allowed);
    QVERIFY(d.reason.contains("Real-hardware"));
}

void TestSafetyGate::manualAllowedInSimulation()
{
    SafetyGate g;
    const auto d = g.canStartManualControl(liveSimVehicle(), connectedStick(), true);
    QVERIFY(d.allowed);
}

void TestSafetyGate::manualAllowedInPx4Sitl()
{
    SafetyGate g;
    auto s = liveSimVehicle();
    s.simulated = false;
    // Phase 7: SafetyGate now requires a non-hardware transport. PX4 SITL
    // arrives over UDP — stamp the link kind so the predicate accepts it.
    s.linkKind  = gcs::comms::LinkKind::Udp;
    const auto d = g.canStartManualControl(s, connectedStick(), true);
    QVERIFY(d.allowed);
}

void TestSafetyGate::continueBlockedOnStaleHeartbeat()
{
    SafetyGate g;
    auto s = liveSimVehicle();
    s.lastHeartbeatUtcMs -= 10'000;
    const auto d = g.canContinueManualControl(s, connectedStick());
    QVERIFY(!d.allowed);
}

void TestSafetyGate::otherActionsAreBlockedInThisPhase()
{
    SafetyGate g;
    const auto s = liveSimVehicle();
    QVERIFY(!g.canArm(s).allowed);
    QVERIFY(!g.canTakeoff(s, 5.0).allowed);
    QVERIFY(!g.canTriggerRTL(s).allowed);
}

QTEST_MAIN(TestSafetyGate)
#include "tst_safety_gate.moc"
