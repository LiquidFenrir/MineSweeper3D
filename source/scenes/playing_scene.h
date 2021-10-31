#ifndef GAMESCENE_PLAYING_INC
#define GAMESCENE_PLAYING_INC

#include "../gamescene.h"
#include "../gameboard.h"

namespace scenes {

START_SCENE(playing_scene)
    next_scene update(const ctr::hid& input, ctr::audio& audio, const double dt) override final;
    void draw(ctr::gfx& gfx) override final;

private:
    playing_scene(game::board::numbers n, game::board::mode b, const int self_idx);

    C3D_Mtx projection;
    int point_count;
    int point_count_margin;
    int point_count_cursors;
    game::board::numbers nums;
    game::board::mode board_mode;
    game::player& self;
    bool stop_music;
    int selected_tab;
    C2D_Image bottom_screen_background_img;
    C2D_Image normal_tab_img, selected_tab_img;
    C2D_Image minimap_cover, no_minimap_img, player_indicator;
    C2D_Image tab_icons[3];
    C2D_Image minimap_imgs[12];
    C2D_Image grass_img, cursor_img;
END_SCENE

}

#endif
