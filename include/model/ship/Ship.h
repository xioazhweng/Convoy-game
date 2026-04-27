#pragma once

#include <memory>
#include <mutex>
#include <string>

/**
 * @brief Базовый интерфейс корабля
 */
class Ship {
    public:
        virtual ~Ship() = default;
        
        /**
         * @brief Проверить, уничтожен ли корабль
         * @return true если корабль уничтожен, иначе false
         */
        virtual bool is_destroyed() = 0;

        
        /**
         * @brief Клонировать корабль
         * @return Умный указатель на клон корабля
         */
        virtual std::unique_ptr<Ship> clone() = 0;
        
        /**
         * @brief Нанести урон кораблю
         * @param damage Количество урона
         */
        virtual void take_damage(unsigned damage) = 0;

        /**
         * @brief Получить текущую скорость
         * @return Текущая скорость
         */
        virtual unsigned get_velocity() const = 0;

        
        /**
         * @brief Получить позывной корабля
         * @return Позывной корабля
         */
        virtual const std::string & get_callsign() const = 0;
        
        /**
         * @brief Получить максимальную скорость
         * @return Максимальная скорость
         */
        virtual unsigned get_max_velocity() const = 0;
        
        /**
         * @brief Получить текущую скорость
         * @return Текущая скорость
         */
        virtual unsigned get_current_velocity() const = 0;
        
        /**
         * @brief Получить максимальное HP
         * @return Максимальное HP
         */
        virtual unsigned get_max_hp() const = 0;
        
        /**
         * @brief Получить текущее HP
         * @return Текущее HP
         */
        virtual unsigned get_current_hp() const = 0;
        
        /**
         * @brief Получить тип корабля
         * @return Строка с типом корабля
         */
        virtual std::string get_type() const = 0;

        /**
         * @brief Получить стоимость корабля
         * @return Стоимость корабля
         */
        virtual unsigned get_cost() const = 0; 
       
        /**
         * @brief Получить имя капитана
         * @return Имя капитана
         */
        virtual const std::string & get_captain() const = 0;

         /**
         * @brief Установить макс. количество HP
         * @param hp - максимальное количесво HP
         */
        virtual void set_max_hp(unsigned hp) = 0;

        /**
         * @brief Установить текущее количество HP
         * @param hp - текущее количесво HP
         */
        virtual void set_current_hp(unsigned hp) = 0;

        /**
         * @brief Установить цену
         * @param cost_ - цена
         */
        virtual void set_cost(unsigned cost_) = 0;
};

/**
 * @brief Базовая реализация Ship
 *
 */
class DefaultShip: public Ship {
    protected:
        // mutable позволяет синхронизировать доступ в const-методах 
        mutable std::mutex m;
        std::string callsign;
        unsigned max_velocity;
        unsigned current_velocity;
        unsigned max_HP;
        unsigned current_HP;
        unsigned cost;
        std::string captain;
   
    public:
        /**
         * @brief Конструктор с параметрами
         * @param name_ Позывной корабля
         * @param max_velocity Максимальная скорость
         * @param cur_velocity Текущая скорость
         * @param max_HP_ Максимальное HP
         * @param cost_ Стоимость
         * @param captain_ Имя капитана
         */
        explicit DefaultShip(const std::string& name_,
        unsigned max_velocity, unsigned cur_velocity,
        unsigned max_HP_, unsigned cost_, const std::string& captain_)
        : callsign(name_), max_velocity(max_velocity),
        current_velocity(cur_velocity), max_HP(max_HP_),
        current_HP(max_HP_), cost(cost_), captain(captain_) {}

        /**
         * @brief Явный копирующий конструктор.
         *
         * std::mutex не копируется, поэтому генерируемый компилятором
         * конструктор копирования был удалён. Здесь мы копируем только
         * состояние корабля, а мьютекс создаётся заново.
         */
        DefaultShip(const DefaultShip& other)
            : callsign(other.callsign)
            , max_velocity(other.max_velocity)
            , current_velocity(other.current_velocity)
            , max_HP(other.max_HP)
            , current_HP(other.current_HP)
            , cost(other.cost)
            , captain(other.captain) {}

        /**
         * @brief Явный оператор копирующего присваивания по тем же причинам.
         */
        DefaultShip& operator=(const DefaultShip& other) {
            if (this == &other) {
                return *this;
            }
            
            std::scoped_lock lock(m, other.m); // Блокируем оба мьютекса (для безопасного чтения/записи состояния).
            callsign = other.callsign;
            max_velocity = other.max_velocity;
            current_velocity = other.current_velocity;
            max_HP = other.max_HP;
            current_HP = other.current_HP;
            cost = other.cost;
            captain = other.captain;
            return *this;
        }
        
        /**
         * @brief Проверить, уничтожен ли корабль
         * @return true если корабль уничтожен, иначе false
         */
        bool is_destroyed() override {
            return current_HP == 0;
        }
        
        /**
         * @brief Клонировать корабль
         * @return Умный указатель на клон корабля
         */
        std::unique_ptr<Ship> clone() override {
            return std::make_unique<DefaultShip>(*this);
        }
        
        /**
         * @brief Нанести урон кораблю
         * @param damage Количество урона
         */
        void take_damage(unsigned damage) override {
            std::lock_guard<std::mutex> lock(m);
            if (damage > current_HP) {
                current_HP = 0;
            } else {
                current_HP -= damage;
            }
        }
        
        /**
         * @brief Получить тип корабля
         * @return Строка с типом корабля
         */
        std::string get_type() const override {
            return "DefaultShip";
        }
        
        /**
         * @brief Получить текущую скорость
         * @return Текущая скорость
         */
        unsigned get_velocity() const override {
            return current_velocity;
        };

        
        /**
         * @brief Получить позывной корабля
         * @return Позывной корабля
         */
        const std::string & get_callsign() const override { return callsign; }
        
        /**
         * @brief Получить максимальную скорость
         * @return Максимальная скорость
         */
        unsigned get_max_velocity() const override { return max_velocity; }
        
        /**
         * @brief Получить текущую скорость
         * @return Текущая скорость
         */
        unsigned get_current_velocity() const override { return current_velocity; }
        
        /**
         * @brief Получить максимальное HP
         * @return Максимальное HP
         */
        unsigned get_max_hp() const override { return max_HP; }
        
        /**
         * @brief Получить текущее HP
         * @return Текущее HP
         */
        unsigned get_current_hp() const override { return current_HP; }
        
        /**
         * @brief Получить стоимость корабля
         * @return Стоимость корабля
         */
        unsigned get_cost() const override { return cost; }
        
        /**
         * @brief Получить имя капитана
         * @return Имя капитана
         */
        const std::string & get_captain() const override { return captain; }


         /**
         * @brief Установить макс. количество HP
         * @param hp - максимальное количесво HP
         */
        void set_max_hp(unsigned hp) override {
            max_HP = hp;
        };

        /**
         * @brief Установить текущее количество HP
         * @param hp - текущее количесво HP
         */
        void set_current_hp(unsigned hp) override {
            if (hp > max_HP) {
                hp = max_HP;
            }
            current_HP = hp;
        };

        /**
         * @brief Установить цену
         * @param cost_ - цена
         */
        void set_cost(unsigned cost_) override {
            cost = cost_;
        };
};
