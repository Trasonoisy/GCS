# SITL Testing

This document explains how to demonstrate and test Lab GCS with PX4 SITL and
ArduPilot SITL. SITL support is for telemetry and mission upload/download only.
It does not enable real drone commands.

## Safety Scope

Allowed in SITL:

- Receive heartbeat, position, attitude, GPS, battery, VFR HUD, and status text.
- Show PX4 or ArduPilot vehicle state in the UI.
- Upload and download missions through the MAVLink mission protocol.
- Exercise the manual-control framework through a logged SITL stub.

Not allowed:

- Arm.
- Takeoff.
- Land.
- RTL.
- Mission start.
- Force-arm.
- RC override.
- Real MAVLink `MANUAL_CONTROL` transmission.

## App Setup

Start the app:

```powershell
$env:Path = 'C:\QtOnline\6.11.1\mingw_64\bin;C:\QtOnline\Tools\mingw1310_64\bin;' + $env:Path
.\build\LabGCSApp.exe
```

Open the Fly tab. In the Connection panel:

1. Select UDP.
2. Set the listen port used by the simulator, usually `14550`.
3. Start the UDP listener.
4. Wait for the SITL vehicle heartbeat.

The active vehicle label should change from the mock vehicle to a PX4 or
ArduPilot vehicle when heartbeats arrive.

## PX4 SITL

PX4 SITL commonly sends MAVLink to UDP port `14550`.

Typical workflow:

1. Start PX4 SITL from the PX4 development environment.
2. Confirm it is configured to output MAVLink to `127.0.0.1:14550` or an
   equivalent local UDP endpoint.
3. Start Lab GCS.
4. Start the UDP listener on port `14550`.
5. Confirm telemetry appears:
   - Autopilot: `PX4`
   - Vehicle type: Copter/Plane/etc. from heartbeat type
   - Flight mode decoded by `PX4FirmwarePlugin`
   - Battery, GPS, attitude, global position, and VFR HUD where available

Mission transfer:

1. Open Plan.
2. Add waypoints.
3. Validate the mission.
4. Click upload to `PX4 SITL (UDP)`.
5. Download from the same target to verify round-trip behavior.

The mission upload is data transfer only. It does not arm, start the mission,
take off, land, RTL, or change flight mode.

## ArduPilot SITL

ArduPilot SITL can output MAVLink over UDP or TCP. The current UI exposes UDP
listener support; TCP is represented in backend policy but there is no TCP
connection panel in the current UI.

Recommended UDP workflow:

1. Start ArduPilot SITL.
2. Configure or confirm a MAVLink UDP output to the Lab GCS listen port,
   commonly `14550`.
3. Start Lab GCS.
4. Start the UDP listener.
5. Confirm telemetry appears:
   - Autopilot: `ArduPilot`
   - Airframe plugin: `ArduCopter`, `ArduPlane`, `ArduRover`, or `ArduSub`
   - Flight mode decoded by the selected ArduPilot firmware plugin

Mission transfer:

1. Open Plan.
2. Add waypoints.
3. Validate the mission.
4. Click upload to `ArduPilot SITL (UDP)`.
5. Download from the same target to verify round-trip behavior.

## Mission Protocol Covered

Upload:

```text
GCS -> MISSION_COUNT
SITL -> MISSION_REQUEST_INT(seq)
GCS -> MISSION_ITEM_INT(seq)
SITL -> MISSION_ACK
```

Download:

```text
GCS -> MISSION_REQUEST_LIST
SITL -> MISSION_COUNT
GCS -> MISSION_REQUEST_INT(seq)
SITL -> MISSION_ITEM_INT(seq)
GCS -> MISSION_ACK
```

The state machines include timeout/retry handling and concurrent-transfer
rejection. Only `MISSION_ITEM_INT` is supported.

## Manual Control in SITL

The Manual tab can show joystick normalization, packed values, and watchdog
state. For SITL vehicles, manual-control samples are delivered to
`SitlStubManualControlSink`. This sink is intentionally a stub: it logs and
stores sample information but does not send MAVLink `MANUAL_CONTROL` bytes.

## Troubleshooting

| Symptom | Likely cause |
|---|---|
| No vehicle appears | UDP listener is on the wrong port, or SITL is not outputting to Lab GCS. |
| Vehicle appears as unknown | Heartbeat autopilot/type values are not mapped yet. |
| Mission upload button disabled | Active vehicle is mock/serial/replay/unknown, no mission manager is wired, or SafetyGate blocks the path. |
| Mission upload times out | SITL did not request mission items or ACK the transaction. |
| Manual control blocked | Joystick disconnected, operator not enabled, heartbeat stale, or vehicle is not mock/SITL. |

