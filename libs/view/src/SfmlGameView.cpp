#include "../include/SfmlGameView.h"

#include "dto/WeaponDto.h"
#include "presenter/GamePresenter.h"
#include "service/MissionService.h"
#include "model/point/Point.h"

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Audio.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>
#include <set>
#include <limits>



bool        is_strings_equal(const std::string& a, const std::string& b);
std::string lower_string(const std::string & s);
void        fit_sprite_to_square(sf::Sprite& sprite, sf::Vector2f pos, float size);
void        fit_sprite_to_screen(sf::Sprite& sprite, sf::Vector2u window_size);

static constexpr const char * configp = "../config/";
static constexpr const char * iconsp = "../icons/";
static constexpr const char * fontsp = "../fonts/BoldPixels.otf";
static constexpr const char * musicp = "../music/Intense Battle Theme.wav";
static constexpr const char * clickp = "../music/click2.wav";


SfmlGameView::SfmlGameView(GamePresenter& presenter, MissionService& mission_service)
    : presenter_(presenter), mission_service_(mission_service),
      config_path_dir(configp), font_path(fontsp),
      icon_path_dir(iconsp), screen(Screen::Start),
      main_tab(MainTab::ATTACK), aux_panel(AuxPanel::NONE), 
      buy_ship_type("military"),
      win(sf::VideoMode::getDesktopMode(), "Space Convoy (SFML)") {
        if (!font.openFromFile(font_path)) {
            throw std::runtime_error( "SFML UI: could not load a font");
        }
        win.setPosition({0, 0});
        win.setFramerateLimit(60);

        if (background_music_.openFromFile(musicp)) {
            background_music_.setLooping(true);
            background_music_.setVolume(35.f);
            background_music_.play();
        }

        if (click_buffer_.loadFromFile(clickp)) {
            click_sound_.emplace(click_buffer_);
            click_sound_->setVolume(55.f);
        }

        pressed_area = sf::Vector2f{-10000.f, -10000.f};
        convoy_info_scroll_lines = 0;
        pirate_info_scroll_lines = 0;
        init_buttons();
        init_text_inputs();
        init_info_fields();
        init_textures();
      };

void SfmlGameView::init_buttons() {
    buttons.try_emplace("buy_ship", font, "Buy Ship");
    buttons.try_emplace("sell_ship", font, "Sell Ship");
    buttons.try_emplace("set_weapon", font, "Set Weapon");
    buttons.try_emplace("unset_weapon", font, "Unset Weapon");
    buttons.try_emplace("reload_all", font, "Reload all (100$)");
    buttons.try_emplace("cargo_auto", font, "Auto distribute cargo");
    buttons.try_emplace("cargo_load", font, "Load cargo x1.5$");
    buttons.try_emplace("cargo_unload", font, "Unload cargo");
    buttons.try_emplace("bow", font, "Bow");
    buttons.try_emplace("stern", font, "Stern");
    buttons.try_emplace("starboard", font, "Starboard");
    buttons.try_emplace("portside", font, "Portside");
    buttons.try_emplace("do_attack", font, "Fire!");
    buttons.try_emplace("fleet_confirm", font, "OK");
    buttons.try_emplace("fleet_cancel", font, "Cancel");
    buttons.try_emplace("weapon_confirm", font, "OK");
    buttons.try_emplace("weapon_cancel", font, "Cancel");
    buttons.try_emplace("cargo_confirm", font, "OK");
    buttons.try_emplace("cargo_cancel", font, "Cancel");
    buttons.try_emplace("attack", font, "Attack");
    buttons.try_emplace("ships", font, "Ships");
    buttons.try_emplace("weapons", font, "Weapons");
    buttons.try_emplace("cargo", font, "Load cargo");
    buttons.try_emplace("end_turn", font, "End Turn");
    buttons.try_emplace("exit_game", font, "Exit");
    buttons.try_emplace("load", font, "Load");
    buttons.try_emplace("new_game", font, "New Game");
    buttons.try_emplace("exit", font, "Exit");
    buttons.try_emplace("easy_game", font, "Easy");
    buttons.try_emplace("medium_game", font, "Medium");
    buttons.try_emplace("hard_game", font, "Hard");
    buttons.try_emplace("back", font, "Back");

    buttons.at("reload_all").label.setCharacterSize(18);
    buttons.at("cargo_auto").label.setCharacterSize(16);

    buttons.try_emplace("yes", font, "Yes");
    buttons.try_emplace("no", font, "No");
    buttons.try_emplace("mission_complete_exit", font, "Exit");
    auto& btn_dialog_yes = buttons.at("yes");
    auto& btn_dialog_no = buttons.at("no");

    btn_dialog_yes.rect.setSize(sf::Vector2f{100.f, 40.f});
    btn_dialog_yes.rect.setFillColor(rgb(50, 50, 50));
    btn_dialog_yes.rect.setOutlineColor(rgb(0, 0, 0));
    btn_dialog_yes.rect.setOutlineThickness(1.f);
    btn_dialog_yes.label.setCharacterSize(18);
    btn_dialog_yes.label.setFillColor(sf::Color::Black);

    btn_dialog_no.rect.setSize(sf::Vector2f{100.f, 40.f});
    btn_dialog_no.rect.setFillColor(rgb(50, 50, 50));
    btn_dialog_no.rect.setOutlineColor(rgb(0,0,0));
    btn_dialog_no.rect.setOutlineThickness(1.f);
    btn_dialog_no.label.setCharacterSize(18);
    btn_dialog_no.label.setFillColor(sf::Color::Black);

};

void SfmlGameView::init_text_inputs() {
    text_inputs.try_emplace("weapon_slot", font, 16, "bow");
    text_inputs.try_emplace("buy_callsign", font, 16);
    text_inputs.try_emplace("buy_captain", font, 16);
    text_inputs.try_emplace("cargo_weight", font, 16);
    text_inputs.try_emplace("cargo_unload_ship", font, 16);

};

void SfmlGameView::init_info_fields() {
    info_fields.try_emplace("attack_attacker", font, 16);
    info_fields.try_emplace("attack_target", font, 16);
    info_fields.try_emplace("sell_ship", font, 16);
    info_fields.try_emplace("weapon_ship", font, 16);
    info_fields.try_emplace("unset_ship", font, 16);
    info_fields.try_emplace("cargo_ship", font, 16);

    info_fields.at("attack_attacker").set_value("(click on map)");
    info_fields.at("attack_target").set_value("(click on map)");
    info_fields.at("sell_ship").set_value("(click on map)");
    info_fields.at("weapon_ship").set_value("(click on map)");
    info_fields.at("unset_ship").set_value("(click on map)");
    info_fields.at("cargo_ship").set_value("(click on map)");
}

void SfmlGameView::init_textures() {
    textures.clear();
    sprites.clear();
    textures.reserve(16);
    sprites.reserve(16);

    auto load = [&](const std::string& tex_key, const std::string& file) -> sf::Texture& {
        auto [it, inserted] = textures.try_emplace(tex_key);
        sf::Texture& tex = it->second;
        if (!tex.loadFromFile(icon_path_dir + file)) {
            throw std::runtime_error("SFML UI: could not load texture: " + (icon_path_dir + file));
        }
        return tex;
    };

    auto try_load_any = [&](const std::string& tex_key, const std::vector<std::string>& files) -> sf::Texture* {
        auto [it, inserted] = textures.try_emplace(tex_key);
        sf::Texture& tex = it->second;
        for (const auto& f : files) {
            if (tex.loadFromFile(icon_path_dir + f)) {
                return &tex;
            }
        }
        textures.erase(tex_key);
        return nullptr;
    };

    sf::Texture& icon_military_texture  = load("icon_military_texture",  "military_ship.png");
    sf::Texture& icon_transport_texture = load("icon_transport_texture", "transport_ship.png");
    sf::Texture& icon_tm_texture        = load("icon_tm_texture",        "tm_ship.png");
    sf::Texture& icon_cannon_texture    = load("icon_cannon_texture",    "cannon.png");
    sf::Texture& icon_missile_texture   = load("icon_missile_texture",   "missile.png");
    sf::Texture& icon_torpedo_texture   = load("icon_torpedo_texture",   "torpedo.png");
    sf::Texture& start_bg_texture       = load("start_bg_texture",       "start_bg.png");
    sf::Texture& map_bg_texture         = load("map_bg_texture",         "map_bg.png");
    load("button_texture",             "button.png");

    try_load_any("map_empire_military_texture", {"empire_military.png"});
    try_load_any("map_empire_transport_texture", {"empire_transport.png", "empire_trasport.png"});
    try_load_any("map_empire_transport_military_texture", {"empire_transport_milirary.png"});
    try_load_any("map_pirate_texture", {"pirate.png"});

    sprites.emplace("icon_military_sprite", sf::Sprite(icon_military_texture));
    sprites.emplace("icon_transport_sprite", sf::Sprite(icon_transport_texture));
    sprites.emplace("icon_tm_sprite", sf::Sprite(icon_tm_texture));
    sprites.emplace("icon_cannon_sprite", sf::Sprite(icon_cannon_texture));
    sprites.emplace("icon_missile_sprite", sf::Sprite(icon_missile_texture));
    sprites.emplace("icon_torpedo_sprite", sf::Sprite(icon_torpedo_texture));
    sprites.emplace("start_bg_sprite", sf::Sprite(start_bg_texture));
    sprites.emplace("map_bg_sprite", sf::Sprite(map_bg_texture));

    if (auto it = textures.find("map_empire_military_texture"); it != textures.end()) {
        sprites.emplace("map_empire_military_sprite", sf::Sprite(it->second));
    }
    if (auto it = textures.find("map_empire_transport_texture"); it != textures.end()) {
        sprites.emplace("map_empire_transport_sprite", sf::Sprite(it->second));
    }
    if (auto it = textures.find("map_empire_transport_military_texture"); it != textures.end()) {
        sprites.emplace("map_empire_transport_military_sprite", sf::Sprite(it->second));
    }
    if (auto it = textures.find("map_pirate_texture"); it != textures.end()) {
        sprites.emplace("map_pirate_sprite", sf::Sprite(it->second));
    }

    rectshapes.try_emplace("icon_military_frame");
    rectshapes.try_emplace("icon_transport_frame");
    rectshapes.try_emplace("icon_tm_frame");
    rectshapes.try_emplace("icon_cannon_frame");
    rectshapes.try_emplace("icon_missile_frame");
    rectshapes.try_emplace("icon_torpedo_frame");

    auto window_size = win.getSize();
    sf::RectangleShape overlay;
    overlay.setPosition({0.f, 0.f});
    overlay.setSize({static_cast<float>(window_size.x), static_cast<float>(window_size.y)});
    overlay.setFillColor(sf::Color(0, 0, 0, 170));
    rectshapes.emplace(std::string("overlay"), overlay);

    sf::RectangleShape box;
    box.setPosition({ (window_size.x - 400.f) / 2, (window_size.y - 200.f) / 2});
    box.setSize({400.f, 200.f});
    box.setFillColor(rgb(25, 25, 25));
    box.setOutlineColor(rgb(160, 160, 160));
    box.setOutlineThickness(1.f);
    rectshapes.emplace(std::string("box"), box);

    apply_button_background();

}

void SfmlGameView::apply_button_background() {
    auto it = textures.find("button_texture");
    if (it == textures.end()) {
        return;
    }
    const sf::Texture* tex = &it->second;
    for (auto& [key, b] : buttons) {
        (void)key;
        b.set_background_texture(tex);
    }
}

void SfmlGameView::run() {
    process_game();
}

void SfmlGameView::process_start_screen() {
    auto & btn_load = buttons.at("load");
    auto & btn_new = buttons.at("new_game");
    auto & btn_exit = buttons.at("exit");

    sf::Vector2u window_size = win.getSize();
    
    if (sprites.find("start_bg_sprite") != sprites.end()) {
        auto& bg_sprite = sprites.at("start_bg_sprite");
        fit_sprite_to_screen(bg_sprite, window_size);
        win.draw(bg_sprite);
    } else {
        win.clear(rgb(8, 8, 10));
    }
    
    float x_size, y_size;
    x_size = 200.f;
    y_size = 50.f;
    float x_pos = (window_size.x - x_size) / 2.0f;
    float y_pos = window_size.y / 2.0f - y_size * 1.5f;

    for (Button * b: {&btn_load, &btn_new, &btn_exit}) {
        place_button(*b, {x_pos, y_pos}, {x_size, y_size});
        y_pos += y_size * 1.5f;
    }
    
    sf::Text title(font);
    title.setCharacterSize(80);
    title.setFillColor(sf::Color::White);
    title.setString("Kriegsgeleit");
    sf::FloatRect title_bounds = title.getLocalBounds();
    title.setOrigin(sf::Vector2f{title_bounds.position.x + title_bounds.size.x / 2.f, title_bounds.position.y + title_bounds.size.y / 2.f});
    title.setPosition(sf::Vector2f{window_size.x / 2.f, 120.f});
    win.draw(title);

    for (auto* b : {&btn_load, &btn_new, &btn_exit}) {
        draw_button(*b);
    }
    if (btn_load.hit(pressed_area)) {
        try_load_game(config_path_dir, log, screen);
    } else if (btn_new.hit(pressed_area)) {
        screen = Screen::MissionSelect;
    } else if (btn_exit.hit(pressed_area)) {
        win.close();
    }
    pressed_area = sf::Vector2f{-10000.f, -10000.f};
}

