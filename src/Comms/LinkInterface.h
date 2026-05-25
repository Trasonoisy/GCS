#pragma once

#include <QByteArray>
#include <QObject>
#include <QString>

#include "LinkKind.h"

namespace gcs::comms {

// Transport-layer abstraction for one bidirectional byte stream.
//
// Concrete links (MockLink, UdpLink, TcpLink, SerialLink, TlogReplayLink) share
// this contract. Higher layers (MAVLinkProtocol, MessageRouter) consume bytes
// from this interface and never know which transport produced them.
//
// Implementations MUST NOT decode MAVLink, mutate VehicleState, or touch UI.
class LinkInterface : public QObject
{
    Q_OBJECT
public:
    explicit LinkInterface(QObject* parent = nullptr) : QObject(parent) {}
    ~LinkInterface() override = default;

    virtual QString  name() const = 0;
    virtual bool     isConnected() const = 0;

    // Phase 7: classifies the transport so SafetyGate can refuse risky
    // operations on a real-hardware link even when the autopilot identity
    // is indistinguishable from SITL. Default is Unknown — concrete links
    // override.
    virtual LinkKind kind() const { return LinkKind::Unknown; }

public slots:
    virtual void connectLink() = 0;
    virtual void disconnectLink() = 0;
    virtual void writeBytes(const QByteArray& bytes) = 0;

signals:
    void bytesReceived(gcs::comms::LinkInterface* link, const QByteArray& bytes);
    void connectedChanged(bool connected);
    void errorOccurred(const QString& message);
};

} // namespace gcs::comms
