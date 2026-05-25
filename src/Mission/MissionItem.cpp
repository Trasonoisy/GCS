#include "MissionItem.h"

#include <cmath>

namespace gcs::mission {

QString commandName(int command)
{
    switch (command) {
        case cmd::NavWaypoint:       return QStringLiteral("WAYPOINT");
        case cmd::NavLoiterUnlim:    return QStringLiteral("LOITER");
        case cmd::NavReturnToLaunch: return QStringLiteral("RTL");
        case cmd::NavLand:           return QStringLiteral("LAND");
        case cmd::NavTakeoff:        return QStringLiteral("TAKEOFF");
        default:                     return QStringLiteral("CMD_%1").arg(command);
    }
}

QString frameName(int f)
{
    switch (f) {
        case frame::Global:               return QStringLiteral("GLOBAL");
        case frame::GlobalRelativeAlt:    return QStringLiteral("GLOBAL_RELATIVE_ALT");
        case frame::GlobalInt:            return QStringLiteral("GLOBAL_INT");
        case frame::GlobalRelativeAltInt: return QStringLiteral("GLOBAL_RELATIVE_ALT_INT");
        case frame::GlobalTerrainAlt:     return QStringLiteral("GLOBAL_TERRAIN_ALT");
        default:                          return QStringLiteral("FRAME_%1").arg(f);
    }
}

namespace {
bool eqNanAware(double a, double b)
{
    if (std::isnan(a) && std::isnan(b)) return true;
    return a == b;
}
} // namespace

bool operator==(const MissionItem& a, const MissionItem& b)
{
    return a.seq == b.seq
        && a.command == b.command
        && a.frame == b.frame
        && a.latitudeDeg == b.latitudeDeg
        && a.longitudeDeg == b.longitudeDeg
        && a.altitudeM == b.altitudeM
        && a.holdTimeSec == b.holdTimeSec
        && a.acceptanceRadiusM == b.acceptanceRadiusM
        && eqNanAware(a.yawDeg, b.yawDeg)
        && a.autocontinue == b.autocontinue;
}

} // namespace gcs::mission
