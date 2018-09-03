#set(TOOLS_BIN ${XCSoar_BINARY_DIR}/build/tools)

add_custom_target(base_debug
                  COMMAND echo CMAKE_CURRENT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}
                  COMMAND echo OUTPUT_BASE ${OUTPUT_BASE}
                  COMMAND echo OUTPUT_INCLUDE ${OUTPUT_INCLUDE}
                 )

add_subdirectory(./tools)
add_subdirectory(./resource)
add_subdirectory(./generate)
add_definitions(${BOOST_CPPFLAGS})
add_subdirectory(./lib)
add_subdirectory(./src)

# Patch panel
add_custom_target(release  DEPENDS lib-release xcsoar-release)
add_custom_target(debug    DEPENDS lib-debug xcsoar-debug)
add_custom_target(sanitize DEPENDS lib-sanitize xcsoar-sanitize)
