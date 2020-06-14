#pragma once

#include "common.h"

#include <citro3d.h>

#define CLEAR_COLOR_TOP 0x68B0D8FF
#define CLEAR_COLOR_BOT 0xFFC8AAFF

#define DISPLAY_TRANSFER_FLAGS \
    (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | \
    GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | \
    GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

struct Vertex {
    float position[3];
    float texcoord[2];
    float norm[3];

    Vertex() : position{0,0,0}, texcoord{0,0}, norm{0,0,1} { }
    Vertex(C3D_FVec pos, float u, float v, C3D_FVec n)
    :
    position{pos.x, pos.y, pos.z},
    texcoord{u, v},
    norm{n.x, n.y, n.z}
    {

    }

    float& x()
    {
        return position[0];
    }
    float& y()
    {
        return position[1];
    }
    float& z()
    {
        return position[2];
    }
};

namespace ProgramWide {
    void init(C3D_Tex* tex);
    void exit();
};

namespace LevelWide {
    void init(size_t count, int w, int h);
    Vertex* get_floor_verts();
    Vertex* get_wall_verts();
    Vertex* get_cursor_verts();
    Vertex* get_crosshair_verts();
    void exit();
};

namespace ThreeD {
    void bind();
    void draw(float posX, float posZ, float angleX, float angleY, bool looking_at_floor, float iod);
};
