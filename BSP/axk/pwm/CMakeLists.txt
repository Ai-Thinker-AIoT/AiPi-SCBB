# axk BSP：PWM
add_library(aipi_bsp_pwm STATIC ${CMAKE_CURRENT_SOURCE_DIR}/axk_bsp_pwm.c)
target_include_directories(aipi_bsp_pwm PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
