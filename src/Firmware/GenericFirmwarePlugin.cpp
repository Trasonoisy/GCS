#include "GenericFirmwarePlugin.h"

namespace gcs::firmware {

GenericFirmwarePlugin::GenericFirmwarePlugin(QObject* parent)
    : FirmwarePlugin(parent)
{
}

QString GenericFirmwarePlugin::decodeFlightMode(uint8_t /*baseMode*/,
                                                uint32_t customMode) const
{
    return QStringLiteral("Mode %1").arg(customMode);
}

bool GenericFirmwarePlugin::encodeFlightMode(const QString& /*mode*/,
                                             uint8_t& /*baseMode*/,
                                             uint32_t& /*customMode*/) const
{
    return false;
}

MissionFramePolicy GenericFirmwarePlugin::missionFramePolicy() const
{
    MissionFramePolicy p;
    p.useMissionItemInt = true;
    p.homeIsMissionSeq0 = false;
    return p;
}

ManualControlPolicy GenericFirmwarePlugin::manualControlPolicy() const
{
    ManualControlPolicy p;
    p.supportsManualControl = false;
    p.allowsRcChannelsOverride = false;
    p.maxStreamRateHz = 0.0;
    return p;
}

} // namespace gcs::firmware
