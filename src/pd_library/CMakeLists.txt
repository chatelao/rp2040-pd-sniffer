add_library(pd_library STATIC
    pd_library.c
    pd_sink.c
)

target_include_directories(pd_library PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if (NATIVE_BUILD)
    target_compile_definitions(pd_library PUBLIC NATIVE_BUILD)
else()
    pico_generate_pio_header(pd_library ${CMAKE_CURRENT_SOURCE_DIR}/pd_receiver.pio)
    pico_generate_pio_header(pd_library ${CMAKE_CURRENT_SOURCE_DIR}/pd_transmitter.pio)

    target_link_libraries(pd_library PUBLIC
        pico_stdlib
        hardware_pio
        hardware_dma
    )
endif()
