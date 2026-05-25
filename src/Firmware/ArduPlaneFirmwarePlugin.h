#pragma once

#include "ArduPilotFirmwarePlugin.h"

namespace gcs::firmware {

class ArduPlaneFirmwarePlugin : public ArduPilotFirmwarePlugin
{
    Q_OBJECT
public:
    explicit ArduPlaneFirmwarePlugin(QObject* parent = nullptr);
    QString decodeFlightMode(uint8_t baseMode, uint32_t customMode) const override;
};

} // namespace gcs::firmware
