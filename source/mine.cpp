#include "mine.h"

#include "spritesheet.h"

MineSweeper::MineSweeper(C2D_SpriteSheet sheet)
:
width(MIN_SZ), height(MIN_SZ), bombpercent(MIN_BOMBS_PERCENT),
selected_editing(Editing::Width),
angleX(0.0f), angleY(0.0f), positionX(0.0f), positionZ(0.0f),
playing(false), dead(false), win(false), looking_at_floor(false), stuff_changed(false)
{
    hidden_image = C2D_SpriteSheetGetImage(sheet, spritesheet_hidden_idx);
    open_image = C2D_SpriteSheetGetImage(sheet, spritesheet_open_idx);
    red_image = C2D_SpriteSheetGetImage(sheet, spritesheet_red_idx);
    flag_image = C2D_SpriteSheetGetImage(sheet, spritesheet_flag_idx);
    wall_image = C2D_SpriteSheetGetImage(sheet, spritesheet_wall_idx);
    empty_image = C2D_SpriteSheetGetImage(sheet, spritesheet_empty_idx);
    width_image = C2D_SpriteSheetGetImage(sheet, spritesheet_width_idx);
    height_image = C2D_SpriteSheetGetImage(sheet, spritesheet_height_idx);
    bomb_image = C2D_SpriteSheetGetImage(sheet, spritesheet_bomb_idx);
    ok_image = C2D_SpriteSheetGetImage(sheet, spritesheet_ok_idx);
    up_image = C2D_SpriteSheetGetImage(sheet, spritesheet_up_idx);
    down_image = C2D_SpriteSheetGetImage(sheet, spritesheet_down_idx);
    outline_image = C2D_SpriteSheetGetImage(sheet, spritesheet_select_idx);
    logo_image = C2D_SpriteSheetGetImage(sheet, spritesheet_logo_idx);
    crosshair_image = C2D_SpriteSheetGetImage(sheet, spritesheet_crosshair_idx);
    win_image = C2D_SpriteSheetGetImage(sheet, spritesheet_win_idx);
    dead_image = C2D_SpriteSheetGetImage(sheet, spritesheet_dead_idx);

    for(int i = 0; i < 10; i++)
    {
        numbers_images[i] = C2D_SpriteSheetGetImage(sheet, spritesheet_0_idx + i);
    }
    for(int i = 0; i <= 2; i++)
    {
        cursor_images[i] = C2D_SpriteSheetGetImage(sheet, spritesheet_cursor_0_idx + i);
    }

    C2D_PlainImageTint(&back_tint, C2D_Color32f(0.125f, 0.125f, 0.125f, 1), 1.0f);
    C2D_PlainImageTint(&front_tint, C2D_Color32f(0.875f, 0.875f, 0.875f, 1), 1.0f);
    C2D_PlainImageTint(&selected_tint, C2D_Color32(255, 200, 76, 255), 1.0f);
    C2D_PlainImageTint(&crosshair_tint, C2D_Color32(102, 255, 102, 160), 1.0f);
}

namespace {
    #define XY_TO_IDX(x, y, m) ((x) + ((y) * (m)->width))
    #define IDX_TO_X(idx, m) ((idx) % (m)->width)
    #define IDX_TO_Y(idx, m) ((idx) / (m)->width)

    #define PT_TO_IDX(pt, m) ((pt).x + ((pt).y * (m)->width))

    int isTopPoint(Coord point, MineSweeper& mine)
    {
        return point.y == 0;
    }
    int isBotPoint(Coord point, MineSweeper& mine)
    {
        return point.y == mine.height - 1;
    }
    int isLeftPoint(Coord point, MineSweeper& mine)
    {
        return point.x == 0;
    }
    int isRightPoint(Coord point, MineSweeper& mine)
    {
        return point.x == mine.width - 1;
    }
    Coord modifyPoint(Coord point, short dx, short dy)
    {
        return (Coord){point.x + dx, point.y + dy};
    }
}

