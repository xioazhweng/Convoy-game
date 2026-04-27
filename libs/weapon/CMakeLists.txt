add_library(weapon_core INTERFACE)
target_include_directories(weapon_core INTERFACE include)

add_subdirectory(cannon)
add_subdirectory(missile)
add_subdirectory(torpedo)
