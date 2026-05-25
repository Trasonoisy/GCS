#pragma once

#include <QString>
#include <cstdint>

namespace gcs::manual {

// Where a ManualControlManager delivers each MANUAL_CONTROL sample. Two
// concrete sinks exist in Phase 4:
//   1. MockManualControlSink (in src/Simulation) — pushes the sample into
//      MockVehicle's last-sample slot for tests and UI display.
//   2. SitlStubManualControlSink (defined inline below) — logs the values
//      without writing any bytes. The brief explicitly permits this for
//      Phase 4; the wire-level MAVLink path is intentionally NOT enabled.
//
// SAFETY: A real-hardware sink does NOT exist in this build. Adding one
// must coincide with full SafetyGate + watchdog plumbing and a security
// review (Phase 5+).
class IManualControlSink
{
public:
    virtual ~IManualControlSink() = default;

    // Each axis: x/y/r in [-1000, +1000], z in [0, 1000]. buttons is a
    // bitmask; sinks must NOT decode it into actions.
    virtual void onManualControlSample(int16_t x, int16_t y,
                                       int16_t z, int16_t r,
                                       uint16_t buttons) = 0;

    virtual QString sinkName() const = 0;
    virtual bool    isSimulated() const = 0;
};

} // namespace gcs::manual
