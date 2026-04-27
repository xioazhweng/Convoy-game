#include "TransportShip.h"

TransportShip::TransportShip(const std::string& name, unsigned max_velocity,
                     unsigned current_velocity, unsigned max_HP,
                     unsigned cost, const std::string& captain,
                     double alpha, unsigned max_cargo_weight, unsigned current_cargo_weight)
    : DefaultShip(name, max_velocity, current_velocity, max_HP, cost, captain),
      DefaultTransport(alpha, max_cargo_weight, current_cargo_weight) {};

unsigned TransportShip::get_actual_max_velocity() noexcept {
    double cargo_factor = 1.0 - (static_cast<double>(current_cargo_weight) / max_cargo_weight);
    return static_cast<unsigned>(max_velocity * alpha * cargo_factor);
}