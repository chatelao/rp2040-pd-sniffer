cmake_minimum_required(VERSION 3.13)

option(NATIVE_BUILD "Build for native target (PC)" OFF)

if(NATIVE_BUILD)
    # For native build, we only build the tests
    project(rp2040-usb-pd-sniffer-tests C CXX)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_C_STANDARD 11)
    add_subdirectory(src/pd_library)
    add_subdirectory(src/test_firmware)
    enable_testing()
    add_subdirectory(test)
else()
    # For pico build, we build the firmware
    # Initialize the SDK
    include(pico-sdk/pico_sdk_init.cmake)
    project(rp2040-usb-pd-sniffer C CXX ASM)
    pico_sdk_init()

    add_subdirectory(src/pd_library)
    add_subdirectory(src/sniffer)
    add_subdirectory(src/sink)
add_subdirectory(src/source)
    add_subdirectory(src/test_firmware)
endif()
