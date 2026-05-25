#include "UdpLink.h"

#include <QUdpSocket>

namespace gcs::comms {

UdpLink::UdpLink(quint16 listenPort, QObject* parent)
    : LinkInterface(parent),
      m_socket(std::make_unique<QUdpSocket>(this)),
      m_port(listenPort)
{
    connect(m_socket.get(), &QUdpSocket::readyRead,
            this, &UdpLink::onReadyRead);
}

UdpLink::~UdpLink() = default;

void UdpLink::setListenAddress(QHostAddress addr)
{
    if (m_connected) return; // ignore while open — caller must disconnect first
    m_addr = std::move(addr);
}

void UdpLink::setListenPort(quint16 port)
{
    if (m_connected) return;
    m_port = port;
}

QString UdpLink::name() const
{
    return QStringLiteral("UDP %1:%2")
        .arg(m_addr.toString()).arg(m_port);
}

void UdpLink::connectLink()
{
    if (m_connected) return;

    const auto mode = QUdpSocket::ShareAddress
                    | QUdpSocket::ReuseAddressHint;
    if (!m_socket->bind(m_addr, m_port, mode)) {
        emit errorOccurred(QStringLiteral("UdpLink bind %1:%2 failed: %3")
                           .arg(m_addr.toString())
                           .arg(m_port)
                           .arg(m_socket->errorString()));
        return;
    }
    m_connected = true;
    emit connectedChanged(true);
}

void UdpLink::disconnectLink()
{
    if (!m_connected) return;
    m_socket->close();
    m_connected    = false;
    m_peerObserved = false;
    emit connectedChanged(false);
}

void UdpLink::writeBytes(const QByteArray& bytes)
{
    // SAFETY: Phase 3 does not initiate outbound MAVLink. The path exists
    // for future command-queue code (Phase 4+) and is harmless until then
    // because no caller in this build constructs MAVLink frames.
    if (!m_connected) {
        emit errorOccurred(QStringLiteral("UdpLink not connected"));
        return;
    }
    if (!m_peerObserved) {
        emit errorOccurred(QStringLiteral(
            "UdpLink has no observed peer yet — refusing to write"));
        return;
    }
    m_socket->writeDatagram(bytes, m_peerAddress, m_peerPort);
}

void UdpLink::onReadyRead()
{
    while (m_socket->hasPendingDatagrams()) {
        QByteArray buf;
        buf.resize(int(m_socket->pendingDatagramSize()));
        QHostAddress sender;
        quint16 senderPort = 0;
        const qint64 n = m_socket->readDatagram(buf.data(), buf.size(),
                                                &sender, &senderPort);
        if (n <= 0) continue;
        if (n != buf.size()) buf.resize(int(n));

        m_peerObserved = true;
        m_peerAddress  = sender;
        m_peerPort     = senderPort;

        emit bytesReceived(this, buf);
    }
}

} // namespace gcs::comms
