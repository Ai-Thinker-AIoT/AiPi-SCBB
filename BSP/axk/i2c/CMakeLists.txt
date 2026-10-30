# axk BSP：软件 I2C
# 依赖 GPIO 与延时库；某些模块组合（如只开 SHT3x）根 CMakeLists 不会加 gpio 目录，
# 所以这里主动补上。目标已存在时 add_subdirectory 的 if(NOT TARGET) 会跳过建库。
if(NOT TARGET aipi_bsp_gpio)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../gpio ${CMAKE_CURRENT_BINARY_DIR}/axk_gpio)
endif()
if(NOT TARGET aipi_bsp_delay)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../delay ${CMAKE_CURRENT_BINARY_DIR}/axk_delay)
endif()

add_library(aipi_bsp_i2c STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/axk_bsp_i2c.c
    ${CMAKE_CURRENT_SOURCE_DIR}/axk_bsp_i2c_hw.c
)
target_include_directories(aipi_bsp_i2c PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
target_link_libraries(aipi_bsp_i2c PUBLIC aipi_bsp_gpio aipi_bsp_delay)
