#include "playing_scene.h"
#include "transition_scene.h"
#include "main_menu_scene.h"
#include "ingame_sprites.h"
#include "menu_sprites.h"
#include "../debugging.h"
#include <algorithm>

namespace foggy {

struct fog {
    float den, gra, near, far;

    constexpr bool operator!=(const fog& other) const
    {
        const auto tuple_magic = [](const fog& fo)
        {
            return std::tuple(fo.den, fo.gra, fo.near, fo.far);
        };
        return tuple_magic(*this) != tuple_magic(other);
    }
};
static fog f{
    1.0f,
    2.0f,
    0.0000625f,
    10.0f,
};
}

scenes::playing_scene::playing_scene(game::board::numbers n, game::board::mode b, const int self_idx)
    : nums{n}
    , board_mode{b}
    , self{game_config->data.players[self_idx]}
    , stop_music{true}
    , selected_tab{0}
{
    clear_color_top = ctr::gfx::color{0x00, 0x94, 0xff, 0xff};
    std::tie(point_count, point_count_margin) = game_config->data.game_board->reset(nums, board_mode, time(nullptr));
    point_count *= 6;
    point_count_margin *= 6;

    GSPGPU_FlushDataCache(game_config->data.board_vbo.get(), point_count * sizeof(game::board::buffer_point));
    if(point_count_margin)
        GSPGPU_FlushDataCache(game_config->data.board_margin_vbo.get(), point_count_margin * sizeof(game::board::buffer_point));

    game_config->data.game_board->fill_cursor_render_buffer(static_cast<game::board::buffer_point*>(game_config->data.board_cursors_vbo.get()));
    GSPGPU_FlushDataCache(game_config->data.board_cursors_vbo.get(), 6 * sizeof(game::board::buffer_point));

    Mtx_PerspTilt(&projection, C3D_AngleFromDegrees(50.0f), C3D_AspectRatioTop, 0.01f, 12.0f, false);

    debugging::log("starting game\n");
    debugging::log("point_count: {}\n", point_count);
    debugging::log("point_count_margin: {}\n", point_count_margin);
    FogLut_Exp(&game_config->data.fog_lut, foggy::f.den, foggy::f.gra, foggy::f.near, foggy::f.far);

    auto& sheet = *game_config->data.ingame_sheet;
    bottom_screen_background_img = sheet.get_image(ingame_sprites_bottom_screen_background_idx);
    normal_tab_img = sheet.get_image(ingame_sprites_game_closed_tab_idx);
    selected_tab_img = sheet.get_image(ingame_sprites_game_open_tab_idx);
    tab_icons[0] = sheet.get_image(ingame_sprites_game_minimap_tab_icon_idx);
    tab_icons[1] = sheet.get_image(ingame_sprites_game_teams_tab_icon_idx);
    tab_icons[2] = sheet.get_image(ingame_sprites_game_numbers_tab_icon_idx);
    minimap_cover = sheet.get_image(ingame_sprites_minimap_tab_overlay_idx);
    no_minimap_img = sheet.get_image(ingame_sprites_minimap_disabled_idx);
    player_indicator = sheet.get_image(ingame_sprites_minimap_player_indicator_idx);
    const std::size_t idxs[] = {
        ingame_sprites_cell_open_idx,
        ingame_sprites_cell_1_idx,
        ingame_sprites_cell_2_idx,
        ingame_sprites_cell_3_idx,
        ingame_sprites_cell_4_idx,
        ingame_sprites_cell_5_idx,
        ingame_sprites_cell_6_idx,
        ingame_sprites_cell_7_idx,
        ingame_sprites_cell_8_idx,
        ingame_sprites_cell_hide_idx,
        ingame_sprites_cell_flag_idx,
        ingame_sprites_cell_mine_idx,
    };
    for(int i = 0; i < 12; ++i)
    {
        minimap_imgs[i] = sheet.get_image(idxs[i]);
    }
    grass_img = sheet.get_image(ingame_sprites_grass_idx);
    cursor_img = sheet.get_image(ingame_sprites_cursor_idx);
    team_won_img = sheet.get_image(ingame_sprites_team_victory_idx);
    team_lost_img = sheet.get_image(ingame_sprites_team_exploded_idx);
    team_playing_img = sheet.get_image(ingame_sprites_team_playing_idx);

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
}

