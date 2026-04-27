#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <mutex>
#include <set>
#include <thread>

#include "repository/include/MissionRepositoryImpl.h"
#include "service/mission_service/include/MissionServiceImpl.h"
#include "strategy/AttackStrategy.h"
#include "ship/military_ship/include/MilitaryShip.h"
#include "ship/transport_ship/include/TransportShip.h"

#include "weapon/cannon/include/WeaponCannon.h"

namespace {
class RecordingAttackStrategy final : public AttackStrategy {
    std::string target_;
    mutable std::mutex m_;
    mutable std::set<std::thread::id> tids_;

    void record_tid() const {
        std::lock_guard<std::mutex> lk(m_);
        tids_.insert(std::this_thread::get_id());
    }

public:
    explicit RecordingAttackStrategy(std::string target) : target_(std::move(target)) {}

    [[nodiscard]] std::size_t thread_count() const {
        std::lock_guard<std::mutex> lk(m_);
        return tids_.size();
    }

    std::vector<std::string> auto_shoot(unsigned range, bool is_pirate) override {
        (void)range;
        (void)is_pirate;
        record_tid();
        return {target_};
    }

    std::vector<std::string> auto_shoot(const Point& p, unsigned range, bool is_pirate) override {
        (void)p;
        (void)range;
        (void)is_pirate;
        record_tid();
        return {target_};
    }
};
} 

TEST_CASE("MissionServiceImpl: покупка/продажа корабля", "[service]") {
    Mission_params mp{};
    mp.budget = 1000;
    mp.goal_weight = 10;
    mp.delivered_weight = 0;
    mp.total_cargo_weight = 0;
    mp.map.dx = 100;
    mp.map.dy = 100;
    mp.map.center = Point(0, 0);

    MissionRepositoryImpl repo(mp);
    DefaultAttackStrategy strat(repo);
    MissionServiceImpl svc(repo, strat);

    const bool ok = svc.buy_ship(
        std::make_unique<TransportShip>("I1", 100, 5, 100, 100, "cap", 1.0, 50, 0),
        Point(0, 0));
    REQUIRE(ok);
    REQUIRE(repo.get_imperials().size() == 1);
    REQUIRE(repo.get_mission_params().spent_budged == 100);
    REQUIRE(repo.get_mission_params().budget == 1000);

    REQUIRE(svc.sell_ship("I1"));
    REQUIRE(repo.get_imperials().empty());
    REQUIRE(repo.get_mission_params().budget == 1100);
}

TEST_CASE("MissionServiceImpl: cargo", "[service]") {
    Mission_params mp{};
    mp.budget = 0;
    mp.goal_weight = 10;
    mp.delivered_weight = 10;
    mp.total_cargo_weight = 0;
    mp.map.dx = 100;
    mp.map.dy = 100;
    mp.map.center = Point(0, 0);

    MissionRepositoryImpl repo(mp);
    DefaultAttackStrategy strat(repo);
    MissionServiceImpl svc(repo, strat);

    repo.add_ship(std::make_unique<TransportShip>("T1", 100, 1, 100, 0, "cap", 1.0, 10, 0), Point(0, 0), false);
    REQUIRE(svc.load_cargo("T1", 5, 0));
    REQUIRE(repo.get_mission_params().total_cargo_weight >= 5);
    REQUIRE(svc.unload_cargo("T1"));

    REQUIRE(svc.check_cargo_goal());
}

TEST_CASE("MissionServiceImpl: оружие + атака", "[service]") {
    Mission_params mp{};
    mp.budget = 0;
    mp.goal_weight = 0;
    mp.delivered_weight = 0;
    mp.total_cargo_weight = 0;
    mp.map.dx = 100;
    mp.map.dy = 100;
    mp.map.center = Point(0, 0);

    MissionRepositoryImpl repo(mp);
    DefaultAttackStrategy strat(repo);
    MissionServiceImpl svc(repo, strat);

    repo.add_ship(std::make_unique<MilitaryShip>("I1", 100, 1, 100, 0, "cap", 1.0), Point(0, 0), false);
    repo.add_ship(std::make_unique<MilitaryShip>("P1", 100, 1, 1, 0, "cap", 1.0), Point(5, 5), true);

    REQUIRE(svc.install_weapon("I1", std::make_unique<WeaponCannon>(10), BOW));
    REQUIRE(svc.attack("I1", BOW, "P1", false));
    REQUIRE(repo.get_pirates().empty());

    REQUIRE(svc.uninstall_weapon("I1", BOW));
}

TEST_CASE("MissionServiceImpl: buy_weapon/sell_weapon", "[service]") {
    Mission_params mp{};
    mp.map.center = Point(0, 0);

    MissionRepositoryImpl repo(mp);
    DefaultAttackStrategy strat(repo);
    MissionServiceImpl svc(repo, strat);

    repo.add_ship( std::make_unique<MilitaryShip>("I1", 100, 1, 100, 0, "cap", 1.0), Point(0, 0), false);

    REQUIRE(svc.buy_weapon("I1", std::make_unique<WeaponCannon>(10), BOW));
    REQUIRE(svc.sell_weapon("I1", std::unique_ptr<Weapon>{}, BOW));
    REQUIRE_FALSE(svc.uninstall_weapon("I1", BOW));
}

