
add_custom_target(release_debug
                  COMMAND echo CMAKE_CURRENT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}
                  COMMAND echo PWD ${PWD})

include("../Include.cmake")
