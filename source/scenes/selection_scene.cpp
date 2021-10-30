#include "selection_scene.h"
#include "main_menu_scene.h"
#include "transition_scene.h"
#include "room_setup_scene.h"
#include "room_browser_scene.h"
#include "menu_sprites.h"
#include <span>
#include <ranges>
#include <utility>

namespace {

struct menu_structure_t {
    using source_t = ::scenes::selection_scene;
    using tab_t = game::scene_menu<source_t>::menu_tab;
    using entry_t = tab_t::menu_entry;
    std::array<entry_t, 4> entries;
    std::array<tab_t, 1> tabs;
};

static const menu_structure_t menu_structure = []() {
    menu_structure_t out;
    using source_t = menu_structure_t::source_t;
    using entry_t = menu_structure_t::entry_t;

    const entry_t::func_t handler_cancel = [](source_t& scene, const entry_t& ent) -> const entry_t* {
        return &ent.parent_tab->entries.back();
    };
    const entry_t::func_t handlers_validate[] = {
        // singleplayer
        [](source_t& scene, const entry_t&) -> const entry_t* {
            scene.set_next_scene_to(scenes::transition_scene::create(scene.get_ptr(), scenes::room_setup_scene::create(false)));
            return nullptr;
        },
        // host room
        [](source_t& scene, const entry_t&) -> const entry_t* {
            scene.set_next_scene_to(scenes::transition_scene::create(scene.get_ptr(), scenes::room_setup_scene::create(true)));
            return nullptr;
        },
        // join room
        [](source_t& scene, const entry_t&) -> const entry_t* {
            // scene.set_next_scene_to(scenes::transition_scene::create(scene.get_ptr(), scenes::room_browser_scene::create()));
            return nullptr;
        },
        [](source_t& scene, const entry_t&) -> const entry_t* {
            scene.set_next_scene_to(scenes::transition_scene::create(scene.get_ptr(), scenes::main_menu_scene::create()));
            return nullptr;
        },
    };

    int idx = 0;
    for(auto& entry : out.entries)
    {
        entry.entry_index = idx;
        entry.up_entry = &out.entries[(idx + out.entries.size() - 1) % out.entries.size()];
        entry.down_entry = &out.entries[(idx + 1) % out.entries.size()];
        entry.validate_func = handlers_validate[idx];
        entry.cancel_func = handler_cancel;
        entry.parent_tab = &out.tabs[0];
        ++idx;
    }

    out.tabs[0].tab_index = 0;
    out.tabs[0].entries = out.entries;

    return out;
}();

}

