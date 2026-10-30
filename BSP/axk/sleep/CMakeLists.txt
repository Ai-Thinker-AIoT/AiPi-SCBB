# axk BSP：低功耗
add_library(aipi_bsp_sleep STATIC ${CMAKE_CURRENT_SOURCE_DIR}/axk_bsp_sleep.c)
target_include_directories(aipi_bsp_sleep PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
