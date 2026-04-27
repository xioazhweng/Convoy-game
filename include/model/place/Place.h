#pragma once
#include <algorithm>
#include <string>

/**
 * @brief Места установки оружия на корабле
 */
enum Place {
    STERN,
    BOW,
    STARBOARD,
    PORTSIDE,
    NONE,
};

/**
 * @brief Перевод Place -> строка (для конфигов/DTO)
 * @param p Место установки оружия
 * @return Строка, соответствующая месту
 */
inline std::string get_place_name(Place p) {
    switch(p) {
        case STERN:
            return "stern";
        case BOW:
            return "bow";
        case STARBOARD:
            return "starboard";
        case PORTSIDE:
            return "portside";
        default: 
            return "none";
    }
    return "none";
}

/**
 * @brief Перевод строки -> Place
 * @param p Строка с названием места
 * @return Место установки оружия
 */
inline Place get_place_id(const std::string& p) {
    std::string lower_p = p;  
    std::transform(lower_p.begin(), lower_p.end(), lower_p.begin(), ::tolower);

    if (lower_p == "stern") {
        return STERN;
    }
    if (lower_p == "bow") {
        return BOW;
    }
    if (lower_p == "starboard") {
        return STARBOARD;
    }
    if (lower_p == "portside") {
        return PORTSIDE;
    }
    return NONE;  
}
