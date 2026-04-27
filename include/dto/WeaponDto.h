#pragma once
#include <string>

/**
 * @brief DTO оружия 
 *
 * @details
 * Класс кораблей не зависит от класса оружия. Было принято решения, что корабль буде хранить id нужного ему
 * оружия, и само оружие хранит свой id.
 */
struct WeaponDto {
    unsigned id;
    std::string type;
    unsigned current_ammunition;
};
