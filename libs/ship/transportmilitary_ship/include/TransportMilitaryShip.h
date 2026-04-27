#pragma once

#include "model/ship/Military.h"
#include "model/ship/Ship.h"
#include "model/ship/Transport.h"

/**
 * @brief Гибридный корабль (Ship + Military + Transport)
 */
class TransportMilitaryShip: public DefaultShip, 
                            public DefaultMilitary, 
                            public DefaultTransport {
    public:
        TransportMilitaryShip(const std::string& name, unsigned max_velocity,
                             unsigned current_velocity, unsigned max_HP,
                             unsigned cost, const std::string& captain, 
                             double alpha, unsigned max_cargo_weight, unsigned current_cargo_weight,
                             double beta);

        [[nodiscard]] unsigned get_actual_max_velocity() noexcept override;
        std::string get_type() const override { return "TransportMilitaryShip"; }
};