void MineSweeper::generateBombs()
{
    const int size = width * height;
    internal.clear();
    internal.resize(size, '0');
    visible.clear();
    visible.resize(size, '.');
    around.clear();
    around.resize(size, 0);

    DEBUGPRINT("resized vectors\n");

    for(int i = 0; i < bombs; i++)
    {
        int pos = rand() % size;
        while(internal[pos] == '.' || ( \
        (IDX_TO_X(pos, this) >= (looking_at_x - 1) && IDX_TO_X(pos, this) <= (looking_at_x + 1)) && \
        (IDX_TO_Y(pos, this) >= (looking_at_y - 1) && IDX_TO_Y(pos, this) <= (looking_at_y + 1))))
            pos = rand() % size;

        internal[pos] = '.';
        const int x_beg = IDX_TO_X(pos, this) - 1;
        const int x_end = x_beg + 2;
        const int y_beg = IDX_TO_Y(pos, this) - 1;
        const int y_end = y_beg + 2;
        for(int x = x_beg; x <= x_end; x++)
        {
            if(x < 0 || x >= width)
                continue;

            for(int y = y_beg; y <= y_end; y++)
            {
                if(y < 0 || y >= height)
                    continue;
                
                const int idx = XY_TO_IDX(x, y, this);
                if(internal[idx] != '.')
                    internal[idx]++;
            }
        }
    }

    DEBUGPRINT("generated bomb\n");

    for(auto& s : internal)
    {
        if(s == '0')
            s = ' ';
    }
    
    DEBUGPRINT("generation complete\n");

}
void MineSweeper::checkAround(Coord point)
{
    const int pos = PT_TO_IDX(point, this);
    if(around[pos] != 0)
        return;

    const char square = internal[pos];
    if(square == ' ')
    {
        if(visible[pos] == 'f') flags_count--;
        visible[pos] = square;
        around[pos] = 1;

        if(!isTopPoint(point, *this))
            checkAround(modifyPoint(point, 0, -1));
        if(!isBotPoint(point, *this))
            checkAround(modifyPoint(point, 0, +1));
        if(!isLeftPoint(point, *this))
            checkAround(modifyPoint(point, -1, 0));
        if(!isRightPoint(point, *this))
            checkAround(modifyPoint(point, +1, 0));
            
        if(!isTopPoint(point, *this) && !isLeftPoint(point, *this))
            checkAround(modifyPoint(point, -1, -1));
        if(!isBotPoint(point, *this) && !isRightPoint(point, *this))
            checkAround(modifyPoint(point, +1, +1));
        if(!isTopPoint(point, *this) && !isRightPoint(point, *this))
            checkAround(modifyPoint(point, +1, -1));
        if(!isBotPoint(point, *this) && !isLeftPoint(point, *this))
            checkAround(modifyPoint(point, -1, +1));
    }
    else if(square != '.')
    {
        visible[pos] = square;
        around[pos] = 1;
    }
    else
        around[pos] = 1;
}

void MineSweeper::reveal()
{
    const int pos = XY_TO_IDX(looking_at_x, looking_at_y, this);
    const char square = internal[pos];
    visible[pos] = square;

    if(square == ' ')
    {
        checkAround({looking_at_x, looking_at_y});
    }

    if(square == '.')
    {
        DEBUGPRINT("hit bomb\n");
        for(short y = 0; y < height; y++)
        {
            for(short x = 0; x < width; x++)
            {
                const int idx = XY_TO_IDX(x, y, this);
                if(internal[idx] == '.')
                    visible[idx] = '#';
            }
        }
        dead = true;
        DEBUGPRINT("done\n");
    }
    else
    {
        for(int y = 0; y < height; y++)
        {
            const int posY = y * width;
            for(int x = 0; x < width; x++)
            {
                const int posT = x + posY;
                if(internal[posT] == '.' && (visible[posT] == '.' || visible[posT] == 'f')) // bomb and it's (not found) or (flagged)
                {
                    continue;
                }
                else if(internal[posT] != visible[posT]) // not revealed yet
                {
                    return;
                }
            }
        }
        win = true;
    }
}

void MineSweeper::placeFlag()
{
    const int pos = XY_TO_IDX(looking_at_x, looking_at_y, this);
    if(visible[pos] == '.')
    {
        visible[pos] = 'f';
        flags_count++;
    }
    else if(visible[pos] == 'f')
    {
        visible[pos] = '.';
        flags_count--;
    }
}

