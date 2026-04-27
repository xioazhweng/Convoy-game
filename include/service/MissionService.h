#pragma once

#include "model/ship/Ship.h"
#include "model/weapon/Weapon.h"
#include "model/point/Point.h"
#include "model/place/Place.h"

#include <cstddef> 
#include <memory>
#include <string>
#include <vector>


/**
 * @brief Интерфейс сервисов миссии
 *
 */
class MissionService {
    public:
        virtual ~MissionService() = default;
        
        // сервисы кораблей
        /**
         * @brief Купить корабль
         * @param ship Умный указатель на корабль
         * @param coords Координаты корабля
         * @return true если покупка успешна, иначе false
         */
        virtual bool buy_ship(std::unique_ptr<Ship> ship, const Point& coords) = 0;
        
        /**
         * @brief Продать корабль
         * @param callsign Позывной корабля
         * @return true если продажа успешна, иначе false
         */
        virtual bool sell_ship(const std::string & callsign) = 0;
        
        /**
         * @brief Переместить корабли
         */
        virtual void move() = 0;
       

        // грузовые сервисы
        /**
         * @brief Проверить выполнение цели по грузу
         * @return true если цель выполнена, иначе false
         */
        virtual bool check_cargo_goal() = 0;
        
        /**
         * @brief Загрузить груз
         * @param callsign Позывной корабля
         * @param weight Вес груза
         * @return true если загрузка успешна, иначе false
         */
        virtual bool load_cargo(const std::string & callsign, size_t weight, unsigned cost) = 0;
        
        /**
         * @brief Разгрузить корабль
         * @param callsign Позывной корабля
         * @return true если разгрузка успешна, иначе false
         */
        virtual bool unload_cargo(const std::string & callsign) = 0;
        
        /**
         * @brief Автоматически распределить груз
         * @return true если распределение успешно, иначе false
         */
        virtual bool auto_distribute_cargo() = 0;

        // сервисы атаки
        /**
         * @brief Атаковать по области
         * @param callsigns Позывные целей для атаки
         * @param damage Урон
         * @param is_pirates true для пиратов, false для имперцев
         */
        virtual void attack_by_area(const std::vector<std::string>& callsigns, unsigned damage, bool is_pirates = true) = 0;
        
        /**
         * @brief Атаковать корабль
         * @param callsign Позывной корабля-атакующего
         * @param place Место установки оружия
         * @param target_callsign Позывной корабля-цели
         * @param is_pirates true для пиратов, false для имперцев
         * @return true если атака успешна, иначе false
         */
        virtual bool attack(const std::string & callsign, Place place, 
                            const std::string & target_callsign, bool is_pirates = true) = 0;

        // сервисы оружия
        /**
         * @brief Купить оружие
         * @param weapon Умный указатель на оружие
         * @param place Место установки
         * @return true если покупка успешна, иначе false
         */
        virtual bool buy_weapon(const std::string & callsign, std::unique_ptr<Weapon> weapon, Place place) = 0;
        
        /**
         * @brief Продать оружие
         * @param callsign Позывной корабля
         * @param place Место установки
         * @param removed_weapon Умный указатель на проданное оружие
         * @return true если продажа успешна, иначе false
         */
        virtual bool sell_weapon(const std::string & callsign, std::unique_ptr<Weapon> weapon, Place place) = 0;
        
        /**
         * @brief Снять оружие
         * @param callsign Позывной корабля
         * @param place Место установки
         * @return true если снятие успешно, иначе false
         */
        virtual bool uninstall_weapon(const std::string & callsign, Place place) = 0;
        
        /**
         * @brief Установить оружие
         * @param callsign Позывной корабля
         * @param weapon Умный указатель на оружие
         * @param place Место установки
         * @return true если установка успешна, иначе false
         */
        virtual bool install_weapon(const std::string & callsign, std::unique_ptr<Weapon> weapon, 
                                    Place place) = 0;

        /**
         * @brief Автоматическая атака пиратов
         * @return true если атака успешна, иначе false
         */
        virtual bool pirate_attack(unsigned range, unsigned damage) = 0;


        /**
         * @brief Автоматическая атака пиратов (параллельная версия)
         *
         * @details
         * По умолчанию вызывает последовательную версию `pirate_attack()`.
         * Реализация может переопределить и выполнить обработку в несколько потоков.
         *
         * @param range Дальность атаки
         * @param damage Урон за одно попадание
         * @param thread_count Кол-во потоков (0 => выбрать автоматически)
         */
        virtual bool pirate_attack_parallel(unsigned range, unsigned damage, unsigned thread_count = 0) {
            (void)thread_count;
            return pirate_attack(range, damage);
        }

    };