game::scenes::next_scene scenes::playing_scene::update(const ctr::hid& input, ctr::audio& audio, const double dt)
{
    if(stop_music)
    {
        stop_music = false;
        audio.play_bgm();
    }

    constexpr float MV_SPEED = 0.002f;
    constexpr float ROT_SPEED = 0.00075f;

    bool any_change = false;
    const auto advance = [&](const float rad) {
        const float c = std::cos(rad);
        const float s = std::sin(rad);

        self.x += c * MV_SPEED * dt;
        self.y += s * MV_SPEED * dt;

        if(int(board_mode) & 2)
        {
            if(self.x <= -2)
            {
                self.x += nums.dims.width;
            }
            else if(self.x >= (nums.dims.width + 2))
            {
                self.x -= nums.dims.width;
            }
        }
        else
        {
            if(self.x < 0.125f)
            {
                self.x = 0.125f;
            }
            else if(self.x > (nums.dims.width - 0.125f))
            {
                self.x = nums.dims.width - 0.125f;
            }
        }

        if(int(board_mode) & 1)
        {
            if(self.y <= -2)
            {
                self.y += nums.dims.height;
            }
            else if(self.y >= (nums.dims.height + 2))
            {
                self.y -= nums.dims.height;
            }
        }
        else
        {
            if(self.y < 0.125f)
            {
                self.y = 0.125f;
            }
            else if(self.y > (nums.dims.height - 0.125f))
            {
                self.y = nums.dims.height - 0.125f;
            }
        }

        any_change = true;
    };
    const auto lookDir = [&](const int dx, const int dy) {
        self.yaw += dx * ROT_SPEED * dt;
        self.pitch += dy * ROT_SPEED * dt;

        if(self.pitch < -90.0f)
        {
            self.pitch = -90.0f;
        }
        else if(self.pitch > 90.0f)
        {
            self.pitch = 90.0f;
        }

        if(self.yaw < -360.0f)
        {
            self.yaw += 360.0f;
        }
        else if(self.yaw > 360.0f)
        {
            self.yaw -= 360.0f;
        }

        any_change = true;
    };

    const auto cpad = input.left_stick();
    const auto cstick = input.right_stick();
    const auto held = input.held();
    const auto pressed = input.pressed();

    using key_usage = decltype(game::config::config_data::keymap_game)::key_usage;
    const auto& key_map = game_config->conf.keymap_game;

    if(cpad.distance() > 720)
    {
        switch(key_map.map.stick_left)
        {
        case key_usage::movement:
            advance(cpad.angle() + C3D_AngleFromDegrees(self.yaw - 90.0f));
            break;
        case key_usage::camera:
            lookDir(cpad.x(), -cpad.y());
            break;
        default:
            break;
        }
    }

    if(cstick.distance() > 720)
    {
        switch(key_map.map.stick_right)
        {
        case key_usage::movement:
            advance(cstick.angle() + C3D_AngleFromDegrees(self.yaw - 90.0f));
            break;
        case key_usage::camera:
            lookDir(cstick.x(), -cstick.y());
            break;
        default:
            break;
        }
    }

    const auto handle_button = [&](const u32 key, const key_usage usage, const float dirX, const float dirY)
    {
        switch(usage)
        {
        case key_usage::movement:
            if(held.check(key))
            {
                advance(std::atan2(dirY, dirX) + C3D_AngleFromDegrees(self.yaw));
            }
            break;
        case key_usage::camera:
            if(held.check(key))
            {
                lookDir(dirX * 160, dirY * 160);
            }
            break;
        case key_usage::flag:
            if(pressed.check(key) && self.cursor)
            {
                if(auto res = game_config->data.game_board->flag_at({self.cursor->x, self.cursor->y}); res.first)
                {
                    GSPGPU_FlushDataCache(game_config->data.board_vbo.get(), point_count * sizeof(game::board::buffer_point));
                    if(res.second)
                    {
                        audio.play_sfx("place_flag", 6);
                    }
                    else
                    {
                        audio.play_sfx("remove_flag", 7);
                    }
                }
            }
            break;
        case key_usage::reveal:
            if(pressed.check(key) && self.cursor)
            {
                if(game_config->data.game_board->reveal_at({self.cursor->x, self.cursor->y}))
                {
                    GSPGPU_FlushDataCache(game_config->data.board_vbo.get(), point_count * sizeof(game::board::buffer_point));
                    const auto state = game_config->data.game_board->get_current_state();
                    if(state == game::board::state::lost)
                    {
                        game_config->data.teams[self.team_id - 1].current_state = game::team::state::lost;
                        audio.play_sfx("explosion", 1);
                    }
                    else if(state == game::board::state::won)
                    {
                        game_config->data.teams[self.team_id - 1].current_state = game::team::state::won;
                        audio.play_sfx("victory", 4);
                    }
                    else
                    {
                        audio.play_sfx("reveal", 5);
                    }
                }
            }
            break;
        case key_usage::tab_prev:
            if(pressed.check(key))
            {
                selected_tab = (selected_tab + 3 - 1) % 3;
            }
            break;
        case key_usage::tab_next:
            if(pressed.check(key))
            {
                selected_tab = (selected_tab + 1) % 3;
            }
            break;
        }
    };

    handle_button(KEY_DUP, key_map.map.dpad_up, 0, -1);
    handle_button(KEY_DDOWN, key_map.map.dpad_down, 0, 1);
    handle_button(KEY_DLEFT, key_map.map.dpad_left, -1, 0);
    handle_button(KEY_DRIGHT, key_map.map.dpad_right, 1, 0);

    handle_button(KEY_A, key_map.map.button_a, 1, 0);
    handle_button(KEY_B, key_map.map.button_b, 0, 1);
    handle_button(KEY_X, key_map.map.button_x, 0, -1);
    handle_button(KEY_Y, key_map.map.button_y, -1, 0);

    handle_button(KEY_L, key_map.map.shoulder_l, -1, 0);
    handle_button(KEY_R, key_map.map.shoulder_r, 1, 0);
    handle_button(KEY_ZL, key_map.map.trigger_l, 0, 1);
    handle_button(KEY_ZR, key_map.map.trigger_r, 0, -1);

    if(held.check(KEY_START)) return ::scenes::transition_scene::create(get_ptr(), ::scenes::main_menu_scene::create());
    if(input.touch_active())
    {
        for(int i = 0; i < 3; ++i)
        {
            if(input.touching_offset({u16(i * 106 + 2), 0}, {104, 48}))
            {
                selected_tab = i;
                break;
            }
        }
    }

    if(any_change)
    {
        game_config->data.game_board->fill_cursor_positions(game_config->data.players, self.team_id);
    }

    return std::nullopt;
}

