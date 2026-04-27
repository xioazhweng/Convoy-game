cmake_minimum_required(VERSION 3.10)
project(MY_Tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Catch2 REQUIRED)

add_executable(logic_tests
    src/lookup_table_tests.cpp
    src/model_tests.cpp
    src/weapon_tests.cpp
    src/mapper_tests.cpp
    src/repository_tests.cpp
    src/mission_service_tests.cpp
    src/state_service_tests.cpp
    src/game_presenter_tests.cpp
    src/strategy_tests.cpp
)

target_include_directories(logic_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/libs
)

target_link_libraries(logic_tests
    PRIVATE
        Catch2::Catch2WithMain
        game_presenter_impl
        mission_service_impl
        mission_repo_impl
        yalm_service
        military_ship
        transport_ship
        transport_military_ship
        weapon_cannon
        weapon_missile
        weapon_torpedo
        mapper
        ship
        weapon
        point
        lookup_table
)

include(CTest)
include(Catch)
catch_discover_tests(logic_tests)
