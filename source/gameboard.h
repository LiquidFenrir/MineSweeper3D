#ifndef GAMEBOARD_INC
#define GAMEBOARD_INC

#include "gameplayer.h"
#include <span>
#include <array>
#include <ctime>
#include <random>
#include <optional>

namespace game {

struct board {
    struct location {
        int x, y;
    };
    struct cell {
        enum class state : unsigned char {
            none,
            exploded,
            flagged,
            revealed,
        };
        state st{state::none};
        // negative indicates mine, positive indicates adjacent mines, 0 means safe
        signed char content{0};
    };

    static inline constexpr int MIN_WIDTH = 10;
    static inline constexpr int MIN_HEIGHT = 10;
    static inline constexpr int MIN_PERCENT = 10;
    static inline constexpr int MAX_WIDTH = 99;
    static inline constexpr int MAX_HEIGHT = 99;
    static inline constexpr int MAX_PERCENT = 20;

    struct numbers {
        struct size {
            int width;
            int height;
        };
        size dims;
        int mines;
    };
    enum class mode : int {
        regular,
        loop_vertical,
        loop_horizontal,
        loop_both,
    };
    // clear cells, set new height and width, prepare for generating
    // returns number of points in the render buffer
    int reset(const numbers& nums, const mode board_mode, const std::time_t seed_value);

    // returns true if the click changed anything
    bool reveal_at(location pos);
    bool flag_at(location pos);

    enum state {
        viewing,
        playing,
        won,
        lost,
    };
    state get_current_state() const;
    mode get_current_mode() const;
    numbers get_numbers_info() const;

    struct buffer_point_geo {
        struct pos {
            float x;
            float y;
            float z;
        };
        struct axis {
            float i;
            float j;
            float k;
            float r;
        };

        pos p;
        float tex_num;
        axis a;
        axis b;
    };
    struct buffer_point_vtx {
        struct pos {
            float x;
            float y;
            float z;
        };
        struct texcoords {
            float u;
            float v;
        };

        pos p;
        texcoords tex;
    };

    using buffer_point = buffer_point_vtx;

    board(buffer_point* const render_output);

    struct direction {
        float x, y, z;
    };
    void set_render_tile(int index, const buffer_point::pos& center, const float tex_num, const direction& dir_a,  const direction& dir_b);
    void update_render_tile_uv(int index, const float tex_num);
    void fill_cursor_positions(std::span<const game::player> players, buffer_point* const cursors_output);
    void dump(const char* filename);

private:
    int initial_render();

    cell& get_cell(location pos);
    cell& get_cell(int index);
    void initialize_mines(location pos);
    void visit_all_adjacent_empty(location pos);
    void switch_to_state(state new_state);

    std::array<cell, MAX_WIDTH * MAX_HEIGHT> cells;

    // used as a list of possible mine positions during generation
    // and as a stack when visiting adjacent safe cells
    std::array<location, MAX_WIDTH * MAX_HEIGHT> coordinates;
    int actual_width, actual_height, mine_count;
    std::mt19937 gen;
    state current_state;
    mode current_board_mode;

    buffer_point* const render_buffer;
    int render_buffer_tiles_offset;
};

}

#endif