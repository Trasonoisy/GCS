# ADR-003: Simulation-first development

## Status
Accepted.

## Context
Real-drone testing is expensive, slow, and risky for students. Architectural
mistakes caught only on a real airframe cost flight hardware.

## Decision
Every feature must work end-to-end against `MockVehicle` first, then PX4
SITL, then ArduPilot SITL, then HITL with propellers removed, before any
real flight.

The `LinkInterface` abstraction lets `MockLink`, `UdpLink`, `SerialLink`,
and `TlogReplayLink` share one contract so test setups are interchangeable.

## Consequences
- Higher upfront investment in `MockVehicle` and `MockLink`.
- Bugs surface in CI rather than at the airfield.
- New contributors can develop with no hardware at all.
