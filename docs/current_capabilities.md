# Current Capabilities

This page lists what the current GCS MVP can actually do.

## Application

- Native desktop Qt/QML application.
- Fly, Plan, and Manual views.
- Connection panel for mock/default state, UDP SITL, and serial read-only
  hardware telemetry.
- View models expose backend state to QML through Qt properties.
- QML does not parse or send MAVLink.

## Simulation Mode

- `MockVehicle` starts automatically.
- Simulated heartbeat, position, attitude, heading, ground speed, battery, GPS,
  flight mode, and event messages update the UI.
- `SIMULATION MODE` banner appears for the mock vehicle.
- Mock mission upload/download works through `MockMissionLink`.
- Mock manual-control samples are consumed by `MockVehicle` for UI/test
  feedback.

## PX4 SITL

- UDP telemetry input.
- Supported telemetry messages include heartbeat, system status, GPS raw,
  attitude, global position, VFR HUD, battery status, and status text.
- PX4 flight modes are decoded by `PX4FirmwarePlugin`.
- SITL mission upload/download uses `MavlinkMissionLink`.
- Mission transfer is data-only and does not start or control the vehicle.
- Manual-control samples can be sent as MAVLink `MANUAL_CONTROL` over UDP SITL
  when SafetyGate allows the session.

## ArduPilot SITL

- UDP telemetry input.
- ArduPilot airframe selection through `FirmwarePluginManager`.
- Supported plugin classes include ArduCopter, ArduPlane, ArduRover, and
  ArduSub.
- ArduPilot mode names are decoded by firmware-specific plugins.
- SITL mission upload/download uses the same mission manager and SafetyGate
  policy as PX4 SITL.
- UDP SITL manual-control samples use the same `MANUAL_CONTROL` sink path.

## Mission Planning

- Add, edit, delete, and reorder mission waypoints.
- Validate mission items.
- Save and load a QGC-compatible `.plan` subset.
- Upload/download to mock vehicle.
- Upload/download to PX4/ArduPilot SITL over supported SITL links.
- Timeout/retry handling and concurrent-transfer rejection in the mission
  state machines.

## Manual Control Framework

- Mock joystick backend.
- Axis normalization, deadzone, expo, inversion, and packing.
- Manual-control state machine and watchdog.
- SafetyGate checks before activation and before each sample.
- Mock vehicle sink.
- UDP SITL MAVLink `MANUAL_CONTROL` sink.
- Stub sink only for unsupported development transports.

No serial/real-hardware `MANUAL_CONTROL` packet is transmitted in this MVP.

## SafetyGate

- Blocks stale, disconnected, unknown, replay, and serial-hardware conditions.
- Allows only mock and recognized SITL paths for mission/manual framework use.
- Always blocks arm, takeoff, and RTL in this MVP.
- Blocks real-hardware mission upload and manual control.

## Logging

- In-memory event log for UI display.
- JSONL file logging when a writable log directory is available.
- Operator action logging for mission file actions and manual-control intent.
- Safety event logging for blocked/failsafe manual-control states.
- Link and vehicle event logging.

## Hardware Read-Only Mode

- Serial telemetry can be opened when Qt SerialPort is available.
- UI displays a hardware read-only banner.
- Incoming MAVLink updates vehicle telemetry.
- Outbound serial writes are rejected.
- Hardware mission and manual-control paths are blocked.

