#include "intro_scene.h"
#include "main_menu_scene.h"
#include "intro_sprites.h"
#include "menu_sprites.h"
#include <span>
#include <ranges>
#include <utility>

namespace {

struct letter_info {
    std::size_t image_id;
    scenes::intro_scene::letter::pos_timing pt;
};
#define PART(pos, center) {pos, center, 0.5f, 0.0f}

#define IMG_DP(x, y) {x, y, 64, 64}
#define LETTER_DP(x, y) {x, y, 32, 32}
#define ICON_DP(x, y) {x, y, 0, 0}

#define IMG_CENTER {32,32}
#define LETTER_CENTER {0,0}
#define ICON_CENTER {0,0}

constexpr static inline const letter_info letters_info[] = {
    {intro_sprites_flag_big_idx, {PART(IMG_DP(16 + 32, (240 - 64)/2 + 32 - 90), IMG_CENTER), {}}},
    {intro_sprites_mine_big_idx, {PART(IMG_DP(400 - 64 - 16 + 32, (240 - 64)/2 + 32 + 90), IMG_CENTER), {}}},

    {intro_sprites_logo_M_idx, {PART(LETTER_DP(200 - (32 * 4)/2 + 32 * 0, 120 - 16 - 11), LETTER_CENTER), {}}},
    {intro_sprites_logo_i_idx, {PART(LETTER_DP(200 - (32 * 4)/2 + 32 * 1, 120 - 16 - 11), LETTER_CENTER), {}}},
    {intro_sprites_logo_n_idx, {PART(LETTER_DP(200 - (32 * 4)/2 + 32 * 2, 120 - 16 - 11), LETTER_CENTER), {}}},
    {intro_sprites_logo_e_idx, {PART(LETTER_DP(200 - (32 * 4)/2 + 32 * 3, 120 - 16 - 11), LETTER_CENTER), {}}},

    {intro_sprites_logo_s_idx, {PART(LETTER_DP(200 - (32 * 7)/2 + 32 * 0, 120 + 16 - 11), LETTER_CENTER), {}}},
    {intro_sprites_logo_w_idx, {PART(LETTER_DP(200 - (32 * 7)/2 + 32 * 1, 120 + 16 - 11), LETTER_CENTER), {}}},
    {intro_sprites_logo_e_idx, {PART(LETTER_DP(200 - (32 * 7)/2 + 32 * 2, 120 + 16 - 11), LETTER_CENTER), {}}},
    {intro_sprites_logo_e_idx, {PART(LETTER_DP(200 - (32 * 7)/2 + 32 * 3, 120 + 16 - 11), LETTER_CENTER), {}}},
    {intro_sprites_logo_p_idx, {PART(LETTER_DP(200 - (32 * 7)/2 + 32 * 4, 120 + 19 - 11), LETTER_CENTER), {}}},
    {intro_sprites_logo_e_idx, {PART(LETTER_DP(200 - (32 * 7)/2 + 32 * 5, 120 + 16 - 11), LETTER_CENTER), {}}},
    {intro_sprites_logo_r_idx, {PART(LETTER_DP(200 - (32 * 7)/2 + 32 * 6, 120 + 17 - 11), LETTER_CENTER), {}}},

    {intro_sprites_logo_3D_idx, {PART(ICON_DP(200, 128), ICON_CENTER), {}}},
};
using letter_modifier = bool (*)(scenes::intro_scene::letter::pos_timing& pt, const int frame, const int index);

constexpr static inline letter_modifier modify_flag = [](scenes::intro_scene::letter::pos_timing& pt, const int frame, const int index)
{
    if(frame > 90) return false;

    const float t = (frame / 90.0f);
    pt.draw_params.pos.y += std::ceil(1 - t);

    C2D_AlphaImageTint(&pt.tint, t);

    pt.draw_params.angle = M_TAU * t;

    return true;
};
constexpr static inline letter_modifier modify_mine = [](scenes::intro_scene::letter::pos_timing& pt, const int frame, const int index)
{
    if(frame > 90) return false;

    const float t = (frame / 90.0f);
    pt.draw_params.pos.y -= std::ceil(1 - t);

    C2D_AlphaImageTint(&pt.tint, t);

    pt.draw_params.angle = -M_TAU * t;

    return true;
};
constexpr static inline letter_modifier modify_top_letter = [](scenes::intro_scene::letter::pos_timing& pt, const int frame, const int index)
{
    const int start_time = 30 + index * 2;
    const int stop_time = start_time + 15;
    if(frame >= stop_time)
    {
        C2D_PlainImageTint(&pt.tint, C2D_Color32(0,0,0, 255), 1.0f);
        return false;
    }
    else if(frame >= start_time)
    {
        const int f = frame - start_time;
        const float t = f/(stop_time - start_time);
        C2D_PlainImageTint(&pt.tint, C2D_Color32(0,0,0, 255 * t), 1.0f);
        pt.draw_params.pos.y -= 1;
        return true;
    }
    else
    {
        C2D_PlainImageTint(&pt.tint, C2D_Color32(0,0,0,0), 1.0f);
        return false;
    }
};
constexpr static inline letter_modifier modify_bottom_letter = [](scenes::intro_scene::letter::pos_timing& pt, const int frame, const int index)
{
    const int start_time = 30 + index * 2;
    const int stop_time = start_time + 15;
    if(frame >= stop_time)
    {
        C2D_PlainImageTint(&pt.tint, C2D_Color32(24,24,24, 255), 1.0f);
        return false;
    }
    else if(frame >= start_time)
    {
        const int f = frame - start_time;
        const float t = f/(stop_time - start_time);
        C2D_PlainImageTint(&pt.tint, C2D_Color32(24,24,24, 255 * t), 1.0f);
        pt.draw_params.pos.y += 1;
        return true;
    }
    else
    {
        C2D_PlainImageTint(&pt.tint, C2D_Color32(24,24,24,0), 1.0f);
        return false;
    }
};
constexpr static inline letter_modifier modify_3D_logo = [](scenes::intro_scene::letter::pos_timing& pt, const int frame, const int index)
{
    if(frame >= 100)
    {
        return false;
    }
    else if(frame >= (30 + (15) * 3))
    {
        C2D_AlphaImageTint(&pt.tint, 1.0f);
        const int f = (frame - (30 + (15) * 3));
        const float t = f/25.0f;
        pt.draw_params.pos.w = t * 64;
        pt.draw_params.pos.h = t * 64;
        pt.draw_params.center.x = pt.draw_params.pos.w/2.0f;
        pt.draw_params.center.y = pt.draw_params.pos.h/2.0f;
        return true;
    }
    else
    {
        return false;
    }
};

constexpr static inline const letter_modifier modifiers[] = {
    modify_flag,
    modify_mine,

    modify_top_letter,
    modify_top_letter,
    modify_top_letter,
    modify_top_letter,

    modify_bottom_letter,
    modify_bottom_letter,
    modify_bottom_letter,
    modify_bottom_letter,
    modify_bottom_letter,
    modify_bottom_letter,
    modify_bottom_letter,

    modify_3D_logo,
};

}

