#include "MavlinkMissionLink.h"

#include <cmath>

#include "Comms/LinkInterface.h"
#include "Protocol/MAVLinkProtocol.h"
#include "Protocol/MavlinkFrame.h"

namespace gcs::mission {

using gcs::protocol::msg::MissionAck;
using gcs::protocol::msg::MissionCount;
using gcs::protocol::msg::MissionItemInt;
using gcs::protocol::msg::MissionRequestInt;
using gcs::protocol::msg::MissionRequestList;

namespace {
// Convert lat/lon degrees → MAVLink's int32 1e7-scaled representation.
qint32 degToE7(double deg)
{
    if (std::isnan(deg)) return 0;
    return static_cast<qint32>(std::lround(deg * 1.0e7));
}
double e7ToDeg(qint32 e7) { return double(e7) * 1.0e-7; }
} // namespace

MavlinkMissionLink::MavlinkMissionLink(int targetSystemId,
                                       int targetComponentId,
                                       gcs::protocol::MAVLinkProtocol* protocol,
                                       gcs::comms::LinkInterface* outbound,
                                       int gcsSystemId,
                                       int gcsComponentId,
                                       QObject* parent)
    : IMissionLink(parent),
      m_targetSysid(targetSystemId),
      m_targetCompid(targetComponentId),
      m_gcsSysid(gcsSystemId),
      m_gcsCompid(gcsComponentId),
      m_protocol(protocol),
      m_outbound(outbound)
{
    Q_ASSERT(protocol);
    // SAFETY: we tolerate a null outbound here (the link can be torn down
    // mid-session). Outbound calls fail closed in writeFrame().
    connect(protocol, &gcs::protocol::MAVLinkProtocol::missionCountReceived,
            this, &MavlinkMissionLink::onMissionCountReceived);
    connect(protocol, &gcs::protocol::MAVLinkProtocol::missionRequestIntReceived,
            this, &MavlinkMissionLink::onMissionRequestIntReceived);
    connect(protocol, &gcs::protocol::MAVLinkProtocol::missionItemIntReceived,
            this, &MavlinkMissionLink::onMissionItemIntReceived);
    connect(protocol, &gcs::protocol::MAVLinkProtocol::missionAckReceived,
            this, &MavlinkMissionLink::onMissionAckReceived);
}

bool MavlinkMissionLink::matchesTarget(int sysid, int compid) const
{
    if (sysid != m_targetSysid) return false;
    // Be lenient on compid — PX4 SITL emits mission ACKs/items from
    // compid 1 (autopilot), while ArduPilot may use mission planner compid
    // values. Accept any component from the matching system.
    Q_UNUSED(compid);
    return true;
}

void MavlinkMissionLink::writeFrame(int msgid, const QByteArray& payload)
{
    if (!m_outbound) return; // link gone — SAFETY: silently swallow
    const QByteArray frame = gcs::protocol::buildV2Frame(
        m_gcsSysid, m_gcsCompid, msgid, payload);
    m_outbound->writeBytes(frame);
}

// ---------- Outbound: GCS -> autopilot ----------

void MavlinkMissionLink::sendMissionCount(int count)
{
    MissionCount m;
    m.count            = static_cast<uint16_t>(qMax(0, count));
    m.target_system    = static_cast<uint8_t>(m_targetSysid);
    m.target_component = static_cast<uint8_t>(m_targetCompid);
    writeFrame(gcs::protocol::msgid::MissionCount,
               gcs::protocol::msg::encodeMissionCount(m));
}

void MavlinkMissionLink::sendMissionItemInt(const MissionItem& item)
{
    MissionItemInt m;
    m.param1   = static_cast<float>(item.holdTimeSec);
    m.param2   = static_cast<float>(item.acceptanceRadiusM);
    m.param3   = 0.0f;
    m.param4   = std::isnan(item.yawDeg) ? std::nanf("") : static_cast<float>(item.yawDeg);
    m.x        = degToE7(item.latitudeDeg);
    m.y        = degToE7(item.longitudeDeg);
    m.z        = static_cast<float>(item.altitudeM);
    m.seq      = static_cast<uint16_t>(item.seq);
    m.command  = static_cast<uint16_t>(item.command);
    m.target_system    = static_cast<uint8_t>(m_targetSysid);
    m.target_component = static_cast<uint8_t>(m_targetCompid);
    m.frame            = static_cast<uint8_t>(item.frame);
    m.current          = 0;
    m.autocontinue     = item.autocontinue ? 1 : 0;
    m.mission_type     = 0;
    writeFrame(gcs::protocol::msgid::MissionItemInt,
               gcs::protocol::msg::encodeMissionItemInt(m));
}

void MavlinkMissionLink::sendMissionAck(int result)
{
    MissionAck m;
    m.target_system    = static_cast<uint8_t>(m_targetSysid);
    m.target_component = static_cast<uint8_t>(m_targetCompid);
    m.type             = static_cast<uint8_t>(result);
    writeFrame(gcs::protocol::msgid::MissionAck,
               gcs::protocol::msg::encodeMissionAck(m));
}

void MavlinkMissionLink::sendMissionRequestList()
{
    MissionRequestList m;
    m.target_system    = static_cast<uint8_t>(m_targetSysid);
    m.target_component = static_cast<uint8_t>(m_targetCompid);
    writeFrame(gcs::protocol::msgid::MissionRequestList,
               gcs::protocol::msg::encodeMissionRequestList(m));
}

void MavlinkMissionLink::sendMissionRequestInt(int seq)
{
    MissionRequestInt m;
    m.seq              = static_cast<uint16_t>(qMax(0, seq));
    m.target_system    = static_cast<uint8_t>(m_targetSysid);
    m.target_component = static_cast<uint8_t>(m_targetCompid);
    writeFrame(gcs::protocol::msgid::MissionRequestInt,
               gcs::protocol::msg::encodeMissionRequestInt(m));
}

// ---------- Inbound: autopilot -> GCS ----------
//
// MAVLinkProtocol fans out every mission message to every MavlinkMissionLink.
// We filter by target system id so only the right Vehicle's state machine
// reacts (compare with QGC's MAVLinkMissionItemHandler — same shape).

void MavlinkMissionLink::onMissionCountReceived(int sysid, int compid,
                                                const MissionCount& m)
{
    if (!matchesTarget(sysid, compid)) return;
    emit missionCountReceived(static_cast<int>(m.count));
}

void MavlinkMissionLink::onMissionRequestIntReceived(int sysid, int compid,
                                                     const MissionRequestInt& m)
{
    if (!matchesTarget(sysid, compid)) return;
    emit missionRequestIntReceived(static_cast<int>(m.seq));
}

void MavlinkMissionLink::onMissionItemIntReceived(int sysid, int compid,
                                                  const MissionItemInt& m)
{
    if (!matchesTarget(sysid, compid)) return;

    MissionItem it;
    it.seq               = static_cast<int>(m.seq);
    it.command           = static_cast<int>(m.command);
    it.frame             = static_cast<int>(m.frame);
    it.latitudeDeg       = e7ToDeg(m.x);
    it.longitudeDeg      = e7ToDeg(m.y);
    it.altitudeM         = static_cast<double>(m.z);
    it.holdTimeSec       = static_cast<double>(m.param1);
    it.acceptanceRadiusM = static_cast<double>(m.param2);
    if (std::isnan(m.param4)) it.yawDeg = qQNaN();
    else                       it.yawDeg = static_cast<double>(m.param4);
    it.autocontinue = (m.autocontinue != 0);

    emit missionItemIntReceived(it);
}

void MavlinkMissionLink::onMissionAckReceived(int sysid, int compid,
                                              const MissionAck& m)
{
    if (!matchesTarget(sysid, compid)) return;
    emit missionAckReceived(static_cast<int>(m.type));
}

} // namespace gcs::mission
