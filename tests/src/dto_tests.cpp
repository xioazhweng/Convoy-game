#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>
#include <map>

#include "dto/GameStateDto.h"
#include "dto/ShipDto.h"
#include "dto/WeaponDto.h"
#include "dto/ShipUnitDto.h"

TEST_CASE("ShipDto: базовые поля", "[dto][ship]") {
    ShipDto dto;
    dto.type = "military";
    dto.callsign = "M1";
    dto.captain = "Captain";
    dto.current_velocity = 50;
    dto.current_HP = 200;
    
    REQUIRE(dto.type == "military");
    REQUIRE(dto.callsign == "M1");
    REQUIRE(dto.captain == "Captain");
    REQUIRE(dto.current_velocity == 50);
    REQUIRE(dto.current_HP == 200);
    REQUIRE(dto.params.empty());
}

TEST_CASE("ShipDto: с параметрами", "[dto][ship]") {
    ShipDto dto;
    dto.type = "transport";
    dto.callsign = "T1";
    dto.params["max_cargo_weight"] = "150";
    dto.params["current_cargo_weight"] = "75";
    dto.params["bow"] = "12345";
    
    REQUIRE(dto.params.size() == 3);
    REQUIRE(dto.params.at("max_cargo_weight") == "150");
    REQUIRE(dto.params.at("current_cargo_weight") == "75");
    REQUIRE(dto.params.at("bow") == "12345");
}

TEST_CASE("WeaponDto: базовые поля", "[dto][weapon]") {
    WeaponDto dto;
    dto.id = 12345;
    dto.type = "cannon";
    dto.current_ammunition = 50;
    
    REQUIRE(dto.id == 12345);
    REQUIRE(dto.type == "cannon");
    REQUIRE(dto.current_ammunition == 50);
}

TEST_CASE("ShipUnitDTO: базовые поля", "[dto][ship_unit]") {
    ShipUnitDTO dto;
    dto.callsign = "M1";
    dto.x = 100;
    dto.y = 200;
    dto.side = "imperial";
    
    REQUIRE(dto.callsign == "M1");
    REQUIRE(dto.x == 100);
    REQUIRE(dto.y == 200);
    REQUIRE(dto.side == "imperial");
}

TEST_CASE("ShipParamsDto: базовые поля", "[dto][ship_params]") {
    ShipParamsDto dto;
    dto.max_velocity = 100;
    dto.max_HP = 200;
    dto.cost = 1000;
    
    REQUIRE(dto.max_velocity == 100);
    REQUIRE(dto.max_HP == 200);
    REQUIRE(dto.cost == 1000);
}

TEST_CASE("WeaponParamsDto: базовые поля", "[dto][weapon_params]") {
    WeaponParamsDto dto;
    dto.fire_rate = 300;
    dto.fire_range = 300;
    dto.max_ammunition = 300;
    dto.cost = 300;
    dto.damage = 300;
    
    REQUIRE(dto.fire_rate == 300);
    REQUIRE(dto.fire_range == 300);
    REQUIRE(dto.max_ammunition == 300);
    REQUIRE(dto.cost == 300);
    REQUIRE(dto.damage == 300);
}

TEST_CASE("MissionParamsDto::Base: базовые поля", "[dto][mission_params]") {
    MissionParamsDto::Base base;
    base.x = 50;
    base.y = 100;
    
    REQUIRE(base.x == 50);
    REQUIRE(base.y == 100);
}

TEST_CASE("MissionParamsDto::Map: базовые поля", "[dto][mission_params]") {
    MissionParamsDto::Map map;
    map.dx = 20;
    map.dy = 15;
    map.center = {50, 60};
    
    REQUIRE(map.dx == 20);
    REQUIRE(map.dy == 15);
    REQUIRE(map.center.x == 50);
    REQUIRE(map.center.y == 60);
}

TEST_CASE("MissionParamsDto: полная структура", "[dto][mission_params]") {
    MissionParamsDto dto;
    dto.budget = 1000;
    dto.spent_budget = 100;
    dto.total_cargo_weight = 75;
    dto.goal_weight = 100;
    dto.lost_weight = 10;
    dto.delivered_weight = 65;
    
    dto.bases["base_a"] = {0, 0};
    dto.bases["base_b"] = {100, 0};
    dto.pirates_bases["pirate_base"] = {50, 50};
    dto.map.dx = 20;
    dto.map.dy = 15;
    dto.map.center = {50, 50};
    
    REQUIRE(dto.budget == 1000);
    REQUIRE(dto.spent_budget == 100);
    REQUIRE(dto.total_cargo_weight == 75);
    REQUIRE(dto.goal_weight == 100);
    REQUIRE(dto.lost_weight == 10);
    REQUIRE(dto.delivered_weight == 65);
    
    REQUIRE(dto.bases.size() == 2);
    REQUIRE(dto.bases.at("base_a").x == 0);
    REQUIRE(dto.bases.at("base_b").y == 0);
    
    REQUIRE(dto.pirates_bases.size() == 1);
    REQUIRE(dto.pirates_bases.at("pirate_base").x == 50);
    
    REQUIRE(dto.map.dx == 20);
    REQUIRE(dto.map.dy == 15);
    REQUIRE(dto.map.center.x == 50);
}

TEST_CASE("GameStateDto: полная структура", "[dto][game_state]") {
    GameStateDto dto;
    
    ShipDto ship1;
    ship1.type = "military";
    ship1.callsign = "M1";
    dto.ships.push_back(ship1);
    
    ShipUnitDTO unit1;
    unit1.callsign = "M1";
    unit1.x = 100;
    unit1.y = 200;
    unit1.side = "imperial";
    dto.ship_units.push_back(unit1);
    
    WeaponDto weapon1;
    weapon1.id = 12345;
    weapon1.type = "cannon";
    weapon1.current_ammunition = 50;
    dto.weapons.push_back(weapon1);
    
    ShipParamsDto ship_params;
    ship_params.max_velocity = 100;
    dto.ships_params["military"] = ship_params;
    
    WeaponParamsDto weapon_params;
    weapon_params.fire_rate = 300;
    dto.weapons_params["cannon"] = weapon_params;
    
    dto.mission.budget = 1000;
    
    REQUIRE(dto.ships.size() == 1);
    REQUIRE(dto.ship_units.size() == 1);
    REQUIRE(dto.weapons.size() == 1);
    REQUIRE(dto.ships_params.size() == 1);
    REQUIRE(dto.weapons_params.size() == 1);
    
    REQUIRE(dto.ships[0].callsign == "M1");
    REQUIRE(dto.ship_units[0].callsign == "M1");
    REQUIRE(dto.weapons[0].id == 12345);
    REQUIRE(dto.ships_params.at("military").max_velocity == 100);
    REQUIRE(dto.weapons_params.at("cannon").fire_rate == 300);
    REQUIRE(dto.mission.budget == 1000);
}