#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <string>

#include "presenter/include/GamePresenterImpl.h"
#include "repository/include/MissionRepositoryImpl.h"
#include "service/mission_service/include/MissionServiceImpl.h"
#include "strategy/include/RandomAttackStrategy.h"
#include "service/state_service/include/YalmStateService.h"

#include "mapper/ShipMapper.h"
#include "mapper/WeaponMapper.h"

#include "ship/military_ship/include/MilitaryShipMapper.h"
#include "ship/transport_ship/include/TransportShipMapper.h"
#include "ship/transportmilitary_ship/include/TMShipMapper.h"

#include "weapon/cannon/include/CannonMapper.h"
#include "weapon/missile/include/MissileMapper.h"
#include "weapon/torpedo/include/TorpedoMapper.h"

#include "weapon/cannon/include/WeaponCannon.h"

namespace fs = std::filesystem;

static void writeFile(const fs::path& p, const std::string& s) {
    fs::create_directories(p.parent_path());
    std::ofstream f(p);
    REQUIRE(f.is_open());
    f << s;
}

TEST_CASE("GamePresenterImpl: load + базовые операции", "[presenter]") {
    const fs::path dir = fs::temp_directory_path() / "my_game_presenter_cfg";
    fs::remove_all(dir);
    fs::create_directories(dir);


    WeaponCannon wc(3);
    const unsigned wid = wc.get_id();

    writeFile(dir / "weapons_params.yaml",
              "weapons:\n"
              "  cannon:\n"
              "    fire_rate: 1\n"
              "    fire_range: 2\n"
              "    max_ammunition: 300\n"
              "    cost: 4\n"
              "    damage: 300\n");

    writeFile(dir / "ships_params.yaml",
              "ships:\n"
              "  military:\n"
              "    max_velocity: 10\n"
              "    max_HP: 20\n"
              "    cost: 30\n");

    writeFile(dir / "mission_params.yaml",
              "mission:\n"
              "  budget: 1000\n"
              "  spent_budget: 0\n"
              "  total_cargo_weight: 0\n"
              "  goal_weight: 0\n"
              "  lost_weight: 0\n"
              "  delivered_weight: 0\n"
              "  bases:\n"
              "    a_base: {x: 0, y: 0}\n"
              "    b_base: {x: 10, y: 0}\n"
              "  pirates_bases: []\n"
              "  map:\n"
              "    dx: 10\n"
              "    dy: 10\n"
              "    center: {x: 5, y: 5}\n");

    writeFile(dir / "ships.yaml",
              "ships:\n"
              "  - type: military\n"
              "    callsign: I1\n"
              "    captain: Cap\n"
              "    current_velocity: 10\n"
              "    current_HP: 20\n"
              "    params: {bow: '" + std::to_string(wid) + "'}\n");

    writeFile(dir / "repository_data.yaml",
              "repository:\n"
              "  - callsign: I1\n"
              "    x: 0\n"
              "    y: 0\n"
              "    side: imperial\n");

    writeFile(dir / "weapons.yaml",
              "weapons:\n"
              "  - id: " + std::to_string(wid) + "\n"
              "    type: cannon\n"
              "    current_ammunition: 3\n");

    Mission_params mp{};
    mp.map.center = Point(0, 0);
    MissionRepositoryImpl repo(mp);
    RandomAttackStrategy strat(repo);
    MissionServiceImpl missionSvc(repo, strat);
    YalmStateService stateSvc(dir.string());

    PolymorphicShipMapper shipPoly;
    MilitaryShipSuperMapper ms;
    TransportShipSuperMapper ts;
    TMShipSuperMapper tms;
    shipPoly.add_sub_mapper(ms);
    shipPoly.add_sub_mapper(ts);
    shipPoly.add_sub_mapper(tms);

    PolymorphicWeaponMapper weaponPoly;
    CannonSuperMapper cm;
    MissileSuperMapper mm;
    TorpedoSuperMapper tm;
    weaponPoly.add_sub_mapper(cm);
    weaponPoly.add_sub_mapper(mm);
    weaponPoly.add_sub_mapper(tm);

    GamePresenterImpl p(repo, missionSvc, stateSvc, shipPoly, weaponPoly);


    p.load_game_state(dir.string());
    REQUIRE(p.is_game_loaded());
    REQUIRE(p.get_ships().size() == 1);
    REQUIRE(p.get_weapons().size() == 1);

    REQUIRE(p.remove_weapon_from_ship("I1", "bow"));
    p.end_turn();

    REQUIRE(p.assign_weapon_to_ship("I1", wid, "bow"));
    p.end_turn();

    p.save_game_state(dir.string());
}

