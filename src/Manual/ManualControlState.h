#pragma once

#include <QObject>

namespace gcs::manual {

Q_NAMESPACE

// State machine values per the brief's Section 12. We omit "Calibrating"
// because Phase 4 has no joystick-calibration UI yet.
enum class ManualControlState
{
    Disabled,           // operator has not enabled manual control
    WaitingForJoystick, // enabled but no joystick is connected
    Ready,              // enabled, joystick present, gate would allow
    Active,             // send loop is running
    Blocked,            // gate refused (e.g. wrong autopilot type)
    Failsafe,           // active session was force-stopped by a watchdog
};
Q_ENUM_NS(ManualControlState)

inline const char* manualControlStateName(ManualControlState s)
{
    switch (s) {
        case ManualControlState::Disabled:           return "Disabled";
        case ManualControlState::WaitingForJoystick: return "WaitingForJoystick";
        case ManualControlState::Ready:              return "Ready";
        case ManualControlState::Active:             return "Active";
        case ManualControlState::Blocked:            return "Blocked";
        case ManualControlState::Failsafe:           return "Failsafe";
    }
    return "Unknown";
}

} // namespace gcs::manual