using SubtexUVFPtr = void(*)(const Tex3DS_SubTexture*, float*, float*);
static constexpr SubtexUVFPtr subtex_uv_funcs[6] = {
    &Tex3DS_SubTextureBottomRight,
    &Tex3DS_SubTextureBottomLeft,
    &Tex3DS_SubTextureTopRight,
    &Tex3DS_SubTextureBottomLeft,
    &Tex3DS_SubTextureTopLeft,
    &Tex3DS_SubTextureTopRight,
};

void MineSweeper::lookDir(float x, float y)
{
    angleX += x;
    angleY += y;
    if(angleY < -90.0f)
    {
        angleY = -90.0f;
    }
    else if(angleY > 90.0f)
    {
        angleY = 90.0f;
    }
    
    looking_at_floor = false;
    should_update_cursor = false;
    if(angleY < 0.0f)
    {
        should_update_cursor = true;
    }
}
void MineSweeper::advance(float angle, float delta)
{
    const float c = cosf(C3D_AngleFromDegrees(angle));
    const float s = sinf(C3D_AngleFromDegrees(angle));
    C3D_FVec dir = FVec3_Scale(FVec3_New(c, 0.0f, s), delta);
    C3D_FVec pos = FVec3_Add(FVec3_New(positionX, 0.0f, positionZ), dir);

    const float mX = get_terrain_min_x();
    const float mY = get_terrain_min_y();
    constexpr float proximity = 0.25f;
    if(pos.x <= (mX + proximity) || pos.x > (-mX - proximity) || pos.z <= (mY + proximity) || pos.z > (-mY - proximity)) return;
    positionX = pos.x;
    positionZ = pos.z;

    looking_at_floor = false;
    should_update_cursor = false;
    if(angleY < 0.0f)
    {
        should_update_cursor = true;
    }
}

