#include <QSignalSpy>
#include <QtTest/QtTest>

#include "Vehicle/VehicleStateStore.h"

using gcs::vehicle::LinkStatus;
using gcs::vehicle::VehicleStateStore;

class TestVehicleStateStore : public QObject
{
    Q_OBJECT
private slots:
    void emitsChangedOnUpdate();
    void coalescesNoOpUpdates();
    void updatesHeartbeatAndPromotesLink();
    void updatesIndividualFields();
};

void TestVehicleStateStore::emitsChangedOnUpdate()
{
    VehicleStateStore store;
    QSignalSpy spy(&store, &VehicleStateStore::stateChanged);

    store.updateArmed(true);
    QCOMPARE(spy.count(), 1);
    QVERIFY(store.state().armed);
}

void TestVehicleStateStore::coalescesNoOpUpdates()
{
    VehicleStateStore store;
    store.updateArmed(true);

    QSignalSpy spy(&store, &VehicleStateStore::stateChanged);
    store.updateArmed(true); // no change
    QCOMPARE(spy.count(), 0);
}

void TestVehicleStateStore::updatesHeartbeatAndPromotesLink()
{
    VehicleStateStore store;
    QCOMPARE(store.state().linkStatus, LinkStatus::Disconnected);

    store.updateHeartbeat(1, 1,
                          QStringLiteral("quadrotor"),
                          QStringLiteral("PX4"),
                          1'700'000'000'000LL);

    QCOMPARE(store.state().systemId, 1);
    QCOMPARE(store.state().componentId, 1);
    QCOMPARE(store.state().vehicleType, QStringLiteral("quadrotor"));
    QCOMPARE(store.state().autopilotType, QStringLiteral("PX4"));
    QCOMPARE(store.state().linkStatus, LinkStatus::Connected);
    QCOMPARE(store.state().lastHeartbeatUtcMs, 1'700'000'000'000LL);
}

void TestVehicleStateStore::updatesIndividualFields()
{
    VehicleStateStore store;
    store.updatePosition(10.0, 20.0, 30.0);
    store.updateHeading(123.4);
    store.updateGroundSpeed(5.5);
    store.updateBattery(15.8, 87.0);
    store.updateGps(6, 18);
    store.updateFlightMode(QStringLiteral("AUTO"));
    store.markSimulated(true);

    const auto& s = store.state();
    QCOMPARE(s.latitudeDeg, 10.0);
    QCOMPARE(s.longitudeDeg, 20.0);
    QCOMPARE(s.relativeAltitudeM, 30.0);
    QCOMPARE(s.headingDeg, 123.4);
    QCOMPARE(s.groundSpeedMps, 5.5);
    QCOMPARE(s.batteryVoltage, 15.8);
    QCOMPARE(s.batteryPercent, 87.0);
    QCOMPARE(s.gpsFixType, 6);
    QCOMPARE(s.satellitesVisible, 18);
    QCOMPARE(s.flightMode, QStringLiteral("AUTO"));
    QVERIFY(s.simulated);
}

QTEST_MAIN(TestVehicleStateStore)
#include "tst_vehicle_state_store.moc"
