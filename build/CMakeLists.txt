#set(TOOLS_BIN ${XCSoar_BINARY_DIR}/build/tools)
set(OUTPUT_BASE ${XCSoar_BINARY_DIR})
set(OUTPUT_INCLUDE ${OUTPUT_BASE}/include)

add_subdirectory(./tools)
add_subdirectory(./generate)
add_subdirectory(./lib)
add_subdirectory(./src)
