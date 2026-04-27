#pragma once
#include <memory>
#include "Mapper.h"
#include "PolymorphicMapper.h"
#include "dto/ShipDto.h"
#include "model/ship/Ship.h"

using ShipMapper = Mapper<Ship, ShipDto, std::unique_ptr<Ship>>;

template<typename Subtype>
using ConcreteShipMapper = SubtypeMapper<Subtype, ShipDto, std::unique_ptr<Subtype>>;

template<typename ConcreteMapper>
using SuperShipMapper = SubtypeMapperAdapter<Ship, std::unique_ptr<Ship>, ConcreteMapper>;

class PolymorphicShipMapper final : public PolymorphicMapper<Ship, ShipDto, std::unique_ptr<Ship>> {
    public:
        [[nodiscard]] std::string get_key(const ShipDto& dto) const override {
            return dto.type;
        }
    };
