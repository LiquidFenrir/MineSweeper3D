#include "transition_scene.h"
#include "../debugging.h"
#include "ingame_sprites.h"
#include <algorithm>
#include <ranges>
#include <random>
#include <ctime>
#include <span>

namespace {

static constexpr std::size_t pattern_index_to_image[] = {
    ingame_sprites_cell_hide_idx,
    ingame_sprites_cell_open_idx,
    ingame_sprites_cell_1_idx,
    ingame_sprites_cell_2_idx,
    ingame_sprites_cell_3_idx,
    ingame_sprites_cell_4_idx,
    ingame_sprites_cell_5_idx,
    ingame_sprites_cell_6_idx,
    ingame_sprites_cell_7_idx,
    ingame_sprites_cell_8_idx,
};

}

::scenes::transition_scene::transition_scene(game::scenes::scene_ptr before, game::scenes::scene_ptr after)
    : scene_before(std::move(before))
    , scene_after(std::move(after))
{
    const time_t seed_value = time(nullptr);
    unsigned char seed_bytes[sizeof(seed_value)];
    std::memcpy(seed_bytes, &seed_value, sizeof(seed_value));
    std::seed_seq seed(std::begin(seed_bytes), std::end(seed_bytes));
    std::mt19937 gen(seed);
    std::uniform_int_distribution<unsigned char> dist(0, std::size(pattern_index_to_image) - 1);

    std::generate(top_screen_pattern.begin(), top_screen_pattern.end(), [&]() { return dist(gen); });
    std::generate(bottom_screen_pattern.begin(), bottom_screen_pattern.end(), [&]() { return dist(gen); });

    this->clear_color_bottom = scene_before->clear_color_bottom;
    this->clear_color_top = scene_before->clear_color_top;
}

game::scenes::next_scene scenes::transition_scene::update(const ctr::hid& input, const double dt)
{
    
    if(input.pressed().any())
    {
ret_next:
        return scene_after;
    }

    switch(current_state)
    {
    case state::hiding:
        scene_before->tick(dt);
        break;
    case state::revealing:
        scene_after->tick(dt);
        break;
    case state::done:
        goto ret_next;
    default:
        break;
    }

    ++frame;
    return std::nullopt;
}

void scenes::transition_scene::draw(ctr::gfx& gfx)
{
    switch(current_state)
    {
    case state::hiding:
        scene_before->draw(gfx);
        break;
    case state::revealing:
        scene_after->draw(gfx);
        break;
    }

    C2D_Flush();
    gfx.render_2d();
    auto& sheet = *game_config->data.ingame_sheet;

    const auto do_column = [&](std::span<unsigned char> col, int x, float scale = 1.0f) {
        int y = 0;
        for(auto& cell : col)
        {
            C2D_DrawImageAt(sheet.get_image(pattern_index_to_image[cell]), x, y, 1.0f, nullptr, scale, 1.0f);
            y += cell_size;
        }
    };

    const int frames_per_col = 4;
    auto qr = std::div(frame, frames_per_col);
    const auto dims = gfx.get_screen_dimensions(GFX_TOP);
    const int q = ((dims.width - 320)/2)/cell_size;
    const int t = 240 / cell_size;
    const float s = qr.rem / float(frames_per_col);
    if(current_state == state::hiding)
    {
        gfx.get_screen(GFX_TOP)->focus();
        int x = 0;
        int d = 0;
        for(int i = 0; i < qr.quot; ++i)
        {
            do_column(top_screen_pattern | std::views::drop(d) | std::views::take(t), x);
            x += cell_size;
            d += t;
        }
        if(qr.rem)
        {
            do_column(top_screen_pattern | std::views::drop(d) | std::views::take(t), x, s);
        }

        if(qr.quot >= q && (qr.quot - q * 2) <= (320 / cell_size))
        {
            gfx.get_screen(GFX_BOTTOM)->focus();
            x = 0;
            d = 0;
            for(int i = 0; i < (qr.quot - q) && x < 320; ++i)
            {
                do_column(bottom_screen_pattern | std::views::drop(d) | std::views::take(t), x);
                x += cell_size;
                d += t;
            }
            if(qr.rem)
            {
                do_column(bottom_screen_pattern | std::views::drop(d) | std::views::take(t), x, s);
            }
        }

        if(qr.quot == dims.width/cell_size && (qr.rem + 1) == frames_per_col)
        {
            current_state = state::revealing;
            this->clear_color_bottom = scene_after->clear_color_bottom;
            this->clear_color_top = scene_after->clear_color_top;
        }
    }
    else if(current_state == state::revealing)
    {
        gfx.get_screen(GFX_TOP)->focus();
        qr.quot -= dims.width/cell_size;

        int x = dims.width - cell_size;
        int d = ((dims.width / cell_size) - 1) * t;
        for(int i = dims.width/cell_size; i > qr.quot; --i)
        {
            do_column(top_screen_pattern | std::views::drop(d) | std::views::take(t), x);
            x -= cell_size;
            d -= t;
        }

        do_column(top_screen_pattern | std::views::drop(d) | std::views::take(t), x + (s  * cell_size), 1.0f - s);

        if((qr.quot - q) <= (320/cell_size))
        {
            gfx.get_screen(GFX_BOTTOM)->focus();
            x = 320 - cell_size;
            d = bottom_screen_pattern.size() - t;
            for(int i = 320/cell_size; i > (qr.quot - q) && x >= 0; --i)
            {
                do_column(bottom_screen_pattern | std::views::drop(d) | std::views::take(t), x);
                x -= cell_size;
                d -= t;
            }

            if(0 <= (qr.quot - q) && (qr.quot - q) <= (320/cell_size))
            {
                do_column(bottom_screen_pattern | std::views::drop(d) | std::views::take(t), x + (s * cell_size), 1.0f - s);
            }
        }

        if(qr.quot == dims.width/cell_size && (qr.rem + 1) == frames_per_col)
        {
            current_state = state::done;
        }
    }
}
