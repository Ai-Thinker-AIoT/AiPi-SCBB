# axk BSP：软件 SPI
# 依赖 GPIO 与延时库；模块组合不同时根 CMakeLists 未必会加这两个目录，这里兜底。
if(NOT TARGET aipi_bsp_gpio)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../gpio ${CMAKE_CURRENT_BINARY_DIR}/axk_gpio)
endif()
if(NOT TARGET aipi_bsp_delay)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../delay ${CMAKE_CURRENT_BINARY_DIR}/axk_delay)
endif()

add_library(aipi_bsp_spi STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/axk_bsp_spi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/axk_bsp_spi_hw.c
)
target_include_directories(aipi_bsp_spi PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
target_link_libraries(aipi_bsp_spi PUBLIC aipi_bsp_gpio aipi_bsp_delay)
