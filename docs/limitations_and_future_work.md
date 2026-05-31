# Limitations and Future Work

This document is intentionally explicit. The current MVP is safe because many
flight-critical features are not implemented.

## Current Limitations

### Real Flight

Real flight is not supported. The app must not be used to fly a real UAV.

Disabled:

- Arm/disarm.
- Takeoff.
- Land.
- RTL.
- Mission start.
- Mode change.
- Force-arm.
- RC override.
- Real-hardware mission upload/download.
- Real-hardware manual control.

### MAVLink Coverage

The current MAVLink layer supports only the messages needed for the MVP. It is
not a full MAVLink stack.

Limitations:

- MAVLink `c_library_v2` is not yet the production parser/encoder.
- Message coverage is limited.
- Advanced signing/security is not implemented.
- Parameter protocol is not implemented.
- Full command ACK handling for real commands is not implemented.

### Mission Planning

Implemented mission planning is a basic waypoint workflow.

Not implemented:

- Survey/grid complex items.
- Terrain following.
- Geofence.
- Rally points.
- Camera trigger planning.
- Mission start.
- Mission clear-all.
- Hardware mission transfer.

### Manual Control

Manual control is a framework, not real flight control.

Limitations:

- Real joystick hardware backends such as SDL are not implemented.
- UDP SITL manual control uses MAVLink `MANUAL_CONTROL`, but only with the
  mock joystick backend.
- No real-hardware sink exists.
- Buttons are not mapped to commands.

### Logging and Replay

Structured event logging exists. Full telemetry replay does not.

Not implemented:

- Raw `.tlog` recording.
- `TlogReader`.
- `TlogReplayLink`.
- Replay UI.
- PX4 `.ulg` or ArduPilot `.bin` analysis.

### UI

The UI is functional for MVP demonstration, not a full production GCS.

Not implemented:

- Full map integration.
- Offline map tiles.
- Parameter editor.
- Multi-vehicle operations UI.
- Camera/gimbal controls.
- Plugin marketplace.
- Scripting console.
- Cloud/fleet features.

## Future Work Roadmap

Recommended sequence:

1. Integrate MAVLink `c_library_v2` and broaden protocol tests.
2. Add raw telemetry logging and replay as a read-only diagnostic path.
3. Add stronger PX4 and ArduPilot SITL smoke scripts.
4. Add real joystick backend while keeping output limited to SITL.
5. Design and test a command queue for real command ACK/retry/backoff.
6. Add operator confirmations and emergency procedures.
7. Add HITL command testing with propellers removed.
8. Only then consider a limited real-flight phase with a safety pilot.

## Safety Conditions for Future Real Commands

Before any real command is enabled, the project must have:

- A documented design review.
- MAVLink `c_library_v2` integration.
- A command queue with tests for ACK, retry, timeout, and rejection.
- SafetyGate tests for allowed and blocked cases.
- Operator confirmation UI.
- Hardware bench tests with propellers removed.
- SITL and HITL test evidence.
- An emergency-stop and safety-pilot procedure.

No future phase should weaken the current blocked paths silently.

