#include "../include/YalmStateService.h"
#include <stdexcept>
#include <filesystem>
#include <fstream>
#include "dto/GameStateDto.h"
#include "yaml-cpp/yaml.h"

YalmStateService::YalmStateService(const std::string& config_path) 
    : config_path_(config_path), game_state_(std::make_unique<GameStateDto>()) {
}

void YalmStateService::set_game_state(GameStateDto state) {
    auto new_state = std::make_unique<GameStateDto>(std::move(state));
    game_state_.swap(new_state);
}

void YalmStateService::load_repo(const std::string& path) {
    const std::filesystem::path p(path);
    const std::string file_path = (p.extension() == ".yaml") ? p.string() : (p / "repository_data.yaml").string();
    game_state_->ship_units.clear();

    load_from_yaml(file_path, [this](const YAML::Node& node) {
        auto repository = node["repository"];
        for (const auto& unit : repository) {
            ShipUnitDTO unit_dto;
            unit_dto.callsign = unit["callsign"].as<std::string>();
            unit_dto.x = unit["x"].as<int>();
            unit_dto.y = unit["y"].as<int>();
            unit_dto.side = unit["side"].as<std::string>();
            game_state_->ship_units.push_back(unit_dto);
        }
    });
}

void YalmStateService::load_weapons_params(const std::string& path) {
    load_from_yaml(path + "/weapons_params.yaml", [this](const YAML::Node& node) {
        auto weapons_params = node["weapons"];
        for (const auto& weapon_type : weapons_params) {
            WeaponParamsDto params;
            params.fire_rate = weapon_type.second["fire_rate"].as<unsigned>();
            params.fire_range = weapon_type.second["fire_range"].as<unsigned>();
            params.max_ammunition = weapon_type.second["max_ammunition"].as<unsigned>();
            params.cost = weapon_type.second["cost"].as<unsigned>();
            params.damage = weapon_type.second["damage"].as<unsigned>();
            game_state_->weapons_params[weapon_type.first.as<std::string>()] = params;
        }
    });
}

void YalmStateService::load_ships_params(const std::string& path) {
    load_from_yaml(path + "/ships_params.yaml", [this](const YAML::Node& node) {
        auto ships_params = node["ships"];
        for (const auto& ship_type : ships_params) {
            ShipParamsDto params;
            params.max_velocity = ship_type.second["max_velocity"].as<unsigned>();
            params.max_HP = ship_type.second["max_HP"].as<unsigned>();
            params.cost = ship_type.second["cost"].as<unsigned>();

            game_state_->ships_params[ship_type.first.as<std::string>()] = params;
        }
    });
}

void YalmStateService::load_mission_params(const std::string& path) {
    load_from_yaml(path + "/mission_params.yaml", [this](const YAML::Node& node) {
        auto mission = node["mission"];
        game_state_->mission.budget = mission["budget"].as<unsigned>();
        game_state_->mission.spent_budget = mission["spent_budget"].as<unsigned>();
        game_state_->mission.total_cargo_weight = mission["total_cargo_weight"].as<unsigned>();
        game_state_->mission.goal_weight = mission["goal_weight"].as<unsigned>();
        game_state_->mission.lost_weight = mission["lost_weight"].as<unsigned>();
        game_state_->mission.delivered_weight = mission["delivered_weight"].as<unsigned>();
        
        auto bases = mission["bases"];
        for (const auto& base : bases) {
            MissionParamsDto::Base base_data;
            base_data.x = base.second["x"].as<int>();
            base_data.y = base.second["y"].as<int>();
            game_state_->mission.bases[base.first.as<std::string>()] = base_data;
        }
        
        auto pirates_bases = mission["pirates_bases"];
        for (const auto& base : pirates_bases) {
            MissionParamsDto::Base base_data;
            base_data.x = base["x"].as<int>();
            base_data.y = base["y"].as<int>();
            game_state_->mission.pirates_bases[std::to_string(game_state_->mission.pirates_bases.size())] = base_data;
        }
        
        auto map = mission["map"];
        game_state_->mission.map.dx = map["dx"].as<int>();
        game_state_->mission.map.dy = map["dy"].as<int>();
        game_state_->mission.map.center.x = map["center"]["x"].as<int>();
        game_state_->mission.map.center.y = map["center"]["y"].as<int>();
    });
}

