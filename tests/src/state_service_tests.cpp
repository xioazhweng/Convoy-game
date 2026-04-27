#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <string>

#include "service/state_service/include/YalmStateService.h"

namespace fs = std::filesystem;

static void writeFile(const fs::path& p, const std::string& s) {
    fs::create_directories(p.parent_path());
    std::ofstream f(p);
    REQUIRE(f.is_open());
    f << s;
    
}

TEST_CASE("YalmStateService: load_* читает YAML", "[yaml]") {
    const fs::path dir = fs::temp_directory_path() / "my_game_yaml_test";
    fs::remove_all(dir);
    fs::create_directories(dir);

    writeFile(dir / "weapons_params.yaml",
              "weapons:\n"
              "  cannon:\n"
              "    fire_rate: 1\n"
              "    fire_range: 2\n"
              "    max_ammunition: 3\n"
              "    cost: 4\n"
              "    damage: 5\n");

    writeFile(dir / "ships_params.yaml",
              "ships:\n"
              "  military:\n"
              "    max_velocity: 10\n"
              "    max_HP: 20\n"
              "    cost: 30\n");

    writeFile(dir / "mission_params.yaml",
              "mission:\n"
              "  budget: 100\n"
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
              "    params: {}\n");

    writeFile(dir / "repository_data.yaml",
              "repository:\n"
              "  - callsign: I1\n"
              "    x: 0\n"
              "    y: 0\n"
              "    side: imperial\n");

    writeFile(dir / "weapons.yaml",
              "weapons:\n"
              "  - id: 1\n"
              "    type: cannon\n"
              "    current_ammunition: 3\n");

    YalmStateService svc(dir.string());
    svc.set_game_state(GameStateDto{});
    svc.load_repo(dir.string());
    svc.load_weapons_params(dir.string());
    svc.load_ships_params(dir.string());
    svc.load_mission_params(dir.string());
    svc.load_ships(dir.string());
    svc.load_weapons(dir.string());

    const auto& st = svc.get_game_state();
    REQUIRE(st.ships.size() == 1);
    REQUIRE(st.ship_units.size() == 1);
    REQUIRE(st.weapons.size() == 1);
    REQUIRE(st.ships_params.contains("military"));
    REQUIRE(st.weapons_params.contains("cannon"));
}

TEST_CASE("YalmStateService: save_* пишет YAML", "[yaml]") {
    const fs::path dir = fs::temp_directory_path() / "my_game_yaml_test_save";
    fs::remove_all(dir);
    fs::create_directories(dir);

    YalmStateService svc(dir.string());
    GameStateDto st{};
    st.mission.budget = 1;
    svc.set_game_state(st);
    svc.save_mission_params(dir.string());

    REQUIRE(fs::exists(dir / "mission_params.yaml"));
}

TEST_CASE("YalmStateService: get_game_state и get_* методы", "[yaml]") {
    const fs::path dir = fs::temp_directory_path() / "my_game_yaml_test_getters";
    fs::remove_all(dir);
    fs::create_directories(dir);

    YalmStateService svc(dir.string());
    
    const auto& game_state = svc.get_game_state();
    REQUIRE(game_state.ships.empty());
    REQUIRE(game_state.ship_units.empty());
    REQUIRE(game_state.weapons.empty());
    
    const auto& ships = svc.get_ships();
    const auto& ship_units = svc.get_ship_units();
    const auto& weapons = svc.get_weapons();
    
    REQUIRE(ships.empty());
    REQUIRE(ship_units.empty());
    REQUIRE(weapons.empty());
}

TEST_CASE("YalmStateService: save_repo", "[yaml]") {
    const fs::path dir = fs::temp_directory_path() / "my_game_yaml_test_save_repo";
    fs::remove_all(dir);
    fs::create_directories(dir);

    YalmStateService svc(dir.string());
    svc.save_repo(dir.string());

    REQUIRE(fs::exists(dir / "repository_data.yaml"));
}

