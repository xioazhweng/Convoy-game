#pragma once
#include <string>

/**
 * @brief DTO юнита корабля на карте
 *
 * @details
 * Сам класс корабля не зависит от своего местоположения и стороны (имперцы или пираты),
 *  поэтому сделана отдельная структура, которая по сути связывает класс с этими параметрами 
 */
struct ShipUnitDTO {
    std::string callsign;
    int x;
    int y;
    std::string side;
};
