cmake_minimum_required(VERSION 3.10)
project(MilitaryConvoyGame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-std=c++20)

find_package(Catch2 REQUIRED)
find_package(yaml-cpp REQUIRED)

include(CTest)
enable_testing()

include_directories(include)

add_subdirectory(include)
add_subdirectory(libs)
add_subdirectory(loader)
add_subdirectory(tests)
#add_compile_options(--coverage)
#add_link_options(--coverage)
add_executable(game main.cpp)
add_compile_options(-fsanitize=thread -g)
add_link_options(-fsanitize=thread)

add_executable(data_race_bench tests/data_race/main.cpp)
target_include_directories(data_race_bench PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/libs
)
target_link_libraries(data_race_bench
    PRIVATE
    mission_service_impl
    mission_repo_impl
    military_ship
)

target_link_libraries(game
    PRIVATE
    game_loader
    game_view
    mission_repo_impl
    mission_service_impl
    yalm_service
    game_presenter_impl
    yaml-cpp
)

