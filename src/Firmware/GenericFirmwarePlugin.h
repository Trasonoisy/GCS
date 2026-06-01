#pragma once

#include "FirmwarePlugin.h"

namespace gcs::firmware {

// Fail-closed plugin for MAV_AUTOPILOT_GENERIC or unrecognised autopilots.
// It keeps labels useful while ensuring SafetyGate does not treat the vehicle
// as PX4/ArduPilot SITL.
class GenericFirmwarePlugin : public FirmwarePlugin
{
    Q_OBJECT
public:
    explicit GenericFirmwarePlugin(QObject* parent = nullptr);

    QString firmwareName() const override { return QStringLiteral("unknown"); }
    QString decodeFlightMode(uint8_t baseMode, uint32_t customMode) const override;
    bool    encodeFlightMode(const QString& mode,
                             uint8_t& baseMode,
                             uint32_t& customMode) const override;

    MissionFramePolicy  missionFramePolicy() const override;
    ManualControlPolicy manualControlPolicy() const override;
    QList<int> supportedMissionCommands() const override { return {}; }
    QString prearmTextPrefix() const override { return {}; }
};

} // namespace gcs::firmware
