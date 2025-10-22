if(NOT NATIVE_BUILD)
    add_executable(mitm
        main.c
    )

    target_link_libraries(mitm
        pd_library
        hal_${TARGET_PLATFORM}
        pico_stdlib
    )

    pico_add_extra_outputs(mitm)
endif()
