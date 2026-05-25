#pragma once

#include <QHostAddress>
#include <QList>
#include <QObject>
#include <QString>

namespace gcs::protocol { class MAVLinkProtocol; }

namespace gcs::comms {

class LinkInterface;
class UdpLink;
class SerialLink;

// Owns a collection of LinkInterfaces and attaches them to MAVLinkProtocol.
//
// In Phase 3 we manage exactly two link kinds:
//   - the MockLink that already drives the simulated vehicle path
//   - a single UdpLink for PX4 SITL telemetry
//
// SAFETY: This is plumbing only. No command initiation happens here.
class LinkManager : public QObject
{
    Q_OBJECT
public:
    explicit LinkManager(gcs::protocol::MAVLinkProtocol* protocol,
                         QObject* parent = nullptr);
    ~LinkManager() override;

    void addLink(LinkInterface* link); // takes Qt parent ownership
    const QList<LinkInterface*>& links() const { return m_links; }

    // UDP convenience: returns nullptr if already connected on this port.
    UdpLink* startUdpListener(QHostAddress addr, quint16 port,
                              QString* errorOut = nullptr);
    bool     stopUdpListener();
    UdpLink* udpLink() const { return m_udp; }

    // Serial convenience (Phase 7): opens a SerialLink for HITL / real
    // hardware telemetry monitoring. Returns nullptr if already connected
    // or the SerialPort backend is unavailable (errorOut filled in).
    SerialLink* startSerialConnect(const QString& portName, qint32 baudRate,
                                   QString* errorOut = nullptr);
    bool        stopSerialConnect();
    SerialLink* serialLink() const { return m_serial; }

    QString lastError() const { return m_lastError; }

signals:
    void linkAdded(LinkInterface* link);
    void udpStateChanged();           // listener created / connected / removed
    void udpError(const QString& msg);
    void serialStateChanged();        // serial connected / disconnected
    void serialError(const QString& msg);

private:
    gcs::protocol::MAVLinkProtocol* m_protocol;
    QList<LinkInterface*>           m_links;
    UdpLink*                        m_udp    = nullptr;
    SerialLink*                     m_serial = nullptr;
    QString                         m_lastError;
};

} // namespace gcs::comms
