#ifndef GAMESCENE_BROWSER_INC
#define GAMESCENE_BROWSER_INC

#include "../gamescene.h"

namespace scenes {

START_SCENE(room_browser_scene)
    next_scene update(const ctr::hid& input, const double dt) override final;
    void draw(ctr::gfx& gfx) override final;

private:
    room_browser_scene();
END_SCENE

}

#endif
