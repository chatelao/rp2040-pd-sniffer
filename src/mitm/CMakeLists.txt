add_executable(mitm
    main.c
)

target_link_libraries(mitm PRIVATE
    pico_stdlib
    pd_library
)

pico_add_extra_outputs(mitm)
