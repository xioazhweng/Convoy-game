add_library(mission_repo INTERFACE MissionRepository.h)

target_include_directories(mission_repo INTERFACE include)

target_link_libraries(mission_repo INTERFACE 
    lookup_table
    ship
    point
)
