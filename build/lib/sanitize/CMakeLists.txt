set(CMAKE_BUILD_TYPE Debug)
add_definitions("-fsanitize=address,leak")
set(T "linux-sanitize")
include(../Include.cmake)
