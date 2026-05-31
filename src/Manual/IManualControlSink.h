#pragma once

#include <QString>
#include <cstdint>

namespace gcs::manual {

// Where a ManualControlManager delivers each MANUAL_CONTROL sample.
// Concrete sinks in this build:
//   1. MockVehicle (in src/Simulation) stores the latest sample for tests
//      and UI display.
//   2. MavlinkManualControlSink emits MAVLink MANUAL_CONTROL for UDP/TCP
//      SITL only.
//   3. SitlStubManualControlSink is a fallback logger for unsupported
//      development transports.
//
// SAFETY: A serial/real-hardware sink does NOT exist in this build. Adding
// one must coincide with full SafetyGate + watchdog plumbing and a security
// review.
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
