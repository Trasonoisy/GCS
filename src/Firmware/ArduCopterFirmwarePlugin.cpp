#include "ArduCopterFirmwarePlugin.h"

namespace gcs::firmware {

ArduCopterFirmwarePlugin::ArduCopterFirmwarePlugin(QObject* parent)
    : ArduPilotFirmwarePlugin(parent) {}

QString ArduCopterFirmwarePlugin::decodeFlightMode(uint8_t /*baseMode*/,
                                                   uint32_t customMode) const
{
    // ArduCopter flight-mode names. Source: ardupilot/ArduCopter/mode.h.
    // We surface the names QGroundControl uses so the Fly view label looks
    // familiar to operators.
    switch (customMode) {
        case 0:  return QStringLiteral("Stabilize");
        case 1:  return QStringLiteral("Acro");
        case 2:  return QStringLiteral("Alt Hold");
        case 3:  return QStringLiteral("Auto");
        case 4:  return QStringLiteral("Guided");
        case 5:  return QStringLiteral("Loiter");
        case 6:  return QStringLiteral("RTL");
        case 7:  return QStringLiteral("Circle");
        case 9:  return QStringLiteral("Land");
        case 11: return QStringLiteral("Drift");
        case 13: return QStringLiteral("Sport");
        case 14: return QStringLiteral("Flip");
        case 15: return QStringLiteral("Autotune");
        case 16: return QStringLiteral("PosHold");
        case 17: return QStringLiteral("Brake");
        case 18: return QStringLiteral("Throw");
        case 19: return QStringLiteral("Avoid ADSB");
        case 20: return QStringLiteral("Guided NoGPS");
        case 21: return QStringLiteral("Smart RTL");
        case 22: return QStringLiteral("FlowHold");
        case 23: return QStringLiteral("Follow");
        case 24: return QStringLiteral("ZigZag");
        case 25: return QStringLiteral("SystemID");
        case 26: return QStringLiteral("Autorotate");
        case 27: return QStringLiteral("Auto RTL");
        case 28: return QStringLiteral("Turtle");
        default: return QStringLiteral("Copter mode %1").arg(customMode);
    }
}

} // namespace gcs::firmware
