#pragma once

#include "mapper/WeaponMapper.h"
#include "WeaponTorpedo.h"

/**
 * @brief Маппер WeaponTorpedo <-> WeaponDto
 */
class TorpedoMapper final : public ConcreteWeaponMapper<WeaponTorpedo> {
public:
    [[nodiscard]] WeaponDto map_to(const WeaponTorpedo& from) const override {
        WeaponDto dto;
        dto.id = from.get_id();
        dto.type = "torpedo";
        dto.current_ammunition = from.get_current_ammunition();
        return dto;
    }

    [[nodiscard]] std::unique_ptr<WeaponTorpedo> map_from(const WeaponDto& dto) const override {
        auto weapon = std::make_unique<WeaponTorpedo>(dto.current_ammunition);
        return weapon;
    }

    [[nodiscard]] std::string get_key() const override {
        return "torpedo";
    }

    [[nodiscard]] std::type_index get_type() const override {
        return typeid(WeaponTorpedo);
    }
};

using TorpedoSuperMapper = SuperWeaponMapper<TorpedoMapper>;
