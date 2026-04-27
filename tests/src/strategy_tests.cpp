#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <vector>

#include "MissionRepositoryImpl.h"
#include "model/repository/MissionRepository.h"
#include "strategy/AttackStrategy.h"
#include "RandomAttackStrategy.h"

#include "model/ship/Ship.h"
#include "MilitaryShip.h"


MissionRepositoryImpl make_repo(std::size_t pirates, std::size_t imperials) {
    Mission_params mp{};
    MissionRepositoryImpl repo(mp);
    for (std::size_t i = 0; i < pirates; ++i) {
        auto ship = std::make_unique<MilitaryShip>(
            "P" + std::to_string(i),
            100,
            50,
            1'000'000,
            1,
            "pirate",
            1.0);
        repo.add_ship(std::move(ship), Point(static_cast<int>(2 * i), std::rand() % pirates), true);
    }
    for (std::size_t i = 0; i < imperials; ++i) {
        auto ship = std::make_unique<MilitaryShip>(
            "I" + std::to_string(i),
            100,
            50,
            1'000'000,
            1,
            "imperial",
            1.0);
        repo.add_ship(std::move(ship), Point(static_cast<int>(2 * i + 1), std::rand() % imperials), false);
    }
    return std::move(repo);
};

TEST_CASE("DefaultAttackStrategy, RandomAttackStrategy: auto_shoot", "[strategy]") {
    std::vector<Ship*> ships;
    MissionRepositoryImpl repo_ = make_repo(100, 100);
    DefaultAttackStrategy strategy(repo_);
    std::unique_ptr<DefaultAttackStrategy> random_strategy = std::make_unique<RandomAttackStrategy>(repo_);
    auto targets = strategy.auto_shoot(10, false);
    auto rand_targets = random_strategy->auto_shoot(10, false);
    REQUIRE(!targets.empty()); 
}

