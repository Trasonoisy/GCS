#include "SafetyGate.h"

#include <QDateTime>

#include "Manual/JoystickState.h"
#include "Mission/MissionPlan.h"
#include "Vehicle/VehicleState.h"

namespace gcs::safety {

using gcs::vehicle::LinkStatus;
using gcs::vehicle::VehicleState;
using gcs::manual::JoystickState;

namespace {

bool linkHealthy(const VehicleState& s)
{
    if (s.linkStatus != LinkStatus::Connected) return false;
    if (s.lastHeartbeatUtcMs <= 0) return false;
    const qint64 age = QDateTime::currentMSecsSinceEpoch() - s.lastHeartbeatUtcMs;
    return age >= 0 && age <= SafetyGate::kFreshHeartbeatMs;
}

bool simulationOrSitlAllowed(const VehicleState& s)
{
    // SAFETY (Phase 7): allow manual control / mission upload only when the
    // vehicle's transport guarantees we are NOT talking to real hardware.
    //
    //   - LinkKind::Mock           always allowed (MockVehicle is offline).
    //   - LinkKind::Udp / Tcp      allowed iff autopilot is PX4 / ArduPilot.
    //                              These are the SITL transports; real
    //                              autopilots over the same UDP/TCP would
    //                              still register here, but the lab posture
    //                              is that we use UDP/TCP for SITL and
    //                              serial for real hardware. If your wiring
    //                              changes, tighten this rule.
    //   - LinkKind::Serial         BLOCKED. A real Pixhawk reports
    //                              autopilot=PX4 identically to PX4 SITL -
    //                              the LinkKind is the only reliable
    //                              discriminator at the protocol layer.
    //   - LinkKind::Replay/Unknown BLOCKED, fail-closed by default.
    if (s.simulated) return true;
    switch (s.linkKind) {
        case gcs::comms::LinkKind::Mock:
            return true;
        case gcs::comms::LinkKind::Udp:
        case gcs::comms::LinkKind::Tcp: {
            const auto& a = s.autopilotType;
            return a.compare(QStringLiteral("PX4"),       Qt::CaseInsensitive) == 0
                || a.compare(QStringLiteral("ArduPilot"), Qt::CaseInsensitive) == 0;
        }
        case gcs::comms::LinkKind::Serial:
        case gcs::comms::LinkKind::Replay:
        case gcs::comms::LinkKind::Unknown:
            return false;
    }
    return false;
}

} // namespace

// ============================================================
// Phase 4: live checks
// ============================================================

SafetyDecision SafetyGate::canStartManualControl(
    const VehicleState& s,
    const JoystickState& js,
    bool operatorOptedIn) const
{
    if (!operatorOptedIn) {
        return SafetyDecision::block(QStringLiteral(
            "Blocked: Operator has not enabled manual control."));
    }
    if (s.systemId <= 0) {
        return SafetyDecision::block(QStringLiteral(
            "Blocked: no active vehicle is selected."));
    }
    if (!linkHealthy(s)) {
        return SafetyDecision::block(QStringLiteral(
            "Blocked: link is disconnected or the heartbeat is stale."));
    }
    if (!js.connected) {
        return SafetyDecision::block(QStringLiteral(
            "Blocked: Mock Joystick is not connected."));
    }
    if (!simulationOrSitlAllowed(s)) {
        return SafetyDecision::block(QStringLiteral(
            "Blocked: Real-hardware manual control is disabled; serial hardware is read-only."));
    }
    return SafetyDecision::allow();
}

SafetyDecision SafetyGate::canContinueManualControl(
    const VehicleState& s,
    const JoystickState& js) const
{
    // Same predicates as canStartManualControl, minus the operator-opt-in
    // check (the manager remembers that the operator opted in once).
    if (s.systemId <= 0) {
        return SafetyDecision::block(QStringLiteral("Blocked: active vehicle changed."));
    }
    if (!linkHealthy(s)) {
        return SafetyDecision::block(QStringLiteral("Blocked: heartbeat is stale or link was lost."));
    }
    if (!js.connected) {
        return SafetyDecision::block(QStringLiteral("Blocked: mock joystick disconnected."));
    }
    if (!simulationOrSitlAllowed(s)) {
        return SafetyDecision::block(QStringLiteral(
            "Blocked: Real-hardware manual control is disabled; serial hardware is read-only."));
    }
    return SafetyDecision::allow();
}

// ============================================================
// Future-phase stubs - every method returns blocked. SAFETY: do not soften
// these without simultaneously wiring the corresponding command-queue path.
// ============================================================

SafetyDecision SafetyGate::canArm(const VehicleState&) const
{
    return SafetyDecision::block(QStringLiteral(
        "Blocked: arm is not implemented for real vehicle in this phase."));
}

SafetyDecision SafetyGate::canTakeoff(const VehicleState&, double) const
{
    return SafetyDecision::block(QStringLiteral(
        "Blocked: takeoff is not implemented for real vehicle in this phase."));
}

SafetyDecision SafetyGate::canTriggerRTL(const VehicleState&) const
{
    return SafetyDecision::block(QStringLiteral(
        "Blocked: RTL is not implemented for real vehicle in this phase."));
}

SafetyDecision SafetyGate::canUploadMission(
    const VehicleState& s,
    const gcs::mission::MissionPlan& plan) const
{
    if (plan.items.isEmpty()) {
        return SafetyDecision::block(QStringLiteral("Blocked: mission is empty."));
    }
    if (!linkHealthy(s)) {
        return SafetyDecision::block(QStringLiteral(
            "Blocked: mission upload requires a connected link with a fresh heartbeat."));
    }
    // Phase 2 mission upload runs against MockMissionLink only. Phase 7
    // additionally blocks any vehicle on a hardware transport (LinkKind ==
    // Serial) - even in simulated-flag mode this MUST NOT auto-trigger a
    // real upload path. We mirror the manual-control predicate.
    if (s.linkKind == gcs::comms::LinkKind::Serial) {
        return SafetyDecision::block(QStringLiteral(
            "Blocked: mission upload to real-hardware serial link is disabled; hardware mode is read-only."));
    }
    if (simulationOrSitlAllowed(s)) {
        // Only the mock vehicle has a MissionManager wired today, so even
        // the SITL-allowed branch will be ignored by the real call path -
        // but we keep the gate result accurate so future SITL-mission
        // wiring drops in without re-deriving safety.
        return SafetyDecision::allow();
    }
    return SafetyDecision::block(QStringLiteral(
        "Blocked: mission upload is allowed only for MockVehicle or PX4/ArduPilot SITL in this MVP."));
}

} // namespace gcs::safety
