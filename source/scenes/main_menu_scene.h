#ifndef GAMESCENE_MAIN_MENU_INC
#define GAMESCENE_MAIN_MENU_INC

#include "../gamescene.h"
#include "../gamebutton.h"

namespace scenes {

START_SCENE(main_menu_scene)
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

    void now_at(const int entry_index, const int tab_index);

    void set_next_scene_to(scene_ptr ptr);

private:
    main_menu_scene();

    game::scene_menu<main_menu_scene> menu;
    next_scene next;

    int entry_index;
    letter letter_sprites[14];
    C2D_Text version_text;

    template<std::size_t N>
    struct button_page_ : public game::button_page<main_menu_scene, N> {
        static constexpr inline int Size = N;
        C2D_Text text[N];
    };
    button_page_<3> buttons;

    game::button::parts button_parts, selected_button_parts;
END_SCENE

}

#endif
