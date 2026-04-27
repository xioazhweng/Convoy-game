#include "MissionServiceImpl.h"
#include "model/ship/Military.h"
#include "model/ship/Transport.h"
#include "model/weapon/Weapon.h"
#include <memory>
#include <algorithm>
#include <climits>
#include <cstdint>
#include <optional>
#include <thread>
#include <set>

MissionServiceImpl::MissionServiceImpl(MissionRepository& repo_, AttackStrategy & as) 
    : repo(repo_), strategy(as) {
}

bool MissionServiceImpl::buy_ship(std::unique_ptr<Ship> ship, const Point& coords) {
    auto cost = ship->get_cost();
    bool is_ok = repo.add_ship(std::move(ship), coords, false);
    if (is_ok) {
        repo.get_mission_params().spent_budged += cost;
    }
    return is_ok;
}

bool MissionServiceImpl::sell_ship(const std::string & callsign) {
    auto& imperials = repo.get_imperials();
    auto it = imperials.find(callsign);
    if (it != imperials.end()) {
        auto cost = it->second.ship->get_cost();
        imperials.erase(callsign);
          repo.get_mission_params().budget += cost;
        return true;
    }
    return false;
}

void MissionServiceImpl::move() {
    auto moveFleetByMinSpeed = [](ShipTable& table) {
        if (table.empty()) {
            return;
        }

        unsigned min_speed = UINT_MAX;
        for (const auto& [callsign, ship_info] : table) {
            (void)callsign;
            if (ship_info.ship) {
                min_speed = std::min<unsigned>(min_speed, ship_info.ship->get_current_velocity());
            }
        }
        if (min_speed == UINT_MAX || min_speed == 0) {
            return;
        }

        for (auto& [callsign, ship_info] : table) {
            (void)callsign;
            ship_info.coords.add_x(static_cast<int>(min_speed));
        }
    };

    moveFleetByMinSpeed(repo.get_imperials());
    moveFleetByMinSpeed(repo.get_pirates());
}

bool MissionServiceImpl::check_cargo_goal() {
    auto& params = repo.get_mission_params();
    if (params.delivered_weight >= params.goal_weight) {
        return true;
    }
    return false;
}

bool MissionServiceImpl::load_cargo(const std::string & callsign, size_t weight, unsigned cost) {
    auto& imperials = repo.get_imperials();
    auto it = imperials.find(callsign);
    
    if (it != imperials.end()) {
        info& ship_info = it->second;
        if (ship_info.ship) {
            auto* transport_ship = dynamic_cast<Transport*>(ship_info.ship.get());
            if (transport_ship && transport_ship->can_load_cargo(static_cast<unsigned>(weight))) {
                transport_ship->add_cargo_weight(static_cast<unsigned>(weight));
                
                auto& params = repo.get_mission_params();
                params.total_cargo_weight += static_cast<unsigned>(weight);
                params.spent_budged += cost;
                return true;
            } else {
                return false;
            }
        }
    }
    
    return false;
}

bool MissionServiceImpl::unload_cargo(const std::string & callsign) {
    auto& imperials = repo.get_imperials();
    auto it = imperials.find(callsign);
    
    if (it != imperials.end()) {
        info& ship_info = it->second;
        if (ship_info.ship) {
            auto* transport_ship = dynamic_cast<Transport*>(ship_info.ship.get());
            if (transport_ship) {
                const unsigned removed = transport_ship->get_current_cargo_weight();
                transport_ship->set_current_cargo_weight(0);
                auto& params = repo.get_mission_params();
                params.total_cargo_weight = (params.total_cargo_weight >= removed) ? (params.total_cargo_weight - removed) : 0u;
                return true;
            }
        }
    }
    return false;
}

