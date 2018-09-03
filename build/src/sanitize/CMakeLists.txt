set(CMAKE_BUILD_TYPE Debug)
add_definitions("-fsanitize=address,leak")
set(T "linux-sanitize")
add_custom_target(release_debug-${T}
                  COMMAND echo CMAKE_CURRENT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}
                  COMMAND echo PWD ${PWD})

include("../Include.cmake")
