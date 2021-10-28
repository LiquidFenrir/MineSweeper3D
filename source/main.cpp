#include <exception>

#include "debugging.h"
#include "ctrhid.h"
#include "ctrgfx.h"
#include "ctrthread.h"
// #include "ctrsound.h"
#include "gameconfig.h"
#include "gamescene.h"

extern "C" {
u32 __stacksize__ = 128 * 1024;
}

int main(int argc, char** argv)
{
    if(debugging::should_enable_from_start())
    {
        debugging::enable();
    }
    else
    {
        debugging::disable();
    }

    char username[0x18] = {0};
    {
    cfguInit();
    u8 tmp[0x1c];
    CFGU_GetConfigInfoBlk2(sizeof(tmp), 0x000A0000, tmp);
    utf16_to_utf8((u8*)username, (u16*)tmp, 20);
    cfguExit();
    }

    udsInit(0x3000, nullptr);
    romfsInit();
    try {
        // trickery to ensure persistent data like graphics and audio is destroyed before the associated module
        std::optional<ctr::gfx> gfx_holder;
        std::optional<ctr::hid> hid_holder;
        // std::optional<ctr::audio> audio_holder;

        auto config = std::make_unique<game::config>(argv ? argv[0] : nullptr, username);
        game::scenes::game_config = config.get();

        auto& gfx = gfx_holder.emplace(config->screen_settings);
        auto& hid = hid_holder.emplace();
        // auto& audio = audio_holder.emplace();

        {
        auto scene = game::scenes::get_default_scene();
        scene->enter_scene();

        TickCounter tick_counter;
        osTickCounterStart(&tick_counter);
        while(aptMainLoop() && scene)
        {
            hid.update();
            osTickCounterUpdate(&tick_counter);

            auto next = scene->update(hid, osTickCounterRead(&tick_counter));

            if(next)
            {
                if(*next)
                {
                    scene->exit_scene();
                    scene.swap(*next);
                    scene->enter_scene();
                }
                else
                    break;
            }

            // Render the scene
            gfx.begin_frame();
            {
                gfx.clear_screens(scene->clear_color_top, scene->clear_color_bottom);
                scene->draw(gfx);
            }
            gfx.end_frame();
        }
        }
    }
    catch (const std::exception& e) { // reference to the base of a polymorphic object
        debugging::enable();
        const char* const msg = e.what();
        debugging::log("\nAn exception occured!\n");
        debugging::log(msg);
        debugging::log("\nThat was it.\n");
        debugging::show_error(msg);
    }
    romfsExit();
    udsExit();
}
