add_library(weapon_torpedo
            include/WeaponTorpedo.h
            src/WeaponTorpedo.cpp)

target_include_directories(weapon_torpedo PUBLIC include)
target_include_directories(weapon_torpedo PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(weapon_torpedo weapon_core point)
