add_subdirectory(Unity)

add_executable(test_runner
    main.c
    test_data.c
    test_decoder.c
    test_encoder.c
    test_header_builder.c
)

target_include_directories(test_runner PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/pd_library
    ${CMAKE_CURRENT_SOURCE_DIR}/Unity/src
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(test_runner
    pd_library
    unity
)

add_test(NAME test_runner COMMAND test_runner)
