#pragma once

#include "model/point/Point.h"
#include <vector>

/**
 * @brief Интерфейс оружия
 *
 * @details
 * Описывает общие операции: стоимость, урон, область поражения,
 * стрельба/перезарядка и получение id.
 */
class Weapon {
    public:
        virtual ~Weapon() = default;
        
        /**
         * @brief Получить стоимость оружия
         * @return Стоимость оружия
         */
        [[nodiscard]] virtual unsigned get_cost() const = 0;
        
        /**
         * @brief Проверить возможность стрельбы
         * @return true если можно стрелять, иначе false
         */
        [[nodiscard]] virtual bool can_shoot() = 0;
        
        /**
         * @brief Получить область поражения
         * @param center Центральная точка
         * @return Ссылка на вектор точек области поражения
         */
        virtual std::vector<Point>& get_shoot_area(Point center) const = 0;
        
        /**
         * @brief Перезарядить оружие
         * @return true если перезарядка успешна, иначе false
         */
        [[nodiscard]] virtual bool reload() = 0;
        
        /**
         * @brief Произвести выстрел
         */
        virtual void shoot() = 0;
        
        /**
         * @brief Получить урон оружия
         * @return Урон оружия
         */
        [[nodiscard]] virtual unsigned get_damage() const = 0;
        
        /**
         * @brief Получить ID оружия
         * @return ID оружия
         */
        [[nodiscard]] virtual unsigned get_id() const = 0;

          /**
         * @brief Установить текущий боезапас
         * @param ammunition Количество боеприпасов
         */
        virtual void set_current_ammunition(unsigned ammunition) = 0;

        /**
         * @brief Установить скорострельность
         */
        virtual void set_fire_rate(unsigned v) = 0;

        /**
         * @brief Установить дальность стрельбы
         */
        virtual void set_fire_range(unsigned v) = 0;

        /**
         * @brief Установить максимальный боезапас
         */
        virtual void set_max_ammunition(unsigned v) = 0;

        /**
         * @brief Установить стоимость
         */
        virtual void set_cost(unsigned v) = 0;

        /**
         * @brief Установить урон
         */
        virtual void set_damage(unsigned v) = 0;
        
        /**
         * @brief Получить скорострельность
         * @return Скорострельность
         */
        virtual unsigned get_fire_rate() const = 0;
        
        /**
         * @brief Получить дальность стрельбы
         * @return Дальность стрельбы
         */
        virtual unsigned get_fire_range() const = 0;
        
        /**
         * @brief Получить максимальный боезапас
         * @return Максимальный боезапас
         */
        virtual unsigned get_max_ammunition() const = 0;
        
        /**
         * @brief Получить текущий боезапас
         * @return Текущий боезапас
         */
        virtual unsigned get_current_ammunition() const  = 0;
        


      
};


/**
 * @brief Базовая реализация оружия (общая логика боезапаса)
 *
 * @note Класс остаётся абстрактным из-за get_id(): конкретные типы должны его определить.
 */
class DefaultWeapon: public Weapon {
    protected:
        unsigned fire_rate;
        unsigned fire_range;
        unsigned max_ammunition;
        unsigned current_ammunition;
        unsigned cost;
        unsigned damage;
    public:
        DefaultWeapon() :
        fire_rate(100), fire_range(100), max_ammunition(100),
        current_ammunition(100), cost(100), damage(50) {};
        
        /**
         * @brief Конструктор с параметрами
         * @param fire_rate Скорострельность
         * @param fire_range Дальность стрельбы
         * @param max_ammunition Максимальный боезапас
         * @param current_ammunition Текущий боезапас
         * @param cost_ Стоимость
         * @param damage_ Урон
         */
        explicit DefaultWeapon(unsigned fire_rate, unsigned fire_range, unsigned max_ammunition,
                               unsigned current_ammunition, unsigned cost_, unsigned damage_)
            : fire_rate(fire_rate), fire_range(fire_range), max_ammunition(max_ammunition),
             current_ammunition(current_ammunition), cost(cost_), damage(damage_) {};
        
        DefaultWeapon(const DefaultWeapon& other)
        : fire_rate(other.fire_rate), fire_range(other.fire_range),
          max_ammunition(other.max_ammunition), current_ammunition(other.current_ammunition),
          cost(other.cost), damage(other.damage) {};
        
