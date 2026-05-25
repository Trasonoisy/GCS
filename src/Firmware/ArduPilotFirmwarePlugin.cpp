#include "ArduPilotFirmwarePlugin.h"

namespace gcs::firmware {

ArduPilotFirmwarePlugin::ArduPilotFirmwarePlugin(QObject* parent)
    : FirmwarePlugin(parent) {}

QString ArduPilotFirmwarePlugin::decodeFlightMode(uint8_t /*baseMode*/,
                                                  uint32_t customMode) const
{
    // Generic fallback. ArduPilot encodes the flight mode as a flat number
    // in custom_mode, but the name table is vehicle-specific — ArduCopter,
    // ArduPlane, ArduRover, ArduSub each override this method. If we hit
    // the base implementation the airframe is unrecognised; surface the raw
    // index so the UI label is at least informative.
    return QStringLiteral("Mode %1").arg(customMode);
}

bool ArduPilotFirmwarePlugin::encodeFlightMode(const QString& /*mode*/,
                                               uint8_t& /*baseMode*/,
                                               uint32_t& /*customMode*/) const
{
    // SAFETY: see PX4FirmwarePlugin::encodeFlightMode.
    return false;
}

MissionFramePolicy ArduPilotFirmwarePlugin::missionFramePolicy() const
{
    MissionFramePolicy p;
    p.useMissionItemInt = true;
    p.homeIsMissionSeq0 = true; // ArduPilot expects home at index 0
    return p;
}

ManualControlPolicy ArduPilotFirmwarePlugin::manualControlPolicy() const
{
    ManualControlPolicy p;
    p.supportsManualControl    = true;
    p.allowsRcChannelsOverride = false; // never for human control
    p.maxStreamRateHz          = 50.0;
    return p;
}

QList<int> ArduPilotFirmwarePlugin::supportedMissionCommands() const
{
    // TODO(phase5): expand for ArduCopter/ArduPlane.
    return {};
}

} // namespace gcs::firmware
