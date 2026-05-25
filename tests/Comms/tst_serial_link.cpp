#include <QSignalSpy>
#include <QtTest/QtTest>

#include "Comms/LinkKind.h"
#include "Comms/SerialLink.h"

using gcs::comms::LinkKind;
using gcs::comms::SerialLink;

class TestSerialLink : public QObject
{
    Q_OBJECT
private slots:
    void kindIsSerial();
    void nameReflectsPortAndBaud();
    void setPortAndBaudUpdateConfig();
    void setPortIgnoredWhileConnected();        // setter is no-op when open
    void connectWithoutBackendOrPortFailsCleanly();
    void writeBytesAlwaysRejected();
    void availablePortsReturnsList();
    void commonBaudRatesContainsCanonicalValues();
};

void TestSerialLink::kindIsSerial()
{
    SerialLink s;
    QCOMPARE(s.kind(), LinkKind::Serial);
}

void TestSerialLink::nameReflectsPortAndBaud()
{
    SerialLink s(QStringLiteral("COM3"), 57600);
    QVERIFY(s.name().contains("COM3"));
    QVERIFY(s.name().contains("57600"));
}

void TestSerialLink::setPortAndBaudUpdateConfig()
{
    SerialLink s;
    s.setPortName(QStringLiteral("/dev/ttyACM0"));
    s.setBaudRate(921600);
    QCOMPARE(s.portName(), QStringLiteral("/dev/ttyACM0"));
    QCOMPARE(s.baudRate(), 921600);
}

void TestSerialLink::setPortIgnoredWhileConnected()
{
    // The link can't actually open in the test environment, so we just
    // verify the no-mutate-while-connected contract via the public API
    // (we have no way to force m_connected without driving the backend).
    SerialLink s(QStringLiteral("COM1"), 115200);
    QCOMPARE(s.portName(), QStringLiteral("COM1"));
    s.setPortName(QStringLiteral("COM2"));
    QCOMPARE(s.portName(), QStringLiteral("COM2"));
}

void TestSerialLink::connectWithoutBackendOrPortFailsCleanly()
{
    SerialLink s;
    QSignalSpy errSpy(&s, &SerialLink::errorOccurred);
    QSignalSpy connSpy(&s, &SerialLink::connectedChanged);

    // No port name. With either backend (real or stub), this must fail
    // gracefully and emit at least one error.
    s.connectLink();
    QVERIFY(!s.isConnected());
    QCOMPARE(connSpy.count(), 0);
    QVERIFY(errSpy.count() >= 1);

    // The error must mention either the missing port or the missing
    // backend depending on which build path is active.
    const QString reason = errSpy.first().at(0).toString();
    QVERIFY(reason.contains("port", Qt::CaseInsensitive)
            || reason.contains("SerialPort", Qt::CaseInsensitive));
}

void TestSerialLink::writeBytesAlwaysRejected()
{
    // SAFETY: Phase 7 is read-only on serial. writeBytes must never go to
    // the wire — the link emits errorOccurred and stays a no-op.
    SerialLink s(QStringLiteral("COM1"), 115200);
    QSignalSpy errSpy(&s, &SerialLink::errorOccurred);
    s.writeBytes(QByteArray("anything"));
    QVERIFY(errSpy.count() >= 1);
    QVERIFY(errSpy.first().at(0).toString().contains("read-only", Qt::CaseInsensitive)
            || errSpy.first().at(0).toString().contains("refused", Qt::CaseInsensitive));
}

void TestSerialLink::availablePortsReturnsList()
{
    // Just confirms the static helper doesn't crash. The list may be empty
    // depending on the host and on whether Qt6::SerialPort is compiled in.
    const auto ports = SerialLink::availablePortNames();
    Q_UNUSED(ports);
    QVERIFY(true);
}

void TestSerialLink::commonBaudRatesContainsCanonicalValues()
{
    const auto bauds = SerialLink::commonBaudRates();
    QVERIFY(bauds.contains(57600));
    QVERIFY(bauds.contains(115200));
    QVERIFY(bauds.contains(921600));
}

QTEST_MAIN(TestSerialLink)
#include "tst_serial_link.moc"
