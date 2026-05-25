#include <QtTest/QtTest>
#include <cmath>

#include "Manual/AxisConfig.h"
#include "Manual/AxisMapper.h"

using namespace gcs::manual;

class TestAxisMapper : public QObject
{
    Q_OBJECT
private slots:
    void deadzoneZeroesCenter();
    void deadzoneRescalesOutsideZone();
    void expoCompressesNearZero();
    void inversionFlipsSign();
    void packAxisClampsAndRounds();
    void packThrottleMapsRangeCorrectly();
    void nanInputsAreSafe();
};

void TestAxisMapper::deadzoneZeroesCenter()
{
    AxisConfig cfg; cfg.deadzone = 0.10; cfg.expo = 0.0;
    QCOMPARE(axis::processAxis(0.0,  cfg), 0.0);
    QCOMPARE(axis::processAxis(0.05, cfg), 0.0);
    QCOMPARE(axis::processAxis(-0.05, cfg), 0.0);
    // Just outside the deadzone -> small but non-zero.
    QVERIFY(axis::processAxis(0.11, cfg) > 0.0);
}

void TestAxisMapper::deadzoneRescalesOutsideZone()
{
    AxisConfig cfg; cfg.deadzone = 0.10; cfg.expo = 0.0;
    // Full deflection should still hit +/-1.
    QCOMPARE(axis::processAxis(1.0,  cfg),  1.0);
    QCOMPARE(axis::processAxis(-1.0, cfg), -1.0);
}

void TestAxisMapper::expoCompressesNearZero()
{
    AxisConfig cfg; cfg.deadzone = 0.0; cfg.expo = 1.0; // pure cubic
    // y = x^3 — magnitude smaller than the input near zero.
    QVERIFY(std::fabs(axis::processAxis(0.5, cfg)) < 0.5);
    QCOMPARE(axis::processAxis(1.0, cfg),  1.0);
    QCOMPARE(axis::processAxis(-1.0, cfg), -1.0);
}

void TestAxisMapper::inversionFlipsSign()
{
    AxisConfig cfg; cfg.deadzone = 0.0; cfg.expo = 0.0; cfg.inverted = true;
    QCOMPARE(axis::processAxis(0.42, cfg), -0.42);
    QCOMPARE(axis::processAxis(-0.5, cfg),  0.5);
}

void TestAxisMapper::packAxisClampsAndRounds()
{
    QCOMPARE(int(axis::packAxis(0.0)), 0);
    QCOMPARE(int(axis::packAxis(1.0)), 1000);
    QCOMPARE(int(axis::packAxis(-1.0)), -1000);
    QCOMPARE(int(axis::packAxis(1.5)), 1000);   // clamped
    QCOMPARE(int(axis::packAxis(-1.5)), -1000); // clamped
}

void TestAxisMapper::packThrottleMapsRangeCorrectly()
{
    // -1 -> 0, 0 -> 500, +1 -> 1000
    QCOMPARE(int(axis::packThrottle(-1.0)), 0);
    QCOMPARE(int(axis::packThrottle( 0.0)), 500);
    QCOMPARE(int(axis::packThrottle( 1.0)), 1000);
    QCOMPARE(int(axis::packThrottle( 2.0)), 1000); // clamped high
    QCOMPARE(int(axis::packThrottle(-2.0)), 0);    // clamped low
}

void TestAxisMapper::nanInputsAreSafe()
{
    AxisConfig cfg;
    QCOMPARE(axis::processAxis(std::nan(""), cfg), 0.0);
    QCOMPARE(int(axis::packAxis(std::nan(""))), 0);
    QCOMPARE(int(axis::packThrottle(std::nan(""))), 0);
}

QTEST_MAIN(TestAxisMapper)
#include "tst_axis_mapper.moc"
