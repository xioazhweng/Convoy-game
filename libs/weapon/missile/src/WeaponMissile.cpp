#include "../include/WeaponMissile.h"
#include "mapper/Mapper.h"
#include <cstdlib> 
#include <string>
#include <functional>

WeaponMissile::WeaponMissile(unsigned current_ammunition_)
    : DefaultWeapon(
          fire_rate_missile,
          fire_range_missile,
          ammunition_missile,
          current_ammunition_,
          cost_missile,
          damage_missile) {}

std::vector<Point>& WeaponMissile::get_shoot_area(Point center) const{
    static std::vector<Point> area;
    area.clear();
    int num_shots = 3 + (rand() % 3); 
    for (int i = 0; i < num_shots; i++) {
        int offset_x = (rand() % 21) - 10; 
        int offset_y = (rand() % 21) - 10; 
        Point target(center.get_x() + offset_x, center.get_y() + offset_y);
        area.push_back(target);
    }
    
    return area;
}

unsigned WeaponMissile::get_id() const {
    std::hash<std::string> str_hash;
    const unsigned type_tag = static_cast<unsigned>(str_hash("missile") & 0xFFFFu);
    return ::get_id(get_current_ammunition(), type_tag);
};