TEST_CASE("GamePresenterImpl: get_game_state/get_ships/get_weapons", "[presenter]") {
    const fs::path dir = fs::temp_directory_path() / "my_game_presenter_cfg_getters";
    fs::remove_all(dir);
    fs::create_directories(dir);

    WeaponCannon wc(3);
    const unsigned wid = wc.get_id();

    writeFile(dir / "weapons_params.yaml",
              "weapons:\n"
              "  cannon:\n"
              "    fire_rate: 1\n"
              "    fire_range: 2\n"
              "    max_ammunition: 300\n"
              "    cost: 4\n"
              "    damage: 300\n");

    writeFile(dir / "ships_params.yaml",
              "ships:\n"
              "  military:\n"
              "    max_velocity: 10\n"
              "    max_HP: 20\n"
              "    cost: 30\n");

    writeFile(dir / "mission_params.yaml",
              "mission:\n"
              "  budget: 1000\n"
              "  spent_budget: 0\n"
              "  total_cargo_weight: 0\n"
              "  goal_weight: 0\n"
              "  lost_weight: 0\n"
              "  delivered_weight: 0\n"
              "  bases:\n"
              "    a_base: {x: 0, y: 0}\n"
              "    b_base: {x: 10, y: 0}\n"
              "  pirates_bases: []\n"
              "  map:\n"
              "    dx: 10\n"
              "    dy: 10\n"
              "    center: {x: 5, y: 5}\n");

    writeFile(dir / "ships.yaml",
              "ships:\n"
              "  - type: military\n"
              "    callsign: I1\n"
              "    captain: Cap\n"
              "    current_velocity: 10\n"
              "    current_HP: 20\n"
              "    params: {bow: '" + std::to_string(wid) + "'}\n");

    writeFile(dir / "repository_data.yaml",
              "repository:\n"
              "  - callsign: I1\n"
              "    x: 0\n"
              "    y: 0\n"
              "    side: imperial\n");

    writeFile(dir / "weapons.yaml",
              "weapons:\n"
              "  - id: " + std::to_string(wid) + "\n"
              "    type: cannon\n"
              "    current_ammunition: 3\n");

    writeFile(dir / "ship_units.yaml",
              "ship_units:\n"
              "  - callsign: I1\n"
              "    x: 0\n"
              "    y: 0\n"
              "    side: imperial\n");

    Mission_params mp{};
    mp.map.center = Point(0, 0);
    MissionRepositoryImpl repo(mp);
    RandomAttackStrategy strat(repo);
    MissionServiceImpl missionSvc(repo, strat);
    YalmStateService stateSvc(dir.string());

    PolymorphicShipMapper shipPoly;
    MilitaryShipSuperMapper ms;
    TransportShipSuperMapper ts;
    TMShipSuperMapper tms;
    shipPoly.add_sub_mapper(ms);
    shipPoly.add_sub_mapper(ts);
    shipPoly.add_sub_mapper(tms);

    PolymorphicWeaponMapper weaponPoly;
    CannonSuperMapper cm;
    MissileSuperMapper mm;
    TorpedoSuperMapper tm;
    weaponPoly.add_sub_mapper(cm);
    weaponPoly.add_sub_mapper(mm);
    weaponPoly.add_sub_mapper(tm);

    GamePresenterImpl p(repo, missionSvc, stateSvc, shipPoly, weaponPoly);

    p.load_game_state(dir.string());

    auto& game_state = p.get_game_state();
    REQUIRE(game_state.ships.size() == 1);
    REQUIRE(game_state.weapons.size() == 1);
    REQUIRE(game_state.ship_units.size() == 1);

    auto ships = p.get_ships();
    auto weapons = p.get_weapons();
    auto ship_units = p.get_ship_units();

    REQUIRE(ships.size() == 1);
    REQUIRE(weapons.size() == 1);
    REQUIRE(ship_units.size() == 1);

    auto ship = p.get_ship_by_callsign("I1");
    REQUIRE(ship.callsign == "I1");

    auto weapon = p.get_weapon_by_id(wid);
    REQUIRE(weapon.id == wid);
}


