project(XCSoar)
cmake_minimum_required(VERSION 3.0)

enable_testing()

set(XCSOAR ${CMAKE_HOME_DIRECTORY})
set(OUTPUT_BASE ${XCSoar_BINARY_DIR})
set(TOOLS_DIR ${XCSOAR}/tools)
set(XCSOAR_SRC ${XCSOAR}/src)
set(XCSOAR_BUILD ${XCSOAR}/build)
set(TOOLS_BIN ${OUTPUT_BASE}/build/tools)
set(OUTPUT_INCLUDE ${XCSoar_BINARY_DIR}/include)
set(OUTPUT_SRC ${XCSoar_BINARY_DIR}/src})

include_directories(${XCSOAR_SRC})

include("temporary-flags.cmake")

set(warnings "-Wmissing-noreturn -Wno-unused-parameter -Wno-missing-field-initializers  -Wcast-align -Wall -Wextra -Wwrite-strings -Wcast-qual -Wpointer-arith -Wsign-compare -Wundef -Wmissing-declarations -Wredundant-decls")
set(defines "-DUSE_WGS84")
set(flags "-std=gnu++14 -Werror")

#if (NOT CONFIGURED_ONCE)
    set(CMAKE_CXX_FLAGS "${warnings} ${defines} ${flags} ${temporary_cxx_flags}"
        CACHE STRING "Flags used by the compiler during all build types." FORCE)
    set(CMAKE_C_FLAGS   "${warnings} ${defines} ${temporary_c_flags} -Wmissing-prototypes -Wstrict-prototypes -Wnested-externs -Werror"
        CACHE STRING "Flags used by the compiler during all build types." FORCE)
    add_definitions(-DXCSOAR_VERSION=\"7.0_preview11\")
#endif()

#include_directories(./submoldule/xcsoar-exp/src)

# Suppress the generation of various resource files.
#
# The generation of the resource files is a bit of a mess. And the tools used
# are written in Perl! A temporary fix is to get the files that would be
# generated from the old master build, commit them to git, and suppress their
# regeneration by setting SUPPRESS_RESOURCES_GENERATION to Y.
#
# /TODO /todo Fix the resources generation system.
set(SUPPRESS_RESOURCES_GENERATION "Y")

add_subdirectory(./build)

set(CONFIGURED_ONCE TRUE CACHE INTERNAL
    "A flag showing that CMake has configured at least once.")


