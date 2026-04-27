#pragma once

#include "view/GameView.h"
#include "model/place/Place.h"
#include "unordered_map"
#include "model/point/Point.h"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Audio.hpp>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

class GamePresenter;
class MissionService;

inline sf::Color rgb(unsigned r, unsigned g, unsigned b) {
    return sf::Color(static_cast<std::uint8_t>(r), 
                    static_cast<std::uint8_t>(g), 
                    static_cast<std::uint8_t>(b));
}

struct Button {
    sf::RectangleShape rect;
    sf::Text label;
    bool enabled = true;    
    Button(const sf::Font& font, const std::string& text, const sf::Texture* texture = nullptr) 
        : label(font) {
        rect.setFillColor(rgb(50, 50, 50));
        rect.setOutlineColor(rgb(180, 180, 180));
        rect.setOutlineThickness(1.f);
        if (texture) {
            rect.setTexture(texture);
            rect.setFillColor(sf::Color::White);
        }
        label.setCharacterSize(25);
        label.setFillColor(sf::Color::Black);
        label.setString(text);
    }

    void set_background_texture(const sf::Texture* texture) {
        rect.setTexture(texture);
        rect.setFillColor(sf::Color::White);
    }

    Button(const Button&) = default;
    Button(Button&&) noexcept = default;
    Button& operator=(const Button&) = default;
    Button& operator=(Button&&) noexcept = default;


    bool hit(sf::Vector2f p) const {
        return rect.getGlobalBounds().contains(p);
    }
};

struct TextInput {
    sf::RectangleShape rect;
    sf::Text text;
    bool active = false;
    std::string value;

    TextInput(const sf::Font& font, unsigned char_size) : text(font) {
        rect.setFillColor(rgb(30, 30, 30));
        rect.setOutlineColor(rgb(180, 180, 180));
        rect.setOutlineThickness(1.f);
        text.setCharacterSize(char_size);
        text.setFillColor(sf::Color::White);
    }

    TextInput(const sf::Font& font, unsigned char_size, const std::string & str) : text(font) {
        rect.setFillColor(rgb(30, 30, 30));
        rect.setOutlineColor(rgb(180, 180, 180));
        rect.setOutlineThickness(1.f);
        text.setCharacterSize(char_size);
        text.setFillColor(sf::Color::White);
        value = str;
    }
    bool hit(sf::Vector2f p) const { return rect.getGlobalBounds().contains(p); }
};

struct InfoField {
    sf::RectangleShape rect;
    sf::Text text;
    std::string value;

    InfoField(const sf::Font& font, unsigned char_size) : text(font) {
        rect.setFillColor(rgb(24, 24, 24));
        rect.setOutlineColor(rgb(180, 180, 180));
        rect.setOutlineThickness(1.f);
        text.setCharacterSize(char_size);
        text.setFillColor(sf::Color::White);
    }

    void set_value(std::string v) {
        value = std::move(v);
        text.setString(value);
    }
};

enum class Screen { 
    Start, 
    MissionSelect, 
    Game 
};

enum class AuxPanel { 
    NONE, 
    ATTACK, 
    SHIPBUY, 
    SHIPSELL, 
    WEAPONSET, 
    WEAPONUNSET, 
    CARGOLOAD, 
    CARGOUNLOAD 
};

enum class MainTab { 
    ATTACK, 
    SHIPS, 
    WEAPONS, 
    CARGO 
};


class SfmlGameViewLoader {
    private:
        sf::Font font;
    public:
        SfmlGameViewLoader(const std::string & path);
        void set_font(const std::string & path);
        sf::Font & get_font();
       ~SfmlGameViewLoader() = default;
};


class SfmlGameView final : public GameView {
    private:
        GamePresenter& presenter_;
        MissionService& mission_service_;

        bool gameLoaded_ = false;
        GameStateDto currentGameState_{};
        sf::Font font;
        std::string config_path_dir;
        std::string font_path;
        std::string icon_path_dir;


    public:
        SfmlGameView(GamePresenter& presenter, MissionService& mission_service);
        ~SfmlGameView() override = default;
        void run() override;
        void update_game_state(const GameStateDto& gameState) override;
        bool is_game_loaded() const override;

        void set_config_path(std::string path);
        void set_font(std::string font_path);

        inline sf::Font get_font() const { return font;};
        const std::string & get_config_path() override { return config_path_dir;};

    private: 
        Screen screen;
        std::vector<std::string> log;
        sf::Vector2f pressed_area;
        MainTab main_tab;
        AuxPanel aux_panel;

        std::unordered_map<std::string, Button> buttons;
        std::unordered_map<std::string, TextInput> text_inputs;
        std::unordered_map<std::string, InfoField> info_fields;
        std::unordered_map<std::string, sf::Texture> textures;
        std::unordered_map<std::string, sf::Sprite> sprites;
        std::unordered_map<std::string, sf::RectangleShape> rectshapes;