void SfmlGameView::process_mission_select_screen() {
    auto & btn_easy = buttons.at("easy_game");
    auto & btn_medium = buttons.at("medium_game");
    auto & btn_hard = buttons.at("hard_game");
    auto & btn_back = buttons.at("back");

    sf::Vector2u window_size = win.getSize();
    
    if (sprites.find("start_bg_sprite") != sprites.end()) {
        auto& bg_sprite = sprites.at("start_bg_sprite");
        fit_sprite_to_screen(bg_sprite, window_size);
        win.draw(bg_sprite);
    } else {
        win.clear(rgb(8, 8, 10));
    }
    
    float x_size, y_size;
    x_size = 200.f;
    y_size = 50.f;
    float x_pos = (window_size.x - x_size) / 2.0f;
    float y_pos = window_size.y / 2.0f - y_size * 1.5f;
    for (auto * b: {&btn_easy, &btn_medium, &btn_hard, &btn_back}) {
        place_button(*b, {x_pos, y_pos}, {x_size, y_size});
        y_pos += y_size * 1.5f;
    }
    sf::Text title(font);
    title.setCharacterSize(50);
    title.setFillColor(sf::Color::White);
    title.setString("Select mission");
    sf::FloatRect title_bounds = title.getLocalBounds();
    title.setOrigin(sf::Vector2f{title_bounds.position.x + title_bounds.size.x / 2.f, title_bounds.position.y + title_bounds.size.y / 2.f});
    title.setPosition(sf::Vector2f{window_size.x / 2.f, 80.f});
    win.draw(title);
    
    for (auto* b : {&btn_easy, &btn_medium, &btn_hard, &btn_back}) {
        draw_button(*b);
    }
    if (btn_easy.hit(pressed_area)) {
        try_load_game(config_path_dir    + "/easy", log, screen);
    } else if (btn_medium.hit(pressed_area)) {
        try_load_game(config_path_dir + "/medium", log, screen);
    } else if (btn_hard.hit(pressed_area)) {
        try_load_game(config_path_dir + "/hard", log, screen);
    } else if (btn_back.hit(pressed_area)) {
        screen = Screen::Start;
    }    

    pressed_area = sf::Vector2f{-10000.f, -10000.f};
}

void SfmlGameView::process_game_screen() {
    auto & btn_attack = buttons.at("attack");
    auto & buy_callsign = text_inputs.at("buy_callsign");
    auto & buy_captain = text_inputs.at("buy_captain");
    (void)text_inputs.at("weapon_slot");

    auto & btn_ships = buttons.at("ships");
    auto & btn_weapons = buttons.at("weapons");
    auto & btn_cargo = buttons.at("cargo");
    auto & btn_end_turn = buttons.at("end_turn");

    planned_attack_center.reset();

    if (btn_attack.hit(pressed_area)) 
    {
        main_tab = MainTab::ATTACK;
        aux_panel = (aux_panel == AuxPanel::ATTACK) ? AuxPanel::NONE : AuxPanel::ATTACK;
        buy_callsign.active = buy_captain.active = false;

        if (aux_panel == AuxPanel::ATTACK) {
            attack_pick_stage = AttackPickStage::Attacker;
            selected_attack_attacker_.reset();
            selected_attack_target_.reset();
            info_fields.at("attack_attacker").set_value("(click on map)");
            info_fields.at("attack_target").set_value("(click on map)");
        }
    } 
    else if (btn_ships.hit(pressed_area)) 
    {
        main_tab = MainTab::SHIPS;
        if (!(aux_panel == AuxPanel::SHIPBUY || aux_panel == AuxPanel::SHIPSELL)) {
            aux_panel = AuxPanel::NONE;
        }
    } 
    else if (btn_weapons.hit(pressed_area)) 
    {
        main_tab = MainTab::WEAPONS;
        if (!(aux_panel == AuxPanel::WEAPONSET || aux_panel == AuxPanel::WEAPONUNSET)) {
            aux_panel = AuxPanel::NONE;
        }
    } 
    else if (btn_cargo.hit(pressed_area)) 
    {
        main_tab = MainTab::CARGO;
        if (!(aux_panel == AuxPanel::CARGOLOAD || aux_panel == AuxPanel::CARGOUNLOAD)) {
            aux_panel = AuxPanel::NONE;
        }
    } 
    else if (btn_end_turn.hit(pressed_area)) 
    {
        push_log(log, "Move: pirates left, imperials right");
        presenter_.move_ships();
        push_log(log, "Pirates auto-attack");
        presenter_.process_pirate_attacks();
        presenter_.end_turn();
        refresh_state();

        if (presenter_.check_base_arrival()) {
            push_log(log, "MISSION COMPLETE!");
            mission_complete = true;
            mission_complete_clock.restart();
            show_mission_complete_dialog = true;
        }
    }
}

void SfmlGameView::place_main_menu () {
    
    pressed_area = sf::Vector2f{-10000.f, -10000.f};
    win.clear(rgb(8, 8, 10));

    const float W = static_cast<float>(win.getSize().x);
    const float H = static_cast<float>(win.getSize().y);
    const float margin = 12.f;
    const float gap = 12.f;
    const float header_h = 72.f;
    const float content_top = header_h + margin;
    const float content_h = H - content_top - margin;
    const float available_w = W - 2.f * margin - 4.f * gap;
        
    float map_w = std::max(620.f, available_w * 0.58f);
    float main_w = std::max(170.f, available_w * 0.10f);
    float aux_w = std::max(230.f, available_w * 0.12f);
    float mission_w = std::max(210.f, available_w * 0.10f);
    float pirate_w = std::max(210.f, available_w * 0.10f);

    const float total = map_w + main_w + aux_w + mission_w + pirate_w;
    if (total > available_w) {
        map_w = std::max(450.f, map_w - (total - available_w));
    }

    float x = margin;
    const float map_x = x;
    x += map_w + gap;
    const float main_x = x;
    x += main_w + gap;
    const float aux_x = x;
    x += aux_w + gap;
    const float mission_x = x;
    x += mission_w + gap;
    const float pirate_x = x;

    const float log_h = std::clamp(content_h * 0.18f, 120.f, 240.f);
    const float map_h = std::max(240.f, content_h - log_h - gap);
    map_rect = sf::FloatRect(sf::Vector2f{map_x, content_top}, sf::Vector2f{map_w, map_h});
    log_rect = sf::FloatRect(sf::Vector2f{map_x, content_top + map_h + gap}, sf::Vector2f{map_w, log_h});

    const float min_right_top_h = std::min(540.f, content_h);
    const float max_mission_info_h = std::max(0.f, content_h - min_right_top_h - gap);
    float mission_info_h = std::clamp(content_h * 0.22f, 120.f, 220.f);
    mission_info_h = std::min(mission_info_h, max_mission_info_h);
    const float right_top_h = std::max(min_right_top_h, content_h - mission_info_h - gap);

    main_menu_rect = sf::FloatRect(sf::Vector2f{main_x, content_top}, sf::Vector2f{main_w, right_top_h});
    aux_rect = sf::FloatRect(sf::Vector2f{aux_x, content_top}, sf::Vector2f{aux_w, right_top_h});

    mission_info_rect = sf::FloatRect(
        sf::Vector2f{main_x, content_top + right_top_h + gap},
        sf::Vector2f{main_w + gap + aux_w, mission_info_h}
    );

    mission_rect = sf::FloatRect(sf::Vector2f{mission_x, content_top}, sf::Vector2f{mission_w, content_h});
    pirate_rect = sf::FloatRect(sf::Vector2f{pirate_x, content_top}, sf::Vector2f{pirate_w, content_h});

    const float aux_top_h = std::clamp(right_top_h * 0.20f, 130.f, 210.f);
    aux_top_rect = sf::FloatRect(sf::Vector2f{aux_x, content_top}, sf::Vector2f{aux_w, aux_top_h});
    aux_bottom_rect = sf::FloatRect(sf::Vector2f{aux_x, content_top + aux_top_h + gap}, sf::Vector2f{aux_w, std::max(120.f, right_top_h - aux_top_h - gap)});

    const float mx = main_menu_rect.position.x + 10.f;
    float my = main_menu_rect.position.y + 34.f;
    const sf::Vector2f m_size{main_menu_rect.size.x - 20.f, 40.f};

    const float m_step = 50.f;
    for (const auto& b : {"attack", "ships", "weapons", "cargo", "end_turn", "exit_game"}) {
        place_button(buttons.at(b), {mx, my}, m_size);
        my += m_step;
    }

    const float ax = aux_top_rect.position.x + 10.f;
    float ay = aux_top_rect.position.y + 34.f;
    const sf::Vector2f a_size{aux_top_rect.size.x - 20.f, 40.f};
    const float a_step = 50.f;
    place_button(buttons.at("buy_ship"), {ax, ay}, a_size);
    ay += a_step;
    place_button(buttons.at("sell_ship"), {ax, ay}, a_size);

    float ay_weapon = aux_top_rect.position.y + 34.f;
    for (const auto& b : {"set_weapon", "unset_weapon", "reload_all"}) {
        place_button(buttons.at(b), {ax, ay_weapon}, a_size);
        ay_weapon += a_step;
    }

    ay = aux_top_rect.position.y + 34.f;
    for (const auto& b : {"cargo_auto", "cargo_load", "cargo_unload"}) {
        place_button(buttons.at(b), {ax, ay}, a_size);
        ay += a_step;
    }

    {
        const float fx = aux_top_rect.position.x + 10.f;
        const float input_h = 34.f;
        const float field_gap = 66.f;
        const float form_top = aux_top_rect.position.y + 60.f;

        place_info_field(info_fields.at("attack_attacker"), {fx, form_top}, {aux_top_rect.size.x - 20.f, input_h});
        place_info_field(info_fields.at("attack_target"), {fx, form_top + field_gap}, {aux_top_rect.size.x - 20.f, input_h});

        place_text_input(text_inputs.at("weapon_slot"), {-10000.f, -10000.f}, {1.f, 1.f});

    }

    const float fx = aux_bottom_rect.position.x + 10.f;
    const float input_h = 36.f;
    const float small_btn_h = 36.f;
    const float field_gap = 76.f;
    const float form_top = aux_bottom_rect.position.y + 72.f;

    {
        if (aux_panel == AuxPanel::ATTACK) {
            float by = aux_bottom_rect.position.y + 110.f;
            for (const auto & b : {"bow", "stern", "starboard", "portside", "do_attack"}) {
                place_button(buttons.at(b), {fx, by}, {aux_bottom_rect.size.x - 20.f, 40.f});
                by += 52.f;
            }
        } else if (aux_panel == AuxPanel::WEAPONSET || aux_panel == AuxPanel::WEAPONUNSET) {
            const float bw = std::max(70.f, (aux_bottom_rect.size.x - 30.f) * 0.5f);
            const float bh = 40.f;
            const float bx1 = fx;
            const float bx2 = fx + bw + 10.f;
            const float callsign_y_local = form_top + 54.f + 44.f;
            float by = callsign_y_local + field_gap + 10.f;

            place_button(buttons.at("bow"), {bx1, by}, {bw, bh});
            place_button(buttons.at("stern"), {bx2, by}, {bw, bh});
            by += 52.f;
            place_button(buttons.at("portside"), {bx1, by}, {bw, bh});
            place_button(buttons.at("starboard"), {bx2, by}, {bw, bh});

            place_button(buttons.at("do_attack"), {-10000.f, -10000.f}, {1.f, 1.f});
        } else {
            for (const auto & b : {"bow", "stern", "starboard", "portside", "do_attack"}) {
                place_button(buttons.at(b), {-10000.f, -10000.f}, {1.f, 1.f});
            }
        }
    }
    const float icon_size = 54.f;
    const float icon_gap = 14.f;
    const float icons_y = form_top;
    const float icon_total_w = icon_size * 3.f + icon_gap * 2.f;
    const float icons_area_w = aux_bottom_rect.size.x - 20.f;
    const float icons_x0 = fx + std::max(0.f, (icons_area_w - icon_total_w) * 0.5f);
    const sf::Vector2f pos_military{icons_x0, icons_y};
    const sf::Vector2f pos_transport{icons_x0 + icon_size + icon_gap, icons_y};
    const sf::Vector2f pos_tm{icons_x0 + (icon_size + icon_gap) * 2.f, icons_y};

    fit_sprite_to_square(sprites.at("icon_military_sprite"), pos_military, icon_size);
    fit_sprite_to_square(sprites.at("icon_transport_sprite"), pos_transport, icon_size);
    fit_sprite_to_square(sprites.at("icon_tm_sprite"), pos_tm, icon_size);

    auto setup_icon_frame = [&](sf::RectangleShape& frame, sf::Vector2f pos, bool selected) {
        frame.setPosition(pos);
        frame.setSize(sf::Vector2f{icon_size, icon_size});
        frame.setFillColor(sf::Color(0, 0, 0, 0));
        frame.setOutlineThickness(2.f);
        frame.setOutlineColor(selected ? sf::Color::White : rgb(120, 120, 120));
    };

    setup_icon_frame(rectshapes.at("icon_military_frame"), pos_military, buy_ship_type == "military");
    setup_icon_frame(rectshapes.at("icon_transport_frame"), pos_transport, buy_ship_type == "transport");
    setup_icon_frame(rectshapes.at("icon_tm_frame"), pos_tm, buy_ship_type == "transport_military");

    fit_sprite_to_square(sprites.at("icon_cannon_sprite"), pos_military, icon_size);
    fit_sprite_to_square(sprites.at("icon_missile_sprite"), pos_transport, icon_size);
    fit_sprite_to_square(sprites.at("icon_torpedo_sprite"), pos_tm, icon_size);

    setup_icon_frame(rectshapes.at("icon_cannon_frame"), pos_military, weapon_set_type == "cannon");
    setup_icon_frame(rectshapes.at("icon_missile_frame"), pos_transport, weapon_set_type == "missile");
    setup_icon_frame(rectshapes.at("icon_torpedo_frame"), pos_tm, weapon_set_type == "torpedo");

    const float callsign_y = icons_y + icon_size + 44.f;
    place_text_input(text_inputs.at("buy_callsign"), {fx, callsign_y}, {aux_bottom_rect.size.x - 20.f, input_h});
    place_text_input(text_inputs.at("buy_captain"), {fx, callsign_y + field_gap}, {aux_bottom_rect.size.x - 20.f, input_h});
    place_info_field(info_fields.at("sell_ship"), {fx, form_top}, {aux_bottom_rect.size.x - 20.f, input_h});

    const float bottom_btn_y = aux_bottom_rect.position.y + aux_bottom_rect.size.y - (small_btn_h + 10.f);
    place_button(buttons.at("fleet_confirm"), {fx, bottom_btn_y}, {std::max(70.f, (aux_bottom_rect.size.x - 30.f) * 0.5f), small_btn_h});
    place_button(buttons.at("fleet_cancel"), {fx + (aux_bottom_rect.size.x - 30.f) * 0.5f + 10.f, bottom_btn_y}, {std::max(70.f, (aux_bottom_rect.size.x - 30.f) * 0.5f), small_btn_h});

    if (aux_panel == AuxPanel::WEAPONSET) {
        place_info_field(info_fields.at("weapon_ship"), {fx, callsign_y}, {aux_bottom_rect.size.x - 20.f, input_h});
    } else {
        place_info_field(info_fields.at("weapon_ship"), {fx, form_top}, {aux_bottom_rect.size.x - 20.f, input_h});
    }

    if (aux_panel == AuxPanel::WEAPONUNSET) {
        place_info_field(info_fields.at("unset_ship"), {fx, callsign_y}, {aux_bottom_rect.size.x - 20.f, input_h});
    } else {
        place_info_field(info_fields.at("unset_ship"), {fx, form_top}, {aux_bottom_rect.size.x - 20.f, input_h});
    }

    const float bottom_btn_y2 = aux_bottom_rect.position.y + aux_bottom_rect.size.y - (small_btn_h + 10.f);
    place_button(buttons.at("weapon_confirm"), {fx, bottom_btn_y2}, {std::max(70.f, (aux_bottom_rect.size.x - 30.f) * 0.5f), small_btn_h});
    place_button(buttons.at("weapon_cancel"), {fx + (aux_bottom_rect.size.x - 30.f) * 0.5f + 10.f, bottom_btn_y2}, {std::max(70.f, (aux_bottom_rect.size.x - 30.f) * 0.5f), small_btn_h});

    place_info_field(info_fields.at("cargo_ship"), {fx, form_top}, {aux_bottom_rect.size.x - 20.f, input_h});
    place_text_input(text_inputs.at("cargo_weight"), {fx, form_top + field_gap}, {aux_bottom_rect.size.x - 20.f, input_h});
    place_text_input(text_inputs.at("cargo_unload_ship"), {fx, form_top}, {aux_bottom_rect.size.x - 20.f, input_h});

    const float bottom_btn_y3 = aux_bottom_rect.position.y + aux_bottom_rect.size.y - (small_btn_h + 10.f);
    place_button(buttons.at("cargo_confirm"), {fx, bottom_btn_y3}, {std::max(70.f, (aux_bottom_rect.size.x - 30.f) * 0.5f), small_btn_h});
    place_button(buttons.at("cargo_cancel"), {fx + (aux_bottom_rect.size.x - 30.f) * 0.5f + 10.f, bottom_btn_y3}, {std::max(70.f, (aux_bottom_rect.size.x - 30.f) * 0.5f), small_btn_h});
}

