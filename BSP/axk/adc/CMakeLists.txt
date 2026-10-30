# axk BSP：ADC
add_library(aipi_bsp_adc STATIC ${CMAKE_CURRENT_SOURCE_DIR}/axk_bsp_adc.c)
target_include_directories(aipi_bsp_adc PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
