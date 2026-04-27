#pragma once

#include "mapper/WeaponMapper.h"
#include "WeaponMissile.h"

/**
 * @brief Маппер WeaponMissile <-> WeaponDto
 */
class MissileMapper final : public ConcreteWeaponMapper<WeaponMissile> {
public:
    [[nodiscard]] WeaponDto map_to(const WeaponMissile& from) const override {
        WeaponDto dto;
        dto.id = from.get_id();
        dto.type = "missile";
        dto.current_ammunition = from.get_current_ammunition();
        return dto;
    }

    [[nodiscard]] std::unique_ptr<WeaponMissile> map_from(const WeaponDto& dto) const override {
        auto weapon = std::make_unique<WeaponMissile>(dto.current_ammunition);
        return weapon;
    }

    [[nodiscard]] std::string get_key() const override {
        return "missile";
    }

    [[nodiscard]] std::type_index get_type() const override {
        return typeid(WeaponMissile);
    }
};

using MissileSuperMapper = SuperWeaponMapper<MissileMapper>;