TEST_CASE("YalmStateService: save_weapons_params", "[yaml]") {
    const fs::path dir = fs::temp_directory_path() / "my_game_yaml_test_save_weapons";
    fs::remove_all(dir);
    fs::create_directories(dir);

    YalmStateService svc(dir.string());
    svc.save_weapons_params(dir.string());

    REQUIRE(fs::exists(dir / "weapons_params.yaml"));
}

TEST_CASE("YalmStateService: save_ships_params", "[yaml]") {
    const fs::path dir = fs::temp_directory_path() / "my_game_yaml_test_save_ships";
    fs::remove_all(dir);
    fs::create_directories(dir);

    YalmStateService svc(dir.string());
    svc.save_ships_params(dir.string());

    REQUIRE(fs::exists(dir / "ships_params.yaml"));
}

TEST_CASE("YalmStateService: save_ships", "[yaml]") {
    const fs::path dir = fs::temp_directory_path() / "my_game_yaml_test_save_ships_data";
    fs::remove_all(dir);
    fs::create_directories(dir);

    YalmStateService svc(dir.string());
    svc.save_ships(dir.string());

    REQUIRE(fs::exists(dir / "ships.yaml"));
}

TEST_CASE("YalmStateService: save_ship_units", "[yaml]") {
    const fs::path dir = fs::temp_directory_path() / "my_game_yaml_test_save_ship_units";
    fs::remove_all(dir);
    fs::create_directories(dir);

    YalmStateService svc(dir.string());
    svc.save_ship_units(dir.string());

    REQUIRE(fs::exists(dir / "ship_units.yaml"));
}

TEST_CASE("YalmStateService: save_weapons", "[yaml]") {
    const fs::path dir = fs::temp_directory_path() / "my_game_yaml_test_save_weapons_data";
    fs::remove_all(dir);
    fs::create_directories(dir);

    YalmStateService svc(dir.string());
    svc.save_weapons(dir.string());

    REQUIRE(fs::exists(dir / "weapons.yaml"));
}


TEST_CASE("YalmStateService: загрузка всех данных", "[yaml]") {
    const fs::path dir = fs::temp_directory_path() / "my_game_yaml_test_full_load";
    fs::remove_all(dir);
    fs::create_directories(dir);

    writeFile(dir / "weapons_params.yaml",
              "weapons:\n"
              "  cannon:\n"
              "    fire_rate: 1\n"
              "    fire_range: 2\n"
              "    max_ammunition: 3\n"
              "    cost: 4\n"
              "    damage: 5\n");

    writeFile(dir / "ships_params.yaml",
              "ships:\n"
              "  military:\n"
              "    max_velocity: 10\n"
              "    max_HP: 20\n"
              "    cost: 30\n");

    writeFile(dir / "mission_params.yaml",
              "mission:\n"
              "  budget: 100\n"
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
              "    params: {}\n");

    writeFile(dir / "repository_data.yaml",
              "repository:\n"
              "  - callsign: I1\n"
              "    x: 0\n"
              "    y: 0\n"
              "    side: imperial\n");

    writeFile(dir / "weapons.yaml",
              "weapons:\n"
              "  - id: 1\n"
              "    type: cannon\n"
              "    current_ammunition: 3\n");

    writeFile(dir / "ship_units.yaml",
              "ship_units:\n"
              "  - callsign: I1\n"
              "    x: 0\n"
              "    y: 0\n"
              "    side: imperial\n");

    YalmStateService svc(dir.string());
    svc.load_repo(dir.string());
    svc.load_weapons_params(dir.string());
    svc.load_ships_params(dir.string());
    svc.load_mission_params(dir.string());
    svc.load_ships(dir.string());
    svc.load_weapons(dir.string());
    svc.load_ship_units(dir.string());

    const auto& st = svc.get_game_state();
    REQUIRE(st.ships.size() == 1);
    REQUIRE(st.ship_units.size() == 1);
    REQUIRE(st.weapons.size() == 1);
    REQUIRE(st.ships_params.contains("military"));
    REQUIRE(st.weapons_params.contains("cannon"));
    REQUIRE(st.mission.budget == 100);
}