bool MissionServiceImpl::auto_distribute_cargo() {
    auto& imperials = repo.get_imperials();
    auto& params = repo.get_mission_params();
    
    if (params.total_cargo_weight == 0) {
        return true;
    }
    
    std::vector<Transport*> transports;
    transports.reserve(imperials.size());
    for (auto& [callsign, ship_info] : imperials) {
        (void)callsign;
        if (!ship_info.ship) {
            continue;
        }
        if (auto* t = dynamic_cast<Transport*>(ship_info.ship.get())) {
            transports.push_back(t);
        }
    }
    
    if (transports.empty()) {
        return false;
    }

    const unsigned total = params.total_cargo_weight;

    for (auto* t : transports) {
        t->set_current_cargo_weight(0);
    }

    unsigned remaining = total;
    for (size_t i = 0; i < transports.size(); ++i) {
        auto* t = transports[i];
        const size_t shipsLeft = transports.size() - i;
        const unsigned base = remaining / static_cast<unsigned>(shipsLeft);
        const unsigned extra = (remaining % static_cast<unsigned>(shipsLeft)) ? 1u : 0u;
        const unsigned desired = base + extra;
        const unsigned cap = t->get_max_cargo_weight();
        const unsigned loadNow = std::min(desired, cap);
        t->set_current_cargo_weight(loadNow);
        remaining -= loadNow;
    }

    params.total_cargo_weight = total - remaining;
    return true;
}


void MissionServiceImpl::attack_by_area(const std::vector<std::string>& callsigns, unsigned damage, bool is_pirates) {
    auto & table = is_pirates ? repo.get_pirates() : repo.get_imperials();
    for (const auto& cs : callsigns) {
        auto it = table.find(cs);
        if (it == table.end() || !it->second.ship) {
            continue;
        }
        it->second.ship->take_damage(damage);
        if (it->second.ship->is_destroyed()) {
            table.erase(cs);
        }
    }
}

namespace {
    static inline std::uint64_t point_key(const Point& p) noexcept {
        const std::uint64_t ux = static_cast<std::uint32_t>(p.get_x());
        const std::uint64_t uy = static_cast<std::uint32_t>(p.get_y());
        return (ux << 32) | uy;
    }
}


bool MissionServiceImpl::attack(const std::string & callsign, Place place, const std::string & target_callsign, bool is_pirates) {
    auto & attackers = is_pirates ? repo.get_pirates() : repo.get_imperials();
    auto it = attackers.find(callsign);
    if (it == attackers.end()) {
        return false;
    }
    auto* military_attacker = dynamic_cast<Military*>(it->second.ship.get());
    if (military_attacker == nullptr) {
        return false;
    }

    auto & targets_table = is_pirates ? repo.get_imperials() : repo.get_pirates();
    auto tit = targets_table.find(target_callsign);
    if (tit == targets_table.end()) {
        return false;
    }
    const Point center = tit->second.coords;

    auto weapons = military_attacker->get_weapon_by_place(place);
    if (weapons.empty()) {
        return false;
    }
    for (auto w : weapons) {
        if (w->get_current_ammunition() == 0) {
             return false;
        }
        const bool target_is_pirate = !is_pirates;
        auto area = w->get_shoot_area(center);

        std::vector<std::string> area_callsigns;
        area_callsigns.reserve(area.size());
        for (const auto& pt : area) {
            auto cs = repo.find_by_point(pt, target_is_pirate);
            if (cs == std::nullopt) {
                continue; 
            }
            area_callsigns.push_back(*cs);
        }
      
        attack_by_area(area_callsigns, w->get_damage(), target_is_pirate);
        w->shoot();
    }
      
    return true;
};


bool MissionServiceImpl::install_weapon(const std::string  & callsign, std::unique_ptr<Weapon> weapon, Place place) {
    auto& imperials = repo.get_imperials();
    auto it = imperials.find(callsign);
    
    if (it != imperials.end()) {
        info& ship_info = it->second;
        if (ship_info.ship) {
            auto* military_ship = dynamic_cast<Military*>(ship_info.ship.get());
            if (military_ship) {
                if (military_ship->set_weapon(place, std::move(weapon))) {
                    return true;
                } else {
                    return false;
                }
            } else {
                return false;
            }
        }
    }

    return false;
}

