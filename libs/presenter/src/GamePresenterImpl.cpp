#include "GamePresenterImpl.h"
#include "dto/GameStateDto.h"
#include "model/ship/Ship.h"
#include "model/ship/Military.h"
#include "model/ship/Transport.h"
#include "model/weapon/Weapon.h"
#include "../../weapon/missile/include/WeaponMissile.h"
#include "../../weapon/cannon/include/WeaponCannon.h"
#include "../../weapon/torpedo/include/WeaponTorpedo.h"
#include "../../service/state_service/include/YalmStateService.h"
#include <climits>
#include <algorithm>

namespace {
    constexpr int kPirateBuyForbiddenRadius = 10;
    constexpr long long kPirateBuyForbiddenRadius2 = 1LL * kPirateBuyForbiddenRadius * kPirateBuyForbiddenRadius;

    static inline long long sqrll(long long v) noexcept { return v * v; }
    static inline long long dist2(const Point& a, const Point& b) noexcept {
        const long long dx = static_cast<long long>(a.get_x()) - static_cast<long long>(b.get_x());
        const long long dy = static_cast<long long>(a.get_y()) - static_cast<long long>(b.get_y());
        return sqrll(dx) + sqrll(dy);
    }
}

GamePresenterImpl::GamePresenterImpl(MissionRepository& repo, MissionService& mission_service,
                                   StateService& state_service, ShipMapper& ship_mapper,
                                   WeaponMapper& weapon_mapper)
    : repo_(repo), mission_service_(mission_service), state_service_(state_service),
      ship_mapper_(ship_mapper), weapon_mapper_(weapon_mapper) {
}

void GamePresenterImpl::load_game_state(const std::string& config_path) {
    const bool old_loaded = gameLoaded_;
    auto old_presenter_state = currentGameState_;
    auto old_persistence_state = state_service_.get_game_state();

    ShipTable old_imperials;
    ShipTable old_pirates;
    repo_.get_imperials().swap(old_imperials);
    repo_.get_pirates().swap(old_pirates);

    try {
        load_all_game_data(config_path);
        sync_mission_params_to_repo();
        process_ship_units_and_link_weapons();
        validate_game_state();

        gameLoaded_ = true;
    } catch (const std::exception& e) {
        repo_.get_imperials().swap(old_imperials);
        repo_.get_pirates().swap(old_pirates);
        currentGameState_ = std::move(old_presenter_state);
        state_service_.set_game_state(std::move(old_persistence_state));
        gameLoaded_ = old_loaded;

        throw std::runtime_error("Failed to load game state: " + std::string(e.what()));
    }
}

void GamePresenterImpl::save_game_state(const std::string& config_path) {
    if (!gameLoaded_) {
        throw std::runtime_error("Cannot save: No game state loaded");
    }
    try {
        update_ship_units_from_fleets();
        sync_dynamic_state_from_repo();
        sync_mission_params_from_repo();
        state_service_.set_game_state(currentGameState_);
        save_all_game_data(config_path);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to save game state: " + std::string(e.what()));
    }
}

const GameStateDto& GamePresenterImpl::get_game_state() const {
    if (!gameLoaded_) {
        throw std::runtime_error("No game state loaded");
    }
    return currentGameState_;
}

std::vector<ShipDto> GamePresenterImpl::get_ships() const {
    if (!gameLoaded_) {
        throw std::runtime_error("No game state loaded");
    }
    return currentGameState_.ships;
}

std::vector<ShipUnitDTO> GamePresenterImpl::get_ship_units() const {
    if (!gameLoaded_) {
        throw std::runtime_error("No game state loaded");
    }
    return currentGameState_.ship_units;
}

ShipDto GamePresenterImpl::get_ship_by_callsign(const std::string& callsign) const {
    auto ship = find_ship_by_callsign(callsign);
    if (!ship) {
        throw std::runtime_error("Ship not found with callsign: " + callsign);
    }
    return *ship;
}

