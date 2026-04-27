#include <catch2/catch_test_macros.hpp>

#include <memory>

#include "dto/ShipDto.h"
#include "dto/WeaponDto.h"

#include "mapper/ShipMapper.h"
#include "mapper/WeaponMapper.h"

#include "ship/military_ship/include/MilitaryShip.h"
#include "ship/transport_ship/include/TransportShip.h"
#include "ship/transportmilitary_ship/include/TransportMilitaryShip.h"

#include "ship/military_ship/include/MilitaryShipMapper.h"
#include "ship/transport_ship/include/TransportShipMapper.h"
#include "ship/transportmilitary_ship/include/TMShipMapper.h"

#include "weapon/cannon/include/WeaponCannon.h"
#include "weapon/missile/include/WeaponMissile.h"
#include "weapon/torpedo/include/WeaponTorpedo.h"

#include "weapon/cannon/include/CannonMapper.h"
#include "weapon/missile/include/MissileMapper.h"
#include "weapon/torpedo/include/TorpedoMapper.h"

TEST_CASE("Concrete ship mappers: mapTo/mapFrom", "[mapper][ship]") {
    MilitaryShipMapper mm;
    MilitaryShip ms("M1", 100, 50, 200, 10, "cap", 1.0);
    ShipDto dto = mm.map_to(ms);
    REQUIRE(dto.type == "military");
    REQUIRE(dto.callsign == "M1");

    auto back = mm.map_from(dto);
    REQUIRE(back != nullptr);
    REQUIRE(back->get_callsign() == "M1");
    REQUIRE(back->get_captain() == "cap");
}

TEST_CASE("PolymorphicShipMapper: dispatch по typeid/key", "[mapper][ship]") {
    PolymorphicShipMapper poly;
    MilitaryShipSuperMapper m;
    TransportShipSuperMapper t;
    TMShipSuperMapper tm;
    poly.add_sub_mapper(m);
    poly.add_sub_mapper(t);
    poly.add_sub_mapper(tm);

    MilitaryShip s1("M1", 100, 10, 100, 0, "c", 1.0);
    Ship& asShip = s1;
    ShipDto dto = poly.map_to(asShip);
    REQUIRE(dto.type == "military");

    auto ptr = poly.map_from(dto);
    REQUIRE(ptr != nullptr);
    REQUIRE(ptr->get_type() == "MilitaryShip");
}

TEST_CASE("PolymorphicWeaponMapper: dispatch", "[mapper][weapon]") {
    PolymorphicWeaponMapper poly;
    CannonSuperMapper c;
    MissileSuperMapper m;
    TorpedoSuperMapper t;
    poly.add_sub_mapper(c);
    poly.add_sub_mapper(m);
    poly.add_sub_mapper(t);

    WeaponCannon cannon(5);
    Weapon& w = cannon;
    WeaponDto dto = poly.map_to(w);
    REQUIRE(dto.type == "cannon");
    REQUIRE(dto.current_ammunition == 5);

    auto back = poly.map_from(dto);
    REQUIRE(back != nullptr);
    REQUIRE(back->can_shoot());
}

TEST_CASE("TransportShipMapper: mapTo/mapFrom", "[mapper][ship]") {
    TransportShipMapper tm;
    TransportShip ts("T1", 100, 50, 200, 1000, "cap", 0.8, 150, 50);
    ShipDto dto = tm.map_to(ts);
    REQUIRE(dto.type == "transport");
    REQUIRE(dto.callsign == "T1");
    REQUIRE(dto.current_velocity == 50);
    REQUIRE(dto.current_HP == 200);
    REQUIRE(dto.params.at("max_cargo_weight") == "150");
    REQUIRE(dto.params.at("current_cargo_weight") == "50");

    auto back = tm.map_from(dto);
    REQUIRE(back != nullptr);
    REQUIRE(back->get_callsign() == "T1");
    REQUIRE(back->get_captain() == "cap");
}

TEST_CASE("TMShipMapper: mapTo/mapFrom", "[mapper][ship]") {
    TMShipMapper tm;
    TransportMilitaryShip tms("TMS1", 100, 50, 200, 1000, "cap", 0.8, 150, 50, 0.9);
    ShipDto dto = tm.map_to(tms);
    REQUIRE(dto.type == "transport_military");
    REQUIRE(dto.callsign == "TMS1");
    REQUIRE(dto.current_velocity == 50);
    REQUIRE(dto.current_HP == 200);
    REQUIRE(dto.params.at("max_cargo_weight") == "150");
    REQUIRE(dto.params.at("current_cargo_weight") == "50");

    auto back = tm.map_from(dto);
    REQUIRE(back != nullptr);
    REQUIRE(back->get_callsign() == "TMS1");
    REQUIRE(back->get_captain() == "cap");
}

TEST_CASE("MissileMapper: mapTo/mapFrom", "[mapper][weapon]") {
    MissileMapper mm;
    WeaponMissile missile(15);
    WeaponDto dto = mm.map_to(missile);
    REQUIRE(dto.type == "missile");
    REQUIRE(dto.current_ammunition == 15);
    REQUIRE(dto.id == missile.get_id());

    auto back = mm.map_from(dto);
    REQUIRE(back != nullptr);
    REQUIRE(back->can_shoot());
    REQUIRE(back->get_current_ammunition() == 15);
}

