# axk BSP：GPIO（软件 I2C / 软件 SPI 也依赖本库）
# 用 if(NOT TARGET) 防止被 i2c/spi 的 CMakeLists 提前 add_subdirectory 后重复建目标。
if(NOT TARGET aipi_bsp_gpio)
    add_library(aipi_bsp_gpio STATIC ${CMAKE_CURRENT_SOURCE_DIR}/axk_bsp_gpio.c)
    target_include_directories(aipi_bsp_gpio PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
    )
endif()
