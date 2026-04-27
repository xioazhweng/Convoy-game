#pragma once

#include "model/repository/MissionRepository.h"
#include "model/ship/Ship.h"
#include "model/point/Point.h"

/**
 * @brief Реализация MissionRepository на основе LookUpTable
 */
class MissionRepositoryImpl final: public MissionRepository {
    private:    
        ShipTable imperials;
        ShipTable pirates;
        mutable Mission_params params{};
        
    public:
        MissionRepositoryImpl() = default;
        
        /**
         * @brief Конструктор с параметрами
         * @param mp Параметры миссии
         */
        MissionRepositoryImpl(Mission_params & mp);
        
        /**
         * @brief Получить таблицу пиратских кораблей
         * @return Ссылка на ShipTable пиратов
         */
        ShipTable & get_pirates() override;
        
        /**
         * @brief Получить таблицу имперских кораблей
         * @return Ссылка на ShipTable имперцев
         */
        ShipTable & get_imperials() override;
        
        /**
         * @brief Получить параметры миссии
         * @return Ссылка на параметры миссии
         */
        Mission_params& get_mission_params() const override;
        
        /**
         * @brief Добавить корабль в репозиторий
         * @param ship Умный указатель на корабль
         * @param coords Координаты корабля
         * @param is_pirate true для пиратов, false для имперцев
         */
        bool add_ship(std::unique_ptr<Ship> ship, const Point& coords, bool is_pirate = false) override;
                      
        /**
         * @brief Удалить корабль из репозитория
         * @param callsign Позывной корабля
         * @param is_pirate true для пиратов, false для имперцев
         */
        bool remove_ship(const std::string & callsign, bool is_pirate) override;
        
        /**
         * @brief Найти корабль с максимальной координатой X
         * @return Позывной найденного корабля
         */
        std::optional<std::string> find_max_x_point_ship() override;
        
        /**
         * @brief Найти корабль по координате
         * @param point Координата для поиска
         * @param is_pirate true для пиратов, false для имперцев
         * @return Позывной найденного корабля
         */
        std::optional<std::string> find_by_point(const Point& point, bool is_pirate) override;
        
        /**
         * @brief Найти минимальную скорость среди кораблей
         * @param is_pirate true для пиратов, false для имперцев
         * @return Минимальная скорость
         */
        unsigned find_min_velocity(bool is_pirate) override;
        
        /**
         * @brief Сдвинуть корабли по оси X
         * @param shift Величина сдвига
         * @param is_pirate true для пиратов, false для имперцев
         */
        void move_by_x(unsigned shift, bool is_pirate) override;

        
         /**
         * @brief Получение n ближайших кораблей
         * @return Вектора точек, на которых эти корабли располагаются
         */
        std::map<unsigned, Point> find_n_nearest(const Point & point, size_t n, bool is_pirate) override;

        /**
         * @brief Получение кораблей, которые потенциально может атаковать
         * @return Вектора точек, на которых эти корабли располагаются
         */
        std::vector<Point> find_opposite_ships_in_fire_range(const Point & p, unsigned range, bool is_pirate) override;
        

};