std::vector<WeaponDto> GamePresenterImpl::get_weapons() const {
    if (!gameLoaded_) {
        throw std::runtime_error("No game state loaded");
    }
    return currentGameState_.weapons;
}

WeaponDto GamePresenterImpl::get_weapon_by_id(unsigned id) const {
    auto weapon = find_weapon_by_id(id);
    if (!weapon) {
        throw std::runtime_error("Weapon not found with id: " + std::to_string(id));
    }
    return *weapon;
}

bool GamePresenterImpl::assign_weapon_to_ship(const std::string& ship_callsign, unsigned weapon_id, const std::string& place) {
    if (!gameLoaded_) {
        return false;
    }
    
    try {
        auto ship = find_ship_by_callsign(ship_callsign);
        auto weapon = find_weapon_by_id(weapon_id);
        
        if (!ship || !weapon) return false;
        
        validate_ship_weapons_link(*ship, *weapon, place);
        
        auto weapon_obj = weapon_mapper_.map_from(*weapon);
        Place place_enum = get_place_id(place);

        if (dynamic_cast<Weapon*>(weapon_obj.get()) == nullptr) {
            throw std::runtime_error("WeaponMapper returned non-Weapon instance");
        }
        auto default_weapon = std::unique_ptr<Weapon>(static_cast<Weapon*>(weapon_obj.release()));
        return mission_service_.install_weapon(ship_callsign, std::move(default_weapon), place_enum);
        
    } catch (const std::exception&) {
        return false;
    }
}

bool GamePresenterImpl::remove_weapon_from_ship(const std::string& ship_callsign, 
                                                const std::string& place) {
    if (!gameLoaded_) {
        return false;
    }

    Place place_enum = get_place_id(place);
    return mission_service_.uninstall_weapon(ship_callsign, place_enum);
}

 bool GamePresenterImpl::set_weapon_from_ship(const WeaponDto & weapon, 
                                              const std::string& ship_callsign, 
                                              const std::string& place) {
    if (!gameLoaded_) {
        return false;
    }

    auto weapon_obj = weapon_mapper_.map_from(weapon);
    if (!weapon_obj) {
        return false;
    }

    if (dynamic_cast<Weapon*>(weapon_obj.get()) == nullptr) {
        throw std::runtime_error("WeaponMapper returned non-Weapon instance");
    }

    {
        WeaponParamsDto params;
        if (dynamic_cast<WeaponCannon*>(weapon_obj.get())) {
            params = currentGameState_.weapons_params.at("cannon");
        } else if (dynamic_cast<WeaponMissile*>(weapon_obj.get())) {
            params = currentGameState_.weapons_params.at("missile");
        } else if (dynamic_cast<WeaponTorpedo*>(weapon_obj.get())) {
            params = currentGameState_.weapons_params.at("torpedo");
        } else {
            throw std::runtime_error("Unknown weapon subtype (expected cannon/missile/torpedo)");
        }

    
        weapon_obj->set_fire_rate(params.fire_rate);
        weapon_obj->set_fire_range(params.fire_range);
        weapon_obj->set_max_ammunition(params.max_ammunition);
        weapon_obj->set_current_ammunition(params.max_ammunition);
       weapon_obj->set_cost(params.cost);
        weapon_obj->set_damage(params.damage);
    }

    const Place place_enum = ::get_place_id(place);
    const bool ok = mission_service_.buy_weapon(ship_callsign, std::move(weapon_obj), place_enum);
    if (ok) {
        sync_dynamic_state_from_repo();
        sync_mission_params_from_repo();
    }
     return ok;
  }

bool GamePresenterImpl::is_buy_ship_position_allowed(const Point& p) const {

    const auto& pirates = repo_.get_pirates();
    if (pirates.empty()) {
        return true;
    }

    int max_pirate_x = INT_MIN;
    for (const auto& [_, info] : pirates) {
        max_pirate_x = std::max(max_pirate_x, info.coords.get_x());
        if (dist2(p, info.coords) <= kPirateBuyForbiddenRadius2) {
            return false;
        }
    }

    if (max_pirate_x != INT_MIN && p.get_x() > max_pirate_x) {
        return false;
    }
    return true;
}

