#pragma once

#include "model/weapon/Weapon.h"
#include "model/place/Place.h"
#include <vector>
#include <memory>

constexpr float weapon_const = 0.05; //каждое оружие уменьшает скорость корабля на 5 %

/**
 * @brief Слот оружия (место + указатель на оружие)
 */
struct WeaponUnit {
    Place place;
    std::unique_ptr<Weapon> weapon;
    WeaponUnit(Place place_, std::unique_ptr<Weapon> weapon_)
        : place(place_), weapon(std::move(weapon_)) {};
    WeaponUnit(WeaponUnit&& other) noexcept
        : place(other.place), weapon(std::move(other.weapon)) {}
    
    WeaponUnit& operator=(WeaponUnit&& other) noexcept {
        if (this != &other) {
            place = other.place;
            weapon = std::move(other.weapon);
        }
        return *this;
    }
    WeaponUnit(const WeaponUnit&) = delete;
    WeaponUnit& operator=(const WeaponUnit&) = delete;
    [[nodiscard]] bool can_shoot() const { return weapon && weapon->can_shoot(); }
    
};

/**
 * @brief Интерфейс «военного» корабля (операции с оружием)
 */
class Military {
    public:
        /**
         * @brief Проверить возможность стрельбы
         * @param weapon_index Индекс оружия
         * @return true если можно стрелять, иначе false
         */
        virtual bool can_shoot(unsigned weapon_index) = 0;
        
        /**
         * @brief Получить актуальную максимальную скорость
         * @return Актуальная максимальная скорость
         */
        [[nodiscard]] virtual unsigned get_actual_max_velocity() noexcept = 0;
        
        /**
         * @brief Установить оружие на место
         * @param place Место установки
         * @param weapon Умный указатель на оружие
         * @return true если установка успешна, иначе false
         */
        virtual bool set_weapon(Place place, std::unique_ptr<Weapon> weapon) = 0;
        
        /**
         * @brief Заменить оружие на месте
         * @param place Место установки
         * @param weapon Умный указатель на оружие
         * @return true если замена успешна, иначе false
         */
        virtual bool replace_weapon(Place place, std::unique_ptr<Weapon> weapon) = 0;
        
        /**
         * @brief Снять оружие с места
         * @param place Место установки
         * @return Умный указатель на снятое оружие
         */
        virtual std::unique_ptr<Weapon> remove_weapon(Place place) = 0;
        
        /**
         * @brief Получить оружие по индексу
         * @param weapon_index Индекс оружия
         * @return Указатель на оружие или nullptr
         */
        virtual Weapon* get_weapon(unsigned weapon_index) = 0;


        /**
         * @brief Получить оружие по месту установки
         * @param p Место установки
         * @return Вектор указателей на оружие
         */
        virtual std::vector<Weapon*> get_weapon_by_place(Place p) const = 0;

        /**
         * @brief Получить количество оружия
         * @return Количество оружия
         */
        virtual size_t weapon_count() const noexcept = 0;
        
        /**
         * @brief Получить коэффициент beta
         * @return Коэффициент beta
         */
        virtual double get_beta() const = 0;
        
        /**
         * @brief Получить список оружия
         * @return Константная ссылка на список оружия
         */
        virtual const std::vector<WeaponUnit>& get_weapons() const = 0;
};


/**
 * @brief Базовая реализация Military
 *
 * @details
 * Хранит список WeaponUnit и предоставляет операции установки/снятия.
 */
class DefaultMilitary: public Military {
    protected:
        double beta;
        std::vector<WeaponUnit> weapons;

    public:
        /**
         * @brief Конструктор с коэффициентом
         * @param b_ Коэффициент beta
         */
        explicit DefaultMilitary(double b_) : beta(b_) {}
        
        /**
         * @brief Проверить возможность стрельбы
         * @param weapon_index Индекс оружия
         * @return true если можно стрелять, иначе false
         */
        bool can_shoot(unsigned weapon_index) override {
            if (weapon_index >= weapons.size()) {
                return false;
            }
            return weapons[weapon_index].can_shoot();
        }
        
        /**
         * @brief Установить оружие на место
         * @param place Место установки
         * @param weapon Умный указатель на оружие
         * @return true если установка успешна, иначе false
         */
        bool set_weapon(Place place, std::unique_ptr<Weapon> weapon) override {
            for (const auto& weapon_unit : weapons) {
                if (weapon_unit.place == place) {
                    return false;
                }
            }
            weapons.emplace_back(place, std::move(weapon));
            return true;
        }
        
        /**
         * @brief Заменить оружие на месте
         * @param place Место установки
         * @param weapon Умный указатель на оружие
         * @return true если замена успешна, иначе false
         */
        bool replace_weapon(Place place, std::unique_ptr<Weapon> weapon) override {
            for (auto& weapon_unit : weapons) {
                if (weapon_unit.place == place) {
                    weapon_unit.weapon = std::move(weapon);
                    return true;
                }
            }
            return false;
        }
        
        /**
         * @brief Снять оружие с места
         * @param place Место установки
         * @return Умный указатель на снятое оружие
         */
        std::unique_ptr<Weapon> remove_weapon(Place place) override {
            for (auto it = weapons.begin(); it != weapons.end(); ++it) {
                if (it->place == place) {
                    auto removed_weapon = std::move(it->weapon);
                    weapons.erase(it);
                    return removed_weapon;
                }
            }
            return nullptr;
        }
        
        /**
         * @brief Получить актуальную максимальную скорость
         * @return Актуальная максимальная скорость
         */
        [[nodiscard]] unsigned get_actual_max_velocity() noexcept override {
            return static_cast<unsigned>(100 * beta);
        }
        
        /**
         * @brief Получить оружие по индексу
         * @param weapon_index Индекс оружия
         * @return Указатель на оружие или nullptr
         */
        Weapon* get_weapon(unsigned weapon_index) override {
            if (weapon_index >= weapons.size()) {
                return nullptr;
            }
            return weapons[weapon_index].weapon.get();
        }
        
        /**
         * @brief Получить количество оружия
         * @return Количество оружия
         */
        size_t weapon_count() const noexcept override { return weapons.size(); }
        
        /**
         * @brief Получить коэффициент beta
         * @return Коэффициент beta
         */
        double get_beta() const override { return beta; }
        
        /**
         * @brief Получить список оружия
         * @return Константная ссылка на список оружия
         */
        const std::vector<WeaponUnit>& get_weapons() const override { return weapons; }
        
        /**
         * @brief Получить оружие по месту установки
         * @param p Место установки
         * @return Вектор указателей на оружие
         */
        std::vector<Weapon*> get_weapon_by_place(Place p) const override {
            std::vector<Weapon*> result;
            for (const auto& w : weapons) {
                if (w.place == p) {
                    result.push_back(w.weapon.get());
                }
            }
            return std::move(result);
        }
};
