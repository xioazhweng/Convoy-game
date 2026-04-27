#include "../include/RandomAttackStrategy.h"
#include <random>
#include <chrono>

std::vector<std::string> RandomAttackStrategy::auto_shoot(const Point & p, unsigned range, bool is_pirate) {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 gen(seed);

    auto all_targets = DefaultAttackStrategy::auto_shoot(p, range, is_pirate);
    if (all_targets.empty()) {
        return {};
    }

    std::uniform_int_distribution<size_t> pick(0, all_targets.size() - 1);
    return {all_targets[pick(gen)]};
    
};
