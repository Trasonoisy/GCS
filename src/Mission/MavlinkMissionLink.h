#pragma once

#include <QObject>
#include <QPointer>
#include <cstdint>

#include "IMissionLink.h"

namespace gcs::comms    { class LinkInterface; }
namespace gcs::protocol { class MAVLinkProtocol; }
namespace gcs::protocol { namespace msg {
    struct MissionCount;
    struct MissionRequestInt;
    struct MissionItemInt;
    struct MissionAck;
    struct MissionRequestList;
}}

namespace gcs::mission {

// IMissionLink backed by real MAVLink frames. Reuses the existing
// MissionUploader / MissionDownloader state machines — they cannot tell that
// the other end is a real SITL autopilot rather than MockMissionLink.
//
// SAFETY (Phase 8):
//   - Mission upload is data transfer only. The MAVLink mission protocol
//     does not arm, takeoff, switch mode, or start the mission. Those
//     stay blocked at SafetyGate.
//   - Constructed only by main.cpp's vehicle factory when the spawning
//     heartbeat came over a SITL transport (UDP / TCP) with a PX4 or
//     ArduPilot autopilot. Hardware (LinkKind::Serial) vehicles never
//     get a MavlinkMissionLink — see [`SafetyGate::canUploadMission`].
//   - MISSION_CLEAR_ALL is deliberately NOT implemented: it could erase a
//     pre-loaded mission on a real vehicle and we have no operator-
//     confirmation surface for it in this phase.
class MavlinkMissionLink : public IMissionLink
{
    Q_OBJECT
public:
    // targetSystemId / targetComponentId : the autopilot we are talking to.
    // gcsSystemId / gcsComponentId       : our identity on the bus. QGC uses
    //                                      sysid=255, compid=190 (MISSION_PLANNER).
    MavlinkMissionLink(int targetSystemId,
                       int targetComponentId,
                       gcs::protocol::MAVLinkProtocol* protocol,
                       gcs::comms::LinkInterface* outbound,
                       int gcsSystemId = 255,
                       int gcsComponentId = 190,
                       QObject* parent = nullptr);
    ~MavlinkMissionLink() override = default;

    int targetSystemId() const    { return m_targetSysid; }
    int targetComponentId() const { return m_targetCompid; }
    int gcsSystemId() const       { return m_gcsSysid; }
    int gcsComponentId() const    { return m_gcsCompid; }

    // IMissionLink — outbound from GCS to autopilot.
    void sendMissionCount(int count) override;
    void sendMissionItemInt(const MissionItem& item) override;
    void sendMissionAck(int result) override;
    void sendMissionRequestList() override;
    void sendMissionRequestInt(int seq) override;

private slots:
    // MAVLinkProtocol -> per-vehicle filter -> IMissionLink signals
    void onMissionCountReceived       (int sysid, int compid,
                                       const gcs::protocol::msg::MissionCount& m);
    void onMissionRequestIntReceived  (int sysid, int compid,
                                       const gcs::protocol::msg::MissionRequestInt& m);
    void onMissionItemIntReceived     (int sysid, int compid,
                                       const gcs::protocol::msg::MissionItemInt& m);
    void onMissionAckReceived         (int sysid, int compid,
                                       const gcs::protocol::msg::MissionAck& m);

private:
    bool matchesTarget(int sysid, int compid) const;
    void writeFrame(int msgid, const QByteArray& payload);

    int m_targetSysid;
    int m_targetCompid;
    int m_gcsSysid;
    int m_gcsCompid;

    QPointer<gcs::protocol::MAVLinkProtocol> m_protocol;
    QPointer<gcs::comms::LinkInterface>      m_outbound;
};

} // namespace gcs::mission
