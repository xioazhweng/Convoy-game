#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <optional>

#include "repository/include/MissionRepositoryImpl.h"
#include "ship/military_ship/include/MilitaryShip.h"

TEST_CASE("MissionRepositoryImpl: add/find/remove/move", "[repo]") {
    Mission_params mp{};
    mp.budget = 1000;
    mp.goal_weight = 10;
    mp.delivered_weight = 0;
    mp.map.dx = 100;
    mp.map.dy = 100;
    mp.map.center = Point(0, 0);

    MissionRepositoryImpl repo(mp);
    REQUIRE(repo.get_imperials().empty());
    REQUIRE(repo.get_pirates().empty());

    repo.add_ship(std::make_unique<MilitaryShip>("I1", 100, 5, 100, 1, "cap", 1.0), Point(10, 0),false);
    repo.add_ship(std::make_unique<MilitaryShip>("P1", 100, 7, 100, 1, "cap", 1.0), Point(-10, 0),true);

    REQUIRE(repo.get_imperials().size() == 1);
    REQUIRE(repo.get_pirates().size() == 1);

    REQUIRE(repo.find_by_point(Point(10, 0), false) == "I1");
    REQUIRE(repo.find_by_point(Point(-10, 0), true) == "P1");

    REQUIRE(repo.find_min_velocity(false) == 5);
    REQUIRE(repo.find_min_velocity(true) == 7);

    repo.move_by_x(3, false);
    REQUIRE(repo.find_by_point(Point(13, 0), false) == "I1");

    REQUIRE(repo.find_max_x_point_ship() == "I1");

    repo.remove_ship("I1", false);
    REQUIRE(repo.get_imperials().empty());
}

TEST_CASE("MissionRepositoryImpl: конструктор по умолчанию", "[repo]") {
    MissionRepositoryImpl repo;
    REQUIRE(repo.get_imperials().empty());
    REQUIRE(repo.get_pirates().empty());
}

TEST_CASE("MissionRepositoryImpl: get_mission_params", "[repo]") {
    Mission_params mp{};
    mp.budget = 1000;
    mp.goal_weight = 100;
    mp.delivered_weight = 50;
    mp.total_cargo_weight = 75;
    mp.map.dx = 100;
    mp.map.dy = 100;
    mp.map.center = Point(0, 0);
    mp.A_base = Point(0, 0);
    mp.B_base = Point(100, 0);
    
    MissionRepositoryImpl repo(mp);
    auto& params = repo.get_mission_params();
    
    REQUIRE(params.budget == 1000);
    REQUIRE(params.goal_weight == 100);
    REQUIRE(params.delivered_weight == 50);
    REQUIRE(params.total_cargo_weight == 75);
    REQUIRE(params.map.dx == 100);
    REQUIRE(params.map.dy == 100);
    REQUIRE(params.map.center.get_x() == 0);
    REQUIRE(params.map.center.get_y() == 0);
    REQUIRE(params.A_base.get_x() == 0);
    REQUIRE(params.A_base.get_y() == 0);
    REQUIRE(params.B_base.get_x() == 100);
    REQUIRE(params.B_base.get_y() == 0);
}

TEST_CASE("MissionRepositoryImpl: empty tables", "[repo]") {
    Mission_params mp{};
    mp.map.center = Point(0, 0);
    MissionRepositoryImpl repo(mp);
    
    REQUIRE(repo.get_imperials().empty());
    REQUIRE(repo.get_pirates().empty());
    
    REQUIRE(repo.get_imperials().size() == 0);
    REQUIRE(repo.get_pirates().size() == 0);
}

