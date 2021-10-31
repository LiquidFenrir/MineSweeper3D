#ifndef GAMEROOM_INC
#define GAMEROOM_INC

#include <3ds.h>
#include <array>
#include <string>
#include <optional>
#include <vector>
#include <span>

namespace game {

struct room_info {
    enum extra_info : unsigned char {
        PASSWORD_PROTECTION = BIT(0),
        LOOPING_VERT = BIT(1),
        LOOPING_HORI = BIT(2),
    };

    unsigned char identifier{0x3D};
    unsigned char map_width, map_height, mine_percentage;
    unsigned char current_players, max_players;
    unsigned char extra{};
};

struct filter {
    std::optional<bool> only_rooms_with_password_setting;
};

struct basic_room {
    friend struct room;

    explicit basic_room(const room_info& info_in);
    explicit basic_room(const udsNetworkScanInfo& scan_info);

    bool satisfies(const filter& want) const;
    const char* get_creator_name() const;

private:
    char creator_name[20];
    udsNetworkStruct network;
    room_info info;
};

struct room {
    static constexpr inline int MAX_PLAYERS = 8;
    static constexpr inline int MAX_TEAMS = MAX_PLAYERS;
    room(const basic_room& to_join, std::optional<std::string_view> password);
    room(const room_info& info, std::optional<std::string_view> password);
    ~room();

private:
    basic_room basic;
    udsBindContext bindctx;
    bool is_host;
};

struct room_list {
    void scan();

    static constexpr inline int ROOMS_PER_PAGE = 3;
    using page_t = std::array<const basic_room*, ROOMS_PER_PAGE>;

    int get_room_count(const filter& want) const;
    page_t get_page(const filter& want, int page_index) const;

private:
    std::vector<basic_room> list;
};

}

#endif