void MineSweeper::renderGui()
{
    if(playing)
    {
        if(win)
        {
            constexpr float x = (320.0f - 64.0f) / 2.0f;
            constexpr float y = (240.0f - 64.0f) / 2.0f;
            C2D_DrawImageAt(win_image, x + 1, y + 1, 0.0f, &back_tint);
            C2D_DrawImageAt(win_image, x - 1, y - 1, 0.25f, &front_tint);
            return;
        }
        else if(dead)
        {
            constexpr float x = (320.0f - 64.0f) / 2.0f;
            constexpr float y = (240.0f - 64.0f) / 2.0f;
            C2D_DrawImageAt(dead_image, x + 1, y + 1, 0.0f, &back_tint);
            C2D_DrawImageAt(dead_image, x - 1, y - 1, 0.25f, &front_tint);
            return;
        }
        const C2D_Image* icons[2] = {
            &bomb_image,
            &flag_image,
        };
        int* parts[2] = {
            &bombs,
            &flags_count,
        };

        int y = 0;

        constexpr int icon_w = 64, icon_h = 64;
        constexpr int icon_padding_y = 8;
        constexpr int icon_base_x = (320 - 64 - 96)/2, icon_base_y = (240 - (64 * 2) - (icon_padding_y * 1))/2;

        y = icon_base_y;
        for(int i = 0; i < 2; i++)
        {
            C2D_DrawImageAt(*(icons[i]), icon_base_x + (icon_w - icons[i]->subtex->width) / 2, y + (icon_h - icons[i]->subtex->height) / 2, 0.0f);
            
            C2D_DrawImageAt(outline_image, icon_base_x + 2 + icon_w + 4, y + 2, 0.0f, &back_tint);

            C2D_DrawImageAt(outline_image, icon_base_x + icon_w + 4, y, 0.25f, &front_tint);

            div_t hundreds = div(*(parts[i]), 100);
            div_t smaller = div(hundreds.rem, 10);
            C2D_DrawImageAt(numbers_images[hundreds.quot], icon_base_x + icon_w + 4 + 10, y + (64 - 32)/2, 0.5f);
            C2D_DrawImageAt(numbers_images[smaller.quot], icon_base_x + icon_w + 4 + (96 - 32)/2, y + (64 - 32)/2, 0.5f);
            C2D_DrawImageAt(numbers_images[smaller.rem], icon_base_x + icon_w + 4 + (96 - 32 - 10), y + (64 - 32)/2, 0.5f);

            y += icon_padding_y + icon_h;
        }
    }
    else
    {
        const C2D_Image* icons[3] = {
            &width_image,
            &height_image,
            &bomb_image,
        };
        constexpr Editing edit[3] = {
            Editing::Width,
            Editing::Height,
            Editing::Bombs,
        };
        short* parts[3] = {
            &width,
            &height,
            &bombpercent,
        };

        int y = 0;

        constexpr int icon_w = 64, icon_h = 64;
        constexpr int icon_base_x = 8, icon_base_y = 16;
        constexpr int icon_padding_y = 8;
        // constexpr int outline_w = 96;
        // constexpr int arrow_h = 32;

        constexpr int ok_x = 320 - 8 - 96;
        constexpr int ok_y = (240 - 64)/2;

        y = icon_base_y;
        for(int i = 0; i < 3; i++)
        {
            C2D_DrawImageAt(*(icons[i]), icon_base_x + (icon_w - icons[i]->subtex->width) / 2, y + (icon_h - icons[i]->subtex->height) / 2, 0.0f);
            
            C2D_DrawImageAt(outline_image, icon_base_x + 2 + icon_w + 4, y + 2, 0.0f, &back_tint);
            // C2D_DrawImageAt(up_image, icon_base_x + 2 + icon_w + 4 + outline_w + 4, y + 2, 0.0f, &back_tint);
            // C2D_DrawImageAt(down_image, icon_base_x + 2 + icon_w + 4 + outline_w + 4, y + 2 + arrow_h, 0.0f, &back_tint);

            C2D_DrawImageAt(outline_image, icon_base_x + icon_w + 4, y, 0.25f, edit[i] == selected_editing ? &selected_tint : &front_tint);
            // C2D_DrawImageAt(up_image, icon_base_x + icon_w + 4 + outline_w + 4, y, 0.25f, &front_tint);
            // C2D_DrawImageAt(down_image, icon_base_x + icon_w + 4 + outline_w + 4, y + arrow_h, 0.25f, &front_tint);

            div_t vals = div(*(parts[i]), 10);
            C2D_DrawImageAt(numbers_images[vals.quot], icon_base_x + icon_w + 4 + 16, y + (64 - 32)/2, 0.5f);
            C2D_DrawImageAt(numbers_images[vals.rem], icon_base_x + icon_w + 4 + 16 + 32, y + (64 - 32)/2, 0.5f);

            y += icon_padding_y + icon_h;
        }

        C2D_DrawImageAt(outline_image, ok_x + 2, ok_y + 2, 0.0f, &back_tint);
        C2D_DrawImageAt(outline_image, ok_x, ok_y, 0.25f, Editing::Ok == selected_editing ? &selected_tint : &front_tint);
        C2D_DrawImageAt(ok_image, ok_x + (96 - 64)/2, ok_y + (64 - 32)/2, 0.5f, &front_tint);
    }
}
void MineSweeper::renderLogo()
{
    constexpr float x = (400.0f - 128.0f) / 2.0f;
    constexpr float y = (240.0f - 128.0f) / 2.0f;
    C2D_DrawImageAt(logo_image, x, y, 0.0f);
}
void MineSweeper::renderCrosshair()
{
    constexpr float x = (400.0f - 32.0f) / 2.0f;
    constexpr float y = (240.0f - 32.0f) / 2.0f;
    C2D_DrawImageAt(crosshair_image, x, y, 1.0f, &crosshair_tint);
}

