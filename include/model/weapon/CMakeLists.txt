add_library(weapon INTERFACE Weapon.h)
target_link_libraries(weapon INTERFACE point)
target_include_directories(weapon INTERFACE include)
