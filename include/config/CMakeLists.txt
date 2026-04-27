add_library(loader INTERFACE Loader.h)

target_link_libraries(loader INTERFACE
        ship_mapper
        weapon_mapper
        mission_repo
        mission_service
        state_service
        transport_presenter
        transport_view)
