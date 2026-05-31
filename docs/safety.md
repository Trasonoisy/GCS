# Safety

Lab GCS is safety-first. The current MVP is for simulation, SITL, mission
workflow validation, telemetry monitoring, and university demonstration. It is
not approved for real-drone flight.

## Non-Negotiable Rules

1. QML must not parse, construct, or send raw MAVLink.
2. Dangerous actions must route through `SafetyGate`.
3. Mission upload must not arm, take off, change mode, start mission, land, or
   RTL.
4. Manual control must use the `MANUAL_CONTROL` model, never
   `RC_CHANNELS_OVERRIDE`.
5. Real hardware is read-only in this MVP.
6. Unknown or stale state must fail closed.
7. No real flight until mock, SITL, HITL, and code review gates pass.

## SafetyGate Posture

`SafetyGate` is the central allow/block service.

Active checks:

- `canUploadMission`
- `canStartManualControl`
- `canContinueManualControl`

Always-blocked checks in this MVP:

- `canArm`
- `canTakeoff`
- `canTriggerRTL`

`canUploadMission` allows:

- Mock vehicle mission transfer.
- PX4 SITL mission transfer over UDP/TCP.
- ArduPilot SITL mission transfer over UDP/TCP.

`canUploadMission` blocks:

- Empty missions.
- `LinkKind::Serial`.
- `LinkKind::Replay`.
- `LinkKind::Unknown`.
- Any non-SITL or unrecognized vehicle path.

Manual control is allowed only when:

- The operator explicitly enables it.
- There is an active vehicle.
- The link heartbeat is fresh.
- The joystick is connected.
- The vehicle is mock or recognized SITL.

Manual control is blocked or stopped on:

- Stale heartbeat.
- Link loss.
- Joystick disconnect.
- Active vehicle change.
- Operator disable.
- Serial hardware.
- Replay or unknown links.

## Hardware Read-Only Mode

Serial hardware support exists only for telemetry monitoring.

When a serial link is connected:

- The UI shows a red `HARDWARE READ-ONLY` banner.
- MAVLink telemetry can update the Fly view.
- `SerialLink::writeBytes` rejects every outbound byte.
- Serial vehicles receive no `MissionManager`.
- Mission upload/download buttons are disabled.
- Manual control is blocked by SafetyGate.

This is true even if the flight controller reports PX4 or ArduPilot. A real
Pixhawk and SITL can report the same autopilot identity, so the project uses
`VehicleState::linkKind` as the safety discriminator.

## Disabled Command Paths

The following are intentionally disabled:

| Action | Status |
|---|---|
| Arm / disarm real vehicle | No sender; SafetyGate arm path blocked. |
| Force-arm | No sender; no bypass path. |
| Takeoff | No sender; SafetyGate takeoff path blocked. |
| Land | No immediate command sender; mission item is data only. |
| RTL | No sender; SafetyGate RTL path blocked. |
| Mission start | No sender. |
| Mode change | No command path enabled. |
| RC override | No `RC_CHANNELS_OVERRIDE` implementation exists. |
| Real hardware mission upload | Blocked by missing serial mission manager and SafetyGate. |
| Real hardware manual control | Blocked by SafetyGate and absence of real hardware sink. |
| Serial/hardware MAVLink MANUAL_CONTROL transmission | Not implemented; only UDP/TCP SITL network links can receive manual samples. |

## Mission Safety

Mission items such as TAKEOFF, LAND, RTL, and LOITER are treated as mission
plan data. They are not immediate operator commands.

Mission upload/download supports only the mission protocol transfer itself:

- `MISSION_COUNT`
- `MISSION_REQUEST_INT`
- `MISSION_ITEM_INT`
- `MISSION_ACK`
- `MISSION_REQUEST_LIST`

The MVP deliberately does not implement `MISSION_CLEAR_ALL` because it can
erase an existing vehicle mission and there is no operator-confirmation surface
for that action.

## Manual Control Safety

Manual control has a state machine:

```text
Disabled -> WaitingForJoystick -> Ready -> Active
                             \-> Blocked / Failsafe
```

The manager checks SafetyGate before activation and before every sample. Mock
vehicle samples are consumed by `MockVehicle`. UDP SITL samples go to
`MavlinkManualControlSink`, which writes MAVLink `MANUAL_CONTROL` frames only
to network SITL links. Serial hardware gets no manual-control sink and is
blocked by SafetyGate.

Joystick buttons are stored only as a bitmask. They are not mapped to arm,
takeoff, mode changes, RTL, or any other dangerous command.

## Logging and Audit

Structured event logging is enabled through:

- `EventLogger`
- `MemoryLogSink`
- `FileLogSink`
- `OperatorActionLogger`

Logged categories include:

| Category | Examples |
|---|---|
| `App` | Startup and shutdown. |
| `Link` | UDP start/stop, serial connect/disconnect, link errors. |
| `Vehicle` | Vehicle detected, active vehicle changed, heartbeat and state transitions. |
| `Mission` | Validation, upload/download start, completion, rejection. |
| `Safety` | Manual-control blocked/failsafe states and pre-arm status text. |
| `OperatorAction` | Mission file actions, manual-control enable/disable intent. |

Logs are JSONL records. They are for operator-level audit and debugging, not a
complete wire trace.

## Known Safety Limitations

- The current MAVLink parser is a limited MVP parser and does not replace a
  full MAVLink `c_library_v2` integration for real flight.
- Replay through `TlogReplayLink` is not implemented.
- There is no emergency-stop command path because no real command path is
  enabled. Before any real command path is added, a separate emergency and
  safety-pilot procedure is required.

