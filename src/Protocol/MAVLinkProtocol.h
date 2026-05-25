#pragma once

#include <QByteArray>
#include <QObject>

#include "Comms/LinkInterface.h"
#include "MavlinkFrame.h"

namespace gcs::protocol {

// Drives MavlinkV2Parser over bytes arriving from one or more LinkInterfaces
// and emits typed signals for each decoded message. Higher layers
// (MAVLinkMessageRouter) subscribe to these signals.
//
// SAFETY: This object decodes inbound bytes only. It NEVER constructs or
// sends MAVLink commands. Outbound MAVLink waits for SafetyGate + a real
// command queue (Phase 4+).
class MAVLinkProtocol : public QObject
{
    Q_OBJECT
public:
    explicit MAVLinkProtocol(QObject* parent = nullptr);
    ~MAVLinkProtocol() override = default;

    void attachLink(gcs::comms::LinkInterface* link);
    void detachLink(gcs::comms::LinkInterface* link);

    int decodedMessageCount() const { return m_decodedCount; }
    int totalFrames() const { return m_parser.totalFrames(); }

    // Phase 7: identifies the LinkInterface that produced the currently-
    // dispatching message. Valid ONLY inside the receiver of one of the
    // typed signals (heartbeatReceived / attitudeReceived / …). The router
    // uses this to stamp newly-spawned vehicles with the source link's
    // LinkKind so SafetyGate can distinguish hardware vs SITL.
    gcs::comms::LinkInterface* currentSourceLink() const { return m_currentSourceLink; }

    // Test hook — feed bytes directly without going through a link.
    void feedBytesForTest(const QByteArray& bytes);

signals:
    void rawBytesIn(int byteCount);
    void heartbeatReceived(int sysid, int compid, const gcs::protocol::msg::Heartbeat& hb);
    void sysStatusReceived(int sysid, int compid, const gcs::protocol::msg::SysStatus& s);
    void gpsRawIntReceived(int sysid, int compid, const gcs::protocol::msg::GpsRawInt& g);
    void attitudeReceived(int sysid, int compid, const gcs::protocol::msg::Attitude& a);
    void globalPositionIntReceived(int sysid, int compid,
                                   const gcs::protocol::msg::GlobalPositionInt& p);
    void vfrHudReceived(int sysid, int compid, const gcs::protocol::msg::VfrHud& v);
    void batteryStatusReceived(int sysid, int compid,
                               const gcs::protocol::msg::BatteryStatus& b);
    void statustextReceived(int sysid, int compid, const gcs::protocol::msg::Statustext& t);

    // Phase 8: mission-protocol inbound (vehicle -> GCS) signals.
    void missionCountReceived       (int sysid, int compid, const gcs::protocol::msg::MissionCount& m);
    void missionRequestIntReceived  (int sysid, int compid, const gcs::protocol::msg::MissionRequestInt& m);
    void missionItemIntReceived     (int sysid, int compid, const gcs::protocol::msg::MissionItemInt& m);
    void missionAckReceived         (int sysid, int compid, const gcs::protocol::msg::MissionAck& m);
    void missionRequestListReceived (int sysid, int compid, const gcs::protocol::msg::MissionRequestList& m);

    void unknownMessageReceived(int sysid, int compid, int msgid);

private slots:
    void onBytesReceived(gcs::comms::LinkInterface* link, const QByteArray& bytes);

private:
    void handleFrame(const DecodedFrame& f);

    MavlinkV2Parser m_parser;
    int m_decodedCount = 0;
    gcs::comms::LinkInterface* m_currentSourceLink = nullptr;
};

} // namespace gcs::protocol

Q_DECLARE_METATYPE(gcs::protocol::msg::Heartbeat)
Q_DECLARE_METATYPE(gcs::protocol::msg::SysStatus)
Q_DECLARE_METATYPE(gcs::protocol::msg::GpsRawInt)
Q_DECLARE_METATYPE(gcs::protocol::msg::Attitude)
Q_DECLARE_METATYPE(gcs::protocol::msg::GlobalPositionInt)
Q_DECLARE_METATYPE(gcs::protocol::msg::VfrHud)
Q_DECLARE_METATYPE(gcs::protocol::msg::BatteryStatus)
Q_DECLARE_METATYPE(gcs::protocol::msg::Statustext)
Q_DECLARE_METATYPE(gcs::protocol::msg::MissionCount)
Q_DECLARE_METATYPE(gcs::protocol::msg::MissionRequestInt)
Q_DECLARE_METATYPE(gcs::protocol::msg::MissionItemInt)
Q_DECLARE_METATYPE(gcs::protocol::msg::MissionAck)
Q_DECLARE_METATYPE(gcs::protocol::msg::MissionRequestList)
