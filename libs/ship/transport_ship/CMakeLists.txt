add_library(transport_ship
            include/TransportShip.h
            src/TransportShip.cpp)
target_include_directories(transport_ship PUBLIC
                           include
                           ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(transport_ship PUBLIC ship transport)
