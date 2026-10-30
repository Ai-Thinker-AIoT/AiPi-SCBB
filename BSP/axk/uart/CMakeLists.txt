# axk BSP：UART
# HXD039B2 用到 AXK_HXD039B2_DELAY_MS，而根 CMakeLists 为它只加 uart/gpio 目录，
# 因此这里主动补上延时库；目标已存在时 add_subdirectory 的 if(NOT TARGET) 会跳过建库。
if(NOT TARGET aipi_bsp_delay)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../delay ${CMAKE_CURRENT_BINARY_DIR}/axk_delay)
endif()

add_library(aipi_bsp_uart STATIC ${CMAKE_CURRENT_SOURCE_DIR}/axk_bsp_uart.c)
target_include_directories(aipi_bsp_uart PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
target_link_libraries(aipi_bsp_uart PUBLIC aipi_bsp_delay)
