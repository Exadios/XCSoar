project(XCSoar)
cmake_minimum_required(VERSION 3.0)

enable_testing()

set(XCSOAR ${CMAKE_HOME_DIRECTORY})
set(TOOLS_DIR ${XCSOAR}/tools)
set(XCSOAR_SRC ${XCSOAR}/src)
set(TOOLS_BIN ${XCSoar_BINARY_DIR}/tools) # YCSoar_BINARY_DIR set by cmake.
set(OUTPUT_INCLUDE ${XCSoar_BINARY_DIR}/include)

include_directories(${XCSOAR_SRC})

set(warnings "-Wno-error=unused-parameter -Wall -Wextra -Wwrite-strings -Wcast-qual -Wpointer-arith -Wsign-compare -Wundef -Wmissing-declarations -Wredundant-decls -Wmissing-noreturn -Wno-unused-parameter -Wno-missing-field-initializers  -Wcast-align -Werror")
set(defines "-DUSE_WGS84")
set(flags "-fno-exceptions -fno-rtti -std=gnu++0x -fno-threadsafe-statics -fmerge-all-constants -fvisibility=hidden -fpic -funwind-tables -finput-charset=utf-8 -funit-at-a-time")

#if (NOT CONFIGURED_ONCE)
    set(CMAKE_CXX_FLAGS "${warnings} ${defines} ${flags}"
        CACHE STRING "Flags used by the compiler during all build types." FORCE)
    set(CMAKE_C_FLAGS   "${warnings} ${defines}"
        CACHE STRING "Flags used by the compiler during all build types." FORCE)
    add_definitions(-DXCSOAR_VERSION=\"6.8.2\")
#endif()

#include_directories(./submoldule/xcsoar-exp/src)

#add_subdirectory(./tools)
#add_subdirectory(./generate)
add_subdirectory(./build)
#add_subdirectory(./src)

set(CONFIGURED_ONCE TRUE CACHE INTERNAL
    "A flag showing that CMake has configured at least once.")


