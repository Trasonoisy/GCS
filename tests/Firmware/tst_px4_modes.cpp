#include <QtTest/QtTest>

#include "Firmware/PX4FirmwarePlugin.h"

using gcs::firmware::PX4FirmwarePlugin;

class TestPx4Modes : public QObject
{
    Q_OBJECT
private slots:
    void manualMode();
    void posctlMode();
    void autoMission();
    void autoRtl();
    void unknownMainModeFallsBack();
    void noCustomFlagFallsBackToManual();
};

static uint32_t packMode(uint8_t main, uint8_t sub = 0)
{
    return (uint32_t(main) << 16) | (uint32_t(sub) << 24);
}

void TestPx4Modes::manualMode()
{
    PX4FirmwarePlugin p;
    QCOMPARE(p.decodeFlightMode(0x01, packMode(1)), QStringLiteral("Manual"));
}

void TestPx4Modes::posctlMode()
{
    PX4FirmwarePlugin p;
    QCOMPARE(p.decodeFlightMode(0x01, packMode(3)), QStringLiteral("Position"));
}

void TestPx4Modes::autoMission()
{
    PX4FirmwarePlugin p;
    QCOMPARE(p.decodeFlightMode(0x01, packMode(4, 4)), QStringLiteral("Mission"));
}

void TestPx4Modes::autoRtl()
{
    PX4FirmwarePlugin p;
    QCOMPARE(p.decodeFlightMode(0x01, packMode(4, 5)), QStringLiteral("Return"));
}

void TestPx4Modes::unknownMainModeFallsBack()
{
    PX4FirmwarePlugin p;
    const QString name = p.decodeFlightMode(0x01, packMode(99));
    QVERIFY(name.startsWith("PX4.main="));
}

void TestPx4Modes::noCustomFlagFallsBackToManual()
{
    PX4FirmwarePlugin p;
    QCOMPARE(p.decodeFlightMode(0x00, packMode(4)), QStringLiteral("MANUAL"));
}

QTEST_MAIN(TestPx4Modes)
#include "tst_px4_modes.moc"
