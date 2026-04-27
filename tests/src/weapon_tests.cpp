#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <vector>

#include "model/point/Point.h"

#include "weapon/cannon/include/WeaponCannon.h"
#include "weapon/missile/include/WeaponMissile.h"
#include "weapon/torpedo/include/WeaponTorpedo.h"

TEST_CASE("DefaultWeapon: shoot/reload", "[weapon]") {
    WeaponCannon w( 2);
    REQUIRE(w.can_shoot());
    REQUIRE(w.get_current_ammunition() == 2);

    w.shoot();
    REQUIRE_FALSE(w.can_shoot());
    REQUIRE(w.get_current_ammunition() == 0);

    REQUIRE(w.reload());
    REQUIRE(w.get_current_ammunition() == w.get_max_ammunition());

    REQUIRE_FALSE(w.reload());
}

TEST_CASE("DefaultWeapon: area по умолчанию", "[weapon]") {
    WeaponCannon w;
    Point c(3, 4);
    auto& area = w.get_shoot_area(c);
    REQUIRE(area.size() == 1);
    REQUIRE(area[0].get_x() == 3);
    REQUIRE(area[0].get_y() == 4);
}

TEST_CASE("Оружия: get_shoot_area() + get_id()", "[weapon]") {
    WeaponCannon cannon(10);
    WeaponMissile missile(10);
    WeaponTorpedo torpedo(10);

    Point center(0, 0);

    auto& a1 = cannon.get_shoot_area(center);
    REQUIRE(a1.size() == 1);

    auto& a2 = missile.get_shoot_area(center);
    REQUIRE(a2.size() >= 3);

    auto& a3 = torpedo.get_shoot_area(center);
    REQUIRE(a3.size() >= 9);

    const unsigned id1 = cannon.get_id();
    cannon.shoot();
    const unsigned id2 = cannon.get_id();
    REQUIRE(id1 != id2);
}

TEST_CASE("DefaultWeapon: все методы получения параметров", "[weapon]") {
    WeaponCannon w(5);
    
    REQUIRE(w.get_fire_rate() == 100);
    REQUIRE(w.get_fire_range() == 100);
    REQUIRE(w.get_max_ammunition() == 100);
    REQUIRE(w.get_current_ammunition() == 5);
    REQUIRE(w.get_cost() == 100);
    REQUIRE(w.get_damage() == 50);
    
    w.set_current_ammunition(10);
    REQUIRE(w.get_current_ammunition() == 10);
}

TEST_CASE("DefaultWeapon: copy и move конструкторы", "[weapon]") {
    WeaponCannon w1(5);
    WeaponCannon w2(w1);
    REQUIRE(w2.get_current_ammunition() == 5);
    
    WeaponMissile w3(10);
    WeaponMissile w4(std::move(w3));
    REQUIRE(w4.get_current_ammunition() == 10);
}

TEST_CASE("DefaultWeapon: операторы присваивания", "[weapon]") {
    WeaponCannon w1(5);
    WeaponMissile w2(10);
    
    WeaponCannon w3;
    w3 = w1;
    REQUIRE(w3.get_current_ammunition() == 5);
    
    WeaponMissile w4;
    w4 = std::move(w2);
    REQUIRE(w4.get_current_ammunition() == 10);
}

TEST_CASE("WeaponMissile: область поражения", "[weapon][missile]") {
    WeaponMissile missile(10);
    Point center(5, 5);
    
    auto& area = missile.get_shoot_area(center);
    REQUIRE(area.size() >= 3);
    REQUIRE(area.size() <= 5);

    for (const auto& p : area) {
        REQUIRE(std::abs(p.get_x() - center.get_x()) <= 10);
        REQUIRE(std::abs(p.get_y() - center.get_y()) <= 10);
    }
}

TEST_CASE("WeaponTorpedo: область поражения", "[weapon][torpedo]") {
    WeaponTorpedo torpedo(10);
    Point center(10, 10);
    
    auto& area = torpedo.get_shoot_area(center);
    REQUIRE(area.size() >= 9);
    
    bool found_center = false;
    for (const auto& point : area) {
        if (point.get_x() == 10 && point.get_y() == 10) {
            found_center = true;
            break;
        }
    }
    REQUIRE(found_center);
}

TEST_CASE("Weapon: уникальные ID", "[weapon]") {
    WeaponCannon cannon1(5);
    WeaponCannon cannon2(3);
    WeaponMissile missile(7);
    WeaponTorpedo torpedo(4);
    
    unsigned id1 = cannon1.get_id();
    unsigned id2 = cannon2.get_id();
    unsigned id3 = missile.get_id();
    unsigned id4 = torpedo.get_id();
    
    REQUIRE(id1 != id2);
    REQUIRE(id1 != id3);
    REQUIRE(id1 != id4);
    REQUIRE(id2 != id3);
    REQUIRE(id2 != id4);
    REQUIRE(id3 != id4);
}
