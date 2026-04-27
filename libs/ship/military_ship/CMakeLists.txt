add_library(military_ship
            include/MilitaryShip.h
            src/MilitaryShip.cpp)
target_include_directories(military_ship PUBLIC
                           include
                           ${CMAKE_SOURCE_DIR}/include)
#target_link_libraries(weapon_cannon weapon point)
