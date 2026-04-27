#pragma once

#include "dto/GameStateDto.h"
#include <string>

class GameView {
    public:
        virtual ~GameView() = default;

        virtual void run() = 0;
        virtual const std::string & get_config_path() = 0;
        virtual void update_game_state(const GameStateDto& gameState) = 0;
        virtual bool is_game_loaded() const = 0;
};

