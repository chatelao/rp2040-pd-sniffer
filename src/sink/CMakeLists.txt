if(NOT NATIVE_BUILD)
    add_executable(sink
        main.c
    )

    target_link_libraries(sink
        pd_library
        hal_${TARGET_PLATFORM}
        pico_stdlib
    )

    pico_add_extra_outputs(sink)
endif()