void SfmlGameView::refresh_text_input_states() {
    auto & buy_callsign = text_inputs.at("buy_callsign");
    auto & buy_captain = text_inputs.at("buy_captain");
    auto & cargo_weight = text_inputs.at("cargo_weight");
    auto & cargo_unload_ship = text_inputs.at("cargo_unload_ship");
    buy_callsign.active        = (aux_panel == AuxPanel::SHIPBUY) && buy_callsign.hit(pressed_area);
    buy_captain.active         = (aux_panel == AuxPanel::SHIPBUY) && buy_captain.hit(pressed_area);
    cargo_weight.active        = (aux_panel == AuxPanel::CARGOLOAD) && cargo_weight.hit(pressed_area);
    cargo_unload_ship.active   = (aux_panel == AuxPanel::CARGOUNLOAD) && cargo_unload_ship.hit(pressed_area);
}

void SfmlGameView::set_text_input_states(bool state) {
    for (auto & ti: text_inputs) {
        ti.second.active = state;
    }
}


void SfmlGameView::process_game() {
    auto & btn_attack = buttons.at("attack");
    auto & btn_ships = buttons.at("ships");
    auto & btn_weapons = buttons.at("weapons");
    auto & btn_cargo = buttons.at("cargo");
    auto & btn_end_turn = buttons.at("end_turn");
    auto & btn_exit_game = buttons.at("exit_game");
    auto & btn_buy_ship = buttons.at("buy_ship");
    auto & btn_sell_ship = buttons.at("sell_ship");
    auto & btn_set_weapon = buttons.at("set_weapon");
    auto & btn_unset_weapon = buttons.at("unset_weapon");
    auto & btn_reload_all = buttons.at("reload_all");
    auto & btn_cargo_auto = buttons.at("cargo_auto");
    auto & btn_cargo_load = buttons.at("cargo_load");
    auto & btn_cargo_unload = buttons.at("cargo_unload");
    auto & btn_bow = buttons.at("bow");
    auto & btn_stern = buttons.at("stern");
    auto & btn_starboard = buttons.at("starboard");
    auto & btn_portside = buttons.at("portside");
    auto & btn_do_attack = buttons.at("do_attack");
    auto & btn_fleet_confirm = buttons.at("fleet_confirm");
    auto & btn_fleet_cancel = buttons.at("fleet_cancel");
    auto & btn_weapon_confirm = buttons.at("weapon_confirm");
    auto & btn_weapon_cancel = buttons.at("weapon_cancel");
    auto & btn_cargo_confirm = buttons.at("cargo_confirm");
    auto & btn_cargo_cancel = buttons.at("cargo_cancel");
    auto & btn_dialog_yes = buttons.at("yes");
    auto & btn_dialog_no = buttons.at("no");

    auto & weapon_slot = text_inputs.at("weapon_slot");
    auto & buy_callsign = text_inputs.at("buy_callsign");
    auto & buy_captain = text_inputs.at("buy_captain");
    auto & cargo_weight = text_inputs.at("cargo_weight");
    auto & cargo_unload_ship = text_inputs.at("cargo_unload_ship");
    

    while (win.isOpen()) {

        place_main_menu();
        while (const auto ev = win.pollEvent()) {
            const sf::Event& e = *ev;

        if (e.getIf<sf::Event::Closed>() != nullptr) {
            win.close();
        }


        for (const auto & t : {"weapon_slot", "buy_callsign", "buy_captain", "cargo_weight", "cargo_unload_ship"}) {
            update_text_input(text_inputs.at(t), e);
        }



        if (const auto* wheel = e.getIf<sf::Event::MouseWheelScrolled>()) {
            const sf::Vector2f mp_wheel = win.mapPixelToCoords({wheel->position.x, wheel->position.y});
            if (mission_rect.contains(mp_wheel)) {
                convoy_info_scroll_lines -= static_cast<int>(wheel->delta);
            }
            if (pirate_rect.contains(mp_wheel)) {
                pirate_info_scroll_lines -= static_cast<int>(wheel->delta);
            }
        }

        if (const auto* mb = e.getIf<sf::Event::MouseButtonPressed>()) {
            
            if (mb->button != sf::Mouse::Button::Left) {
                continue;
            }
            

            const sf::Vector2f mp = win.mapPixelToCoords(mb->position);
            pressed_area = mp;

            if (click_sound_) {
                bool hit_any_button = false;
                for (const auto& [key, b] : buttons) {
                    (void)key;
                    if (b.hit(mp)) {
                        hit_any_button = true;
                        break;
                    }
                }
                if (hit_any_button) {
                    click_sound_->play();
                }
            }

            

            switch (screen) {
                case Screen::Start:
                    process_start_screen();
                    break;
                case Screen::MissionSelect:
                    process_mission_select_screen();
                    break;
                case Screen::Game:
                    process_game_screen();
                    break;
                default: break;
            }

            if (btn_exit_game.hit(mp)) {
                show_save_dialog = true;
            }

            refresh_text_input_states();
            switch (main_tab) {
                case MainTab::SHIPS:
                    if (btn_buy_ship.hit(mp)) {
                        aux_panel = (aux_panel == AuxPanel::SHIPBUY) ? AuxPanel::NONE : AuxPanel::SHIPBUY;
                        point_for_ship.reset();
                        buy_callsign.active = (aux_panel == AuxPanel::SHIPBUY);
                        buy_captain.active = false;
                    } else if (btn_sell_ship.hit(mp)) {
                        aux_panel = (aux_panel == AuxPanel::SHIPSELL) ? AuxPanel::NONE : AuxPanel::SHIPSELL;
                        point_for_ship.reset();
                        buy_callsign.active = buy_captain.active = false;

                        selected_sell_ship_.reset();
                        info_fields.at("sell_ship").set_value("(click on map)");
                    }
                    break;
                case MainTab::WEAPONS:
                    if (btn_set_weapon.hit(mp)) {
                        aux_panel = (aux_panel == AuxPanel::WEAPONSET) ? AuxPanel::NONE : AuxPanel::WEAPONSET;
                        weapon_set_type = "cannon";
                        weapon_set_place = "bow";

                        selected_weapon_ship_.reset();
                        info_fields.at("weapon_ship").set_value("(click on map)");

                        selected_unset_ship_.reset();
                        info_fields.at("unset_ship").set_value("(click on map)");
                    } else if (btn_unset_weapon.hit(mp)) {
                        aux_panel = (aux_panel == AuxPanel::WEAPONUNSET) ? AuxPanel::NONE : AuxPanel::WEAPONUNSET;
                        weapon_unset_place = "bow";

                        selected_unset_ship_.reset();
                        info_fields.at("unset_ship").set_value("(click on map)");

                        selected_weapon_ship_.reset();
                        info_fields.at("weapon_ship").set_value("(click on map)");
                    } else if (btn_reload_all.hit(mp)) {
                        const bool ok = presenter_.reload_all_imperial_weapons(100);
                        if (ok) {
                            refresh_state();
                            push_log(log, "Reloaded all weapons (-100$)");
                        } else {
                            push_log(log, "Reload failed (not enough money or nothing to reload)");
                        }
                    }
                    break;

                case MainTab::CARGO:
                    if (btn_cargo_auto.hit(mp)) {
                        const bool ok = mission_service_.auto_distribute_cargo();
                        refresh_state();
                        push_log(log, ok ? "Cargo distributed" : "Cargo distribute failed");
                    } else if (btn_cargo_load.hit(mp)) {
                        aux_panel = (aux_panel == AuxPanel::CARGOLOAD) ? AuxPanel::NONE : AuxPanel::CARGOLOAD;
                        selected_cargo_ship_.reset();
                        info_fields.at("cargo_ship").set_value("(click on map)");
                        cargo_weight.active = false;
                        cargo_unload_ship.active = false;
                    } else if (btn_cargo_unload.hit(mp)) {
                        aux_panel = (aux_panel == AuxPanel::CARGOUNLOAD) ? AuxPanel::NONE : AuxPanel::CARGOUNLOAD;
                        cargo_unload_ship.active = (aux_panel == AuxPanel::CARGOUNLOAD);
                        cargo_weight.active = false;
                    }
                    break;
                default:
                    break;
            }


            switch (aux_panel) {
                
                case AuxPanel::ATTACK: 
                    process_attack_auxpanel();
                    break;

                case AuxPanel::SHIPBUY: 
                    process_ship_buy_auxpanel();
                    break;
                case AuxPanel::SHIPSELL:
                    process_ship_sell_auxpanel();
                    break;  
                case AuxPanel::WEAPONSET:
                    process_weapon_buy_auxpanel();
                    break;
                case AuxPanel::WEAPONUNSET:
                    process_weapon_sell_auxpanel();
                    break;
                case AuxPanel::CARGOLOAD:
                    process_cargo_load_auxpanel();
                    break;
                case AuxPanel::CARGOUNLOAD:
                    process_cargo_unload_auxpanel();

                    break;
                default: break;
            }    
            
            if (show_mission_complete_dialog) {
                if (buttons.at("mission_complete_exit").hit(mp)) {
                    win.close();
                }
            }

            if (show_save_dialog) {
                if (btn_dialog_yes.hit(mp)) {
                    try {
                        presenter_.save_game_state(config_path_dir);
                        push_log(log, "Saved to: " + config_path_dir);
                        win.close();
                    } catch (const std::exception& e) {
                        push_log(log, std::string("Save failed: ") + e.what());
                        show_save_dialog = false;
                    }
                } else if (btn_dialog_no.hit(mp)) {
                    show_save_dialog = false;
                    win.close();
                }
            }
            }
        }


        if (screen == Screen::Start) {
            process_start_screen();
        } else if (screen == Screen::MissionSelect) {
            process_mission_select_screen();
        } else {
            draw_map(map_rect);
            draw_log_panel(log_rect, log);
            draw_main_menu(main_menu_rect, btn_attack, btn_ships, btn_weapons, btn_cargo, btn_end_turn, btn_exit_game);
            draw_imperial_ships_panel(mission_rect, convoy_info_scroll_lines);
            draw_pirates_panel(pirate_rect, pirate_info_scroll_lines);

            draw_panel_background(aux_rect, rgb(18, 18, 18), 0.f);
            draw_panel_background(aux_top_rect, rgb(18, 18, 18), 0.f);
            draw_panel_background(aux_bottom_rect, rgb(18, 18, 18), 0.f);

            draw_mission_info_panel(mission_info_rect);



            if (main_tab == MainTab::SHIPS) {
                draw_button(btn_buy_ship);
                draw_button(btn_sell_ship);
            } else if (main_tab == MainTab::WEAPONS) {
                draw_button(btn_set_weapon);
                draw_button(btn_unset_weapon);
                draw_button(btn_reload_all);
            } else if (main_tab == MainTab::CARGO) {
                draw_button(btn_cargo_auto);
                draw_button(btn_cargo_load);
                draw_button(btn_cargo_unload);
                
            }

            switch (aux_panel) {
                case AuxPanel::ATTACK: {
                    auto& f_attacker = info_fields.at("attack_attacker");
                    auto& f_target = info_fields.at("attack_target");
                    draw_label("Attacker (imperial)", f_attacker.rect);
                    draw_info_field(f_attacker);
                    draw_label("Target (pirate)", f_target.rect);
                    draw_info_field(f_target);
                    draw_button(btn_bow);
                    draw_button(btn_stern);
                    draw_button(btn_starboard);
                    draw_button(btn_portside);
                    draw_button(btn_do_attack);

                    if (gameLoaded_) {
                        auto lower = [](std::string s) {
                            std::transform(s.begin(), s.end(), s.begin(), ::tolower);
                            return s;
                        };
                        const std::string attacker = selected_attack_attacker_.value_or(std::string{});
                        const std::string slot = lower(weapon_slot.value);

                        std::string weapon_text = "Weapon: none";
                        std::string ammo_text = "Ammunition: -";

                        const ShipDto* ship = nullptr;
                        for (const auto& s : currentGameState_.ships) {
                            if (s.callsign == attacker) {
                                ship = &s;
                                break;
                            }
                        }

                        if (ship) {
                            auto it = ship->params.find(slot);
                            if (it != ship->params.end()) {
                                unsigned wid = 0;
                                try {
                                    wid = static_cast<unsigned>(std::stoul(it->second));
                                } catch (...) {
                                    wid = 0;
                                }
                                if (wid != 0) {
                                    const WeaponDto* w = nullptr;
                                    for (const auto& ww : currentGameState_.weapons) {
                                        if (ww.id == wid) {
                                            w = &ww;
                                            break;
                                        }
                                    }
                                    if (w) {
                                        weapon_text = "Weapon: " + lower(w->type);
                                        auto itp = currentGameState_.weapons_params.find(w->type);
                                        if (itp != currentGameState_.weapons_params.end()) {
                                            ammo_text = "Ammunition: " + std::to_string(w->current_ammunition) + "/" + std::to_string(itp->second.max_ammunition);
                                        } else {
                                            ammo_text = "Ammunition: " + std::to_string(w->current_ammunition) + "/?";
                                        }
                                    }
                                }
                            }
                        }

                        const float info_x = aux_bottom_rect.position.x + 10.f;
                        const float info_y = aux_bottom_rect.position.y + 10.f;
                        draw_label(weapon_text, {info_x, info_y});
                        draw_label(ammo_text, {info_x, info_y + 20.f});
                    }
                    break;
                }
                case AuxPanel::SHIPBUY:
                    draw_ship_buy_panel();
                    break;
                case AuxPanel::SHIPSELL:
                    draw_ship_sell_panel();
                    break;
                case AuxPanel::WEAPONSET:
                    {
                        auto& f_ship = info_fields.at("weapon_ship");
                        draw_label("Ship (imperial)", f_ship.rect);
                        draw_info_field(f_ship);
                    }

                    for (auto & s: {"icon_cannon_sprite", "icon_missile_sprite", "icon_torpedo_sprite"}) {
                        win.draw(sprites.at(s));
                    }
                    for (auto & r : {"icon_cannon_frame", "icon_missile_frame", "icon_torpedo_frame"}) {
                        win.draw(rectshapes.at(r));
                    }

                    if (gameLoaded_) {
                        auto draw_weapon_cost = [&](const std::string& type, const std::string& frame_key) {
                            auto it = currentGameState_.weapons_params.find(type);
                            if (it == currentGameState_.weapons_params.end()) {
                                return;
                            }
                            const auto& frame = rectshapes.at(frame_key);

                            sf::Text t(font);
                            t.setCharacterSize(12);
                            t.setFillColor(rgb(240, 240, 240));
                            t.setString("$" + std::to_string(it->second.cost));

                            const sf::FloatRect b = t.getLocalBounds();
                            t.setOrigin(sf::Vector2f{b.position.x + b.size.x / 2.f, b.position.y});
                            t.setPosition(sf::Vector2f{
                                frame.getPosition().x + frame.getSize().x / 2.f,
                                frame.getPosition().y + frame.getSize().y + 2.f + 10.f,
                            });
                            win.draw(t);
                        };

                        draw_weapon_cost("cannon", "icon_cannon_frame");
                        draw_weapon_cost("missile", "icon_missile_frame");
                        draw_weapon_cost("torpedo", "icon_torpedo_frame");
                    }

                    draw_label("Place", {btn_bow.rect.getPosition().x, btn_bow.rect.getPosition().y - 18.f});
                    draw_button(btn_bow);
                    draw_button(btn_stern);
                    draw_button(btn_portside);
                    draw_button(btn_starboard);

                    {
                        const float info_x = aux_bottom_rect.position.x + 10.f;
                        const float info_y = aux_bottom_rect.position.y + 15.f;
                        draw_label("Weapon: " + weapon_set_type, {info_x, info_y});
                        draw_label("Place: " + weapon_set_place, {info_x, info_y + 20.f});
                    }

                    draw_button(btn_weapon_confirm);
                    draw_button(btn_weapon_cancel);
                    break;
                case AuxPanel::WEAPONUNSET:
                    {
                        auto& f_ship = info_fields.at("unset_ship");
                        draw_label("Ship (imperial)", f_ship.rect);
                        draw_info_field(f_ship);
                    }

                    draw_label("Place", {btn_bow.rect.getPosition().x, btn_bow.rect.getPosition().y - 18.f});
                    draw_button(btn_bow);
                    draw_button(btn_stern);
                    draw_button(btn_portside);
                    draw_button(btn_starboard);

                    {
                        std::string weapon_text = "none";
                        if (gameLoaded_ && selected_unset_ship_.has_value()) {
                            const ShipDto* ship = nullptr;
                            for (const auto& s : currentGameState_.ships) {
                                if (s.callsign == *selected_unset_ship_) {
                                    ship = &s;
                                    break;
                                }
                            }
                            if (ship) {
                                auto it = ship->params.find(weapon_unset_place);
                                if (it != ship->params.end()) {
                                    try {
                                        const unsigned wid = static_cast<unsigned>(std::stoul(it->second));
                                        for (const auto& w : currentGameState_.weapons) {
                                            if (w.id == wid) {
                                                weapon_text = lower_string(w.type);
                                                break;
                                            }
                                        }
                                    } catch (...) {
                                        weapon_text = "none";
                                    }
                                }
                            }
                        }

                        const float info_x = aux_bottom_rect.position.x + 10.f;
                        const float info_y = aux_bottom_rect.position.y + 44.f;
                        draw_label("Place: " + weapon_unset_place, {info_x, info_y});
                        draw_label("Weapon: " + weapon_text, {info_x, info_y + 20.f});
                    }

                    draw_button(btn_weapon_confirm);
                    draw_button(btn_weapon_cancel);
                    break;
                case AuxPanel::CARGOLOAD:
                    {
                        auto& f_ship = info_fields.at("cargo_ship");
                        draw_label("Ship (imperial)", f_ship.rect);
                        draw_info_field(f_ship);
                    }

                    draw_label("Cargo weight", cargo_weight.rect);
                    draw_input(cargo_weight);
                    draw_button(btn_cargo_confirm);
                    draw_button(btn_cargo_cancel);
                    break;
                case AuxPanel::CARGOUNLOAD:
                    draw_label("Ship callsign", cargo_unload_ship.rect);
                    draw_input(cargo_unload_ship);
                    draw_button(btn_cargo_confirm);
                    draw_button(btn_cargo_cancel);
                    break;
                default:
                    break;
            }
        }

        if (show_save_dialog) {
            sf::Vector2u window_size = win.getSize();

            const sf::RectangleShape& box = rectshapes.at("box");
            const sf::Vector2f box_pos = box.getPosition();
            const sf::Vector2f box_size = box.getSize();

            const float btn_width = 100.f;
            const float btn_height = 40.f;
            const float btn_gap = 20.f;
            const float total_btns_width = btn_width * 2 + btn_gap;
            const float btn_start_x = box_pos.x + (box_size.x - total_btns_width) / 2.f;
            const float btn_y = box_pos.y + box_size.y - btn_height - 15.f;

            buttons.at("yes").rect.setPosition(sf::Vector2f{btn_start_x, btn_y});
            sf::FloatRect yes_text_bounds = buttons.at("yes").label.getLocalBounds();
            buttons.at("yes").label.setOrigin(
                sf::Vector2f{yes_text_bounds.position.x + yes_text_bounds.size.x / 2.f,
                           yes_text_bounds.position.y + yes_text_bounds.size.y / 2.f}
            );
            buttons.at("yes").label.setPosition(
                sf::Vector2f{btn_start_x + btn_width / 2.f, btn_y + btn_height / 2.f}
            );

            buttons.at("no").rect.setPosition(sf::Vector2f{btn_start_x + btn_width + btn_gap, btn_y});
            sf::FloatRect no_text_bounds = buttons.at("no").label.getLocalBounds();
            buttons.at("no").label.setOrigin(
                sf::Vector2f{no_text_bounds.position.x + no_text_bounds.size.x / 2.f,
                           no_text_bounds.position.y + no_text_bounds.size.y / 2.f}
            );
            buttons.at("no").label.setPosition(
                sf::Vector2f{btn_start_x + btn_width + btn_gap + btn_width / 2.f, btn_y + btn_height / 2.f}
            );

            win.draw(rectshapes.at("overlay"));         
            win.draw(rectshapes.at("box"));

            sf::Text t(font);
            t.setCharacterSize(25);
            t.setFillColor(sf::Color::White);
            t.setString("Save game before exit?");
            sf::FloatRect text_bounds = t.getLocalBounds();
            t.setOrigin(sf::Vector2f{text_bounds.position.x + text_bounds.size.x / 2.f,
                                     text_bounds.position.y + text_bounds.size.y / 2.f});
            t.setPosition(sf::Vector2f{box_pos.x + box_size.x / 2.f,
                                       box_pos.y + 50.f});
            win.draw(t);

            draw_button(buttons.at("yes"));
            draw_button(buttons.at("no"));
        }

        if (show_mission_complete_dialog) {
            sf::Vector2u window_size = win.getSize();

            const sf::RectangleShape& box = rectshapes.at("box");
            const sf::Vector2f box_pos = box.getPosition();
            const sf::Vector2f box_size = box.getSize();

            const float btn_width = 120.f;
            const float btn_height = 50.f;
            const float btn_x = box_pos.x + (box_size.x - btn_width) / 2.f;
            const float btn_y = box_pos.y + box_size.y - btn_height - 20.f;

            buttons.at("mission_complete_exit").rect.setPosition(sf::Vector2f{btn_x, btn_y});
            buttons.at("mission_complete_exit").rect.setSize(sf::Vector2f{btn_width, btn_height});
            sf::FloatRect exit_text_bounds = buttons.at("mission_complete_exit").label.getLocalBounds();
            buttons.at("mission_complete_exit").label.setOrigin(
                sf::Vector2f{exit_text_bounds.position.x + exit_text_bounds.size.x / 2.f,
                           exit_text_bounds.position.y + exit_text_bounds.size.y / 2.f}
            );
            buttons.at("mission_complete_exit").label.setPosition(
                sf::Vector2f{btn_x + btn_width / 2.f, btn_y + btn_height / 2.f}
            );

            win.draw(rectshapes.at("overlay"));         
            win.draw(rectshapes.at("box"));

            sf::Text t(font);
            t.setCharacterSize(32);
            t.setFillColor(sf::Color(0, 200, 0));
            t.setString("MISSION COMPLETE!");
            sf::FloatRect text_bounds = t.getLocalBounds();
            t.setOrigin(sf::Vector2f{text_bounds.position.x + text_bounds.size.x / 2.f,
                                     text_bounds.position.y + text_bounds.size.y / 2.f});
            t.setPosition(sf::Vector2f{box_pos.x + box_size.x / 2.f,
                                       box_pos.y + 45.f});
            win.draw(t);

            sf::Text stats(font);
            stats.setCharacterSize(18);
            stats.setFillColor(sf::Color::White);
            std::string stats_text = "Cargo delivered: " + std::to_string(currentGameState_.mission.delivered_weight) + "/" + std::to_string(currentGameState_.mission.goal_weight);
            stats.setString(stats_text);
            sf::FloatRect stats_bounds = stats.getLocalBounds();
            stats.setOrigin(sf::Vector2f{stats_bounds.position.x + stats_bounds.size.x / 2.f,
                                         stats_bounds.position.y + stats_bounds.size.y / 2.f});
            stats.setPosition(sf::Vector2f{box_pos.x + box_size.x / 2.f,
                                            box_pos.y + 85.f});
            win.draw(stats);

            draw_button(buttons.at("mission_complete_exit"));
        }

        win.display();
    }
}

