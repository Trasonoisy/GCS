# ADR-004: All risky actions route through SafetyGate

## Status
Accepted (`SafetyGate` itself ships in Phase 4 alongside manual control).

## Context
A GCS that can arm or take off a drone has the blast radius of a flight
controller bug. We need one auditable place to reason about whether an
action is safe.

## Decision
A single `SafetyGate` service owns the allow/block decision for every
risky operation: arm, disarm in flight, takeoff, landing, RTL, mission
upload, manual-control engagement, mode change.

ViewModels call `SafetyGate.canX(state, params)`, receive a
`SafetyDecision { allowed, reason, warnings, requiresConfirmation,
confirmationText }`, and surface it in the UI. The UI never bypasses the
gate to send a MAVLink command directly.

Default behavior:
- unknown state -> block
- stale state -> block
- real-vehicle dangerous action -> block unless explicitly implemented
  and tested
- simulation -> allow only test-safe actions

## Consequences
- Single audit point for safety reviews and CR checklists.
- Easier to add new firmware or new commands without re-deriving safety
  logic in three places.
- Slight indirection cost — worth it.
