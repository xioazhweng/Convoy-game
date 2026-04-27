#pragma once

/**
 * @brief Точка на карте (x,y)
 *
 */
class Point {
    private:
        int x;
        int y;
    public:
        Point() : x(0), y(0) {}; 
        
        /**
         * @brief Конструктор с параметрами
         * @param xx Координата X
         * @param yy Координата Y
         */
        Point(int xx, int yy): x(xx), y(yy) {}; 

        /**
         * @brief Получить координату X
         * @return Координата X
         */
        int get_x() const {return x;};
        
        /**
         * @brief Получить координату Y
         * @return Координата Y
         */
        int get_y() const {return y;};
        
        /**
         * @brief Добавить к X
         * @param xx Значение для добавления к X
         */
        void add_x(int xx) { x += xx;};
        
        /**
         * @brief Установить точку
         * @param xx Новая координата X
         * @param yy Новая координата Y
         */
        void set_point(int xx, int yy) { x = xx; y = yy; };


        /**
         * @brief Оператор равенства
         * @param point Точка для сравнения
         * @return true если точки равны, иначе false
         */
        bool operator == (const Point & point) const {
            return x == point.get_x() && y == point.get_y();
        };
        
        /**
         * @brief Оператор неравенства
         * @param point Точка для сравнения
         * @return true если точки не равны, иначе false
         */
        bool operator != (const Point & point) const {
            return !(x == point.get_x() && y == point.get_y());
        };

        /**
         * @brief Расчет квадрата расстояний
         * @param point Точка до которой считаем квадрат расстояний
         * @return квадрат расстояний
         */
        unsigned get_distance2(const Point & point) const {
            const auto dx = x - point.x;
            const auto dy = y - point.y;
            return static_cast<unsigned>(dx * dx + dy * dy);
        }
        
};

