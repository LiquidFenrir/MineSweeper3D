#include "gameroom.h"
#include <cstring>
#include <cstdlib>
#include <memory>
#include <ranges>
#include <algorithm>

namespace {

static constexpr u32 wlancommID = 0xF680DA90;
static constexpr u8 gamever = 1;

}

game::basic_room::basic_room(const room_info& info_in)
    : info(info_in)
{
    udsGenerateDefaultNetworkStruct(&network, wlancommID, gamever, info.max_players);
}

game::basic_room::basic_room(const udsNetworkScanInfo& scan_info)
    : network(scan_info.network)
{
    udsGetNodeInfoUsername(&scan_info.nodes[0], creator_name);
    udsGetNetworkStructApplicationData(&network, &info, sizeof(room_info), nullptr);
}

game::room::room(const basic_room& to_join, std::optional<std::string_view> password)
    : basic(to_join)
    , is_host(false)
{

}

game::room::room(const room_info& info, std::optional<std::string_view> password)
    : basic(info)
    , is_host(true)
{
    udsCreateNetwork(&basic.network, password ? password->data() : "", password ? password->size() : 1, &bindctx, 6, UDS_DEFAULT_RECVBUFSIZE);
    u8 appdata[0x14]{0};
    if(password)
        basic.info.extra |= room_info::PASSWORD_PROTECTION;
    std::memcpy(appdata, &basic.info, sizeof(room_info));
    udsSetApplicationData(appdata, sizeof(appdata));
}
game::room::~room()
{
    if(is_host)
    {
        udsDestroyNetwork();
    }
    else
    {
        udsDisconnectNetwork();
    }
}

struct FreeDeleter {
    void operator()(void* ptr)
    {
        free(ptr);
    }
};
template<typename T>
using MFPtr = std::unique_ptr<T, FreeDeleter>;

void game::room_list::scan()
{
    const std::size_t tmpbuf_size = 0x4000;
    auto tmpbuf = std::make_unique<unsigned char[]>(tmpbuf_size);

    udsNetworkScanInfo *networks = nullptr;
    std::size_t total_networks = 0;

    udsScanBeacons(tmpbuf.get(), tmpbuf_size, &networks, &total_networks, wlancommID, gamever, nullptr, false);
    MFPtr<udsNetworkScanInfo[]> networks_holder(networks);

    list.clear();
    if(total_networks)
    {
        list.reserve(total_networks);
        for(auto& network : std::span(networks, networks + total_networks))
        {
            list.emplace_back(network);
        }
    }
}

int game::room_list::get_room_count(const filter& want) const
{
    const auto passes = [want](const basic_room& br) { return br.satisfies(want); };
    return std::count_if(list.begin(), list.end(), passes);
}

game::room_list::page_t game::room_list::get_page(const filter& want, int page_index) const
{
    const auto passes = [&want](const basic_room& br) { return br.satisfies(want); };

    auto tmp = list
        | std::views::filter(passes);

    auto result = tmp
        | std::views::drop(page_index * ROOMS_PER_PAGE)
        | std::views::take(ROOMS_PER_PAGE);

    page_t out;
    out.fill(nullptr);
    auto into = out.begin();
    for(const auto& br : result)
    {
        *(into++) = &br;
    }
    return out;
}
