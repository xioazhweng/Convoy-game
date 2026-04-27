add_library(game_view STATIC
    include/SfmlGameView.h
    src/SfmlGameView.cpp
)

target_include_directories(game_view
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(game_view
    PUBLIC
        yaml-cpp
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SFML REQUIRED sfml-graphics sfml-window sfml-system sfml-audio)

target_include_directories(game_view PUBLIC ${SFML_INCLUDE_DIRS})
target_compile_options(game_view PRIVATE ${SFML_CFLAGS_OTHER})
target_link_libraries(game_view PUBLIC ${SFML_LIBRARIES})
