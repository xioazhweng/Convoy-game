#include <iostream>
#include "loader/include/GameLoader.h"

int main() {
    try {
        GameLoader loader;
        auto& view = loader.get_game_view();
        (void)view.run();

        std::cout << "Game ended." << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
