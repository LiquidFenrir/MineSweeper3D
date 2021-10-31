#include "gameboard.h"
#include <cstring>
#include <algorithm>
#include "debugging.h"
#include "useful_utilities.h"

void game::board::dump(const char* filename)
{
    int x_min{0}, x_max{actual_width}, y_min{0}, y_max{actual_height};

    switch(current_board_mode)
    {
    case mode::regular:
        break;
    case mode::loop_vertical:
        y_min -= actual_height;
        y_max += actual_height;
        break;
    case mode::loop_horizontal:
        x_min -= actual_width;
        x_max += actual_width;
        break;
    case mode::loop_both:
        x_min -= actual_width;
        x_max += actual_width;
        y_min -= actual_height;
        y_max += actual_height;
        break;
    }

    auto holder = std::make_unique<char[]>((x_max - x_min + 1) * (y_max - y_min));
    if(!holder)
        return;

    char* out = holder.get();
    for(int y = y_min; y < y_max; ++y)
    {
        for(int x = x_min; x < x_max; ++x)
        {
            const auto c = get_cell({x, y});
            char val = '_';
            if(c.content < 0)
            {
                val = '*';
            }
            else if(c.content == 0)
            {
                val = '.';
            }
            else
            {
                val = '0' + c.content;
            }
            *(out++) = val;
        }
        *(out++) = '\n';
    }

    util::file_ptr file(fopen(filename, "w"));
    if(file)
        fwrite(holder.get(), 1, out - holder.get(), file.get());
}

game::board::state game::board::get_current_state() const
{
    return current_state;
}

game::board::mode game::board::get_current_mode() const
{
    return current_board_mode;
}

game::board::numbers game::board::get_numbers_info() const
{
    return {{actual_width, actual_height}, mine_count};
}

std::pair<int, int> game::board::reset(const numbers& nums, const mode board_mode, const std::time_t seed_value)
{
    debugging::log("resetting board with W/H/M {}/{}/{}, mode {} and seed: {}\n", nums.dims.width, nums.dims.height, nums.mines, int(board_mode), seed_value);
    cells.fill(cell{});

    unsigned char seed_bytes[sizeof(seed_value)];
    std::memcpy(seed_bytes, &seed_value, sizeof(seed_value));
    std::seed_seq seed(std::begin(seed_bytes), std::end(seed_bytes));
    gen.seed(seed);

    switch_to_state(state::viewing);
    current_board_mode = board_mode;
    actual_width = nums.dims.width;
    actual_height = nums.dims.height;
    mine_count = nums.mines;

    return initial_render();
}

void game::board::initialize_mines(location pos)
{
    switch_to_state(state::playing);

    // fill the coordinates array with all the possible locations given the size of the board
    {
        int y = 0;
        int x = 0;
        
        for(auto& coord : coordinates)
        {
            coord = {x, y};
            if(++x == actual_width)
            {
                x = 0;
                ++y;
                if(y == actual_height)
                {
                    break;
                }
            }
        }
    }

    // shuffle the locations we just filled to have random mine locations
    std::shuffle(coordinates.begin(), coordinates.begin() + (actual_height * actual_width), gen);

    // put mines in the order
    int current_mine = 0;
    debugging::log("want: {} mines\n", mine_count);
    debugging::log("have: {} points to check\n", actual_height * actual_width);
    for(auto mine_coord : std::span(coordinates.begin(), coordinates.begin() + (actual_height * actual_width)))
    {
        if(current_mine == mine_count)
            break;

        // don't put mines when we are too close to the click location (to make starting easier)
        const auto adjacent_x = (std::abs(mine_coord.x - pos.x) % actual_width) <= 1;
        const auto adjacent_y = (std::abs(mine_coord.y - pos.y) % actual_height) <= 1;

        if(adjacent_x && adjacent_y)
            continue;

        // trick: a mine is indicated by a negative content
        // the max amount of adjacent mines is 8 (+ self)
        // setting to -10 ensures we always get a negative value for mines
        get_cell(mine_coord).content = -10;
        current_mine++;

        for(int x = -1; x <= 1; ++x)
        {
            const auto new_x = mine_coord.x + x;
            const bool x_oob = new_x < 0 || new_x >= actual_width;
            for(int y = -1; y <= 1; ++y)
            {
                const auto new_y = mine_coord.y + y;
                const bool y_oob = new_y < 0 || new_y >= actual_height;

                // depending on the board mode, looping is allowed in some directions
                // get_cell handles everything fine, just need to constrain here
                switch(current_board_mode)
                {
                case mode::regular:
                    if(x_oob || y_oob) break;
                    get_cell({new_x, new_y}).content++;
                    break;
                case mode::loop_vertical:
                    if(x_oob) break;
                    get_cell({new_x, new_y}).content++;
                    break;
                case mode::loop_horizontal:
                    if(y_oob) break;
                    get_cell({new_x, new_y}).content++;
                    break;
                case mode::loop_both:
                    get_cell({new_x, new_y}).content++;
                    break;
                }
            }
        }
    }

    debugging::log("have: {} mines\n", current_mine);
    if(current_mine != mine_count)
    {
        throw std::runtime_error("WTF HAPPENED? NOT ENOUGH MINES BRO");
    }
}

