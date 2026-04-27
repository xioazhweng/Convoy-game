add_library(transport_military_ship
            include/TransportMilitaryShip.h
            src/TransportMilitaryShip.cpp)
target_include_directories(transport_military_ship PUBLIC
                           include
                           ${CMAKE_SOURCE_DIR}/include)
#target_link_libraries(weapon_cannon weapon point)
