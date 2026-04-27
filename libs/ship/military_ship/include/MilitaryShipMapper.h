#pragma once

#include "dto/ShipDto.h"
#include "mapper/ShipMapper.h"
#include "MilitaryShip.h"
#include "WeaponCannon.h"
#include "WeaponMissile.h"
#include "WeaponTorpedo.h"
#include <memory>

/**
 * @brief Маппер MilitaryShip <-> ShipDto
 */
class MilitaryShipMapper final : public ConcreteShipMapper<MilitaryShip> {
public:
    [[nodiscard]] ShipDto map_to(const MilitaryShip& from) const override {
        ShipDto dto;
        dto.type = "military";
        dto.callsign = from.get_callsign();
        dto.captain = from.get_captain();
        dto.current_HP = from.get_current_hp();
        dto.current_velocity = from.get_current_velocity();
        for (const auto& weapon_unit : from.get_weapons()) {
            if (!weapon_unit.weapon) {
                continue;
            }
            dto.params.emplace(get_place_name(weapon_unit.place), std::to_string(weapon_unit.weapon->get_id()));
        }
        return dto;
    }

    [[nodiscard]] std::unique_ptr<MilitaryShip> map_from(const ShipDto& dto) const override {
        auto ship = std::make_unique<MilitaryShip>(
            dto.callsign,
            dto.current_velocity,
            dto.current_velocity,
            dto.current_HP,
            0,
            dto.captain,
            1.0);

        for (const auto& [place_key, id_str] : dto.params) {
            const Place place = get_place_id(place_key);
            if (place == NONE) {
                continue;
            }

            unsigned weapon_id = 0;
            try {
                weapon_id = static_cast<unsigned>(std::stoul(id_str));
            } catch (...) {
                continue;
            }

            const unsigned ammo = (weapon_id >> 16) & 0xFFFFu;
            const unsigned tag = weapon_id & 0xFFFFu;

            std::hash<std::string> str_hash;
            const unsigned cannon_tag = static_cast<unsigned>(str_hash("cannon") & 0xFFFFu);
            const unsigned missile_tag = static_cast<unsigned>(str_hash("missile") & 0xFFFFu);
            const unsigned torpedo_tag = static_cast<unsigned>(str_hash("torpedo") & 0xFFFFu);

            std::unique_ptr<DefaultWeapon> w;
            if (tag == cannon_tag) {
                w = std::make_unique<WeaponCannon>(ammo);
            } else if (tag == missile_tag) {
                w = std::make_unique<WeaponMissile>(ammo);
            } else if (tag == torpedo_tag) {
                w = std::make_unique<WeaponTorpedo>(ammo);
            } else {
                w = std::make_unique<WeaponCannon>(ammo);
            }

            (void)ship->set_weapon(place, std::move(w));
        }

        return ship;
    }

    [[nodiscard]] std::string get_key() const override {
        return "military";
    }

    [[nodiscard]] std::type_index get_type() const override {
        return typeid(MilitaryShip);
    }
};

using MilitaryShipSuperMapper = SuperShipMapper<MilitaryShipMapper>;