// depth first search from v1 but with a stack instead of recursion
void game::board::visit_all_adjacent_empty(location pos)
{
    int stack_size = 0;
    const auto push = [&](const location& c) {
        coordinates[stack_size++] = c;
    };
    const auto pop = [&]() -> location {
        return coordinates[--stack_size];
    };

    std::optional<int> x_min, x_max, y_min, y_max;

    switch(current_board_mode)
    {
    case mode::regular:
        x_min = 0;
        x_max = actual_width - 1;
        y_min = 0;
        y_max = actual_height - 1;
        break;
    case mode::loop_vertical:
        x_min = 0;
        x_max = actual_width - 1;
        break;
    case mode::loop_horizontal:
        y_min = 0;
        y_max = actual_height - 1;
        break;
    case mode::loop_both:
        break;
    }

    push(pos);
    while(stack_size)
    {
        pos = pop();
        auto& c = get_cell(pos);
        if(c.st == cell::state::revealed)
            continue;

        c.st = cell::state::revealed;
        update_render_tile_uv(ptrdiff_t(&c - cells.data()) + render_buffer_tiles_offset, c.content);
        if(c.content != 0)
            continue;

        const bool ok_x_min = !(x_min && pos.x == *x_min);
        const bool ok_x_max = !(x_max && pos.x == *x_max);
        const bool ok_y_min = !(y_min && pos.y == *y_min);
        const bool ok_y_max = !(y_max && pos.y == *y_max);
        #define MOD_POS(dx, dy) {pos.x + (dx), pos.y + (dy)}
        if(ok_x_min && ok_y_min)
        {
            push(MOD_POS(-1, -1));
        }
        if(ok_y_min)
        {
            push(MOD_POS(0, -1));
        }
        if(ok_x_max && ok_y_min)
        {
            push(MOD_POS(+1, -1));
        }
        if(ok_x_max)
        {
            push(MOD_POS(+1, 0));
        }
        if(ok_x_max && ok_y_max)
        {
            push(MOD_POS(+1, +1));
        }
        if(ok_y_max)
        {
            push(MOD_POS(0, +1));
        }
        if(ok_x_min && ok_y_max)
        {
            push(MOD_POS(-1, +1));
        }
        if(ok_x_min)
        {
            push(MOD_POS(-1, 0));
        }
        #undef MOD_POS
    }
}

bool game::board::reveal_at(location pos)
{
    if(current_state == state::lost || current_state == state::won)
        return false;

    if(auto& cell = get_cell(pos); cell.st == cell::state::none)
    {
        if(get_current_state() == state::viewing)
        {
            initialize_mines(pos);
        }

        // clicked a mine, lost and showed all the mines
        if(cell.content < 0)
        {
            switch_to_state(state::lost);
            for(auto& board_cell : std::span(cells.data(), actual_width * actual_height))
            {
                if(board_cell.content < 0)
                {
                    update_render_tile_uv(ptrdiff_t(&board_cell - cells.data()) + render_buffer_tiles_offset, 11);
                    board_cell.st = cell::state::exploded;
                }
            }
            return true;
        }
        // clicked a cell adjacent to a mine, reveal that one only
        else if(cell.content > 0)
        {
            update_render_tile_uv(ptrdiff_t(&cell - cells.data()) + render_buffer_tiles_offset, cell.content);
            cell.st = cell::state::revealed;
        }
        // clicked a safe cell, reveal all the safe area and edge
        else
        {
            visit_all_adjacent_empty({pos.x, pos.y});
        }

        // check victory:
        for(const auto& board_cell : std::span(cells.data(), actual_width * actual_height))
        {
            if(board_cell.content >= 0 && board_cell.st != cell::state::revealed)
            {
                return true;
            }
        }
        switch_to_state(state::won);
        return true;
    }
    return false;
}

