#pragma once
#include <memory>
#include <algorithm>
#include "mapper/ShipMapper.h"
#include "TransportShip.h"

/**
 * @brief Маппер TransportShip <-> ShipDto
 */
class TransportShipMapper final : public ConcreteShipMapper<TransportShip> {
public:
    [[nodiscard]] ShipDto map_to(const TransportShip& from) const override {
        ShipDto dto;
        dto.type = "transport";
        dto.callsign = from.get_callsign();
        dto.captain = from.get_captain();
        dto.current_HP = from.get_current_hp();
        dto.current_velocity = from.get_current_velocity();
        dto.params = {
            {"max_cargo_weight", std::to_string(from.get_max_cargo_weight())},
            {"current_cargo_weight", std::to_string(from.get_current_cargo_weight())}
        };
        
        return dto;
    }

    [[nodiscard]] std::unique_ptr<TransportShip> map_from(const ShipDto& dto) const override {
        auto it = dto.params.find("current_cargo_weight");
        unsigned current_cargo_weight = (it != dto.params.end()) ? std::stoul(it->second) : 0;

        unsigned max_cargo_weight = current_cargo_weight;
        if (auto itMax = dto.params.find("max_cargo_weight"); itMax != dto.params.end()) {
            max_cargo_weight = std::stoul(itMax->second);
        }
        max_cargo_weight = std::max({1u, max_cargo_weight, current_cargo_weight});
        return std::make_unique<TransportShip>(
            dto.callsign,
            dto.current_velocity,
            dto.current_velocity,
            dto.current_HP,
            0,
            dto.captain,
            1.0,
            max_cargo_weight,
            current_cargo_weight);
    }

    [[nodiscard]] std::string get_key() const override {
        return "transport";
    }

    [[nodiscard]] std::type_index get_type() const override {
        return typeid(TransportShip);
    }
};

using TransportShipSuperMapper = SuperShipMapper<TransportShipMapper>;
