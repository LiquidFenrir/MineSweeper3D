#ifndef GAMECONFIG_INC
#define GAMECONFIG_INC

#include "ctrgfx.h"
#include "ctrhid.h"
#include "gameboard.h"
#include "gameplayer.h"
#include "gameroom.h"
#include <map>
#include <array>
#include <vector>
#include <memory>
#include <optional>
#include "useful_utilities.h"

namespace game {

struct linear_deleter {
    void operator()(void* ptr)
    {
        linearFree(ptr);
    }
};
using linear_ptr = util::ptr_for<void*, linear_deleter>;

struct config {
    config(const char* exe_path, const char* user);
    config& operator=(const config&) = delete;
    config& operator=(config&&) = delete;
    config(const config&) = delete;
    config(config&&) = delete;

    void save() const;

    static inline constexpr int VERSION = 1;

    template<typename T>
    struct key_map {
        using settings = T;
        using key_usage = T::usage;

        struct keys {
            key_usage stick_left{key_usage::none};
            key_usage stick_right{key_usage::none};

            key_usage dpad_up{key_usage::none};
            key_usage dpad_down{key_usage::none};
            key_usage dpad_left{key_usage::none};
            key_usage dpad_right{key_usage::none};

            key_usage button_a{key_usage::none};
            key_usage button_b{key_usage::none};
            key_usage button_x{key_usage::none};
            key_usage button_y{key_usage::none};

            key_usage shoulder_l{key_usage::none};
            key_usage shoulder_r{key_usage::none};
            key_usage trigger_l{key_usage::none};
            key_usage trigger_r{key_usage::none};
        };

        keys map;

        using k_t = key_usage keys::*;
        using v_t = u32;
        constexpr static inline std::pair<k_t, v_t> kv[] = {
            {&keys::dpad_up, KEY_DUP},
            {&keys::dpad_down, KEY_DDOWN},
            {&keys::dpad_left, KEY_DLEFT},
            {&keys::dpad_right, KEY_DRIGHT},
            
            {&keys::button_a, KEY_A},
            {&keys::button_b, KEY_B},
            {&keys::button_x, KEY_X},
            {&keys::button_y, KEY_Y},

            {&keys::shoulder_l, KEY_L},
            {&keys::shoulder_r, KEY_R},
            {&keys::trigger_l, KEY_ZL},
            {&keys::trigger_r, KEY_ZR},
        };

        ctr::hid::input::key get_key_for(const key_usage usage) const
        {
            ctr::hid::input::key out(0);
            if(usage == key_usage::none) return out;

            for(const auto& [k, v] : kv)
            {
                if(map.*k == usage)
                {
                    out = out | v;
                }
            }
            return out;
        }

        key_map& operator=(const key_map&) = delete;
        key_map& operator=(key_map&&) = delete;
        key_map(const key_map&) = delete;
        key_map(key_map&&) = delete;

        key_map()
            : map(T::default_map())
        {
            
        }
    };

    struct menu_settings {
        enum class usage {
            none,
            cursor_up,
            cursor_down,
            cursor_left,
            cursor_right,
            tab_prev,
            tab_next,
            validate,
            cancel,
        };

        static key_map<menu_settings>::key_map::keys default_map();
    };
    struct game_settings {
        enum class usage {
            none,
            movement,
            camera,
            flag,
            reveal,
        };

        static key_map<game_settings>::key_map::keys default_map();
    };

    struct axis {
        bool reverse;
        unsigned char sensitivity; // 1-100
    };
    enum class distance_draw_mode {
        fog,
        drop,
    };

    struct config_data {
        ctr::gfx::screen_mode screen_settings{ctr::gfx::screen_mode::regular};
        distance_draw_mode draw_mode{distance_draw_mode::fog};
        axis camera_y_axis, camera_x_axis;
        key_map<menu_settings> keymap_menu;
        key_map<game_settings> keymap_game;
        bool enable_music{true};
        bool enable_sfx{true};
        bool skip_intro{false};
    };
    config_data conf;

    // start and select are for opening/closing the menu
    const std::string path;
    const char* const username;

    struct common_scene_data {
        std::optional<ctr::gfx::spritesheet> intro_sheet, ingame_sheet, menu_sheet;
        C2D_Image top_bg_full, bottom_bg_full;

        // std::optional<ctr::gfx::font> text_font_big;
        std::optional<ctr::gfx::font> text_font_small;
        template<unsigned N>
        struct double_text_buf {
            double_text_buf(const std::size_t sz, ctr::gfx::font* fnt = nullptr)
                : index{0}
            {
                for(auto& c : contained)
                    c.emplace(sz, fnt);
            }

            ctr::gfx::text_generator& get_active_buf()
            {
                ctr::gfx::text_generator& out = contained[index]->gen;
                index = (index + 1) % N;
                return out;
            }

        private:
            unsigned index;
            struct cont {
                ctr::gfx::text_buf buf;
                ctr::gfx::text_generator gen;

                cont(const std::size_t sz, ctr::gfx::font* fnt)
                    : buf(sz)
                    , gen(buf, fnt)
                {

                }
            };
            std::optional<cont> contained[N];
        };
        // makes transition scenes work, otherwise clearing and reuse happens
        // std::optional<double_text_buf> static_text_gen, dynamic_text_gen;
        std::optional<double_text_buf<2>> static_small_text_gen;
        std::optional<double_text_buf<1>> dynamic_small_text_gen;

        std::optional<ctr::gfx::texture> sheet_3d, sheet_cursors;
        linear_ptr board_vbo, board_margin_vbo, board_cursors_vbo;
        std::optional<ctr::gfx::shader_with_uniforms<3>> board_shader_basic;
        std::optional<ctr::gfx::shader_with_uniforms<4>> board_shader_drop;
        C3D_FogLut fog_lut;
        C3D_AttrInfo board_attr;
        C3D_BufInfo board_vbo_buf;
        C3D_BufInfo board_margin_vbo_buf;
        C3D_BufInfo board_cursors_vbo_buf;

        std::optional<game::room> joined_room;
        std::optional<game::board> game_board;
        std::array<game::player, game::room::MAX_PLAYERS> players;
    };

    common_scene_data data;
};

}

#endif
