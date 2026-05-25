#include "AxisMapper.h"

#include <algorithm>
#include <cmath>

namespace gcs::manual::axis {

namespace {
double clamp01(double v) { return std::clamp(v, -1.0, 1.0); }
} // namespace

double processAxis(double raw, const AxisConfig& cfg)
{
    if (std::isnan(raw)) return 0.0;
    if (cfg.inverted)    raw = -raw;
    raw = clamp01(raw);

    const double dz = std::clamp(cfg.deadzone, 0.0, 0.95);
    double mag = std::fabs(raw);
    if (mag <= dz) return 0.0;

    // Rescale (dz .. 1) -> (0 .. 1).
    mag = (mag - dz) / (1.0 - dz);

    // Cubic expo blend in [0, 1] of expo factor.
    const double e = std::clamp(cfg.expo, 0.0, 1.0);
    mag = (1.0 - e) * mag + e * mag * mag * mag;

    return std::copysign(mag, raw);
}

int16_t packAxis(double processed)
{
    if (std::isnan(processed)) return 0;
    const double v = clamp01(processed) * 1000.0;
    return static_cast<int16_t>(std::lround(v));
}

int16_t packThrottle(double processedThrottle)
{
    if (std::isnan(processedThrottle)) return 0;
    // [-1, +1] -> [0, 1000]
    const double v = (clamp01(processedThrottle) + 1.0) * 500.0;
    return static_cast<int16_t>(std::lround(std::clamp(v, 0.0, 1000.0)));
}

} // namespace gcs::manual::axis
