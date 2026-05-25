#include <QSignalSpy>
#include <QUdpSocket>
#include <QtTest/QtTest>

#include "Comms/UdpLink.h"

using gcs::comms::UdpLink;

class TestUdpLink : public QObject
{
    Q_OBJECT
private slots:
    void bindsAndReceivesDatagram();
};

void TestUdpLink::bindsAndReceivesDatagram()
{
    // Use an ephemeral port to avoid clashing with a real PX4 SITL on 14550.
    UdpLink link(0);
    link.setListenAddress(QHostAddress::LocalHost);
    QSignalSpy connSpy(&link, &UdpLink::connectedChanged);
    QSignalSpy rxSpy(&link, &UdpLink::bytesReceived);
    link.connectLink();
    QVERIFY(link.isConnected());
    QCOMPARE(connSpy.count(), 1);
    const quint16 port = link.listenPort();
    // After bind on port 0 the OS assigns an ephemeral; UdpLink's m_port
    // still says 0, so query the underlying socket via a sender approach:
    // we send TO LocalHost:<actual> by asking the socket on the receive side.
    // Workaround: ask the OS for the link's local port via a sender probe.

    // Send a datagram from a peer socket.
    QUdpSocket sender;
    QVERIFY(sender.bind(QHostAddress::LocalHost, 0));
    // We need to know the receiver's actual port. UdpLink doesn't expose the
    // OS-assigned port (m_port is 0 here), so re-bind to a known free port
    // by trying a small range until one binds. Easiest: pick a high port we
    // assume is free in CI; if it's busy, the test re-tries.
    link.disconnectLink();

    quint16 chosen = 0;
    for (quint16 p = 41555; p < 41600; ++p) {
        link.setListenPort(p);
        link.connectLink();
        if (link.isConnected()) { chosen = p; break; }
        link.disconnectLink();
    }
    QVERIFY(chosen != 0);

    const QByteArray payload("hello", 5);
    sender.writeDatagram(payload, QHostAddress::LocalHost, chosen);

    QVERIFY(rxSpy.wait(2000));
    QVERIFY(link.peerObserved());
    const auto args = rxSpy.first();
    QCOMPARE(args.at(1).toByteArray(), payload);

    link.disconnectLink();
    QVERIFY(!link.isConnected());
}

QTEST_MAIN(TestUdpLink)
#include "tst_udp_link.moc"
