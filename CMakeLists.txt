cmake_minimum_required(VERSION 3.12)

set(PICO_BOARD pico_w)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(ProjectGarageDoor C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

pico_sdk_init()

add_compile_options(
        -Wall
        -Wno-format
        -Wno-unused-function
        -Wno-maybe-uninitialized
)

file(GLOB PAHO_MQTTCLIENT_SRC
        "${CMAKE_CURRENT_LIST_DIR}/paho.mqtt.embedded-c/MQTTClient/src/*.cpp"
)

file(GLOB PAHO_MQTTPACKET_SRC
        "${CMAKE_CURRENT_LIST_DIR}/paho.mqtt.embedded-c/MQTTPacket/src/*.c"
)

add_executable(${PROJECT_NAME}
        main.cpp

        core/garage_door_controller.cpp
        core/garage_door_controller.h
        core/garage_types.h

        hardware/board_config.h
        hardware/buttons.cpp
        hardware/buttons.h
        hardware/limit_switches.cpp
        hardware/limit_switches.h
        hardware/rotary_encoder.cpp
        hardware/rotary_encoder.h
        hardware/stepper_motor.cpp
        hardware/stepper_motor.h
        hardware/status_leds.cpp
        hardware/status_leds.h

        logic/calibration_engine.cpp
        logic/calibration_engine.h
        logic/position_tracker.cpp
        logic/position_tracker.h
        logic/safety_monitor.cpp
        logic/safety_monitor.h

        communication/command_parser.cpp
        communication/command_parser.h
        communication/mqtt_client.cpp
        communication/mqtt_client.h
        communication/status_serializer.cpp
        communication/status_serializer.h
        communication/wifi_config.h

        storage/persistent_storage.cpp
        storage/persistent_storage.h

        mqtt/Countdown.cpp
        mqtt/Countdown.h
        mqtt/IPStack.cpp
        mqtt/IPStack.h
        mqtt/lwipopts.h

        ${PAHO_MQTTCLIENT_SRC}
        ${PAHO_MQTTPACKET_SRC}
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/core
        ${CMAKE_CURRENT_LIST_DIR}/hardware
        ${CMAKE_CURRENT_LIST_DIR}/logic
        ${CMAKE_CURRENT_LIST_DIR}/communication
        ${CMAKE_CURRENT_LIST_DIR}/storage
        ${CMAKE_CURRENT_LIST_DIR}/mqtt
        ${CMAKE_CURRENT_LIST_DIR}/paho.mqtt.embedded-c/MQTTClient/src
        ${CMAKE_CURRENT_LIST_DIR}/paho.mqtt.embedded-c/MQTTPacket/src
)

target_link_libraries(${PROJECT_NAME}
        pico_stdlib
        hardware_gpio
        hardware_flash
        hardware_sync
        hardware_irq
        pico_cyw43_arch_lwip_poll
)

pico_add_extra_outputs(${PROJECT_NAME})

pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)