std::pair<bool, bool> game::board::flag_at(location pos)
{
    if(get_current_state() == state::playing)
    {
        auto& cell = get_cell(pos);
        if(cell.st == cell::state::none)
        {
            cell.st = cell::state::flagged;
            update_render_tile_uv(ptrdiff_t(&cell - cells.data()) + render_buffer_tiles_offset, 10);
            return {true, true};
        }
        else if(cell.st == cell::state::flagged)
        {
            cell.st = cell::state::none;
            update_render_tile_uv(ptrdiff_t(&cell - cells.data()) + render_buffer_tiles_offset, 9);
            return {true, false};
        }
    }
    return {false, false};
}

int game::board::get_cell_content_index(location pos)
{
    const auto& c = get_cell(pos);
    switch(c.st)
    {
    case cell::state::revealed:
        return c.content;
    case cell::state::flagged:
        return 10;
    case cell::state::none:
        return 9;
    case cell::state::exploded:
        return 11;
    }
    return -1;
}

game::board::cell& game::board::get_cell(location pos)
{
    return cells[((pos.x + actual_width) % actual_width) + ((pos.y + actual_height) % actual_height) * actual_width];
}
game::board::cell& game::board::get_cell(int index)
{
    return cells[(index + (actual_height * actual_width)) % (actual_height * actual_width)];
}

void game::board::switch_to_state(state new_state)
{
    current_state = new_state;
}

/*
int game::board::initial_render()
{
    // not actual quaternions: they're all rotations by pi rad, but the length of the axis is important
    // therefore, keep an inverse length in the last member that's supposed to be 0
    constexpr auto negate_axis = [](const buffer_point::axis a) -> buffer_point::axis {
        return {-a.i, -a.j, -a.k, -a.r};
    };
    constexpr buffer_point::axis axis_x{
        0.5f,
        0.0f,
        0.0f,
        0.0f,
    };
    constexpr buffer_point::axis axis_y{
        0.0f,
        0.5f,
        0.0f,
        0.0f,
    };
    constexpr buffer_point::axis axis_z{
        0.0f,
        0.0f,
        0.5f,
        0.0f,
    };
    constexpr buffer_point::axis axis_nx = negate_axis(axis_x);
    constexpr buffer_point::axis axis_ny = negate_axis(axis_y);
    constexpr buffer_point::axis axis_nz = negate_axis(axis_z);

    int idx = 0;

    if(!(current_board_mode == mode::loop_vertical || current_board_mode == mode::loop_both))
    {
        for(int x = 0; x < actual_width; ++x)
        {
            {
            auto& cur = render_buffer[idx];

            cur.tex_num = 13;

            cur.p.x = x + 0.5f;
            cur.p.y = -0.5f;
            cur.p.z = actual_height;

            cur.a = axis_nx;
            cur.b = axis_y;

            ++idx;
            }
            {
            auto& cur = render_buffer[idx];

            cur.tex_num = 13;

            cur.p.x = x + 0.5f;
            cur.p.y = -0.5f;
            cur.p.z = 0.0f;

            cur.a = axis_x;
            cur.b = axis_y;

            ++idx;
            }
        }
    }

    if(!(current_board_mode == mode::loop_horizontal || current_board_mode == mode::loop_both))
    {
        for(int y = 0; y < actual_height; ++y)
        {
            {
            auto& cur = render_buffer[idx];

            cur.tex_num = 13;

            cur.p.x = 0;
            cur.p.y = -0.5f;
            cur.p.z = y + 0.5f;

            cur.a = axis_nz;
            cur.b = axis_y;

            ++idx;
            }
            {
            auto& cur = render_buffer[idx];

            cur.tex_num = 13;

            cur.p.x = actual_width;
            cur.p.y = -0.5f;
            cur.p.z = y + 0.5f;

            cur.a = axis_z;
            cur.b = axis_y;

            ++idx;
            }
        }
    }

    render_buffer_tiles_offset = idx;

    for(int y = 0; y < actual_height; ++y)
    {
        for(int x = 0; x < actual_width; ++x)
        {
            auto& cur = render_buffer[idx];
            cur.tex_num = 10;

            cur.p.x = x + 0.5f;
            cur.p.y = -1.0f;
            cur.p.z = y + 0.5f;

            cur.a = axis_nx;
            cur.b = axis_z;

            ++idx;
        }
    }

    return idx;
}
*/

