# axk BSP：RD-01 雷达协议
add_library(aipi_bsp_radar STATIC ${CMAKE_CURRENT_SOURCE_DIR}/axk_bsp_radar.c)
target_include_directories(aipi_bsp_radar PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