void MineSweeper::updateFloor()
{
    const float miny = get_terrain_min_y();
    const float minx = get_terrain_min_x();
    // const float maxy = -miny;
    const float maxx = -minx;

    constexpr float floor_dx[6] = {0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f};
    constexpr float floor_dz[6] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f};
    
    // planes offset vertically
    constexpr float floor_dy[2] = {
        0.0f,
        0.0625f/8.0f,
    };

    // outer: {bottom layer, top layer}
    // inner: {vert 0 1 2 3 0 2 3}
    float vs[6 + 8][6];
    float us[6 + 8][6];
    const Tex3DS_SubTexture* subtexes[6 + 8] = {
        hidden_image.subtex,
        open_image.subtex,
        red_image.subtex,
        empty_image.subtex,
        bomb_image.subtex,
        flag_image.subtex,
    };
    for(int i = 1; i <= 8; i++)
    {
        subtexes[i + 5] = numbers_images[i].subtex;
    }
    for(int typ = 0; typ < (6+8); typ++)
    {
        auto sub = subtexes[typ];
        for(int vert = 0; vert < 6; vert++)
        {
            subtex_uv_funcs[vert](sub, &us[typ][vert], &vs[typ][vert]);
        }
    }

    size_t idx = floor_idx;
    const size_t layer_size = width * height;
    C3D_FVec normal_floor_up = FVec3_New(0.0f, 1.0f, 0.0f);
    for(size_t layer = 0; layer < 2; layer++)
    {
        float x = minx;
        float y = miny;
        for(size_t square = 0; square < layer_size; square++)
        {
            int subtex_idx = -1;
            if(layer == 0)
            {
                if(visible[square] == '.')
                    subtex_idx = 0;
                else if(visible[square] == 'f')
                    subtex_idx = 0;
                else if(visible[square] == '#')
                    subtex_idx = 2;
                else
                    subtex_idx = 1;
            }
            else
            {
                if(visible[square] == '.' || visible[square] == ' ')
                    subtex_idx = 3;
                else if(visible[square] == '#')
                    subtex_idx = 4;
                else if(visible[square] == 'f')
                    subtex_idx = 5;
                else
                    subtex_idx = 6 + (visible[square] - '1');
            }
            for(size_t vert = 0; vert < 6; vert++)
            {
                C3D_FVec pos = FVec3_New(x + floor_dx[vert], -1.0f + floor_dy[layer], y + floor_dz[vert]);
                Vertex v(pos, us[subtex_idx][vert], vs[subtex_idx][vert], normal_floor_up);
                vertices[idx] = v;
                idx++;
            }

            x += 1.0f;
            if(x >= maxx)
            {
                x = minx;
                y += 1.0f;
            }
        }
    }
}

void MineSweeper::updateCursorUVAndPos()
{
    float cursor_u[6];
    float cursor_v[6];
    const Tex3DS_SubTexture* cursor_subtex = cursor_images[cursor_frame].subtex;
    for(int vert = 0; vert < 6; vert++)
    {
        subtex_uv_funcs[vert](cursor_subtex, &cursor_u[vert], &cursor_v[vert]);
    }

    const float miny = get_terrain_min_y();
    const float minx = get_terrain_min_x();

    constexpr float cursor_dx[6] = {0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f};
    constexpr float cursor_dz[6] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f};

    C3D_FVec normal_up = FVec3_New(0.0f, 1.0f, 0.0f);
    const float x = miny + float(looking_at_x);
    const float y = miny + float(looking_at_y);
    for(size_t vert = 0; vert < 6; vert++)
    {
        C3D_FVec pos = FVec3_New(x + cursor_dx[vert], -1.0f + (0.0625f/4.0f), y + cursor_dz[vert]);
        Vertex v(pos, cursor_u[vert], cursor_v[vert], normal_up);
        vertices[vertices.size() - 6 + vert] = v;
    }
}