        DefaultWeapon(DefaultWeapon&& other) noexcept
        : fire_rate(other.fire_rate), fire_range(other.fire_range),
          max_ammunition(other.max_ammunition), current_ammunition(other.current_ammunition),
          cost(other.cost), damage(other.damage) {
            other.current_ammunition = 0;
        };
        
        inline DefaultWeapon& operator=(const DefaultWeapon& other);
        inline DefaultWeapon& operator=(DefaultWeapon&& other) noexcept;
        virtual ~DefaultWeapon() = default;
        
        /**
         * @brief Получить стоимость оружия
         * @return Стоимость оружия
         */
        [[nodiscard]] unsigned get_cost()const  override { return cost;};
        
        /**
         * @brief Проверить возможность стрельбы
         * @return true если можно стрелять, иначе false
         */
        [[nodiscard]] bool can_shoot() override { return current_ammunition > 0;};
        
    
        /**
         * @brief Получить область поражения
         * @param center Центральная точка
         * @return Ссылка на вектор точек области поражения
         */
        virtual std::vector<Point>& get_shoot_area(Point center) const override;
        
        /**
         * @brief Перезарядить оружие
         * @return true если перезарядка успешна, иначе false
         */
        [[nodiscard]] bool reload() override;
        
        /**
         * @brief Получить урон оружия
         * @return Урон оружия
         */
        [[nodiscard]] unsigned get_damage() const override { return damage; };
        
        /**
         * @brief Установить текущий боезапас
         * @param ammunition Количество боеприпасов
         */
        void set_current_ammunition(unsigned ammunition) override  { current_ammunition = ammunition; };

        /**
         * @brief Установить скорострельность
         */
        void set_fire_rate(unsigned v) override { fire_rate = v; }

        /**
         * @brief Установить дальность стрельбы
         */
        void set_fire_range(unsigned v) override { fire_range = v; }

        /**
         * @brief Установить максимальный боезапас
         */
        void set_max_ammunition(unsigned v) override { max_ammunition = v; }

        /**
         * @brief Установить стоимость
         */
        void set_cost(unsigned v) override { cost = v; }

        /**
         * @brief Установить урон
         */
        void set_damage(unsigned v) override { damage = v; }
        
        /**
         * @brief Получить скорострельность
         * @return Скорострельность
         */
        unsigned get_fire_rate() const override { return fire_rate; }
        
        /**
         * @brief Получить дальность стрельбы
         * @return Дальность стрельбы
         */
        unsigned get_fire_range() const override { return fire_range; }
        
        /**
         * @brief Получить максимальный боезапас
         * @return Максимальный боезапас
         */
        unsigned get_max_ammunition() const override { return max_ammunition; }
        
        /**
         * @brief Получить текущий боезапас
         * @return Текущий боезапас
         */
        unsigned get_current_ammunition() const override { return current_ammunition; }
        

        /**
         * @brief Выстрелить (убавить current_amm)
         */
        void shoot() override {
            if (fire_rate > current_ammunition) {
                current_ammunition = 0;
                return;
            }
            current_ammunition = current_ammunition - fire_rate;
            
        };
};

inline DefaultWeapon& DefaultWeapon::operator=(const DefaultWeapon& other) {
    if (this != &other) {
        fire_rate = other.fire_rate;
        fire_range = other.fire_range;
        max_ammunition = other.max_ammunition;
        current_ammunition = other.current_ammunition;
        cost = other.cost;
        damage = other.damage;
    }
    return *this;
}


inline DefaultWeapon& DefaultWeapon::operator=(DefaultWeapon&& other) noexcept {
    if (this != &other) {
        fire_rate = other.fire_rate;
        fire_range = other.fire_range;
        max_ammunition = other.max_ammunition;
        current_ammunition = other.current_ammunition;
        cost = other.cost;
        damage = other.damage;
        other.current_ammunition = 0;
    }
    return *this;
}

inline std::vector<Point>& DefaultWeapon::get_shoot_area(Point center) const {
    static std::vector<Point> area;
    area.clear();
    area.push_back(center);
    return area;
}

inline bool DefaultWeapon::reload() {
    if (current_ammunition < max_ammunition) {
        current_ammunition = max_ammunition;
        return true;
    }
    return false;
}


