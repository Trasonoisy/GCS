# Lab GCS

Lab GCS is a university research Ground Control Station MVP for UAV
experiments. It is a new lab-owned desktop application built with Qt 6,
QML, C++17, CMake, and QtTest.

The project goal is to prove a safe, modular GCS architecture before any
real-flight capability is attempted. The current MVP supports simulation,
PX4 SITL telemetry, ArduPilot SITL telemetry, mission planning, mock mission
transfer, SITL mission transfer, a safety-gated manual-control framework,
structured logging, and hardware read-only telemetry monitoring.

Important safety statement: this build must not be used to fly a real drone.
Real arming, takeoff, landing, RTL, force-arm, mission start, RC override,
real-hardware mission upload, and real-hardware manual control are disabled.

## Documentation Map

- [Architecture](docs/architecture.md)
- [Safety rules](docs/safety.md)
- [Current capabilities](docs/current_capabilities.md)
- [SITL testing](docs/sitl_testing.md)
- [HITL / real UAV preparation](docs/hitl_real_uav_preparation.md)
- [Development plan](docs/development_plan.md)
- [Limitations and future work](docs/limitations_and_future_work.md)
- [Safety audit](docs/safety_audit.md)

## Architecture Summary

The selected architecture is a Qt/QML/C++ modular monolith:

```text
QML UI
  -> ViewModels
    -> LinkManager / MissionViewModel / ManualControlViewModel
      -> MultiVehicleManager
        -> Vehicle / VehicleStateStore
          -> FirmwarePlugin
          -> MissionManager / ManualControlManager / SafetyGate
            -> MAVLinkProtocol / MAVLinkMessageRouter
              -> LinkInterface
                -> MockLink / UdpLink / SerialLink
```

The UI is QML because the project needs a desktop operator interface with
live telemetry, tables, forms, and status panels. C++ owns protocol parsing,
safety checks, vehicle state, mission transfer, logging, and tests. QML never
constructs or parses MAVLink packets.

The design is MAVLink-based. The current MVP includes a local MAVLink 2 frame
parser/encoder for the messages it needs today: telemetry, status text, and
mission protocol messages. The project brief still requires MAVLink
`c_library_v2` as the future production substrate before real flight.

## Current MVP Modes

| Mode | Status |
|---|---|
| Simulation Mode | Enabled. `MockVehicle` drives UI telemetry and mock mission transfer without a drone or SITL. |
| PX4 SITL | Enabled for telemetry over UDP and SITL mission upload/download. |
| ArduPilot SITL | Enabled for telemetry and SITL mission upload/download. |
| Manual Control | Framework enabled with `MockJoystickBackend`. Mock vehicle consumes samples; SITL uses a logged stub. No real MANUAL_CONTROL MAVLink is transmitted. |
| Hardware Read-Only | Serial telemetry can be opened when Qt SerialPort is available. `SerialLink::writeBytes` refuses outbound bytes. |
| Logging | Enabled. JSONL event logs are written when a log path can be created. |

## Requirements

- Qt 6.4 or newer
- Qt modules: `Core`, `Gui`, `Network`, `Qml`, `Quick`, `QuickControls2`, `Test`
- Optional Qt module: `SerialPort` for hardware read-only telemetry
- CMake 3.21 or newer
- Ninja or another CMake generator
- C++17 compiler

The checked build on this machine uses:

- Qt: `C:\QtOnline\6.11.1\mingw_64`
- CMake: `C:\QtOnline\Tools\CMake_64\bin\cmake.exe`
- Compiler: `C:\QtOnline\Tools\mingw1310_64\bin\g++.exe`
- Ninja: `C:\QtOnline\Tools\Ninja\ninja.exe`

## Windows Convenience Scripts

PowerShell scripts are provided for the checked Qt/MinGW/Ninja workflow:

```powershell
.\scripts\build_windows.ps1
.\scripts\test_windows.ps1
.\scripts\run_windows.ps1
.\scripts\clean_build_windows.ps1
```

The scripts default to the paths listed above and can be configured either
with parameters:

```powershell
.\scripts\build_windows.ps1 `
  -QtPath 'C:\QtOnline\6.11.1\mingw_64' `
  -CMakePath 'C:\QtOnline\Tools\CMake_64\bin\cmake.exe' `
  -MingwPath 'C:\QtOnline\Tools\mingw1310_64' `
  -NinjaPath 'C:\QtOnline\Tools\Ninja\ninja.exe'
```

