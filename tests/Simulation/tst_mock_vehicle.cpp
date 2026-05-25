#include <QSignalSpy>
#include <QtTest/QtTest>

#include "Firmware/PX4FirmwarePlugin.h"
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
    void appendsStartEvent();
};

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
