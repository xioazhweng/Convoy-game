#pragma once
#include <memory>
#include "Mapper.h"
#include "PolymorphicMapper.h"
#include "dto/WeaponDto.h"
#include "model/weapon/Weapon.h"


using WeaponMapper = Mapper<Weapon, WeaponDto, std::unique_ptr<Weapon>>;

template<typename Subtype>
using ConcreteWeaponMapper = SubtypeMapper<Subtype, WeaponDto, std::unique_ptr<Subtype>>;

template<typename ConcreteMapper>
using SuperWeaponMapper = SubtypeMapperAdapter<Weapon, std::unique_ptr<Weapon>, ConcreteMapper>;

class PolymorphicWeaponMapper final : public PolymorphicMapper<Weapon, WeaponDto, std::unique_ptr<Weapon>> {
    public:
        [[nodiscard]] std::string get_key(const WeaponDto& dto) const override {
            return dto.type;
        }
    };
