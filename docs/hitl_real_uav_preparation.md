# HITL / Real UAV Preparation

This document is the entry point for connecting Lab GCS to a real flight
controller. The current MVP supports hardware telemetry only.

Do not connect propellers. Do not attempt real flight with this build.

## Current Hardware Scope

| Capability | Status |
|---|---|
| Read MAVLink telemetry over serial | Enabled when Qt SerialPort is available. |
| Display hardware telemetry in Fly view | Enabled. |
| Hardware read-only banner | Enabled. |
| Structured logging of hardware connection events | Enabled. |
| Serial outbound bytes | Blocked by `SerialLink::writeBytes`. |
| Hardware mission upload/download | Blocked. Serial vehicles receive no `MissionManager`. |
| Hardware manual control | Blocked by SafetyGate. |
| Arm/takeoff/land/RTL/mode change | Not implemented. |
| Force-arm | Not implemented. |
| RC override | Not implemented. |

## Enabling Qt SerialPort

`SerialLink` has two build modes:

- Real backend: `Qt6::SerialPort` was found during CMake configure.
- Stub backend: Qt SerialPort was not found; serial connection attempts report
  a clear error and remain disconnected.

Configure example:

```powershell
& 'C:\QtOnline\Tools\CMake_64\bin\cmake.exe' -S . -B build -G Ninja `
  -DCMAKE_BUILD_TYPE=Debug `
  -DCMAKE_PREFIX_PATH='C:/QtOnline/6.11.1/mingw_64' `
  -DCMAKE_CXX_COMPILER='C:/QtOnline/Tools/mingw1310_64/bin/g++.exe'
```

CMake prints one of:

```text
Qt6::SerialPort found - SerialLink will use the real backend
Qt6::SerialPort NOT found - SerialLink will compile as a stub
```

## Bench Checklist

Do these every time before connecting a real flight controller:

1. Remove all propellers.
2. Keep the vehicle restrained on the bench.
3. Use USB or telemetry radio only for MAVLink telemetry.
4. Close other GCS programs that may hold the same serial port.
5. Identify the port (`COMx`, `/dev/ttyACM0`, or `/dev/ttyUSB0`).
6. Choose a matching baud rate.
7. Confirm this repository builds and tests pass.
8. Confirm `docs/safety_audit.md` still states no real command path is enabled.

## Connect Procedure

1. Build and run the app.
2. Open Fly.
3. In Connection, select Serial.
4. Refresh ports.
5. Select the flight-controller port and baud rate.
6. Click connect.
7. Confirm the red `HARDWARE READ-ONLY` banner appears.
8. Wait for heartbeat and telemetry.

Expected telemetry:

- Autopilot: PX4 or ArduPilot when recognized.
- Vehicle type from heartbeat.
- Flight mode from the firmware plugin.
- Position, attitude, battery, GPS, and status text when emitted.

## Required Read-Only Verification

Before any lab demonstration with hardware:

- Confirm the mission upload/download buttons are disabled for serial hardware.
- Confirm the Manual tab cannot activate hardware manual control.
- Confirm the event log records hardware read-only connection.
- Confirm no test has been modified to allow serial writes.

Relevant tests:

- `tst_serial_link`
- `tst_safety_gate_hardware_mode`
- `tst_safety_gate`

Run:

```powershell
$env:Path = 'C:\QtOnline\6.11.1\mingw_64\bin;C:\QtOnline\Tools\mingw1310_64\bin;' + $env:Path
& 'C:\QtOnline\Tools\CMake_64\bin\ctest.exe' --test-dir build --output-on-failure
```

## What Not To Do

Do not:

- Install propellers.
- Arm the vehicle.
- Try to start a mission.
- Try to upload a mission to hardware.
- Try to use joystick/manual control on hardware.
- Add temporary command-sending code for a demo.
- Use `RC_CHANNELS_OVERRIDE`.
- Treat UDP hardware as safe just because it is not serial. The current lab
  safety rule is UDP/TCP for SITL and serial for hardware.

## Before Any Future Real-Command Phase

A future phase that enables real commands must first add:

- MAVLink `c_library_v2` integration.
- A reviewed command queue with ACK/retry/backoff.
- Explicit SafetyGate rules for each action.
- Operator confirmation UI.
- Emergency procedure and safety-pilot procedure.
- HITL tests with propellers removed.
- Separate tests proving unsafe paths remain blocked.

Until that work is complete, hardware mode remains telemetry-only.

