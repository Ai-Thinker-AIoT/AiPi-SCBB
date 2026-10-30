# axk BSP：红外接收
add_library(aipi_bsp_ir STATIC ${CMAKE_CURRENT_SOURCE_DIR}/axk_bsp_ir.c)
target_include_directories(aipi_bsp_ir PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
