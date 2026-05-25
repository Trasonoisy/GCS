#include <QSignalSpy>
#include <QtTest/QtTest>

#include "Manual/MockJoystickBackend.h"

using gcs::manual::MockJoystickBackend;

class TestMockJoystick : public QObject
{
    Q_OBJECT
private slots:
    void connectDisconnect();
    void rawSettersAreProcessed();
    void disconnectRecentersAxes();
};

void TestMockJoystick::connectDisconnect()
{
    MockJoystickBackend j;
    QVERIFY(!j.isConnected());

    QSignalSpy spy(&j, &gcs::manual::JoystickBackend::stateChanged);
    j.setConnected(true, QStringLiteral("USB Stick"));
    QVERIFY(j.isConnected());
    QCOMPARE(j.state().name, QStringLiteral("USB Stick"));
    QCOMPARE(spy.count(), 1);

    // Idempotent only when the (connected, name) tuple matches exactly.
    j.setConnected(true, QStringLiteral("USB Stick"));
    QCOMPARE(spy.count(), 1);

    j.setConnected(false);
    QVERIFY(!j.isConnected());
    QCOMPARE(spy.count(), 2);
}

void TestMockJoystick::rawSettersAreProcessed()
{
    MockJoystickBackend j;
    j.setConnected(true);
    j.pitchConfig().deadzone = 0.0;
    j.pitchConfig().expo     = 0.0;

    j.setRawPitch(0.5);
    QCOMPARE(j.rawPitch(), 0.5);
    QCOMPARE(j.state().pitch, 0.5);

    j.pitchConfig().inverted = true;
    j.setRawPitch(0.3);
    QCOMPARE(j.state().pitch, -0.3);
}

void TestMockJoystick::disconnectRecentersAxes()
{
    MockJoystickBackend j;
    j.setConnected(true);
    j.setRawPitch(0.9);
    j.setRawRoll(-0.4);
    j.setButtons(0xF00F);

    j.setConnected(false);
    QCOMPARE(j.rawPitch(), 0.0);
    QCOMPARE(j.rawRoll(),  0.0);
    QCOMPARE(j.state().buttons, quint16(0));
}

QTEST_MAIN(TestMockJoystick)
#include "tst_mock_joystick.moc"
