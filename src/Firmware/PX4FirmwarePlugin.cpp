#include "PX4FirmwarePlugin.h"

namespace gcs::firmware {

namespace {

// PX4 packs custom_mode like this (px4_custom_mode.h):
//   union { struct { uint16_t reserved; uint8_t main; uint8_t sub; }; uint32_t data; };
// On little-endian wire that is:
//   bits  0..15 : reserved
//   bits 16..23 : main_mode
//   bits 24..31 : sub_mode
//
// We expose the names QGroundControl uses so the Fly view label looks familiar.
constexpr quint8 kPx4MainManual     = 1;
constexpr quint8 kPx4MainAltctl     = 2;
constexpr quint8 kPx4MainPosctl     = 3;
constexpr quint8 kPx4MainAuto       = 4;
constexpr quint8 kPx4MainAcro       = 5;
constexpr quint8 kPx4MainOffboard   = 6;
constexpr quint8 kPx4MainStabilized = 7;
constexpr quint8 kPx4MainRattitude  = 8;
constexpr quint8 kPx4MainSimple     = 9;

constexpr quint8 kPx4AutoReady        = 1;
constexpr quint8 kPx4AutoTakeoff      = 2;
constexpr quint8 kPx4AutoLoiter       = 3;
constexpr quint8 kPx4AutoMission      = 4;
constexpr quint8 kPx4AutoRtl          = 5;
constexpr quint8 kPx4AutoLand         = 6;
constexpr quint8 kPx4AutoRtgs         = 7;
constexpr quint8 kPx4AutoFollowTarget = 8;
constexpr quint8 kPx4AutoPrecland     = 9;

QString autoSubName(quint8 sub)
{
    switch (sub) {
        case kPx4AutoReady:        return QStringLiteral("Ready");
        case kPx4AutoTakeoff:      return QStringLiteral("Takeoff");
        case kPx4AutoLoiter:       return QStringLiteral("Hold");
        case kPx4AutoMission:      return QStringLiteral("Mission");
        case kPx4AutoRtl:          return QStringLiteral("Return");
        case kPx4AutoLand:         return QStringLiteral("Land");
        case kPx4AutoRtgs:         return QStringLiteral("RtGS");
        case kPx4AutoFollowTarget: return QStringLiteral("Follow");
        case kPx4AutoPrecland:     return QStringLiteral("PrecLand");
        default:                   return QStringLiteral("Auto.%1").arg(sub);
    }
}

constexpr quint8 kBaseModeCustomEnabled = 0x01; // MAV_MODE_FLAG_CUSTOM_MODE_ENABLED

} // namespace

PX4FirmwarePlugin::PX4FirmwarePlugin(QObject* parent) : FirmwarePlugin(parent) {}

QString PX4FirmwarePlugin::decodeFlightMode(uint8_t baseMode, uint32_t customMode) const
{
    // If the custom flag is not set, we can't trust custom_mode. Fall back
    // to the bare base-mode label (rarely seen in practice on PX4).
    if (!(baseMode & kBaseModeCustomEnabled) || customMode == 0) {
        return QStringLiteral("MANUAL");
    }
    const quint8 main = quint8((customMode >> 16) & 0xFF);
    const quint8 sub  = quint8((customMode >> 24) & 0xFF);

    switch (main) {
        case kPx4MainManual:     return QStringLiteral("Manual");
        case kPx4MainAltctl:     return QStringLiteral("Altitude");
        case kPx4MainPosctl:     return QStringLiteral("Position");
        case kPx4MainAuto:       return autoSubName(sub);
        case kPx4MainAcro:       return QStringLiteral("Acro");
        case kPx4MainOffboard:   return QStringLiteral("Offboard");
        case kPx4MainStabilized: return QStringLiteral("Stabilized");
        case kPx4MainRattitude:  return QStringLiteral("Rattitude");
        case kPx4MainSimple:     return QStringLiteral("Simple");
        default:                 return QStringLiteral("PX4.main=%1").arg(main);
    }
}

bool PX4FirmwarePlugin::encodeFlightMode(const QString& /*mode*/,
                                         uint8_t& /*baseMode*/,
                                         uint32_t& /*customMode*/) const
{
    // SAFETY: mode changes go through SafetyGate in Phase 4+. Refuse to
    // produce a real encoding until that path exists.
    return false;
}

MissionFramePolicy PX4FirmwarePlugin::missionFramePolicy() const
{
    MissionFramePolicy p;
    p.useMissionItemInt = true;
    p.homeIsMissionSeq0 = false; // PX4 does not require home as seq 0
    return p;
}

ManualControlPolicy PX4FirmwarePlugin::manualControlPolicy() const
{
    ManualControlPolicy p;
    p.supportsManualControl    = true;
    p.allowsRcChannelsOverride = false;
    p.maxStreamRateHz          = 50.0;
    return p;
}

QList<int> PX4FirmwarePlugin::supportedMissionCommands() const
{
    // SAFETY: real mission upload to PX4 is not enabled in Phase 3. We keep
    // returning an empty list so MissionValidator fails closed when a real
    // PX4 vehicle becomes active. Mission planning continues to target the
    // MockVehicle only.
    return {};
}

} // namespace gcs::firmware