TEST_CASE("GamePresenterImpl: process_player_turn и другие игровые методы", "[presenter]") {
    const fs::path dir = fs::temp_directory_path() / "my_game_presenter_cfg_turns";
    fs::remove_all(dir);
    fs::create_directories(dir);

    WeaponCannon wc(3);
    const unsigned wid = wc.get_id();

    writeFile(dir / "weapons_params.yaml",
              "weapons:\n"
              "  cannon:\n"
              "    fire_rate: 1\n"
              "    fire_range: 2\n"
              "    max_ammunition: 300\n"
              "    cost: 4\n"
              "    damage: 300\n");

    writeFile(dir / "ships_params.yaml",
              "ships:\n"
              "  military:\n"
              "    max_velocity: 10\n"
              "    max_HP: 20\n"
              "    cost: 30\n");

    writeFile(dir / "mission_params.yaml",
              "mission:\n"
              "  budget: 1000\n"
              "  spent_budget: 0\n"
              "  total_cargo_weight: 0\n"
              "  goal_weight: 0\n"
              "  lost_weight: 0\n"
              "  delivered_weight: 0\n"
              "  bases:\n"
              "    a_base: {x: 0, y: 0}\n"
              "    b_base: {x: 10, y: 0}\n"
              "  pirates_bases: []\n"
              "  map:\n"
              "    dx: 10\n"
              "    dy: 10\n"
              "    center: {x: 5, y: 5}\n");

    writeFile(dir / "ships.yaml",
              "ships:\n"
              "  - type: military\n"
              "    callsign: I1\n"
              "    captain: Cap\n"
              "    current_velocity: 10\n"
              "    current_HP: 20\n"
              "    params: {bow: '" + std::to_string(wid) + "'}\n");

    writeFile(dir / "repository_data.yaml",
              "repository:\n"
              "  - callsign: I1\n"
              "    x: 0\n"
              "    y: 0\n"
              "    side: imperial\n");

    writeFile(dir / "weapons.yaml",
              "weapons:\n"
              "  - id: " + std::to_string(wid) + "\n"
              "    type: cannon\n"
              "    current_ammunition: 3\n");

    Mission_params mp{};
    mp.map.center = Point(0, 0);
    MissionRepositoryImpl repo(mp);
    RandomAttackStrategy strat(repo);
    MissionServiceImpl missionSvc(repo, strat);
    YalmStateService stateSvc(dir.string());

    PolymorphicShipMapper shipPoly;
    MilitaryShipSuperMapper ms;
    TransportShipSuperMapper ts;
    TMShipSuperMapper tms;
    shipPoly.add_sub_mapper(ms);
    shipPoly.add_sub_mapper(ts);
    shipPoly.add_sub_mapper(tms);

    PolymorphicWeaponMapper weaponPoly;
    CannonSuperMapper cm;
    MissileSuperMapper mm;
    TorpedoSuperMapper tm;
    weaponPoly.add_sub_mapper(cm);
    weaponPoly.add_sub_mapper(mm);
    weaponPoly.add_sub_mapper(tm);

    GamePresenterImpl p(repo, missionSvc, stateSvc, shipPoly, weaponPoly);

    p.load_game_state(dir.string());
    REQUIRE(p.is_game_loaded());

    p.process_player_turn();
    p.process_pirate_attacks();
    p.move_ships();
    REQUIRE(p.check_base_arrival());
    p.end_turn();

    REQUIRE(p.reload_all_imperial_weapons(10));
}

