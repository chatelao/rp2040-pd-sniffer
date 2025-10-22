add_library(hal_stm32 STATIC
    hal_stm32.c
)

target_include_directories(hal_stm32 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../
)