void GamePresenterImpl::buy_ship(const ShipDto& ship, const Point& p) {
    if (!gameLoaded_) {
        gameLoaded_ = true;
    }

    if (!is_buy_ship_position_allowed(p)) {
        return;
    }

    auto ship_obj = ship_mapper_.map_from(ship);
    if (dynamic_cast<Ship*>(ship_obj.get()) == nullptr) {
        throw std::runtime_error("ShipMapper returned non-Ship instance");
    }

    {
        auto* as_transport = dynamic_cast<Transport*>(ship_obj.get());
        auto* as_military = dynamic_cast<Military*>(ship_obj.get());

        if (as_transport && as_military) {
            ship_obj->set_cost(currentGameState_.ships_params.at("transport_military").cost);
            ship_obj->set_max_hp(currentGameState_.ships_params.at("transport_military").cost);
            as_transport->set_current_cargo_weight(0);
            as_transport->set_max_cargo_weight(500);
        } else if (as_military) {
            ship_obj->set_cost(currentGameState_.ships_params.at("military").cost);
             ship_obj->set_max_hp(currentGameState_.ships_params.at("military").cost);
        } else if (as_transport) {
            ship_obj->set_cost(currentGameState_.ships_params.at("transport").cost);
            ship_obj->set_max_hp(currentGameState_.ships_params.at("transport").cost);
            as_transport->set_current_cargo_weight(0);
            as_transport->set_max_cargo_weight(1000);
        }
    }

    const bool ok = mission_service_.buy_ship(std::move(ship_obj), p);
    if (!ok) {
        return;
    }

    update_ship_units_from_fleets();
    sync_dynamic_state_from_repo();
    sync_mission_params_from_repo();
  };

bool GamePresenterImpl::sell_ship(const std::string & callsign) {

    if (!gameLoaded_) {
        return false;
    }
    return mission_service_.sell_ship(callsign);

 };

bool GamePresenterImpl::is_game_loaded() const {
    return gameLoaded_;
}

void GamePresenterImpl::load_all_game_data(const std::string& config_path) {
   
    state_service_.set_game_state(GameStateDto{});
    state_service_.load_repo(config_path);
    state_service_.load_ships_params(config_path);
    state_service_.load_weapons_params(config_path);
    state_service_.load_mission_params(config_path);
    state_service_.load_ships(config_path);
    state_service_.load_weapons(config_path);
    currentGameState_ = state_service_.get_game_state();
}

void GamePresenterImpl::process_ship_units_and_link_weapons() {

    repo_.get_imperials().clear();
    repo_.get_pirates().clear();

    
    for (const auto& unit : currentGameState_.ship_units) {
        auto ship_dto = find_ship_by_callsign(unit.callsign);
        if (!ship_dto) {
            throw std::runtime_error("ShipUnit references non-existent ship: " + unit.callsign);
        }
        auto ship_obj = ship_mapper_.map_from(*ship_dto);
        auto * k = dynamic_cast<Transport *>(ship_obj.get());
        auto * p = dynamic_cast<Military *>(ship_obj.get());
        if (p != nullptr && k != nullptr) {
            ship_obj->set_cost(currentGameState_.ships_params.at("transport_military").cost); 
        } else if (p != nullptr) {
            ship_obj->set_cost(currentGameState_.ships_params.at("military").cost);
        } else if (k != nullptr) {
            ship_obj->set_cost(currentGameState_.ships_params.at("transport").cost);
        }
        
        const bool is_pirate = (unit.side == "pirate" || unit.side == "pirates");
        
        Point coords(unit.x, unit.y);
        
    
        repo_.add_ship(std::move(ship_obj), coords, is_pirate);
        
        for (const auto& [param_key, param_value] : ship_dto->params) {
            std::string place_key;
            if (param_key.rfind("weapon_", 0) == 0) {
                place_key = param_key.substr(7);
            } else {
                place_key = param_key;
            }

            const Place place = get_place_id(place_key);
            if (place == NONE) {
                continue;
            }

            try {
                const unsigned weapon_id = std::stoul(param_value);
                auto weapon_dto = find_weapon_by_id(weapon_id);
                if (!weapon_dto) {
                    continue;
                }

                auto weapon_obj = weapon_mapper_.map_from(*weapon_dto);

                if (dynamic_cast<Weapon*>(weapon_obj.get()) == nullptr) {
                    throw std::runtime_error("WeaponMapper returned non-Weapon instance");
                }
                auto default_weapon = std::unique_ptr<Weapon>(static_cast<Weapon*>(weapon_obj.release()));
                mission_service_.install_weapon(unit.callsign, std::move(default_weapon), place);

            } catch (const std::exception& e) {
                throw std::runtime_error("Warning: Invalid weapon ID for ship " + unit.callsign + ": " + e.what());
            }
        }
    }
}

