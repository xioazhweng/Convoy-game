#pragma once

/**
 * @brief Интерфейс грузового корабля
 */
class Transport {
    public:
        virtual ~Transport() = default;
        
        /**
         * @brief Проверить возможность загрузки груза
         * @param weight Вес груза
         * @return true если можно загрузить, иначе false
         */
        virtual bool can_load_cargo(unsigned weight) = 0;
        
        /**
         * @brief Получить актуальную максимальную скорость
         * @return Актуальная максимальная скорость
         */
        virtual unsigned get_actual_max_velocity() = 0;
        
        /**
         * @brief Добавить вес груза
         * @param weight Вес груза для добавления
         */
        virtual void add_cargo_weight(unsigned weight) = 0;



        /**
         * @brief Получить максимальный вес груза
         * @return Максимальный вес груза
         */
        virtual unsigned get_max_cargo_weight() const = 0;

        /**
         * @brief Установить текущий вес груза
         * @param weight Новый вес груза
         */
        virtual void set_current_cargo_weight(unsigned weight) = 0;

        /**
        * @brief Установить текущий максимальный вес груза
        * @param weight Новый максимальный вес груза
        */
        virtual void set_max_cargo_weight(unsigned weight)=0;

        /**
         * @brief Получить текущий вес груза
         * @return Текущий вес груза
         */
        virtual unsigned get_current_cargo_weight() const = 0; 

        /**
         * @brief Получить коэффициент alpha
        * @return Коэффициент alpha
        */
        virtual double get_alpha() const = 0; 
};

/**
 * @brief Базовая реализация Transport
 *
 * @details
 * Скорость кораблей зависит от коэффициента alpha,
 */
class DefaultTransport: public Transport {
    protected:
        double alpha;
        unsigned max_cargo_weight;
        unsigned current_cargo_weight;
        
    public:
        /**
         * @brief Конструктор с параметрами
         * @param a Коэффициент alpha
         * @param mw Максимальный вес груза
         * @param cw Текущий вес груза
         */
        explicit DefaultTransport(double a, unsigned mw, unsigned cw)
        : alpha(a), max_cargo_weight(mw), current_cargo_weight(cw) {}

       
        /**
         * @brief Получить максимальный вес груза
         * @return Максимальный вес груза
         */
        unsigned get_max_cargo_weight() const override {
            return max_cargo_weight;
        }
        
        /**
         * @brief Получить текущий вес груза
         * @return Текущий вес груза
         */
        unsigned get_current_cargo_weight() const override {
            return current_cargo_weight;
        }
        
        /**
         * @brief Установить текущий вес груза
         * @param weight Новый вес груза
         */
        void set_current_cargo_weight(unsigned weight) override {
            current_cargo_weight = weight;
        }
        
        /**
        * @brief Установить текущий максимальный вес груза
        * @param weight Новый максимальный вес груза
        */
        void set_max_cargo_weight(unsigned weight) override {
            max_cargo_weight = weight;
        }
        
        /**
         * @brief Проверить возможность загрузки груза
         * @param weight Вес груза
         * @return true если можно загрузить, иначе false
         */
        bool can_load_cargo(unsigned weight) override {
            return (max_cargo_weight - current_cargo_weight) >= weight;
        }
        
        /**
         * @brief Получить актуальную максимальную скорость
         * @return Актуальная максимальная скорость
         */
        unsigned get_actual_max_velocity() override {
            return static_cast<unsigned>(100 * alpha);
        }
        
        /**
         * @brief Добавить вес груза
         * @param weight Вес груза для добавления
         */
        void add_cargo_weight(unsigned weight) override {
            if (can_load_cargo(weight)) {
                current_cargo_weight += weight;
            }
        };
    
    /**
     * @brief Получить коэффициент alpha
     * @return Коэффициент alpha
     */
    double get_alpha() const override { return alpha; }
};
