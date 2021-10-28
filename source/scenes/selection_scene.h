#ifndef GAMESCENE_SELECTION_INC
#define GAMESCENE_SELECTION_INC

#include "../gamescene.h"
#include "../gamebutton.h"

namespace scenes {

START_SCENE(selection_scene)
    next_scene update(const ctr::hid& input, const double dt) override final;
    void draw(ctr::gfx& gfx) override final;

    void now_at(const int entry_index, const int tab_index);

    void set_next_scene_to(scene_ptr ptr);

private:
    selection_scene();

    game::scene_menu<selection_scene> menu;
    next_scene next;

    int entry_index;
    template<std::size_t N>
    struct button_page_ : public game::button_page<selection_scene, N> {
        static constexpr inline int Size = N;
        C2D_Text text[N];
    };
    button_page_<4> buttons;
    game::button::parts button_parts, selected_button_parts;
    C2D_Text top_text;
END_SCENE

}

#endif
