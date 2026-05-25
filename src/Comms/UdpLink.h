#pragma once

#include <QHostAddress>
#include <memory>

#include "LinkInterface.h"

class QUdpSocket;

namespace gcs::comms {

// LinkInterface implementation backed by QUdpSocket.
//
// Listens on a local port (default 14550 for PX4 SITL). All received
// datagrams are emitted via bytesReceived. Phase 3 does not send outbound
// MAVLink — writeBytes is implemented but only used by future phases (and
// requires a peer to have been observed first).
//
// SAFETY: Phase 3 is telemetry-only. writeBytes is wired but no caller in
// this build constructs MAVLink packets; the path exists only so future
// command-queue code (Phase 4+) can target a peer.
class UdpLink : public LinkInterface
{
    Q_OBJECT
public:
    explicit UdpLink(quint16 listenPort = 14550,
                     QObject* parent = nullptr);
    ~UdpLink() override;

    void setListenAddress(QHostAddress addr);
    void setListenPort(quint16 port);

    quint16 listenPort()    const { return m_port; }
    QHostAddress listenAddress() const { return m_addr; }

    QString  name() const override;
    bool     isConnected() const override { return m_connected; }
    LinkKind kind() const override { return LinkKind::Udp; }

    // True once at least one datagram has been received; populated peer
    // info is available via peerAddress()/peerPort().
    bool peerObserved() const { return m_peerObserved; }
    QHostAddress peerAddress() const { return m_peerAddress; }
    quint16 peerPort() const { return m_peerPort; }

public slots:
    void connectLink() override;
    void disconnectLink() override;
    void writeBytes(const QByteArray& bytes) override;

private slots:
    void onReadyRead();

private:
    std::unique_ptr<QUdpSocket> m_socket;
    QHostAddress m_addr = QHostAddress::AnyIPv4;
    quint16 m_port      = 14550;
    bool    m_connected = false;
    bool    m_peerObserved = false;
    QHostAddress m_peerAddress;
    quint16 m_peerPort = 0;
};

} // namespace gcs::comms
