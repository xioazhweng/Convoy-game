#pragma once
#include <memory>
#include <algorithm>
#include "mapper/ShipMapper.h"
#include "TransportMilitaryShip.h"

/**
 * @brief Маппер TransportMilitaryShip <-> ShipDto
 */
class TMShipMapper final : public ConcreteShipMapper<TransportMilitaryShip> {
public:
    [[nodiscard]] ShipDto map_to(const TransportMilitaryShip& from) const override {
        ShipDto dto;
        dto.type = "transport_military";
        dto.callsign = from.get_callsign();
        dto.current_velocity = from.get_current_velocity();
        dto.captain = from.get_captain();
        dto.current_HP = from.get_current_hp();
        dto.params = {
            {"max_cargo_weight", std::to_string(from.get_max_cargo_weight())},
            {"current_cargo_weight", std::to_string(from.get_current_cargo_weight())},
        };
        for (auto & weapon_unit : from.get_weapons()) {
            dto.params.emplace(get_place_name(weapon_unit.place), std::to_string(weapon_unit.weapon->get_id()));
        }
        return dto;
    }

    [[nodiscard]] std::unique_ptr<TransportMilitaryShip> map_from(const ShipDto& dto) const override {
        auto it = dto.params.find("current_cargo_weight");
        unsigned current_cargo_weight = (it != dto.params.end()) ? std::stoul(it->second) : 0;

        unsigned max_cargo_weight = current_cargo_weight;
        if (auto itMax = dto.params.find("max_cargo_weight"); itMax != dto.params.end()) {
            max_cargo_weight = std::stoul(itMax->second);
        }
        max_cargo_weight = std::max({1u, max_cargo_weight, current_cargo_weight});
        return std::make_unique<TransportMilitaryShip>(
            dto.callsign,
            dto.current_velocity,
            dto.current_velocity,
            dto.current_HP,
            0,
            dto.captain,
            1.0,
            max_cargo_weight,
            current_cargo_weight,
            1.0);
    }

    [[nodiscard]] std::string get_key() const override {
        return "transport_military";
    }

    [[nodiscard]] std::type_index get_type() const override {
        return typeid(TransportMilitaryShip);
    }
};

using TMShipSuperMapper = SuperShipMapper<TMShipMapper>;