bool MissionServiceImpl::uninstall_weapon(const std::string & callsign, Place place) {
    auto& imperials = repo.get_imperials();
    auto it = imperials.find(callsign);
    
    if (it != imperials.end()) {
        info& ship_info = it->second;
        if (ship_info.ship) {
            auto* military_ship = dynamic_cast<Military*>(ship_info.ship.get());
            if (military_ship) {
                if (military_ship->remove_weapon(place)) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool MissionServiceImpl::buy_weapon(const std::string & callsign, std::unique_ptr<Weapon> weapon, Place place) {
    if (!weapon) {
        return false;
    }

    const unsigned cost = weapon->get_cost();
    const bool is_ok = install_weapon(callsign, std::move(weapon), place);
    if (is_ok) {
        repo.get_mission_params().spent_budged += cost;
    }
    return is_ok;
}

bool MissionServiceImpl::sell_weapon(const std::string & callsign, std::unique_ptr<Weapon> weapon, Place place) {
    auto& imperials = repo.get_imperials();
    auto it = imperials.find(callsign);
    
    if (it != imperials.end()) {
        info& ship_info = it->second;
        if (ship_info.ship) {
            auto* military_ship = dynamic_cast<Military*>(ship_info.ship.get());
            if (military_ship) {
                weapon = military_ship->remove_weapon(place);
                if (weapon) {
                    return true;
                }
            }
        }
    } 
    return false;
}

#include <chrono>

bool MissionServiceImpl::pirate_attack(unsigned range, unsigned damage) {
    auto & pirates = repo.get_pirates();
    if (pirates.empty()) {
        return false;
    }
    std::vector<std::string> results;
    for (auto & ship : pirates) {
        auto vect = strategy.auto_shoot(ship.second.coords, range, true); 
        if (vect.empty()) {
            continue;
        }
        results.insert(results.end(), std::make_move_iterator(vect.begin()), std::make_move_iterator(vect.end()) );
    }

    attack_by_area(results, damage, false);
    return true;
}

bool MissionServiceImpl::pirate_attack_parallel(unsigned range, unsigned damage, unsigned thread_count) {
    
    unsigned thread_num = thread_count;
    if (thread_num == 0) {
        thread_num = std::thread::hardware_concurrency();
        if (thread_num == 0) {
            thread_num = 16;
        }
    }

    auto & pirate = repo.get_pirates();
    auto & imperials = repo.get_imperials();
    if (pirate.empty()) {
        return false;
    }
    
    std::vector<std::string> results;
    std::mutex mux;
    std::vector<std::thread> threads(thread_num);
    auto elements = std::distance(pirate.begin(), pirate.end());
    
    for (size_t i = 0; i < thread_num; i++) {
        size_t start_i = i * elements / thread_num;
        size_t end_i   = (i + 1) * elements / thread_num;

        auto start = std::next(pirate.begin(), start_i);
        auto end   = std::next(pirate.begin(), end_i);

        threads[i] = std::thread(
            [start, end, range, &results, &mux, this]()
            {
                for (auto it = start; it != end; ++it) {
                const auto & ship = it->second;
                auto vect = strategy.auto_shoot(ship.coords, range, true);        auto end = std::chrono::high_resolution_clock::now();
                std::lock_guard<std::mutex> lock(mux);
                results.insert(results.end(), std::make_move_iterator(vect.begin()), std::make_move_iterator(vect.end()) );
               
            }
        }
        );
    }

    for (auto & t : threads ) {
        t.join();
    }
     
    if (results.empty()) {
        return false;
    }

    std::set<std::string> to_remove;
    elements = std::distance(results.begin(), results.end());
    
    for (size_t i = 0; i < thread_num; i++) {
        size_t start_i = i * elements / thread_num;
        size_t end_i = (i+1) * elements / thread_num;

        auto start = std::next(results.begin(), start_i);
        auto end = std::next(results.begin(), end_i);

        threads[i] = std::thread([start, end, damage, &to_remove, &imperials, &mux]()
            {

                for (auto it = start; it != end; ++it) {
                    const std::string& callsign = *it;
                    auto sit = imperials.find(callsign);
                    if (sit == imperials.end() || !sit->second.ship) {
                        continue;
                    }

                    auto* ship = sit->second.ship.get();
                    ship->take_damage(damage); // ship uses internal mutex
                    if (ship->is_destroyed()) {
                        std::lock_guard<std::mutex> lock(mux);
                        to_remove.insert(callsign);
                    }
                    
            }
            });
        }

    for (auto & t : threads ) {
        t.join();
    }
    
    
    for (const auto & c : to_remove){
        imperials.erase(c);
    }


    return true;

}
