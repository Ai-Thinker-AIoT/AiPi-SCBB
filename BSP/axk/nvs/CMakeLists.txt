# axk BSP：NVS
add_library(aipi_bsp_nvs STATIC ${CMAKE_CURRENT_SOURCE_DIR}/axk_bsp_nvs.c)
target_include_directories(aipi_bsp_nvs PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
