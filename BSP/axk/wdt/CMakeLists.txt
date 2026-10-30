# axk BSP：看门狗
add_library(aipi_bsp_wdt STATIC ${CMAKE_CURRENT_SOURCE_DIR}/axk_bsp_wdt.c)
target_include_directories(aipi_bsp_wdt PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
