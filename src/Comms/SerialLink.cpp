#include "SerialLink.h"

#ifdef GCS_HAS_SERIAL_PORT
#  include <QSerialPort>
#  include <QSerialPortInfo>
#endif

namespace gcs::comms {

SerialLink::SerialLink(QString portName, qint32 baudRate, QObject* parent)
    : LinkInterface(parent),
      m_portName(std::move(portName)),
      m_baudRate(baudRate)
{
#ifdef GCS_HAS_SERIAL_PORT
    m_serial = std::make_unique<QSerialPort>(this);
    connect(m_serial.get(), &QSerialPort::readyRead,
            this, &SerialLink::onReadyRead);
#endif
}

SerialLink::~SerialLink() = default;

bool SerialLink::isBackendAvailable()
{
#ifdef GCS_HAS_SERIAL_PORT
    return true;
#else
    return false;
#endif
}

QList<QString> SerialLink::availablePortNames()
{
    QList<QString> out;
#ifdef GCS_HAS_SERIAL_PORT
    for (const auto& info : QSerialPortInfo::availablePorts()) {
        out.append(info.portName());
    }
#endif
    return out;
}

QList<qint32> SerialLink::commonBaudRates()
{
    return { 57600, 115200, 230400, 460800, 921600 };
}

void SerialLink::setPortName(const QString& portName)
{
    if (m_connected) return; // ignore while open — caller must disconnect first
    m_portName = portName;
}

void SerialLink::setBaudRate(qint32 baudRate)
{
    if (m_connected) return;
    if (baudRate > 0) m_baudRate = baudRate;
}

QString SerialLink::name() const
{
    return QStringLiteral("Serial %1 @ %2")
        .arg(m_portName.isEmpty() ? QStringLiteral("(none)") : m_portName)
        .arg(m_baudRate);
}

void SerialLink::connectLink()
{
    if (m_connected) return;

#ifdef GCS_HAS_SERIAL_PORT
    if (m_portName.isEmpty()) {
        emit errorOccurred(QStringLiteral(
            "SerialLink: no port name selected"));
        return;
    }
    m_serial->setPortName(m_portName);
    m_serial->setBaudRate(m_baudRate);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serial->open(QIODevice::ReadOnly)) {
        emit errorOccurred(QStringLiteral(
            "SerialLink open failed: %1 (%2)")
            .arg(m_serial->errorString(), m_portName));
        return;
    }
    m_connected = true;
    emit connectedChanged(true);
#else
    // Backend stub: no SerialPort module compiled in. Tell the operator
    // exactly how to enable it; do not silently pretend to connect.
    emit errorOccurred(QStringLiteral(
        "SerialLink unavailable: this build was compiled without "
        "Qt6::SerialPort. Install the Qt SerialPort module and rebuild "
        "(see docs/hitl_real_uav_preparation.md)."));
#endif
}

void SerialLink::disconnectLink()
{
    if (!m_connected) return;
#ifdef GCS_HAS_SERIAL_PORT
    if (m_serial && m_serial->isOpen()) m_serial->close();
#endif
    m_connected = false;
    emit connectedChanged(false);
}

void SerialLink::writeBytes(const QByteArray& /*bytes*/)
{
    // SAFETY: Phase 7 is read-only over serial. We refuse outbound bytes
    // even though the LinkInterface contract supports them. Real outbound
    // commands wait for: (1) Qt6::SerialPort enabled, (2) MavCommandQueue
    // implemented, (3) SafetyGate explicitly allowing the action. None of
    // those are true today.
    emit errorOccurred(QStringLiteral(
        "SerialLink: outbound bytes refused — Phase 7 is read-only "
        "(no real-hardware command path is enabled in this build)"));
}

void SerialLink::onReadyRead()
{
#ifdef GCS_HAS_SERIAL_PORT
    if (!m_serial) return;
    const QByteArray chunk = m_serial->readAll();
    if (chunk.isEmpty()) return;
    emit bytesReceived(this, chunk);
#endif
}

} // namespace gcs::comms
