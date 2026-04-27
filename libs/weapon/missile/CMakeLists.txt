add_library(weapon_missile
            include/WeaponMissile.h
            src/WeaponMissile.cpp)

target_include_directories(weapon_missile PUBLIC include)
target_include_directories(weapon_missile PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(weapon_missile weapon_core point)
