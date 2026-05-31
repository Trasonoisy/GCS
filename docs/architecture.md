# Architecture

This document describes the current Lab GCS MVP architecture as implemented in
the repository.

## Goal

Lab GCS is a safe research Ground Control Station for university UAV work. The
MVP is intended to demonstrate the architecture, mission-planning workflow,
SITL telemetry, safety gates, and logging before any real-flight control path
is considered.

The project is not a fork of QGroundControl. It follows proven GCS layering
ideas, but the codebase is lab-owned and intentionally smaller.

## Why Desktop Qt/QML/C++

The selected solution is a desktop Qt 6 / QML / C++ application because:

- A GCS needs a native desktop operator interface with low-latency telemetry,
  mission editing, serial/UDP links, and long-running sessions.
- QML is well suited for the Fly, Plan, Manual, connection, and log panels.
- C++ is appropriate for MAVLink parsing, state machines, safety decisions,
  serial/UDP I/O, and deterministic unit tests.
- Qt gives one framework for UI, signals/slots, timers, JSON, files, network,
  serial support, and QtTest.
- CMake and QtTest make the project portable and testable for lab machines.

## Main Data Flow

Telemetry flows upward:

```text
MockVehicle / PX4 SITL / ArduPilot SITL / Serial hardware telemetry
  -> LinkInterface
  -> MAVLinkProtocol
  -> MAVLinkMessageRouter
  -> Vehicle
  -> VehicleStateStore
  -> VehicleViewModel
  -> QML Fly View
```

Mission transfer flows downward:

```text
QML Plan View
  -> MissionViewModel
  -> MissionValidator
  -> SafetyGate.canUploadMission
  -> MissionManager
  -> MissionUploader / MissionDownloader
  -> IMissionLink
  -> MockMissionLink or MavlinkMissionLink
  -> MockVehicle or SITL transport
```

Manual control framework flow:

```text
QML Manual View
  -> ManualControlViewModel
  -> MockJoystickBackend
  -> AxisMapper
  -> ManualControlManager
  -> SafetyGate.canStartManualControl / canContinueManualControl
  -> MockVehicle or MavlinkManualControlSink (UDP SITL)
```

Real-hardware serial flow is read-only:

```text
SerialLink read bytes
  -> MAVLinkProtocol
  -> MAVLinkMessageRouter
  -> VehicleStateStore
  -> QML

SerialLink write bytes
  -> rejected by SerialLink::writeBytes
```

## Module Responsibilities

| Module | Key classes | Responsibility |
|---|---|---|
| `App` | `main.cpp` | Creates the Qt app, backend objects, logging, view models, and QML engine. |
| `Comms` | `LinkInterface`, `MockLink`, `UdpLink`, `SerialLink`, `LinkManager` | Transport bytes. Does not parse MAVLink. Serial is read-only for hardware. |
| `Protocol` | `MAVLinkProtocol`, `MAVLinkMessageRouter`, `MavlinkFrame` | Decode supported MAVLink 2 frames, emit typed signals, route telemetry to vehicles. |
| `Vehicle` | `Vehicle`, `VehicleState`, `VehicleStateStore`, `MultiVehicleManager` | Own per-vehicle state and active-vehicle selection. |
| `Firmware` | `FirmwarePlugin`, `PX4FirmwarePlugin`, `ArduPilotFirmwarePlugin`, Ardu* subclasses | Isolate PX4/ArduPilot mode naming, airframe handling, and mission policy. |
| `Mission` | `MissionPlan`, `MissionItem`, `MissionValidator`, `MissionManager`, `MavlinkMissionLink` | Mission model, validation, `.plan` I/O, upload/download state machines. |
| `Manual` | `MockJoystickBackend`, `AxisMapper`, `ManualControlManager`, `MavlinkManualControlSink`, `SitlStubManualControlSink` | Normalize joystick input, watchdog manual-control state, deliver mock samples and UDP SITL MAVLink manual-control samples. |
| `Safety` | `SafetyGate`, `SafetyDecision` | Central allow/block service for dangerous operations. |
| `Logging` | `EventLogger`, `MemoryLogSink`, `FileLogSink`, `OperatorActionLogger` | Structured JSONL event logging and UI log buffer. |
| `Simulation` | `MockVehicle`, `MockMissionLink` | Hardware-free telemetry and mission testing. |
| `ViewModels` | `VehicleViewModel`, `MissionViewModel`, `LinkViewModel`, `ManualControlViewModel`, `LogViewModel` | QML-facing properties and commands. |
| `qml` | `FlyView`, `PlanView`, `ManualView`, common banners/cards | UI only. No MAVLink parsing or raw packet sending. |

## MAVLink Design

The MVP is MAVLink-based, but scoped:

- MAVLink telemetry is decoded in `MAVLinkProtocol`.
- `MAVLinkMessageRouter` creates or updates vehicles from heartbeats and
  routes supported messages into `Vehicle`.
- `MavlinkMissionLink` sends and receives the mission protocol for SITL only.
- `MavlinkManualControlSink` sends `MANUAL_CONTROL` frames for UDP SITL only.
- `MissionUploader` uses `MISSION_COUNT`, waits for `MISSION_REQUEST_INT`,
  sends `MISSION_ITEM_INT`, and waits for `MISSION_ACK`.
- `MissionDownloader` sends `MISSION_REQUEST_LIST`, receives count/items,
  requests each item, and acknowledges completion.
- No command queue for `COMMAND_LONG` or `COMMAND_INT` is enabled.
- No arming, takeoff, landing, RTL, mission-start, force-arm, or RC override
  sender exists.

Production MAVLink work should replace the current minimal frame support with
MAVLink `c_library_v2` before real flight.

## Safety Boundaries

Safety boundaries are part of the architecture:

- QML cannot send raw MAVLink.
- Mission upload calls `SafetyGate::canUploadMission` before any bytes are
  written.
- Manual control calls `SafetyGate::canStartManualControl` before activation
  and `canContinueManualControl` before every sample.
- Serial hardware vehicles receive no mission manager.
- `SerialLink::writeBytes` rejects outbound bytes even if a caller tries to
  write.
- SafetyGate always blocks arm, takeoff, RTL, and real-hardware manual/mission
  paths in this MVP.

## Current Build Shape

The app starts with a `MockVehicle`. It can also listen for SITL telemetry over
UDP and hardware telemetry over serial if Qt SerialPort is available. The active
vehicle is selected by `MultiVehicleManager`, and the view models update when
the active vehicle changes.

The project remains a modular monolith: all code is in one executable, but
backend modules are separated and tested independently.

