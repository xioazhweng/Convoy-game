
add_library(game_loader STATIC
    include/GameLoader.h
    src/GameLoader.cpp
)

target_include_directories(game_loader PRIVATE include)

target_link_libraries(game_loader
    # Model interfaces (headers)
    ship
    weapon

    # Concrete implementations used by `loader/src/GameLoader.cpp`
    weapon_cannon
    weapon_missile
    weapon_torpedo
    military_ship
    transport_ship
    transport_military_ship
    mission_repo_impl
    mission_service_impl
    yalm_service
    game_presenter_impl
    mapper
)
