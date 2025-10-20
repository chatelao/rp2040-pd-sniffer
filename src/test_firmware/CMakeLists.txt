add_executable(pinger
    pinger.c
)
target_link_libraries(pinger pd_library pico_stdlib)
pico_add_extra_outputs(pinger)

add_executable(ponger
    ponger.c
)
target_link_libraries(ponger pd_library pico_stdlib hardware_dma)
pico_add_extra_outputs(ponger)

add_executable(source_emulator
    source_emulator.c
)
target_link_libraries(source_emulator pd_library pico_stdlib)
pico_add_extra_outputs(source_emulator)

add_executable(sink_tester
    sink_tester.c
)
target_link_libraries(sink_tester pd_library pico_stdlib hardware_dma)
pico_add_extra_outputs(sink_tester)