void SfmlGameView::process_attack_auxpanel() {
    const sf::Vector2f mp = pressed_area;
    auto& btn_bow = buttons.at("bow");
    auto& btn_stern = buttons.at("stern");
    auto& btn_portside = buttons.at("portside");
    auto& btn_starboard = buttons.at("starboard");
    auto& btn_do_attack = buttons.at("do_attack");
    auto& weapon_slot = text_inputs.at("weapon_slot");

    if (gameLoaded_ && map_rect.contains(mp)) {
        if (attack_pick_stage == AttackPickStage::Attacker) {
            auto cs = pick_ship_callsign_on_map(mp, /*require_pirate=*/false);
            if (!cs) {
                push_log(log, "Select an imperial ship (attacker) on the map");
                return;
            }
            selected_attack_attacker_ = *cs;
            info_fields.at("attack_attacker").set_value(*cs);
            attack_pick_stage = AttackPickStage::Target;
            push_log(log, "Attacker selected: " + *cs + ". Now click a pirate ship.");
            return;
        }

        auto cs = pick_ship_callsign_on_map(mp, /*require_pirate=*/true);
        if (!cs) {
            push_log(log, "Select a pirate ship (target) on the map");
            return;
        }
        selected_attack_target_ = *cs;
        info_fields.at("attack_target").set_value(*cs);
        attack_pick_stage = AttackPickStage::Attacker;
        push_log(log, "Target selected: " + *cs);
        return;
    }

    if (btn_bow.hit(mp)) {
        weapon_slot.value = "bow";
        weapon_slot.text.setString(weapon_slot.value);
        return;
    }
    if (btn_stern.hit(mp)) {
        weapon_slot.value = "stern";
        weapon_slot.text.setString(weapon_slot.value);
        return;
    }
    if (btn_portside.hit(mp)) {
        weapon_slot.value = "portside";
        weapon_slot.text.setString(weapon_slot.value);
        return;
    }
    if (btn_starboard.hit(mp)) {
        weapon_slot.value = "starboard";
        weapon_slot.text.setString(weapon_slot.value);
        return;
    }

    if (btn_do_attack.hit(mp)) {
        const auto place = weapon_place_from_ui(weapon_slot.value);
        if (!place) {
            push_log(log, "Attack failed: invalid weapon slot (use bow/stern/portside/starboard)");
            return;
        }

        if (!selected_attack_attacker_.has_value() || !selected_attack_target_.has_value()) {
            push_log(log, "Attack failed: select attacker (imperial) and target (pirate) by clicking on the map");
            return;
        }

        const bool ok = mission_service_.attack(*selected_attack_attacker_, *place, *selected_attack_target_, false);
        refresh_state();
        push_log(log, ok ? "Attack executed" : "Attack failed (no weapon / invalid attacker/ no ammunition)");
        return;
    }
}


