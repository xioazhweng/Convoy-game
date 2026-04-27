add_library(mission_service_impl
            include/MissionServiceImpl.h
            src/MissionServiceImpl.cpp)
            
target_include_directories(mission_service_impl PUBLIC include)

target_link_libraries(mission_service_impl
                    PUBLIC
                    mission_repo
                    ship
                    weapon
                    random_attack_strategy)
