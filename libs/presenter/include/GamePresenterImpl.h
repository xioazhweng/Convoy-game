#pragma once

#include "presenter/GamePresenter.h"
#include "mapper/ShipMapper.h"
#include "mapper/WeaponMapper.h"
#include "model/repository/MissionRepository.h"
#include "service/MissionService.h"
#include "service/StateService.h"
#include <string>

/**
 * @brief Реализация GamePresenter
 */
class GamePresenterImpl final : public GamePresenter {
    private:
        MissionRepository& repo_;
        MissionService& mission_service_;
        StateService& state_service_;

        ShipMapper& ship_mapper_;
        WeaponMapper& weapon_mapper_;
        
        bool gameLoaded_ = false;
        GameStateDto currentGameState_;

        
        void load_all_game_data(const std::string& config_path);
        void process_ship_units_and_link_weapons();
        void populate_fleets_from_ship_units();
        void save_all_game_data(const std::string& config_path) const;
        void update_ship_units_from_fleets();
        void sync_dynamic_state_from_repo();
        void sync_mission_params_from_repo();
        void sync_mission_params_to_repo();
        ShipDto* find_ship_by_callsign(const std::string& callsign);
        const ShipDto* find_ship_by_callsign(const std::string& callsign) const;
        WeaponDto* find_weapon_by_id(unsigned id);
        const WeaponDto* find_weapon_by_id(unsigned id) const;
        void validate_game_state() const;
        void validate_ship_weapons_link(const ShipDto& ship, const WeaponDto& weapon, const std::string& place) const;

        // Ограничения на покупку/размещение корабля игроком
        [[nodiscard]] bool is_buy_ship_position_allowed(const Point& p) const;

    public:
        /**
         * @brief Конструктор с параметрами
         * @param repo Ссылка на MissionRepository
         * @param mission_service Ссылка на MissionService
         * @param state_service Ссылка на StateService
         * @param ship_mapper Ссылка на ShipMapper
         * @param weapon_mapper Ссылка на WeaponMapper
         */
        GamePresenterImpl(MissionRepository& repo, MissionService& mission_service, 
                        StateService& state_service, ShipMapper& ship_mapper, 
                        WeaponMapper& weapon_mapper);
        ~GamePresenterImpl() = default;

        /**
         * @brief Загрузить состояние игры
         * @param config_path Путь к файлу конфигурации
         * @throw std::runtime_error ошибка при загрузке игры
         */
        void load_game_state(const std::string& config_path) override;
        
        /**
         * @brief Сохранить состояние игры
         * @param config_path Путь к файлу конфигурации
         * @throw std::runtime_error ошибка при сохранении игры
         */
        void save_game_state(const std::string& config_path) override;
        
        /**
         * @brief Получить состояние игры
         * @return Константная ссылка на GameStateDto
         */
        [[nodiscard]] const GameStateDto& get_game_state() const override;
        
        /**
         * @brief Получить список кораблей
         * @return Вектор ShipDto
         */
        [[nodiscard]] std::vector<ShipDto> get_ships() const override;
        
        /**
         * @brief Получить список юнитов кораблей
         * @return Вектор ShipUnitDTO
         */
        [[nodiscard]] std::vector<ShipUnitDTO> get_ship_units() const override;
        
        /**
         * @brief Получить корабль по позывному
         * @param callsign Позывной корабля
         * @return ShipDto найденного корабля
         */
        [[nodiscard]] ShipDto get_ship_by_callsign(const std::string& callsign) const override;
        
        /**
         * @brief Получить список оружия
         * @return Вектор WeaponDto
         */
        [[nodiscard]] std::vector<WeaponDto> get_weapons() const override;
        
        /**
         * @brief Получить оружие по ID
         * @param id ID оружия
         * @return WeaponDto найденного оружия
         */
        [[nodiscard]] WeaponDto get_weapon_by_id(unsigned id) const override;
        
        /**
         * @brief Назначить оружие кораблю
         * @param ship_callsign Позывной корабля
         * @param weapon_id ID оружия
         * @param place Место установки
         * @return true если назначение успешно, иначе false
         */
        bool assign_weapon_to_ship(const std::string& ship_callsign, unsigned weapon_id, const std::string& place) override;
        
        /**
         * @brief Снять оружие с корабля
         * @param ship_callsign Позывной корабля
         * @param place Место установки
         * @return true если снятие успешно, иначе false
         */
        bool remove_weapon_from_ship(const std::string& ship_callsign, const std::string& place) override;
        
        /**
        * @brief Установить оружие с корабля
        * @param weapon - данные корабля
        * @param ship_callsign Позывной корабля
        * @param place Место установки
        * @return true если установка успешно, иначе false
        */
        bool set_weapon_from_ship(const WeaponDto & weapon, const std::string& ship_callsign, const std::string& place) override;

        
         /**
        * @brief Добавить корабль в имперский флот
        * @param ship Данные корабля
        * @param p Точка на которую ставим корабль
        */
         void buy_ship(const ShipDto& ship, const Point& p) override;

        /**
        * @brief Добавить корабль в имперский флот
        * @param callsign Позывной корабля
        * 
        */
        bool sell_ship(const std::string & callsign) override;

        /**
         * @brief Проверить, загружена ли игра
         * @return true если игра загружена, иначе false
         */
        [[nodiscard]] bool is_game_loaded() const override;


        /**
         * @brief Обработать ход игрока
         */
        void process_player_turn() override;
        
        /**
         * @brief Обработать атаки пиратов
         */
        void process_pirate_attacks() override;
        
        /**
         * @brief Переместить корабли
         */
        void move_ships() override;
        
        /**
         * @brief Проверить прибытие на базу
         * @return true если корабли прибыли, иначе false
         */
        bool check_base_arrival() override;
        
        /**
         * @brief Завершить ход
         */
        void end_turn() override;

        /**
         * @brief Перезарядить все оружие имперцев
         * @param price Цена перезарядки
         * @return true если перезарядка успешна, иначе false
         */
        bool reload_all_imperial_weapons(unsigned price) override;
};
