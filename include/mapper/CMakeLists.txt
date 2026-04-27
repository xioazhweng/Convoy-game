add_library(mapper INTERFACE Mapper.h)

add_library(polymorphic_mapper INTERFACE PolymorphicMapper.h)
target_link_libraries(polymorphic_mapper INTERFACE mapper)

add_library(ship_mapper INTERFACE ShipMapper.h)
target_link_libraries(ship_mapper INTERFACE mapper ship dto)

add_library(weapon_mapper INTERFACE WeaponMapper.h)
target_link_libraries(weapon_mapper INTERFACE mapper weapon dto)
