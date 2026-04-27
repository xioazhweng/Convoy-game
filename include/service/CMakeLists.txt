add_library(mission_service INTERFACE MissionService.h)
target_link_libraries(mission_service INTERFACE ship weapon point place )

add_library(state_service INTERFACE StateService.h)
target_link_libraries(state_service INTERFACE dto)
