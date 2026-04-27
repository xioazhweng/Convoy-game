#include "TransportMilitaryShip.h"


TransportMilitaryShip::TransportMilitaryShip(const std::string& name, unsigned max_velocity,
                                             unsigned current_velocity, unsigned max_HP,
                                             unsigned cost, const std::string& captain, 
                                             double alpha, unsigned max_cargo_weight, unsigned current_cargo_weight,
                                             double beta) 
    : DefaultShip(name, max_velocity, current_velocity, max_HP, cost, captain), 
      DefaultMilitary(beta),
      DefaultTransport(alpha, max_cargo_weight, current_cargo_weight) {};

unsigned TransportMilitaryShip::get_actual_max_velocity() noexcept {
    double cargo_factor = 1.0 - (static_cast<double>(current_cargo_weight) / max_cargo_weight);
    double weapon_factor = 1.0 - (static_cast<double>(weapon_count()) * weapon_const); 
    return static_cast<unsigned>(max_velocity * alpha * beta * cargo_factor * weapon_factor);
}