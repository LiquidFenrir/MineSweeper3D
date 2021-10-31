#include "room_setup_scene.h"
#include "selection_scene.h"
#include "playing_scene.h"
#include "transition_scene.h"
#include "menu_sprites.h"
#include "curved_plane_drop_shbin.h"
#include "curved_plane_basic_shbin.h"
#include "menu_sprites.h"
#include <span>
#include <ranges>
#include <utility>
#include <algorithm>
#include <numeric>

namespace {

struct menu_structure_single_t {
    using source_t = ::scenes::room_setup_scene;
    using tab_t = game::scene_menu<source_t>::menu_tab;
    using entry_t = tab_t::menu_entry;
    std::array<entry_t, 3 * 3 + 2> entries_numbers;
    std::array<entry_t, 4 + 2> entries_type;
    std::array<entry_t, 2> entries_final;

    std::array<tab_t, 3> tabs;
};

static const menu_structure_single_t menu_structure = []() {
    menu_structure_single_t out;
    using source_t = menu_structure_single_t::source_t;
    using entry_t = menu_structure_single_t::entry_t;

    out.tabs[0].tab_index = 0;
    out.tabs[0].entries = out.entries_numbers;
    out.tabs[0].next_tab = &out.tabs[1];

    out.tabs[1].tab_index = 1;
    out.tabs[1].entries = out.entries_type;
    out.tabs[1].prev_tab = &out.tabs[0];
    out.tabs[1].next_tab = &out.tabs[2];

    out.tabs[2].tab_index = 2;
    out.tabs[2].entries = out.entries_final;
    out.tabs[2].prev_tab = &out.tabs[1];

    {
    const entry_t::func_t handler_cancel = [](source_t& scene, const entry_t& ent) -> const entry_t* {
        return &ent.parent_tab->entries[ent.parent_tab->entries.size() - 2];
    };
    const entry_t::func_t handlers_validate[] = {
        [](source_t& scene, const entry_t&) -> const entry_t* {
            scene.change_width(-1);
            return nullptr;
        },
        [](source_t& scene, const entry_t&) -> const entry_t* {
            scene.ask_width();
            return nullptr;
        },
        [](source_t& scene, const entry_t&) -> const entry_t* {
            scene.change_width(+1);
            return nullptr;
        },
        [](source_t& scene, const entry_t&) -> const entry_t* {
            scene.change_height(-1);
            return nullptr;
        },
        [](source_t& scene, const entry_t&) -> const entry_t* {
            scene.ask_height();
            return nullptr;
        },[](source_t& scene, const entry_t&) -> const entry_t* {
            scene.change_height(+1);
            return nullptr;
        },
        [](source_t& scene, const entry_t&) -> const entry_t* {
            scene.change_mine_percent(-1);
            return nullptr;
        },
        [](source_t& scene, const entry_t&) -> const entry_t* {
            scene.ask_mine_percent();
            return nullptr;
        },[](source_t& scene, const entry_t&) -> const entry_t* {
            scene.change_mine_percent(+1);
            return nullptr;
        },
        [](source_t& scene, const entry_t&) -> const entry_t* {
            scene.set_next_scene_to(scenes::transition_scene::create(scene.get_ptr(), scenes::selection_scene::create()));
            return nullptr;
        },
        [](source_t& scene, const entry_t& ent) -> const entry_t* {
            return &ent.parent_tab->next_tab->entries.front();
        },
    };

    int idx = 0;
    /*
     * .............
     * v/     /    v
     * 0 <-> 1 <-> 2
     * ^     ^     ^
     * v     v     v
     * 3 <-> 4 <-> 5
     * ^     ^     ^
     * v     v     v
     * 6 <-> 7 <-> 8
     * ^     \_>__ ^
     * ^          \v
     * 9 <------> 10
     * v ______>__/^
     * .............
     */

    // ROW 1
    out.entries_numbers[0].right_entry = &out.entries_numbers[1];
    out.entries_numbers[0].left_entry = &out.entries_numbers[2];
    out.entries_numbers[1].right_entry = &out.entries_numbers[2];
    out.entries_numbers[1].left_entry = &out.entries_numbers[0];
    out.entries_numbers[2].right_entry = &out.entries_numbers[0];
    out.entries_numbers[2].left_entry = &out.entries_numbers[1];

    out.entries_numbers[0].up_entry = &out.entries_numbers[10];
    out.entries_numbers[1].up_entry = &out.entries_numbers[10];
    out.entries_numbers[2].up_entry = &out.entries_numbers[10];
    out.entries_numbers[0].down_entry = &out.entries_numbers[3];
    out.entries_numbers[1].down_entry = &out.entries_numbers[4];
    out.entries_numbers[2].down_entry = &out.entries_numbers[5];

    // ROW 2
    out.entries_numbers[3].right_entry = &out.entries_numbers[4];
    out.entries_numbers[3].left_entry = &out.entries_numbers[5];
    out.entries_numbers[4].right_entry = &out.entries_numbers[5];
    out.entries_numbers[4].left_entry = &out.entries_numbers[3];
    out.entries_numbers[5].right_entry = &out.entries_numbers[3];
    out.entries_numbers[5].left_entry = &out.entries_numbers[4];

    out.entries_numbers[3].up_entry = &out.entries_numbers[0];
    out.entries_numbers[4].up_entry = &out.entries_numbers[1];
    out.entries_numbers[5].up_entry = &out.entries_numbers[2];
    out.entries_numbers[3].down_entry = &out.entries_numbers[6];
    out.entries_numbers[4].down_entry = &out.entries_numbers[7];
    out.entries_numbers[5].down_entry = &out.entries_numbers[8];

    // ROW 3
    out.entries_numbers[6].right_entry = &out.entries_numbers[7];
    out.entries_numbers[6].left_entry = &out.entries_numbers[8];
    out.entries_numbers[7].right_entry = &out.entries_numbers[8];
    out.entries_numbers[7].left_entry = &out.entries_numbers[6];
    out.entries_numbers[8].right_entry = &out.entries_numbers[6];
    out.entries_numbers[8].left_entry = &out.entries_numbers[7];

    out.entries_numbers[6].up_entry = &out.entries_numbers[3];
    out.entries_numbers[7].up_entry = &out.entries_numbers[4];
    out.entries_numbers[8].up_entry = &out.entries_numbers[5];
    out.entries_numbers[6].down_entry = &out.entries_numbers[10];
    out.entries_numbers[7].down_entry = &out.entries_numbers[10];
    out.entries_numbers[8].down_entry = &out.entries_numbers[10];

    // ROW 4 (back/ok)
    out.entries_numbers[9].right_entry = &out.entries_numbers[10];
    out.entries_numbers[9].left_entry = &out.entries_numbers[10];
    out.entries_numbers[10].right_entry = &out.entries_numbers[9];
    out.entries_numbers[10].left_entry = &out.entries_numbers[9];

    out.entries_numbers[9].up_entry = &out.entries_numbers[6];
    out.entries_numbers[10].up_entry = &out.entries_numbers[8];
    out.entries_numbers[9].down_entry = &out.entries_numbers[0];
    out.entries_numbers[10].down_entry = &out.entries_numbers[2];

    for(auto& entry : out.entries_numbers)
    {
        entry.entry_index = idx;
        entry.validate_func = handlers_validate[idx];
        entry.cancel_func = handler_cancel;
        entry.parent_tab = &out.tabs[0];
        ++idx;
    }
    }

    {
    const entry_t::func_t handler_cancel = [](source_t& scene, const entry_t& ent) -> const entry_t* {
        return &ent.parent_tab->entries[ent.parent_tab->entries.size() - 2];
    };
    const entry_t::func_t handlers_validate[] = {
        [](source_t& scene, const entry_t&) -> const entry_t* {
            scene.set_board_type(game::board::mode::regular);
            return nullptr;
        },
        [](source_t& scene, const entry_t&) -> const entry_t* {
            scene.set_board_type(game::board::mode::loop_horizontal);
            return nullptr;
        },
        [](source_t& scene, const entry_t&) -> const entry_t* {
            scene.set_board_type(game::board::mode::loop_vertical);
            return nullptr;
        },
        [](source_t& scene, const entry_t&) -> const entry_t* {
            scene.set_board_type(game::board::mode::loop_both);
            return nullptr;
        },
        [](source_t& scene, const entry_t& ent) -> const entry_t* {
            return &ent.parent_tab->prev_tab->entries.back();
        },
        [](source_t& scene, const entry_t& ent) -> const entry_t* {
            return &ent.parent_tab->next_tab->entries.front();
        },
    };

    int idx = 0;
    /*
     * ......v
     * v    \
     * 0 <-> 1
     * ^     ^
     * v     v
     * 2 <-> 3
     * ^ ___/^
     * v/    ^
     * 4 <-> 5
     * ^\___ v
     * .......
     */

    // ROW 1
    out.entries_type[0].right_entry = &out.entries_type[1];
    out.entries_type[0].left_entry = &out.entries_type[1];
    out.entries_type[1].right_entry = &out.entries_type[0];
    out.entries_type[1].left_entry = &out.entries_type[0];

    out.entries_type[0].up_entry = &out.entries_type[5];
    out.entries_type[1].up_entry = &out.entries_type[5];
    out.entries_type[0].down_entry = &out.entries_type[2];
    out.entries_type[1].down_entry = &out.entries_type[3];
    
    // ROW 2
    out.entries_type[2].right_entry = &out.entries_type[3];
    out.entries_type[2].left_entry = &out.entries_type[3];
    out.entries_type[3].right_entry = &out.entries_type[2];
    out.entries_type[3].left_entry = &out.entries_type[2];

    out.entries_type[2].up_entry = &out.entries_type[0];
    out.entries_type[3].up_entry = &out.entries_type[1];
    out.entries_type[2].down_entry = &out.entries_type[5];
    out.entries_type[3].down_entry = &out.entries_type[5];

    // ROW 3 (back/ok)
    out.entries_type[4].right_entry = &out.entries_type[5];
    out.entries_type[4].left_entry = &out.entries_type[5];
    out.entries_type[5].right_entry = &out.entries_type[4];
    out.entries_type[5].left_entry = &out.entries_type[4];

    out.entries_type[4].up_entry = &out.entries_type[2];
    out.entries_type[5].up_entry = &out.entries_type[3];
    out.entries_type[4].down_entry = &out.entries_type[0];
    out.entries_type[5].down_entry = &out.entries_type[1];

    for(auto& entry : out.entries_type)
    {
        entry.entry_index = idx;
        entry.validate_func = handlers_validate[idx];
        entry.cancel_func = handler_cancel;
        entry.parent_tab = &out.tabs[1];
        ++idx;
    }
    }

    {
    const entry_t::func_t handler_cancel = [](source_t& scene, const entry_t& ent) -> const entry_t* {
        return &ent.parent_tab->entries[ent.parent_tab->entries.size() - 2];
    };
    const entry_t::func_t handlers_validate[] = {
        [](source_t& scene, const entry_t& ent) -> const entry_t* {
            return &ent.parent_tab->prev_tab->entries.back();
        },
        [](source_t& scene, const entry_t&) -> const entry_t* {
            scene.prepare_starting();
            return nullptr;
        },
    };

    int idx = 0;
    /**
     * 
     * 0 <-> 1
     * 
     **/

    // ROW 1
    out.entries_final[0].right_entry = &out.entries_final[1];
    out.entries_final[0].left_entry = &out.entries_final[1];
    out.entries_final[1].right_entry = &out.entries_final[0];
    out.entries_final[1].left_entry = &out.entries_final[0];

    for(auto& entry : out.entries_final)
    {
        entry.entry_index = idx;
        entry.validate_func = handlers_validate[idx];
        entry.cancel_func = handler_cancel;
        entry.parent_tab = &out.tabs[2];
        ++idx;
    }
    }

    return out;
}();

}

