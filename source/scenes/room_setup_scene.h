#ifndef GAMESCENE_ROOM_SETUP_INC
#define GAMESCENE_ROOM_SETUP_INC

#include "../gamescene.h"
#include "../gamebutton.h"

namespace scenes {

START_SCENE(room_setup_scene)
    next_scene update(const ctr::hid& input, const double dt) override final;
    void draw(ctr::gfx& gfx) override final;

    void now_at(const int entry_index, const int tab_index);

    void set_next_scene_to(scene_ptr ptr);

    const bool is_multiplayer;

    void change_width(int by);
    void change_height(int by);
    void change_mine_percent(int by);
    void ask_width();
    void ask_height();
    void ask_mine_percent();
    void set_board_type(game::board::mode mode);

    void prepare_starting();

private:
    room_setup_scene(bool multi);

    game::scene_menu<room_setup_scene> menu;
    next_scene next;

    int entry_index, tab_index;
    game::button_page<room_setup_scene, 14> tab_numbers_buttons;
    game::button_page<room_setup_scene, 6> tab_type_buttons;
    game::button_page<room_setup_scene, 6> tab_final_buttons;

    game::button::parts button_parts, selected_button_parts;
    C2D_Text top_text[3];
    C2D_Text back_text, ok_text;
    C2D_Image width_img, height_img, mines_img;
    C2D_Image plus_img, minus_img;
    C2D_Image board_imgs[2][2];
    game::board::numbers numbers;
    game::board::mode board_mode;

END_SCENE

}

#endif
