add_library( mission_repo_impl
             include/MissionRepositoryImpl.h
             src/MissionRepositoryImpl.cpp
)

target_include_directories(mission_repo_impl PUBLIC include)

target_link_libraries(mission_repo_impl
        mission_repo
        point
        ship
        lookup_table
)