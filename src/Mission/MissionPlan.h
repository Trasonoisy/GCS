#pragma once

#include <QList>
#include <QString>

#include "MissionItem.h"

namespace gcs::mission {

// Working-copy mission plan held by the editor (MissionViewModel) and
// (post-upload) by the Vehicle's MissionManager.
struct MissionPlan
{
    QString fileType      = QStringLiteral("Plan");
    QString groundStation = QStringLiteral("LabGCS");
    int     version       = 1;

    // Firmware/vehicle ids are kept as strings to avoid leaking MAVLink enum
    // values through our schema. Phase 3 maps them to MAV_AUTOPILOT /
    // MAV_TYPE if needed.
    QString firmwareType  = QStringLiteral("PX4");
    QString vehicleType   = QStringLiteral("quadrotor");

    double  defaultAltitudeM = 50.0;
    double  cruiseSpeedMps   = 15.0;
    double  hoverSpeedMps    = 5.0;

    // Planned home position. Null lat/lon means "not set" (we save NaN).
    double  homeLatitudeDeg  = qQNaN();
    double  homeLongitudeDeg = qQNaN();
    double  homeAltitudeM    = 0.0;

    QList<MissionItem> items;

    bool isEmpty() const { return items.isEmpty(); }
    void renumberSequencesInPlace();
};

} // namespace gcs::mission
