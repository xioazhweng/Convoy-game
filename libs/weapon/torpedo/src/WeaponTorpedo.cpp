#include "../include/WeaponTorpedo.h"
#include "mapper/Mapper.h"
#include <cmath>
#include <string>
#include <functional>

WeaponTorpedo::WeaponTorpedo(unsigned current_ammunition_)
    : DefaultWeapon(
          fire_rate_torpedo,
          fire_range_torpedo,
          /*max_ammunition*/ ammunition_torpedo,
          /*current_ammunition*/ current_ammunition_,
          cost_torpedo,
          damage_torpedo) {}

std::vector<Point>& WeaponTorpedo::get_shoot_area(Point center) const {
    static std::vector<Point> area;
    area.clear();
    
 
    int radius = 5; 
    int num_points = 8; 
    
    for (int i = 0; i < num_points; i++) {
        double angle = (2.0 * M_PI * i) / num_points;
        int x = center.get_x() + static_cast<int>(radius * cos(angle));
        int y = center.get_y() + static_cast<int>(radius * sin(angle));
        Point target(x, y);
        area.push_back(target);
    }
    area.push_back(center);
    
    return area;
}

unsigned WeaponTorpedo::get_id() const {
    std::hash<std::string> str_hash;
    const unsigned type_tag = static_cast<unsigned>(str_hash("torpedo") & 0xFFFFu);
    return ::get_id(get_current_ammunition(), type_tag);
};
