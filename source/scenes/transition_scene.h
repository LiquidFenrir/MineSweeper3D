#ifndef GAMESCENE_TRANSITION_INC
#define GAMESCENE_TRANSITION_INC

#include "../gamescene.h"

namespace scenes {

START_SCENE(transition_scene)
    next_scene update(const ctr::hid& input, ctr::audio& audio, const double dt) override final;
    void draw(ctr::gfx& gfx) override final;

private:
    transition_scene(game::scenes::scene_ptr before, game::scenes::scene_ptr after);

    enum class state {
        hiding,
        revealing,
        done,
    };
    state current_state{state::hiding};
    int frame{0};
    game::scenes::scene_ptr scene_before, scene_after;
    static constexpr inline int cell_size = 20;
    std::array<unsigned char, (800 / cell_size) * (240 / cell_size)> top_screen_pattern;
    std::array<unsigned char, (320 / cell_size) * (240 / cell_size)> bottom_screen_pattern;
END_SCENE

}

#endif
