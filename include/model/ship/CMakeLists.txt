add_library(ship INTERFACE Ship.h)
add_library(military INTERFACE Military.h)
add_library(transport INTERFACE Transport.h)
target_link_libraries(military INTERFACE weapon point place)