TEST_CASE("GamePresenterImpl: buy_ship обновляет spent_budget и не сбрасывает cargo", "[presenter]") {
    const fs::path dir = fs::temp_directory_path() / "my_game_presenter_cfg_buy_ship";
    fs::remove_all(dir);
    fs::create_directories(dir);

    WeaponCannon wc(3);
    const unsigned wid = wc.get_id();

    writeFile(dir / "weapons_params.yaml",
              "weapons:\n"
              "  cannon:\n"
              "    fire_rate: 1\n"
              "    fire_range: 2\n"
              "    max_ammunition: 300\n"
              "    cost: 4\n"
              "    damage: 300\n");

    writeFile(dir / "ships_params.yaml",
              "ships:\n"
              "  military:\n"
              "    max_velocity: 10\n"
              "    max_HP: 20\n"
              "    cost: 30\n"
              "  transport:\n"
              "    max_velocity: 10\n"
              "    max_HP: 20\n"
              "    cost: 50\n");

    writeFile(dir / "mission_params.yaml",
              "mission:\n"
              "  budget: 1000\n"
              "  spent_budget: 0\n"
              "  total_cargo_weight: 7\n"
              "  goal_weight: 0\n"
              "  lost_weight: 0\n"
              "  delivered_weight: 0\n"
              "  bases:\n"
              "    a_base: {x: 0, y: 0}\n"
              "    b_base: {x: 10, y: 0}\n"
              "  pirates_bases: []\n"
              "  map:\n"
              "    dx: 10\n"
              "    dy: 10\n"
              "    center: {x: 5, y: 5}\n");

    writeFile(dir / "ships.yaml",
              "ships:\n"
              "  - type: military\n"
              "    callsign: I1\n"
              "    captain: Cap\n"
              "    current_velocity: 10\n"
              "    current_HP: 20\n"
              "    params: {bow: '" + std::to_string(wid) + "'}\n");

    writeFile(dir / "repository_data.yaml",
              "repository:\n"
              "  - callsign: I1\n"
              "    x: 0\n"
              "    y: 0\n"
              "    side: imperial\n");

    writeFile(dir / "weapons.yaml",
              "weapons:\n"
              "  - id: " + std::to_string(wid) + "\n"
              "    type: cannon\n"
              "    current_ammunition: 3\n");

    Mission_params mp{};
    mp.map.center = Point(0, 0);
    MissionRepositoryImpl repo(mp);
    RandomAttackStrategy strat(repo);
    MissionServiceImpl missionSvc(repo, strat);
    YalmStateService stateSvc(dir.string());

    PolymorphicShipMapper shipPoly;
    MilitaryShipSuperMapper ms;
    TransportShipSuperMapper ts;
    TMShipSuperMapper tms;
    shipPoly.add_sub_mapper(ms);
    shipPoly.add_sub_mapper(ts);
    shipPoly.add_sub_mapper(tms);

    PolymorphicWeaponMapper weaponPoly;
    CannonSuperMapper cm;
    MissileSuperMapper mm;
    TorpedoSuperMapper tm;
    weaponPoly.add_sub_mapper(cm);
    weaponPoly.add_sub_mapper(mm);
    weaponPoly.add_sub_mapper(tm);

    GamePresenterImpl p(repo, missionSvc, stateSvc, shipPoly, weaponPoly);
    p.load_game_state(dir.string());

    REQUIRE(p.get_game_state().mission.total_cargo_weight == 7);
    REQUIRE(p.get_game_state().mission.spent_budget == 0);
    REQUIRE(p.get_game_state().mission.budget == 1000);

    ShipDto newShip;
    newShip.type = "transport";
    newShip.callsign = "I2";
    newShip.captain = "Cap2";
    newShip.current_velocity = 10;
    newShip.current_HP = 20;
    newShip.params = {
        {"max_cargo_weight", "100"},
        {"current_cargo_weight", "0"},
    };

    p.buy_ship(newShip, Point(1, 0));

    const auto& st = p.get_game_state();
    REQUIRE(st.mission.spent_budget == 50);
    REQUIRE(st.mission.budget == 1000);
    REQUIRE(st.mission.total_cargo_weight == 7);
    REQUIRE(st.ships.size() == 2);
    REQUIRE(st.ship_units.size() == 2);
}

