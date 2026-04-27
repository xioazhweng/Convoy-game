#pragma once

#include "model/point/Point.h"
#include "model/repository/MissionRepository.h"
#include <string>
#include <vector>

/**
 * @brief Стратегия выбора целей для атаки 
 */
class AttackStrategy {
    public:
        virtual ~AttackStrategy() = default;
        
        /**
         * @brief Автоматический выбор целей для атаки
         * @param range Радиус атаки
         * @param is_pirate true если атакующие корабли являются пиратами, иначе - false
         * @return Вектор позывных (callsign) целей для атаки
         */
        virtual std::vector<std::string> auto_shoot(unsigned range, bool is_pirate) = 0;

         /**
         * @brief Автоматический выбор целей для атаки
         * @param p Точка на которой располагается корабль
         * @param range Радиус атаки
         * @param is_pirate true если атакующие корабли являются пиратами, иначе - false
         * @return Вектор позывных (callsign) целей для атаки
         */
        virtual std::vector<std::string> auto_shoot(const Point & p, unsigned range, bool is_pirate) = 0;


};

/**
 * @brief Простая (базовая) стратегия атаки
 *
 */
class DefaultAttackStrategy : public AttackStrategy {
    protected:
        MissionRepository & repo;
    public:
        DefaultAttackStrategy(MissionRepository & repo_): repo(repo_) {};
         /**
         * @brief Автоматический выбор целей для атаки
         * @param range Радиус атаки
         * @param is_pirate true если атакующие корабли являются пиратами, иначе - false
         * @return Вектор позывных (callsign) целей для атаки
         */
        std::vector<std::string> auto_shoot(unsigned range, bool is_pirate) override {
            std::vector<std::string> targets;
            auto & table = is_pirate ? repo.get_pirates() : repo.get_imperials();

            for (auto & ship : table) {
                auto vect = auto_shoot(ship.second.coords, range, is_pirate); 
                if (vect.empty()) {
                    continue;
                }
                targets.insert(targets.end(), std::make_move_iterator(vect.begin()), std::make_move_iterator(vect.end()));
            }
            return targets;
        };

         

         /**
         * @brief Автоматический выбор целей для атаки
         * @param p Точка на которой располагается корабль
         * @param range Радиус атаки
         * @param is_pirate true если атакующие корабли являются пиратами, иначе - false
         * @return Вектор позывных (callsign) целей для атаки
         */
        std::vector<std::string> auto_shoot(const Point & p, unsigned range, bool is_pirate) override {
            const bool target_is_pirate = !is_pirate;
            auto points = repo.find_opposite_ships_in_fire_range(p, range, is_pirate);

            std::vector<std::string> targets;
            targets.reserve(points.size());
            for (const auto& pt : points) {
                auto clsgn = repo.find_by_point(pt, target_is_pirate);
                if (clsgn == std::nullopt) {
                    continue;
                }
                targets.push_back(*clsgn);
            }
            return targets;
        };
};