or with environment variables:

```powershell
$env:GCS_QT_PATH = 'C:\QtOnline\6.11.1\mingw_64'
$env:GCS_CMAKE_PATH = 'C:\QtOnline\Tools\CMake_64\bin\cmake.exe'
$env:GCS_MINGW_PATH = 'C:\QtOnline\Tools\mingw1310_64'
$env:GCS_NINJA_PATH = 'C:\QtOnline\Tools\Ninja\ninja.exe'
```

Useful options:

- `-BuildDir build-debug` selects another build directory.
- `-BuildType Release` configures a release build.
- `-CleanConfigure` on `build_windows.ps1` removes and reconfigures the build directory.
- `-SkipBuild` on `test_windows.ps1` or `run_windows.ps1` uses an existing build.

If a required Qt, CMake, MinGW, or Ninja path is missing, the scripts stop with
a clear error message naming the missing path.

If Windows blocks local scripts with an execution-policy error, run the same
script with a process-scoped bypass:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build_windows.ps1
```

## Configure

Windows PowerShell example:

```powershell
& 'C:\QtOnline\Tools\CMake_64\bin\cmake.exe' -S . -B build -G Ninja `
  -DCMAKE_BUILD_TYPE=Debug `
  -DCMAKE_PREFIX_PATH='C:/QtOnline/6.11.1/mingw_64' `
  -DCMAKE_CXX_COMPILER='C:/QtOnline/Tools/mingw1310_64/bin/g++.exe'
```

Generic Linux/macOS example:

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_PREFIX_PATH="$HOME/Qt/6.7.0/gcc_64"
```

## Build

```powershell
& 'C:\QtOnline\Tools\CMake_64\bin\cmake.exe' --build build
```

or, if CMake is on `PATH`:

```bash
cmake --build build
```

## Run

Windows PowerShell:

```powershell
$env:Path = 'C:\QtOnline\6.11.1\mingw_64\bin;C:\QtOnline\Tools\mingw1310_64\bin;' + $env:Path
.\build\LabGCSApp.exe
```

Linux/macOS:

```bash
./build/LabGCSApp
```

Expected startup behavior:

- The Fly tab opens with a simulated vehicle.
- A `SIMULATION MODE` banner is visible for the mock vehicle.
- Telemetry values update without any external hardware.
- Plan and Manual tabs are available.

## Test

Windows PowerShell:

```powershell
$env:Path = 'C:\QtOnline\6.11.1\mingw_64\bin;C:\QtOnline\Tools\mingw1310_64\bin;' + $env:Path
& 'C:\QtOnline\Tools\CMake_64\bin\ctest.exe' --test-dir build --output-on-failure
```

Generic:

```bash
ctest --test-dir build --output-on-failure
```

Current acceptance result: 32/32 tests pass.

## Demo Script

1. Start the app.
2. On Fly, show the simulation banner, live position, attitude, battery, GPS,
   heartbeat age, and event log.
3. On Plan, add three waypoints, edit latitude/longitude/altitude, validate,
   save to a `.plan` file, load it again, then upload/download against
   `MockVehicle`.
4. Start PX4 SITL or ArduPilot SITL and connect the UDP listener. Show the
   active vehicle label change from mock to SITL telemetry.
5. Upload/download the same mission to the SITL vehicle only. State clearly
   that upload does not arm, take off, change mode, start mission, land, or RTL.
6. On Manual, connect the mock joystick and show the watchdog/checklist state.
   For SITL, explain that the sink is a logged stub and no MANUAL_CONTROL frame
   goes to the autopilot.
7. If a serial flight controller is used for bench demonstration, remove
   propellers, connect only for telemetry, show the red Hardware Read-Only
   banner, and show that mission/manual controls are blocked.
8. Open the Event Log panel and show JSONL logging of operator and safety events.

## Disabled Capabilities

The following are intentionally not available:

- Real drone arming or disarming
- Real takeoff, landing, RTL, or mode-change commands
- Mission start
- Force-arm or pre-arm bypass
- `RC_CHANNELS_OVERRIDE`
- Real-hardware mission upload/download
- Real-hardware manual control
- Real MAVLink `MANUAL_CONTROL` transmission
- Parameter editing
- Geofence/rally editing
- Survey grid generation
- Terrain following
- Camera/gimbal control
- Replay through `TlogReplayLink`
- Real flight
