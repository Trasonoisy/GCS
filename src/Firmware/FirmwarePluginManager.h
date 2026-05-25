#pragma once

#include <QObject>
#include <cstdint>

namespace gcs::firmware {

class FirmwarePlugin;

// Factory that picks the right FirmwarePlugin for a freshly observed
// vehicle, based on HEARTBEAT.autopilot and HEARTBEAT.type.
//
//   MAV_AUTOPILOT_PX4              -> PX4FirmwarePlugin
//   MAV_AUTOPILOT_ARDUPILOTMEGA    -> ArduCopter / ArduPlane / ArduRover /
//                                     ArduSub depending on MAV_TYPE
//   anything else                  -> ArduPilotFirmwarePlugin (base) as a
//                                     safe fallback — its decodeFlightMode
//                                     returns "Mode N" so the UI still
//                                     surfaces *something*.
//
// SAFETY: This object only chooses a decoder for telemetry. It never
// constructs command paths.
class FirmwarePluginManager
{
public:
    // `parent` is the Qt parent for the returned plugin. The caller takes
    // ownership through the Qt parent/child relationship.
    static FirmwarePlugin* createForHeartbeat(uint8_t autopilot,
                                              uint8_t mavType,
                                              QObject* parent);
};

} // namespace gcs::firmware
