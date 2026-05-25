#pragma once

#include "FirmwarePlugin.h"

namespace gcs::firmware {

class ArduPilotFirmwarePlugin : public FirmwarePlugin
{
    Q_OBJECT
public:
    explicit ArduPilotFirmwarePlugin(QObject* parent = nullptr);

    QString firmwareName() const override { return QStringLiteral("ArduPilot"); }

    QString decodeFlightMode(uint8_t baseMode, uint32_t customMode) const override;
    bool    encodeFlightMode(const QString& mode,
                             uint8_t& baseMode,
                             uint32_t& customMode) const override;

    MissionFramePolicy  missionFramePolicy() const override;
    ManualControlPolicy manualControlPolicy() const override;
    QList<int>          supportedMissionCommands() const override;
    QString             prearmTextPrefix() const override
    { return QStringLiteral("PreArm"); }
};

} // namespace gcs::firmware
