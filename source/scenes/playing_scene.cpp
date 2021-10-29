#include "playing_scene.h"
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
    0.5f,
    2.0f,
    0.01f,
    10.0f,
};
}

scenes::playing_scene::playing_scene(game::board::numbers n, game::board::mode b)
    : nums{n}
    , board_mode{b}
{
    clear_color_top = ctr::gfx::color{0x00, 0x94, 0xff, 0xff};
    point_count = game_config->data.game_board->reset(nums, board_mode, 0) * 6;

    GSPGPU_FlushDataCache(game_config->data.board_vbo.get(), point_count * sizeof(game::board::buffer_point));

    auto& player_me = game_config->data.players[0];
    player_me.on_map = true;
    player_me.x = nums.dims.width / 2.0f;
    player_me.y = nums.dims.height / 2.0f;

    Mtx_PerspTilt(&projection, C3D_AngleFromDegrees(50.0f), C3D_AspectRatioTop, 0.01f, 20.0f, false);

    debugging::log("starting game\n");
    debugging::log("point_count: {}\n", point_count);
    FogLut_Exp(&game_config->data.fog_lut, foggy::f.den, foggy::f.gra, foggy::f.near, foggy::f.far);
}

game::scenes::next_scene scenes::playing_scene::update(const ctr::hid& input, const double dt)
{
    auto& player_me = game_config->data.players[0];
    auto held = input.held();
    constexpr float MV_SPEED = 0.002f;
    constexpr float ROT_SPEED = 0.0005f;

    bool any_change = false;
    const auto advance = [&](const float rad) {
        const float c = std::cos(rad);
        const float s = std::sin(rad);

        player_me.x += c * MV_SPEED * dt;
        player_me.y += s * MV_SPEED * dt;

        if(int(board_mode) & 2)
        {
            if(player_me.x <= -2)
            {
                player_me.x += nums.dims.width;
            }
            else if(player_me.x >= (nums.dims.width + 2))
            {
                player_me.x -= nums.dims.width;
            }
        }
        else
        {
            if(player_me.x < 0.125f)
            {
                player_me.x = 0.125f;
            }
            else if(player_me.x > (nums.dims.width - 0.125f))
            {
                player_me.x = nums.dims.width - 0.125f;
            }
        }

        if(int(board_mode) & 1)
        {
            if(player_me.y <= -2)
            {
                player_me.y += nums.dims.height;
            }
            else if(player_me.y >= (nums.dims.height + 2))
            {
                player_me.y -= nums.dims.height;
            }
        }
        else
        {
            if(player_me.y < 0.125f)
            {
                player_me.y = 0.125f;
            }
            else if(player_me.y > (nums.dims.height - 0.125f))
            {
                player_me.y = nums.dims.height - 0.125f;
            }
        }

        any_change = true;
    };
    const auto lookDir = [&](const int dx, const int dy) {
        player_me.yaw += dx * ROT_SPEED * dt;
        player_me.pitch += dy * ROT_SPEED * dt;

        if(player_me.pitch < -90.0f)
        {
            player_me.pitch = -90.0f;
        }
        else if(player_me.pitch > 90.0f)
        {
            player_me.pitch = 90.0f;
        }

        if(player_me.yaw < -360.0f)
        {
            player_me.yaw += 360.0f;
        }
        else if(player_me.yaw > 360.0f)
        {
            player_me.yaw -= 360.0f;
        }

        any_change = true;
    };

    auto cpad = input.left_stick();
    if(cpad.distance() > 800)
    {
        advance(cpad.angle() + C3D_AngleFromDegrees(player_me.yaw - 90.0f));
    }

    auto cstick = input.right_stick();
    if(cstick.distance() > 800)
    {
        lookDir(cstick.x(), -cstick.y());
    }

    if(held.any(KEY_DUP | KEY_DDOWN))
    {
        const int dir = held.check(KEY_DDOWN) ? 1 : -1;
        lookDir(0, dir * 160);
    }

    if(held.any(KEY_DLEFT | KEY_DRIGHT))
    {
        const int dir = held.check(KEY_DRIGHT) ? 1 : -1;
        lookDir(dir * 160, 0);
    }

    if(held.check(KEY_START)) return nullptr;

    if(any_change)
    {
        game_config->data.game_board->fill_cursor_positions(game_config->data.players, static_cast<game::board::buffer_point*>(game_config->data.board_cursors_vbo.get()));
        GSPGPU_FlushDataCache(game_config->data.board_cursors_vbo.get(), ((game::room::MAX_PLAYERS) * 6 * sizeof(game::board::buffer_point)));
    }

    return std::nullopt;
}

