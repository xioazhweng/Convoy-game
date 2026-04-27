#pragma once
#include <string>
#include <memory>
#include <vector>
#include <functional>

#include "service/StateService.h"
#include "dto/GameStateDto.h"

namespace YAML {
    class Node;
    class Emitter;
}

struct ShipDto;
struct ShipUnitDTO;
struct WeaponDto;
struct GameStateDto;


/**
 * @brief YAML реализация StateService
 */
class YalmStateService: public StateService {
public:
    /**
     * @brief Конструктор с параметрами
     * @param config_path Путь к конфигурации
     */
    explicit YalmStateService(const std::string& config_path);
    virtual ~YalmStateService() = default;

    /**
     * @brief Установить состояние игры
     * @param state Состояние игры
     */
    void set_game_state(GameStateDto state) override;
    
    /**
     * @brief Загрузить репозиторий
     * @param path Путь к файлу
     */
    void load_repo(const std::string& path) override;
    
    /**
     * @brief Загрузить параметры оружия
     * @param path Путь к файлу
     */
    void load_weapons_params(const std::string& path) override;
    
    /**
     * @brief Загрузить параметры кораблей
     * @param path Путь к файлу
     */
    void load_ships_params(const std::string& path) override;
    
    /**
     * @brief Загрузить параметры миссии
     * @param path Путь к файлу
     */
    void load_mission_params(const std::string& path) override;

    /**
     * @brief Загрузить корабли
     * @param path Путь к файлу
     */
    void load_ships(const std::string& path) override;
    
    /**
     * @brief Загрузить юниты кораблей
     * @param path Путь к файлу
     */
    void load_ship_units(const std::string& path) override;
    
    /**
     * @brief Загрузить оружие
     * @param path Путь к файлу
     */
    void load_weapons(const std::string& path) override;
    
    /**
     * @brief Сохранить репозиторий
     * @param path Путь к файлу
     */
    void save_repo(const std::string& path) const  override;
    
    /**
     * @brief Сохранить параметры оружия
     * @param path Путь к файлу
     */
    void save_weapons_params(const std::string& path) const  override;
    
    /**
     * @brief Сохранить параметры кораблей
     * @param path Путь к файлу
     */
    void save_ships_params(const std::string& path) const  override;
    
    /**
     * @brief Сохранить параметры миссии
     * @param path Путь к файлу
     */
    void save_mission_params(const std::string& path) const override;
    
    /**
     * @brief Сохранить корабли
     * @param path Путь к файлу
     */
    void save_ships(const std::string& path) const override;
    
    /**
     * @brief Сохранить юниты кораблей
     * @param path Путь к файлу
     */
    void save_ship_units(const std::string& path) const override;
    
    /**
     * @brief Сохранить оружие
     * @param path Путь к файлу
     */
    void save_weapons(const std::string& path) const override;
    
    /**
     * @brief Получить состояние игры
     * @return Константная ссылка на GameStateDto
     */
    [[nodiscard]] const GameStateDto& get_game_state() const override;
    
    /**
     * @brief Получить список кораблей
     * @return Константная ссылка на вектор ShipDto
     */
    [[nodiscard]] const std::vector<ShipDto>& get_ships() const override;
    
    /**
     * @brief Получить список юнитов кораблей
     * @return Константная ссылка на вектор ShipUnitDTO
     */
    [[nodiscard]] const std::vector<ShipUnitDTO>& get_ship_units() const override;
    
    /**
     * @brief Получить список оружия
     * @return Константная ссылка на вектор WeaponDto
     */
    [[nodiscard]] const std::vector<WeaponDto>& get_weapons() const override;

private:
    void load_from_yaml(const std::string& file_path, std::function<void(const YAML::Node&)> processor);
    void save_to_yaml(const std::string& file_path, std::function<void(YAML::Emitter&)> emitter_func) const;
    
    std::string config_path_;
    std::unique_ptr<GameStateDto> game_state_;
};
