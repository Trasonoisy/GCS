#include <QDateTime>
#include <QtTest/QtTest>

#include "Mission/MissionItem.h"
#include "Mission/MissionPlan.h"
#include "Safety/SafetyGate.h"
#include "Vehicle/VehicleState.h"

using gcs::comms::LinkKind;
using gcs::mission::MissionItem;
using gcs::mission::MissionPlan;
using gcs::safety::SafetyDecision;
using gcs::safety::SafetyGate;
using gcs::vehicle::LinkStatus;
using gcs::vehicle::VehicleState;

// Phase 8: SafetyGate must allow mission upload only over SITL transports
// (UDP/TCP + PX4/ArduPilot) and the Mock vehicle, never over hardware. These
// tests are companions to the Phase 7 hardware-mode tests but focus on the
// mission-upload predicate specifically — the new "real MAVLink mission
// upload to SITL" path lights up here.
class TestSafetyGateMissionSitl : public QObject
{
    Q_OBJECT
private slots:
    void uploadAllowedOnUdpPx4Sitl();
    void uploadAllowedOnUdpArdupilotSitl();
    void uploadAllowedOnTcpPx4Sitl();
    void uploadBlockedWhenLinkDisconnected();
    void uploadBlockedWhenHeartbeatStale();
    void uploadBlockedOnSerialEvenWithPx4Autopilot();
    void uploadBlockedOnEmptyMission();
    void uploadBlockedOnReplayLink();
    void uploadBlockedOnUnknownLink();
};

namespace {

VehicleState sitlVehicle(LinkKind link, const QString& autopilot)
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

MissionPlan threePoint()
{
    MissionPlan p;
    for (int i = 0; i < 3; ++i) {
        MissionItem it;
        it.command      = gcs::mission::cmd::NavWaypoint;
        it.frame        = gcs::mission::frame::GlobalRelativeAltInt;
        it.latitudeDeg  = 21.0 + i * 0.001;
        it.longitudeDeg = 105.0 + i * 0.001;
        it.altitudeM    = 50.0;
        p.items.append(it);
    }
    return p;
}

} // namespace

void TestSafetyGateMissionSitl::uploadAllowedOnUdpPx4Sitl()
{
    SafetyGate g;
    const auto d = g.canUploadMission(
        sitlVehicle(LinkKind::Udp, QStringLiteral("PX4")), threePoint());
    QVERIFY(d.allowed);
}

void TestSafetyGateMissionSitl::uploadAllowedOnUdpArdupilotSitl()
{
    SafetyGate g;
    const auto d = g.canUploadMission(
        sitlVehicle(LinkKind::Udp, QStringLiteral("ArduPilot")), threePoint());
    QVERIFY(d.allowed);
}

void TestSafetyGateMissionSitl::uploadAllowedOnTcpPx4Sitl()
{
    SafetyGate g;
    const auto d = g.canUploadMission(
        sitlVehicle(LinkKind::Tcp, QStringLiteral("PX4")), threePoint());
    QVERIFY(d.allowed);
}

void TestSafetyGateMissionSitl::uploadBlockedWhenLinkDisconnected()
{
    SafetyGate g;
    auto s = sitlVehicle(LinkKind::Udp, QStringLiteral("ArduPilot"));
    s.linkStatus = LinkStatus::Disconnected;
    const auto d = g.canUploadMission(s, threePoint());
    QVERIFY(!d.allowed);
    QVERIFY(d.reason.contains("link", Qt::CaseInsensitive));
}

void TestSafetyGateMissionSitl::uploadBlockedWhenHeartbeatStale()
{
    SafetyGate g;
    auto s = sitlVehicle(LinkKind::Udp, QStringLiteral("ArduPilot"));
    s.lastHeartbeatUtcMs -= 10'000;
    const auto d = g.canUploadMission(s, threePoint());
    QVERIFY(!d.allowed);
    QVERIFY(d.reason.contains("heartbeat", Qt::CaseInsensitive));
}

void TestSafetyGateMissionSitl::uploadBlockedOnSerialEvenWithPx4Autopilot()
{
    // SAFETY: this is the load-bearing test for Phase 8. A real Pixhawk
    // reports `autopilot=PX4` identically to PX4 SITL — only LinkKind
    // tells them apart, and only Serial-on-PX4 is blocked.
    SafetyGate g;
    const auto d = g.canUploadMission(
        sitlVehicle(LinkKind::Serial, QStringLiteral("PX4")), threePoint());
    QVERIFY(!d.allowed);
    QVERIFY(d.reason.contains("serial", Qt::CaseInsensitive));
}

void TestSafetyGateMissionSitl::uploadBlockedOnEmptyMission()
{
    SafetyGate g;
    MissionPlan empty;
    const auto d = g.canUploadMission(
        sitlVehicle(LinkKind::Udp, QStringLiteral("PX4")), empty);
    QVERIFY(!d.allowed);
    QVERIFY(d.reason.contains("empty", Qt::CaseInsensitive));
}

void TestSafetyGateMissionSitl::uploadBlockedOnReplayLink()
{
    SafetyGate g;
    const auto d = g.canUploadMission(
        sitlVehicle(LinkKind::Replay, QStringLiteral("PX4")), threePoint());
    QVERIFY(!d.allowed);
}

void TestSafetyGateMissionSitl::uploadBlockedOnUnknownLink()
{
    SafetyGate g;
    const auto d = g.canUploadMission(
        sitlVehicle(LinkKind::Unknown, QStringLiteral("PX4")), threePoint());
    QVERIFY(!d.allowed);
}

QTEST_MAIN(TestSafetyGateMissionSitl)
#include "tst_safety_gate_mission_sitl.moc"
