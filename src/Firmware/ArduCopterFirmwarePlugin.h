#pragma once

#include "ArduPilotFirmwarePlugin.h"

namespace gcs::firmware {

// ArduCopter custom_mode mapping. Mode index lives in
// HEARTBEAT.custom_mode (lowest byte / whole 32-bit value — ArduPilot stores
// it as a flat integer, not bit-packed like PX4).
class ArduCopterFirmwarePlugin : public ArduPilotFirmwarePlugin
{
    Q_OBJECT
public:
    explicit ArduCopterFirmwarePlugin(QObject* parent = nullptr);

    QString decodeFlightMode(uint8_t baseMode, uint32_t customMode) const override;
};

} // namespace gcs::firmware
