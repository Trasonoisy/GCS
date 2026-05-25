#include <QtTest/QtTest>

#include "Firmware/MavType.h"

using namespace gcs::firmware;

class TestMavType : public QObject
{
    Q_OBJECT
private slots:
    void copterFamilyMapsToCopter();
    void planeFamilyMapsToPlane();
    void roverFamilyMapsToRover();
    void submarineMapsToSub();
    void unknownMapsToOther();
    void namesArePlaintext();
};

void TestMavType::copterFamilyMapsToCopter()
{
    QCOMPARE(airframeKindFromMavType(2),  AirframeKind::Copter); // QUADROTOR
    QCOMPARE(airframeKindFromMavType(13), AirframeKind::Copter); // HEXAROTOR
    QCOMPARE(airframeKindFromMavType(14), AirframeKind::Copter); // OCTOROTOR
    QCOMPARE(airframeKindFromMavType(4),  AirframeKind::Copter); // HELICOPTER
}

void TestMavType::planeFamilyMapsToPlane()
{
    QCOMPARE(airframeKindFromMavType(1),  AirframeKind::Plane); // FIXED_WING
    QCOMPARE(airframeKindFromMavType(20), AirframeKind::Plane); // VTOL_QUADROTOR
    QCOMPARE(airframeKindFromMavType(21), AirframeKind::Plane); // VTOL_TILTROTOR
}

void TestMavType::roverFamilyMapsToRover()
{
    QCOMPARE(airframeKindFromMavType(10), AirframeKind::Rover); // GROUND_ROVER
    QCOMPARE(airframeKindFromMavType(11), AirframeKind::Rover); // SURFACE_BOAT
}

void TestMavType::submarineMapsToSub()
{
    QCOMPARE(airframeKindFromMavType(12), AirframeKind::Sub);
}

void TestMavType::unknownMapsToOther()
{
    QCOMPARE(airframeKindFromMavType(0),   AirframeKind::Other);
    QCOMPARE(airframeKindFromMavType(99),  AirframeKind::Other);
    QCOMPARE(airframeKindFromMavType(255), AirframeKind::Other);
}

void TestMavType::namesArePlaintext()
{
    QCOMPARE(airframeNameFromMavType(2),  QStringLiteral("Copter"));
    QCOMPARE(airframeNameFromMavType(1),  QStringLiteral("Plane"));
    QCOMPARE(airframeNameFromMavType(10), QStringLiteral("Rover"));
    QCOMPARE(airframeNameFromMavType(12), QStringLiteral("Sub"));
    QCOMPARE(airframeNameFromMavType(99), QStringLiteral("Other"));
}

QTEST_MAIN(TestMavType)
#include "tst_mav_type.moc"
