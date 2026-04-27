#pragma once

#include <vector>
#include <string>

#include "dto/GameStateDto.h"
#include "dto/ShipDto.h"
#include "dto/WeaponDto.h"
#include "dto/ShipUnitDto.h"
#include "model/point/Point.h"


/**
 * @brief Интерфейс Presenter 
 *
 */
class GamePresenter {
public:
    virtual ~GamePresenter() = default;
    
    /**
     * @brief Загрузить состояние игры
     * @param config_path Путь к файлу конфигурации
     */
    virtual void load_game_state(const std::string& config_path) = 0;
    
    /**
     * @brief Сохранить состояние игры
     * @param config_path Путь к файлу конфигурации
     */
    virtual void save_game_state(const std::string& config_path) = 0;

    /**
     * @brief Получить состояние игры
     * @return Константная ссылка на GameStateDto
     */
    [[nodiscard]] virtual const GameStateDto& get_game_state() const = 0;
        
    /**
     * @brief Получить список кораблей
     * @return Вектор ShipDto
     */
    [[nodiscard]] virtual std::vector<ShipDto> get_ships() const = 0;
    
    /**
     * @brief Получить список юнитов кораблей
     * @return Вектор ShipUnitDTO
     */
    [[nodiscard]] virtual std::vector<ShipUnitDTO> get_ship_units() const = 0;
    
    /**
     * @brief Получить корабль по позывному
     * @param callsign Позывной корабля
     * @return ShipDto найденного корабля
     */
    [[nodiscard]] virtual ShipDto get_ship_by_callsign(const std::string& callsign) const = 0;
    
    /**
     * @brief Получить список оружия
     * @return Вектор WeaponDto
     */
    [[nodiscard]] virtual std::vector<WeaponDto> get_weapons() const = 0;
    
    /**
     * @brief Получить оружие по ID
     * @param id ID оружия
     * @return WeaponDto найденного оружия
     */
    [[nodiscard]] virtual WeaponDto get_weapon_by_id(unsigned id) const = 0;
    
    /**
     * @brief Назначить оружие кораблю
     * @param ship_callsign Позывной корабля
     * @param weapon_id ID оружия
     * @param place Место установки
     * @return true если назначение успешно, иначе false
     */
    virtual bool assign_weapon_to_ship(const std::string& ship_callsign, unsigned weapon_id, const std::string& place) = 0;
    
    /**
     * @brief Снять оружие с корабля
     * @param ship_callsign Позывной корабля
     * @param place Место установки
     * @return true если снятие успешно, иначе false
     */
    virtual bool remove_weapon_from_ship(const std::string& ship_callsign, const std::string& place) = 0;
    
     /**
     * @brief Установить оружие с корабля
     * @param weapon - данные корабля
     * @param ship_callsign Позывной корабля
     * @param place Место установки
     * @return true если установка успешно, иначе false
     */
    virtual bool set_weapon_from_ship(const WeaponDto & weapon, const std::string& ship_callsign, const std::string& place) = 0;

    /**
     * @brief Добавить корабль в имперский флот
     * @param ship Данные корабля
     * @param p Точка на которую ставим корабль
     */
    virtual void buy_ship(const ShipDto& ship, const Point& p) = 0;

    /**
     * @brief Добавить корабль в имперский флот
     * @param callsign Позывной корабля
     * 
     */
    virtual bool sell_ship(const std::string & callsign) = 0;
    
    /**
     * @brief Проверить, загружена ли игра
     * @return true если игра загружена, иначе false
     */
    [[nodiscard]] virtual bool is_game_loaded() const = 0;

    /**
     * @brief Обработать ход игрока
     */
    virtual void process_player_turn() = 0;
    
    /**
     * @brief Обработать атаки пиратов
     */
    virtual void process_pirate_attacks() = 0;
    
    /**
     * @brief Переместить корабли
     */
    virtual void move_ships() = 0;
    
    /**
     * @brief Проверить прибытие на базу
     * @return true если корабли прибыли, иначе false
     */
    virtual bool check_base_arrival() = 0;
    
    /**
     * @brief Завершить ход
     */
    virtual void end_turn() = 0;
    
    /**
     * @brief Перезарядить все оружие имперцев
     * @param price Цена перезарядки
     * @return true если перезарядка успешна, иначе false
     */
    virtual bool reload_all_imperial_weapons(unsigned price) = 0;

    
};