scenes::room_setup_scene::room_setup_scene(bool multi)
    : is_multiplayer{multi}
    , menu(menu_structure.entries_numbers[0])
    , entry_index{0}
    , tab_index{0}
{
    numbers.dims.width = game::board::MIN_WIDTH;
    numbers.dims.height = game::board::MIN_HEIGHT;
    numbers.mines = game::board::MIN_PERCENT;
    board_mode = game::board::mode::regular;

    auto& gen = game_config->data.static_small_text_gen->get_active_buf();
    gen.clear();
    auto& menu_sheet = *game_config->data.menu_sheet;

    top_text[0] = gen.parse("Choose the board!\nHeight, width and\nmine percentage.");
    top_text[1] = gen.parse("Choose the board!\nNormal, or looping?");
    top_text[2] = gen.parse("Make sure your\nsettings are fine!");
    back_text = gen.parse("BACK");
    ok_text = gen.parse("OK");

    width_img = menu_sheet.get_image(menu_sprites_board_width_idx);
    height_img = menu_sheet.get_image(menu_sprites_board_height_idx);
    mines_img = menu_sheet.get_image(menu_sprites_board_mine_percent_idx);

    plus_img = menu_sheet.get_image(menu_sprites_button_face_plus_idx);
    minus_img = menu_sheet.get_image(menu_sprites_button_face_minus_idx);

    board_imgs[0][0] = menu_sheet.get_image(menu_sprites_board_regular_idx);
    board_imgs[0][1] = menu_sheet.get_image(menu_sprites_board_loop_h_idx);
    board_imgs[1][0] = menu_sheet.get_image(menu_sprites_board_loop_v_idx);
    board_imgs[1][1] = menu_sheet.get_image(menu_sprites_board_loop_both_idx);

    constexpr std::pair<C2D_Image game::button::parts::*, std::size_t> parts_idx[] = {
        {&game::button::parts::tl, menu_sprites_button_normal_top_left_idx},
        {&game::button::parts::bl, menu_sprites_button_normal_bottom_left_idx},
        {&game::button::parts::tr, menu_sprites_button_normal_top_right_idx},
        {&game::button::parts::br, menu_sprites_button_normal_bottom_right_idx},
        {&game::button::parts::vl, menu_sprites_button_normal_vertical_left_idx},
        {&game::button::parts::vr, menu_sprites_button_normal_vertical_right_idx},
        {&game::button::parts::ht, menu_sprites_button_normal_horizontal_top_idx},
        {&game::button::parts::hb, menu_sprites_button_normal_horizontal_bottom_idx},
    };
    button_parts.inner_color = {0xc0, 0xc0, 0xc0, 0xff};
    for(const auto& [part, idx] : parts_idx)
    {
        button_parts.*part = menu_sheet.get_image(idx);
    }

    constexpr std::pair<C2D_Image game::button::parts::*, std::size_t> selected_parts_idx[] = {
        {&game::button::parts::tl, menu_sprites_button_selected_top_left_idx},
        {&game::button::parts::bl, menu_sprites_button_selected_bottom_left_idx},
        {&game::button::parts::tr, menu_sprites_button_selected_top_right_idx},
        {&game::button::parts::br, menu_sprites_button_selected_bottom_right_idx},
        {&game::button::parts::vl, menu_sprites_button_selected_vertical_left_idx},
        {&game::button::parts::vr, menu_sprites_button_selected_vertical_right_idx},
        {&game::button::parts::ht, menu_sprites_button_selected_horizontal_top_idx},
        {&game::button::parts::hb, menu_sprites_button_selected_horizontal_bottom_idx},
    };
    selected_button_parts.inner_color = {0x50, 0x50, 0x50, 0xff};
    for(const auto& [part, idx] : selected_parts_idx)
    {
        selected_button_parts.*part = menu_sheet.get_image(idx);
    }

    {
    auto& buttons = tab_numbers_buttons.buttons;
    {
    // icon
    for(int i = 0; i < 3; ++i)
    {
        auto& button = buttons[i * 4];
        button.x = 35;
        button.y = 5 + (i * 64);
        button.w = 54;
        button.h = 54;
        button.selectable = false;
        button.associated_menu_entry = nullptr;
    }
    // -
    for(int i = 0; i < 3; ++i)
    {
        auto& button = buttons[i * 4 + 1];
        button.x = 121;
        button.y = 18 + (i * 64);
        button.w = 30;
        button.h = 30;
        button.selectable = true;
        button.associated_menu_entry = &menu_structure.entries_numbers[i * 3];
    }
    // number
    for(int i = 0; i < 3; ++i)
    {
        auto& button = buttons[i * 4 + 2];
        button.x = 154;
        button.y = 10 + (i * 64);
        button.w = 88;
        button.h = 44;
        button.selectable = true;
        button.associated_menu_entry = &menu_structure.entries_numbers[i * 3 + 1];
    }
    // +
    for(int i = 0; i < 3; ++i)
    {
        auto& button = buttons[i * 4 + 3];
        button.x = 245;
        button.y = 18 + (i * 64);
        button.w = 30;
        button.h = 30;
        button.selectable = true;
        button.associated_menu_entry = &menu_structure.entries_numbers[i * 3 + 2];
    }
    // back
    {
        auto& button = buttons[12];
        button.x = 20;
        button.y = 240 - 4 - 40;
        button.w = 120;
        button.h = 40;
        button.selectable = true;
        button.associated_menu_entry = &menu_structure.entries_numbers[9];
    }
    // ok
    {
        auto& button = buttons[13];
        button.x = 320 - 20 - 120;
        button.y = 240 - 4 - 40;
        button.w = 120;
        button.h = 40;
        button.selectable = true;
        button.associated_menu_entry = &menu_structure.entries_numbers[10];
    }
    }
    }

    {
    auto& buttons = tab_type_buttons.buttons;
    {
    // top
    for(int i = 0; i < 2; ++i)
    {
        auto& button = buttons[i];
        button.x = 64 + (112 * i);
        button.y = 20;
        button.w = 80;
        button.h = 80;
        button.selectable = true;
        button.associated_menu_entry = &menu_structure.entries_type[i];
    }
    // bottom
    for(int i = 0; i < 2; ++i)
    {
        auto& button = buttons[i + 2];
        button.x = 64 + (112 * i);
        button.y = 110;
        button.w = 80;
        button.h = 80;
        button.selectable = true;
        button.associated_menu_entry = &menu_structure.entries_type[i + 2];
    }
    // back
    {
        auto& button = buttons[4];
        button.x = 20;
        button.y = 240 - 4 - 40;
        button.w = 120;
        button.h = 40;
        button.selectable = true;
        button.associated_menu_entry = &menu_structure.entries_type[4];
    }
    // ok
    {
        auto& button = buttons[5];
        button.x = 320 - 20 - 120;
        button.y = 240 - 4 - 40;
        button.w = 120;
        button.h = 40;
        button.selectable = true;
        button.associated_menu_entry = &menu_structure.entries_type[5];
    }
    }
    }

    {
    auto& buttons = tab_final_buttons.buttons;
    // top
    for(int i = 0; i < 2; ++i)
    {
        auto& button = buttons[i];
        button.x = 50 + (120 * i);
        button.y = 20;
        button.w = 100;
        button.h = 80;
        button.selectable = false;
        button.associated_menu_entry = nullptr;
    }
    // bottom
    for(int i = 0; i < 2; ++i)
    {
        auto& button = buttons[i + 2];
        button.x = 50 + (120 * i);
        button.y = 110;
        button.w = 100;
        button.h = 80;
        button.selectable = false;
        button.associated_menu_entry = nullptr;
    }
    // back
    {
        auto& button = buttons[4];
        button.x = 20;
        button.y = 240 - 4 - 40;
        button.w = 120;
        button.h = 40;
        button.selectable = true;
        button.associated_menu_entry = &menu_structure.entries_final[0];
    }
    // ok
    {
        auto& button = buttons[5];
        button.x = 320 - 20 - 120;
        button.y = 240 - 4 - 40;
        button.w = 120;
        button.h = 40;
        button.selectable = true;
        button.associated_menu_entry = &menu_structure.entries_final[1];
    }
    }
}

