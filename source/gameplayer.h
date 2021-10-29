#ifndef GAMEPLAYER_INC
#define GAMEPLAYER_INC

#include <optional>

namespace game {

struct player {
    float x{0.0f}, y{0.0f};
    float yaw{0.0f}, pitch{0.0f};
    char name[20];
    bool on_map{false};
    struct cursor_pos {
        float x, y;
    };
    std::optional<cursor_pos> cursor;
};

}

#endif
