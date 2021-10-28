#include "main_menu_scene.h"
#include "selection_scene.h"
#include "settings_scene.h"
#include "transition_scene.h"
#include "intro_sprites.h"
#include "menu_sprites.h"
#include <span>
#include <ranges>
#include <utility>

namespace {

#define PART(pos, center) {pos, center, 0.125f, 0.0f}

#define IMG_DP(x, y) {x, y, 64, 64}
#define LETTER_DP(x, y) {x, y, 32, 32}
#define ICON_DP(x, y) {x, y, 64, 64}

#define IMG_CENTER {32,32}
#define LETTER_CENTER {0,0}
#define ICON_CENTER {32,32}

using tint_maker = void(*)(C2D_ImageTint&);
constexpr static inline tint_maker image_tint_maker = [](C2D_ImageTint& tint)
{
    C2D_AlphaImageTint(&tint, 1.0f);
};
constexpr static inline tint_maker top_text_tint_maker = [](C2D_ImageTint& tint)
{
    C2D_PlainImageTint(&tint, C2D_Color32(0,0,0, 255), 1.0f);
};
constexpr static inline tint_maker bottom_text_tint_maker = [](C2D_ImageTint& tint)
{
    C2D_PlainImageTint(&tint, C2D_Color32(24,24,24, 255), 1.0f);
};
struct letter_info {
    std::size_t image_id;
    scenes::main_menu_scene::letter::pos_timing pt;
    tint_maker tinter;
};

constexpr static inline letter_info letters_info[] = {
    {intro_sprites_flag_big_idx, {PART(IMG_DP(16 + 32, (240 - 64)/2 + 32), IMG_CENTER), {}}, image_tint_maker},
    {intro_sprites_mine_big_idx, {PART(IMG_DP(400 - 64 - 16 + 32, (240 - 64)/2 + 32), IMG_CENTER), {}}, image_tint_maker},

    {intro_sprites_logo_M_idx, {PART(LETTER_DP(200 - (32 * 4)/2 + 32 * 0, 120 - 31 - 11), LETTER_CENTER), {}}, top_text_tint_maker},
    {intro_sprites_logo_i_idx, {PART(LETTER_DP(200 - (32 * 4)/2 + 32 * 1, 120 - 31 - 11), LETTER_CENTER), {}}, top_text_tint_maker},
    {intro_sprites_logo_n_idx, {PART(LETTER_DP(200 - (32 * 4)/2 + 32 * 2, 120 - 31 - 11), LETTER_CENTER), {}}, top_text_tint_maker},
    {intro_sprites_logo_e_idx, {PART(LETTER_DP(200 - (32 * 4)/2 + 32 * 3, 120 - 31 - 11), LETTER_CENTER), {}}, top_text_tint_maker},

    {intro_sprites_logo_s_idx, {PART(LETTER_DP(200 - (32 * 7)/2 + 32 * 0, 120 + 31 - 11), LETTER_CENTER), {}}, bottom_text_tint_maker},
    {intro_sprites_logo_w_idx, {PART(LETTER_DP(200 - (32 * 7)/2 + 32 * 1, 120 + 31 - 11), LETTER_CENTER), {}}, bottom_text_tint_maker},
    {intro_sprites_logo_e_idx, {PART(LETTER_DP(200 - (32 * 7)/2 + 32 * 2, 120 + 31 - 11), LETTER_CENTER), {}}, bottom_text_tint_maker},
    {intro_sprites_logo_e_idx, {PART(LETTER_DP(200 - (32 * 7)/2 + 32 * 3, 120 + 31 - 11), LETTER_CENTER), {}}, bottom_text_tint_maker},
    {intro_sprites_logo_p_idx, {PART(LETTER_DP(200 - (32 * 7)/2 + 32 * 4, 120 + 31 + 3 - 11), LETTER_CENTER), {}}, bottom_text_tint_maker},
    {intro_sprites_logo_e_idx, {PART(LETTER_DP(200 - (32 * 7)/2 + 32 * 5, 120 + 31 - 11), LETTER_CENTER), {}}, bottom_text_tint_maker},
    {intro_sprites_logo_r_idx, {PART(LETTER_DP(200 - (32 * 7)/2 + 32 * 6, 120 + 31 + 1 - 11), LETTER_CENTER), {}}, bottom_text_tint_maker},

    {intro_sprites_logo_3D_idx, {PART(ICON_DP(200, 128), ICON_CENTER), {}}, image_tint_maker},
};

struct menu_structure_t {
    using source_t = ::scenes::main_menu_scene;
    using tab_t = game::scene_menu<source_t>::menu_tab;
    using entry_t = tab_t::menu_entry;
    std::array<entry_t, 3> entries;
    std::array<tab_t, 1> tabs;
};

/*
static void initialize_game_board_data(game::config* game_config)
{
}
*/

static const menu_structure_t menu_structure = []() {
    menu_structure_t out;
    using source_t = menu_structure_t::source_t;
    using entry_t = menu_structure_t::entry_t;

    const entry_t::func_t handler_cancel = [](source_t& scene, const entry_t& ent) -> const entry_t* {
        return &ent.parent_tab->entries.back();
    };

    out.entries[0].entry_index = 0;
    out.entries[0].up_entry = &out.entries[2];
    out.entries[0].down_entry = &out.entries[1];
    out.entries[0].validate_func = [](source_t& scene, const entry_t&) -> const entry_t* {
        scene.set_next_scene_to(scenes::transition_scene::create(scene.get_ptr(), scenes::selection_scene::create()));
        return nullptr;
    };
    out.entries[0].cancel_func = handler_cancel;

    out.entries[1].entry_index = 1;
    out.entries[1].up_entry = &out.entries[0];
    out.entries[1].down_entry = &out.entries[2];
    out.entries[1].validate_func = [](source_t& scene, const entry_t&) -> const entry_t* {
        // scene.set_next_scene_to(scenes::transition_scene::create(scene.get_ptr(), scenes::settings_scene::create()));
        return nullptr;
    };
    out.entries[0].cancel_func = handler_cancel;

    out.entries[2].entry_index = 2;
    out.entries[2].up_entry = &out.entries[1];
    out.entries[2].down_entry = &out.entries[0];
    out.entries[2].validate_func = [](source_t& scene, const entry_t&) -> const entry_t* {
        scene.set_next_scene_to(nullptr);
        return nullptr;
    };

    for(auto& e : out.entries)
    {
        e.parent_tab = &out.tabs[0];
    }

    out.tabs[0].tab_index = 0;
    out.tabs[0].entries = out.entries;

    return out;
}();

}

