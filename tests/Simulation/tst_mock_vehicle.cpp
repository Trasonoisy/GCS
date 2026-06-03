#include <QSignalSpy>
#include <QtTest/QtTest>

#include "Firmware/PX4FirmwarePlugin.h"
#include "Mission/MissionPlan.h"
#include "Simulation/MockVehicle.h"
#include "Vehicle/Vehicle.h"
#include "Vehicle/VehicleStateStore.h"

using gcs::firmware::PX4FirmwarePlugin;
using gcs::simulation::MockVehicle;
using gcs::vehicle::LinkStatus;
using gcs::vehicle::Vehicle;

class TestMockVehicle : public QObject
{
    Q_OBJECT
private slots:
    void marksStateSimulated();
    void heartbeatTickPromotesLinkAndStampsTime();
    void telemetryTickUpdatesMotionFields();
    void manualSamplesDriveSimulatedTelemetry();
    void missionPreviewStartsAtFirstWaypointAndMoves();
    void missionPreviewCompletesAndDisarms();
    void appendsStartEvent();
};

namespace {
gcs::mission::MissionPlan previewPlan(double lonDelta)
{
    gcs::mission::MissionPlan plan;
    plan.cruiseSpeedMps = 10.0;

    gcs::mission::MissionItem first;
    first.seq = 0;
    first.latitudeDeg = 21.0285;
    first.longitudeDeg = 105.8048;
    first.altitudeM = 10.0;

    gcs::mission::MissionItem second = first;
    second.seq = 1;
    second.longitudeDeg += lonDelta;
    second.altitudeM = 20.0;

    plan.items = {first, second};
    return plan;
}
} // namespace

void TestMockVehicle::marksStateSimulated()
{
    PX4FirmwarePlugin fw;
    Vehicle vehicle(1, 1, &fw);
    MockVehicle mock(&vehicle);

    QVERIFY(vehicle.stateStore()->state().simulated);
}

void TestMockVehicle::heartbeatTickPromotesLinkAndStampsTime()
{
    PX4FirmwarePlugin fw;
    Vehicle vehicle(7, 1, &fw);
    MockVehicle mock(&vehicle);

    QVERIFY(vehicle.stateStore()->state().linkStatus == LinkStatus::Disconnected);

    mock.tickHeartbeatNow();

    const auto& s = vehicle.stateStore()->state();
    QCOMPARE(s.systemId, 7);
    QCOMPARE(s.componentId, 1);
    QCOMPARE(s.linkStatus, LinkStatus::Connected);
    QVERIFY(s.lastHeartbeatUtcMs > 0);
    QCOMPARE(s.autopilotType, QStringLiteral("MockAutopilot"));
}

void TestMockVehicle::telemetryTickUpdatesMotionFields()
{
    PX4FirmwarePlugin fw;
    Vehicle vehicle(1, 1, &fw);
    MockVehicle mock(&vehicle);

    const double headingBefore = vehicle.stateStore()->state().headingDeg;
    mock.tickTelemetryNow();
    const auto& s = vehicle.stateStore()->state();

    QVERIFY(s.latitudeDeg != 0.0);
    QVERIFY(s.longitudeDeg != 0.0);
    QVERIFY(s.headingDeg != headingBefore);
    QVERIFY(s.groundSpeedMps >= 2.0); // sin(...) baseline ~3 m/s
}

void TestMockVehicle::manualSamplesDriveSimulatedTelemetry()
{
    PX4FirmwarePlugin fw;
    Vehicle vehicle(1, 1, &fw);
    MockVehicle mock(&vehicle);

    mock.tickTelemetryNow();
    const auto before = vehicle.stateStore()->state();

    mock.onManualControlSample(/*pitch*/ 1000, /*roll*/ 500,
                               /*throttle*/ 1000, /*yaw*/ 1000,
                               /*buttons*/ 0);
    mock.tickTelemetryNow();
    const auto after = vehicle.stateStore()->state();

    QVERIFY(after.latitudeDeg != before.latitudeDeg
            || after.longitudeDeg != before.longitudeDeg);
    QVERIFY(after.relativeAltitudeM > before.relativeAltitudeM);
    QVERIFY(after.headingDeg != before.headingDeg);
    QVERIFY(after.rollDeg > 0.0);
    QVERIFY(after.pitchDeg > 0.0);
    QVERIFY(after.groundSpeedMps > 0.0);
    QCOMPARE(mock.manualSampleCount(), 1);
}

void TestMockVehicle::missionPreviewStartsAtFirstWaypointAndMoves()
{
    PX4FirmwarePlugin fw;
    Vehicle vehicle(1, 1, &fw);
    MockVehicle mock(&vehicle);

    const auto plan = previewPlan(0.001);
    QVERIFY(mock.startMissionPreview(plan, 10.0));

    auto s = vehicle.stateStore()->state();
    QVERIFY(mock.missionPreviewActive());
    QCOMPARE(s.flightMode, QStringLiteral("AUTO (PREVIEW)"));
    QVERIFY(s.armed);
    QCOMPARE(s.latitudeDeg, plan.items.first().latitudeDeg);
    QCOMPARE(s.longitudeDeg, plan.items.first().longitudeDeg);

    mock.tickTelemetryNow();
    s = vehicle.stateStore()->state();
    QVERIFY(s.longitudeDeg > plan.items.first().longitudeDeg);
    QVERIFY(s.longitudeDeg < plan.items.last().longitudeDeg);
    QVERIFY(s.relativeAltitudeM > plan.items.first().altitudeM);
    QVERIFY(s.groundSpeedMps > 0.0);
    QVERIFY(mock.missionPreviewProgress() > 0.0);
}

void TestMockVehicle::missionPreviewCompletesAndDisarms()
{
    PX4FirmwarePlugin fw;
    Vehicle vehicle(1, 1, &fw);
    MockVehicle mock(&vehicle);

    const auto plan = previewPlan(0.00001);
    QSignalSpy completed(&mock, &MockVehicle::missionPreviewCompleted);
    QVERIFY(mock.startMissionPreview(plan, 50.0));

    mock.tickTelemetryNow();

    const auto& s = vehicle.stateStore()->state();
    QVERIFY(!mock.missionPreviewActive());
    QCOMPARE(completed.count(), 1);
    QCOMPARE(s.flightMode, QStringLiteral("MISSION COMPLETE"));
    QVERIFY(!s.armed);
    QVERIFY(qAbs(s.longitudeDeg - plan.items.last().longitudeDeg) < 1e-9);
    QVERIFY(mock.missionPreviewProgress() >= 1.0);
}

void TestMockVehicle::appendsStartEvent()
{
    PX4FirmwarePlugin fw;
    Vehicle vehicle(1, 1, &fw);
    MockVehicle mock(&vehicle);

    QSignalSpy spy(&vehicle, &Vehicle::eventAppended);
    mock.start();
    QVERIFY(spy.count() >= 1);
    QVERIFY(spy.takeFirst().at(0).toString().contains("MockVehicle started"));
    mock.stop();
}

QTEST_MAIN(TestMockVehicle)
#include "tst_mock_vehicle.moc"
