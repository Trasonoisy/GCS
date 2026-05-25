#pragma once

#include <QList>
#include <QObject>
#include <QString>
#include <cstdint>

namespace gcs::firmware {

// Policies returned by FirmwarePlugin so callers don't bake firmware-specific
// rules into generic code.

struct ManualControlPolicy {
    bool   supportsManualControl     = false; // disabled by default — Phase 4
    bool   allowsRcChannelsOverride  = false; // SAFETY: never enable for humans
    double maxStreamRateHz           = 50.0;
};

struct MissionFramePolicy {
    bool useMissionItemInt           = true;  // prefer integer lat/lon
    bool homeIsMissionSeq0           = false; // ArduPilot=true, PX4=false
};

// All firmware-specific quirks live behind this interface. Mode encoding,
// mission frame rules, manual-control policy, and pre-arm message prefixes
// belong here — NOT in QML, NOT in generic protocol code.
class FirmwarePlugin : public QObject
{
    Q_OBJECT
public:
    explicit FirmwarePlugin(QObject* parent = nullptr) : QObject(parent) {}
    ~FirmwarePlugin() override = default;

    virtual QString firmwareName() const = 0;

    // Mode encode/decode. Phase 1 returns placeholders; real decode in Phase 3+.
    virtual QString decodeFlightMode(uint8_t baseMode, uint32_t customMode) const = 0;
    virtual bool    encodeFlightMode(const QString& mode,
                                     uint8_t& baseMode,
                                     uint32_t& customMode) const = 0;

    virtual MissionFramePolicy  missionFramePolicy() const = 0;
    virtual ManualControlPolicy manualControlPolicy() const = 0;

    // Phase 3+: list of MAV_CMD ids the firmware accepts in a mission.
    virtual QList<int> supportedMissionCommands() const = 0;

    // Prefix used by the firmware on pre-arm STATUSTEXT messages.
    virtual QString prearmTextPrefix() const = 0;
};

} // namespace gcs::firmware