#ifndef GAME_VERSION_STRING
#define GAME_VERSION_STRING "MineSweeper3D vDebug"
#endif

scenes::main_menu_scene::main_menu_scene()
    : menu(menu_structure.entries[0])
    , entry_index{0}
{
    auto& gen = game_config->data.static_small_text_gen->get_active_buf();
    gen.clear();

    auto& sheet = *game_config->data.intro_sheet;
    auto& menu_sheet = *game_config->data.menu_sheet;

    for(int i = 0; i < 14; ++i)
    {
        auto& cur_letter = letter_sprites[i];
        const auto& cur_info = letters_info[i];
        cur_letter.img = sheet.get_image(cur_info.image_id);
        cur_letter.pt = cur_info.pt;
        cur_info.tinter(cur_letter.pt.tint);
    }

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
    const int h = 60;
    const int stride = 70;

    int y = start_y;
    int idx = 0;
    const char* texts[] = {
        "PLAY",
        "SETTINGS",
        "EXIT",
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
    C2D_TextFontParse(&version_text, game_config->data.text_font_small->fnt, gen.buf.buf, GAME_VERSION_STRING);
}

void scenes::main_menu_scene::now_at(const int entry_index, const int tab_index)
{
    this->entry_index = entry_index;
}

void scenes::main_menu_scene::set_next_scene_to(scene_ptr ptr)
{
    next = std::move(ptr);
}

game::scenes::next_scene scenes::main_menu_scene::update(const ctr::hid& input, const double dt)
{
    if(!buttons.react(*this, input, menu))
        menu.react(*this, input, game_config->keymap_menu);
    if(next)
    {
        auto out = std::move(next.value());
        next.reset();
        return out;
    }
    else
        return std::nullopt;
}

void scenes::main_menu_scene::draw(ctr::gfx& gfx)
{
    gfx.render_2d();
    gfx.get_screen(GFX_TOP, GFX_LEFT)->focus();
    C2D_DrawImageAt(game_config->data.top_bg_full, 0, 0, 0);
    for(const auto& [img, pt] : letter_sprites)
    {
        C2D_DrawImage(img, &pt.draw_params, &pt.tint);
    }

    C2D_Flush();
    gfx.get_screen(GFX_BOTTOM)->focus();
    C2D_DrawImageAt(game_config->data.bottom_bg_full, 0, 0, 0);

    constexpr u32 shadow_color = C2D_Color32f(0.375f, 0.375f, 0.375f, 1.0f);
    constexpr u32 text_color = C2D_Color32f(0, 0, 0, 1);
    for(int i = 0; i < buttons.Size; ++i)
    {
        const auto& button = buttons.buttons[i];
        button.draw(i == entry_index ? selected_button_parts : button_parts);
        const C2D_Text* txt = &buttons.text[i];
        float textH = 0.0f;
        C2D_TextGetDimensions(txt, 1.0f, 1.0f, nullptr, &textH);
        const int y = button.y + (button.h - textH)/2 + button.h/8;
        C2D_DrawText(txt, C2D_AlignCenter | C2D_WithColor, 160.0f + 2, y + 2, 0.25f, 1.0f, 1.0f, shadow_color);
        C2D_DrawText(txt, C2D_AlignCenter | C2D_WithColor, 160.0f, y, 0.5f, 1.0f, 1.0f, text_color);
    }
    C2D_DrawText(&version_text, C2D_AtBaseline | C2D_WithColor, 4.0f, 240.0f - 4.0f, 0.0f, 0.5f, 0.5f, text_color);
}
