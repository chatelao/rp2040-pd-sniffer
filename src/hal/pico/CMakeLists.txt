add_library(hal_pico STATIC
    hal_pico.c
)

target_include_directories(hal_pico PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../
    ${CMAKE_CURRENT_SOURCE_DIR}/../../pd_library
)
