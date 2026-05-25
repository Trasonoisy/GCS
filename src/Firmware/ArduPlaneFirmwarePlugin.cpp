#include "ArduPlaneFirmwarePlugin.h"

namespace gcs::firmware {

ArduPlaneFirmwarePlugin::ArduPlaneFirmwarePlugin(QObject* parent)
    : ArduPilotFirmwarePlugin(parent) {}

QString ArduPlaneFirmwarePlugin::decodeFlightMode(uint8_t /*baseMode*/,
                                                  uint32_t customMode) const
{
    // ArduPlane mode table. Source: ardupilot/ArduPlane/mode.h.
    switch (customMode) {
        case 0:  return QStringLiteral("Manual");
        case 1:  return QStringLiteral("Circle");
        case 2:  return QStringLiteral("Stabilize");
        case 3:  return QStringLiteral("Training");
        case 4:  return QStringLiteral("Acro");
        case 5:  return QStringLiteral("FBWA");
        case 6:  return QStringLiteral("FBWB");
        case 7:  return QStringLiteral("Cruise");
        case 8:  return QStringLiteral("Autotune");
        case 10: return QStringLiteral("Auto");
        case 11: return QStringLiteral("RTL");
        case 12: return QStringLiteral("Loiter");
        case 13: return QStringLiteral("Takeoff");
        case 14: return QStringLiteral("Avoid ADSB");
        case 15: return QStringLiteral("Guided");
        case 16: return QStringLiteral("Initialising");
        case 17: return QStringLiteral("QStabilize");
        case 18: return QStringLiteral("QHover");
        case 19: return QStringLiteral("QLoiter");
        case 20: return QStringLiteral("QLand");
        case 21: return QStringLiteral("QRTL");
        case 22: return QStringLiteral("QAutotune");
        case 23: return QStringLiteral("QAcro");
        case 24: return QStringLiteral("Thermal");
        default: return QStringLiteral("Plane mode %1").arg(customMode);
    }
}

} // namespace gcs::firmware
