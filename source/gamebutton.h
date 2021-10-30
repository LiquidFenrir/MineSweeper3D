#ifndef GAMEBUTTON_INC
#define GAMEBUTTON_INC

#include "ctrgfx.h"
#include "ctrhid.h"
#include "gamescene.h"

namespace game {

struct button {
    int x, y;
    float depth;
    int w, h;

    struct parts {
        C2D_Image tl, tr, bl, br;
        C2D_Image ht, hb, vl, vr;
        ctr::gfx::color inner_color;
    };

    void draw(const parts& using_parts) const;
};
template<class T>
struct menu_button : public button {
    const game::scene_menu<T>::menu_tab::menu_entry* associated_menu_entry;
    bool selectable;
};

template<class T, std::size_t N>
struct button_page {
    menu_button<T> buttons[N];

    bool react(T& source, const ctr::hid& input, ctr::audio& audio, game::scene_menu<T>& associated_menu)
    {
        if(input.touch_active())
        {
            for(auto& b : buttons)
            {
                if(b.selectable && input.touching_offset({b.x, b.y}, {b.w, b.h}))
                {
                    if(auto ent = b.associated_menu_entry)
                    {
                        if(associated_menu.get_current_entry() == ent)
                        {
                            if((ent = ent->validate_func(source, *ent)))
                            {
                                goto change_entry;
                            }
                            else
                            {
                                audio.play_sfx("selection_click", 2);
                            }
                        }
                        else
                        {
change_entry:
                            audio.play_sfx("selection_move", 3);
                            associated_menu.jump_to(source, ent);
                        }
                    }
                    return true;
                }
            }
        }
        return false;
    }
};

}

#endif