void SfmlGameView::process_ship_buy_auxpanel() {
    const sf::Vector2f mp = pressed_area;
    auto& btn_fleet_cancel = buttons.at("fleet_cancel");
    auto& btn_fleet_confirm = buttons.at("fleet_confirm");
    auto& buy_callsign = text_inputs.at("buy_callsign");
    auto& buy_captain = text_inputs.at("buy_captain");

    if (gameLoaded_ && map_rect.contains(mp)) {
        point_for_ship = get_map_point();
        return;
    }

    if (rectshapes.at("icon_military_frame").getGlobalBounds().contains(mp)) {
        buy_ship_type = "military";
    } else if (rectshapes.at("icon_transport_frame").getGlobalBounds().contains(mp)) {
        buy_ship_type = "transport";
    } else if (rectshapes.at("icon_tm_frame").getGlobalBounds().contains(mp)) {
        buy_ship_type = "transport_military";
    } else if (btn_fleet_cancel.hit(mp)) {
        aux_panel = AuxPanel::NONE;
        point_for_ship.reset();
        buy_callsign.active = buy_captain.active = false;
    } else if (btn_fleet_confirm.hit(mp)) {
        if (buy_callsign.value.empty() || buy_captain.value.empty()) {
            push_log(log, "Buy failed: callsign and captain are required");
        } else if (!point_for_ship) {
            push_log(log, "Buy failed: select a point on the map first");
        } else {
            const auto cost_opt = ship_cost_by_type(buy_ship_type);
            if (!cost_opt) {
                push_log(log, "Buy failed: unknown ship type");
            } else if (*cost_opt > money_left()) {
                push_log(log, "Buy failed: not enough money (need " + std::to_string(*cost_opt) + ")");
            } else {
                ShipDto ship;
                ship.type = buy_ship_type;
                ship.callsign = buy_callsign.value;
                ship.captain = buy_captain.value;
                ship.current_velocity = currentGameState_.ships_params.at(buy_ship_type).max_velocity;
                ship.current_HP = currentGameState_.ships_params.at(buy_ship_type).max_HP;
                ship.params = {};

                
                try {
                    presenter_.buy_ship(ship, *point_for_ship);
                    refresh_state();

                    const bool added = std::any_of(currentGameState_.ship_units.begin(), currentGameState_.ship_units.end(),
                                                  [&](const ShipUnitDTO& u) { return u.callsign == ship.callsign; });
                    if (added) {
                        push_log(log, "Bought ship: " + ship.callsign + " (" + ship.type + ")");
                        buy_callsign.value.clear();
                        buy_captain.value.clear();
                        aux_panel = AuxPanel::NONE;
                        point_for_ship.reset();
                        buy_callsign.active = buy_captain.active = false;
                    } else {
                        push_log(log, "Buy failed: ship was not placed (point occupied / callsign exists / restricted by pirates)");
                    }
                } catch (const std::exception& e2) {
                    push_log(log, std::string("Buy failed: ") + e2.what());
                }
            }
        }
    }
};

void SfmlGameView::process_ship_sell_auxpanel() {
    const sf::Vector2f mp = pressed_area;
    auto& btn_fleet_cancel = buttons.at("fleet_cancel");
    auto& btn_fleet_confirm = buttons.at("fleet_confirm");

    if (gameLoaded_ && map_rect.contains(mp)) {
        auto cs = pick_ship_callsign_on_map(mp, false);
        if (!cs) {
            push_log(log, "Select an imperial ship to sell (click on ship icon)");
            return;
        }
        selected_sell_ship_ = *cs;
        info_fields.at("sell_ship").set_value(*cs);
        return;
    }

    if (btn_fleet_cancel.hit(mp)) {
        aux_panel = AuxPanel::NONE;
        selected_sell_ship_.reset();
        info_fields.at("sell_ship").set_value("(click on map)");
    } else if (btn_fleet_confirm.hit(mp)) {
        if (!selected_sell_ship_.has_value()) {
            push_log(log, "Sell failed: select an imperial ship on the map");
        } else {
            const bool ok = presenter_.sell_ship(*selected_sell_ship_);
            
            refresh_state();
            push_log(log, ok ? ("Sold ship: " + *selected_sell_ship_) : "Sell failed: ship not found");
            aux_panel = AuxPanel::NONE;
            selected_sell_ship_.reset();
            info_fields.at("sell_ship").set_value("(click on map)");
        }
    }
};

void SfmlGameView::process_weapon_buy_auxpanel() {
    const sf::Vector2f mp = pressed_area;
    auto& btn_weapon_cancel = buttons.at("weapon_cancel");
    auto& btn_weapon_confirm = buttons.at("weapon_confirm");

    if (gameLoaded_ && map_rect.contains(mp)) {
        auto cs = pick_ship_callsign_on_map(mp, false);
        if (!cs) {
            push_log(log, "Select an imperial ship to set weapon (click on ship icon)");
            return;
        }
        selected_weapon_ship_ = *cs;
        info_fields.at("weapon_ship").set_value(*cs);
        return;
    }

    if (rectshapes.at("icon_cannon_frame").getGlobalBounds().contains(mp)) {
        weapon_set_type = "cannon";
        return;
    }
    if (rectshapes.at("icon_missile_frame").getGlobalBounds().contains(mp)) {
        weapon_set_type = "missile";
        return;
    }
    if (rectshapes.at("icon_torpedo_frame").getGlobalBounds().contains(mp)) {
        weapon_set_type = "torpedo";
        return;
    }

    if (buttons.at("bow").hit(mp)) {
        weapon_set_place = "bow";
        return;
    }
    if (buttons.at("stern").hit(mp)) {
        weapon_set_place = "stern";
        return;
    }
    if (buttons.at("portside").hit(mp)) {
        weapon_set_place = "portside";
        return;
    }
    if (buttons.at("starboard").hit(mp)) {
        weapon_set_place = "starboard";
        return;
    }


    if (btn_weapon_cancel.hit(mp)) {
        aux_panel = AuxPanel::NONE;
        selected_weapon_ship_.reset();
        info_fields.at("weapon_ship").set_value("(click on map)");
    } else if (btn_weapon_confirm.hit(mp)) {
        if (!selected_weapon_ship_.has_value()) {
            push_log(log, "Set weapon failed: select an imperial ship on the map");
            return;
        }

        WeaponDto weapon;
        weapon.type = weapon_set_type;
        weapon.id = 0;
        weapon.current_ammunition = 0;
        
        const bool ok = presenter_.set_weapon_from_ship(weapon, *selected_weapon_ship_, weapon_set_place);
        refresh_state();

        unsigned chosen_id = 0;
        if (ok && gameLoaded_) {
            const ShipDto* ship = nullptr;
            for (const auto& s : currentGameState_.ships) {
                if (s.callsign == *selected_weapon_ship_) {
                    ship = &s;
                    break;
                }
            }
            if (ship) {
                auto it = ship->params.find(weapon_set_place);
                if (it == ship->params.end()) {
                    it = ship->params.find("weapon_" + weapon_set_place);
                }
                if (it != ship->params.end()) {
                    try {
                        chosen_id = static_cast<unsigned>(std::stoul(it->second));
                    } catch (...) {
                        chosen_id = 0;
                    }
                }
            }
        }

        if (ok) {
            if (chosen_id != 0) {
                push_log(log, "Weapon bought: #" + std::to_string(chosen_id) + " (" + weapon_set_type + ")");
            } else {
                push_log(log, "Weapon bought: (" + weapon_set_type + ")");
            }
        } else {
            push_log(log, "Weapon buy failed");
        }
        aux_panel = AuxPanel::NONE;
        selected_weapon_ship_.reset();
        info_fields.at("weapon_ship").set_value("(click on map)");
    }
};
void SfmlGameView::process_weapon_sell_auxpanel() {
    const sf::Vector2f mp = pressed_area;
    auto& btn_weapon_cancel = buttons.at("weapon_cancel");
    auto& btn_weapon_confirm = buttons.at("weapon_confirm");

    if (gameLoaded_ && map_rect.contains(mp)) {
        auto cs = pick_ship_callsign_on_map(mp, /*require_pirate=*/false);
        if (!cs) {
            push_log(log, "Select an imperial ship to unset weapon (click on ship icon)");
            return;
        }
        selected_unset_ship_ = *cs;
        info_fields.at("unset_ship").set_value(*cs);
        return;
    }

    if (buttons.at("bow").hit(mp)) {
        weapon_unset_place = "bow";
        return;
    }
    if (buttons.at("stern").hit(mp)) {
        weapon_unset_place = "stern";
        return;
    }
    if (buttons.at("portside").hit(mp)) {
        weapon_unset_place = "portside";
        return;
    }
    if (buttons.at("starboard").hit(mp)) {
        weapon_unset_place = "starboard";
        return;
    }

    if (btn_weapon_cancel.hit(mp)) {
        aux_panel = AuxPanel::NONE;
        selected_unset_ship_.reset();
        info_fields.at("unset_ship").set_value("(click on map)");
    } else if (btn_weapon_confirm.hit(mp)) {
        if (!selected_unset_ship_.has_value()) {
            push_log(log, "Unset weapon failed: select an imperial ship on the map");
            return;
        }
        const bool ok = presenter_.remove_weapon_from_ship(*selected_unset_ship_, weapon_unset_place);
        refresh_state();
        push_log(log, ok ? "Weapon removed" : "Weapon remove failed");
        aux_panel = AuxPanel::NONE;
        selected_unset_ship_.reset();
        info_fields.at("unset_ship").set_value("(click on map)");
    }
};
void SfmlGameView::process_cargo_load_auxpanel() {
    const sf::Vector2f mp = pressed_area;
    auto& btn_cargo_cancel = buttons.at("cargo_cancel");
    auto& btn_cargo_confirm = buttons.at("cargo_confirm");
    auto& cargo_weight = text_inputs.at("cargo_weight");

    if (gameLoaded_ && map_rect.contains(mp)) {
        auto cs = pick_ship_callsign_on_map(mp, /*require_pirate=*/false);
        if (!cs) {
            push_log(log, "Select an imperial ship to load cargo (click on ship icon)");
            return;
        }
        selected_cargo_ship_ = *cs;
        info_fields.at("cargo_ship").set_value(*cs);
        return;
    }

    if (btn_cargo_cancel.hit(mp)) {
        aux_panel = AuxPanel::NONE;
        selected_cargo_ship_.reset();
        info_fields.at("cargo_ship").set_value("(click on map)");
        cargo_weight.active = false;
    } else if (btn_cargo_confirm.hit(mp)) {
        if (!selected_cargo_ship_.has_value() || cargo_weight.value.empty()) {
            push_log(log, "Load cargo failed: select ship (click on map) and enter weight");
        } else {
            size_t w = 0;
            try {
                w = static_cast<size_t>(std::stoull(cargo_weight.value));
            } catch (...) {
                push_log(log, "Load cargo failed: weight must be a number");
                return;
            }

            const bool ok = mission_service_.load_cargo(*selected_cargo_ship_, w, w * 1.5);
            refresh_state();
            push_log(log, ok ? "Cargo loaded" : "Cargo load failed");
            aux_panel = AuxPanel::NONE;
            selected_cargo_ship_.reset();
            info_fields.at("cargo_ship").set_value("(click on map)");
            cargo_weight.active = false;
        }
    }
};
void SfmlGameView::process_cargo_unload_auxpanel() {
    const sf::Vector2f mp = pressed_area;
    auto& btn_cargo_cancel = buttons.at("cargo_cancel");
    auto& btn_cargo_confirm = buttons.at("cargo_confirm");
    auto& cargo_unload_ship = text_inputs.at("cargo_unload_ship");
    if (btn_cargo_cancel.hit(mp)) {
        aux_panel = AuxPanel::NONE;
        cargo_unload_ship.active = false;
    } else if (btn_cargo_confirm.hit(mp)) {
        if (cargo_unload_ship.value.empty()) {
            push_log(log, "Unload cargo failed: ship is required");
        } else {
            const bool ok = mission_service_.unload_cargo(cargo_unload_ship.value);
            refresh_state();
            push_log(log, ok ? "Cargo unloaded" : "Cargo unload failed");
            aux_panel = AuxPanel::NONE;
            cargo_unload_ship.active = false;
        }
    }
};

