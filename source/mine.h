#pragma once

#include "common.h"

#include "verts.h"

#include <citro2d.h>
#include <tex3ds.h>

typedef struct {
    short x, y;
} Coord;

#define ROTATE_SPEED (rotate_speed_factor * ROTATE_SPEED_BASE)

struct MineSweeper {
    static constexpr short MIN_SZ = 10;
    static constexpr short MAX_SZ = 99;
    static constexpr int MIN_BOMBS_PERCENT = 10;
    static constexpr int MAX_BOMBS_PERCENT = 40;

    static constexpr float ROTATE_SPEED_BASE = 0.75f;
    static constexpr float ROTATE_SPEED_BASE_FACTOR = 1.0f;
    static constexpr float MOVEMENT_SPEED = 0.125f/2.75f;

    enum class Editing {
        Width,
        Height,
        Bombs,
        Ok,
    };

    enum class EditingControls {
        ABXY,
        DPAD,
        YAxis,
        Sensitivity,
    };
    std::vector<char> internal, visible;
    std::vector<signed char> around;
    std::vector<Vertex> vertices;

    static constexpr size_t cursor_idx = 0, cursor_vert_count = 6;
    size_t floor_idx;

    short looking_at_x, looking_at_y;
    int cursor_frame, cursor_frame_dir;
    int framectr;

    short width, height, bombpercent;
    int bombs;
    int flags_count;

    Editing selected_editing;
    float angleX, angleY;
    float positionX, positionZ;
    float rotate_speed_factor;

    bool playing;
    bool dead;
    bool win;
    bool looking_at_floor;
    bool should_update_cursor;
    bool stuff_changed;
    bool generated;
    bool in_controls;

    EditingControls editing_control_type;
    bool abxy_look;
    bool dpad_look;
    bool y_axis_inverted;

    C2D_Image hidden_image,
              open_image,
              red_image,
              flag_image,
              wall_image,
              empty_image,
              width_image,
              height_image,
              bomb_image,
              ok_image,
              up_image,
              down_image,
              outline_image,
              crosshair_image,
              logo_image,
              abxy_image,
              dpad_image,
              axis_image,
              sensi_image,
              look_image,
              move_image,
              reg_image,
              inv_image,
              win_image,
              dead_image;

    C2D_Image numbers_images[10];
    C2D_Image cursor_images[3];
    C2D_ImageTint back_tint,
                  front_tint,
                  selected_tint;

    float get_terrain_min_y()
    {
        return height/-2.0f;
    }
    float get_terrain_min_x()
    {
        return width/-2.0f;
    }

    MineSweeper(C2D_SpriteSheet sheet);
    u32 getKeysForFlag(bool flag, u32 abxy, u32 dpad)
    {
        u32 keys = 0;
        if(abxy_look == flag)
            keys |= abxy;
        if(dpad_look == flag)
            keys |= dpad;
        return keys;
    }

    u32 getLookUpKeys()
    {
        return getKeysForFlag(true, KEY_X | KEY_CSTICK_UP, KEY_UP);
    }
    u32 getLookDownKeys()
    {
        return getKeysForFlag(true, KEY_B | KEY_CSTICK_DOWN, KEY_DOWN);
    }
    u32 getLookLeftKeys()
    {
        return getKeysForFlag(true, KEY_Y | KEY_CSTICK_LEFT, KEY_LEFT);
    }
    u32 getLookRightKeys()
    {
        return getKeysForFlag(true, KEY_A | KEY_CSTICK_RIGHT, KEY_RIGHT);
    }

    u32 getMoveForwardKeys()
    {
        return getKeysForFlag(false, KEY_X | KEY_CSTICK_UP, KEY_UP);
    }
    u32 getMoveBackwardsKeys()
    {
        return getKeysForFlag(false, KEY_B | KEY_CSTICK_DOWN, KEY_DOWN);
    }
    u32 getMoveLeftKeys()
    {
        return getKeysForFlag(false, KEY_Y | KEY_CSTICK_LEFT, KEY_LEFT);
    }
    u32 getMoveRightKeys()
    {
        return getKeysForFlag(false, KEY_A | KEY_CSTICK_RIGHT, KEY_RIGHT);
    }

    void generateBombs();
    void checkAround(Coord point);
    void reveal();
    void placeFlag();

    void generateCrosshair();
    void generateCursor(int looking_at_idx);
    void generateFloorLayers(size_t& idx, size_t layer_size);
    void generateWalls(size_t& idx);
    void generateVertices();

    void renderTerrain(float iod)
    {
        ThreeD::bind();
        ThreeD::draw(positionX, positionZ, angleX, angleY, looking_at_floor, iod);
    }
    void renderGui();
    void renderLogo();

    void updateFloor();
    void updateCursorUVAndPos();
    void updateCursorLookingAt();

    void lookDir(float x, float y);
    void advance(float angle, float delta);

    void goForward(float v)
    {
        advance(angleX, v);
    }
    void goBackwards(float v)
    {
        advance(angleX, -v);
    }
    void goLeft(float v)
    {
        advance(angleX - 90, v);
    }
    void goRight(float v)
    {
        advance(angleX - 90, -v);
    }

    void lookLeft(float v)
    {
        lookDir(-v, 0.0f);
    }
    void lookRight(float v)
    {
        lookDir(v, 0.0f);
    }
    void lookUp(float v)
    {
        lookDir(0.0f, v);
    }
    void lookDown(float v)
    {
        lookDir(0.0f, -v);
    }
    
    void update(u32 kDown, u32 kHeld, touchPosition touch);
};
