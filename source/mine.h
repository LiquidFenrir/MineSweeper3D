#pragma once

#include "common.h"

#include "verts.h"

#include <citro2d.h>
#include <tex3ds.h>

typedef struct {
    short x, y;
} Coord;

struct MineSweeper {
    static constexpr short MIN_SZ = 10;
    static constexpr short MAX_SZ = 99;
    static constexpr int MIN_BOMBS_PERCENT = 10;
    static constexpr int MAX_BOMBS_PERCENT = 40;
    
    static constexpr float ROTATE_SPEED = 0.75f;
    static constexpr float MOVEMENT_SPEED = 0.125f/3.0f;

    enum class Editing {
        Width,
        Height,
        Bombs,
        Ok,
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

    bool playing;
    bool dead;
    bool win;
    bool looking_at_floor;
    bool should_update_cursor;
    bool stuff_changed;
    bool generated;

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
              win_image,
              dead_image;

    C2D_Image numbers_images[10];
    C2D_Image cursor_images[3];
    C2D_ImageTint back_tint,
                  front_tint,
                  selected_tint,
                  crosshair_tint;

    float get_terrain_min_y()
    {
        return height/-2.0f;
    }
    float get_terrain_min_x()
    {
        return width/-2.0f;
    }
    
    MineSweeper(C2D_SpriteSheet sheet);

    void generateBombs();
    void checkAround(Coord point);
    void reveal();
    void placeFlag();

    void generateCursor(int looking_at_idx);
    void generateFloorLayers(size_t& idx, size_t layer_size);
    void generateWalls(size_t& idx);
    void generateVertices();

    void renderTerrain()
    {
        ThreeD::bind();
        ThreeD::draw(positionX, positionZ, angleX, angleY, looking_at_floor);
    }
    void renderGui();
    void renderLogo();
    void renderCrosshair();

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
