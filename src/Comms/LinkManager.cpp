#include "LinkManager.h"

#include "LinkInterface.h"
#include "Protocol/MAVLinkProtocol.h"
#include "SerialLink.h"
#include "UdpLink.h"

namespace gcs::comms {

LinkManager::LinkManager(gcs::protocol::MAVLinkProtocol* protocol, QObject* parent)
    : QObject(parent), m_protocol(protocol)
{
}

LinkManager::~LinkManager() = default;

void LinkManager::addLink(LinkInterface* link)
{
    if (!link || m_links.contains(link)) return;
    link->setParent(this);
    m_links.append(link);
    if (m_protocol) m_protocol->attachLink(link);
    emit linkAdded(link);
}

UdpLink* LinkManager::startUdpListener(QHostAddress addr, quint16 port,
                                       QString* errorOut)
{
    if (m_udp && m_udp->isConnected()) {
        m_lastError = QStringLiteral("UDP listener already running on %1:%2")
                      .arg(m_udp->listenAddress().toString())
                      .arg(m_udp->listenPort());
        if (errorOut) *errorOut = m_lastError;
        emit udpError(m_lastError);
        return nullptr;
    }

    if (!m_udp) {
        m_udp = new UdpLink(port, this);
        connect(m_udp, &UdpLink::errorOccurred,
                this, [this](const QString& msg) {
                    m_lastError = msg;
                    emit udpError(msg);
                });
        connect(m_udp, &UdpLink::connectedChanged,
                this, &LinkManager::udpStateChanged);
        addLink(m_udp);
    }

    // Always (re)apply addr/port — UdpLink's constructor takes only a port,
    // and on a re-listen the caller may have changed either field.
    m_udp->setListenAddress(addr);
    m_udp->setListenPort(port);
    m_udp->connectLink();

    if (!m_udp->isConnected()) {
        // bind failed — m_lastError was set via the error slot
        if (errorOut) *errorOut = m_lastError;
        emit udpStateChanged();
        return nullptr;
    }
    m_lastError.clear();
    emit udpStateChanged();
    return m_udp;
}

bool LinkManager::stopUdpListener()
{
    if (!m_udp || !m_udp->isConnected()) return false;
    m_udp->disconnectLink();
    emit udpStateChanged();
    return true;
}

// ---------- Serial (Phase 7) -------------------------------------------------

SerialLink* LinkManager::startSerialConnect(const QString& portName,
                                            qint32 baudRate,
                                            QString* errorOut)
{
    if (m_serial && m_serial->isConnected()) {
        m_lastError = QStringLiteral(
            "Serial link already connected on %1").arg(m_serial->portName());
        if (errorOut) *errorOut = m_lastError;
        emit serialError(m_lastError);
        return nullptr;
    }
    if (!SerialLink::isBackendAvailable()) {
        m_lastError = QStringLiteral(
            "Qt6::SerialPort module is not available in this build. "
            "Install it and reconfigure with CMake to enable hardware "
            "serial connections — see docs/hitl_real_uav_preparation.md.");
        if (errorOut) *errorOut = m_lastError;
        emit serialError(m_lastError);
        return nullptr;
    }

    if (!m_serial) {
        m_serial = new SerialLink(portName, baudRate, this);
        connect(m_serial, &SerialLink::errorOccurred,
                this, [this](const QString& msg) {
                    m_lastError = msg;
                    emit serialError(msg);
                });
        connect(m_serial, &SerialLink::connectedChanged,
                this, &LinkManager::serialStateChanged);
        addLink(m_serial);
    }

    m_serial->setPortName(portName);
    m_serial->setBaudRate(baudRate);
    m_serial->connectLink();

    if (!m_serial->isConnected()) {
        if (errorOut) *errorOut = m_lastError;
        emit serialStateChanged();
        return nullptr;
    }
    m_lastError.clear();
    emit serialStateChanged();
    return m_serial;
}

bool LinkManager::stopSerialConnect()
{
    if (!m_serial || !m_serial->isConnected()) return false;
    m_serial->disconnectLink();
    emit serialStateChanged();
    return true;
}

} // namespace gcs::comms