void MineSweeper::updateCursorLookingAt()
{
    const float miny = get_terrain_min_y();
    const float minx = get_terrain_min_x();
    C3D_FVec camera_pos_vec = FVec3_New(positionX, 0.0f, positionZ);
    C3D_FVec floor_normal_vec = FVec3_New(0.0f, 1.0f, 0.0f);
    C3D_FVec floor_pos_vec = FVec3_New(0.0f, -1.0f, 0.0f);
    C3D_FVec camera_dir_vec = FVec3_Normalize(FVec3_New(
        cosf(C3D_AngleFromDegrees(angleX))*cosf(C3D_AngleFromDegrees(angleY)),
        sinf(C3D_AngleFromDegrees(angleY)),
        sinf(C3D_AngleFromDegrees(angleX))*cosf(C3D_AngleFromDegrees(angleY))
    ));
    float dot = FVec3_Dot(floor_normal_vec, camera_dir_vec);
    if(dot != 0.0f)
    {
        float dist = FVec3_Dot(floor_normal_vec, FVec3_Subtract(floor_pos_vec, camera_pos_vec)) / dot;
        if(dist <= 3.16f)
        {
            C3D_FVec endpoint = FVec3_Add(camera_pos_vec, FVec3_Scale(camera_dir_vec, dist));
            int x = width - int(endpoint.x - minx) - 1;
            int y = height - int(endpoint.z - miny) - 1;
            int total = x + y * width;

            if(x < 0 || y < 0 || total < 0 || x >= width || y >= height) return;

            looking_at_x = x;
            looking_at_y = y;
            looking_at_floor = true;
            updateCursorUVAndPos();
            stuff_changed = true;
        }
    }
}

void MineSweeper::generateCursor(int looking_at_idx)
{
    updateCursorUVAndPos();
}

void MineSweeper::generateFloorLayers(size_t& idx, size_t layer_size)
{
    const float miny = get_terrain_min_y();
    const float minx = get_terrain_min_x();
    // const float maxy = -miny;
    const float maxx = -minx;

    constexpr float floor_dx[6] = {0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f};
    constexpr float floor_dz[6] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f};
    
    // planes offset vertically
    constexpr float floor_dy[2] = {
        0.0f,
        0.0625f/8.0f,
    };

    // outer: {bottom layer, top layer}
    // inner: {vert 0 1 2 3 0 2 3}
    float us[2][6];
    float vs[2][6];
    const Tex3DS_SubTexture* subtexes[2] = {
        hidden_image.subtex,
        empty_image.subtex,
    };
    for(int layer = 0; layer < 2; layer++)
    {
        auto sub = subtexes[layer];
        for(int vert = 0; vert < 6; vert++)
        {
            subtex_uv_funcs[vert](sub, &us[layer][vert], &vs[layer][vert]);
        }
    }

    C3D_FVec normal_floor_up = FVec3_New(0.0f, 1.0f, 0.0f);
    for(size_t layer = 0; layer < 2; layer++)
    {
        float x = minx;
        float y = miny;
        for(size_t square = 0; square < layer_size; square++)
        {
            for(size_t vert = 0; vert < 6; vert++)
            {
                C3D_FVec pos = FVec3_New(x + floor_dx[vert], -1.0f + floor_dy[layer], y + floor_dz[vert]);
                Vertex v(pos, us[layer][vert], vs[layer][vert], normal_floor_up);
                vertices[idx] = v;
                idx++;
            }

            x += 1.0f;
            if(x >= maxx)
            {
                x = minx;
                y += 1.0f;
            }
        }
    }
}

