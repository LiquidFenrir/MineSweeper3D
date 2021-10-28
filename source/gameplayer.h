#ifndef GAMEPLAYER_INC
#define GAMEPLAYER_INC

namespace game {

struct player {
    float x{0.0f}, y{0.0f};
    float yaw{0.0f}, pitch{0.0f};
    char name[20];
    bool on_map{false};
};

}

#endif