TEST_CASE("GamePresenterImpl: buy_ship запрещён близко к пиратам и правее пиратов по X", "[presenter]") {
    const fs::path dir = fs::temp_directory_path() / "my_game_presenter_cfg_buy_ship_pirate_restrictions";
    fs::remove_all(dir);
    fs::create_directories(dir);

    writeFile(dir / "weapons_params.yaml",
              "weapons:\n"
              "  cannon:\n"
              "    fire_rate: 1\n"
              "    fire_range: 2\n"
              "    max_ammunition: 300\n"
              "    cost: 4\n"
              "    damage: 300\n");

    writeFile(dir / "ships_params.yaml",
              "ships:\n"
              "  military:\n"
              "    max_velocity: 10\n"
              "    max_HP: 20\n"
              "    cost: 30\n"
              "  transport:\n"
              "    max_velocity: 10\n"
              "    max_HP: 20\n"
              "    cost: 50\n");

    writeFile(dir / "mission_params.yaml",
              "mission:\n"
              "  budget: 1000\n"
              "  spent_budget: 0\n"
              "  total_cargo_weight: 0\n"
              "  goal_weight: 0\n"
              "  lost_weight: 0\n"
              "  delivered_weight: 0\n"
              "  bases:\n"
              "    a_base: {x: 0, y: 0}\n"
              "    b_base: {x: 10, y: 0}\n"
              "  pirates_bases: []\n"
              "  map:\n"
              "    dx: 10\n"
              "    dy: 10\n"
              "    center: {x: 5, y: 5}\n");

    writeFile(dir / "ships.yaml",
              "ships:\n"
              "  - type: military\n"
              "    callsign: I1\n"
              "    captain: Cap\n"
              "    current_velocity: 10\n"
              "    current_HP: 20\n"
              "    params: {}\n"
              "  - type: military\n"
              "    callsign: P1\n"
              "    captain: Pirate\n"
              "    current_velocity: 10\n"
              "    current_HP: 20\n"
              "    params: {}\n");

    writeFile(dir / "repository_data.yaml",
              "repository:\n"
              "  - callsign: I1\n"
              "    x: 0\n"
              "    y: 0\n"
              "    side: imperial\n"
              "  - callsign: P1\n"
              "    x: 150\n"
              "    y: 0\n"
              "    side: pirate\n");

    writeFile(dir / "weapons.yaml",
              "weapons: []\n");

    Mission_params mp{};
    mp.map.center = Point(0, 0);
    MissionRepositoryImpl repo(mp);
    RandomAttackStrategy strat(repo);
    MissionServiceImpl missionSvc(repo, strat);
    YalmStateService stateSvc(dir.string());

    PolymorphicShipMapper shipPoly;
    MilitaryShipSuperMapper ms;
    TransportShipSuperMapper ts;
    TMShipSuperMapper tms;
    shipPoly.add_sub_mapper(ms);
    shipPoly.add_sub_mapper(ts);
    shipPoly.add_sub_mapper(tms);

    PolymorphicWeaponMapper weaponPoly;
    CannonSuperMapper cm;
    MissileSuperMapper mm;
    TorpedoSuperMapper tm;
    weaponPoly.add_sub_mapper(cm);
    weaponPoly.add_sub_mapper(mm);
    weaponPoly.add_sub_mapper(tm);

    GamePresenterImpl p(repo, missionSvc, stateSvc, shipPoly, weaponPoly);
    p.load_game_state(dir.string());

    ShipDto newShip;
    newShip.type = "transport";
    newShip.callsign = "I2";
    newShip.captain = "Cap2";
    newShip.current_velocity = 10;
    newShip.current_HP = 20;
    newShip.params = {
        {"max_cargo_weight", "100"},
        {"current_cargo_weight", "0"},
    };

    p.buy_ship(newShip, Point(150, 5));
    REQUIRE(p.get_game_state().ships.size() == 2);
    REQUIRE(p.get_game_state().ship_units.size() == 2);

    p.buy_ship(newShip, Point(1000, 1000));
    REQUIRE(p.get_game_state().ships.size() == 2);
    REQUIRE(p.get_game_state().ship_units.size() == 2);

    p.buy_ship(newShip, Point(150, 200));
    REQUIRE(p.get_game_state().ships.size() == 3);
    REQUIRE(p.get_game_state().ship_units.size() == 3);
    REQUIRE(p.get_game_state().mission.spent_budget == 50);
}

