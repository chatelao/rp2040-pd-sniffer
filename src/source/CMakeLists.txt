add_executable(source
    main.c
)

target_link_libraries(source PRIVATE
    pico_stdlib
    pd_library
)

pico_add_extra_outputs(source)
