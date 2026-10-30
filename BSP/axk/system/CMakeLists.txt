# axk BSP：系统（复位/随机数/芯片名）
add_library(aipi_bsp_system STATIC ${CMAKE_CURRENT_SOURCE_DIR}/axk_bsp_system.c)
target_include_directories(aipi_bsp_system PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
