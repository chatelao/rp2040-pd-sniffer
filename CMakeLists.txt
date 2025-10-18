cmake_minimum_required(VERSION 3.13)

# Set the path to the Pico SDK
set(PICO_SDK_PATH ${CMAKE_CURRENT_SOURCE_DIR}/pico-sdk)

# Initialize the Pico SDK
include(${PICO_SDK_PATH}/pico_sdk_init.cmake)

project(rp2040_pd_sniffer C CXX ASM)

# Initialize the SDK
pico_sdk_init()

add_executable(rp2040_pd_sniffer
    src/main.c
)

pico_generate_pio_header(rp2040_pd_sniffer ${CMAKE_CURRENT_SOURCE_DIR}/src/pd_sniffer.pio)

# Pull in Raspberry Pi Pico specific libraries
target_link_libraries(rp2040_pd_sniffer pico_stdlib hardware_pio hardware_dma hardware_clocks)

# Create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(rp2040_pd_sniffer)

# Enable usb output, disable uart output
pico_enable_stdio_usb(rp2040_pd_sniffer 1)
pico_enable_stdio_uart(rp2040_pd_sniffer 0)
