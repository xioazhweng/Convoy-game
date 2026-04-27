#pragma once
#include <string>
#include <map>

/**
 * @brief DTO для корабля 
 */
struct ShipDto {
    std::string type;
    std::string callsign;
    std::string captain;
    unsigned current_velocity;
    unsigned current_HP;
    std::map<std::string, std::string> params;
    // current_cargo_weight если надо
    // параметры оружий
};
