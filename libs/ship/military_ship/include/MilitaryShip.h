#pragma once

#include "model/ship/Military.h"
#include "model/ship/Ship.h"

/**
 * @brief Военный корабль (Ship + Military)
 */
class MilitaryShip: public DefaultShip, public DefaultMilitary {
    public:
        MilitaryShip(const std::string& name, unsigned max_velocity,
                    unsigned current_velocity, unsigned max_HP,
                    unsigned cost, const std::string& captain, double beta);

        [[nodiscard]] unsigned get_actual_max_velocity() noexcept override;
        
        std::string get_type() const override { return "MilitaryShip"; }
};
