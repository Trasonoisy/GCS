# Development Plan

This plan follows the project brief and records the current MVP status.

## Phase Status

| Phase | Goal | Current status |
|---|---|---|
| 0 | Project foundation | Complete. Qt 6 / CMake app, folders, docs, tests. |
| 1 | Mock vehicle and basic UI | Complete. Mock telemetry appears in Fly view. |
| 2 | Mission planning against mock | Complete. Mission model, validation, `.plan` I/O, mock upload/download. |
| 3 | PX4 SITL telemetry | Complete. UDP listener, MAVLink telemetry decode, PX4 mode names. |
| 4 | Manual control and SafetyGate | Complete for mock and UDP SITL. Sends MAVLink `MANUAL_CONTROL` to SITL only; hardware remains blocked/read-only. |
| 5 | ArduPilot SITL parity | Complete. ArduPilot heartbeat, airframe plugins, mode naming. |
| 6 | Logging | Complete. Structured JSONL event logging. Replay not implemented. |
| 7 | HITL preparation | Complete as read-only telemetry. Serial writes blocked. |
| 8 | SITL mission upload/download | Complete. MAVLink mission protocol for PX4/ArduPilot SITL only. |

## Current MVP Definition

The current MVP demonstrates:

- Desktop Qt/QML/C++ app shell.
- Simulation-first vehicle state and UI.
- MAVLink telemetry decode for PX4 and ArduPilot SITL.
- Mission planning, validation, save/load, upload, and download.
- Mock mission transfer.
- SITL mission upload/download through MAVLink mission protocol.
- Manual-control framework with joystick normalization, watchdog, and UDP SITL
  MAVLink `MANUAL_CONTROL` output.
- SafetyGate enforcement.
- Structured logging.
- Hardware read-only serial telemetry.

## What Counts as Complete

The MVP is complete when:

- The app builds.
- The app starts and shows the mock vehicle without external dependencies.
- All tests pass.
- Mission planning works against mock and SITL where configured.
- SafetyGate blocks all real-hardware command paths.
- Documentation states current capabilities and limitations accurately.

The safety audit for this state is in [safety_audit.md](safety_audit.md).

## Next Development Direction

Future work must remain simulation-first and test-first. Recommended order:

1. Replace the limited MAVLink frame support with MAVLink `c_library_v2`.
2. Add raw telemetry logging/replay only as a read-only diagnostic path.
3. Add stronger SITL integration tests and scripted demo helpers.
4. Add a real command queue only after explicit design review.
5. Add HITL command testing with propellers removed only after command queue,
   SafetyGate, emergency procedures, and tests are complete.
6. Consider real flight only after SITL and HITL sign-off with a safety pilot.

## Work That Must Not Be Added Casually

These items require a separate phase, design review, and tests:

- Arm/disarm.
- Takeoff.
- Land.
- RTL.
- Mission start.
- Mode change.
- Force-arm.
- Real-hardware mission upload.
- Real-hardware manual control.
- `RC_CHANNELS_OVERRIDE`.

## Out of Scope for Current MVP

- Full parameter editor.
- Advanced survey/grid mission items.
- Terrain following.
- Geofence/rally editor.
- Offline maps.
- Camera/gimbal control.
- Multi-vehicle operations UI.
- Cloud/fleet features.
- Plugin marketplace.
- Scripting console.
- Real flight.