TEST_CASE("GamePresenterImpl: set_weapon_from_ship + sell_ship", "[presenter]") {
    const fs::path dir = fs::temp_directory_path() / "my_game_presenter_cfg_set_weapon";
    fs::remove_all(dir);
    fs::create_directories(dir);

    WeaponCannon wc(3);
    const unsigned wid = wc.get_id();

    writeFile(dir / "weapons_params.yaml",
              "weapons:\n"
              "  cannon:\n"
              "    fire_rate: 1\n"
              "    fire_range: 2\n"
              "    max_ammunition: 300\n"
              "    cost: 4\n"
              "    damage: 300\n");

    writeFile(dir / "ships_params.yaml",
              "ships:\n"
              "  military:\n"
              "    max_velocity: 10\n"
              "    max_HP: 20\n"
              "    cost: 30\n");

    writeFile(dir / "mission_params.yaml",
              "mission:\n"
              "  budget: 1000\n"
              "  spent_budget: 0\n"
              "  total_cargo_weight: 0\n"
              "  goal_weight: 0\n"
              "  lost_weight: 0\n"
              "  delivered_weight: 0\n"
              "  bases:\n"
              "    a_base: {x: 0, y: 0}\n"
              "    b_base: {x: 10, y: 0}\n"
              "  pirates_bases: []\n"
              "  map:\n"
              "    dx: 10\n"
              "    dy: 10\n"
              "    center: {x: 5, y: 5}\n");

    writeFile(dir / "ships.yaml",
              "ships:\n"
              "  - type: military\n"
              "    callsign: I1\n"
              "    captain: Cap\n"
              "    current_velocity: 10\n"
              "    current_HP: 20\n"
              "    params: {bow: '" + std::to_string(wid) + "'}\n");

    writeFile(dir / "repository_data.yaml",
              "repository:\n"
              "  - callsign: I1\n"
              "    x: 0\n"
              "    y: 0\n"
              "    side: imperial\n");

    writeFile(dir / "weapons.yaml",
              "weapons:\n"
              "  - id: " + std::to_string(wid) + "\n"
              "    type: cannon\n"
              "    current_ammunition: 3\n");

    Mission_params mp{};
    mp.map.center = Point(0, 0);
    MissionRepositoryImpl repo(mp);
    RandomAttackStrategy strat(repo);
    MissionServiceImpl missionSvc(repo, strat);
    YalmStateService stateSvc(dir.string());

    PolymorphicShipMapper shipPoly;
    MilitaryShipSuperMapper ms;
    TransportShipSuperMapper ts;
    TMShipSuperMapper tms;
    shipPoly.add_sub_mapper(ms);
    shipPoly.add_sub_mapper(ts);
    shipPoly.add_sub_mapper(tms);

    PolymorphicWeaponMapper weaponPoly;
    CannonSuperMapper cm;
    MissileSuperMapper mm;
    TorpedoSuperMapper tm;
    weaponPoly.add_sub_mapper(cm);
    weaponPoly.add_sub_mapper(mm);
    weaponPoly.add_sub_mapper(tm);

    GamePresenterImpl p(repo, missionSvc, stateSvc, shipPoly, weaponPoly);
    p.load_game_state(dir.string());
    REQUIRE(p.is_game_loaded());
    REQUIRE(p.get_weapons().size() == 1);
    REQUIRE(p.get_game_state().mission.spent_budget == 0);

    WeaponDto newWeapon;
    newWeapon.type = "cannon";
    newWeapon.id = 9999;
    newWeapon.current_ammunition = 1;

    REQUIRE(p.set_weapon_from_ship(newWeapon, "I1", "stern"));
    REQUIRE(p.get_weapons().size() == 2);
    REQUIRE(p.get_game_state().mission.spent_budget == 4);

    REQUIRE(p.sell_ship("I1"));
    p.end_turn();
    REQUIRE(p.get_ship_units().empty());
    REQUIRE(repo.get_imperials().empty());
}