void YalmStateService::load_ships(const std::string& path) {
    load_from_yaml(path + "/ships.yaml", [this](const YAML::Node& node) {
        auto ships = node["ships"];
        for (const auto& ship : ships) {
            ShipDto ship_dto;
            ship_dto.type = ship["type"].as<std::string>();
            ship_dto.callsign = ship["callsign"].as<std::string>();
            ship_dto.captain = ship["captain"].as<std::string>();
            if (ship["current_velocity"]) {
                ship_dto.current_velocity = ship["current_velocity"].as<unsigned>();
            } else if (ship["current_velosity"]) {
                ship_dto.current_velocity = ship["current_velosity"].as<unsigned>();
            } else {
                throw std::runtime_error("Ship entry missing current_velocity/current_velosity for callsign: " + ship_dto.callsign);
            }
            ship_dto.current_HP = ship["current_HP"].as<unsigned>();
            
            auto params = ship["params"];
            for (const auto& param : params) {
                if (param.second.IsScalar()) {
                    ship_dto.params[param.first.as<std::string>()] = param.second.Scalar();
                } else {
                    ship_dto.params[param.first.as<std::string>()] = YAML::Dump(param.second);
                }
            }
            
            game_state_->ships.push_back(ship_dto);
        }
    });
}

void YalmStateService::load_ship_units(const std::string& path) {
    const std::filesystem::path p(path);
    const std::string file_path = (p.extension() == ".yaml") ? p.string() : (p / "ship_units.yaml").string();

    game_state_->ship_units.clear();

    load_from_yaml(file_path, [this](const YAML::Node& node) {
        auto units = node["ship_units"];
        for (const auto& unit : units) {
            ShipUnitDTO unit_dto;
            unit_dto.callsign = unit["callsign"].as<std::string>();
            unit_dto.x = unit["x"].as<int>();
            unit_dto.y = unit["y"].as<int>();
            unit_dto.side = unit["side"].as<std::string>();
            game_state_->ship_units.push_back(unit_dto);
        }
    });
}

void YalmStateService::load_weapons(const std::string& path) {
    load_from_yaml(path + "/weapons.yaml", [this](const YAML::Node& node) {
        auto weapons = node["weapons"];
        for (const auto& weapon : weapons) {
            WeaponDto weapon_dto;
            weapon_dto.id = weapon["id"].as<unsigned>();
            weapon_dto.type = weapon["type"].as<std::string>();
            weapon_dto.current_ammunition = weapon["current_ammunition"].as<unsigned>();
            game_state_->weapons.push_back(weapon_dto);
        }
    });
}

void YalmStateService::load_from_yaml(const std::string& file_path, std::function<void(const YAML::Node&)> processor) {
    try {
        std::filesystem::path resolved = file_path;

        if (!std::filesystem::exists(resolved)) {
            if (resolved.is_relative()) {
                std::filesystem::path cur = std::filesystem::current_path();
                std::filesystem::path probe = cur;
                while (!probe.empty()) {
                    const std::filesystem::path candidate = probe / resolved;
                    if (std::filesystem::exists(candidate)) {
                        resolved = candidate;
                        break;
                    }
                    const auto parent = probe.parent_path();
                    if (parent == probe) {
                        break;
                    }
                    probe = parent;
                }
            }
        }

        if (!std::filesystem::exists(resolved)) {
            throw std::runtime_error("YAML file not found: " + resolved.string());
        }

        const auto docs = YAML::LoadAllFromFile(resolved.string());
        if (docs.empty()) {
            throw std::runtime_error("YAML file is empty: " + resolved.string());
        }

        YAML::Node root = docs.front();
        if (root.IsScalar() && docs.size() >= 2 && (docs[1].IsMap() || docs[1].IsSequence())) {
            YAML::Node wrapped(YAML::NodeType::Map);
            wrapped[root.Scalar()] = docs[1];
            root = wrapped;
        }

        processor(root);
        
    } catch (const YAML::Exception& e) {
        throw std::runtime_error("YAML parsing error in " + file_path + ": " + std::string(e.what()));
    } catch (const std::exception& e) {
        throw std::runtime_error("Error loading " + file_path + ": " + std::string(e.what()));
    }
}

