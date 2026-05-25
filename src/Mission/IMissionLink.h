#pragma once

#include <QObject>

#include "MissionItem.h"

namespace gcs::mission {

// MAVLink-shaped abstraction over the mission protocol. Modelled on the
// MISSION_COUNT / MISSION_REQUEST_INT / MISSION_ITEM_INT / MISSION_ACK
// exchange so a Phase 3 `MavlinkMissionLink` can drop in unchanged.
//
// SAFETY: this is a transport seam, not a command issuer. Nothing here can
// arm/take-off/RTL. Mission upload is data transfer only.
//
// "send*" methods are GCS-side outbound. "*Received" signals fire when the
// other end (autopilot or MockMissionLink) talks back.
class IMissionLink : public QObject
{
    Q_OBJECT
public:
    enum AckResult {
        Accepted             = 0,
        Error                = 1,
        UnsupportedFrame     = 2,
        UnsupportedCommand   = 3,
        NoSpace              = 4,
        Invalid              = 5,
        InvalidSequence      = 7,
    };
    Q_ENUM(AckResult)

    explicit IMissionLink(QObject* parent = nullptr) : QObject(parent) {}
    ~IMissionLink() override = default;

    // Outbound (GCS -> "vehicle")
    virtual void sendMissionCount(int count) = 0;
    virtual void sendMissionItemInt(const MissionItem& item) = 0;
    virtual void sendMissionAck(int result) = 0;
    virtual void sendMissionRequestList() = 0;
    virtual void sendMissionRequestInt(int seq) = 0;

signals:
    // Inbound (from "vehicle" -> GCS)
    void missionRequestIntReceived(int seq);
    void missionAckReceived(int result);
    void missionCountReceived(int count);
    void missionItemIntReceived(const gcs::mission::MissionItem& item);
};

} // namespace gcs::mission
