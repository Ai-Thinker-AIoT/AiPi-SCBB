# axk BSP：I2S
add_library(aipi_bsp_i2s STATIC ${CMAKE_CURRENT_SOURCE_DIR}/axk_bsp_i2s.c)
target_include_directories(aipi_bsp_i2s PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
