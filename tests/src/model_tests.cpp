#include <catch2/catch_test_macros.hpp>

#include <memory>

#include "model/place/Place.h"
#include "model/point/Point.h"
#include "model/repository/MissionRepository.h"
#include "model/ship/Military.h"
#include "model/ship/Ship.h"
#include "model/ship/Transport.h"

#include "weapon/cannon/include/WeaponCannon.h"

TEST_CASE("Point: базовые операции", "[model][point]") {
    Point p;
    REQUIRE(p.get_x() == 0);
    REQUIRE(p.get_y() == 0);

    p.set_point(10, -5);
    REQUIRE(p.get_x() == 10);
    REQUIRE(p.get_y() == -5);

    p.add_x(7);
    REQUIRE(p.get_x() == 17);

    Point q(17, -5);
    REQUIRE(p == q);

    Point r(0, 0);
    REQUIRE(p != r);
}

TEST_CASE("Place: преобразования", "[model][place]") {
    REQUIRE(get_place_name(BOW) == "bow");
    REQUIRE(get_place_name(STERN) == "stern");
    REQUIRE(get_place_name(STARBOARD) == "starboard");
    REQUIRE(get_place_name(PORTSIDE) == "portside");
    REQUIRE(get_place_name(NONE) == "none");

    REQUIRE(get_place_id("bow") == BOW);
    REQUIRE(get_place_id("BoW") == BOW);
    REQUIRE(get_place_id("invalid") == NONE);
}

TEST_CASE("MissionMap: попадание в область", "[model][mission]") {
    MissionMap map;
    map.dx = 5;
    map.dy = 3;
    map.center = Point(10, 10);

    REQUIRE(map.is_in_mission_area(Point(10, 10)));
    REQUIRE(map.is_in_mission_area(Point(15, 10)));
    REQUIRE(map.is_in_mission_area(Point(5, 10)));
    REQUIRE(map.is_in_mission_area(Point(10, 13)));
    REQUIRE(map.is_in_mission_area(Point(10, 7)));
    REQUIRE_FALSE(map.is_in_mission_area(Point(16, 10)));
    REQUIRE_FALSE(map.is_in_mission_area(Point(10, 14)));
}

TEST_CASE("DefaultShip: урон и флаги", "[model][ship]") {
    DefaultShip s("S1", 100, 80, 200, 777, "Cap");
    REQUIRE_FALSE(s.is_destroyed());
    REQUIRE(s.get_current_hp() == 200);

    s.take_damage(50);
    REQUIRE(s.get_current_hp() == 150);
    REQUIRE_FALSE(s.is_destroyed());

    s.take_damage(1000);
    REQUIRE(s.get_current_hp() == 0);
    REQUIRE(s.is_destroyed());
}

TEST_CASE("DefaultTransport: груз", "[model][transport]") {
    DefaultTransport t(0.5, 100, 0);
    REQUIRE(t.get_actual_max_velocity() == 50);
    REQUIRE(t.can_load_cargo(100));
    REQUIRE_FALSE(t.can_load_cargo(101));

    t.add_cargo_weight(40);
    REQUIRE(t.get_current_cargo_weight() == 40);
    REQUIRE(t.can_load_cargo(60));
    REQUIRE_FALSE(t.can_load_cargo(61));

    t.add_cargo_weight(1000);
    REQUIRE(t.get_current_cargo_weight() == 40);
}

TEST_CASE("DefaultMilitary: простая работа с оружием", "[model][military]") {
    DefaultMilitary m(1.0);
    REQUIRE(m.weapon_count() == 0);
    REQUIRE_FALSE(m.can_shoot(0));
    REQUIRE(m.get_weapon(0) == nullptr);

    auto cannon = std::make_unique<WeaponCannon>(2);
    REQUIRE(m.set_weapon(BOW, std::move(cannon)));
    REQUIRE(m.weapon_count() == 1);
    REQUIRE(m.get_weapon(0) != nullptr);
    REQUIRE(m.can_shoot(0));

    auto cannon2 = std::make_unique<WeaponCannon>(2);
    REQUIRE_FALSE(m.set_weapon(BOW, std::move(cannon2)));

    auto cannon3 = std::make_unique<WeaponCannon>(1);
    REQUIRE(m.replace_weapon(BOW, std::move(cannon3)));

    auto w = m.get_weapon_by_place(BOW);
    REQUIRE(w.size() == 1);

    auto removed = m.remove_weapon(BOW);
    REQUIRE(removed != nullptr);
    REQUIRE(m.weapon_count() == 0);
}

TEST_CASE("DefaultMilitary: get_actual_max_velocity и beta", "[model][military]") {
    DefaultMilitary m(0.8);
    REQUIRE(m.get_actual_max_velocity() == 80);
    REQUIRE(m.get_beta() == 0.8);
}