void GamePresenterImpl::populate_fleets_from_ship_units() {
    // (๑ↀᆺↀ๑) 
}

void GamePresenterImpl::save_all_game_data(const std::string& config_path) const {
    state_service_.save_repo(config_path);
    state_service_.save_ships_params(config_path);
    state_service_.save_weapons_params(config_path);
    state_service_.save_mission_params(config_path);
    state_service_.save_ships(config_path);
    state_service_.save_weapons(config_path);
}

void GamePresenterImpl::update_ship_units_from_fleets() {
    
    currentGameState_.ship_units.clear();
    for (const auto& [callsign, ship_info] : repo_.get_imperials()) {
        ShipUnitDTO unit;
        unit.callsign = callsign;
        unit.x = ship_info.coords.get_x();
        unit.y = ship_info.coords.get_y();
        unit.side = "imperial";
        currentGameState_.ship_units.push_back(unit);
    }
    
    for (const auto& [callsign, ship_info] : repo_.get_pirates()) {
        ShipUnitDTO unit;
        unit.callsign = callsign;
        unit.x = ship_info.coords.get_x();
        unit.y = ship_info.coords.get_y();
        unit.side = "pirate";
        currentGameState_.ship_units.push_back(unit);
    }
}

void GamePresenterImpl::sync_dynamic_state_from_repo() {
    if (!gameLoaded_) {
        return;
    }

    auto sync_from_table = [&](ShipTable& table) {
        for (const auto& [callsign, ship_info] : table) {
            if (!ship_info.ship) {
                continue;
            }

            ShipDto mapped = ship_mapper_.map_to(*ship_info.ship);

            if (auto* existing_ship_dto = find_ship_by_callsign(callsign)) {
                auto* mil = dynamic_cast<Military*>(ship_info.ship.get());
                if (mil) {
                    for (const auto& wu : mil->get_weapons()) {
                        if (!wu.weapon) {
                            continue;
                        }

                        const std::string place_key = get_place_name(wu.place);
                        unsigned old_id = wu.weapon->get_id();
                        if (auto it = existing_ship_dto->params.find(place_key); it != existing_ship_dto->params.end()) {
                            try {
                                old_id = static_cast<unsigned>(std::stoul(it->second));
                            } catch (...) {
                               
                            }
                        }

                        WeaponDto new_weapon_dto = weapon_mapper_.map_to(*wu.weapon);

                        if (auto* inv_weapon = find_weapon_by_id(old_id)) {
                            *inv_weapon = new_weapon_dto;
                        } else if (!find_weapon_by_id(new_weapon_dto.id)) {
                            currentGameState_.weapons.push_back(new_weapon_dto);
                        }
                    }
                }

                *existing_ship_dto = std::move(mapped);
            } else {
                currentGameState_.ships.push_back(std::move(mapped));
            }
        }
    };

    sync_from_table(repo_.get_imperials());
    sync_from_table(repo_.get_pirates());
}

