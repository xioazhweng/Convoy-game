#pragma once

#include <string>
#include "dto/GameStateDto.h"

/**
 * @brief Интрефейс сервиса загрузки/сохранение состояние игры
 */
class StateService {
    public:
        virtual ~StateService() = default;
        
        /**
         * @brief Установить состояние игры
         * @param state Состояние игры
         */
        virtual void set_game_state(GameStateDto state) = 0;

        /**
         * @brief Загрузить репозиторий
         * @param path Путь к файлу
         */
        virtual void load_repo(const std::string& path) = 0;
        
        /**
         * @brief Загрузить параметры оружия
         * @param path Путь к файлу
         */
        virtual void load_weapons_params(const std::string& path) = 0;
        
        /**
         * @brief Загрузить параметры кораблей
         * @param path Путь к файлу
         */
        virtual void load_ships_params(const std::string& path)  = 0;
        
        /**
         * @brief Загрузить параметры миссии
         * @param path Путь к файлу
         */
        virtual void load_mission_params(const std::string& path)  = 0;

        /**
         * @brief Загрузить корабли
         * @param path Путь к файлу
         */
        virtual void load_ships(const std::string& path) = 0;
        
        /**
         * @brief Загрузить юниты кораблей
         * @param path Путь к файлу
         */
        virtual void load_ship_units(const std::string& path) = 0;
        
        /**
         * @brief Загрузить оружие
         * @param path Путь к файлу
         */
        virtual void load_weapons(const std::string& path) = 0;
        
        /**
         * @brief Сохранить репозиторий
         * @param path Путь к файлу
         */
        virtual void save_repo(const std::string& path) const  = 0; 
        
        /**
         * @brief Сохранить параметры оружия
         * @param path Путь к файлу
         */
        virtual void save_weapons_params(const std::string& path) const = 0;
        
        /**
         * @brief Сохранить параметры кораблей
         * @param path Путь к файлу
         */
        virtual void save_ships_params(const std::string& path) const = 0;
        
        /**
         * @brief Сохранить параметры миссии
         * @param path Путь к файлу
         */
        virtual void save_mission_params(const std::string& path) const = 0;
        
        /**
         * @brief Сохранить корабли
         * @param path Путь к файлу
         */
        virtual void save_ships(const std::string& path) const = 0;
        
        /**
         * @brief Сохранить юниты кораблей
         * @param path Путь к файлу
         */
        virtual void save_ship_units(const std::string& path) const  = 0;
        
        /**
         * @brief Сохранить оружие
         * @param path Путь к файлу
         */
        virtual void save_weapons(const std::string& path) const  = 0;
        

        /**
         * @brief Получить состояние игры
         * @return Константная ссылка на GameStateDto
         */
        [[nodiscard]] virtual const GameStateDto& get_game_state() const = 0;
        
        /**
         * @brief Получить список кораблей
         * @return Константная ссылка на вектор ShipDto
         */
        [[nodiscard]] virtual const std::vector<ShipDto>& get_ships() const = 0;
        
        /**
         * @brief Получить список юнитов кораблей
         * @return Константная ссылка на вектор ShipUnitDTO
         */
        [[nodiscard]] virtual const std::vector<ShipUnitDTO>& get_ship_units() const = 0;
        
        /**
         * @brief Получить список оружия
         * @return Константная ссылка на вектор WeaponDto
         */
        [[nodiscard]] virtual const std::vector<WeaponDto>& get_weapons() const = 0;
};

