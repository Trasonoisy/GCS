# SafetyGate Audit Summary

Date: 2026-05-31

Scope: Phase 8 integration testing plus Phase 4 UDP SITL manual-control
completion. SITL manual-control samples are now transmitted as MAVLink
`MANUAL_CONTROL`; real-hardware control paths remain disabled.

## Result

PASS. The project builds, the app starts, SITL/manual tests pass, and no real
hardware command path is enabled.

## Verified Phase Coverage

| Area | Status |
|---|---|
| Simulation Mode | `MockVehicle`, `MockLink`, active mock vehicle, and UI simulation banner remain wired. |
| PX4 SITL telemetry | UDP listener, MAVLink parser, router, PX4 firmware plugin, and telemetry tests pass. |
| ArduPilot SITL telemetry | ArduPilot firmware plugin selection and mode/airframe tests pass. |
| Mission Planning | Mission model, validator, `.plan` save/load, and PlanView actions remain intact. |
| Mock mission upload/download | `MockMissionLink` and mission manager upload/download tests pass. |
| SITL mission upload/download | `MavlinkMissionLink` supports mission upload/download over UDP/TCP SITL only. |
| Manual Control framework | Mock joystick, axis mapping, manager watchdog, SafetyGate checks, mock sink, and UDP SITL MAVLink sink pass. |
| SafetyGate | Blocks real-hardware mission/manual paths and all unimplemented dangerous actions. |
| Logging | Memory/file/event/operator logging tests pass; default file path now falls back safely. |
| Hardware Read-Only mode | Serial opens read-only, shows hardware banner, and refuses outbound bytes. |

## Blocked Command Paths

| Command/action | Current path | Block behavior |
|---|---|---|
| Arm | `SafetyGate::canArm` | Always blocked: not implemented for real vehicle in this phase. |
| Force-arm | No command path exists. No `COMMAND_LONG` arm/disarm sender exists. | Blocked by absence of sender; any future force-arm must route through SafetyGate and remain blocked until reviewed. |
| Takeoff | `SafetyGate::canTakeoff` | Always blocked: not implemented for real vehicle in this phase. Mission-item TAKEOFF remains data only. |
| Land | No immediate land command path exists. Mission-item LAND remains data only. | Blocked by absence of sender; mission upload does not switch mode, start mission, or command landing. |
| RTL | `SafetyGate::canTriggerRTL` | Always blocked: not implemented for real vehicle in this phase. Mission-item RTL remains data only. |
| Mission start | No mission-start command path exists. | Blocked by absence of sender; mission upload/download transfers data only. |
| RC override | No `RC_CHANNELS_OVERRIDE` path exists. | Blocked by absence of implementation; joystick buttons are not mapped to actions. |
| Real hardware mission upload | `MissionViewModel::uploadToVehicle` plus `SafetyGate::canUploadMission`; serial vehicles receive no `MissionManager`. | Blocked on `LinkKind::Serial`; QML upload button disabled as an additional guard. |
| Real hardware mission download | `MissionViewModel::downloadFromVehicle`; serial vehicles receive no `MissionManager`. | Blocked on `LinkKind::Serial`; QML download button disabled as an additional guard. |
| Real hardware manual control | `ManualControlManager` calls `SafetyGate::canStartManualControl` and `canContinueManualControl`. | Blocked on `LinkKind::Serial`; no real-hardware manual sink exists. |
| MAVLink MANUAL_CONTROL to SITL | `ManualControlManager` uses `MavlinkManualControlSink` for UDP SITL. | Allowed only for SITL network links after SafetyGate permits the session. |

## Audit Notes

- `SerialLink::writeBytes` refuses every outbound byte and emits an error.
- `UdpLink::writeBytes` is used by `MavlinkMissionLink` for SITL mission
  protocol and by `MavlinkManualControlSink` for SITL manual samples after
  SafetyGate approval.
- `MavlinkMissionLink` sends only mission protocol frames:
  `MISSION_COUNT`, `MISSION_ITEM_INT`, `MISSION_REQUEST_LIST`,
  `MISSION_REQUEST_INT`, and `MISSION_ACK`.
- No `COMMAND_LONG`, arming, takeoff, landing, RTL, force-arm, mission-start,
  or `RC_CHANNELS_OVERRIDE` sender was found in `src`, `tests`, or `qml`.
- Manual control remains either simulated (`MockVehicle`) or UDP SITL-only in
  the current UI (`MavlinkManualControlSink` refuses non-network links);
  serial hardware is blocked by SafetyGate and receives no sink.

## Verification

Build:

```powershell
& 'C:\QtOnline\Tools\CMake_64\bin\cmake.exe' --build build
```

Targeted tests:

```powershell
$env:Path = 'C:\QtOnline\6.11.1\mingw_64\bin;C:\QtOnline\Tools\mingw1310_64\bin;' + $env:Path
.\build\tst_axis_mapper.exe
.\build\tst_manual_control_manager.exe
.\build\tst_safety_gate.exe
.\build\tst_safety_gate_hardware_mode.exe
.\build\tst_mavlink_manual_control_sink.exe
.\build\tst_mavlink_parser.exe
.\build\tst_mavlink_mission_codec.exe
.\build\tst_mavlink_mission_link.exe
.\build\tst_safety_gate_mission_sitl.exe
```

Result: targeted tests passed.

App launch smoke test:

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
.\build\LabGCSApp.exe
```

Result: app started and stayed alive until terminated after 5 seconds. The only
stderr output was a Qt font-directory warning.

## Fixes Made During Audit

- `FileLogSink::defaultLogDirectory` now tries app-local data, home, and temp
  log roots before returning empty, so logging remains available in restricted
  test/runtime environments.
- `ManualControlManager` checklist now matches SafetyGate behavior for
  ArduPilot SITL over TCP/UDP instead of showing a false checklist failure.