TEST_CASE("TorpedoMapper: mapTo/mapFrom", "[mapper][weapon]") {
    TorpedoMapper tm;
    WeaponTorpedo torpedo(20);
    WeaponDto dto = tm.map_to(torpedo);
    REQUIRE(dto.type == "torpedo");
    REQUIRE(dto.current_ammunition == 20);
    REQUIRE(dto.id == torpedo.get_id());

    auto back = tm.map_from(dto);
    REQUIRE(back != nullptr);
    REQUIRE(back->can_shoot());
    REQUIRE(back->get_current_ammunition() == 20);
}

TEST_CASE("PolymorphicShipMapper: обработка всех типов", "[mapper][ship]") {
    PolymorphicShipMapper poly;
    MilitaryShipSuperMapper ms;
    TransportShipSuperMapper ts;
    TMShipSuperMapper tms;
    poly.add_sub_mapper(ms);
    poly.add_sub_mapper(ts);
    poly.add_sub_mapper(tms);

    MilitaryShip military("M1", 100, 10, 100, 0, "c", 1.0);
    TransportShip transport("T1", 100, 10, 100, 0, "c", 1.0, 150, 50);
    TransportMilitaryShip hybrid("H1", 100, 10, 100, 0, "c", 1.0, 150, 50, 0.9);

    Ship& m_ship = military;
    Ship& t_ship = transport;
    Ship& h_ship = hybrid;

    ShipDto m_dto = poly.map_to(m_ship);
    ShipDto t_dto = poly.map_to(t_ship);
    ShipDto h_dto = poly.map_to(h_ship);

    REQUIRE(m_dto.type == "military");
    REQUIRE(t_dto.type == "transport");
    REQUIRE(h_dto.type == "transport_military");

    auto m_back = poly.map_from(m_dto);
    auto t_back = poly.map_from(t_dto);
    auto h_back = poly.map_from(h_dto);

    REQUIRE(m_back != nullptr);
    REQUIRE(t_back != nullptr);
    REQUIRE(h_back != nullptr);
    REQUIRE(m_back->get_type() == "MilitaryShip");
    REQUIRE(t_back->get_type() == "TransportShip");
    REQUIRE(h_back->get_type() == "TransportMilitaryShip");
}

TEST_CASE("PolymorphicWeaponMapper: обработка всех типов", "[mapper][weapon]") {
    PolymorphicWeaponMapper poly;
    CannonSuperMapper cm;
    MissileSuperMapper mm;
    TorpedoSuperMapper tm;
    poly.add_sub_mapper(cm);
    poly.add_sub_mapper(mm);
    poly.add_sub_mapper(tm);

    WeaponCannon cannon(10);
    WeaponMissile missile(15);
    WeaponTorpedo torpedo(20);

    Weapon& c_weapon = cannon;
    Weapon& m_weapon = missile;
    Weapon& t_weapon = torpedo;

    WeaponDto c_dto = poly.map_to(c_weapon);
    WeaponDto m_dto = poly.map_to(m_weapon);
    WeaponDto t_dto = poly.map_to(t_weapon);

    REQUIRE(c_dto.type == "cannon");
    REQUIRE(m_dto.type == "missile");
    REQUIRE(t_dto.type == "torpedo");

    auto c_back = poly.map_from(c_dto);
    auto m_back = poly.map_from(m_dto);
    auto t_back = poly.map_from(t_dto);

    REQUIRE(c_back != nullptr);
    REQUIRE(m_back != nullptr);
    REQUIRE(t_back != nullptr);
    REQUIRE(c_back->can_shoot());
    REQUIRE(m_back->can_shoot());
    REQUIRE(t_back->can_shoot());
}

TEST_CASE("Mapper: get_id utility", "[mapper]") {
    unsigned id1 = get_id(100, 200);
    unsigned id2 = get_id(50, 100);
    
    REQUIRE(id1 == ((100u << 16) | 200u));
    REQUIRE(id2 == ((50u << 16) | 100u));
    REQUIRE(id1 != id2);
}

TEST_CASE("MilitaryShip с оружием: полное маппинг", "[mapper][ship]") {
    MilitaryShipMapper mm;
    MilitaryShip ms("M1", 100, 50, 200, 1000, "cap", 1.0);
    
    auto cannon = std::make_unique<WeaponCannon>(10);
    unsigned weapon_id = cannon->get_id();
    ms.set_weapon(BOW, std::move(cannon));
    
    ShipDto dto = mm.map_to(ms);
    REQUIRE(dto.params.find("bow") != dto.params.end());
    REQUIRE(dto.params.at("bow") == std::to_string(weapon_id));
    
    auto back = mm.map_from(dto);
    REQUIRE(back != nullptr);
    REQUIRE(back->weapon_count() == 1);
}

TEST_CASE("TransportShip с грузом: полное маппинг", "[mapper][ship]") {
    TransportShipMapper tm;
    TransportShip ts("T1", 100, 50, 200, 1000, "cap", 0.8, 150, 75);
    
    ShipDto dto = tm.map_to(ts);
    REQUIRE(dto.params.at("max_cargo_weight") == "150");
    REQUIRE(dto.params.at("current_cargo_weight") == "75");
    
    auto back = tm.map_from(dto);
    REQUIRE(back != nullptr);
    REQUIRE(back->get_max_cargo_weight() == 150);
    REQUIRE(back->get_current_cargo_weight() == 75);
}