TEST_CASE("DefaultMilitary: множественные оружия", "[model][military]") {
    DefaultMilitary m(1.0);
    auto cannon1 = std::make_unique<WeaponCannon>(5);
    auto cannon2 = std::make_unique<WeaponCannon>(3);
    auto cannon3 = std::make_unique<WeaponCannon>(2);
    
    REQUIRE(m.set_weapon(BOW, std::move(cannon1)));
    REQUIRE(m.set_weapon(STERN, std::move(cannon2)));
    REQUIRE(m.set_weapon(STARBOARD, std::move(cannon3)));
    
    REQUIRE(m.weapon_count() == 3);
    REQUIRE(m.can_shoot(0));
    REQUIRE(m.can_shoot(1));
    REQUIRE(m.can_shoot(2));
    REQUIRE_FALSE(m.can_shoot(3));
}

TEST_CASE("DefaultTransport: базовые операции", "[model][transport]") {
    DefaultTransport t(0.5, 100, 0);
    REQUIRE(t.get_max_cargo_weight() == 100);
    REQUIRE(t.get_current_cargo_weight() == 0);
    REQUIRE(t.get_alpha() == 0.5);
    REQUIRE(t.get_actual_max_velocity() == 50);
    
    t.set_current_cargo_weight(30);
    REQUIRE(t.get_current_cargo_weight() == 30);
}

TEST_CASE("Ship: clone и получение параметров", "[model][ship]") {
    DefaultShip s("TestShip", 100, 50, 200, 1000, "Captain");
    
    REQUIRE(s.get_callsign() == "TestShip");
    REQUIRE(s.get_max_velocity() == 100);
    REQUIRE(s.get_current_velocity() == 50);
    REQUIRE(s.get_max_hp() == 200);
    REQUIRE(s.get_current_hp() == 200);
    REQUIRE(s.get_cost() == 1000);
    REQUIRE(s.get_captain() == "Captain");
    REQUIRE(s.get_velocity() == 50);
    
    auto clone_ptr = s.clone();
    REQUIRE(clone_ptr != nullptr);
    Ship* cloned_ship = dynamic_cast<Ship*>(clone_ptr.get());
    REQUIRE(cloned_ship != nullptr);
    REQUIRE(cloned_ship->get_callsign() == "TestShip");
    REQUIRE(cloned_ship->get_current_hp() == 200);
}

TEST_CASE("Ship: take_damage alias", "[model][ship]") {
    DefaultShip s("TestShip", 100, 50, 200, 1000, "Captain");
    REQUIRE(s.get_current_hp() == 200);
    
    s.take_damage(50);
    REQUIRE(s.get_current_hp() == 150);
    REQUIRE_FALSE(s.is_destroyed());
    
    s.take_damage(200);
    REQUIRE(s.get_current_hp() == 0);
    REQUIRE(s.is_destroyed());
}

TEST_CASE("DefaultShip: set_max_hp/set_current_hp/set_cost", "[model][ship]") {
    DefaultShip s("TestShip", 100, 50, 200, 1000, "Captain");

    s.set_cost(123);
    REQUIRE(s.get_cost() == 123);
    s.set_max_hp(100);
    REQUIRE(s.get_max_hp() == 100);
    s.set_current_hp(200);
    REQUIRE(s.get_current_hp() == 100);
    s.set_current_hp(77);
    REQUIRE(s.get_current_hp() == 77);
}

TEST_CASE("Mission_params и MissionMap", "[model][mission]") {
    Mission_params params;
    params.budget = 1000;
    params.goal_weight = 100;
    params.delivered_weight = 50;
    params.total_cargo_weight = 75;
    params.A_base = Point(0, 0);
    params.B_base = Point(100, 0);
    
    REQUIRE(params.budget == 1000);
    REQUIRE(params.goal_weight == 100);
    
    MissionMap map;
    map.dx = 20;
    map.dy = 15;
    map.center = Point(50, 50);
    
    REQUIRE(map.is_in_mission_area(Point(50, 50)));
    REQUIRE(map.is_in_mission_area(Point(70, 50)));
    REQUIRE(map.is_in_mission_area(Point(30, 50)));
    REQUIRE(map.is_in_mission_area(Point(50, 65)));
    REQUIRE(map.is_in_mission_area(Point(50, 35)));
    REQUIRE_FALSE(map.is_in_mission_area(Point(71, 50)));
    REQUIRE_FALSE(map.is_in_mission_area(Point(50, 66)));
}

TEST_CASE("info struct", "[model][repository]") {
    auto ship = std::make_unique<DefaultShip>("InfoShip", 100, 50, 200, 1000, "InfoCaptain");
    Point coords(10, 20);
    
    info inf("InfoShip", std::move(ship), coords, "InfoCaptain");
    REQUIRE(inf.callsign == "InfoShip");
    REQUIRE(inf.coords.get_x() == 10);
    REQUIRE(inf.coords.get_y() == 20);
    REQUIRE(inf.ship != nullptr);
    
    info default_info;
    REQUIRE(default_info.callsign == "I1");
    REQUIRE(default_info.coords.get_x() == 0);
    REQUIRE(default_info.coords.get_y() == 0);
}
