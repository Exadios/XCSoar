
add_subdirectory("release")
add_subdirectory("debug")
add_subdirectory("sanitize")

# Patch panel
add_custom_target(xcsoar-release  DEPENDS xcsoar-linux-release)
add_custom_target(xcsoar-debug    DEPENDS xcsoar-linux-debug)
add_custom_target(xcsoar-sanitize DEPENDS xcsoar-linux-sanitize)
