#include <QtTest/QtTest>

#include "Firmware/ArduPilotFirmwarePlugin.h"
#include "Firmware/PX4FirmwarePlugin.h"

using gcs::firmware::ArduPilotFirmwarePlugin;
using gcs::firmware::PX4FirmwarePlugin;

class TestFirmwarePlugins : public QObject
{
    Q_OBJECT
private slots:
    void px4_defaults();
    void apm_defaults();
    void modeEncodeRefusedInPhase1();
};

void TestFirmwarePlugins::px4_defaults()
{
    PX4FirmwarePlugin p;
    QCOMPARE(p.firmwareName(), QStringLiteral("PX4"));
    QVERIFY(p.missionFramePolicy().useMissionItemInt);
    QVERIFY(!p.missionFramePolicy().homeIsMissionSeq0);
    QVERIFY(!p.manualControlPolicy().allowsRcChannelsOverride);
    QVERIFY(p.supportedMissionCommands().isEmpty()); // fail-closed default
}

void TestFirmwarePlugins::apm_defaults()
{
    ArduPilotFirmwarePlugin p;
    QCOMPARE(p.firmwareName(), QStringLiteral("ArduPilot"));
    QVERIFY(p.missionFramePolicy().useMissionItemInt);
    QVERIFY(p.missionFramePolicy().homeIsMissionSeq0);
    QVERIFY(!p.manualControlPolicy().allowsRcChannelsOverride);
}

void TestFirmwarePlugins::modeEncodeRefusedInPhase1()
{
    // SAFETY: encodeFlightMode must refuse in Phase 1 because no SafetyGate
    // exists yet to gate mode changes against a real vehicle.
    PX4FirmwarePlugin px4;
    ArduPilotFirmwarePlugin apm;

    uint8_t baseMode = 0;
    uint32_t customMode = 0;
    QVERIFY(!px4.encodeFlightMode(QStringLiteral("AUTO"), baseMode, customMode));
    QVERIFY(!apm.encodeFlightMode(QStringLiteral("AUTO"), baseMode, customMode));
}

QTEST_MAIN(TestFirmwarePlugins)
#include "tst_firmware_plugins.moc"
