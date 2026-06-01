#include "FirmwarePluginManager.h"

#include "ArduCopterFirmwarePlugin.h"
#include "ArduPilotFirmwarePlugin.h"
#include "ArduPlaneFirmwarePlugin.h"
#include "ArduRoverFirmwarePlugin.h"
#include "ArduSubFirmwarePlugin.h"
#include "GenericFirmwarePlugin.h"
#include "MavType.h"
#include "PX4FirmwarePlugin.h"

namespace gcs::firmware {

FirmwarePlugin* FirmwarePluginManager::createForHeartbeat(uint8_t autopilot,
                                                          uint8_t mavType,
                                                          QObject* parent)
{
    if (autopilot == autopilot::Px4) {
        return new PX4FirmwarePlugin(parent);
    }

    if (autopilot == autopilot::ArduPilotMega) {
        switch (airframeKindFromMavType(mavType)) {
            case AirframeKind::Copter: return new ArduCopterFirmwarePlugin(parent);
            case AirframeKind::Plane:  return new ArduPlaneFirmwarePlugin(parent);
            case AirframeKind::Rover:  return new ArduRoverFirmwarePlugin(parent);
            case AirframeKind::Sub:    return new ArduSubFirmwarePlugin(parent);
            case AirframeKind::Other:  break;
        }

        // Unknown ArduPilot airframe still uses the ArduPilot family plugin,
        // but only the base table is available so the mode label is "Mode N".
        return new ArduPilotFirmwarePlugin(parent);
    }

    // MAV_AUTOPILOT_GENERIC and friends. Unknown autopilots must not inherit
    // ArduPilot's SITL permissions through the UI label or SafetyGate.
    return new GenericFirmwarePlugin(parent);
}

} // namespace gcs::firmware
