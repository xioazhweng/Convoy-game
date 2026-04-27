#pragma once

#include "lookup_table/LookUpTable.h"
#include "model/point/Point.h"
#include "model/ship/Ship.h"
#include <vector>
#include <string>
#include <cmath>
#include <map>
#include <optional>

/**
 * @brief Карта мисии
 */
struct MissionMap {
    unsigned dx;
    unsigned dy;
    Point center;
    bool is_in_mission_area(Point p) const {
        int delta_x = std::abs(p.get_x() - center.get_x());
        int delta_y = std::abs(p.get_y() - center.get_y());
        return delta_x <= dx && delta_y <= dy;
    }
};

/**
 * @brief Параметры миссии в рантайме
 *
 */
struct Mission_params {
    unsigned budget;
    unsigned spent_budged;
    unsigned total_cargo_weight;
    unsigned goal_weight;
    unsigned lost_weight;
    unsigned delivered_weight;
    Point A_base;
    Point B_base;
    std::vector<Point> pirates_bases;
    MissionMap map;
};

/**
 * @brief Запись о корабле в таблице репозитория
 *
 */
struct info {
    std::string callsign;
    std::unique_ptr<Ship> ship;
    Point coords;
    info() : callsign("I1"), ship(nullptr), coords(0, 0) {}
    info(std::string c, std::unique_ptr<Ship> s, const Point& p, const std::string& cap)
        : callsign(c), ship(std::move(s)), coords(p) {}
};

/**
 * @brief Контейнер кораблей в миссии (ShipTable)
 *
 */
typedef LookUpTable<info, std::string> ShipTable;

/**
 * @brief Интерфейс репозитория миссии
 *
 */
class MissionRepository {
    public:
        virtual ~MissionRepository() = default;
        
        /**
         * @brief Получить таблицу пиратских кораблей
         * @return Ссылка на ShipTable пиратов
         */
        [[nodiscard]] virtual ShipTable & get_pirates() = 0;
        
        /**
         * @brief Получить таблицу имперских кораблей
         * @return Ссылка на ShipTable имперцев
         */
        [[nodiscard]] virtual ShipTable & get_imperials() = 0;

        /**
         * @brief Добавить корабль в репозиторий
         * @param ship Умный указатель на корабль
         * @param coords Координаты корабля
         * @param is_pirate true для пиратов, false для имперцев
         */
        virtual bool add_ship(std::unique_ptr<Ship> ship, const Point& coords, bool is_pirate = false) = 0;                      
        
        /**
         * @brief Удалить корабль из репозитория
         * @param callsign Позывной корабля
         * @param is_pirate true для пиратов, false для имперцев
         */
        virtual bool remove_ship(const std::string & callsign, bool is_pirate = false) = 0;
        
        /**
         * @brief Найти корабль с максимальной координатой X
         * @return Позывной найденного корабля
         */
        virtual std::optional<std::string> find_max_x_point_ship() = 0;
        
        /**
         * @brief Найти корабль по координате
         * @param point Координата для поиска
         * @param is_pirate true для пиратов, false для имперцев
         * @return Позывной найденного корабля
         */
        virtual std::optional<std::string> find_by_point(const Point& point, bool is_pirate  = false) = 0;
        
        /**
         * @brief Найти минимальную скорость среди кораблей
         * @param is_pirate true для пиратов, false для имперцев
         * @return Минимальная скорость
         */
        virtual unsigned find_min_velocity(bool is_pirate = false) = 0;
        
        /**
         * @brief Сдвинуть корабли по оси X
         * @param shift Величина сдвига
         * @param is_pirate true для пиратов, false для имперцев
         */
        virtual void move_by_x(unsigned shift, bool is_pirate  = false) = 0;

        /**
         * @brief Получить параметры миссии
         * @return Ссылка на параметры миссии
         */
        [[nodiscard]] virtual Mission_params& get_mission_params() const = 0;

         /**
         * @brief Получение n ближайших кораблей
         * @return Вектора точек, на которых эти корабли располагаются
         */
        virtual std::map<unsigned, Point> find_n_nearest(const Point & point, size_t n, bool is_pirate = true) = 0;
  
        /**
         * @brief Получение кораблей, которые потенциально может атаковать
         * @return Вектора точек, на которых эти корабли располагаются
         */
        virtual std::vector<Point> find_opposite_ships_in_fire_range(const Point & p, unsigned range, bool is_pirate) = 0;
};

