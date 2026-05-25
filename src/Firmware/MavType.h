#pragma once

#include <QString>
#include <cstdint>

namespace gcs::firmware {

// Classify a HEARTBEAT.type byte (MAV_TYPE enum) into a broad airframe
// "kind" used for plugin dispatch and UI labelling. We do NOT vendor the
// full MAV_TYPE enum — Phase 5 only needs the four ArduPilot families plus
// a catch-all.
enum class AirframeKind
{
    Copter,
    Plane,
    Rover,
    Sub,
    Other,
};

inline AirframeKind airframeKindFromMavType(uint8_t mavType)
{
    switch (mavType) {
        // Multicopter / helicopter family -> ArduCopter
        case 2:  // MAV_TYPE_QUADROTOR
        case 4:  // MAV_TYPE_HELICOPTER
        case 13: // MAV_TYPE_HEXAROTOR
        case 14: // MAV_TYPE_OCTOROTOR
        case 15: // MAV_TYPE_TRICOPTER
        case 29: // MAV_TYPE_DODECAROTOR
        case 3:  // MAV_TYPE_COAXIAL
            return AirframeKind::Copter;

        // Fixed-wing family (incl. VTOL) -> ArduPlane
        case 1:  // MAV_TYPE_FIXED_WING
        case 19: // MAV_TYPE_VTOL_DUOROTOR
        case 20: // MAV_TYPE_VTOL_QUADROTOR
        case 21: // MAV_TYPE_VTOL_TILTROTOR
        case 22: // MAV_TYPE_VTOL_FIXEDROTOR
        case 23: // MAV_TYPE_VTOL_TAILSITTER
        case 24: // MAV_TYPE_VTOL_TILTWING
            return AirframeKind::Plane;

        // Ground / surface -> ArduRover
        case 10: // MAV_TYPE_GROUND_ROVER
        case 11: // MAV_TYPE_SURFACE_BOAT
            return AirframeKind::Rover;

        // Submarine -> ArduSub
        case 12: // MAV_TYPE_SUBMARINE
            return AirframeKind::Sub;

        default:
            return AirframeKind::Other;
    }
}

inline QString airframeKindName(AirframeKind k)
{
    switch (k) {
        case AirframeKind::Copter: return QStringLiteral("Copter");
        case AirframeKind::Plane:  return QStringLiteral("Plane");
        case AirframeKind::Rover:  return QStringLiteral("Rover");
        case AirframeKind::Sub:    return QStringLiteral("Sub");
        case AirframeKind::Other:  return QStringLiteral("Other");
    }
    return QStringLiteral("Other");
}

inline QString airframeNameFromMavType(uint8_t mavType)
{
    return airframeKindName(airframeKindFromMavType(mavType));
}

// MAV_AUTOPILOT constants we care about — kept here to avoid a c_library_v2
// dependency in Phase 5.
namespace autopilot {
constexpr uint8_t Generic        = 0;  // MAV_AUTOPILOT_GENERIC
constexpr uint8_t ArduPilotMega  = 3;  // MAV_AUTOPILOT_ARDUPILOTMEGA
constexpr uint8_t Px4            = 12; // MAV_AUTOPILOT_PX4
}

} // namespace gcs::firmware
