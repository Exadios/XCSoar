set(CMAKE_BUILD_TYPE Debug)
set(T "linux-debug")
add_custom_target(release_debug-${T}
                  COMMAND echo CMAKE_CURRENT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}
                  COMMAND echo PWD ${PWD})

include("../Include.cmake")
