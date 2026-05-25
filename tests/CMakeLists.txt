function(gcs_add_test target source_file)
    qt_add_executable(${target} ${source_file})
    target_link_libraries(${target} PRIVATE gcs_core Qt6::Test)
    add_test(NAME ${target} COMMAND ${target})
endfunction()

function(gcs_add_network_test target source_file)
    qt_add_executable(${target} ${source_file})
    target_link_libraries(${target} PRIVATE gcs_core Qt6::Test Qt6::Network)
    add_test(NAME ${target} COMMAND ${target})
endfunction()

gcs_add_test(tst_vehicle_state          Vehicle/tst_vehicle_state.cpp)
gcs_add_test(tst_vehicle_state_store    Vehicle/tst_vehicle_state_store.cpp)
gcs_add_test(tst_multi_vehicle_manager  Vehicle/tst_multi_vehicle_manager.cpp)
gcs_add_test(tst_mock_link              Comms/tst_mock_link.cpp)
gcs_add_test(tst_mock_vehicle           Simulation/tst_mock_vehicle.cpp)
gcs_add_test(tst_firmware_plugins       Firmware/tst_firmware_plugins.cpp)

gcs_add_test(tst_mission_validator      Mission/tst_mission_validator.cpp)
gcs_add_test(tst_plan_file              Mission/tst_plan_file.cpp)
gcs_add_test(tst_mission_uploader       Mission/tst_mission_uploader.cpp)
gcs_add_test(tst_mission_downloader     Mission/tst_mission_downloader.cpp)
gcs_add_test(tst_mission_manager        Mission/tst_mission_manager.cpp)

# Phase 3
gcs_add_test(tst_mavlink_parser         Protocol/tst_mavlink_parser.cpp)
gcs_add_test(tst_message_router         Protocol/tst_message_router.cpp)
gcs_add_test(tst_px4_modes              Firmware/tst_px4_modes.cpp)
gcs_add_network_test(tst_udp_link       Comms/tst_udp_link.cpp)

# Phase 4
gcs_add_test(tst_axis_mapper            Manual/tst_axis_mapper.cpp)
gcs_add_test(tst_mock_joystick          Manual/tst_mock_joystick.cpp)
gcs_add_test(tst_safety_gate            Safety/tst_safety_gate.cpp)
gcs_add_test(tst_manual_control_manager Manual/tst_manual_control_manager.cpp)

# Phase 5
gcs_add_test(tst_mav_type                Firmware/tst_mav_type.cpp)
gcs_add_test(tst_ardupilot_modes         Firmware/tst_ardupilot_modes.cpp)
gcs_add_test(tst_firmware_plugin_manager Firmware/tst_firmware_plugin_manager.cpp)

# Phase 6
gcs_add_test(tst_memory_log_sink            Logging/tst_memory_log_sink.cpp)
gcs_add_test(tst_file_log_sink              Logging/tst_file_log_sink.cpp)
gcs_add_test(tst_event_logger               Logging/tst_event_logger.cpp)
gcs_add_test(tst_operator_action_logger     Logging/tst_operator_action_logger.cpp)
gcs_add_test(tst_safety_logging_integration Logging/tst_safety_logging_integration.cpp)

# Phase 7
gcs_add_test(tst_serial_link                Comms/tst_serial_link.cpp)
gcs_add_test(tst_safety_gate_hardware_mode  Safety/tst_safety_gate_hardware_mode.cpp)

# Phase 8
gcs_add_test(tst_mavlink_mission_codec      Protocol/tst_mavlink_mission_codec.cpp)
gcs_add_test(tst_mavlink_mission_link       Mission/tst_mavlink_mission_link.cpp)
gcs_add_test(tst_safety_gate_mission_sitl   Safety/tst_safety_gate_mission_sitl.cpp)
