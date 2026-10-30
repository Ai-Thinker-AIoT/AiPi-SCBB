# axk BSP：RTC
add_library(aipi_bsp_rtc STATIC ${CMAKE_CURRENT_SOURCE_DIR}/axk_bsp_rtc.c)
target_include_directories(aipi_bsp_rtc PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
