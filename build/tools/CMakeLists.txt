add_custom_target(tools
                  DEPENDS generatesinetables) # Convenience target.

add_executable(generatesinetables ${TOOLS_DIR}/GenerateSineTables.cpp)
