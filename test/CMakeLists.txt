add_subdirectory(Unity)

add_executable(test_runner
    main.c
    test_data.c
    test_decoder.c
    test_encoder.c
    test_header_builder.c
    test_pinger_logic.c
    test_ponger_logic.c
    test_source_emulator_logic.c
    test_sink_tester_logic.c
    test_crc32.c
    test_pd_sink.c
    test_hal_dma.c
)

target_include_directories(test_runner PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/pd_library
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/test_firmware
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/sink
    ${CMAKE_CURRENT_SOURCE_DIR}/Unity/src
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(test_runner
    pd_library
    test_firmware_logic
    hal_pico
    unity
)

add_test(NAME test_runner COMMAND test_runner)
