#ifndef WEAPONCANNON_H
#define WEAPONCANNON_H

#include "model/weapon/Weapon.h"
#include "model/point/Point.h"

constexpr unsigned fire_rate_cannon = 100;
constexpr unsigned fire_range_cannon = 100;
constexpr unsigned ammunition_cannon = 100;
constexpr unsigned cost_cannon = 100;
constexpr unsigned damage_cannon = 50;

/**
 * @brief Пушка (конкретная реализация оружия)
 *
 * @details
 * Стреляет ровно по одной точке (центр).
 */
class WeaponCannon: public DefaultWeapon {
    public:
        explicit WeaponCannon(unsigned current_ammunition_ = ammunition_cannon);
        std::vector<Point>& get_shoot_area(Point center) const override;
        unsigned get_id() const override;       
};

#endif
