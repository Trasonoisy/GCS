#pragma once

namespace gcs::manual {

// Per-axis processing configuration applied by AxisMapper before producing
// the normalized JoystickState. Defaults are conservative.
struct AxisConfig
{
    double deadzone = 0.05; // 5 % around centre
    double expo     = 0.30; // y = (1-e) x + e x^3   in [0, 1]
    bool   inverted = false;
};

} // namespace gcs::manual