std::pair<int, int> game::board::initial_render()
{
    constexpr auto negate_dir = [](const direction d) -> direction {
        return {-d.x, -d.y, -d.z};
    };
    constexpr direction dir_x{
        0.5f + __FLT_EPSILON__,
        0.0f,
        0.0f,
    };
    constexpr direction dir_y{
        0.0f,
        0.5f + __FLT_EPSILON__,
        0.0f,
    };
    constexpr direction dir_z{
        0.0f,
        0.0f,
        0.5f + __FLT_EPSILON__,
    };
    constexpr direction dir_nx = negate_dir(dir_x);
    constexpr direction dir_ny = negate_dir(dir_y);
    constexpr direction dir_nz = negate_dir(dir_z);

    int idx = 0;
    int idx_margin = 0;

    const auto set_render_tile = [&](const buffer_point::pos& center, const float tex_num, const direction& dir_a,  const direction& dir_b)
    {
        this->set_render_tile(idx, this->render_buffer, center, tex_num, dir_a, dir_b);
    };
    const auto set_margin_tile = [&](int x, int y)
    {
        this->set_render_tile(idx_margin, this->margin_buffer, {x + 0.5f, -1.0f, y + 0.5f}, 12, dir_x, dir_z);
    };

    if(!(current_board_mode == mode::loop_vertical || current_board_mode == mode::loop_both))
    {
        for(int x = 0; x < actual_width; ++x)
        {
            set_render_tile({x + 0.5f, -0.5f, float(actual_height)}, 13, dir_nx, dir_ny);
            ++idx;
            set_render_tile({x + 0.5f, -0.5f, 0.0f}, 13, dir_x, dir_ny);
            ++idx;
        }
    }

    if(!(current_board_mode == mode::loop_horizontal || current_board_mode == mode::loop_both))
    {
        for(int y = 0; y < actual_height; ++y)
        {
            set_render_tile({0.0f, -0.5f, y + 0.5f}, 13, dir_nz, dir_ny);
            ++idx;
            set_render_tile({float(actual_width), -0.5f, y + 0.5f}, 13, dir_z, dir_ny);
            ++idx;
        }
    }

    if(current_board_mode != mode::loop_both)
    {
        bool do_corners = true;
        bool do_tb = true;
        bool do_lr = true;
        switch(current_board_mode)
        {
        case mode::loop_vertical:
            do_tb = false;
            do_corners = false;
            break;
        case mode::loop_horizontal:
            do_lr = false;
            do_corners = false;
            break;
        default:
            break;
        }

        if(do_corners)
        {
            for(int y = 0; y < VERT_MARGIN; ++y)
            {
                for(int x = 0; x < HORI_MARGIN; ++x)
                {
                    set_margin_tile(x - HORI_MARGIN, y - VERT_MARGIN);
                    ++idx_margin;
                    set_margin_tile(x - HORI_MARGIN, y + actual_height);
                    ++idx_margin;
                    set_margin_tile(x + actual_width, y - VERT_MARGIN);
                    ++idx_margin;
                    set_margin_tile(x + actual_width, y + actual_height);
                    ++idx_margin;
                }
            }
        }
        if(do_tb)
        {
            for(int y = 0; y < VERT_MARGIN; ++y)
            {
                for(int x = 0; x < actual_width; ++x)
                {
                    set_margin_tile(x, y - VERT_MARGIN);
                    ++idx_margin;
                    set_margin_tile(x, y + actual_height);
                    ++idx_margin;
                }
            }
        }
        if(do_lr)
        {
            for(int y = 0; y < actual_height; ++y)
            {
                for(int x = 0; x < HORI_MARGIN; ++x)
                {
                    set_margin_tile(x - HORI_MARGIN, y);
                    ++idx_margin;
                    set_margin_tile(x + actual_width, y);
                    ++idx_margin;
                }
            }
        }
    }

    render_buffer_tiles_offset = idx;

    for(int y = 0; y < actual_height; ++y)
    {
        for(int x = 0; x < actual_width; ++x)
        {
            set_render_tile({x + 0.5f, -1.0f, y + 0.5f}, 9, dir_x, dir_z);
            ++idx;
        }
    }

    return {idx, idx_margin};
}

game::board::board(buffer_point* const render_output, buffer_point* const margin_output)
    : render_buffer(render_output)
    , margin_buffer(margin_output)
{

}

static constexpr float tex_mul = (64.0f / 1024.0f);