TEST_CASE("MissionRepositoryImpl: работа с пиратскими кораблями", "[repo]") {
    Mission_params mp{};
    mp.map.center = Point(0, 0);
    MissionRepositoryImpl repo(mp);
    
    repo.add_ship(std::make_unique<MilitaryShip>("P1", 100, 5, 100, 1, "cap", 1.0), Point(-10, 0), true);
    repo.add_ship(std::make_unique<MilitaryShip>("P2", 100, 7, 100, 1, "cap", 1.0), Point(-20, 0), true);
    
    REQUIRE(repo.get_pirates().size() == 2);
    REQUIRE(repo.get_imperials().empty());
    
    REQUIRE(repo.find_by_point(Point(-10, 0), true) == "P1");
    REQUIRE(repo.find_by_point(Point(-20, 0), true) == "P2");
    
    REQUIRE(repo.find_min_velocity(true) == 5);
    
    repo.move_by_x(5, true);
    REQUIRE(repo.find_by_point(Point(-5, 0), true) == "P1");
    REQUIRE(repo.find_by_point(Point(-15, 0), true) == "P2");
    
    repo.remove_ship("P1", true);
    REQUIRE(repo.get_pirates().size() == 1);
}

TEST_CASE("MissionRepositoryImpl: find_max_x_point_ship с пустыми таблицами", "[repo]") {
    Mission_params mp{};
    mp.map.center = Point(0, 0);
    MissionRepositoryImpl repo(mp);
    
    auto result = repo.find_max_x_point_ship();
    REQUIRE(result == std::nullopt);
}

TEST_CASE("MissionRepositoryImpl: find_by_point с пустыми таблицами", "[repo]") {
    Mission_params mp{};
    mp.map.center = Point(0, 0);
    MissionRepositoryImpl repo(mp);
    
    auto result = repo.find_by_point(Point(0, 0), false);
    REQUIRE(result == std::nullopt);
    
    result = repo.find_by_point(Point(0, 0), true);
    REQUIRE(result == std::nullopt);
}

TEST_CASE("MissionRepositoryImpl: find_min_velocity с пустыми таблицами", "[repo]") {
    Mission_params mp{};
    mp.map.center = Point(0, 0);
    MissionRepositoryImpl repo(mp);
    
    auto result = repo.find_min_velocity(false);
    REQUIRE(result == 0);
    
    result = repo.find_min_velocity(true);
    REQUIRE(result == 0);
}

TEST_CASE("MissionRepositoryImpl: find_n_nearest", "[repo]") {
    Mission_params mp{};
    mp.map.center = Point(0, 0);
    MissionRepositoryImpl repo(mp);

    repo.add_ship(std::make_unique<MilitaryShip>("P1", 100, 5, 100, 1, "cap", 1.0), Point(3, 4),  true);
    repo.add_ship(std::make_unique<MilitaryShip>("P2", 100, 5, 100, 1, "cap", 1.0), Point(6, 8),  true);
    repo.add_ship(std::make_unique<MilitaryShip>("P3", 100, 5, 100, 1, "cap", 1.0), Point(1, 1), true);

    const auto nearest2 = repo.find_n_nearest(Point(0, 0), 2, true);
    REQUIRE(nearest2.size() == 2);
    auto it = nearest2.begin();
    REQUIRE(it->first == 2u);
    REQUIRE(it->second == Point(1, 1));
    ++it;
    REQUIRE(it->first == 25u);
    REQUIRE(it->second == Point(3, 4));

    const auto nearest10 = repo.find_n_nearest(Point(0, 0), 10, true);
    REQUIRE(nearest10.size() == 3);
}

TEST_CASE("MissionRepositoryImpl: find_opposite_ships_in_fire_range", "[repo]") {
    Mission_params mp{};
    mp.map.center = Point(0, 0);
    MissionRepositoryImpl repo(mp);

    repo.add_ship(std::make_unique<MilitaryShip>("P0", 100, 5, 100, 1, "cap", 1.0), Point(0, 0), true);
    repo.add_ship(std::make_unique<MilitaryShip>("I1", 100, 5, 100, 1, "cap", 1.0), Point(3, 4),  false);
    repo.add_ship(std::make_unique<MilitaryShip>("I2", 100, 5, 100, 1, "cap", 1.0), Point(20, 0), false);

    const auto targets = repo.find_opposite_ships_in_fire_range(Point(0, 0), 5, true);
    REQUIRE(targets.size() == 1);
    REQUIRE(targets[0] == Point(3, 4));
}
