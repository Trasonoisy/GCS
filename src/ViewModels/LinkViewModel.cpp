#include "LinkViewModel.h"

#include <QHostAddress>

#include "Comms/LinkManager.h"
#include "Comms/SerialLink.h"
#include "Comms/UdpLink.h"
#include "Protocol/MAVLinkProtocol.h"

namespace gcs::viewmodels {

LinkViewModel::LinkViewModel(gcs::comms::LinkManager* mgr, QObject* parent)
    : QObject(parent), m_mgr(mgr)
{
    if (!m_mgr) return;
    connect(m_mgr, &gcs::comms::LinkManager::udpStateChanged,
            this, &LinkViewModel::refreshState);
    connect(m_mgr, &gcs::comms::LinkManager::udpError,
            this, &LinkViewModel::onUdpError);
    connect(m_mgr, &gcs::comms::LinkManager::serialStateChanged,
            this, &LinkViewModel::refreshState);
    connect(m_mgr, &gcs::comms::LinkManager::serialError,
            this, &LinkViewModel::onSerialError);

    refreshSerialPorts();
}

void LinkViewModel::setListenPort(quint16 p)
{
    if (m_port == p) return;
    m_port = p;
    emit configChanged();
}

void LinkViewModel::setListenHost(const QString& h)
{
    if (m_host == h) return;
    m_host = h;
    emit configChanged();
}

bool LinkViewModel::connectToSitl()
{
    if (!m_mgr) return false;
    QHostAddress addr(m_host);
    if (addr.isNull()) addr = QHostAddress::AnyIPv4;
    auto* link = m_mgr->startUdpListener(addr, m_port);
    refreshState();
    if (!link) {
        emit errorMessage(m_lastError);
        return false;
    }
    m_bytesIn = 0;
    m_framesIn = 0;
    emit statsChanged();
    emit infoMessage(QStringLiteral("Listening on UDP %1:%2 - waiting for SITL heartbeat")
                     .arg(addr.toString()).arg(m_port));
    return true;
}

void LinkViewModel::disconnectFromSitl()
{
    if (!m_mgr) return;
    m_mgr->stopUdpListener();
    refreshState();
    emit infoMessage(QStringLiteral("Stopped UDP listener"));
}

// ---------- Phase 7: connection kind + serial -----------------------------

void LinkViewModel::setConnectionKind(const QString& kind)
{
    if (m_connectionKind == kind) return;
    m_connectionKind = kind;
    emit configChanged();
}

bool LinkViewModel::serialBackendAvailable() const
{
    return gcs::comms::SerialLink::isBackendAvailable();
}

QStringList LinkViewModel::serialBaudOptions() const
{
    QStringList out;
    for (const auto b : gcs::comms::SerialLink::commonBaudRates()) {
        out.append(QString::number(b));
    }
    return out;
}

void LinkViewModel::setSerialPort(const QString& p)
{
    if (m_serialPort == p) return;
    m_serialPort = p;
    emit configChanged();
}

void LinkViewModel::setSerialBaud(int baud)
{
    if (m_serialBaud == baud) return;
    m_serialBaud = baud;
    emit configChanged();
}

void LinkViewModel::refreshSerialPorts()
{
    m_serialPorts = gcs::comms::SerialLink::availablePortNames();
    if (m_serialPort.isEmpty() && !m_serialPorts.isEmpty()) {
        m_serialPort = m_serialPorts.first();
    }
    emit serialPortsRefreshed();
    emit configChanged();
}

bool LinkViewModel::connectSerial()
{
    if (!m_mgr) return false;
    if (!serialBackendAvailable()) {
        const QString msg = QStringLiteral(
            "Qt6::SerialPort module is not available in this build - see "
            "docs/hitl_real_uav_preparation.md.");
        m_lastError = msg;
        emit stateChanged();
        emit errorMessage(msg);
        return false;
    }
    auto* link = m_mgr->startSerialConnect(m_serialPort, m_serialBaud);
    refreshState();
    if (!link) {
        emit errorMessage(m_lastError);
        return false;
    }
    emit infoMessage(QStringLiteral(
        "Serial connected on %1 @ %2 - HARDWARE READ-ONLY. Commands are disabled.")
            .arg(m_serialPort).arg(m_serialBaud));
    return true;
}

void LinkViewModel::disconnectSerial()
{
    if (!m_mgr) return;
    m_mgr->stopSerialConnect();
    refreshState();
    emit infoMessage(QStringLiteral("Serial disconnected"));
}

// ---------- internals -----------------------------------------------------

void LinkViewModel::refreshState()
{
    if (!m_mgr) return;
    auto* udp    = m_mgr->udpLink();
    auto* serial = m_mgr->serialLink();
    m_connected       = udp && udp->isConnected();
    m_peerSeen        = udp && udp->peerObserved();
    m_peerHint        = m_peerSeen
                          ? QStringLiteral("%1:%2").arg(udp->peerAddress().toString()).arg(udp->peerPort())
                          : QString();
    m_serialConnected = serial && serial->isConnected();
    m_lastError       = m_mgr->lastError();
    emit stateChanged();
}

void LinkViewModel::onUdpError(const QString& msg)
{
    m_lastError = msg;
    emit stateChanged();
    emit errorMessage(msg);
}

void LinkViewModel::onSerialError(const QString& msg)
{
    m_lastError = msg;
    emit stateChanged();
    emit errorMessage(msg);
}

void LinkViewModel::onBytesIn(int n)
{
    m_bytesIn += n;
    emit statsChanged();
}

void LinkViewModel::onUnknownMsg(int, int, int)
{
    ++m_framesIn;
    emit statsChanged();
}

} // namespace gcs::viewmodels
