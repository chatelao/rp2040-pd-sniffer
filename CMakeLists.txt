cmake_minimum_required(VERSION 3.13)

# Set the path to the Pico SDK
set(PICO_SDK_PATH ${CMAKE_CURRENT_SOURCE_DIR}/pico-sdk)

# Initialize the Pico SDK
include(${PICO_SDK_PATH}/pico_sdk_init.cmake)

project(rp2040_pd_sniffer C CXX ASM)

# Initialize the SDK
pico_sdk_init()

# --- Shared Library ---
add_library(pd_library
    src/pd_library/pd_library.c
)
target_include_directories(pd_library PUBLIC src/pd_library)
target_link_libraries(pd_library pico_stdlib hardware_pio)

# --- Sniffer Application ---
add_executable(sniffer
    src/sniffer/main.c
)
pico_generate_pio_header(sniffer ${CMAKE_CURRENT_SOURCE_DIR}/src/sniffer/pd_sniffer.pio)
target_link_libraries(sniffer pd_library pico_stdlib hardware_pio hardware_dma hardware_clocks)
pico_add_extra_outputs(sniffer)
pico_enable_stdio_usb(sniffer 1)
pico_enable_stdio_uart(sniffer 0)

# --- Sink Application ---
add_executable(sink
    src/sink/main.c
)
pico_generate_pio_header(sink ${CMAKE_CURRENT_SOURCE_DIR}/src/sink/pd_transmitter.pio)
pico_generate_pio_header(sink ${CMAKE_CURRENT_SOURCE_DIR}/src/sniffer/pd_sniffer.pio)
target_link_libraries(sink pd_library pico_stdlib hardware_pio hardware_dma hardware_clocks)
pico_add_extra_outputs(sink)
pico_enable_stdio_usb(sink 1)
pico_enable_stdio_uart(sink 0)
