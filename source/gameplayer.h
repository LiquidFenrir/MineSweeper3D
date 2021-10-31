#ifndef GAMEPLAYER_INC
#define GAMEPLAYER_INC

#include <optional>

namespace game {

struct player {
    float x{0.0f}, y{0.0f};
    float yaw{0.0f}, pitch{0.0f};
    char name[20];
    int team_id{0};
    int color_index{0};
    struct cursor_pos {
        int x, y;
    };
    std::optional<cursor_pos> cursor;
};

struct team {
    enum class state {
        none,
        playing,
        lost,
        won,
    };
    state current_state{state::none};
};

}

#endif
