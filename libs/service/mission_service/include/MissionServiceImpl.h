#pragma once

#include "service/MissionService.h"
#include "model/ship/Ship.h"
#include "model/weapon/Weapon.h"
#include "model/point/Point.h"
#include "strategy/AttackStrategy.h"
#include "model/repository/MissionRepository.h"

#include <memory>

/**
 * @brief Реализация MissionService
 */
class MissionServiceImpl final: public MissionService {
    private:
        MissionRepository & repo;
        AttackStrategy & strategy;
        std::vector<Point> find_opposite_ships_in_fire_range(const Point & p, unsigned range, bool is_pirate);
    public:
        /**
         * @brief Конструктор с параметрами
         * @param repository Ссылка на репозиторий миссии
         */
        explicit MissionServiceImpl(MissionRepository& repo_, AttackStrategy & as);

        /**
         * @brief Convenience ctor: uses default attack strategy.
         */
        explicit MissionServiceImpl(MissionRepository& repo_);
        
        // сервисы кораблей
        /**
         * @brief Купить корабль
         * @param callsign Позывной корабля
         * @param ship Умный указатель на корабль
         * @param coords Координаты корабля
         * @param captain Имя капитана
         * @return true если покупка успешна, иначе false
         */
        bool buy_ship(std::unique_ptr<Ship> ship, const Point& coords) override;
        
        /**
         * @brief Продать корабль
         * @param callsign Позывной корабля
         * @return true если продажа успешна, иначе false
         */
        bool sell_ship(const std::string & callsign) override;
        
        /**
         * @brief Переместить корабли
         */
        void move() override;
       
        // грузовые сервисы
        /**
         * @brief Проверить выполнение цели по грузу
         * @return true если цель выполнена, иначе false
         */
        bool check_cargo_goal() override;
        
        /**
         * @brief Загрузить груз
         * @param callsign Позывной корабля
         * @param weight Вес груза
         * @return true если загрузка успешна, иначе false
         */
        bool load_cargo(const std::string  & callsign, size_t weight, unsigned cost) override;
        
        /**
         * @brief Разгрузить корабль
         * @param callsign Позывной корабля
         * @return true если разгрузка успешна, иначе false
         */
        bool unload_cargo(const std::string & callsign) override;
        
        /**
         * @brief Автоматически распределить груз
         * @return true если распределение успешно, иначе false
         */
        bool auto_distribute_cargo() override;

        // сервисы атаки
        /**
         * @brief Атаковать по области
         * @param callsigns Позывные целей для атаки
         * @param damage Урон
         * @param is_pirates true для пиратов, false для имперцев
         */
        void attack_by_area(const std::vector<std::string>& callsigns, unsigned damage,
                            bool is_pirates = true) override;
        
        /**
         * @brief Атаковать корабль
         * @param callsign Позывной корабля-атакующего
         * @param place Место установки оружия
         * @param target_callsign Позывной корабля-цели
         * @param is_pirates true для пиратов, false для имперцев
         * @return true если атака успешна, иначе false
         */
        bool attack(const std::string & callsign, Place place, const std::string & target_callsign,
                    bool is_pirates = true) override;



        // сервисы оружия
        /**
         * @brief Купить оружие
         * @param weapon Умный указатель на оружие
         * @param place Место установки
         * @return true если покупка успешна, иначе false
         */
        bool buy_weapon(const std::string & callsign, std::unique_ptr<Weapon> weapon, Place place) override;
        
        /**
         * @brief Продать оружие
         * @param callsign Позывной корабля
         * @param place Место установки
         * @param removed_weapon Умный указатель на проданное оружие
         * @return true если продажа успешна, иначе false
         */
        bool sell_weapon(const std::string & callsign, std::unique_ptr<Weapon> weapon, Place place) override;
        
        /**
         * @brief Снять оружие
         * @param callsign Позывной корабля
         * @param place Место установки
         * @return true если снятие успешно, иначе false
         */
        bool uninstall_weapon(const std::string & callsign, Place place) override;
        
        /**
         * @brief Установить оружие
         * @param callsign Позывной корабля
         * @param weapon Умный указатель на оружие
         * @param place Место установки
         * @return true если установка успешна, иначе false
         */
        bool install_weapon(const std::string & callsign, std::unique_ptr<Weapon> weapon, 
                            Place place) override;

        /**
         * @brief Автоматическая атака пиратов
         * @return true если атака успешна, иначе false
         */
        bool pirate_attack(unsigned range, unsigned damage) override;

        /**
         * @brief Автоматическая атака пиратов (параллельная версия)
         */
        bool pirate_attack_parallel(unsigned range, unsigned damage, unsigned thread_count = 0) override;


        /**
         * @brief Автоматический выбор целей для атаки
         * @param first Итератор начала контейнера вражеских кораблей
         * @param last Итератор конца контейнера
         * @param range Радиус атаки
         * @param is_pirates true для пиратов, false для имперцев
         * @return Вектор позывных (callsign) целей для атаки
         */
        template <typename It>
        std::vector<std::string> auto_shoot(It first, It last, unsigned range, bool is_pirate) {
            std::vector<std::string> targets;

            for (auto it = first; it != last; ++it) {
                const auto & ship = it->second;
                auto vect = strategy.auto_shoot(ship.coords, range, is_pirate);
                if (vect.empty()) {
                    continue;
                }
                targets.insert(targets.end(), std::make_move_iterator(vect.begin()), std::make_move_iterator(vect.end()));
            }
            return targets;
        };

        template <typename It>
        bool attack(It first, It last, unsigned damage, bool is_pirates) {
            for (auto it = first; it != last; ++it) {
                if constexpr (requires { it->second; }) {
                    const auto & ship = it->second;
                    attack(damage, ship.coords, is_pirates);
                } else {
                    for (const auto& p : *it) {
                        attack(damage, p, is_pirates);
                    }
                }
            }
            return true;
        }
    };