void scenes::room_setup_scene::now_at(const int entry_index, const int tab_index)
{
    this->entry_index = entry_index;
    this->tab_index = tab_index;
}

void scenes::room_setup_scene::set_next_scene_to(scene_ptr ptr)
{
    next = std::move(ptr);
}

void scenes::room_setup_scene::change_width(int by)
{
    numbers.dims.width = std::clamp(numbers.dims.width + by, game::board::MIN_WIDTH, game::board::MAX_WIDTH);
}
void scenes::room_setup_scene::change_height(int by)
{
    numbers.dims.height = std::clamp(numbers.dims.height + by, game::board::MIN_HEIGHT, game::board::MAX_HEIGHT);
}
void scenes::room_setup_scene::change_mine_percent(int by)
{
    numbers.mines = std::clamp(numbers.mines + by, game::board::MIN_PERCENT, game::board::MAX_PERCENT);
}

/// TODO: ask with swkbd
void scenes::room_setup_scene::ask_width()
{
    numbers.dims.width = 30;
}
void scenes::room_setup_scene::ask_height()
{
    numbers.dims.height = 30;
}
void scenes::room_setup_scene::ask_mine_percent()
{

}

void scenes::room_setup_scene::set_board_type(game::board::mode mode)
{
    board_mode = mode;
}
void scenes::room_setup_scene::prepare_starting()
{
    if(!game_config->data.game_board)
    {
        game_config->data.sheet_3d = ctr::gfx::texture::load_from_file("romfs:/gfx/sheet_3D.t3x");
        game_config->data.sheet_cursors = ctr::gfx::texture::load_from_file("romfs:/gfx/sheet_cursors.t3x");
        /*
        {
        const auto t = game_config->data.sheet_3d->get_tex();
        C3D_TexSetFilter(t, GPU_NEAREST, GPU_NEAREST);
        C3D_TexSetFilterMipmap(t, GPU_NEAREST);
        t->border = 0;
        }
        */

        constexpr std::size_t board_vbo_sz = (game::board::MAX_HEIGHT * game::board::MAX_WIDTH + (2 * (game::board::MAX_WIDTH + game::board::MAX_HEIGHT))) * 6;
        game_config->data.board_vbo.reset(linearAlloc(board_vbo_sz * sizeof(game::board::buffer_point)));
        constexpr std::size_t board_margin_vbo_sz = (0
             + (game::board::HORI_MARGIN * game::board::MAX_WIDTH) *2
             + (game::board::VERT_MARGIN * game::board::MAX_HEIGHT) * 2
             + (game::board::VERT_MARGIN * game::board::HORI_MARGIN) * 4
        ) * 6;
        game_config->data.board_margin_vbo.reset(linearAlloc(board_margin_vbo_sz * sizeof(game::board::buffer_point)));
        game_config->data.board_cursors_vbo.reset(linearAlloc((game::room::MAX_PLAYERS) * 6 * sizeof(game::board::buffer_point)));

        game_config->data.game_board.emplace(
            static_cast<game::board::buffer_point*>(game_config->data.board_vbo.get()),
            static_cast<game::board::buffer_point*>(game_config->data.board_margin_vbo.get())
        );

        {
        constexpr std::pair<const char*, shaderInstance_s* shaderProgram_s::*> uniforms_arr[] = {
            {"projection", &shaderProgram_s::vertexShader},
            {"view", &shaderProgram_s::vertexShader},
            {"model", &shaderProgram_s::vertexShader},
            {"arraycolor", &shaderProgram_s::vertexShader},
        };
        game_config->data.board_shader_drop.emplace((u32*)curved_plane_drop_shbin, curved_plane_drop_shbin_size, uniforms_arr);
        }

        {
        constexpr std::pair<const char*, shaderInstance_s* shaderProgram_s::*> uniforms_arr[] = {
            {"projection", &shaderProgram_s::vertexShader},
            {"modelView", &shaderProgram_s::vertexShader},
            {"arraycolor", &shaderProgram_s::vertexShader},
        };
        game_config->data.board_shader_basic.emplace((u32*)curved_plane_basic_shbin, curved_plane_basic_shbin_size, uniforms_arr);
        }

        AttrInfo_Init(&game_config->data.board_attr);
        AttrInfo_AddLoader(&game_config->data.board_attr, 0, GPU_FLOAT, 3); // v0=position
        AttrInfo_AddLoader(&game_config->data.board_attr, 1, GPU_FLOAT, 2); // v1=uv

        BufInfo_Init(&game_config->data.board_vbo_buf);
        BufInfo_Add(&game_config->data.board_vbo_buf, game_config->data.board_vbo.get(), sizeof(game::board::buffer_point), 2, 0x10);
        BufInfo_Init(&game_config->data.board_margin_vbo_buf);
        BufInfo_Add(&game_config->data.board_margin_vbo_buf, game_config->data.board_margin_vbo.get(), sizeof(game::board::buffer_point), 2, 0x10);
        BufInfo_Init(&game_config->data.board_cursors_vbo_buf);
        BufInfo_Add(&game_config->data.board_cursors_vbo_buf, game_config->data.board_cursors_vbo.get(), sizeof(game::board::buffer_point), 2, 0x10);
    }

    game_config->data.players.fill({});
    game_config->data.teams.fill({});

    auto& self = game_config->data.players[0];
    self.team_id = 1;
    self.color_index = 7;
    self.x = numbers.dims.width / 2.0f;
    self.y = numbers.dims.height / 2.0f;

    game_config->data.teams[self.team_id - 1].current_state = game::team::state::playing;

    auto nums = numbers;
    nums.mines = (nums.mines * nums.dims.height * nums.dims.width) / 100;
    set_next_scene_to(::scenes::transition_scene::create(get_ptr(), ::scenes::playing_scene::create(nums, board_mode, 0)));
}

