# axk BSP：Flash
add_library(aipi_bsp_flash STATIC ${CMAKE_CURRENT_SOURCE_DIR}/axk_bsp_flash.c)
target_include_directories(aipi_bsp_flash PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
