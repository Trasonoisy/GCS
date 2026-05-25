#include <QtTest/QtTest>

#include "Vehicle/VehicleState.h"

using gcs::vehicle::LinkStatus;
using gcs::vehicle::VehicleState;

class TestVehicleState : public QObject
{
    Q_OBJECT
private slots:
    void defaults();
};

void TestVehicleState::defaults()
{
    VehicleState s;

    QCOMPARE(s.systemId, 0);
    QCOMPARE(s.componentId, 0);
    QVERIFY(!s.armed);
    QCOMPARE(s.flightMode, QStringLiteral("UNKNOWN"));
    QCOMPARE(s.latitudeDeg, 0.0);
    QCOMPARE(s.longitudeDeg, 0.0);
    QCOMPARE(s.relativeAltitudeM, 0.0);
    QCOMPARE(s.headingDeg, 0.0);
    QCOMPARE(s.groundSpeedMps, 0.0);
    QCOMPARE(s.batteryVoltage, 0.0);
    QCOMPARE(s.batteryPercent, -1.0);
    QCOMPARE(s.gpsFixType, 0);
    QCOMPARE(s.satellitesVisible, 0);
    QVERIFY(s.linkStatus == LinkStatus::Disconnected);
    QCOMPARE(s.lastHeartbeatUtcMs, qint64(0));
    QVERIFY(!s.simulated);
}

QTEST_MAIN(TestVehicleState)
#include "tst_vehicle_state.moc"
