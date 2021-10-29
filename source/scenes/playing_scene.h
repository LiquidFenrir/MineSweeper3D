#ifndef GAMESCENE_PLAYING_INC
#define GAMESCENE_PLAYING_INC

#include "../gamescene.h"
#include "../gameboard.h"

namespace scenes {

START_SCENE(playing_scene)
    next_scene update(const ctr::hid& input, const double dt) override final;
    void draw(ctr::gfx& gfx) override final;

private:
    playing_scene(game::board::numbers n, game::board::mode b, const int self_idx);

    C3D_Mtx projection;
    int point_count;
    game::board::numbers nums;
    game::board::mode board_mode;
    game::player& self;
END_SCENE

}

#endif
