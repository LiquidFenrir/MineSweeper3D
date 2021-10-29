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

int game::board::reset(const numbers& nums, const mode board_mode, const std::time_t seed_value)
{
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

// iterative flood fill algorithm
// span filler from wikipedia
/*
fn fill(x, y):
  if not Inside(x, y) then return
  let s = new empty stack or queue
  add (x, y) to s
  while s is not empty:
    Remove an (x, y) from s
    let lx = x
    while Inside(lx - 1, y):
      Set(lx - 1, y)
      lx = lx - 1
    while Inside(x, y):
      Set(x, y)
      x = x + 1
    scan(lx, x - 1, y + 1, s)
    scan(lx, x - 1, y - 1, s)

fn scan(lx, rx, y, s):
  let added = false
  for x in lx .. rx:
    if not Inside(x, y):
      added = false
    else if not added:
      Add (x, y) to s
      added = true
*/
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
        x_max = actual_width;
        y_min = 0;
        y_max = actual_height;
        break;
    case mode::loop_vertical:
        x_min = 0;
        x_max = actual_width;
        break;
    case mode::loop_horizontal:
        y_min = 0;
        y_max = actual_height;
        break;
    case mode::loop_both:
        break;
    }

    const auto inside = [&](location p) -> std::pair<cell*, bool>
    {
        if(x_min && *x_min > p.x) return std::make_pair(nullptr, false);
        if(x_max && *x_max <= p.x) return std::make_pair(nullptr, false);
        if(y_min && *y_min > p.y) return std::make_pair(nullptr, false);
        if(x_max && *x_max <= p.y) return std::make_pair(nullptr, false);

        cell* const cell_ptr = &get_cell({p.x - 1, p.y - 1});
        return std::make_pair(cell_ptr, cell_ptr->st != cell::state::revealed && cell_ptr->content >= 0);
    };

    const auto set = [this, cells_ptr = cells.data()](cell& c) -> void
    {
        c.st = cell::state::revealed;
        update_render_tile_uv(render_buffer_tiles_offset + ptrdiff_t(&c - cells_ptr), c.content);
    };

    const auto scan = [&](int lx, int rx, int y)
    {
        bool added = false;
        for(int x = lx; x <= rx; ++x)
        {
            if(!inside({x, y}).second)
            {
                added = false;
            }
            else if(!added)
            {
                push({x, y});
            }
        }
    };

    push(pos);
    /*
    while s is not empty:
        Remove an (x, y) from s
        let lx = x
        while Inside(lx - 1, y):
            Set(lx - 1, y)
            lx = lx - 1
        while Inside(x, y):
            Set(x, y)
            x = x + 1
        scan(lx, x - 1, y + 1, s)
        scan(lx, x - 1, y - 1, s)
    */
    while(stack_size)
    {
        auto [x, y] = pop();
        auto lx = x;
        std::pair<cell*, bool> tmp{nullptr, false};
        while((tmp = inside({lx - 1, y})).second)
        {
            set(*tmp.first);
            lx--;
        }
        while((tmp = inside({x, y})).second)
        {
            set(*tmp.first);
            x++;
        }
        scan(lx, x-1, y+1);
        scan(lx, x-1, y-1);
    }
    
}

bool game::board::reveal_at(location pos)
{
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
            for(auto& board_cell : cells)
            {
                if(board_cell.content < 0)
                {
                    update_render_tile_uv(ptrdiff_t(&board_cell - cells.data()) + render_buffer_tiles_offset, 11);
                    board_cell.st = cell::state::exploded;
                }
            }
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
            visit_all_adjacent_empty({pos.x + 1, pos.y + 1});
        }
        return true;
    }
    return false;
}

bool game::board::flag_at(location pos)
{
    if(auto& cell = get_cell(pos); get_current_state() == state::playing)
    {
        if(cell.st == cell::state::none)
        {
            cell.st = cell::state::flagged;
            update_render_tile_uv(ptrdiff_t(&cell - cells.data()) + render_buffer_tiles_offset, 10);
            return true;
        }
        else if(cell.st == cell::state::flagged)
        {
            cell.st = cell::state::none;
            update_render_tile_uv(ptrdiff_t(&cell - cells.data()) + render_buffer_tiles_offset, 9);
            return true;
        }
    }
    return false;
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

int game::board::initial_render()
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

    if(!(current_board_mode == mode::loop_vertical || current_board_mode == mode::loop_both))
    {
        for(int x = 0; x < actual_width; ++x)
        {
            set_render_tile(idx, {x + 0.5f, -0.5f, float(actual_height)}, 12, dir_nx, dir_ny);
            ++idx;
            set_render_tile(idx, {x + 0.5f, -0.5f, 0.0f}, 12, dir_x, dir_ny);
            ++idx;
        }
    }

    if(!(current_board_mode == mode::loop_horizontal || current_board_mode == mode::loop_both))
    {
        for(int y = 0; y < actual_height; ++y)
        {
            set_render_tile(idx, {0.0f, -0.5f, y + 0.5f}, 12, dir_nz, dir_ny);
            ++idx;
            set_render_tile(idx, {float(actual_width), -0.5f, y + 0.5f}, 12, dir_z, dir_ny);
            ++idx;
        }
    }

    render_buffer_tiles_offset = idx;

    for(int y = 0; y < actual_height; ++y)
    {
        for(int x = 0; x < actual_width; ++x)
        {
            set_render_tile(idx, {x + 0.5f, -1.0f, y + 0.5f}, 9, dir_x, dir_z);
            ++idx;
        }
    }

    return idx;
}

game::board::board(buffer_point* const render_output)
    : render_buffer(render_output)
{

}

static constexpr float tex_mul = (32.0f / 512.0f);

void game::board::set_render_tile(int index, const buffer_point::pos& center, const float tex_num, const direction& dir_a,  const direction& dir_b)
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

    render_buffer[index++] = tr;
    render_buffer[index++] = tl;
    render_buffer[index++] = bl;
    render_buffer[index++] = tr;
    render_buffer[index++] = bl;
    render_buffer[index++] = br;
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

void game::board::fill_cursor_positions(std::span<game::player> players, buffer_point* const cursors_output)
{
    int index = 0;
    const auto set_cursor = [&index, cursors_output](const buffer_point::pos& center, bool present)
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

        const float ul = (present ? 13 : 15) * tex_mul;
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
no_cursor:
            p.cursor.reset();
            set_cursor({0, 0, 0}, false);
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
            set_cursor({p.cursor->x + 0.5f, -1.0f + 0.0625f / 32.0f, p.cursor->y + 0.5f}, true);
            continue;
        }
        goto no_cursor;
    }
}