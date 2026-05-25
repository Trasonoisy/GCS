#pragma once

#include <QString>
#include <QtGlobal>

#include "Comms/LinkKind.h"

namespace gcs::vehicle {

enum class LinkStatus {
    Disconnected = 0,
    Connected    = 1,
    Stale        = 2, // connected but heartbeat is overdue
};

// Plain-data snapshot of a vehicle. No Qt object identity, no signals — that
// belongs in VehicleStateStore which owns this struct.
//
// All numeric fields use SI units unless explicitly noted:
//   latitudeDeg / longitudeDeg : degrees (WGS-84)
//   relativeAltitudeM          : metres above home
//   headingDeg                 : 0..360, 0 = north
//   groundSpeedMps             : metres / second
//   batteryVoltage             : volts
//   batteryPercent             : 0..100, -1 = unknown
//   gpsFixType                 : MAVLink GPS_FIX_TYPE enum (0=no GPS .. 6=RTK fixed)
//   satellitesVisible          : count
//   lastHeartbeatUtcMs         : QDateTime::currentMSecsSinceEpoch() at last
//                                heartbeat. 0 = never received.
struct VehicleState
{
    // Identity
    int     systemId      = 0;
    int     componentId   = 0;
    QString vehicleType   = QStringLiteral("unknown");   // e.g. "quadrotor"
    QString autopilotType = QStringLiteral("unknown");   // e.g. "PX4", "ArduPilot"

    // Mode / arming
    QString flightMode    = QStringLiteral("UNKNOWN");
    bool    armed         = false;

    // Position
    double  latitudeDeg       = 0.0;
    double  longitudeDeg      = 0.0;
    double  relativeAltitudeM = 0.0;
    double  headingDeg        = 0.0;
    double  groundSpeedMps    = 0.0;

    // Attitude
    double  rollDeg           = 0.0;
    double  pitchDeg          = 0.0;

    // Battery
    double  batteryVoltage = 0.0;
    double  batteryPercent = -1.0;

    // GPS
    int     gpsFixType        = 0;
    int     satellitesVisible = 0;

    // Link
    LinkStatus           linkStatus         = LinkStatus::Disconnected;
    qint64               lastHeartbeatUtcMs = 0;
    gcs::comms::LinkKind linkKind           = gcs::comms::LinkKind::Unknown;

    // True iff this state came from a simulator. UI must show a banner.
    bool       simulated           = false;
};

} // namespace gcs::vehicle