scenes::selection_scene::selection_scene()
    : menu(menu_structure.entries[0])
    , entry_index{0}
{
    auto& gen = game_config->data.static_small_text_gen->get_active_buf();
    gen.clear();

    top_text = gen.parse("Pick a way to play!");
    auto& menu_sheet = *game_config->data.menu_sheet;

    constexpr std::pair<C2D_Image game::button::parts::*, std::size_t> parts_idx[] = {
        {&game::button::parts::tl, menu_sprites_button_normal_top_left_idx},
        {&game::button::parts::bl, menu_sprites_button_normal_bottom_left_idx},
        {&game::button::parts::tr, menu_sprites_button_normal_top_right_idx},
        {&game::button::parts::br, menu_sprites_button_normal_bottom_right_idx},
        {&game::button::parts::vl, menu_sprites_button_normal_vertical_left_idx},
        {&game::button::parts::vr, menu_sprites_button_normal_vertical_right_idx},
        {&game::button::parts::ht, menu_sprites_button_normal_horizontal_top_idx},
        {&game::button::parts::hb, menu_sprites_button_normal_horizontal_bottom_idx},
    };
    button_parts.inner_color = {0xc0, 0xc0, 0xc0, 0xff};
    for(const auto& [part, idx] : parts_idx)
    {
        button_parts.*part = menu_sheet.get_image(idx);
    }

    constexpr std::pair<C2D_Image game::button::parts::*, std::size_t> selected_parts_idx[] = {
        {&game::button::parts::tl, menu_sprites_button_selected_top_left_idx},
        {&game::button::parts::bl, menu_sprites_button_selected_bottom_left_idx},
        {&game::button::parts::tr, menu_sprites_button_selected_top_right_idx},
        {&game::button::parts::br, menu_sprites_button_selected_bottom_right_idx},
        {&game::button::parts::vl, menu_sprites_button_selected_vertical_left_idx},
        {&game::button::parts::vr, menu_sprites_button_selected_vertical_right_idx},
        {&game::button::parts::ht, menu_sprites_button_selected_horizontal_top_idx},
        {&game::button::parts::hb, menu_sprites_button_selected_horizontal_bottom_idx},
    };
    selected_button_parts.inner_color = {0x50, 0x50, 0x50, 0xff};
    for(const auto& [part, idx] : selected_parts_idx)
    {
        selected_button_parts.*part = menu_sheet.get_image(idx);
    }

    const int x = 30;
    const int start_y = 20;
    const float depth = 0.0625f;
    const int w = 260;
    const int h = 45;
    const int stride = 50;

    int y = start_y;
    int idx = 0;
    const char* texts[] = {
        "SINGLEPLAYER",
        "HOST ROOM",
        "JOIN ROOM",
        "BACK",
    };
    for(auto& but : buttons.buttons)
    {
        but.depth = depth;
        but.x = x;
        but.w = w;
        but.h = h;
        but.y = y;
        but.selectable = true;
        but.associated_menu_entry = &menu_structure.entries[idx];
        buttons.text[idx] = gen.parse(texts[idx]);
        y += stride;
        ++idx;
    }
}

void scenes::selection_scene::now_at(const int entry_index, const int tab_index)
{
    this->entry_index = entry_index;
}

void scenes::selection_scene::set_next_scene_to(scene_ptr ptr)
{
    next = std::move(ptr);
}

game::scenes::next_scene scenes::selection_scene::update(const ctr::hid& input, ctr::audio& audio, const double dt)
{
    if(!buttons.react(*this, input, audio, menu))
        menu.react(*this, input, audio, game_config->conf.keymap_menu);
    if(next)
    {
        auto out = std::move(*next);
        next.reset();
        return out;
    }
    else
        return std::nullopt;
}

void scenes::selection_scene::draw(ctr::gfx& gfx)
{
    gfx.render_2d();
    gfx.get_screen(GFX_TOP, GFX_LEFT)->focus();
    C2D_DrawImageAt(game_config->data.top_bg_full, 0, 0, 0);

    constexpr u32 shadow_color = C2D_Color32f(0.375f, 0.375f, 0.375f, 1.0f);
    constexpr u32 text_color = C2D_Color32f(0, 0, 0, 1);

    C2D_DrawText(&top_text, C2D_AlignCenter | C2D_WithColor, 200.0f + 2, 120.0f - 12 + 2, 0.25f, 1.0f, 1.0f, shadow_color);
    C2D_DrawText(&top_text, C2D_AlignCenter | C2D_WithColor, 200.0f, 120.0f - 12, 0.5f, 1.0f, 1.0f, text_color);

    C2D_Flush();
    gfx.get_screen(GFX_BOTTOM)->focus();
    C2D_DrawImageAt(game_config->data.bottom_bg_full, 0, 0, 0);

    for(int i = 0; i < buttons.Size; ++i)
    {
        const auto& button = buttons.buttons[i];
        button.draw(i == entry_index ? selected_button_parts : button_parts);
        const C2D_Text* txt = &buttons.text[i];
        float textH = 0.0f;
        C2D_TextGetDimensions(txt, 1.0f, 1.0f, nullptr, &textH);
        const int y = button.y + (button.h - textH)/2 + button.h/4;
        C2D_DrawText(txt, C2D_AlignCenter | C2D_WithColor, 160.0f + 2, y + 2, 0.25f, 1.0f, 1.0f, shadow_color);
        C2D_DrawText(txt, C2D_AlignCenter | C2D_WithColor, 160.0f, y, 0.5f, 1.0f, 1.0f, text_color);
    }
}
