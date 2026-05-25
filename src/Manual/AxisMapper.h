#pragma once

#include <cstdint>

#include "AxisConfig.h"

namespace gcs::manual {

// All axis math lives here as free functions so it stays unit-testable and
// has no QObject overhead. The contracts are explicit:
//
//   processAxis: raw [-1, +1] in -> processed [-1, +1] out
//   packAxis:    [-1, +1] -> [-1000, +1000]   (pitch / roll / yaw)
//   packThrottle:[-1, +1] -> [0, 1000]         (throttle z)
//
// All outputs are clamped. NaN in -> 0 out (safe).
namespace axis {

double processAxis  (double raw, const AxisConfig& cfg);
int16_t packAxis    (double processed);
int16_t packThrottle(double processedThrottle);

} // namespace axis

} // namespace gcs::manual
