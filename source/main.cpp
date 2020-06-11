#include "verts.h"
#include "mine.h"
#include "spritesheet.h"

int main()
{
    // Initialize libs
    consoleDebugInit(debugDevice_SVC);

    srand(time(nullptr));

    romfsInit();
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);

    // Initialize the render target
    C3D_RenderTarget* top_screen_left = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    C3D_RenderTargetSetOutput(top_screen_left, GFX_TOP, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);
    C3D_RenderTarget* top_screen_right = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    C3D_RenderTargetSetOutput(top_screen_right, GFX_TOP, GFX_RIGHT, DISPLAY_TRANSFER_FLAGS);

    C3D_RenderTarget* bottom_screen = C3D_RenderTargetCreate(240, 320, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    C3D_RenderTargetSetOutput(bottom_screen, GFX_BOTTOM, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);

    C2D_SpriteSheet sheet = C2D_SpriteSheetLoad("romfs:/gfx/spritesheet.t3x");

    ProgramWide::init(C2D_SpriteSheetGetImage(sheet, 0).tex);

    MineSweeper mines(sheet);

    // Main loop
    while (aptMainLoop())
    {
        hidScanInput();

        // Respond to user input
        u32 kDown = hidKeysDown();
        u32 kHeld = hidKeysHeld();
        if (kDown & KEY_START)
            break; // break in order to return to hbmenu
        else if(kDown & KEY_SELECT)
            mines.in_controls = !mines.in_controls;

        touchPosition touch;
        hidTouchRead(&touch);
        mines.update(kDown, kHeld, touch);

        // Render the scene
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
            C3D_RenderTargetClear(top_screen_left, C3D_CLEAR_ALL, CLEAR_COLOR_TOP, 0);
            C3D_RenderTargetClear(top_screen_right, C3D_CLEAR_ALL, CLEAR_COLOR_TOP, 0);
            C3D_RenderTargetClear(bottom_screen, C3D_CLEAR_ALL, CLEAR_COLOR_BOT, 0);


            const float slider = osGet3DSliderState();
            const float iod = slider/3;

            C3D_FrameDrawOn(top_screen_left);
            if(mines.playing)
            {
                mines.renderTerrain(-iod);
                if(iod > 0.0f)
                {
                    C3D_FrameDrawOn(top_screen_right);
                    mines.renderTerrain(iod);
                }
            }
            else
            {
                C2D_SceneTarget(top_screen_left);

                C2D_Prepare();
                mines.renderLogo();
                C2D_Flush();
            }

            C3D_FrameDrawOn(bottom_screen);
            C2D_SceneTarget(bottom_screen);

            C2D_Prepare();
            mines.renderGui();
            C2D_Flush();

        C3D_FrameEnd(0);
    }

    LevelWide::exit();
    ProgramWide::exit();

    C2D_SpriteSheetFree(sheet);

    // Deinitialize libs
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    romfsExit();
    return 0;
}
