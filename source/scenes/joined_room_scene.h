#ifndef GAMESCENE_JOINED_INC
#define GAMESCENE_JOINED_INC

#include "../gamescene.h"

namespace scenes {

START_SCENE(joined_room_scene)
    next_scene update(const ctr::hid& input, const double dt) override final;
    void draw(ctr::gfx& gfx) override final;

private:
    joined_room_scene();
END_SCENE

}

#endif
