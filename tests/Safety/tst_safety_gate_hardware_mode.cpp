#include <QDateTime>
#include <QtTest/QtTest>

#include "Manual/JoystickState.h"
#include "Mission/MissionItem.h"
#include "Mission/MissionPlan.h"
#include "Safety/SafetyGate.h"
#include "Vehicle/VehicleState.h"

using gcs::comms::LinkKind;
using gcs::manual::JoystickState;
using gcs::mission::MissionItem;
using gcs::mission::MissionPlan;
using gcs::safety::SafetyDecision;
using gcs::safety::SafetyGate;
using gcs::vehicle::LinkStatus;
using gcs::vehicle::VehicleState;

// Phase 7: SafetyGate must distinguish Hardware (serial) from SITL (UDP/TCP)
// transports. A real Pixhawk reports `autopilot=PX4` identically to a PX4
// SITL — the only reliable discriminator at the protocol layer is which
// LinkInterface delivered the bytes. We stamp that into VehicleState.linkKind
// and the gate keys off it.
class TestSafetyGateHardwareMode : public QObject
{
    Q_OBJECT
private slots:
    void manualControlBlockedOnSerialEvenWithPx4Autopilot();
    void manualControlBlockedOnSerialEvenWithArdupilotAutopilot();
    void manualContinueBlockedOnSerial();
    void manualControlAllowedOnUdpPx4();
    void manualControlAllowedOnTcpArduPilot();
    void manualControlBlockedOnUnknownLinkKind();
    void missionUploadBlockedOnSerialLink();
    void missionUploadAllowedOnMockLink();
    void armTakeoffAndRtlBlockedOnSerialLink();
};

namespace {

VehicleState liveVehicle(LinkKind link, const QString& autopilot)
{
    VehicleState s;
    s.systemId           = 1;
    s.componentId        = 1;
    s.vehicleType        = QStringLiteral("Copter");
    s.autopilotType      = autopilot;
    s.linkStatus         = LinkStatus::Connected;
    s.lastHeartbeatUtcMs = QDateTime::currentMSecsSinceEpoch();
    s.linkKind           = link;
    s.simulated          = false;
    return s;
}

JoystickState connectedStick()
{
    JoystickState j;
    j.connected = true;
    j.name      = "MockJoystick";
    return j;
}

MissionPlan oneWaypointMission()
{
    MissionPlan p;
    MissionItem it;
    it.command     = gcs::mission::cmd::NavWaypoint;
    it.frame       = gcs::mission::frame::GlobalRelativeAltInt;
    it.latitudeDeg = 21.0;
    it.longitudeDeg = 105.0;
    it.altitudeM   = 50.0;
    p.items.append(it);
    return p;
}

} // namespace

void TestSafetyGateHardwareMode::manualControlBlockedOnSerialEvenWithPx4Autopilot()
{
    SafetyGate g;
    const auto d = g.canStartManualControl(
        liveVehicle(LinkKind::Serial, QStringLiteral("PX4")),
        connectedStick(), /*operatorOptedIn*/ true);
    QVERIFY(!d.allowed);
    QVERIFY(d.reason.contains("Real-hardware", Qt::CaseInsensitive));
}

void TestSafetyGateHardwareMode::manualControlBlockedOnSerialEvenWithArdupilotAutopilot()
{
    SafetyGate g;
    const auto d = g.canStartManualControl(
        liveVehicle(LinkKind::Serial, QStringLiteral("ArduPilot")),
        connectedStick(), true);
    QVERIFY(!d.allowed);
}

void TestSafetyGateHardwareMode::manualContinueBlockedOnSerial()
{
    SafetyGate g;
    const auto d = g.canContinueManualControl(
        liveVehicle(LinkKind::Serial, QStringLiteral("PX4")),
        connectedStick());
    QVERIFY(!d.allowed);
}

void TestSafetyGateHardwareMode::manualControlAllowedOnUdpPx4()
{
    SafetyGate g;
    const auto d = g.canStartManualControl(
        liveVehicle(LinkKind::Udp, QStringLiteral("PX4")),
        connectedStick(), true);
    QVERIFY(d.allowed);
}

void TestSafetyGateHardwareMode::manualControlAllowedOnTcpArduPilot()
{
    SafetyGate g;
    const auto d = g.canStartManualControl(
        liveVehicle(LinkKind::Tcp, QStringLiteral("ArduPilot")),
        connectedStick(), true);
    QVERIFY(d.allowed);
}

void TestSafetyGateHardwareMode::manualControlBlockedOnUnknownLinkKind()
{
    // Fail-closed: an unstamped vehicle must not be treated as SITL.
    SafetyGate g;
    const auto d = g.canStartManualControl(
        liveVehicle(LinkKind::Unknown, QStringLiteral("PX4")),
        connectedStick(), true);
    QVERIFY(!d.allowed);
}

void TestSafetyGateHardwareMode::missionUploadBlockedOnSerialLink()
{
    SafetyGate g;
    const auto d = g.canUploadMission(
        liveVehicle(LinkKind::Serial, QStringLiteral("PX4")),
        oneWaypointMission());
    QVERIFY(!d.allowed);
    QVERIFY(d.reason.contains("serial", Qt::CaseInsensitive));
}

void TestSafetyGateHardwareMode::missionUploadAllowedOnMockLink()
{
    // MockVehicle's link kind is Mock; mission upload to that path is the
    // only real upload route that exists in this build today.
    SafetyGate g;
    auto s = liveVehicle(LinkKind::Mock, QStringLiteral("PX4"));
    s.simulated = true;
    const auto d = g.canUploadMission(s, oneWaypointMission());
    QVERIFY(d.allowed);
}

void TestSafetyGateHardwareMode::armTakeoffAndRtlBlockedOnSerialLink()
{
    SafetyGate g;
    const auto s = liveVehicle(LinkKind::Serial, QStringLiteral("PX4"));

    const auto arm = g.canArm(s);
    const auto takeoff = g.canTakeoff(s, 10.0);
    const auto rtl = g.canTriggerRTL(s);

    QVERIFY(!arm.allowed);
    QVERIFY(!takeoff.allowed);
    QVERIFY(!rtl.allowed);
}

QTEST_MAIN(TestSafetyGateHardwareMode)
#include "tst_safety_gate_hardware_mode.moc"
