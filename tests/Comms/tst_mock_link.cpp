#include <QSignalSpy>
#include <QtTest/QtTest>

#include "Comms/MockLink.h"

using gcs::comms::MockLink;

class TestMockLink : public QObject
{
    Q_OBJECT
private slots:
    void connectDisconnect();
    void writeRecordedOnlyWhenConnected();
    void incomingBytesReachListener();
};

void TestMockLink::connectDisconnect()
{
    MockLink link;
    QVERIFY(!link.isConnected());

    QSignalSpy spy(&link, &MockLink::connectedChanged);
    link.connectLink();
    QVERIFY(link.isConnected());
    QCOMPARE(spy.count(), 1);

    link.connectLink(); // idempotent
    QCOMPARE(spy.count(), 1);

    link.disconnectLink();
    QVERIFY(!link.isConnected());
    QCOMPARE(spy.count(), 2);
}

void TestMockLink::writeRecordedOnlyWhenConnected()
{
    MockLink link;
    QSignalSpy errorSpy(&link, &MockLink::errorOccurred);

    link.writeBytes(QByteArray("nope"));
    QCOMPARE(link.writeCount(), 0);
    QCOMPARE(errorSpy.count(), 1);

    link.connectLink();
    link.writeBytes(QByteArray("hello"));
    QCOMPARE(link.writeCount(), 1);
    QCOMPARE(link.lastWrittenBytes(), QByteArray("hello"));
}

void TestMockLink::incomingBytesReachListener()
{
    MockLink link;
    link.connectLink();

    QSignalSpy spy(&link, &MockLink::bytesReceived);
    link.injectIncomingBytes(QByteArray("inbound"));

    QCOMPARE(spy.count(), 1);
    const auto args = spy.takeFirst();
    QCOMPARE(args.at(1).toByteArray(), QByteArray("inbound"));
}

QTEST_MAIN(TestMockLink)
#include "tst_mock_link.moc"
