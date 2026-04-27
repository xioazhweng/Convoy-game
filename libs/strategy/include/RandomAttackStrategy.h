#ifndef RANDOMATTACKSTRATEGY_H
#define RANDOMATTACKSTRATEGY_H

#include "model/repository/MissionRepository.h"
#include "strategy/AttackStrategy.h"


/**
 * @brief Стратегия атаки, выбирающая цели случайным образом
 */
class RandomAttackStrategy : public DefaultAttackStrategy {
    public:
        RandomAttackStrategy(MissionRepository & repo_): DefaultAttackStrategy(repo_) {};
        std::vector<std::string> auto_shoot(const Point & p, unsigned range, bool is_pirate) override;
};

#endif
