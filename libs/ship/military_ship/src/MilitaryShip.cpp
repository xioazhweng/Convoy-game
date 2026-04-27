#include "MilitaryShip.h"

MilitaryShip::MilitaryShip(const std::string& name, unsigned max_velocity,
                          unsigned current_velocity, unsigned max_HP,
                          unsigned cost, const std::string& captain, double beta) 
    : DefaultShip(name, max_velocity, current_velocity, max_HP, cost, captain), 
    DefaultMilitary(beta) {};


unsigned MilitaryShip::get_actual_max_velocity() noexcept {
    double weapon_factor = 1.0 - (static_cast<double>(weapon_count()) * weapon_const);
    return static_cast<unsigned>(max_velocity * beta * weapon_factor);
}