TEST_CASE("MissionServiceImpl: move", "[service]") {
    Mission_params mp{};
    mp.budget = 0;
    mp.goal_weight = 0;
    mp.delivered_weight = 0;
    mp.total_cargo_weight = 0;
    mp.map.dx = 100;
    mp.map.dy = 100;
    mp.map.center = Point(0, 0);

    MissionRepositoryImpl repo(mp);
    DefaultAttackStrategy strat(repo);
    MissionServiceImpl svc(repo, strat);

    repo.add_ship( std::make_unique<MilitaryShip>("I1", 100, 10, 100, 0, "cap", 1.0), Point(0, 0), false);
    repo.add_ship(std::make_unique<MilitaryShip>("P1", 100, 5, 100, 0, "cap", 1.0), Point(50, 0),  true);

    svc.move();
    
    REQUIRE(repo.get_imperials().size() == 1);
    REQUIRE(repo.get_pirates().size() == 1);
}

TEST_CASE("MissionServiceImpl: auto_distribute_cargo", "[service]") {
    Mission_params mp{};
    mp.budget = 0;
    mp.goal_weight = 0;
    mp.delivered_weight = 0;
    mp.total_cargo_weight = 0;
    mp.map.dx = 100;
    mp.map.dy = 100;
    mp.map.center = Point(0, 0);

    MissionRepositoryImpl repo(mp);
    DefaultAttackStrategy strat(repo);
    MissionServiceImpl svc(repo, strat);

    repo.add_ship(std::make_unique<TransportShip>("T1", 100, 1, 100, 0, "cap", 1.0, 100, 0), Point(0, 0),  false);
    repo.add_ship(std::make_unique<TransportShip>("T2", 100, 1, 100, 0, "cap", 1.0, 50, 0), Point(0, 0), false);

    REQUIRE(svc.auto_distribute_cargo());
}

TEST_CASE("MissionServiceImpl: attack_by_area", "[service]") {
    Mission_params mp{};
    mp.budget = 0;
    mp.goal_weight = 0;
    mp.delivered_weight = 0;
    mp.total_cargo_weight = 0;
    mp.map.dx = 100;
    mp.map.dy = 100;
    mp.map.center = Point(0, 0);

    MissionRepositoryImpl repo(mp);
    DefaultAttackStrategy strat(repo);
    MissionServiceImpl svc(repo, strat);

    repo.add_ship(std::make_unique<MilitaryShip>("P1", 100, 1, 1, 0, "cap", 1.0), Point(5, 5), true);
    repo.add_ship(std::make_unique<MilitaryShip>("P2", 100, 1, 1, 0, "cap", 1.0), Point(6, 6), true);

    std::vector<std::string> targets;
    targets.push_back("P1");
    targets.push_back("P2");

    svc.attack_by_area(targets, 100, true);
    REQUIRE(repo.get_pirates().empty());
}



TEST_CASE("MissionServiceImpl: pirate_attack vs pirate_attack_parallel (threaded)", "[service][parallel]") {
    Mission_params mp{};
    mp.map.dx = 100;
    mp.map.dy = 100;
    mp.map.center = Point(0, 0);

    {
        MissionRepositoryImpl repo(mp);
        repo.add_ship(std::make_unique<MilitaryShip>("I0", 100, 1, 1, 0, "cap", 1.0), Point(0, 0), false);
        repo.add_ship(std::make_unique<MilitaryShip>("P0", 100, 1, 1, 0, "cap", 1.0), Point(1, 0), true);

        RecordingAttackStrategy strat("I0");
        MissionServiceImpl svc(repo, strat);

        REQUIRE(svc.pirate_attack(10, 100));
        REQUIRE(strat.thread_count() == 1);
        REQUIRE(repo.get_imperials().empty());
    }

    {
        MissionRepositoryImpl repo(mp);

        repo.add_ship(std::make_unique<MilitaryShip>("I0", 100, 1, 1, 0, "cap", 1.0), Point(0, 0), false);
        for (int i = 0; i < 8; ++i) {
            repo.add_ship(std::make_unique<MilitaryShip>("P" + std::to_string(i), 100, 1, 1, 0, "cap", 1.0),
                          Point(10 + i, 0),
                          true);
        }

        RecordingAttackStrategy strat("I0");
        MissionServiceImpl svc(repo, strat);

        REQUIRE(svc.pirate_attack_parallel(10, 100, 4));
        REQUIRE(strat.thread_count() >= 2);
        REQUIRE(repo.get_imperials().empty());
    }
}

TEST_CASE("MissionServiceImpl: buy_weapon/sell_weapon negative cases", "[service]") {
    Mission_params mp{};
    mp.map.center = Point(0, 0);

    MissionRepositoryImpl repo(mp);
    DefaultAttackStrategy strat(repo);
    MissionServiceImpl svc(repo, strat);

    REQUIRE_FALSE(svc.buy_weapon("I1", std::make_unique<WeaponCannon>(10), BOW));
    REQUIRE_FALSE(svc.sell_weapon("nonexistent", std::unique_ptr<Weapon>{}, BOW));
    REQUIRE_FALSE(svc.uninstall_weapon("nonexistent", BOW));
    REQUIRE_FALSE(svc.install_weapon("nonexistent", std::make_unique<WeaponCannon>(10), BOW));
}
