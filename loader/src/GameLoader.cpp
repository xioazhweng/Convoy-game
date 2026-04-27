#include "../include/GameLoader.h"
#include <memory>

#include "../../libs/weapon/cannon/include/CannonMapper.h"
#include "../../libs/weapon/missile/include/MissileMapper.h"
#include "../../libs/weapon/torpedo/include/TorpedoMapper.h"
#include "../../libs/ship/military_ship/include/MilitaryShipMapper.h"
#include "../../libs/ship/transport_ship/include/TransportShipMapper.h"
#include "../../libs/ship/transportmilitary_ship/include/TMShipMapper.h"
#include "../../libs/service/mission_service/include/MissionServiceImpl.h"
#include "../../libs/service/state_service/include/YalmStateService.h"
#include "../../libs/presenter/include/GamePresenterImpl.h"
#include "../../libs/repository/include/MissionRepositoryImpl.h"
#include "../../libs/view/include/SfmlGameView.h"

#include "RandomAttackStrategy.h"
#include "mapper/ShipMapper.h"
#include "mapper/WeaponMapper.h"


GamePresenter & GameLoader::get_presenter() {
    if (presenter_ == nullptr) {
        presenter_ = std::make_unique<GamePresenterImpl>(
            get_repository(), get_service(), get_state_service(),
            get_ship_mapper(), get_weapon_mapper());
    }
    return *presenter_;
}

MissionRepository& GameLoader::get_repository() {
    if (repository_ == nullptr) {
        repository_ = std::make_unique<MissionRepositoryImpl>();
    }
    return *repository_;
}

MissionService& GameLoader::get_service() {
    if (missionService_ == nullptr) {
        missionService_ = std::make_unique<MissionServiceImpl>(get_repository(), get_attack_strategy());
    }
    return *missionService_;
}

StateService& GameLoader::get_state_service() {
    if (stateService_ == nullptr) {
        stateService_ = std::make_unique<YalmStateService>("");
    }
    return *stateService_;
}

ShipMapper& GameLoader::get_ship_mapper() {
    if (shipMapper_ == nullptr) {
        shipSubtypeMappers_.emplace_back(std::make_unique<MilitaryShipSuperMapper>());
        shipSubtypeMappers_.emplace_back(std::make_unique<TransportShipSuperMapper>());
        shipSubtypeMappers_.emplace_back(std::make_unique<TMShipSuperMapper>());
        auto shipMapper = std::make_unique<PolymorphicShipMapper>();
        for (auto & shipSubtypeMapper : shipSubtypeMappers_) {
            shipMapper->add_sub_mapper(*shipSubtypeMapper);
        }
        shipMapper_ = std::move(shipMapper);
    }
    return *shipMapper_;
}

WeaponMapper& GameLoader::get_weapon_mapper() {
    if (weaponMapper_ == nullptr) {
        weaponSubtypeMappers_.emplace_back(std::make_unique<CannonSuperMapper>());
        weaponSubtypeMappers_.emplace_back(std::make_unique<MissileSuperMapper>());
        weaponSubtypeMappers_.emplace_back(std::make_unique<TorpedoSuperMapper>());
        
        auto weaponMapper = std::make_unique<PolymorphicWeaponMapper>();
        for (auto & weaponSubtypeMapper : weaponSubtypeMappers_) {
            weaponMapper->add_sub_mapper(*weaponSubtypeMapper);
        }
        weaponMapper_ = std::move(weaponMapper);
    }
    return *weaponMapper_;
}

GameView & GameLoader::get_game_view() {
    if (view_ == nullptr) {
        view_ = std::make_unique<SfmlGameView>(get_presenter(), get_service());
    }
    return *view_;
};

AttackStrategy & GameLoader::get_attack_strategy() {
    if (strategy_ == nullptr) {
        strategy_ = std::make_unique<RandomAttackStrategy>(get_repository());
    }
    return *strategy_;
}