game::scenes::next_scene scenes::room_setup_scene::update(const ctr::hid& input, ctr::audio& audio, const double dt)
{
    bool should_handle_keys = true;
    switch(tab_index)
    {
    case 0:
        should_handle_keys = !tab_numbers_buttons.react(*this, input, audio, menu);
        break;
    case 1:
        should_handle_keys = !tab_type_buttons.react(*this, input, audio, menu);
        break;
    case 2:
        should_handle_keys = !tab_final_buttons.react(*this, input, audio, menu);
        break;
    }

    if(should_handle_keys)
        menu.react(*this, input, audio, game_config->conf.keymap_menu);
    
    if(next)
    {
        auto out = std::move(*next);
        next.reset();
        return out;
    }
    else
        return std::nullopt;
}

void scenes::room_setup_scene::draw(ctr::gfx& gfx)
{
    gfx.render_2d();
    gfx.get_screen(GFX_TOP, GFX_LEFT)->focus();
    C2D_DrawImageAt(game_config->data.top_bg_full, 0, 0, 0);

    constexpr u32 shadow_color = C2D_Color32f(0.375f, 0.375f, 0.375f, 1.0f);
    constexpr u32 text_color = C2D_Color32f(0, 0, 0, 1);

    const auto top_txt = &top_text[tab_index];
    float topTextH = 0.0f;
    C2D_TextGetDimensions(top_txt, 1.0f, 1.0f, nullptr, &topTextH);
    topTextH *= 0.5f;

    C2D_DrawText(top_txt, C2D_AlignCenter | C2D_WithColor, 200.0f + 2, (240 - topTextH)/2.0f + 2, 0.25f, 1.0f, 1.0f, shadow_color);
    C2D_DrawText(top_txt, C2D_AlignCenter | C2D_WithColor, 200.0f, (240 - topTextH)/2.0f, 0.5f, 1.0f, 1.0f, text_color);

    gfx.get_screen(GFX_BOTTOM)->focus();
    C2D_DrawImageAt(game_config->data.bottom_bg_full, 0, 0, 0);

    auto& gen = game_config->data.dynamic_small_text_gen->get_active_buf();
    gen.clear();

    switch(tab_index)
    {
    case 0:
        {
        for(const auto& button : tab_numbers_buttons.buttons)
        {
            button.draw((button.associated_menu_entry && button.associated_menu_entry->entry_index == entry_index) ? selected_button_parts : button_parts);
        }
        C2D_Text txts[3] = {};
        std::string s = std::to_string(numbers.dims.width);
        txts[0] = gen.parse(s.c_str());
        s = std::to_string(numbers.dims.height);
        txts[1] = gen.parse(s.c_str());
        s = std::to_string(numbers.mines);
        txts[2] = gen.parse(s.c_str());

        C2D_Image imgs[3] = {
            width_img,
            height_img,
            mines_img,
        };
        for(int y = 0; y < 3; ++y)
        {
            for(int x = 0; x < 4; ++x)
            {
                const auto& but = tab_numbers_buttons.buttons[y * 4 + x];
                switch(x)
                {
                case 0:
                    {
                    const auto& im = imgs[y];
                    C2D_DrawImageAt(im, but.x + (but.w - im.subtex->width)/2, but.y + (but.h - im.subtex->height)/2, but.depth + 0.0625f);
                    }
                    break;
                case 1:
                    {
                    const auto& im = minus_img;
                    C2D_DrawImageAt(im, but.x + (but.w - im.subtex->width)/2, but.y + (but.h - im.subtex->height)/2, but.depth + 0.0625f);
                    }
                    break;
                case 2:
                    {
                        float textH = 0.0f;
                        const auto txt = &txts[y];
                        C2D_TextGetDimensions(txt, 1.0f, 1.0f, nullptr, &textH);
                        textH *= 0.5f;
                        C2D_DrawText(txt, C2D_AlignCenter | C2D_WithColor, but.x + but.w/2 + 2, but.y + (but.h - textH)/2 + 2, but.depth + 0.0625f, 1.0f, 1.0f, shadow_color);
                        C2D_DrawText(txt, C2D_AlignCenter | C2D_WithColor, but.x + but.w/2, but.y + (but.h - textH)/2, but.depth + 0.125f, 1.0f, 1.0f, text_color);
                    }
                    break;
                case 3:
                    {
                    const auto& im = plus_img;
                    C2D_DrawImageAt(im, but.x + (but.w - im.subtex->width)/2, but.y + (but.h - im.subtex->height)/2, but.depth + 0.0625f);
                    }
                    break;
                }
            }
        }
        {
        const auto& but = tab_numbers_buttons.buttons[12];
        float textH = 0.0f;
        const auto txt = &back_text;
        C2D_TextGetDimensions(txt, 1.0f, 1.0f, nullptr, &textH);
        textH *= 0.5f;
        C2D_DrawText(txt, C2D_AlignCenter | C2D_WithColor, but.x + but.w/2 + 2, but.y + (but.h - textH)/2 + 2, but.depth + 0.0625f, 1.0f, 1.0f, shadow_color);
        C2D_DrawText(txt, C2D_AlignCenter | C2D_WithColor, but.x + but.w/2, but.y + (but.h - textH)/2, but.depth + 0.125f, 1.0f, 1.0f, text_color);
        }
        {
        const auto& but = tab_numbers_buttons.buttons[13];
        float textH = 0.0f;
        const auto txt = &ok_text;
        C2D_TextGetDimensions(txt, 1.0f, 1.0f, nullptr, &textH);
        textH *= 0.5f;
        C2D_DrawText(txt, C2D_AlignCenter | C2D_WithColor, but.x + but.w/2 + 2, but.y + (but.h - textH)/2 + 2, but.depth + 0.0625f, 1.0f, 1.0f, shadow_color);
        C2D_DrawText(txt, C2D_AlignCenter | C2D_WithColor, but.x + but.w/2, but.y + (but.h - textH)/2, but.depth + 0.125f, 1.0f, 1.0f, text_color);
        }
        }
        break;
    case 1:
        {
        for(const auto& button : tab_type_buttons.buttons)
        {
            button.draw((button.associated_menu_entry && button.associated_menu_entry->entry_index == entry_index) ? selected_button_parts : button_parts);
        }
        constexpr game::board::mode img_modes[2][2] = {
            {game::board::mode::regular, game::board::mode::loop_horizontal},
            {game::board::mode::loop_vertical, game::board::mode::loop_both},
        };
        C2D_ImageTint tint, selected_tint;
        C2D_PlainImageTint(&tint, C2D_Color32(0,0,0,255), 1.0f);
        C2D_PlainImageTint(&selected_tint, C2D_Color32(0,128,0,255), 1.0f);
        for(int y = 0; y < 2; ++y)
        {
            for(int x = 0; x < 2; ++x)
            {
                const auto& but = tab_type_buttons.buttons[y * 2 + x];
                const auto& im = board_imgs[y][x];
                C2D_DrawImageAt(im, but.x + (but.w - im.subtex->width)/2, but.y + (but.h - im.subtex->height)/2, but.depth + 0.0625f, img_modes[y][x] == board_mode ? &selected_tint : &tint);
            }
        }
        {
        const auto& but = tab_type_buttons.buttons[4];
        float textH = 0.0f;
        const auto txt = &back_text;
        C2D_TextGetDimensions(txt, 1.0f, 1.0f, nullptr, &textH);
        textH *= 0.5f;
        C2D_DrawText(txt, C2D_AlignCenter | C2D_WithColor, but.x + but.w/2 + 2, but.y + (but.h - textH)/2 + 2, but.depth + 0.0625f, 1.0f, 1.0f, shadow_color);
        C2D_DrawText(txt, C2D_AlignCenter | C2D_WithColor, but.x + but.w/2, but.y + (but.h - textH)/2, but.depth + 0.125f, 1.0f, 1.0f, text_color);
        }
        {
        const auto& but = tab_type_buttons.buttons[5];
        float textH = 0.0f;
        const auto txt = &ok_text;
        C2D_TextGetDimensions(txt, 1.0f, 1.0f, nullptr, &textH);
        textH *= 0.5f;
        C2D_DrawText(txt, C2D_AlignCenter | C2D_WithColor, but.x + but.w/2 + 2, but.y + (but.h - textH)/2 + 2, but.depth + 0.0625f, 1.0f, 1.0f, shadow_color);
        C2D_DrawText(txt, C2D_AlignCenter | C2D_WithColor, but.x + but.w/2, but.y + (but.h - textH)/2, but.depth + 0.125f, 1.0f, 1.0f, text_color);
        }
        }
        break;
    case 2:
        {
        for(const auto& button : tab_final_buttons.buttons)
        {
            button.draw((button.associated_menu_entry && button.associated_menu_entry->entry_index == entry_index) ? selected_button_parts : button_parts);
        }
        C2D_ImageTint tint;
        C2D_PlainImageTint(&tint, C2D_Color32(0,0,0,255), 1.0f);
        std::string s = std::to_string(numbers.dims.width);
        C2D_Text width_text = gen.parse(s.c_str());
        s = std::to_string(numbers.dims.height);
        C2D_Text height_text = gen.parse(s.c_str());
        s = std::to_string(numbers.mines);
        C2D_Text mines_text = gen.parse(s.c_str());
        C2D_Text* txts[4] = {
            &width_text,
            &height_text,
            &mines_text,
            nullptr,
        };
        C2D_Image imgs[4] = {
            width_img,
            height_img,
            mines_img,
        };
        switch(board_mode)
        {
        case game::board::mode::regular:
            imgs[3] = board_imgs[0][0];
            break;
        case game::board::mode::loop_horizontal:
            imgs[3] = board_imgs[0][1];
            break;
        case game::board::mode::loop_vertical:
            imgs[3] = board_imgs[1][0];
            break;
        case game::board::mode::loop_both:
            imgs[3] = board_imgs[1][1];
            break;
        }

        for(int b = 0; b < 4; ++b)
        {
            const auto& but = tab_type_buttons.buttons[b];
            const auto& im = imgs[b];
            const auto txt = txts[b];
            float textW = 0.0f, textH = 0.0f;
            if(txt)
                C2D_TextGetDimensions(txt, 1.0f, 1.0f, &textW, &textH);
            textW *= 0.75f;
            C2D_DrawImageAt(im, but.x + (but.w - im.subtex->width)/2 - textW, but.y + (but.h - im.subtex->height)/2, but.depth + 0.0625f, &tint);
            if(txt)
            {
                textH *= 0.5f;
                C2D_DrawText(txt, C2D_WithColor, but.x + (but.w - im.subtex->width)/2 - textW + im.subtex->width + 2 + 2, but.y + (but.h - textH)/2 + 2, but.depth + 0.0625f, 1.0f, 1.0f, shadow_color);
                C2D_DrawText(txt, C2D_WithColor, but.x + (but.w - im.subtex->width)/2 - textW + im.subtex->width + 2, but.y + (but.h - textH)/2, but.depth + 0.125f, 1.0f, 1.0f, text_color);
            }
        }
        {
        const auto& but = tab_type_buttons.buttons[4];
        float textH = 0.0f;
        const auto txt = &back_text;
        C2D_TextGetDimensions(txt, 1.0f, 1.0f, nullptr, &textH);
        textH *= 0.5f;
        C2D_DrawText(txt, C2D_AlignCenter | C2D_WithColor, but.x + but.w/2 + 2, but.y + (but.h - textH)/2 + 2, but.depth + 0.0625f, 1.0f, 1.0f, shadow_color);
        C2D_DrawText(txt, C2D_AlignCenter | C2D_WithColor, but.x + but.w/2, but.y + (but.h - textH)/2, but.depth + 0.125f, 1.0f, 1.0f, text_color);
        }
        {
        const auto& but = tab_type_buttons.buttons[5];
        float textH = 0.0f;
        const auto txt = &ok_text;
        C2D_TextGetDimensions(txt, 1.0f, 1.0f, nullptr, &textH);
        textH *= 0.5f;
        C2D_DrawText(txt, C2D_AlignCenter | C2D_WithColor, but.x + but.w/2 + 2, but.y + (but.h - textH)/2 + 2, but.depth + 0.0625f, 1.0f, 1.0f, shadow_color);
        C2D_DrawText(txt, C2D_AlignCenter | C2D_WithColor, but.x + but.w/2, but.y + (but.h - textH)/2, but.depth + 0.125f, 1.0f, 1.0f, text_color);
        }
        }
        break;
    }
}
