#include "ArduSubFirmwarePlugin.h"

namespace gcs::firmware {

ArduSubFirmwarePlugin::ArduSubFirmwarePlugin(QObject* parent)
    : ArduPilotFirmwarePlugin(parent) {}

QString ArduSubFirmwarePlugin::decodeFlightMode(uint8_t /*baseMode*/,
                                                uint32_t customMode) const
{
    // ArduSub mode table. Source: ardupilot/ArduSub/control_modes.cpp.
    switch (customMode) {
        case 0:  return QStringLiteral("Stabilize");
        case 1:  return QStringLiteral("Acro");
        case 2:  return QStringLiteral("Alt Hold");
        case 3:  return QStringLiteral("Auto");
        case 4:  return QStringLiteral("Guided");
        case 7:  return QStringLiteral("Circle");
        case 9:  return QStringLiteral("Surface");
        case 16: return QStringLiteral("PosHold");
        case 19: return QStringLiteral("Manual");
        case 20: return QStringLiteral("Motor Detect");
        default: return QStringLiteral("Sub mode %1").arg(customMode);
    }
}

} // namespace gcs::firmware