void GamePresenterImpl::sync_mission_params_from_repo() {
    auto & mission_params = repo_.get_mission_params();
    currentGameState_.mission.budget = mission_params.budget;
    currentGameState_.mission.spent_budget = mission_params.spent_budged;
    currentGameState_.mission.total_cargo_weight = mission_params.total_cargo_weight;
    currentGameState_.mission.goal_weight = mission_params.goal_weight;
    currentGameState_.mission.lost_weight = mission_params.lost_weight;
    currentGameState_.mission.delivered_weight = mission_params.delivered_weight;
};

void GamePresenterImpl::sync_mission_params_to_repo() {
    auto& mp = repo_.get_mission_params();
    const auto& dto = currentGameState_.mission;

    mp.budget = dto.budget;
    mp.spent_budged = dto.spent_budget;
    mp.total_cargo_weight = dto.total_cargo_weight;
    mp.goal_weight = dto.goal_weight;
    mp.lost_weight = dto.lost_weight;
    mp.delivered_weight = dto.delivered_weight;

    if (auto it = dto.bases.find("a_base"); it != dto.bases.end()) {
        mp.A_base = Point(it->second.x, it->second.y);
    }
    if (auto it = dto.bases.find("b_base"); it != dto.bases.end()) {
        mp.B_base = Point(it->second.x, it->second.y);
    }

    mp.pirates_bases.clear();
    mp.pirates_bases.reserve(dto.pirates_bases.size());
    for (const auto& [_, base] : dto.pirates_bases) {
        mp.pirates_bases.emplace_back(base.x, base.y);
    }

    mp.map.dx = static_cast<unsigned>(dto.map.dx);
    mp.map.dy = static_cast<unsigned>(dto.map.dy);
    mp.map.center = Point(dto.map.center.x, dto.map.center.y);
}

ShipDto* GamePresenterImpl::find_ship_by_callsign(const std::string& callsign) {
    auto& ships = currentGameState_.ships;
    auto it = std::find_if(ships.begin(), ships.end(),
        [&callsign](const ShipDto& ship) { return ship.callsign == callsign; });
    return (it != ships.end()) ? &(*it) : nullptr;
}

const ShipDto* GamePresenterImpl::find_ship_by_callsign(const std::string& callsign) const {
    const auto& ships = currentGameState_.ships;
    auto it = std::find_if(ships.begin(), ships.end(),
        [&callsign](const ShipDto& ship) { return ship.callsign == callsign; });
    return (it != ships.end()) ? &(*it) : nullptr;
}

WeaponDto* GamePresenterImpl::find_weapon_by_id(unsigned id) {
    auto& weapons = currentGameState_.weapons;
    auto it = std::find_if(weapons.begin(), weapons.end(),
        [id](const WeaponDto& weapon) { return weapon.id == id; });
    return (it != weapons.end()) ? &(*it) : nullptr;
}

const WeaponDto* GamePresenterImpl::find_weapon_by_id(unsigned id) const {
    const auto& weapons = currentGameState_.weapons;
    auto it = std::find_if(weapons.begin(), weapons.end(),
        [id](const WeaponDto& weapon) { return weapon.id == id; });
    return (it != weapons.end()) ? &(*it) : nullptr;
}

void GamePresenterImpl::validate_game_state() const {
    if (currentGameState_.ships.empty()) {
        throw std::runtime_error("No ships found in game state");
    }
    
    if (currentGameState_.ship_units.empty()) {
        throw std::runtime_error("No ship units found in game state");
    }
    for (const auto& unit : currentGameState_.ship_units) {
        if (!find_ship_by_callsign(unit.callsign)) {
            throw std::runtime_error("ShipUnit references non-existent ship: " + unit.callsign);
        }
    }
}

void GamePresenterImpl::validate_ship_weapons_link(const ShipDto& ship, const WeaponDto& weapon, const std::string& place) const {
    if (weapon.current_ammunition == 0) {
        throw std::runtime_error("Weapon has no ammunition");
    }
    
    Place place_enum = get_place_id(place);
    if (place_enum == NONE) {
        throw std::runtime_error("Invalid place: " + place);
    }
}

