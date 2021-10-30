#ifndef GAMESCENE_SETTINGS_INC
#define GAMESCENE_SETTINGS_INC

#include "../gamescene.h"

namespace scenes {

START_SCENE(settings_scene)
    next_scene update(const ctr::hid& input, ctr::audio& audio, const double dt) override final;
    void draw(ctr::gfx& gfx) override final;

private:
    settings_scene();
END_SCENE

}

#endif
