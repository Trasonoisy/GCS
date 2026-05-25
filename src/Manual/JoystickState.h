#pragma once

#include <QString>
#include <QtGlobal>

namespace gcs::manual {

// Snapshot of a joystick's normalized state. All axis fields are in
// [-1, +1] after axis processing. The joystick backend is responsible for
// normalization, deadzone, expo, and inversion before producing this struct.
//
// MAVLink MANUAL_CONTROL packing happens later (in ManualControlManager).
struct JoystickState
{
    bool    connected = false;
    QString name;

    double  pitch    = 0.0; // [-1, +1]
    double  roll     = 0.0; // [-1, +1]
    double  throttle = 0.0; // [-1, +1] — packed to MAVLink z [0, 1000]
    double  yaw      = 0.0; // [-1, +1]

    quint16 buttons  = 0;   // bitmask — Phase 4 does NOT bind any to actions

    qint64  lastUpdatedUtcMs = 0;
};

} // namespace gcs::manual
