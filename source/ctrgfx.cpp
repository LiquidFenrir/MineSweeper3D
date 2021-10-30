#include "ctrgfx.h"
#include "debugging.h"
#include <exception>

#define DISPLAY_TRANSFER_FLAGS \
    (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | \
    GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | \
    GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

namespace ctr {

gfx::gfx(const screen_mode mode)
    : current_mode{mode}
    , current_rendering_mode(rendering_mode::none)
    , top_left_target{nullptr}
    , top_right_target{nullptr}
    , bottom_target{nullptr}
{
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE * 4);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS * 4);

    switch(mode)
    {
    case screen_mode::regular:
        top_left_target = C3D_RenderTargetCreate(GSP_SCREEN_WIDTH, GSP_SCREEN_HEIGHT_TOP, GPU_RB_RGBA8, GPU_RB_DEPTH16);
        break;
    case screen_mode::stereo:
        // gfxSet3D(true); // Enable stereoscopic 3D
        top_left_target = C3D_RenderTargetCreate(GSP_SCREEN_WIDTH, GSP_SCREEN_HEIGHT_TOP, GPU_RB_RGBA8, GPU_RB_DEPTH16);
        top_right_target = C3D_RenderTargetCreate(GSP_SCREEN_WIDTH, GSP_SCREEN_HEIGHT_TOP, GPU_RB_RGBA8, GPU_RB_DEPTH16);
        break;
    case screen_mode::wide:
        // gfxSetWide(true); // Enable wide mode
        top_left_target = C3D_RenderTargetCreate(GSP_SCREEN_WIDTH, GSP_SCREEN_HEIGHT_TOP_2X, GPU_RB_RGBA8, GPU_RB_DEPTH16);
        break;
    default:
        throw std::runtime_error("Invalid screen mode in config file.");
    }

    bottom_target = C3D_RenderTargetCreate(GSP_SCREEN_WIDTH, GSP_SCREEN_HEIGHT_BOTTOM, GPU_RB_RGBA8, GPU_RB_DEPTH16);
    C3D_RenderTargetSetOutput(bottom_target, GFX_BOTTOM, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);

    if(top_left_target)
        C3D_RenderTargetSetOutput(top_left_target,  GFX_TOP, GFX_LEFT,  DISPLAY_TRANSFER_FLAGS);
    if(top_right_target)
        C3D_RenderTargetSetOutput(top_right_target,  GFX_TOP, GFX_RIGHT,  DISPLAY_TRANSFER_FLAGS);
}
gfx::~gfx()
{
    C2D_Fini();
    C3D_Fini();
    gfxExit();
}

void gfx::begin_frame()
{
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
}
void gfx::clear_screens(const color top, const color bottom)
{
    if(auto s = get_screen(GFX_TOP, GFX_LEFT); s)
    {
        s->clear(top, 0);
    }
    
    if(auto s = get_screen(GFX_TOP, GFX_RIGHT); s)
    {
        s->clear(top, 0);
    }

    if(auto s = get_screen(GFX_BOTTOM); s)
    {
        s->clear(bottom, 0);
    }
}
void gfx::end_frame()
{
    C3D_FrameEnd(0);
}
std::optional<gfx::target> gfx::get_screen(const gfxScreen_t screen, const gfx3dSide_t side)
{
    switch(screen)
    {
    case GFX_TOP:
        switch(side)
        {
        case GFX_LEFT:
            return target(top_left_target);
        case GFX_RIGHT:
            return top_right_target ? std::make_optional<target>(top_right_target) : std::nullopt;
        }
        break;
    case GFX_BOTTOM:
        return target(bottom_target);
    }
    return std::nullopt;
}
gfx::dimensions gfx::get_screen_dimensions(const gfxScreen_t screen) const
{
    switch(screen)
    {
    case GFX_TOP:
        if(current_mode == screen_mode::wide)
        {
            return {GSP_SCREEN_HEIGHT_TOP_2X, GSP_SCREEN_WIDTH};
        }
        else
        {
            return {GSP_SCREEN_HEIGHT_TOP, GSP_SCREEN_WIDTH};
        }
    case GFX_BOTTOM:
        return {GSP_SCREEN_HEIGHT_BOTTOM, GSP_SCREEN_WIDTH};
    }
    return {-1, -1};
}
float gfx::stereo() const
{
    return top_right_target ? osGet3DSliderState() / 3 : 0.0f;
}

}
