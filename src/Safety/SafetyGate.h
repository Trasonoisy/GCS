#pragma once

#include "SafetyDecision.h"

namespace gcs::vehicle  { struct VehicleState; }
namespace gcs::mission  { struct MissionPlan;  }
namespace gcs::manual   { struct JoystickState; }

namespace gcs::safety {

// Central allow/block service for every risky operator action. Stateless and
// const — each method takes the current state of the world by reference and
// returns a SafetyDecision. The brief makes this the *only* path for
// dangerous actions; UI must call here before invoking command queues.
//
// Phase 4 implements the manual-control checks. Arm / takeoff / RTL /
// mode-change / mission-upload checks are stubbed and return blocked. They
// will be filled in as the corresponding command paths come online — and
// callers wired through this gate at that time.
class SafetyGate
{
public:
    // How fresh a heartbeat must be to count as "alive" for manual control.
    static constexpr qint64 kFreshHeartbeatMs = 2500;

    // ---- Phase 4: live checks ---------------------------------------------

    // operatorOptedIn must be true (UI button held down or toggled on). The
    // brief is explicit: nothing happens unless the operator explicitly asks.
    SafetyDecision canStartManualControl(const gcs::vehicle::VehicleState& s,
                                         const gcs::manual::JoystickState& js,
                                         bool operatorOptedIn) const;

    // Per-tick check used by the manager's send loop to decide whether to
    // continue sending. Returns block on any failsafe condition.
    SafetyDecision canContinueManualControl(const gcs::vehicle::VehicleState& s,
                                            const gcs::manual::JoystickState& js) const;

    // ---- Future-phase stubs (return blocked) ------------------------------

    SafetyDecision canArm           (const gcs::vehicle::VehicleState& s) const;
    SafetyDecision canTakeoff       (const gcs::vehicle::VehicleState& s,
                                     double targetAltitudeM) const;
    SafetyDecision canTriggerRTL    (const gcs::vehicle::VehicleState& s) const;
    SafetyDecision canUploadMission (const gcs::vehicle::VehicleState& s,
                                     const gcs::mission::MissionPlan& plan) const;
};

} // namespace gcs::safety
