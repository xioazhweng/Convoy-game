#ifndef WEAPONTORPEDO_H
#define WEAPONTORPEDO_H

#include "model/weapon/Weapon.h"
#include "model/point/Point.h"
#include <vector>

constexpr unsigned fire_rate_torpedo = 100;
constexpr unsigned fire_range_torpedo = 100;
constexpr unsigned ammunition_torpedo = 100;
constexpr unsigned cost_torpedo = 100;
constexpr unsigned damage_torpedo = 50;

/**
 * @brief Торпеда (конкретная реализация оружия)
 *
 * @details
 * Возвращает область поражения как точки по окружности вокруг центра (+ сам центр).
 */
class WeaponTorpedo: public DefaultWeapon {
    public:
        explicit WeaponTorpedo(unsigned current_ammunition_ = ammunition_torpedo);
        std::vector<Point>& get_shoot_area(Point center) const override;
        unsigned get_id() const override;
};

#endif
