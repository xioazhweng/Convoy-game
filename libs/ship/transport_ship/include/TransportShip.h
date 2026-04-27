#pragma once

#include "model/ship/Transport.h"
#include "model/ship/Ship.h"

/**
 * @brief Транспортный корабль (Ship + Transport)
 */
class TransportShip: public DefaultShip, public DefaultTransport {
    public:
        TransportShip(const std::string& name, unsigned max_velocity,
                     unsigned current_velocity, unsigned max_HP,
                     unsigned cost, const std::string& captain,
                     double alpha, unsigned max_cargo_weight, unsigned current_cargo_weight);

        unsigned get_actual_max_velocity() noexcept override;
        std::string get_type() const override { return "TransportShip";}
};
