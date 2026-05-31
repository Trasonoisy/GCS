# ============================================================
# Core backend library (no Qt Quick dependency, easy to test)
# ============================================================
set(GCS_CORE_SOURCES
    Comms/LinkKind.h
    Comms/LinkInterface.h
    Comms/MockLink.h
    Comms/MockLink.cpp
    Comms/UdpLink.h
    Comms/UdpLink.cpp
    Comms/SerialLink.h
    Comms/SerialLink.cpp
    Comms/LinkManager.h
    Comms/LinkManager.cpp

    Protocol/MavlinkFrame.h
    Protocol/MavlinkFrame.cpp
    Protocol/MAVLinkProtocol.h
    Protocol/MAVLinkProtocol.cpp
    Protocol/MAVLinkMessageRouter.h
    Protocol/MAVLinkMessageRouter.cpp

    Vehicle/VehicleState.h
    Vehicle/VehicleState.cpp
    Vehicle/VehicleStateStore.h
    Vehicle/VehicleStateStore.cpp
    Vehicle/Vehicle.h
    Vehicle/Vehicle.cpp
    Vehicle/MultiVehicleManager.h
    Vehicle/MultiVehicleManager.cpp

    Firmware/FirmwarePlugin.h
    Firmware/MavType.h
    Firmware/PX4FirmwarePlugin.h
    Firmware/PX4FirmwarePlugin.cpp
    Firmware/ArduPilotFirmwarePlugin.h
    Firmware/ArduPilotFirmwarePlugin.cpp
    Firmware/ArduCopterFirmwarePlugin.h
    Firmware/ArduCopterFirmwarePlugin.cpp
    Firmware/ArduPlaneFirmwarePlugin.h
    Firmware/ArduPlaneFirmwarePlugin.cpp
    Firmware/ArduRoverFirmwarePlugin.h
    Firmware/ArduRoverFirmwarePlugin.cpp
    Firmware/ArduSubFirmwarePlugin.h
    Firmware/ArduSubFirmwarePlugin.cpp
    Firmware/FirmwarePluginManager.h
    Firmware/FirmwarePluginManager.cpp

    Mission/MissionItem.h
    Mission/MissionItem.cpp
    Mission/MissionPlan.h
    Mission/MissionPlan.cpp
    Mission/MissionValidator.h
    Mission/MissionValidator.cpp
    Mission/PlanFile.h
    Mission/PlanFile.cpp
    Mission/IMissionLink.h
    Mission/MissionUploader.h
    Mission/MissionUploader.cpp
    Mission/MissionDownloader.h
    Mission/MissionDownloader.cpp
    Mission/MissionManager.h
    Mission/MissionManager.cpp
    Mission/MavlinkMissionLink.h
    Mission/MavlinkMissionLink.cpp

    Safety/SafetyDecision.h
    Safety/SafetyGate.h
    Safety/SafetyGate.cpp

    Logging/LogEvent.h
    Logging/LogSink.h
    Logging/MemoryLogSink.h
    Logging/MemoryLogSink.cpp
    Logging/FileLogSink.h
    Logging/FileLogSink.cpp
    Logging/EventLogger.h
    Logging/EventLogger.cpp
    Logging/OperatorActionLogger.h
    Logging/OperatorActionLogger.cpp

    Manual/JoystickState.h
    Manual/AxisConfig.h
    Manual/AxisMapper.h
    Manual/AxisMapper.cpp
    Manual/JoystickBackend.h
    Manual/MockJoystickBackend.h
    Manual/MockJoystickBackend.cpp
    Manual/IManualControlSink.h
    Manual/ManualControlState.h
    Manual/ManualControlManager.h
    Manual/ManualControlManager.cpp
    Manual/MavlinkManualControlSink.h
    Manual/MavlinkManualControlSink.cpp
    Manual/SitlStubManualControlSink.h
    Manual/SitlStubManualControlSink.cpp

    Simulation/MockVehicle.h
    Simulation/MockVehicle.cpp
    Simulation/MockMissionLink.h
    Simulation/MockMissionLink.cpp
)

add_library(gcs_core STATIC ${GCS_CORE_SOURCES})

target_include_directories(gcs_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(gcs_core
    PUBLIC
        Qt6::Core
        Qt6::Network
)

# Phase 7: optional Qt6::SerialPort. SerialLink compiles either way.
if(GCS_HAS_SERIAL_PORT)
    target_link_libraries(gcs_core PUBLIC Qt6::SerialPort)
    target_compile_definitions(gcs_core PUBLIC GCS_HAS_SERIAL_PORT)
endif()

# ============================================================
# UI / ViewModel library (depends on Qt Quick)
# ============================================================
set(GCS_UI_SOURCES
    ViewModels/VehicleViewModel.h
    ViewModels/VehicleViewModel.cpp
    ViewModels/MissionViewModel.h
    ViewModels/MissionViewModel.cpp
    ViewModels/LinkViewModel.h
    ViewModels/LinkViewModel.cpp
    ViewModels/ManualControlViewModel.h
    ViewModels/ManualControlViewModel.cpp
    ViewModels/LogViewModel.h
    ViewModels/LogViewModel.cpp
)

add_library(gcs_ui STATIC ${GCS_UI_SOURCES})

target_include_directories(gcs_ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(gcs_ui
    PUBLIC
        gcs_core
        Qt6::Gui       # QClipboard, QDesktopServices used by LogViewModel
        Qt6::Qml
        Qt6::Quick
)

# ============================================================
# Application executable + QML module
# ============================================================
qt_add_executable(LabGCSApp
    App/main.cpp
)

target_link_libraries(LabGCSApp
    PRIVATE
        gcs_core
        gcs_ui
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Positioning
        Qt6::Location
)

set_target_properties(LabGCSApp PROPERTIES
    WIN32_EXECUTABLE FALSE
    MACOSX_BUNDLE FALSE
)
