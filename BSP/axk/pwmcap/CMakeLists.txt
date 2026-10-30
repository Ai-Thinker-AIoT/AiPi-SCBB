# axk BSP：PWM 捕获
add_library(aipi_bsp_pwmcap STATIC ${CMAKE_CURRENT_SOURCE_DIR}/axk_bsp_pwmcap.c)
target_include_directories(aipi_bsp_pwmcap PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
