#pragma once
#include "ShipDto.h"
#include "ShipUnitDto.h"
#include "WeaponDto.h"
#include <string>
#include <vector>
#include <map>

/**
 * @brief Параметры типа корабля 
 */
struct ShipParamsDto {
    unsigned max_velocity;
    unsigned max_HP;
    unsigned cost;
};

/**
 * @brief Параметры типа оружия 
 */
struct WeaponParamsDto {
    unsigned fire_rate;
    unsigned fire_range;
    unsigned max_ammunition;
    unsigned cost;
    unsigned damage;
};

/**
 * @brief Параметры миссии 
 */
struct MissionParamsDto {
    unsigned budget;
    unsigned spent_budget;
    unsigned total_cargo_weight;
    unsigned goal_weight;
    unsigned lost_weight;
    unsigned delivered_weight;
    
    struct Base {
        int x;
        int y;
    };
    
    /**
     * @brief Геометрия карты миссии (размеры и центр), по сути прямоугольник
     */
    struct Map {
        int dx;
        int dy;
        Base center;
    };
    std::map<std::string, Base> bases;
    std::map<std::string, Base> pirates_bases;
    Map map;
};

/**
 * @brief Полное состояние игры (DTO)
 *
 */
struct GameStateDto {
    std::vector<ShipDto> ships;
    std::vector<ShipUnitDTO> ship_units;
    std::vector<WeaponDto> weapons;
    std::map<std::string, ShipParamsDto> ships_params;
    std::map<std::string, WeaponParamsDto> weapons_params;
    MissionParamsDto mission;
};
