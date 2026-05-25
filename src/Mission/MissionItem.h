#pragma once

#include <QMetaType>
#include <QString>

namespace gcs::mission {

// MAV_CMD ids used by Phase 2. Names match the MAVLink common dialect so
// Phase 3 can swap to the c_library_v2 generated enum without changing call
// sites. We deliberately keep this short — adding more commands requires
// teaching MissionValidator and FirmwarePlugin::supportedMissionCommands().
namespace cmd {
constexpr int NavWaypoint        = 16;
constexpr int NavLoiterUnlim     = 17;
constexpr int NavReturnToLaunch  = 20;
constexpr int NavLand            = 21;
constexpr int NavTakeoff         = 22;
} // namespace cmd

// MAV_FRAME ids. Phase 2 stores waypoints as MISSION_ITEM_INT, so the
// preferred frame is GlobalRelativeAltInt.
namespace frame {
constexpr int Global                  = 0;
constexpr int GlobalRelativeAlt       = 3;
constexpr int GlobalInt               = 5;
constexpr int GlobalRelativeAltInt    = 6;
constexpr int GlobalTerrainAlt        = 10;
} // namespace frame

QString commandName(int command);
QString frameName(int frame);

// Plain-data mission item. Units:
//   latitudeDeg, longitudeDeg : degrees (WGS-84)
//   altitudeM                 : metres (relative to home for relative frames)
//   holdTimeSec               : seconds at the waypoint
//   acceptanceRadiusM         : metres
//   yawDeg                    : degrees (0..360). NaN = "do not change yaw".
struct MissionItem
{
    int     seq               = 0;
    int     command           = cmd::NavWaypoint;
    int     frame             = frame::GlobalRelativeAltInt;
    double  latitudeDeg       = 0.0;
    double  longitudeDeg      = 0.0;
    double  altitudeM         = 0.0;

    // MAVLink MISSION_ITEM_INT params 1..4. We store the named ones here and
    // pack them into the protocol-level message at the link boundary.
    double  holdTimeSec       = 0.0;
    double  acceptanceRadiusM = 2.5;
    double  yawDeg            = 0.0; // NaN allowed
    bool    autocontinue      = true;
};

bool operator==(const MissionItem& a, const MissionItem& b);
inline bool operator!=(const MissionItem& a, const MissionItem& b) { return !(a == b); }

} // namespace gcs::mission

Q_DECLARE_METATYPE(gcs::mission::MissionItem)
