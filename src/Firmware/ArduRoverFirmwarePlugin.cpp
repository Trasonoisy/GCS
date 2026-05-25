#include "ArduRoverFirmwarePlugin.h"

namespace gcs::firmware {

ArduRoverFirmwarePlugin::ArduRoverFirmwarePlugin(QObject* parent)
    : ArduPilotFirmwarePlugin(parent) {}

QString ArduRoverFirmwarePlugin::decodeFlightMode(uint8_t /*baseMode*/,
                                                  uint32_t customMode) const
{
    // ArduRover mode table. Source: ardupilot/Rover/mode.h.
    switch (customMode) {
        case 0:  return QStringLiteral("Manual");
        case 1:  return QStringLiteral("Acro");
        case 3:  return QStringLiteral("Steering");
        case 4:  return QStringLiteral("Hold");
        case 5:  return QStringLiteral("Loiter");
        case 6:  return QStringLiteral("Follow");
        case 7:  return QStringLiteral("Simple");
        case 8:  return QStringLiteral("Dock");
        case 9:  return QStringLiteral("Circle");
        case 10: return QStringLiteral("Auto");
        case 11: return QStringLiteral("RTL");
        case 12: return QStringLiteral("SmartRTL");
        case 15: return QStringLiteral("Guided");
        case 16: return QStringLiteral("Initialising");
        default: return QStringLiteral("Rover mode %1").arg(customMode);
    }
}

} // namespace gcs::firmware