void scenes::playing_scene::draw(ctr::gfx& gfx)
{
    bool drop = false;
    bool fog = false;

    switch(game_config->conf.draw_mode)
    {
    case game::config::distance_draw_mode::fog:
        gfx.render_3d(*game_config->data.board_shader_basic);
        fog = true;
        break;
    case game::config::distance_draw_mode::drop:
        gfx.render_3d(*game_config->data.board_shader_drop);
        drop = true;
        break;
    }
    gfx.get_screen(GFX_TOP, GFX_LEFT)->focus();
    // C3D_CullFace(GPU_CULL_NONE);

    C3D_SetAttrInfo(&game_config->data.board_attr);

    if(fog)
    {
        C3D_FogGasMode(GPU_FOG, GPU_DEPTH_DENSITY, false);
        // C3D_FogColor(0xD8B068);
        C3D_FogColor(0xff9400);
        C3D_FogLutBind(&game_config->data.fog_lut);
    }
    else
    {
        C3D_FogGasMode(GPU_NO_FOG, GPU_PLAIN_DENSITY, false);
        C3D_FogLutBind(nullptr);
    }

    C3D_TexEnv* env = C3D_GetTexEnv(0);
    C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR);
    C3D_TexEnvFunc(env, C3D_RGB, GPU_MODULATE);

    C3D_Mtx modelView;

    const auto do_render = [&](const float iod) {
        C3D_Mtx actual_proj;
        if(iod)
            Mtx_PerspStereoTilt(&actual_proj, C3D_AngleFromDegrees(50.0f), C3D_AspectRatioTop, 0.01f, 14.0f, iod, 2.0f, false);
        else
            Mtx_Copy(&actual_proj, &projection);

        if(fog)
        {
            Mtx_Identity(&modelView);
            Mtx_RotateX(&modelView, C3D_AngleFromDegrees(self.pitch), true);
            Mtx_RotateY(&modelView, C3D_AngleFromDegrees(self.yaw), true);
            Mtx_Translate(&modelView, -self.x, 0.0f, -self.y, true);

            C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, game_config->data.board_shader_basic->uniforms[0], &actual_proj);
            C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, game_config->data.board_shader_basic->uniforms[1], &modelView);

            C3D_FVUnifSet(GPU_VERTEX_SHADER, game_config->data.board_shader_basic->uniforms[2], 1.0f, 1.0f, 1.0f, 1.0f);
        }
        else if(drop)
        {
            C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, game_config->data.board_shader_drop->uniforms[0], &actual_proj);

            Mtx_Identity(&modelView);
            Mtx_RotateX(&modelView, C3D_AngleFromDegrees(self.pitch), true);
            Mtx_RotateY(&modelView, C3D_AngleFromDegrees(self.yaw), true);
            C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, game_config->data.board_shader_drop->uniforms[1], &modelView);

            Mtx_Identity(&modelView);
            Mtx_Translate(&modelView, -self.x, 0.0f, -self.y, false);
            C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, game_config->data.board_shader_drop->uniforms[2], &modelView);

            C3D_FVUnifSet(GPU_VERTEX_SHADER, game_config->data.board_shader_drop->uniforms[3], 1.0f, 1.0f, 1.0f, 1.0f);
        }

        C3D_Mtx modelcopy;
        Mtx_Copy(&modelcopy, &modelView);

        game_config->data.sheet_3d->bind(0);

        if(board_mode == game::board::mode::regular)
        {
            if(point_count_margin)
            {
                C3D_SetBufInfo(&game_config->data.board_margin_vbo_buf);
                C3D_DrawArrays(GPU_TRIANGLES, 0, point_count_margin);
            }
            C3D_SetBufInfo(&game_config->data.board_vbo_buf);
            C3D_DrawArrays(GPU_TRIANGLES, 0, point_count);
        }
        else
        {
            game::board::numbers::size offs[5];
            int num_offs = 1;
            offs[0] = {0, 0};

            if(board_mode == game::board::mode::loop_vertical)
            {
                if(self.y <= 8)
                    offs[num_offs++] = {0, -nums.dims.height};
                if(self.y >= (nums.dims.height - 8))
                    offs[num_offs++] = {0, +nums.dims.height};
            }
            else if(board_mode == game::board::mode::loop_horizontal)
            {
                if(self.x <= 8)
                    offs[num_offs++] = {-nums.dims.width, 0};
                if(self.x >= (nums.dims.width - 8))
                    offs[num_offs++] = {+nums.dims.width, 0};
            }
            else if(board_mode == game::board::mode::loop_both)
            {
                if(self.y <= 8 && self.x <= 8)
                {
                    offs[num_offs++] = {-nums.dims.width, -nums.dims.height};
                }
                if(self.y <= 8 && self.x >= (nums.dims.width - 8))
                {
                    offs[num_offs++] = {nums.dims.width, -nums.dims.height};
                }
                if(self.y >= (nums.dims.height - 8) && self.x <= 8)
                {
                    offs[num_offs++] = {-nums.dims.width, nums.dims.height};
                }
                if(self.y >= (nums.dims.height - 8) && self.x >= (nums.dims.width - 8))
                {
                    offs[num_offs++] = {nums.dims.width, nums.dims.height};
                }
                if(self.x <= 8)
                {
                    offs[num_offs++] = {-nums.dims.width, 0};
                }
                if(self.x >= (nums.dims.width - 8))
                {
                    offs[num_offs++] = {+nums.dims.width, 0};
                }
                if(self.y <= 8)
                {
                    offs[num_offs++] = {0, -nums.dims.height};
                }
                if(self.y >= (nums.dims.height - 8))
                {
                    offs[num_offs++] = {0, +nums.dims.height};
                }
            }

            if(point_count_margin)
            {
                C3D_SetBufInfo(&game_config->data.board_margin_vbo_buf);
                for(int i = 0; i < num_offs; ++i)
                {
                    Mtx_Copy(&modelView, &modelcopy);
                    const auto& pt = offs[i];
                    Mtx_Translate(&modelView, pt.width, 0.0f, pt.height, true);
                    if(fog)
                    {
                        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, game_config->data.board_shader_basic->uniforms[1], &modelView);
                    }
                    else if(drop)
                    {
                        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, game_config->data.board_shader_drop->uniforms[2], &modelView);
                    }

                    C3D_DrawArrays(GPU_TRIANGLES, 0, point_count_margin);
                }
            }

            C3D_SetBufInfo(&game_config->data.board_vbo_buf);
            for(int i = 0; i < num_offs; ++i)
            {
                Mtx_Copy(&modelView, &modelcopy);
                const auto& pt = offs[i];
                Mtx_Translate(&modelView, pt.width, 0.0f, pt.height, true);
                if(fog)
                {
                    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, game_config->data.board_shader_basic->uniforms[1], &modelView);
                }
                else if(drop)
                {
                    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, game_config->data.board_shader_drop->uniforms[2], &modelView);
                }

                C3D_DrawArrays(GPU_TRIANGLES, 0, point_count);
            }
        }

        C3D_SetBufInfo(&game_config->data.board_cursors_vbo_buf);
        game_config->data.sheet_cursors->bind(0);
        for(const auto& player : game_config->data.players)
        {
            if(player.team_id != self.team_id || !player.cursor) continue;

            const auto color = (C3D_FVec)game_config->data.player_colors[player.color_index];
            Mtx_Copy(&modelView, &modelcopy);
            Mtx_Translate(&modelView, player.cursor->x + 0.5f, 0.0f, player.cursor->y + 0.5f, true);

            if(fog)
            {
                C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, game_config->data.board_shader_basic->uniforms[1], &modelView);
                C3D_FVUnifSet(GPU_VERTEX_SHADER, game_config->data.board_shader_basic->uniforms[2], color.x, color.y, color.z, color.w);
            }
            else if(drop)
            {
                C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, game_config->data.board_shader_drop->uniforms[2], &modelView);
                C3D_FVUnifSet(GPU_VERTEX_SHADER, game_config->data.board_shader_drop->uniforms[3], color.x, color.y, color.z, color.w);
            }

            C3D_DrawArrays(GPU_TRIANGLES, 0, 6);
        }
    };

    const float iod = gfx.stereo();
    if(iod == 0.0f)
    {
        do_render(0.0f);
    }
    else
    {
        do_render(-iod);
        gfx.get_screen(GFX_TOP, GFX_RIGHT)->focus();
        do_render(iod);
    }

    if(fog)
    {
        C3D_FogGasMode(GPU_NO_FOG, GPU_PLAIN_DENSITY, false);
        C3D_FogLutBind(nullptr);
    }

    gfx.get_screen(GFX_BOTTOM, GFX_LEFT)->focus();
    gfx.render_2d();

    C2D_DrawImageAt(bottom_screen_background_img, 0, 0, 0);
    for(int i = 0; i < 3; ++i)
    {
        const int x = 106 * i;
        const int y = 0;
        C2D_DrawImageAt((i == selected_tab) ? selected_tab_img : normal_tab_img, x, y, 0.0625f);
        C2D_DrawImageAt(tab_icons[i], x + 30, y + 1, 0.125f);
    }

    switch(selected_tab)
    {
    case 0: // minimap
        if(game_config->conf.enable_minimap)
        {
            C2D_Flush();
            C3D_SetScissor(GPU_SCISSOR_NORMAL, 11, 80, 171, 240);

            std::optional<int> x_min, x_max, y_min, y_max;
            switch(board_mode)
            {
            case game::board::mode::regular:
                x_min = 0;
                x_max = nums.dims.width;
                y_min = 0;
                y_max = nums.dims.height;
                break;
            case game::board::mode::loop_vertical:
                x_min = 0;
                x_max = nums.dims.width;
                break;
            case game::board::mode::loop_horizontal:
                y_min = 0;
                y_max = nums.dims.height;
                break;
            case game::board::mode::loop_both:
                break;
            }

            for(int dy = -5; dy <= +5; ++dy)
            {
                game::board::location pos;
                pos.y = self.y + dy;
                const bool ok_y_min = !(y_min && pos.y < *y_min);
                const bool ok_y_max = !(y_max && pos.y >= *y_max);
                for(int dx = -5; dx <= +5; ++dx)
                {
                    pos.x = self.x + dx;
                    const bool ok_x_min = !(x_min && pos.x < *x_min);
                    const bool ok_x_max = !(x_max && pos.x >= *x_max);
                    const bool all_ok = (ok_y_min + ok_y_max + ok_x_min + ok_x_max) == 4;
                    C2D_Image* img = &grass_img;
                    if(all_ok)
                    {
                        const int cell_idx = game_config->data.game_board->get_cell_content_index(pos);
                        if(cell_idx >= 0)
                            img = &minimap_imgs[cell_idx];
                    }
                    C2D_DrawImageAt(*img, 160 + (pos.x - self.x) * 20, 50 + 95 + (pos.y - self.y) * 20, 0.0625f);
                }
            }

            for(auto& player : game_config->data.players)
            {
                if(player.team_id != self.team_id) continue;

                C2D_ImageTint tint;
                C2D_PlainImageTint(&tint, u32(game_config->data.player_colors[player.color_index]), 1.0f);
                if(player.cursor)
                {
                    game::board::location pos;
                    pos.x = player.cursor->x;
                    pos.y = player.cursor->y;
                    const bool ok_y_min = !(y_min && pos.y < *y_min);
                    const bool ok_y_max = !(y_max && pos.y >= *y_max);
                    const bool ok_x_min = !(x_min && pos.x < *x_min);
                    const bool ok_x_max = !(x_max && pos.x >= *x_max);
                    const bool all_ok = (ok_y_min + ok_y_max + ok_x_min + ok_x_max) == 4;
                    if(all_ok)
                        C2D_DrawImageAt(cursor_img, 160 + (pos.x - self.x) * 20, 50 + 95 + (pos.y - self.y) * 20, 0.125f, &tint);
                }

                C2D_DrawImageAtRotated(player_indicator, 160 + (player.x - self.x) * 20, 50 + 95 + (player.y - self.y) * 20, 0.125f + 0.0625f / 2.0f, C3D_AngleFromDegrees(player.yaw), &tint);
            }
            C2D_Flush();
            C3D_SetScissor(GPU_SCISSOR_DISABLE, 0, 0, 240, 320);
        }
        else
        {
            C2D_DrawImageAt(no_minimap_img, 80, 50 + 19, 0.0625f);
        }
        // C2D_DrawImageAt(minimap_cover, 0, 50, 0.1875f);
        break;
    case 1: // teams info
        {
        game::button button;
        button.depth = 0.0625f;
        button.w = 76;
        button.h = 60;
        for(int i = 0; i < game::room::MAX_TEAMS; ++i)
        {
            if(game_config->data.teams[i].current_state == game::team::state::none)
                continue;

            const int x = 4 + (i % 4) * (button.w + 2);
            const int y = 60 + (i / 4) * (button.h + 10);
            button.x = x;
            button.y = y;
            button.draw(button_parts);
            C2D_DrawImageAt(game_config->data.team_icons[i], x + 1, y + 6 + 4, 0.125f);
            C2D_Image* img = nullptr;
            switch(game_config->data.teams[i].current_state)
            {
            case game::team::state::playing:
                img = &team_playing_img;
                break;
            case game::team::state::lost:
                img = &team_lost_img;
                break;
            case game::team::state::won:
                img = &team_won_img;
                break;
            default:
                break;
            }

            if(img)
                C2D_DrawImageAt(*img, x + 1 + 24 + 1, y + 6, 0.125f);
        }
        }
        break;
    case 2: // numbers (flags/mines, timer)
        break;
    }
}
