#ifndef WEAPONMISSILE_H
#define WEAPONMISSILE_H

#include "model/weapon/Weapon.h"
#include "model/point/Point.h"

constexpr unsigned fire_rate_missile = 100;
constexpr unsigned fire_range_missile = 100;
constexpr unsigned ammunition_missile = 100;
constexpr unsigned cost_missile = 100;
constexpr unsigned damage_missile = 50;

/**
 * @brief Ракета (конкретная реализация оружия)
 *
 * @details
 * Возвращает область поражения как набор случайных точек около центра.
 */
class WeaponMissile: public DefaultWeapon {
    public:
        explicit WeaponMissile(unsigned current_ammunition_ = ammunition_missile);
        std::vector<Point>& get_shoot_area(Point center) const override;
        unsigned get_id() const override;
};

#endif
