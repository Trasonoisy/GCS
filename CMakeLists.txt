cmake_minimum_required(VERSION 3.21)

project(LabGCS
    VERSION 0.1.0
    DESCRIPTION "Lab-grade UAV Ground Control Station"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Emit compile_commands.json so the VS Code C/C++ extension (and clangd)
# can resolve Qt and project includes without manual configuration.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS
    Core
    Gui
    Network
    Qml
    Quick
    QuickControls2
    Test
)

# Map integration (FlyView + PlanView). Qt Location uses Qt Positioning. The
# OSM plugin renders OpenStreetMap raster tiles, which is policy-safe for a
# lab/research GCS as long as we respect the OSM Tile Usage Policy (sane
# refresh rates, a clear User-Agent, and visible attribution — set in
# qml/Common/MapView.qml). If these modules are missing, install them via the
# Qt Maintenance Tool: "Qt 6.x → MinGW → Qt Positioning" and "Qt Location".
find_package(Qt6 6.4 REQUIRED COMPONENTS
    Positioning
    Location
)

# Phase 7: Qt6::SerialPort is optional. Without it, SerialLink compiles as a
# graceful stub (connectLink() emits an errorOccurred explaining how to
# install the module). Installing the Qt SerialPort module and reconfiguring
# is the only step needed to enable real hardware connections.
find_package(Qt6 6.4 QUIET COMPONENTS SerialPort)
if(TARGET Qt6::SerialPort)
    set(GCS_HAS_SERIAL_PORT TRUE)
    message(STATUS "Qt6::SerialPort found — SerialLink will use the real backend")
else()
    set(GCS_HAS_SERIAL_PORT FALSE)
    message(STATUS "Qt6::SerialPort NOT found — SerialLink will compile as a stub "
                   "(see docs/hitl_real_uav_preparation.md)")
endif()

qt_standard_project_setup(REQUIRES 6.4)

# Auto-generate qmldir files for subdirectories (FlyView/, Common/) — silences
# QTP0004 on Qt 6.5+ and matches the behavior we want anyway.
if(QT_KNOWN_POLICY_QTP0004)
    qt_policy(SET QTP0004 NEW)
endif()

set_source_files_properties(qml/Common/Theme.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE
)

enable_testing()

add_subdirectory(src)
add_subdirectory(tests)

# Declared here (not in src/) so qmlcache paths stay free of ".." segments.
# NO_GENERATE_QMLTYPES: this module exposes no C++ types via QML_ELEMENT —
# the ViewModel is bound through setContextProperty in main.cpp — so the
# .qmltypes registration step would receive an empty metatypes JSON and fail.
qt_add_qml_module(LabGCSApp
    URI LabGCS
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    NO_GENERATE_QMLTYPES
    QML_FILES
        qml/Main.qml
        qml/FlyView/FlyView.qml
        qml/FlyView/TelemetryPanel.qml
        qml/FlyView/LinkStatusPanel.qml
        qml/FlyView/AlertPanel.qml
        qml/FlyView/ConnectionPanel.qml
        qml/PlanView/PlanView.qml
        qml/PlanView/WaypointList.qml
        qml/PlanView/WaypointEditDialog.qml
        qml/PlanView/WaypointEditor.qml
        qml/PlanView/MissionActions.qml
        qml/PlanView/ValidationPanel.qml
        qml/ManualView/ManualControlPanel.qml
        qml/Common/InfoCard.qml
        qml/Common/Theme.qml
        qml/Common/StyledButton.qml
        qml/Common/StyledTextField.qml
        qml/Common/StyledComboBox.qml
        qml/Common/StyledCheckBox.qml
        qml/Common/StyledSlider.qml
        qml/Common/StyledScrollBar.qml
        qml/Common/StyledProgressBar.qml
        qml/Common/SimulationBanner.qml
        qml/Common/HardwareModeBanner.qml
        qml/Common/MapView.qml
        qml/FlyView/FlyMapPanel.qml
        qml/PlanView/PlanMapPanel.qml
        qml/Shell/WelcomeView.qml
        qml/Shell/ConnectView.qml
        qml/Shell/MainShell.qml
    RESOURCES
        qml/assets/logo.png
        qml/assets/home.png
        qml/assets/link.png
        qml/assets/expand_content.png
        qml/assets/vehicle_marker_black.png
)
