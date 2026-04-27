add_library(yalm_service
            include/YalmStateService.h
            src/YamlStateService.cpp)
target_include_directories(yalm_service PUBLIC include)
target_link_libraries(yalm_service state_service mission_repo yaml-cpp)
