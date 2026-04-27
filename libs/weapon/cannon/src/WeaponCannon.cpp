#include "../include/WeaponCannon.h"
#include "mapper/Mapper.h"
#include <string>
#include <functional>



WeaponCannon::WeaponCannon(unsigned current_ammunition_)
    : DefaultWeapon(
          fire_rate_cannon,
          fire_range_cannon,
          ammunition_cannon,
          current_ammunition_,
          cost_cannon,
          damage_cannon) {};


std::vector<Point>& WeaponCannon::get_shoot_area(Point center) const {
    static std::vector<Point> area;
    area.clear();
    area.push_back(center);
    return area;
}

unsigned WeaponCannon::get_id() const {
    std::hash<std::string> str_hash;
    const unsigned type_tag = static_cast<unsigned>(str_hash("cannon") & 0xFFFFu);
    return ::get_id(get_current_ammunition(), type_tag);
};
