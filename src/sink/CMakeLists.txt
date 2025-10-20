add_executable(sink
    main.c
)

target_link_libraries(sink
    pd_library
    pico_stdlib
)

pico_add_extra_outputs(sink)
