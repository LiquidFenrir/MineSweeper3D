#ifndef GAMESCENE_PLAYING_INC
#define GAMESCENE_PLAYING_INC

#include "../gamescene.h"

namespace scenes {

struct playing_scene : public game::scene {
    playing_scene();

    next_scene update(const ctr::hid& input, const double dt) override final;

    void draw(ctr::gfx& gfx) override final;
};

}

#endif