void YalmStateService::save_to_yaml(const std::string& file_path, std::function<void(YAML::Emitter&)> emitter_func) const {
    try {
      
        std::filesystem::path path(file_path);
        if (path.has_parent_path()) {
            std::filesystem::create_directories(path.parent_path());
        }
        
        YAML::Emitter out;
        out << YAML::BeginDoc;
        out << YAML::BeginMap;
        emitter_func(out);
        out << YAML::EndMap;
        out << YAML::EndDoc;
        
        std::ofstream file(file_path);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file for writing: " + file_path);
        }
        
        file << out.c_str();
        file.close();
        
    } catch (const std::exception& e) {
        throw std::runtime_error("Error saving " + file_path + ": " + std::string(e.what()));
    }
}


const GameStateDto& YalmStateService::get_game_state() const {
    return *game_state_;
}

const std::vector<ShipDto>& YalmStateService::get_ships() const {
    return game_state_->ships;
}

const std::vector<ShipUnitDTO>& YalmStateService::get_ship_units() const {
    return game_state_->ship_units;
}

const std::vector<WeaponDto>& YalmStateService::get_weapons() const {
    return game_state_->weapons;
}

void YalmStateService::save_repo(const std::string& path) const {
    save_to_yaml(path + "/repository_data.yaml", [this](YAML::Emitter& out) {
        out << YAML::Key << "repository" << YAML::Value << YAML::BeginSeq;
        for (const auto& unit : game_state_->ship_units) {
            out << YAML::BeginMap;
            out << YAML::Key << "callsign" << YAML::Value << unit.callsign;
            out << YAML::Key << "x" << YAML::Value << unit.x;
            out << YAML::Key << "y" << YAML::Value << unit.y;
            out << YAML::Key << "side" << YAML::Value << unit.side;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
    });
}

void YalmStateService::save_weapons_params(const std::string& path) const {
    save_to_yaml(path + "/weapons_params.yaml", [this](YAML::Emitter& out) {
        out << YAML::Key << "weapons" << YAML::Value << YAML::BeginMap;
        for (const auto& [weapon_type, params] : game_state_->weapons_params) {
            out << YAML::Key << weapon_type << YAML::Value << YAML::BeginMap;
            out << YAML::Key << "fire_rate" << YAML::Value << params.fire_rate;
            out << YAML::Key << "fire_range" << YAML::Value << params.fire_range;
            out << YAML::Key << "max_ammunition" << YAML::Value << params.max_ammunition;
            out << YAML::Key << "cost" << YAML::Value << params.cost;
            out << YAML::Key << "damage" << YAML::Value << params.damage;
            out << YAML::EndMap;
        }
        out << YAML::EndMap;
    });
}

void YalmStateService::save_ships_params(const std::string& path) const {
    save_to_yaml(path + "/ships_params.yaml", [this](YAML::Emitter& out) {
        out << YAML::Key << "ships" << YAML::Value << YAML::BeginMap;
        for (const auto& [ship_type, params] : game_state_->ships_params) {
            out << YAML::Key << ship_type << YAML::Value << YAML::BeginMap;
            out << YAML::Key << "max_velocity" << YAML::Value << params.max_velocity;
            out << YAML::Key << "max_HP" << YAML::Value << params.max_HP;
            out << YAML::Key << "cost" << YAML::Value << params.cost;
            out << YAML::EndMap;
        }
        out << YAML::EndMap;
    });
}

void YalmStateService::save_mission_params(const std::string& path) const {
    save_to_yaml(path + "/mission_params.yaml", [this](YAML::Emitter& out) {
        out << YAML::Key << "mission" << YAML::Value << YAML::BeginMap;
        out << YAML::Key << "budget" << YAML::Value << game_state_->mission.budget;
        out << YAML::Key << "spent_budget" << YAML::Value << game_state_->mission.spent_budget;
        out << YAML::Key << "total_cargo_weight" << YAML::Value << game_state_->mission.total_cargo_weight;
        out << YAML::Key << "goal_weight" << YAML::Value << game_state_->mission.goal_weight;
        out << YAML::Key << "lost_weight" << YAML::Value << game_state_->mission.lost_weight;
        out << YAML::Key << "delivered_weight" << YAML::Value << game_state_->mission.delivered_weight;
        
        out << YAML::Key << "bases" << YAML::Value << YAML::BeginMap;
        for (const auto& [base_name, base_data] : game_state_->mission.bases) {
            out << YAML::Key << base_name << YAML::Value << YAML::BeginMap;
            out << YAML::Key << "x" << YAML::Value << base_data.x;
            out << YAML::Key << "y" << YAML::Value << base_data.y;
            out << YAML::EndMap;
        }
        out << YAML::EndMap;
        
        out << YAML::Key << "pirates_bases" << YAML::Value << YAML::BeginSeq;
        for (const auto& [base_name, base_data] : game_state_->mission.pirates_bases) {
            out << YAML::BeginMap;
            out << YAML::Key << "x" << YAML::Value << base_data.x;
            out << YAML::Key << "y" << YAML::Value << base_data.y;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
        
        out << YAML::Key << "map" << YAML::Value << YAML::BeginMap;
        out << YAML::Key << "dx" << YAML::Value << game_state_->mission.map.dx;
        out << YAML::Key << "dy" << YAML::Value << game_state_->mission.map.dy;
        out << YAML::Key << "center" << YAML::Value << YAML::BeginMap;
        out << YAML::Key << "x" << YAML::Value << game_state_->mission.map.center.x;
        out << YAML::Key << "y" << YAML::Value << game_state_->mission.map.center.y;
        out << YAML::EndMap;
        out << YAML::EndMap;
        
        out << YAML::EndMap;
    });
}

void YalmStateService::save_ships(const std::string& path) const {
    save_to_yaml(path + "/ships.yaml", [this](YAML::Emitter& out) {
        out << YAML::Key << "ships" << YAML::Value << YAML::BeginSeq;
        for (const auto& ship : game_state_->ships) {
            out << YAML::BeginMap;
            out << YAML::Key << "type" << YAML::Value << ship.type;
            out << YAML::Key << "callsign" << YAML::Value << ship.callsign;
            out << YAML::Key << "captain" << YAML::Value << ship.captain;
            out << YAML::Key << "current_velocity" << YAML::Value << ship.current_velocity;
            out << YAML::Key << "current_HP" << YAML::Value << ship.current_HP;

            out << YAML::Key << "params" << YAML::Value << YAML::BeginMap;
            for (const auto& [key, value] : ship.params) {
                out << YAML::Key << key << YAML::Value << value;
            }
            out << YAML::EndMap;
            
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
    });
}

void YalmStateService::save_ship_units(const std::string& path) const {
    const std::filesystem::path p(path);
    const std::string file_path = (p.extension() == ".yaml") ? p.string() : (p / "ship_units.yaml").string();

    save_to_yaml(file_path, [this](YAML::Emitter& out) {
        out << YAML::Key << "ship_units" << YAML::Value << YAML::BeginSeq;
        for (const auto& unit : game_state_->ship_units) {
            out << YAML::BeginMap;
            out << YAML::Key << "callsign" << YAML::Value << unit.callsign;
            out << YAML::Key << "x" << YAML::Value << unit.x;
            out << YAML::Key << "y" << YAML::Value << unit.y;
            out << YAML::Key << "side" << YAML::Value << unit.side;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
    });
}

void YalmStateService::save_weapons(const std::string& path) const {
    save_to_yaml(path + "/weapons.yaml", [this](YAML::Emitter& out) {
        out << YAML::Key << "weapons" << YAML::Value << YAML::BeginSeq;
        for (const auto& weapon : game_state_->weapons) {
            out << YAML::BeginMap;
            out << YAML::Key << "id" << YAML::Value << weapon.id;
            out << YAML::Key << "type" << YAML::Value << weapon.type;
            out << YAML::Key << "current_ammunition" << YAML::Value << weapon.current_ammunition;
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
    });
}