void SfmlGameView::draw_label(const std::string& s, const sf::Vector2f & pos) {
    sf::Text t(font);
    t.setCharacterSize(13);
    t.setFillColor(rgb(220, 220, 220));
    t.setString(s);
    t.setPosition(pos);
    win.draw(t);           
}

void SfmlGameView::draw_label(const std::string& s, const sf::RectangleShape& r) {
    draw_label(s, {r.getPosition().x, r.getPosition().y - 18.f});
};

void SfmlGameView::draw_input(TextInput& ti) {
    ti.rect.setOutlineColor(ti.active ? sf::Color::White : rgb(180, 180, 180));
    win.draw(ti.rect);
    win.draw(ti.text);
};

void SfmlGameView::draw_info_field(InfoField& fi) {
    fi.rect.setOutlineColor(rgb(180, 180, 180));
    win.draw(fi.rect);
    win.draw(fi.text);
}

void SfmlGameView::draw_button(Button& b) {
    win.draw(b.rect);
    win.draw(b.label);
};

void SfmlGameView::draw_ship_buy_panel() {
    auto & icon_military_frame = rectshapes.at("icon_military_frame");
    auto & buy_captain = text_inputs.at("buy_captain");
    auto & buy_callsign = text_inputs.at("buy_callsign");

    draw_label("Choose ship type", {icon_military_frame.getPosition().x, icon_military_frame.getPosition().y - 18.f});
    
    for (auto & s: {"icon_military_sprite", "icon_transport_sprite", "icon_tm_sprite"}) {
        win.draw(sprites.at(s));
    }
    for (auto & r : {"icon_military_frame", "icon_transport_frame", "icon_tm_frame"}) {
        win.draw(rectshapes.at(r));
    }

    
    if (gameLoaded_) {
        auto draw_cost = [&](const std::string& type, const std::string& frame_key) {
            auto it = currentGameState_.ships_params.find(type);
            if (it == currentGameState_.ships_params.end()) {
                return;
            }
            const auto& frame = rectshapes.at(frame_key);

            sf::Text t(font);
            t.setCharacterSize(12);
            t.setFillColor(rgb(240, 240, 240));
            t.setString("$" + std::to_string(it->second.cost));

            const sf::FloatRect b = t.getLocalBounds();
            t.setOrigin(sf::Vector2f{b.position.x + b.size.x / 2.f, b.position.y});
            t.setPosition(sf::Vector2f{
                frame.getPosition().x + frame.getSize().x / 2.f,
                frame.getPosition().y + frame.getSize().y + 2.f + 10.f,
            });
            win.draw(t);
        };

        draw_cost("military", "icon_military_frame");
        draw_cost("transport", "icon_transport_frame");
        draw_cost("transport_military", "icon_tm_frame");
    }
    
    draw_label("Callsign", buy_callsign.rect);
    draw_input(buy_callsign);
    draw_label("Captain", buy_captain.rect);
    draw_input(buy_captain);

    {
        std::string s = "Selected point: (none)";
        if (point_for_ship) {
            s = "Selected point: (" + std::to_string(point_for_ship->get_x()) + ", " + std::to_string(point_for_ship->get_y()) + ")";
        }
        const float info_y = buttons.at("fleet_confirm").rect.getPosition().y - 26.f;
        draw_label(s, {buy_callsign.rect.getPosition().x, info_y});
    }

    draw_button(buttons.at("fleet_confirm"));
    draw_button(buttons.at("fleet_cancel"));
}

void SfmlGameView::draw_ship_sell_panel() {
    auto & field = info_fields.at("sell_ship");
    draw_label("Ship (imperial)", field.rect);
    draw_info_field(field);
    draw_button(buttons.at("fleet_confirm"));
    draw_button(buttons.at("fleet_cancel"));
}

void SfmlGameView::update_game_state(const GameStateDto& game_state) {
    currentGameState_ = game_state;
}

bool SfmlGameView::is_game_loaded() const {
    return gameLoaded_;
}

// ============== реализация внутренних функций =====================

void SfmlGameView::push_log(std::vector<std::string>& log, std::string s) {
    log.push_back(std::move(s));
    if (log.size() > 200) {
        log.erase(log.begin(), log.begin() + 50);
    }
}

void SfmlGameView::place_button(Button& b, sf::Vector2f pos, sf::Vector2f size) {
    b.rect.setPosition(pos);
    b.rect.setSize(size);
    sf::FloatRect text_bounds = b.label.getLocalBounds();
    b.label.setOrigin(
        { text_bounds.position.x + text_bounds.size.x / 2.f,
        text_bounds.position.y + text_bounds.size.y / 2.f}
    );
    b.label.setPosition(
        {pos.x + size.x / 2.f,
        pos.y + size.y / 2.f}
    );
    
}


void SfmlGameView::place_text_input(TextInput& ti, sf::Vector2f pos, sf::Vector2f size) {
    ti.rect.setPosition(pos);
    ti.rect.setSize(size);
    ti.text.setPosition(sf::Vector2f{pos.x + 8.f, pos.y + 6.f});
    ti.text.setString(ti.value);
}

void SfmlGameView::place_info_field(InfoField& fi, sf::Vector2f pos, sf::Vector2f size) {
    fi.rect.setPosition(pos);
    fi.rect.setSize(size);
    fi.text.setPosition(sf::Vector2f{pos.x + 8.f, pos.y + 6.f});
    fi.text.setString(fi.value);
}

void SfmlGameView::refresh_state() {
    if (!presenter_.is_game_loaded()) {
        return;
    }
    presenter_.end_turn();
    currentGameState_ = presenter_.get_game_state();
}

std::optional<Place> SfmlGameView::weapon_place_from_ui(const std::string& weapon_slot_value) {
    const std::string s = lower_string(weapon_slot_value);
    if (s == "bow" || s == "front") {
        return BOW;
    }
    if (s == "stern" || s == "back") {
        return STERN;
    }
    if (s == "portside" || s == "port" || s == "left") {
        return PORTSIDE;
    }
    if (s == "starboard" || s == "star" || s == "right") {
        return STARBOARD;
    }
    return std::nullopt;
}

std::optional<unsigned> SfmlGameView::ship_cost_by_type(const std::string& type) {
    auto it = currentGameState_.ships_params.find(type);
    if (it == currentGameState_.ships_params.end()) {
        return std::nullopt;
    }
    return it->second.cost;
}

unsigned SfmlGameView::money_left() {
    if (currentGameState_.mission.budget >= currentGameState_.mission.spent_budget) {
        return currentGameState_.mission.budget - currentGameState_.mission.spent_budget;
    }
    return 0;
}

void SfmlGameView::try_load_game(const std::string & path, std::vector<std::string>& log, Screen& screen) {
    try {
        presenter_.load_game_state(path);
        gameLoaded_= true;
        refresh_state();
        push_log(log, "Loaded: " + path);
        screen = Screen::Game;
    } catch (const std::exception& e) {
        gameLoaded_= false;
        push_log(log, std::string("Load failed: ") + e.what());
    }
}

void SfmlGameView::draw_log_panel(const sf::FloatRect& log_rect, const std::vector<std::string>& log) {
    sf::RectangleShape bg;
    bg.setPosition(log_rect.position);
    bg.setSize(log_rect.size);
    bg.setFillColor(rgb(20, 20, 20));
    bg.setOutlineColor(rgb(90, 90, 90));
    bg.setOutlineThickness(1.f);
    win.draw(bg);

    sf::Text t(font);
    t.setCharacterSize(14);
    t.setFillColor(sf::Color::White);

    float y = log_rect.position.y + 8.f;
    const int max_lines = static_cast<int>((log_rect.size.y - 16.f) / 18.f);
    const int start = (static_cast<int>(log.size()) > max_lines) ? static_cast<int>(log.size()) - max_lines : 0;
    for (int i = start; i < static_cast<int>(log.size()); ++i) {
        t.setString(log[static_cast<size_t>(i)]);
        t.setPosition({log_rect.position.x + 10.f, y});
        win.draw(t);
        y += 18.f;
    }
}

void SfmlGameView::extend_bounds(bool& have, int& min_x, int& max_x, int& min_y, int& max_y, int x, int y) {
    if (!have) {
        have = true;
        min_x = max_x = x;
        min_y = max_y = y;
        return;
    }
    min_x = std::min(min_x, x);
    max_x = std::max(max_x, x);
    min_y = std::min(min_y, y);
    max_y = std::max(max_y, y);
}

sf::Vector2f SfmlGameView::to_screen_coords(float x, float y, float world_min_x, float world_max_x, 
                                            float world_min_y, float world_max_y, const sf::FloatRect& map_rect) {
    const float nx = (x - world_min_x) / (world_max_x - world_min_x);
    const float ny = (y - world_min_y) / (world_max_y - world_min_y);
    return sf::Vector2f{
        map_rect.position.x + nx * map_rect.size.x,
        map_rect.position.y + (1.f - ny) * map_rect.size.y,
    };
}

sf::Color SfmlGameView::get_ship_color_by_callsign(const std::string& callsign) {

    const auto& ship_units = currentGameState_.ship_units;
    auto it_unit = std::find_if(ship_units.begin(), ship_units.end(),
                                [&](const ShipUnitDTO& u) { return u.callsign == callsign; });
    if (it_unit != ship_units.end()) {
        if (is_strings_equal(it_unit->side, "pirate") || is_strings_equal(it_unit->side, "pirates")) {
            return rgb(228, 26, 28);
        }
    }

    std::string type;
    for (const auto& s : currentGameState_.ships) {
        if (s.callsign == callsign) {
            type = s.type;
            break;
        }
    }
    const std::string t = lower_string(type);
    if (t == "military") {
        return rgb(77, 175, 74);
    }
    if (t == "transport") {
        return rgb(55, 126, 184);
    }
    if (t == "transport_military" || t == "transportmilitary" || t == "tm") {
        return rgb(152, 78, 163);
    }

    return sf::Color::White;
}


