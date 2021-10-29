#include "gameconfig.h"
#include "debugging.h"
#include <string>
#include <string_view>
#include <memory>
#include <cstdio>
#include <libconfig.h++>

using namespace std::string_view_literals;

game::config::config(const char* exe_path, const char* user)
    : path(exe_path ? std::string(exe_path) + ".conf" : "")
    , username(user)
{
    if(exe_path == nullptr) return;

    util::file_ptr file(fopen(path.c_str(), "r"));
    if(!file)
    {
        debugging::log("Config file absent at {}\n", path);
        return;
    }

    libconfig::Config io;
    io.read(file.get());

    if(int version = 0; io.lookupValue("version", version) && version == VERSION)
    {
        if(const char* screen_mode = nullptr; io.lookupValue("screen.mode", screen_mode))
        {
            constexpr std::pair<std::string_view, ctr::gfx::screen_mode> ps[] = {
                {"normal", ctr::gfx::screen_mode::regular},
                {"stereo", ctr::gfx::screen_mode::stereo},
                {"wide", ctr::gfx::screen_mode::wide},
            };
            for(const auto& [s,m] : ps)
            {
                if(s == screen_mode)
                {
                    conf.screen_settings = m;
                    break;
                }
            }
        }

        if(const char* gameplay_mode = nullptr; io.lookupValue("screen.gameplay_mode", gameplay_mode))
        {
            constexpr std::pair<std::string_view, distance_draw_mode> ps[] = {
                {"fog", distance_draw_mode::fog},
                {"drop", distance_draw_mode::drop},
            };
            for(const auto& [s,m] : ps)
            {
                if(s == gameplay_mode)
                {
                    conf.draw_mode = m;
                    break;
                }
            }
        }
        
        if(bool x_axis_rev = false; io.lookupValue("controls.x_axis.reverse", x_axis_rev))
        {
            conf.camera_x_axis.reverse = x_axis_rev;
        }
        if(int x_axis_sens = false; io.lookupValue("controls.x_axis.sensitivity", x_axis_sens))
        {
            conf.camera_x_axis.sensitivity = x_axis_sens;
        }
        
        if(bool y_axis_rev = false; io.lookupValue("controls.y_axis.reverse", y_axis_rev))
        {
            conf.camera_y_axis.reverse = y_axis_rev;
        }
        if(int y_axis_sens = false; io.lookupValue("controls.y_axis.sensitivity", y_axis_sens))
        {
            conf.camera_y_axis.sensitivity = y_axis_sens;
        }
    }
    else
    {
        debugging::log("Outdated config file: {} (current {})\n", version, VERSION);
    }
}

void game::config::save() const
{
    libconfig::Config io;
    auto& root = io.getRoot();

    io.writeFile(path.c_str());
}

#define DEFAULT_KEY_MAP_CREATE(cl, fun) \
game::config::key_map<game::config::cl>::key_map::keys game::config::cl::default_map() { \
    game::config::key_map<game::config::cl>::key_map::keys out; \
    ([&] fun)(); \
    return out; }

DEFAULT_KEY_MAP_CREATE(menu_settings, {
    out.dpad_up = usage::cursor_up;
    out.dpad_down = usage::cursor_down;
    out.dpad_left = usage::cursor_left;
    out.dpad_right = usage::cursor_right;
    out.shoulder_l = usage::tab_prev;
    out.shoulder_r = usage::tab_next;
    out.button_a = usage::validate;
    out.button_b = usage::cancel;
})

DEFAULT_KEY_MAP_CREATE(game_settings, {
    out.stick_left = usage::camera;
    out.stick_right = usage::camera;
    out.dpad_up = usage::movement;
    out.dpad_down = usage::movement;
    out.dpad_left = usage::movement;
    out.dpad_right = usage::movement;
    out.button_a = usage::reveal;
    out.button_b = usage::reveal;
    out.button_x = usage::flag;
    out.button_y = usage::flag;
})
