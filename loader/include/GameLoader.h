#pragma once

#include "config/Loader.h"
#include "strategy/AttackStrategy.h"
#include <memory>
#include <string>
#include <vector>

/**
 * @brief Реализация загрузчика игры
 *
 */
class GameLoader final : public Loader {
    std::unique_ptr<MissionRepository> repository_;
    std::unique_ptr<MissionService> missionService_;
    std::unique_ptr<StateService> stateService_;
    std::unique_ptr<ShipMapper> shipMapper_;
    std::vector<std::unique_ptr<SubtypeMapper<Ship, ShipDto, std::unique_ptr<Ship>>>> shipSubtypeMappers_;
    std::unique_ptr<WeaponMapper> weaponMapper_;
    std::vector<std::unique_ptr<SubtypeMapper<Weapon, WeaponDto, std::unique_ptr<Weapon>>>> weaponSubtypeMappers_;
    std::unique_ptr<GamePresenter> presenter_;
    std::unique_ptr<GameView> view_;
    std::unique_ptr<AttackStrategy> strategy_;

public:
    GameLoader() = default;
    GamePresenter& get_presenter() override;
    MissionRepository& get_repository() override;
    MissionService& get_service() override;
    StateService& get_state_service() override;
    ShipMapper& get_ship_mapper() override;
    WeaponMapper& get_weapon_mapper() override;
    GameView & get_game_view() override;
    AttackStrategy & get_attack_strategy() override;

};