void MineSweeper::generateWalls(size_t& idx)
{
    const float miny = get_terrain_min_y();
    const float minx = get_terrain_min_x();
    const float maxy = -miny;
    const float maxx = -minx;

    constexpr float wall_deltadir[6] = {0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f};
    constexpr float wall_dy[6] = {0.0f, 0.0f, 2.0f, 0.0f, 2.0f, 2.0f};

    float wall_u[6];
    float wall_v[6];
    const Tex3DS_SubTexture* wall_subtex = wall_image.subtex;
    for(int vert = 0; vert < 6; vert++)
    {
        subtex_uv_funcs[vert](wall_subtex, &wall_u[vert], &wall_v[vert]);
    }

    C3D_FVec normal_wall_at_left = FVec3_New(+1.0f, 0.0f, 0.0f);
    C3D_FVec normal_wall_at_right = FVec3_New(-1.0f, 0.0f, 0.0f);
    C3D_FVec normal_wall_at_top = FVec3_New(0.0f, 0.0f, +1.0f);
    C3D_FVec normal_wall_at_bottom = FVec3_New(0.0f, 0.0f, -1.0f);

    for(int i = 0; i < width; i++)
    {
        const float x = float(i) + minx;
        for(size_t vert = 0; vert < 6; vert++)
        {
            C3D_FVec poshi = FVec3_New(x + wall_deltadir[vert], -1.0f + wall_dy[vert], miny + 0.0625f/4.0f);
            C3D_FVec poslo = FVec3_New(x + wall_deltadir[vert], -1.0f + wall_dy[vert], maxy - 0.0625f/4.0f);

            Vertex hi(poshi, wall_u[vert], wall_v[vert], normal_wall_at_top);
            Vertex lo(poslo, wall_u[vert], wall_v[vert], normal_wall_at_bottom);

            vertices[idx + 0] = hi;
            vertices[idx + 6] = lo;
            idx++;
        }
        idx += 6;
    }

    for(int i = 0; i < height; i++)
    {
        const float y = float(i) + miny;
        for(size_t vert = 0; vert < 6; vert++)
        {
            C3D_FVec poshi = FVec3_New(minx + 0.0625f/4.0f, -1.0f + wall_dy[vert], y + wall_deltadir[vert]);
            C3D_FVec poslo = FVec3_New(maxx - 0.0625f/4.0f, -1.0f + wall_dy[vert], y + wall_deltadir[vert]);
            
            Vertex hi(poshi, wall_u[vert], wall_v[vert], normal_wall_at_left);
            Vertex lo(poslo, wall_u[vert], wall_v[vert], normal_wall_at_right);
            
            vertices[idx + 0] = hi;
            vertices[idx + 6] = lo;
            idx += 1;
        }
        idx += 6;
    }
}

void MineSweeper::generateVertices()
{
    vertices.clear();
    const float miny = get_terrain_min_y();
    const float minx = get_terrain_min_x();
    // const float maxy = -miny;
    // const float maxx = -minx;
    const size_t layer_size = width * height;
    const size_t vert_count = (1 + (width * 2 + height * 2) + layer_size * 2) * 6;

    vertices.resize(vert_count);

    generateCursor(0);

    size_t idx = 0;
    floor_idx = idx;
    generateFloorLayers(idx, layer_size);
    generateWalls(idx);
}