        sf::FloatRect map_rect;
        sf::FloatRect log_rect;
        sf::FloatRect main_menu_rect;
        sf::FloatRect aux_rect;
        sf::FloatRect aux_top_rect;
        sf::FloatRect aux_bottom_rect;
        sf::FloatRect mission_info_rect;
        sf::FloatRect mission_rect;
        sf::FloatRect pirate_rect;

        std::optional<Point> planned_attack_center;
        std::optional<Point> point_for_ship; 
        std::string buy_ship_type;

        enum class AttackPickStage { Attacker, Target };
        AttackPickStage attack_pick_stage = AttackPickStage::Attacker;

        std::optional<std::string> selected_attack_attacker_;
        std::optional<std::string> selected_attack_target_;
        std::optional<std::string> selected_sell_ship_;
        std::optional<std::string> selected_weapon_ship_;
        std::optional<std::string> selected_unset_ship_;
        std::optional<std::string> selected_cargo_ship_;

        std::string weapon_set_type = "cannon";   
        std::string weapon_set_place = "bow";     
        std::string weapon_unset_place = "bow";   

         int convoy_info_scroll_lines;
        int pirate_info_scroll_lines;

        bool mission_complete = false;
        sf::Clock mission_complete_clock;

        bool show_mission_complete_dialog = false;

        bool show_save_dialog = false;
        sf::RenderWindow win;

        sf::Music background_music_;
        sf::SoundBuffer click_buffer_;
        std::optional<sf::Sound> click_sound_;


        void init_buttons();
        void init_text_inputs();
        void init_info_fields();
        void init_textures();
        void apply_button_background();
        void place_main_menu();
        
        void process_start_screen();
        void process_mission_select_screen();
        void process_game_screen();
        void process_game();

        void process_attack_auxpanel();
        void process_ship_buy_auxpanel();
        void process_ship_sell_auxpanel();
        void process_weapon_buy_auxpanel();
        void process_weapon_sell_auxpanel();
        void process_cargo_load_auxpanel();
        void process_cargo_unload_auxpanel();

        void push_log(std::vector<std::string>& log, std::string s);
        void place_button(Button& b, sf::Vector2f pos, sf::Vector2f size);
        void place_text_input(TextInput& ti, sf::Vector2f pos, sf::Vector2f size);
        void place_info_field(InfoField& fi, sf::Vector2f pos, sf::Vector2f size);
        void refresh_state();
        std::optional<Place> weapon_place_from_ui(const std::string& weapon_slot_value);

        std::optional<std::string> pick_ship_callsign_on_map(sf::Vector2f mouse_pos, std::optional<bool> require_pirate);
       
        std::optional<unsigned> ship_cost_by_type(const std::string& type);
        unsigned money_left();
        void try_load_game(const std::string & path, std::vector<std::string>& log, Screen& screen);
        void draw_log_panel(const sf::FloatRect& log_rect, 
                            const std::vector<std::string>& log);
        void extend_bounds(bool& have, int& min_x, int& max_x, int& min_y, 
                           int& max_y, int x, int y);
        sf::Vector2f to_screen_coords(float x, float y, float world_min_x, 
                                      float world_max_x, float world_min_y, float world_max_y, const sf::FloatRect& map_rect);
        sf::Color get_ship_color_by_callsign(const std::string& callsign);
        void draw_legend(float& lx, float& ly);
        void draw_map(const sf::FloatRect& map_rect);
        void draw_panel_background(const sf::FloatRect& r, sf::Color fill, float outline_thickness = 1.f, sf::Color outline = rgb(90, 90, 90));
        void draw_mission_info_panel(const sf::FloatRect& mission_info_rect);
        void draw_imperial_ships_panel(const sf::FloatRect& imperial_rect,
                                       int& imperial_info_scroll_lines);
        void draw_pirates_panel(const sf::FloatRect& pirate_rect, int& pirate_info_scroll_lines);
        void draw_main_menu(const sf::FloatRect& main_menu_rect, 
                            const Button& btn_attack, const Button& btn_ships, 
                            const Button& btn_weapons, const Button& btn_cargo, 
                            const Button& btn_end_turn, const Button& btn_exit_game);
        void update_text_input(TextInput& ti, const sf::Event& e);
        std::optional<Point> get_map_point();
        void refresh_text_input_states();
        void draw_label(const std::string & s, const sf::Vector2f & pos);
        void draw_label(const std::string& s, const sf::RectangleShape& r);

        void draw_input(TextInput& ti);
        void draw_info_field(InfoField& fi);
        void draw_button(Button& b);
        
        void draw_ship_buy_panel();
        void draw_ship_sell_panel();

        void set_text_input_states(bool state);

    
};
