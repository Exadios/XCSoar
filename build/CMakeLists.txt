#set(TOOLS_BIN ${XCSoar_BINARY_DIR}/build/tools)
set(OUTPUT_BASE ${XCSoar_BINARY_DIR})
set(OUTPUT_INCLUDE ${OUTPUT_BASE}/include)

add_custom_target(base_debug
                  COMMAND echo CMAKE_CURRENT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}
                  COMMAND echo OUTPUT_BASE ${OUTPUT_BASE}
                  COMMAND echo OUTPUT_INCLUDE ${OUTPUT_INCLUDE}
                 )

add_subdirectory(./tools)
add_subdirectory(./generate)
add_subdirectory(./lib)
add_subdirectory(./src)
