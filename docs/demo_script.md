# Demo Script

This script is for a short university MVP demonstration. It intentionally
shows safe behavior and blocked real-drone paths.

## Scenario 1: Simulation Mode Demo

Goal: prove the app works without a drone or SITL.

Steps:

1. Build and start the app.
2. Open the Fly tab.
3. Point out the `SIMULATION MODE` banner and top mode badge.
4. Show the active vehicle, link status, heartbeat age, battery, GPS, attitude,
   position, heading, and ground speed changing from `MockVehicle`.
5. Open the Plan tab.
6. Add three waypoints.
7. Edit latitude, longitude, altitude, and command type.
8. Run mission validation.
9. Save the plan, then load it again.
10. Upload to `MockVehicle`, then download from `MockVehicle`.
11. Open the Manual tab.
12. Connect the mock joystick, enable manual control, move sliders, and show
    samples delivered to the simulation sink.
13. Disable manual control.
14. Return to Fly and show audit events in the Event Log.

Expected talking points:

- The app starts with no external dependency.
- Mission upload is a mock mission transaction, not a real flight command.
- Manual control is safety-gated and simulation-only in this scenario.
- No real drone is connected.

## Scenario 2: PX4 or ArduPilot SITL Telemetry + Mission Planning Demo

Goal: show MAVLink telemetry and SITL mission transfer.

Preparation:

1. Start PX4 SITL or ArduPilot SITL.
2. Configure SITL to send MAVLink to the Lab GCS UDP listen port, usually
   `14550`.
3. Start Lab GCS.

Steps:

1. Open Fly.
2. In Connection, select UDP.
3. Start the UDP listener on port `14550`.
4. Wait for the active vehicle to change from the mock vehicle to PX4 SITL or
   ArduPilot SITL.
5. Show telemetry: vehicle type, autopilot, flight mode, heartbeat, position,
   attitude, GPS, and battery/status where available.
6. Open Plan.
7. Add or load a waypoint mission.
8. Validate the mission.
9. Upload to `PX4 SITL (UDP)` or `ArduPilot SITL (UDP)`.
10. Download from the same target.
11. Show transfer progress and status messages.
12. Open the Event Log and show the recorded link, vehicle, validation, and
    mission transfer events.

Required safety statement during demo:

- SITL mission transfer sends mission protocol data only.
- It does not arm, take off, change flight mode, start the mission, land, or
  RTL.
- Manual control in SITL is a logged stub in this MVP; no real
  `MANUAL_CONTROL` MAVLink frame is transmitted.

## Scenario 3: SafetyGate Blocked-Command Demo

Goal: show that unsafe paths remain blocked.

Steps:

1. Open Manual with the mock vehicle active.
2. Without connecting the mock joystick, click Enable.
3. Show `Blocked reason` explaining the missing joystick.
4. Connect the mock joystick and enable again.
5. Disconnect the mock joystick while active.
6. Show the failsafe/blocked state and Event Log entry.
7. If a serial flight controller is available for bench telemetry:
   - Remove propellers.
   - Connect serial.
   - Show the red `HARDWARE READ-ONLY` banner.
   - Show that Plan upload/download is blocked for hardware.
   - Show that Manual activation is blocked for hardware.
8. Explain that no UI exists for arm, force-arm, takeoff, land, RTL, mission
   start, or RC override.

Expected talking points:

- SafetyGate is the single allow/block point for mission upload and manual
  control.
- Serial hardware is telemetry-only.
- `SerialLink::writeBytes` refuses outbound bytes.
- The codebase contains no enabled sender for real arm, takeoff, land, RTL,
  mission start, force-arm, or `RC_CHANNELS_OVERRIDE`.

## Verification Commands

Build:

```powershell
& 'C:\QtOnline\Tools\CMake_64\bin\cmake.exe' --build build
```

Tests:

```powershell
$env:Path = 'C:\QtOnline\6.11.1\mingw_64\bin;C:\QtOnline\Tools\mingw1310_64\bin;' + $env:Path
& 'C:\QtOnline\Tools\CMake_64\bin\ctest.exe' --test-dir build --output-on-failure
```

