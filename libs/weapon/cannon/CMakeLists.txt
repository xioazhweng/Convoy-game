add_library(weapon_cannon
            include/WeaponCannon.h
            src/WeaponCannon.cpp)

target_include_directories(weapon_cannon PUBLIC include)
target_link_libraries(weapon_cannon weapon_core point)