void SfmlGameView::draw_map(const sf::FloatRect& map_rect) {
    if (sprites.find("map_bg_sprite") != sprites.end()) {
        auto& map_bg = sprites.at("map_bg_sprite");
        const sf::FloatRect bounds = map_bg.getLocalBounds();
        const float scale_x = map_rect.size.x / bounds.size.x;
        const float scale_y = map_rect.size.y / bounds.size.y;
        map_bg.setScale(sf::Vector2f{scale_x, scale_y});
        map_bg.setPosition(map_rect.position);
        win.draw(map_bg);
    } else {
        sf::RectangleShape bg;
        bg.setPosition(map_rect.position);
        bg.setSize(map_rect.size);
        bg.setFillColor(rgb(173, 216, 230));
        bg.setOutlineColor(rgb(90, 90, 90));
        bg.setOutlineThickness(1.f);
        win.draw(bg);
    }

    if (!gameLoaded_) {
        return;
    }

    

    std::unordered_map<std::string, const ShipDto*> ship_by_callsign;
    ship_by_callsign.reserve(currentGameState_.ships.size());
    for (const auto& s : currentGameState_.ships) {
        ship_by_callsign.emplace(s.callsign, &s);
    }

    bool have = false;
    int min_x = 0, max_x = 0, min_y = 0, max_y = 0;
    for (const auto& u : currentGameState_.ship_units) {
        extend_bounds(have, min_x, max_x, min_y, max_y, u.x, u.y);
    }
    for (const auto& [idx, b] : currentGameState_.mission.pirates_bases) {
        (void)idx;
        extend_bounds(have, min_x, max_x, min_y, max_y, b.x, b.y);
    }
    for (const auto& [name, b] : currentGameState_.mission.bases) {
        (void)name;
        extend_bounds(have, min_x, max_x, min_y, max_y, b.x, b.y);
    }
    if (!have) {
        return;
    }
    const float pad_x = std::max(5.f, (max_x - min_x) * 0.1f);
    const float pad_y = std::max(5.f, (max_y - min_y) * 0.1f);
    const float world_min_x = static_cast<float>(min_x) - pad_x;
    const float world_max_x = static_cast<float>(max_x) + pad_x;
    const float world_min_y = static_cast<float>(min_y) - pad_y;
    const float world_max_y = static_cast<float>(max_y) + pad_y;

    if (aux_panel == AuxPanel::SHIPBUY && point_for_ship) {
        const sf::Vector2f p = to_screen_coords(static_cast<float>(point_for_ship->get_x()),
                                               static_cast<float>(point_for_ship->get_y()),
                                               world_min_x, world_max_x, world_min_y, world_max_y, map_rect);
        sf::CircleShape marker;
        marker.setRadius(10.f);
        marker.setOrigin({10.f, 10.f});
        marker.setPosition(p);
        marker.setFillColor(sf::Color(255, 255, 255, 0));
        marker.setOutlineColor(sf::Color::White);
        marker.setOutlineThickness(2.f);
        win.draw(marker);
    }

    if (aux_panel == AuxPanel::ATTACK) {
        auto lower = [](std::string s) {
            std::transform(s.begin(), s.end(), s.begin(), ::tolower);
            return s;
        };
        const std::string attacker_cs = selected_attack_attacker_.value_or(std::string{});
        const std::string slot = lower(text_inputs.at("weapon_slot").value);

        const ShipUnitDTO* attacker_unit = nullptr;
        for (const auto& u : currentGameState_.ship_units) {
            if (u.callsign == attacker_cs) {
                attacker_unit = &u;
                break;
            }
        }

        unsigned fire_range = 0;
        if (attacker_unit && !attacker_cs.empty()) {
            const ShipDto* ship = nullptr;
            for (const auto& s : currentGameState_.ships) {
                if (s.callsign == attacker_cs) {
                    ship = &s;
                    break;
                }
            }
            if (ship) {
                auto it = ship->params.find(slot);
                if (it != ship->params.end()) {
                    unsigned wid = 0;
                    try {
                        wid = static_cast<unsigned>(std::stoul(it->second));
                    } catch (...) {
                        wid = 0;
                    }
                    if (wid != 0) {
                        const WeaponDto* w = nullptr;
                        for (const auto& ww : currentGameState_.weapons) {
                            if (ww.id == wid) {
                                w = &ww;
                                break;
                            }
                        }
                        if (w) {
                            auto itp = currentGameState_.weapons_params.find(w->type);
                            if (itp != currentGameState_.weapons_params.end()) {
                                fire_range = itp->second.fire_range;
                            }
                        }
                    }
                }
            }
        }

        if (attacker_unit && fire_range > 0) {
            const sf::Vector2f center = to_screen_coords(static_cast<float>(attacker_unit->x), static_cast<float>(attacker_unit->y),
                                                         world_min_x, world_max_x, world_min_y, world_max_y, map_rect);
            const float sx = map_rect.size.x / (world_max_x - world_min_x);
            const float sy = map_rect.size.y / (world_max_y - world_min_y);

            sf::CircleShape range_circle;
            range_circle.setRadius(static_cast<float>(fire_range));
            range_circle.setOrigin({static_cast<float>(fire_range), static_cast<float>(fire_range)});
            range_circle.setPosition(center);
            range_circle.setScale({sx, sy});
            range_circle.setFillColor(sf::Color(255, 255, 255, 0));
            range_circle.setOutlineColor(sf::Color::White);
            range_circle.setOutlineThickness(1.f);
            win.draw(range_circle);
        }
    }

    auto it_a = currentGameState_.mission.bases.find("a_base");
    auto it_b = currentGameState_.mission.bases.find("b_base");
    if (it_a != currentGameState_.mission.bases.end()) {
        const float x = to_screen_coords(static_cast<float>(it_a->second.x), static_cast<float>(it_a->second.y), world_min_x, world_max_x, world_min_y, world_max_y, map_rect).x;
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f{x, map_rect.position.y}, rgb(255, 0, 0)),
            sf::Vertex(sf::Vector2f{x, map_rect.position.y + map_rect.size.y}, rgb(255, 0, 0)),
        };
        win.draw(line, 2, sf::PrimitiveType::Lines);
    }
    if (it_b != currentGameState_.mission.bases.end()) {
        const float x = to_screen_coords(static_cast<float>(it_b->second.x), static_cast<float>(it_b->second.y), world_min_x, world_max_x, world_min_y, world_max_y, map_rect).x;
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f{x, map_rect.position.y}, rgb(160, 160, 160)),
            sf::Vertex(sf::Vector2f{x, map_rect.position.y + map_rect.size.y}, rgb(160, 160, 160)),
        };
        win.draw(line, 2, sf::PrimitiveType::Lines);
    }

    sf::Text label(font);
    label.setCharacterSize(12);
    label.setFillColor(sf::Color::Black);
    int base_idx = 0;
    for (const auto& [idx, b] : currentGameState_.mission.pirates_bases) {
        (void)idx;
        const sf::Vector2f p = to_screen_coords(static_cast<float>(b.x), static_cast<float>(b.y), world_min_x, world_max_x, world_min_y, world_max_y, map_rect);
        sf::CircleShape star;
        star.setRadius(6.f);
        star.setOrigin({6.f, 6.f});
        star.setPosition(p);
        star.setFillColor(rgb(255, 204, 0));
        win.draw(star);
        label.setString("B" + std::to_string(base_idx++));
        label.setPosition(sf::Vector2f{p.x + 8.f, p.y + 6.f});
        label.setFillColor(rgb(200, 160, 0));
        win.draw(label);
    }

    sf::Text cs(font);
    cs.setCharacterSize(12);
    sf::Text hp(font);
    hp.setCharacterSize(11);
    hp.setFillColor(sf::Color::Black);

    const float ship_icon_size = 26.f * 1.5f;
    for (const auto& u : currentGameState_.ship_units) {
        const sf::Vector2f p = to_screen_coords(static_cast<float>(u.x), static_cast<float>(u.y), world_min_x, world_max_x, world_min_y, world_max_y, map_rect);
        const sf::Color c = get_ship_color_by_callsign(u.callsign);

        sf::Sprite* icon = nullptr;
        if (is_strings_equal(u.side, "pirate") || is_strings_equal(u.side, "pirates")) {
            auto it = sprites.find("map_pirate_sprite");
            if (it != sprites.end()) {
                icon = &it->second;
            }
        } else {
            std::string type;
            if (auto it_s = ship_by_callsign.find(u.callsign); it_s != ship_by_callsign.end() && it_s->second != nullptr) {
                type = lower_string(it_s->second->type);
            }

            const std::string key =
                (type == "military")
                    ? "map_empire_military_sprite"
                    : (type == "transport")
                          ? "map_empire_transport_sprite"
                          : (type == "transport_military" || type == "transportmilitary" || type == "tm")
                                ? "map_empire_transport_military_sprite"
                                : std::string{};

            if (!key.empty()) {
                auto it = sprites.find(key);
                if (it != sprites.end()) {
                    icon = &it->second;
                }
            }
        }

        if (icon) {
            fit_sprite_to_square(*icon, {p.x - ship_icon_size / 2.f, p.y - ship_icon_size / 2.f}, ship_icon_size);
            win.draw(*icon);
        } else {
            sf::CircleShape dot;
            dot.setRadius(7.f);
            dot.setOrigin({7.f, 7.f});
            dot.setPosition(p);
            dot.setFillColor(c);
            win.draw(dot);
        }

        {
            cs.setString(u.callsign);
            cs.setFillColor(sf::Color::Black);
            const sf::FloatRect b = cs.getLocalBounds();
            cs.setOrigin(sf::Vector2f{b.position.x + b.size.x / 2.f, b.position.y});
            cs.setPosition(sf::Vector2f{p.x, p.y + ship_icon_size / 2.f + 2.f});
            win.draw(cs);
        }

        auto it_s = ship_by_callsign.find(u.callsign);
        if (it_s != ship_by_callsign.end()) {
            const ShipDto* sd = it_s->second;
            unsigned max_hp = 0;
            auto it_p = currentGameState_.ships_params.find(sd->type);
            if (it_p != currentGameState_.ships_params.end()) {
                max_hp = it_p->second.max_HP;
            }
            const std::string hp_text = (max_hp > 0)
                                            ? (std::to_string(sd->current_HP) + "/" + std::to_string(max_hp))
                                            : std::to_string(sd->current_HP);
            hp.setString(hp_text);
            const sf::FloatRect bhp = hp.getLocalBounds();
            hp.setOrigin(sf::Vector2f{bhp.position.x + bhp.size.x / 2.f, bhp.position.y});
            hp.setPosition(sf::Vector2f{p.x, p.y + ship_icon_size / 2.f + 16.f});
            win.draw(hp);
        }
    }
}

void SfmlGameView::draw_panel_background(const sf::FloatRect& r, sf::Color fill, float outline_thickness, sf::Color outline) {
    sf::RectangleShape bg;
    bg.setPosition(r.position);
    bg.setSize(r.size);
    bg.setFillColor(fill);
    bg.setOutlineColor(outline);
    bg.setOutlineThickness(outline_thickness);
    win.draw(bg);
}

void SfmlGameView::draw_mission_info_panel(const sf::FloatRect& mission_info_rect) {
    draw_panel_background(mission_info_rect, rgb(15, 15, 15));
    if (!gameLoaded_) {
        return;
    }
    std::set<std::string> imperial_callsigns;
    for (const auto& u : currentGameState_.ship_units) {
        if (is_strings_equal(u.side, "pirate") || is_strings_equal(u.side, "pirates")) {
            continue;
        }
        imperial_callsigns.insert(u.callsign);
    }
    std::vector<const ShipDto*> imperial_ships;
    imperial_ships.reserve(currentGameState_.ships.size());
    for (const auto& s : currentGameState_.ships) {
        if (imperial_callsigns.contains(s.callsign)) {
            imperial_ships.push_back(&s);
        }
    }

    unsigned convoy_speed = 0;
    if (!imperial_ships.empty()) {
        convoy_speed = imperial_ships.front()->current_velocity;
        for (const ShipDto* s : imperial_ships) {
            convoy_speed = std::min(convoy_speed, s->current_velocity);
        }
    }
    const unsigned money = (currentGameState_.mission.budget >= currentGameState_.mission.spent_budget)
                               ? (currentGameState_.mission.budget - currentGameState_.mission.spent_budget)
                               : 0u;

    sf::Text title(font);
    title.setCharacterSize(20);
    title.setFillColor(sf::Color::White);
    title.setString("Mission info");
    title.setPosition({mission_info_rect.position.x + 10.f, mission_info_rect.position.y + 8.f});
    win.draw(title);

    sf::Text line(font);
    line.setCharacterSize(16);
    line.setFillColor(sf::Color::White);

    float y = mission_info_rect.position.y + 34.f;
    line.setString("$: " + std::to_string(money) + " (spent " + std::to_string(currentGameState_.mission.spent_budget) + ")");
    line.setPosition({mission_info_rect.position.x + 10.f, y});
    win.draw(line);
    y += 18.f;

    line.setString(
        "Cargo: " + std::to_string(currentGameState_.mission.total_cargo_weight) + "/" +
        std::to_string(currentGameState_.mission.goal_weight) +
        "  delivered: " + std::to_string(currentGameState_.mission.delivered_weight) +
        "  lost: " + std::to_string(currentGameState_.mission.lost_weight)
    );
    line.setPosition({mission_info_rect.position.x + 10.f, y});
    win.draw(line);
    y += 18.f;

    line.setString("Convoy speed: " + std::to_string(convoy_speed));
    line.setPosition({mission_info_rect.position.x + 10.f, y});
    win.draw(line);
}

