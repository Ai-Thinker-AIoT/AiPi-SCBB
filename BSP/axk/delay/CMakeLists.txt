# axk BSP：忙等延时（软件 I2C / 软件 SPI 也依赖本库）
if(NOT TARGET aipi_bsp_delay)
    add_library(aipi_bsp_delay STATIC ${CMAKE_CURRENT_SOURCE_DIR}/axk_bsp_delay.c)
    target_include_directories(aipi_bsp_delay PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
    )
endif()