void GamePresenterImpl::process_player_turn() {
    // (^◕ᴥ◕^)ฅ^•ﻌ•^ฅ
}

void GamePresenterImpl::process_pirate_attacks() {
    if (!gameLoaded_) {
        return;
    }
    auto & weapon_params = currentGameState_.weapons_params.at("cannon");
    mission_service_.pirate_attack_parallel(weapon_params.fire_range, weapon_params.damage);
}

void GamePresenterImpl::move_ships() {
    if (!gameLoaded_) return;
    // имперцы сдвигаются вправо, пираты — влево (минимальная скорость флота).

    auto& imperials = repo_.get_imperials();
    if (!imperials.empty()) {
        unsigned min_speed = UINT_MAX;
        for (const auto& [callsign, ship_info] : imperials) {
            (void)callsign;
            if (ship_info.ship) {
                min_speed = std::min<unsigned>(min_speed, ship_info.ship->get_current_velocity());
            }
        }

        if (min_speed != UINT_MAX && min_speed > 0) {
            for (auto& [callsign, ship_info] : imperials) {
                (void)callsign;
                ship_info.coords.set_point(ship_info.coords.get_x() + min_speed, ship_info.coords.get_y());
            }
        }
    }

    auto& pirates = repo_.get_pirates();
    if (!pirates.empty()) {
        unsigned min_speed = UINT_MAX;
        for (const auto& [callsign, ship_info] : pirates) {
            (void)callsign;
            if (ship_info.ship) {
                min_speed = std::min<unsigned>(min_speed, ship_info.ship->get_current_velocity());
            }
        }

        if (min_speed != UINT_MAX && min_speed > 0) {
            for (auto& [callsign, ship_info] : pirates) {
                (void)callsign;
                ship_info.coords.set_point(ship_info.coords.get_x() - static_cast<int>(min_speed), ship_info.coords.get_y());
            }
        }
    }
}

bool GamePresenterImpl::check_base_arrival() {
    if (!gameLoaded_) {
        return false;
    }

    auto& imperials = repo_.get_imperials();
    int finish_x = INT_MIN;
    auto it_b = currentGameState_.mission.bases.find("b_base");
    if (it_b != currentGameState_.mission.bases.end()) {
        finish_x = it_b->second.x;
    } else {
        for (const auto& [name, base] : currentGameState_.mission.bases) {
            (void)name;
            finish_x = std::max(finish_x, base.x);
        }
    }
    if (finish_x == INT_MIN) {
        return false;
    }

    for (const auto& [callsign, ship_info] : imperials) {
        if (!ship_info.ship || ship_info.ship->get_current_hp() <= 0) {
            continue;
        }
        if (ship_info.coords.get_x() >= finish_x) {
            return true;
        }
    }
    return false;
}

void GamePresenterImpl::end_turn() {
    update_ship_units_from_fleets();
    sync_dynamic_state_from_repo();
    sync_mission_params_from_repo();
}

bool GamePresenterImpl::reload_all_imperial_weapons(unsigned price) {
    if (!gameLoaded_) {
        return false;
    }

    const unsigned money_left = (currentGameState_.mission.budget >= currentGameState_.mission.spent_budget)
                                    ? (currentGameState_.mission.budget - currentGameState_.mission.spent_budget)
                                    : 0u;
    if (price > money_left) {
        return false;
    }

    bool reloaded_any = false;
    for (const auto& [callsign, ship_info] : repo_.get_imperials()) {
        (void)callsign;
        if (!ship_info.ship) {
            continue;
        }
        auto* mil = dynamic_cast<Military*>(ship_info.ship.get());
        if (!mil) {
            continue;
        }
        for (const auto& wu : mil->get_weapons()) {
            if (!wu.weapon) {
                continue;
            }
            reloaded_any = wu.weapon->reload() || reloaded_any;
        }
    }

    if (!reloaded_any) {
        return false;
    }

    currentGameState_.mission.spent_budget += price;
    sync_dynamic_state_from_repo();
    return true;
}
