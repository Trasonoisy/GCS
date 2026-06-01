#include "MAVLinkProtocol.h"

namespace gcs::protocol {

MAVLinkProtocol::MAVLinkProtocol(QObject* parent) : QObject(parent) {}

void MAVLinkProtocol::attachLink(gcs::comms::LinkInterface* link)
{
    if (!link) return;
    connect(link, &gcs::comms::LinkInterface::bytesReceived,
            this, &MAVLinkProtocol::onBytesReceived);
}

void MAVLinkProtocol::detachLink(gcs::comms::LinkInterface* link)
{
    if (!link) return;
    disconnect(link, &gcs::comms::LinkInterface::bytesReceived,
               this, &MAVLinkProtocol::onBytesReceived);
}

void MAVLinkProtocol::feedBytesForTest(const QByteArray& bytes)
{
    onBytesReceived(nullptr, bytes);
}

void MAVLinkProtocol::onBytesReceived(gcs::comms::LinkInterface* link,
                                      const QByteArray& bytes)
{
    // Phase 7: stash the source link for the duration of this fan-out so
    // the router (or any slot connected to a typed *Received signal) can
    // ask currentSourceLink() to learn which transport produced the bytes.
    m_currentSourceLink = link;
    emit rawBytesIn(static_cast<int>(bytes.size()));
    m_parser.feed(bytes, [this](const DecodedFrame& f) { handleFrame(f); });
    m_currentSourceLink = nullptr;
}

void MAVLinkProtocol::handleFrame(const DecodedFrame& f)
{
    ++m_decodedCount;
    switch (f.msgid) {
        case msgid::Heartbeat: {
            msg::Heartbeat hb;
            msg::decodeHeartbeat(f.payload, hb);
            emit heartbeatReceived(f.sysid, f.compid, hb);
            break;
        }
        case msgid::SysStatus: {
            msg::SysStatus s;
            msg::decodeSysStatus(f.payload, s);
            emit sysStatusReceived(f.sysid, f.compid, s);
            break;
        }
        case msgid::GpsRawInt: {
            msg::GpsRawInt g;
            msg::decodeGpsRawInt(f.payload, g);
            emit gpsRawIntReceived(f.sysid, f.compid, g);
            break;
        }
        case msgid::Attitude: {
            msg::Attitude a;
            msg::decodeAttitude(f.payload, a);
            emit attitudeReceived(f.sysid, f.compid, a);
            break;
        }
        case msgid::GlobalPositionInt: {
            msg::GlobalPositionInt p;
            msg::decodeGlobalPositionInt(f.payload, p);
            emit globalPositionIntReceived(f.sysid, f.compid, p);
            break;
        }
        case msgid::VfrHud: {
            msg::VfrHud v;
            msg::decodeVfrHud(f.payload, v);
            emit vfrHudReceived(f.sysid, f.compid, v);
            break;
        }
        case msgid::BatteryStatus: {
            msg::BatteryStatus b;
            msg::decodeBatteryStatus(f.payload, b);
            emit batteryStatusReceived(f.sysid, f.compid, b);
            break;
        }
        case msgid::Statustext: {
            msg::Statustext t;
            msg::decodeStatustext(f.payload, t);
            emit statustextReceived(f.sysid, f.compid, t);
            break;
        }
        // Phase 8: mission-protocol fan-out for SITL upload/download.
        case msgid::MissionRequest: {
            // ArduPilot can still emit deprecated MISSION_REQUEST during an
            // upload transaction. MAVLink marks it as replaced by
            // MISSION_REQUEST_INT, so surface it through the same signal.
            msg::MissionRequestInt m;
            msg::decodeMissionRequestInt(f.payload, m);
            emit missionRequestIntReceived(f.sysid, f.compid, m);
            break;
        }
        case msgid::MissionCount: {
            msg::MissionCount m;
            msg::decodeMissionCount(f.payload, m);
            emit missionCountReceived(f.sysid, f.compid, m);
            break;
        }
        case msgid::MissionRequestInt: {
            msg::MissionRequestInt m;
            msg::decodeMissionRequestInt(f.payload, m);
            emit missionRequestIntReceived(f.sysid, f.compid, m);
            break;
        }
        case msgid::MissionItemInt: {
            msg::MissionItemInt m;
            msg::decodeMissionItemInt(f.payload, m);
            emit missionItemIntReceived(f.sysid, f.compid, m);
            break;
        }
        case msgid::MissionAck: {
            msg::MissionAck m;
            msg::decodeMissionAck(f.payload, m);
            emit missionAckReceived(f.sysid, f.compid, m);
            break;
        }
        case msgid::MissionRequestList: {
            msg::MissionRequestList m;
            msg::decodeMissionRequestList(f.payload, m);
            emit missionRequestListReceived(f.sysid, f.compid, m);
            break;
        }
        default:
            emit unknownMessageReceived(f.sysid, f.compid, f.msgid);
            break;
    }
}

} // namespace gcs::protocol