void scenes::playing_scene::draw(ctr::gfx& gfx)
{
    bool drop = false;
    bool fog = false;

    switch(game_config->draw_mode)
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

    C3D_SetAttrInfo(&game_config->data.board_attr);

    if(fog)
    {
        C3D_FogGasMode(GPU_FOG, GPU_DEPTH_DENSITY, false);
        C3D_FogColor(0xD8B068);
        C3D_FogLutBind(&game_config->data.fog_lut);
    }
    else
    {
        C3D_FogGasMode(GPU_NO_FOG, GPU_PLAIN_DENSITY, false);
        C3D_FogLutBind(nullptr);
    }

    game_config->data.sheet_3d->bind(0);

    C3D_TexEnv* env = C3D_GetTexEnv(0);
    C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR);
    C3D_TexEnvFunc(env, C3D_RGB, GPU_MODULATE);

    const auto& player_me = game_config->data.players[0];
    C3D_Mtx modelView;

    if(fog)
    {
        Mtx_Identity(&modelView);
        // Mtx_Translate(&modelView, 0.0f, 0.0f, 0.0f, true);
        Mtx_RotateX(&modelView, C3D_AngleFromDegrees(player_me.pitch), true);
        Mtx_RotateY(&modelView, C3D_AngleFromDegrees(player_me.yaw), true);
        Mtx_Translate(&modelView, -player_me.x, 0.0f, -player_me.y, true);

        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, game_config->data.board_shader_basic->uniforms[0], &projection);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, game_config->data.board_shader_basic->uniforms[1], &modelView);

        C3D_FVUnifSet(GPU_VERTEX_SHADER, game_config->data.board_shader_basic->uniforms[2], 1.0f, 1.0f, 1.0f, 1.0f);
    }
    else if(drop)
    {
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, game_config->data.board_shader_drop->uniforms[0], &projection);

        Mtx_Identity(&modelView);
        Mtx_RotateX(&modelView, C3D_AngleFromDegrees(player_me.pitch), true);
        Mtx_RotateY(&modelView, C3D_AngleFromDegrees(player_me.yaw), true);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, game_config->data.board_shader_drop->uniforms[1], &modelView);

        Mtx_Identity(&modelView);
        Mtx_Translate(&modelView, -player_me.x, 0.0f, -player_me.y, false);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, game_config->data.board_shader_drop->uniforms[2], &modelView);

        C3D_FVUnifSet(GPU_VERTEX_SHADER, game_config->data.board_shader_drop->uniforms[3], 1.0f, 1.0f, 1.0f, 1.0f);
    }

    C3D_SetBufInfo(&game_config->data.board_vbo_buf);
    C3D_DrawArrays(GPU_TRIANGLES, 0, point_count);

    game::board::numbers::size offs[4];
    int num_offs = 0;

    if(board_mode == game::board::mode::loop_vertical)
    {
        if(player_me.y <= 8)
            offs[num_offs++] = {0, -nums.dims.height};
        if(player_me.y >= (nums.dims.height - 8))
            offs[num_offs++] = {0, +nums.dims.height};
    }
    else if(board_mode == game::board::mode::loop_horizontal)
    {
        if(player_me.x <= 8)
            offs[num_offs++] = {-nums.dims.width, 0};
        if(player_me.x >= (nums.dims.width - 8))
            offs[num_offs++] = {+nums.dims.width, 0};
    }
    else if(board_mode == game::board::mode::loop_both)
    {
        if(player_me.y <= 8 && player_me.x <= 8)
        {
            offs[num_offs++] = {-nums.dims.width, -nums.dims.height};
        }
        if(player_me.y <= 8 && player_me.x >= (nums.dims.width - 8))
        {
            offs[num_offs++] = {nums.dims.width, -nums.dims.height};
        }
        if(player_me.y >= (nums.dims.height - 8) && player_me.x <= 8)
        {
            offs[num_offs++] = {-nums.dims.width, nums.dims.height};
        }
        if(player_me.y >= (nums.dims.height - 8) && player_me.x >= (nums.dims.width - 8))
        {
            offs[num_offs++] = {nums.dims.width, nums.dims.height};
        }
        if(player_me.x <= 8)
        {
            offs[num_offs++] = {-nums.dims.width, 0};
        }
        if(player_me.x >= (nums.dims.width - 8))
        {
            offs[num_offs++] = {+nums.dims.width, 0};
        }
        if(player_me.y <= 8)
        {
            offs[num_offs++] = {0, -nums.dims.height};
        }
        if(player_me.y >= (nums.dims.height - 8))
        {
            offs[num_offs++] = {0, +nums.dims.height};
        }
    }

    C3D_Mtx modelcopy;
    Mtx_Copy(&modelcopy, &modelView);
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

    if(fog)
    {
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, game_config->data.board_shader_basic->uniforms[1], &modelcopy);
        C3D_FVUnifSet(GPU_VERTEX_SHADER, game_config->data.board_shader_basic->uniforms[2], 1.0f, 0.5f, 0.5f, 1.0f);
    }
    else if(drop)
    {
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, game_config->data.board_shader_drop->uniforms[2], &modelcopy);
        C3D_FVUnifSet(GPU_VERTEX_SHADER, game_config->data.board_shader_drop->uniforms[3], 1.0f, 0.5f, 0.5f, 1.0f);
    }

    C3D_SetBufInfo(&game_config->data.board_cursors_vbo_buf);
    C3D_DrawArrays(GPU_TRIANGLES, 0, (game::room::MAX_PLAYERS) * 6);

    if(fog)
    {
        C3D_FogGasMode(GPU_NO_FOG, GPU_PLAIN_DENSITY, false);
        C3D_FogLutBind(nullptr);
    }

    gfx.get_screen(GFX_BOTTOM, GFX_LEFT)->focus();
    gfx.render_2d();
    C2D_DrawRectSolid(20,20,0,20,20, C2D_Color32f(1,0,1,1));
}
