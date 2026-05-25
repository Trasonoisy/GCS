#include "MockLink.h"

namespace gcs::comms {

MockLink::MockLink(QString name, QObject* parent)
    : LinkInterface(parent), m_name(std::move(name))
{
}

void MockLink::connectLink()
{
    if (m_connected) {
        return;
    }
    m_connected = true;
    emit connectedChanged(true);
}

void MockLink::disconnectLink()
{
    if (!m_connected) {
        return;
    }
    m_connected = false;
    emit connectedChanged(false);
}

void MockLink::writeBytes(const QByteArray& bytes)
{
    if (!m_connected) {
        emit errorOccurred(QStringLiteral("MockLink not connected"));
        return;
    }
    m_lastWritten = bytes;
    ++m_writeCount;
}

void MockLink::injectIncomingBytes(const QByteArray& bytes)
{
    if (!m_connected) {
        return;
    }
    emit bytesReceived(this, bytes);
}

void MockLink::clearWriteHistory()
{
    m_lastWritten.clear();
    m_writeCount = 0;
}

} // namespace gcs::comms