scenes::intro_scene::intro_scene()
{
    clear_color_bottom = ctr::gfx::color{0,0,0,0xff};

    auto& sheet = *game_config->data.intro_sheet;
    dev_logo = sheet.get_image(intro_sprites_logo_dev_idx);
    for(int i = 0; i < 14; ++i)
    {
        auto& cur_letter = letter_sprites[i];
        const auto& cur_info = letters_info[i];
        cur_letter.img = sheet.get_image(cur_info.image_id);
        cur_letter.pt = cur_info.pt;
    }
}

game::scenes::next_scene scenes::intro_scene::update(const ctr::hid& input, ctr::audio& audio, const double dt)
{
    if(input.pressed().any())
    {
main_menu:
        // audio.play_sfx("", 1);
        return ::scenes::main_menu_scene::create();
    }

    bool any_changed = false;
    for(int i = 0; i < 14; ++i)
    {
        any_changed |= modifiers[i](letter_sprites[i].pt, frame, i);
    }

    if(frame == 0)
        audio.play_sfx("intro", 1);

    ++frame;
    if(any_changed)
    {
        return std::nullopt;
    }
    else
    {
        goto main_menu;
    }
}

void scenes::intro_scene::draw(ctr::gfx& gfx)
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
    C2D_DrawImageAt(dev_logo, (320 - dev_logo.subtex->width)/2, (240 - dev_logo.subtex->height)/2, 0);
}

game::scenes::scene_ptr game::scenes::get_default_scene()
{
    game_config->data.ingame_sheet.emplace(C2D_SpriteSheetLoad("romfs:/gfx/ingame_sprites.t3x"));
    // game_config->data.text_font_big.emplace("romfs:/gfx/PixelOperatorMono8-Bold_18.bcfnt");
    game_config->data.text_font_small.emplace("romfs:/gfx/PixelOperatorMono8-Bold_12.bcfnt");
    // game_config->data.static_text_gen.emplace(1024, &*game_config->data.text_font_big); // reset on scene entry
    // game_config->data.dynamic_text_gen.emplace(1024, &*game_config->data.text_font_big); // reset every frame
    game_config->data.static_small_text_gen.emplace(1024, &*game_config->data.text_font_small); // reset on scene entry
    game_config->data.dynamic_small_text_gen.emplace(1024, &*game_config->data.text_font_small); // reset every frame

    auto& menu_sheet = game_config->data.menu_sheet.emplace(C2D_SpriteSheetLoad("romfs:/gfx/menu_sprites.t3x"));
    game_config->data.top_bg_full = menu_sheet.get_image(menu_sprites_top_screen_background_idx);
    game_config->data.bottom_bg_full = menu_sheet.get_image(menu_sprites_bottom_screen_background_idx);

    game_config->data.intro_sheet.emplace(C2D_SpriteSheetLoad("romfs:/gfx/intro_sprites.t3x"));

    if(game_config->conf.skip_intro)
        return ::scenes::main_menu_scene::create();
    else
        return ::scenes::intro_scene::create();
}
