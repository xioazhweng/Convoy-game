add_library(game_presenter_impl
    include/GamePresenterImpl.h
    src/GamePresenterImpl.cpp
)
target_include_directories(game_presenter_impl PUBLIC include ../../libs/service)
target_link_libraries(game_presenter_impl PUBLIC
    mission_repo_impl
    mission_service_impl
    yalm_service
)