void SfmlGameView::draw_imperial_ships_panel(const sf::FloatRect& imperial_rect,
                                            int& imperial_info_scroll_lines) {
    draw_panel_background(imperial_rect, rgb(15, 15, 15));
    if (!gameLoaded_) {
        return;
    }

    std::unordered_map<unsigned, WeaponDto> weapon_by_id;
    weapon_by_id.reserve(currentGameState_.weapons.size());
    for (const auto& w : currentGameState_.weapons) {
        weapon_by_id.emplace(w.id, w);
    }

    std::set<std::string> imperial_callsigns;
    for (const auto& u : currentGameState_.ship_units) {
        if (is_strings_equal(u.side, "pirate") || is_strings_equal(u.side, "pirates")) {
            continue;
        }
        imperial_callsigns.insert(u.callsign);
    }

    std::vector<const ShipDto*> imperial_ships;
    imperial_ships.reserve(currentGameState_.ships.size());
    for (const auto& s : currentGameState_.ships) {
        if (imperial_callsigns.contains(s.callsign)) {
            imperial_ships.push_back(&s);
        }
    }

    sf::Text title(font);
    title.setCharacterSize(16);
    title.setFillColor(sf::Color::White);
    title.setString("Imperial Ships");
    title.setPosition({imperial_rect.position.x + 10.f, imperial_rect.position.y + 8.f});
    win.draw(title);

    sf::Text line(font);
    line.setCharacterSize(13);
    line.setFillColor(sf::Color::White);

    float y = imperial_rect.position.y + 34.f;

    struct InfoLine {
        std::string text;
        sf::Color color;
    };
    std::vector<InfoLine> lines;
    auto max_hp_of = [&](const ShipDto& s) -> unsigned {
        auto it = currentGameState_.ships_params.find(s.type);
        return (it == currentGameState_.ships_params.end()) ? 0u : it->second.max_HP;
    };

    auto append_ship_lines = [&](std::vector<InfoLine>& out, const ShipDto& s, sf::Color header_color) {
        const unsigned max_hp = max_hp_of(s);
        const std::string hp = (max_hp > 0)
                                   ? (std::to_string(s.current_HP) + "/" + std::to_string(max_hp))
                                   : std::to_string(s.current_HP);

        out.push_back({s.callsign + " [" + s.type + "]  " + std::to_string(s.current_velocity) + "p/t", header_color});
        out.push_back({"  captain: " + s.captain, rgb(220, 220, 220)});

        
        auto it_cur = s.params.find("current_cargo_weight");
        auto it_max = s.params.find("max_cargo_weight");
        if (it_cur != s.params.end() && it_max != s.params.end()) {
            out.push_back({"  cargo: " + it_cur->second + "/" + it_max->second, rgb(220, 220, 220)});
        }

        if (s.params.empty()) {
            out.push_back({"  (no equipment)", rgb(180, 180, 180)});
            out.push_back({"", sf::Color::White});
            return;
        }
        for (const auto& [place, value] : s.params) {
            if (place == "current_cargo_weight" || place == "max_cargo_weight") {
                continue;
            }
            unsigned wid = 0;
            try {
                wid = static_cast<unsigned>(std::stoul(value));
            } catch (...) {
                out.push_back({"  " + place + ": " + value, rgb(180, 180, 180)});
                continue;
            }
            auto it_w = weapon_by_id.find(wid);
            if (it_w == weapon_by_id.end()) {
                out.push_back({"  " + place + ": #" + std::to_string(wid) + " (missing)", rgb(180, 180, 180)});
                continue;
            }
            const WeaponDto& w = it_w->second;
            unsigned max_ammo = 0;
            auto it_p = currentGameState_.weapons_params.find(w.type);
            if (it_p != currentGameState_.weapons_params.end()) {
                max_ammo = it_p->second.max_ammunition;
            }
            std::string ammo = (max_ammo > 0)
                                   ? (std::to_string(w.current_ammunition) + "/" + std::to_string(max_ammo))
                                   : (std::to_string(w.current_ammunition) + "/?");
            out.push_back({"  " + place + ": " + w.type + " (" + ammo + ")", sf::Color::White});
        }
        out.push_back({"", sf::Color::White});
    };

    lines.reserve(imperial_ships.size() * 7);
    for (const ShipDto* s : imperial_ships) {
        const sf::Color c = get_ship_color_by_callsign(s->callsign);
        append_ship_lines(lines, *s, c);
    }

    const float list_top_y = y;
    const float list_bottom_y = imperial_rect.position.y + imperial_rect.size.y - 8.f;
    const int visible_lines = std::max(1, static_cast<int>((list_bottom_y - list_top_y) / 16.f));
    const int max_scroll = std::max(0, static_cast<int>(lines.size()) - visible_lines);
    imperial_info_scroll_lines = std::clamp(imperial_info_scroll_lines, 0, max_scroll);

    float yy = list_top_y;
    const int start = imperial_info_scroll_lines;
    const int end = std::min(static_cast<int>(lines.size()), start + visible_lines);
    for (int i = start; i < end; ++i) {
        line.setFillColor(lines[static_cast<size_t>(i)].color);
        line.setString(lines[static_cast<size_t>(i)].text);
        line.setPosition({imperial_rect.position.x + 10.f, yy});
        win.draw(line);
        yy += 16.f;
    }
}

void SfmlGameView::draw_pirates_panel(const sf::FloatRect& pirate_rect, int& pirate_info_scroll_lines) {
    draw_panel_background(pirate_rect, rgb(15, 15, 15));
    if (!gameLoaded_) {
        return;
    }

    std::unordered_map<unsigned, WeaponDto> weapon_by_id;
    weapon_by_id.reserve(currentGameState_.weapons.size());
    for (const auto& w : currentGameState_.weapons) {
        weapon_by_id.emplace(w.id, w);
    }

    std::unordered_map<std::string, const ShipDto*> ship_by;
    ship_by.reserve(currentGameState_.ships.size());
    for (const auto& s : currentGameState_.ships) {
        ship_by.emplace(s.callsign, &s);
    }

    struct Line { std::string text; sf::Color color; };
    std::vector<Line> lines;
    lines.reserve(currentGameState_.ship_units.size() * 6);

    auto max_hp_of = [&](const ShipDto& s) -> unsigned {
        auto it = currentGameState_.ships_params.find(s.type);
        return (it == currentGameState_.ships_params.end()) ? 0u : it->second.max_HP;
    };

    auto append_ship_lines = [&](std::vector<Line>& out, const ShipDto& s) {
        const unsigned max_hp = max_hp_of(s);
        const std::string hp = (max_hp > 0)
                                   ? (std::to_string(s.current_HP) + "/" + std::to_string(max_hp))
                                   : std::to_string(s.current_HP);

        out.push_back({s.callsign + " [" + s.type + "]  " + std::to_string(s.current_velocity)+ "p/t", rgb(228, 26, 28)});
        out.push_back({"  captain: " + s.captain, rgb(220, 220, 220)});

        if (s.params.empty()) {
            out.push_back({"  (no equipment)", rgb(180, 180, 180)});
            out.push_back({"", sf::Color::White});
            return;
        }
        
        out.push_back({"", sf::Color::White});
    };

    for (const auto& u : currentGameState_.ship_units) {
        if (!(is_strings_equal(u.side, "pirate") || is_strings_equal(u.side, "pirates"))) {
            continue;
        }
        auto it = ship_by.find(u.callsign);
        if (it == ship_by.end() || it->second == nullptr) {
            lines.push_back({u.callsign + " [?]", rgb(228, 26, 28)});
            continue;
        }
        append_ship_lines(lines, *it->second);
    }

    sf::Text title(font);
    title.setCharacterSize(16);
    title.setFillColor(sf::Color::White);
    title.setString("Pirate Ships");
    title.setPosition({pirate_rect.position.x + 10.f, pirate_rect.position.y + 8.f});
    win.draw(title);

    sf::Text line(font);
    line.setCharacterSize(13);
    line.setFillColor(sf::Color::White);

    const float list_top_y = pirate_rect.position.y + 34.f;
    const float list_bottom_y = pirate_rect.position.y + pirate_rect.size.y - 8.f;
    const int visible_lines = std::max(1, static_cast<int>((list_bottom_y - list_top_y) / 16.f));
    const int max_scroll = std::max(0, static_cast<int>(lines.size()) - visible_lines);
    pirate_info_scroll_lines = std::clamp(pirate_info_scroll_lines, 0, max_scroll);

    float yy = list_top_y;
    const int start = pirate_info_scroll_lines;
    const int end = std::min(static_cast<int>(lines.size()), start + visible_lines);
    for (int i = start; i < end; ++i) {
        line.setFillColor(lines[static_cast<size_t>(i)].color);
        line.setString(lines[static_cast<size_t>(i)].text);
        line.setPosition({pirate_rect.position.x + 10.f, yy});
        win.draw(line);
        yy += 16.f;
    }
}

void SfmlGameView::draw_main_menu(const sf::FloatRect& main_menu_rect, 
                    const Button& btn_attack, const Button& btn_ships, 
                    const Button& btn_weapons, const Button& btn_cargo, 
                    const Button& btn_end_turn, const Button& btn_exit_game) {
    draw_panel_background(main_menu_rect, rgb(18, 18, 18), 0.f);


    for (auto* b : {&btn_attack, &btn_ships, &btn_weapons, &btn_cargo, &btn_end_turn, &btn_exit_game}) {
        win.draw(b->rect);
        win.draw(b->label);
    }
}

void SfmlGameView::update_text_input(TextInput& ti, const sf::Event& e) {
    if (!ti.active) {
        return;
    }
    if (const auto* te = e.getIf<sf::Event::TextEntered>()) {
        const auto code = te->unicode;
        if (code == 8) {
            if (!ti.value.empty()) {
                ti.value.pop_back();
            }
        } else if (code == 13) {
        } else if (code >= 32 && code < 127) {
            ti.value.push_back(static_cast<char>(code));
        }
        ti.text.setString(ti.value);
    }
}

std::optional<Point> SfmlGameView::get_map_point() {
    bool have = false;
    int min_x = 0, max_x = 0, min_y = 0, max_y = 0;
    for (const auto& u : currentGameState_.ship_units) {
        extend_bounds(have, min_x, max_x, min_y, max_y, u.x, u.y);
    }
    for (const auto& [idx, b] : currentGameState_.mission.pirates_bases) {
        (void)idx;
        extend_bounds(have, min_x, max_x, min_y, max_y, b.x, b.y);
    }
    for (const auto& [name, b] : currentGameState_.mission.bases) {
        (void)name;
        extend_bounds(have, min_x, max_x, min_y, max_y, b.x, b.y);
    }
    if (have) {
        const float pad_x = std::max(5.f, (max_x - min_x) * 0.1f);
        const float pad_y = std::max(5.f, (max_y - min_y) * 0.1f);
        const float world_min_x = static_cast<float>(min_x) - pad_x;
        const float world_max_x = static_cast<float>(max_x) + pad_x;
        const float world_min_y = static_cast<float>(min_y) - pad_y;
        const float world_max_y = static_cast<float>(max_y) + pad_y;

        const float nx = (pressed_area.x - map_rect.position.x) / map_rect.size.x;
        const float ny = 1.f - ((pressed_area.y - map_rect.position.y) / map_rect.size.y);
        const float wx = world_min_x + nx * (world_max_x - world_min_x);
        const float wy = world_min_y + ny * (world_max_y - world_min_y);
        return Point(static_cast<int>(std::lround(wx)), static_cast<int>(std::lround(wy)));
        
    }
    return std::nullopt;
}

std::optional<std::string> SfmlGameView::pick_ship_callsign_on_map(sf::Vector2f mouse_pos, std::optional<bool> require_pirate) {
    if (!gameLoaded_ || !map_rect.contains(mouse_pos)) {
        return std::nullopt;
    }

    bool have = false;
    int min_x = 0, max_x = 0, min_y = 0, max_y = 0;
    for (const auto& u : currentGameState_.ship_units) {
        extend_bounds(have, min_x, max_x, min_y, max_y, u.x, u.y);
    }
    for (const auto& [idx, b] : currentGameState_.mission.pirates_bases) {
        (void)idx;
        extend_bounds(have, min_x, max_x, min_y, max_y, b.x, b.y);
    }
    for (const auto& [name, b] : currentGameState_.mission.bases) {
        (void)name;
        extend_bounds(have, min_x, max_x, min_y, max_y, b.x, b.y);
    }
    if (!have) {
        return std::nullopt;
    }

    const float pad_x = std::max(5.f, (max_x - min_x) * 0.1f);
    const float pad_y = std::max(5.f, (max_y - min_y) * 0.1f);
    const float world_min_x = static_cast<float>(min_x) - pad_x;
    const float world_max_x = static_cast<float>(max_x) + pad_x;
    const float world_min_y = static_cast<float>(min_y) - pad_y;
    const float world_max_y = static_cast<float>(max_y) + pad_y;

    auto is_pirate = [&](const std::string& side) {
        return is_strings_equal(side, "pirate") || is_strings_equal(side, "pirates");
    };

    const float ship_icon_size = 26.f * 1.5f;
    const float hit_radius = ship_icon_size * 0.70f;

    float best_dist = std::numeric_limits<float>::infinity();
    const ShipUnitDTO* best = nullptr;

    for (const auto& u : currentGameState_.ship_units) {
        if (require_pirate.has_value()) {
            const bool want_pirate = *require_pirate;
            if (is_pirate(u.side) != want_pirate) {
                continue;
            }
        }

        const sf::Vector2f p = to_screen_coords(
            static_cast<float>(u.x), static_cast<float>(u.y),
            world_min_x, world_max_x, world_min_y, world_max_y, map_rect
        );
        const float dx = p.x - mouse_pos.x;
        const float dy = p.y - mouse_pos.y;
        const float d = std::sqrt(dx * dx + dy * dy);

        if (d < best_dist) {
            best_dist = d;
            best = &u;
        }
    }

    if (!best || !(best_dist <= hit_radius)) {
        return std::nullopt;
    }
    return best->callsign;
}

// ============ вспомогательные функции ========================

bool is_strings_equal(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) {
        return false;
    }
    std::string atemp = a;
    std::string btemp = b;
    std::transform(atemp.begin(), atemp.end(), atemp.begin(), ::tolower);
    std::transform(btemp.begin(), btemp.end(), btemp.begin(), ::tolower);
    return atemp == btemp;
}

std::string lower_string(const std::string & s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return std::move(result);
}

void fit_sprite_to_square(sf::Sprite& sprite, sf::Vector2f pos, float size) {
    const sf::FloatRect bounds = sprite.getLocalBounds();
    sprite.setOrigin(bounds.position);
    const float sx = (bounds.size.x == 0.f) ? 1.f : (size / bounds.size.x);
    const float sy = (bounds.size.y == 0.f) ? 1.f : (size / bounds.size.y);
    sprite.setScale(sf::Vector2f{sx, sy});
    sprite.setPosition(pos);
}

void fit_sprite_to_screen(sf::Sprite& sprite, sf::Vector2u window_size) {
    const sf::FloatRect bounds = sprite.getLocalBounds();
    const float scale_x = window_size.x / bounds.size.x;
    const float scale_y = window_size.y / bounds.size.y;
    sprite.setScale(sf::Vector2f{scale_x, scale_y});
    sprite.setPosition(sf::Vector2f{0.f, 0.f});
}
