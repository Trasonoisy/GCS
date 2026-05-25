#pragma once

#include "FirmwarePlugin.h"

namespace gcs::firmware {

class PX4FirmwarePlugin : public FirmwarePlugin
{
    Q_OBJECT
public:
    explicit PX4FirmwarePlugin(QObject* parent = nullptr);

    QString firmwareName() const override { return QStringLiteral("PX4"); }

    QString decodeFlightMode(uint8_t baseMode, uint32_t customMode) const override;
    bool    encodeFlightMode(const QString& mode,
                             uint8_t& baseMode,
                             uint32_t& customMode) const override;

    MissionFramePolicy  missionFramePolicy() const override;
    ManualControlPolicy manualControlPolicy() const override;
    QList<int>          supportedMissionCommands() const override;
    QString             prearmTextPrefix() const override
    { return QStringLiteral("Pre-arm"); }
};

} // namespace gcs::firmware
