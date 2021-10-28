#ifndef GAMESCENE_INC
#define GAMESCENE_INC

#include "ctrhid.h"
#include "ctrgfx.h"
#include "gameconfig.h"
#include "debugging.h"
#include <span>
#include <memory>
#include <optional>
#include <concepts>

namespace game {

struct scenes : public std::enable_shared_from_this<scenes> {
    virtual ~scenes() = default;

    void enter_scene() const
    {
        debugging::log("ENTERING scene @ {:p}: {:s}\n", static_cast<const void*>(this), this->GET_CLASS_NAME());
    }
    void exit_scene() const
    {
        debugging::log("EXITING scene @ {:p}: {:s}\n", static_cast<const void*>(this), this->GET_CLASS_NAME());
    }

    using scene_ptr = std::shared_ptr<scenes>;
    static scene_ptr get_default_scene();

    using next_scene = std::optional<scene_ptr>;
    virtual next_scene update(const ctr::hid& input, const double dt) = 0;
    virtual void tick([[maybe_unused]] const double dt) { } // override if the scene needs to animate during a transition, for example

    virtual void draw(ctr::gfx& gfx) = 0;

    static inline config* game_config{nullptr};

    ctr::gfx::color clear_color_top{0xc0, 0xc0, 0xc0, 0xff}, clear_color_bottom{0xc0, 0xc0, 0xc0, 0xff};

    scene_ptr get_ptr()
    {
        return this->shared_from_this();
    }

protected:
    virtual const char* GET_CLASS_NAME() const { return "scene base\n"; }
    scenes() = default;
};

template<class T>
struct scene : public scenes {
    using scenes::scene_ptr;
    using scenes::next_scene;

    template<typename... Args>
    [[nodiscard]] static std::shared_ptr<T> create(Args... args) {
        // Not using std::make_shared<T> because the c'tor is private.
        return std::shared_ptr<T>(new T(args...));
    }
};

/*
template<class T>
concept IsScene = std::derived_from<T, scene>;
template<typename T>
concept MenuNotifiable =
requires (T a) {
    { a.now_at(std::declval<int>()) };
};
template<typename T>
concept MenuOkayScene = IsScene<T> && MenuNotifiable<T>;

template<MenuOkayScene T>
*/
template<class T>
struct scene_menu {
    struct menu_tab {
        menu_tab* prev_tab{nullptr};
        menu_tab* next_tab{nullptr};
        int tab_index{-1};

        struct menu_entry {
            menu_tab* parent_tab{nullptr};;
            menu_entry* up_entry{nullptr};
            menu_entry* down_entry{nullptr};
            menu_entry* left_entry{nullptr};
            menu_entry* right_entry{nullptr};

            using func_t = const menu_entry* (*)(T&, const menu_entry&);
            func_t validate_func{nullptr};
            func_t cancel_func{nullptr};

            int entry_index{-1};

            static const menu_entry* change_entry_up(T&, const menu_entry& entry)
            {
                return entry.up_entry;
            }
            static const menu_entry* change_entry_down(T&, const menu_entry& entry)
            {
                return entry.down_entry;
            }
            static const menu_entry* change_entry_left(T&, const menu_entry& entry)
            {
                return entry.left_entry;
            }
            static const menu_entry* change_entry_right(T&, const menu_entry& entry)
            {
                return entry.right_entry;
            }
            static const menu_entry* change_tab_prev(T&, const menu_entry& entry)
            {
                return entry.parent_tab->prev_tab ? &entry.parent_tab->prev_tab->entries.front() : nullptr;
            }
            static const menu_entry* change_tab_next(T&, const menu_entry& entry)
            {
                return entry.parent_tab->next_tab ? &entry.parent_tab->next_tab->entries.front() : nullptr;
            }
        };

        std::span<const menu_entry> entries;
    };

    void jump_to(T& source, const menu_tab::menu_entry* const new_entry)
    {
        source.now_at(new_entry->entry_index, new_entry->parent_tab ? new_entry->parent_tab->tab_index : -1);
        current_entry = new_entry;
    }
    const menu_tab::menu_entry* get_current_entry() const
    {
        return current_entry;
    }

    bool react(T& source, const ctr::hid& input, const decltype(game::config::keymap_menu)& key_mapping)
    {
        using key_usage = decltype(game::config::keymap_menu)::key_usage;
        const auto& entry = *current_entry;
        using k_t = key_usage;
        using v_t = menu_tab::menu_entry::func_t;
        const std::pair<k_t, v_t> kv[] = {
            {key_usage::validate, entry.validate_func},
            {key_usage::cancel, entry.cancel_func},

            {key_usage::cursor_up, &menu_tab::menu_entry::change_entry_up},
            {key_usage::cursor_down, &menu_tab::menu_entry::change_entry_down},
            {key_usage::cursor_left, &menu_tab::menu_entry::change_entry_left},
            {key_usage::cursor_right, &menu_tab::menu_entry::change_entry_right},

            {key_usage::tab_prev, &menu_tab::menu_entry::change_tab_prev},
            {key_usage::tab_next, &menu_tab::menu_entry::change_tab_next},
        };

        const auto ks = input.repeated();
        for(const auto& [k, v] : kv)
        {
            if(ks.check(key_mapping.get_key_for(k)) && v)
            {
                if(auto new_entry = v(source, entry))
                {
                    jump_to(source, new_entry);
                }
                return true;
            }
        }
        return false;
    }

    scene_menu(const menu_tab::menu_entry& entry)
        : current_entry(&entry)
    {

    }

private:
    const menu_tab::menu_entry* current_entry;
};

}

#define START_SCENE(nam) struct nam : public game::scene<nam> { friend scene; public: static inline constexpr const char CLASS_NAME[] = #nam; \
    const char* GET_CLASS_NAME() const override final { return this->CLASS_NAME; }

#define END_SCENE };

#endif
