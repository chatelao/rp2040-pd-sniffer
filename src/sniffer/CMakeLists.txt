add_executable(sniffer
    main.c
)

target_link_libraries(sniffer
    pd_library
    pico_stdlib
)

pico_add_extra_outputs(sniffer)