void game::board::set_render_tile(int index, buffer_point* const buf, const buffer_point::pos& center, const float tex_num, const direction& dir_a,  const direction& dir_b)
{
    index *= 6;
    const float ul = tex_num * tex_mul;
    const float ur = ul + tex_mul;
    const buffer_point tl{
        {
            center.x - dir_a.x - dir_b.x,
            center.y - dir_a.y - dir_b.y,
            center.z - dir_a.z - dir_b.z,
        },
        {
            ul,
            1.0f,
        }
    };
    const buffer_point tr{
        {
            center.x + dir_a.x - dir_b.x,
            center.y + dir_a.y - dir_b.y,
            center.z + dir_a.z - dir_b.z,
        },
        {
            ur,
            1.0f,
        }
    };
    const buffer_point bl{
        {
            center.x - dir_a.x + dir_b.x,
            center.y - dir_a.y + dir_b.y,
            center.z - dir_a.z + dir_b.z,
        },
        {
            ul,
            0.0f,
        }
    };
    const buffer_point br{
        {
            center.x + dir_a.x + dir_b.x,
            center.y + dir_a.y + dir_b.y,
            center.z + dir_a.z + dir_b.z,
        },
        {
            ur,
            0.0f,
        }
    };

    buf[index++] = tr;
    buf[index++] = tl;
    buf[index++] = bl;
    buf[index++] = tr;
    buf[index++] = bl;
    buf[index++] = br;
}

void game::board::update_render_tile_uv(int index, const float tex_num)
{
    index *= 6;

    const float ul = tex_num * tex_mul;
    const float ur = ul + tex_mul;
    const buffer_point::texcoords tl{
        ul,
        1.0f,
    };
    const buffer_point::texcoords tr{
        ur,
        1.0f,
    };
    const buffer_point::texcoords bl{
        ul,
        0.0f,
    };
    const buffer_point::texcoords br{
        ur,
        0.0f,
    };

    render_buffer[index++].tex = tr;
    render_buffer[index++].tex = tl;
    render_buffer[index++].tex = bl;
    render_buffer[index++].tex = tr;
    render_buffer[index++].tex = bl;
    render_buffer[index++].tex = br;
}

#include <3ds/types.h>
#include <c3d/maths.h>

int game::board::fill_cursor_positions(std::span<game::player> players, buffer_point* const cursors_output)
{
    int index = 0;
    const auto set_cursor = [&index, cursors_output](const buffer_point::pos& center)
    {
        constexpr direction dir_a{
            0.5f + __FLT_EPSILON__,
            0.0f,
            0.0f,
        };
        constexpr direction dir_b{
            0.0f,
            0.0f,
            0.5f + __FLT_EPSILON__,
        };

        const float ul = 0.25f;
        const float ur = ul + 0.25f;
        const buffer_point tl{
            {
                center.x - dir_a.x - dir_b.x,
                center.y - dir_a.y - dir_b.y,
                center.z - dir_a.z - dir_b.z,
            },
            {
                ul,
                1.0f,
            }
        };
        const buffer_point tr{
            {
                center.x + dir_a.x - dir_b.x,
                center.y + dir_a.y - dir_b.y,
                center.z + dir_a.z - dir_b.z,
            },
            {
                ur,
                1.0f,
            }
        };
        const buffer_point bl{
            {
                center.x - dir_a.x + dir_b.x,
                center.y - dir_a.y + dir_b.y,
                center.z - dir_a.z + dir_b.z,
            },
            {
                ul,
                0.0f,
            }
        };
        const buffer_point br{
            {
                center.x + dir_a.x + dir_b.x,
                center.y + dir_a.y + dir_b.y,
                center.z + dir_a.z + dir_b.z,
            },
            {
                ur,
                0.0f,
            }
        };

        cursors_output[index++] = tr;
        cursors_output[index++] = tl;
        cursors_output[index++] = bl;
        cursors_output[index++] = tr;
        cursors_output[index++] = bl;
        cursors_output[index++] = br;
    };

    for(auto& p : players)
    {
        if(!p.on_map || p.pitch < 18.0f)
        {
            p.cursor.reset();
            continue;
        }

        const float pitch = C3D_AngleFromDegrees(p.pitch);
        const float yaw = C3D_AngleFromDegrees(p.yaw);

        const float dist = 1.0f/std::tan(pitch);

        if(dist < 5.0f)
        {
            const C3D_FVec ray_vector = FVec3_New(
                dist * std::sin(yaw),
                0.0f,
                dist * -std::cos(yaw)
            );
            const C3D_FVec ray_pos = FVec3_New(
                p.x,
                0.0f,
                p.y
            );
            const C3D_FVec hit = FVec3_Add(ray_pos, ray_vector);
            p.cursor = {int(hit.x), int(hit.z)};
            set_cursor({p.cursor->x + 0.5f, -(1.0f - 0.0625f/16.0f), p.cursor->y + 0.5f});
        }
    }

    return index;
}