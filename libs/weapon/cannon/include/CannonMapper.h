#pragma once

#include "mapper/WeaponMapper.h"
#include "WeaponCannon.h"

/**
 * @brief Маппер WeaponCannon <-> WeaponDto
 */
class CannonMapper final : public ConcreteWeaponMapper<WeaponCannon> {
public:
    [[nodiscard]] WeaponDto map_to(const WeaponCannon& from) const override {
        WeaponDto dto;
        dto.id = from.get_id();
        dto.type = "cannon";
        dto.current_ammunition = from.get_current_ammunition();
        return dto;
    }

    [[nodiscard]] std::unique_ptr<WeaponCannon> map_from(const WeaponDto& dto) const override {
        auto weapon = std::make_unique<WeaponCannon>(dto.current_ammunition);
        return weapon;
    }

    [[nodiscard]] std::string get_key() const override {
        return "cannon";
    }

    [[nodiscard]] std::type_index get_type() const override {
        return typeid(WeaponCannon);
    }
};

using CannonSuperMapper = SuperWeaponMapper<CannonMapper>;
