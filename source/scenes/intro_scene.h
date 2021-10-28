#ifndef GAMESCENE_INTRO_INC
#define GAMESCENE_INTRO_INC

#include "../gamescene.h"

namespace scenes {

START_SCENE(intro_scene)
    next_scene update(const ctr::hid& input, const double dt) override final;
    void draw(ctr::gfx& gfx) override final;

    struct letter {
        C2D_Image img;
        struct pos_timing {
            C2D_DrawParams draw_params;
            C2D_ImageTint tint;
        };
        pos_timing pt;
    };

private:
    intro_scene();

    letter letter_sprites[14];
    int frame{0};
    C2D_Image dev_logo;
END_SCENE

}

#endif
