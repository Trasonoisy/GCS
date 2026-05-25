#include <QtTest/QtTest>

#include "Firmware/ArduCopterFirmwarePlugin.h"
#include "Firmware/ArduPilotFirmwarePlugin.h"
#include "Firmware/ArduPlaneFirmwarePlugin.h"
#include "Firmware/ArduRoverFirmwarePlugin.h"
#include "Firmware/ArduSubFirmwarePlugin.h"

using namespace gcs::firmware;

class TestArdupilotModes : public QObject
{
    Q_OBJECT
private slots:
    void firmwareNameIsArduPilotForAllSubclasses();
    void copterBasicModes();
    void copterUnknownFallsBack();
    void planeBasicModes();
    void roverBasicModes();
    void subBasicModes();
    void baseClassFallback();
    void homeIsMissionSeq0();
    void rcChannelsOverrideStaysDisabled();
    void supportedMissionCommandsRemainsEmpty();
};

void TestArdupilotModes::firmwareNameIsArduPilotForAllSubclasses()
{
    // The family name is consistent so the UI's autopilot label stays
    // stable across airframes. Vehicle kind is in VehicleState.vehicleType.
    QCOMPARE(ArduPilotFirmwarePlugin().firmwareName(),    QStringLiteral("ArduPilot"));
    QCOMPARE(ArduCopterFirmwarePlugin().firmwareName(),   QStringLiteral("ArduPilot"));
    QCOMPARE(ArduPlaneFirmwarePlugin().firmwareName(),    QStringLiteral("ArduPilot"));
    QCOMPARE(ArduRoverFirmwarePlugin().firmwareName(),    QStringLiteral("ArduPilot"));
    QCOMPARE(ArduSubFirmwarePlugin().firmwareName(),      QStringLiteral("ArduPilot"));
}

void TestArdupilotModes::copterBasicModes()
{
    ArduCopterFirmwarePlugin p;
    QCOMPARE(p.decodeFlightMode(0, 0),  QStringLiteral("Stabilize"));
    QCOMPARE(p.decodeFlightMode(0, 2),  QStringLiteral("Alt Hold"));
    QCOMPARE(p.decodeFlightMode(0, 3),  QStringLiteral("Auto"));
    QCOMPARE(p.decodeFlightMode(0, 4),  QStringLiteral("Guided"));
    QCOMPARE(p.decodeFlightMode(0, 5),  QStringLiteral("Loiter"));
    QCOMPARE(p.decodeFlightMode(0, 6),  QStringLiteral("RTL"));
    QCOMPARE(p.decodeFlightMode(0, 9),  QStringLiteral("Land"));
    QCOMPARE(p.decodeFlightMode(0, 16), QStringLiteral("PosHold"));
    QCOMPARE(p.decodeFlightMode(0, 21), QStringLiteral("Smart RTL"));
}

void TestArdupilotModes::copterUnknownFallsBack()
{
    ArduCopterFirmwarePlugin p;
    QVERIFY(p.decodeFlightMode(0, 99).startsWith("Copter mode"));
}

void TestArdupilotModes::planeBasicModes()
{
    ArduPlaneFirmwarePlugin p;
    QCOMPARE(p.decodeFlightMode(0, 0),  QStringLiteral("Manual"));
    QCOMPARE(p.decodeFlightMode(0, 5),  QStringLiteral("FBWA"));
    QCOMPARE(p.decodeFlightMode(0, 10), QStringLiteral("Auto"));
    QCOMPARE(p.decodeFlightMode(0, 11), QStringLiteral("RTL"));
    QCOMPARE(p.decodeFlightMode(0, 15), QStringLiteral("Guided"));
    QCOMPARE(p.decodeFlightMode(0, 21), QStringLiteral("QRTL"));
}

void TestArdupilotModes::roverBasicModes()
{
    ArduRoverFirmwarePlugin p;
    QCOMPARE(p.decodeFlightMode(0, 0),  QStringLiteral("Manual"));
    QCOMPARE(p.decodeFlightMode(0, 4),  QStringLiteral("Hold"));
    QCOMPARE(p.decodeFlightMode(0, 10), QStringLiteral("Auto"));
    QCOMPARE(p.decodeFlightMode(0, 11), QStringLiteral("RTL"));
    QCOMPARE(p.decodeFlightMode(0, 15), QStringLiteral("Guided"));
}

void TestArdupilotModes::subBasicModes()
{
    ArduSubFirmwarePlugin p;
    QCOMPARE(p.decodeFlightMode(0, 0),  QStringLiteral("Stabilize"));
    QCOMPARE(p.decodeFlightMode(0, 2),  QStringLiteral("Alt Hold"));
    QCOMPARE(p.decodeFlightMode(0, 3),  QStringLiteral("Auto"));
    QCOMPARE(p.decodeFlightMode(0, 19), QStringLiteral("Manual"));
}

void TestArdupilotModes::baseClassFallback()
{
    ArduPilotFirmwarePlugin p;
    QCOMPARE(p.decodeFlightMode(0, 5), QStringLiteral("Mode 5"));
}

void TestArdupilotModes::homeIsMissionSeq0()
{
    ArduCopterFirmwarePlugin p;
    QVERIFY(p.missionFramePolicy().homeIsMissionSeq0);
    ArduPlaneFirmwarePlugin pp;
    QVERIFY(pp.missionFramePolicy().homeIsMissionSeq0);
}

void TestArdupilotModes::rcChannelsOverrideStaysDisabled()
{
    ArduCopterFirmwarePlugin p;
    QVERIFY(!p.manualControlPolicy().allowsRcChannelsOverride);
}

void TestArdupilotModes::supportedMissionCommandsRemainsEmpty()
{
    // SAFETY: real ArduPilot mission upload is not enabled in this phase.
    QVERIFY(ArduCopterFirmwarePlugin().supportedMissionCommands().isEmpty());
    QVERIFY(ArduPlaneFirmwarePlugin().supportedMissionCommands().isEmpty());
}

QTEST_MAIN(TestArdupilotModes)
#include "tst_ardupilot_modes.moc"
