#pragma once

#include "presenter/GamePresenter.h"
#include "model/repository/MissionRepository.h"
#include "strategy/AttackStrategy.h"
#include "service/MissionService.h"
#include "service/StateService.h"
#include "mapper/ShipMapper.h"
#include "mapper/WeaponMapper.h"
#include "view/GameView.h"

/**
 * @brief Интерфейс загрузчика
 */
class Loader {
public:
    virtual ~Loader() = default;
    
    /**
     * @brief Получить presenter
     * @return Ссылка на GamePresenter
     */
    virtual GamePresenter& get_presenter() = 0;
    
    /**
     * @brief Получить repository
     * @return Ссылка на MissionRepository
     */
    virtual MissionRepository& get_repository() = 0;
    
    /**
     * @brief Получить mission service
     * @return Ссылка на MissionService
     */
    virtual MissionService& get_service() = 0;
    
    /**
     * @brief Получить state service
     * @return Ссылка на StateService
     */
    virtual StateService& get_state_service() = 0;
    
    /**
     * @brief Получить ship mapper
     * @return Ссылка на ShipMapper
     */
    virtual ShipMapper& get_ship_mapper() = 0;
    
    /**
     * @brief Получить weapon mapper
     * @return Ссылка на WeaponMapper
     */
    virtual WeaponMapper& get_weapon_mapper() = 0;
    
    /**
     * @brief Получить game view
     * @return Ссылка на GameView
     */
    virtual GameView& get_game_view() = 0;

    /**
     * @brief Получить attack_strategy
     * @return Ссылка на Attackstrategy
     */
    virtual AttackStrategy & get_attack_strategy()= 0;

};
