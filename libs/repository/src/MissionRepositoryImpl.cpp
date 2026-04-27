#include "MissionRepositoryImpl.h"
#include "model/point/Point.h"
#include "model/ship/Ship.h"
#include <optional>
#include <string>
#include <algorithm>
#include <climits>
#include <map>

MissionRepositoryImpl::MissionRepositoryImpl(Mission_params & mp)
    : imperials(), pirates(), params(mp) {
}

Mission_params& MissionRepositoryImpl::get_mission_params() const {
    return params;
}

ShipTable& MissionRepositoryImpl::get_pirates() {
    return pirates;
}

ShipTable& MissionRepositoryImpl::get_imperials() {
    return imperials;
}

bool MissionRepositoryImpl::add_ship(std::unique_ptr<Ship> ship, 
                                     const Point& coords, bool is_pirate) {
    
    auto & callsign = ship->get_callsign();
    auto& table = is_pirate ? pirates : imperials;
    auto it = table.find(callsign);
    if (it != table.end()) {
        return false;
    } 
    if (find_by_point(coords, false) != std::nullopt) {
        return false;
    }
    info ship_info;
    ship_info.callsign = callsign;
    ship_info.ship = std::move(ship);
    ship_info.coords = coords;
    
    table.emplace(callsign, std::move(ship_info));
    return true;
}

bool MissionRepositoryImpl::remove_ship(const std::string & callsign, bool is_pirate) {
     auto& table = is_pirate ? pirates : imperials;
    auto it = table.find(callsign);
    if (it != table.end()) {
        table.erase(callsign);
        return true;
    }
    return false;
}

std::optional<std::string> MissionRepositoryImpl::find_max_x_point_ship() {
    if (imperials.empty()) {
        return std::nullopt;
    }

    const auto it = std::max_element(
        imperials.begin(), imperials.end(),
        [](const auto& a, const auto& b) {
            return a.second.coords.get_x() < b.second.coords.get_x();
        });

    return (it == imperials.end()) ? std::nullopt : std::optional<std::string>(it->first);
}

std::optional<std::string> MissionRepositoryImpl::find_by_point(const Point& point, bool is_pirate) {
      auto& table = is_pirate ? pirates : imperials;

    const auto it = std::find_if(
        table.begin(), table.end(),
        [&point](const auto& unit) {
            return unit.second.coords.get_x() == point.get_x() &&
                   unit.second.coords.get_y() == point.get_y();
        });
    return (it == table.end()) 
                        ? std::nullopt
                        : std::optional<std::string>{it->first};
}

unsigned MissionRepositoryImpl::find_min_velocity(bool is_pirate) {
      auto& table = is_pirate ? pirates : imperials;
    if (table.empty()) {
        return 0;
    }

    const auto it = std::min_element(
        table.begin(), table.end(),
        [](const auto& a, const auto& b) {
            const auto av = a.second.ship ? a.second.ship->get_current_velocity() : UINT_MAX;
            const auto bv = b.second.ship ? b.second.ship->get_current_velocity() : UINT_MAX;
            return av < bv;
        });

    if (it == table.end() || !it->second.ship) {
        return 0;
    }
    return it->second.ship->get_current_velocity();
}

void MissionRepositoryImpl::move_by_x(unsigned shift, bool is_pirate) {
    auto& table = is_pirate ? pirates : imperials;

    std::for_each(table.begin(), table.end(), [shift](auto& unit) {
        unit.second.coords.add_x(shift);
    });
}

std::map<unsigned, Point> MissionRepositoryImpl::find_n_nearest(const Point & p, size_t n, bool is_pirate) {
    auto& table = is_pirate ? pirates : imperials;

    std::map<unsigned, Point> dists;

    for (auto& ship : table) {
        dists.emplace(
            ship.second.coords.get_distance2(p),
            ship.second.coords
        );
    }

    std::map<unsigned, Point> result;
    size_t count = 0;

    for (auto it = dists.begin(); it != dists.end() && count < n; ++it, ++count) {
        result.emplace(it->first, it->second);
    }
    return std::move(result);
}

std::vector<Point> MissionRepositoryImpl::find_opposite_ships_in_fire_range(const Point & p, unsigned range, bool is_pirate) {
    auto & table = is_pirate ? get_imperials() : get_pirates();
    std::vector<Point> dists;
    for (auto & ship : table) {
        if (ship.second.coords.get_distance2(p) <= range*range) {
            dists.push_back(ship.second.coords);
        }
    }
    return std::move(dists);
}