void MineSweeper::update(u32 kDown, u32 kHeld, touchPosition touch)  
{
    if(playing)
    {
        if((kDown | kHeld) & KEY_LEFT)
        {
            lookLeft(ROTATE_SPEED);
        }
        else if((kDown | kHeld) & KEY_RIGHT)
        {
            lookRight(ROTATE_SPEED);
        }
        else if((kDown | kHeld) & KEY_UP)
        {
            lookUp(ROTATE_SPEED);
        }
        else if((kDown | kHeld) & KEY_DOWN)
        {
            lookDown(ROTATE_SPEED);
        }

        if((kDown | kHeld) & (KEY_X | KEY_CSTICK_UP)) // move forward
        {
            goForward(MOVEMENT_SPEED);
        }
        else if((kDown | kHeld) & (KEY_Y | KEY_CSTICK_LEFT)) // move left
        {
            goLeft(MOVEMENT_SPEED);
        }
        else if((kDown | kHeld) & (KEY_A | KEY_CSTICK_RIGHT)) // move right
        {
            goRight(MOVEMENT_SPEED);
        }
        else if((kDown | kHeld) & (KEY_B | KEY_CSTICK_DOWN)) // move backwards
        {
            goBackwards(MOVEMENT_SPEED);
        }

        if(dead || win)
        {
            should_update_cursor = false;
            if(kDown & (KEY_L | KEY_R))
            {
                playing = false;
            }
        }
        else if(looking_at_floor)
        {
            if(kDown & KEY_L) // place a fLag
            {
                if(generated)
                {
                    placeFlag();
                    stuff_changed = true;
                    updateFloor();
                }
            }
            else if(kDown & KEY_R) // Reveal a square
            {
                if(!generated)
                {
                    generateBombs();
                    generated = true;
                }
                
                const int pos = XY_TO_IDX(looking_at_x, looking_at_y, this);
                DEBUGPRINT("clicked on: '%c'\n", visible[pos]);
                if(visible[pos] != 'f')
                {
                    reveal();
                    updateFloor();
                }
            }
        }

        if(should_update_cursor)
        {
            updateCursorLookingAt();
            should_update_cursor = false;
        }

        if(looking_at_floor)
        {
            framectr++;
            if(framectr % 30 == 0)
            {
                framectr = 0;
                cursor_frame += cursor_frame_dir;
                if(cursor_frame == 2)
                {
                    cursor_frame_dir = -1;
                }
                else if(cursor_frame == 0)
                {
                    cursor_frame_dir = 1;
                }
                updateCursorUVAndPos();
                stuff_changed = true;
            }
        }

        if(stuff_changed)
        {
            LevelWide::set_verts(vertices);
            stuff_changed = false;
        }
    }
    else
    {
        if(kDown & KEY_A)
        {
            DEBUGPRINT("A noplay\n");
            if(selected_editing == Editing::Ok)
            {
                DEBUGPRINT("playing now\n");
                playing = true;
                stuff_changed = true;
                looking_at_floor = false;
                should_update_cursor = false;
                generated = false;
                cursor_frame = 0;
                cursor_frame_dir = 1;
                framectr = 0;
                flags_count = 0;
                dead = false;
                win = false;
                angleX = 0.0f;
                angleY = 0.0f;
                positionX = 0.0f;
                positionZ = 0.0f;
                bombs = width * height / bombpercent;
                generateVertices();
                LevelWide::init(vertices);
            }
            else selected_editing = Editing::Ok;
        }
        else if(kDown & KEY_X)
        {
            DEBUGPRINT("X noplay\n");
            selected_editing = Editing::Width;
        }
        else if(kDown & KEY_Y)
        {
            DEBUGPRINT("Y noplay\n");
            selected_editing = Editing::Height;
        }
        else if(kDown & KEY_B)
        {
            DEBUGPRINT("B noplay\n");
            selected_editing = Editing::Bombs;
        }
        else if(kDown & KEY_UP)
        {
            DEBUGPRINT("UP noplay\n");
            switch(selected_editing)
            {
                case Editing::Width:
                {
                    if(width != MAX_SZ) width++;
                }
                break;
                case Editing::Height:
                {
                    if(height != MAX_SZ) height++;
                }
                break;
                case Editing::Bombs:
                {
                    if(bombpercent != MAX_BOMBS_PERCENT) bombpercent++;
                }
                break;
                case Editing::Ok:
                    break;
            }
        }
        else if(kDown & KEY_DOWN)
        {
            DEBUGPRINT("DOWN noplay\n");
            switch(selected_editing)
            {
                case Editing::Width:
                {
                    if(width != MIN_SZ) width--;
                }
                break;
                case Editing::Height:
                {
                    if(height != MIN_SZ) height--;
                }
                break;
                case Editing::Bombs:
                {
                    if(bombpercent != MIN_BOMBS_PERCENT) bombpercent--;
                }
                break;
                case Editing::Ok:
                    break;
            }
        }
        else if(kDown & KEY_LEFT)
        {
            DEBUGPRINT("LEFT noplay\n");
            switch(selected_editing)
            {
                case Editing::Width:
                {
                    if(width <= (MAX_SZ - 10)) width += 10;
                }
                break;
                case Editing::Height:
                {
                    if(height <= (MAX_SZ - 10)) height += 10;
                }
                break;
                case Editing::Bombs:
                {
                    if(bombpercent <= (MAX_BOMBS_PERCENT - 10)) bombpercent += 10;
                }
                break;
                case Editing::Ok:
                    break;
            }
        }
        else if(kDown & KEY_RIGHT)
        {
            DEBUGPRINT("RIGHT noplay\n");
            switch(selected_editing)
            {
                case Editing::Width:
                {
                    if(width >= (MIN_SZ + 10)) width -= 10;
                }
                break;
                case Editing::Height:
                {
                    if(height >= (MIN_SZ + 10)) height -= 10;
                }
                break;
                case Editing::Bombs:
                {
                    if(bombpercent >= (MIN_BOMBS_PERCENT + 10)) bombpercent -= 10;
                }
                break;
                case Editing::Ok:
                    break;
            }
        }
    }
}