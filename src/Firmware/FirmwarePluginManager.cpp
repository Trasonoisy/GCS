#include "FirmwarePluginManager.h"

#include "ArduCopterFirmwarePlugin.h"
#include "ArduPilotFirmwarePlugin.h"
#include "ArduPlaneFirmwarePlugin.h"
#include "ArduRoverFirmwarePlugin.h"
#include "ArduSubFirmwarePlugin.h"
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
        // Unknown ArduPilot airframe → generic ArduPilot plugin (decode
        // falls back to "Mode N"). Still safe; nothing is sent to the wire.
        return new ArduPilotFirmwarePlugin(parent);
    }

    // MAV_AUTOPILOT_GENERIC and friends — we don't know the family, so use
    // the ArduPilot base which gives a "Mode N" label. SafetyGate will keep
    // manual control blocked for an unknown autopilot type.
    return new ArduPilotFirmwarePlugin(parent);
}

} // namespace gcs::firmware
