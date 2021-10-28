#include "gameconfig.h"
#include "debugging.h"
#include <string>
#include <memory>
#include <cstdio>
#include "INIReader.h"

game::config::config(const char* exe_path, const char* user)
    : config_path(exe_path ? std::string(exe_path) + ".ini" : "")
    , username(user)
{
    if(exe_path == nullptr) return;

    util::file_ptr fh(fopen(config_path.c_str(), "r"));
    if(!fh)  {
        debugging::log("Config file {} does not exist!\n", config_path);
        return;
    }

    INIReader reader(fh.get());
    if (auto err = reader.ParseError(); err != 0) {
        debugging::log("Error during config load: {}\n", err);
        return;
    }

    if(auto version = reader.GetInteger("donottouch", "version", -1); version != config_version)
    {
        debugging::log("Invalid config version: {}v", version);
        return;
    }
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
