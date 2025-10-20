add_library(test_firmware_logic STATIC
    pinger_logic.c
    ponger_logic.c
    source_emulator_logic.c
    sink_tester_logic.c
    test_firmware_common.c
)

target_include_directories(test_firmware_logic PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(test_firmware_logic PUBLIC
    pd_library
)

if(NOT NATIVE_BUILD)
    add_executable(pinger
        pinger.c
    )
    target_link_libraries(pinger test_firmware_logic pd_library pico_stdlib)
    pico_add_extra_outputs(pinger)

    add_executable(ponger
        ponger.c
    )
    target_link_libraries(ponger test_firmware_logic pd_library pico_stdlib hardware_dma)
    pico_add_extra_outputs(ponger)

    add_executable(source_emulator
        source_emulator.c
    )
    target_link_libraries(source_emulator test_firmware_logic pd_library pico_stdlib)
    pico_add_extra_outputs(source_emulator)

    add_executable(sink_tester
        sink_tester.c
    )
    target_link_libraries(sink_tester test_firmware_logic pd_library pico_stdlib hardware_dma)
    pico_add_extra_outputs(sink_tester)
endif()
