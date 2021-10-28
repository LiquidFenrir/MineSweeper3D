#ifndef HID3DS_INC
#define HID3DS_INC

#include <3ds.h>
#include <algorithm>
#include <numeric>
#include <cmath>

namespace ctr {

struct hid {
    hid()
    {
        APT_CheckNew3DS(&have_cstick);
    }
    hid& operator=(const hid&) = delete;
    hid& operator=(hid&&) = delete;
    hid(const hid&) = delete;
    hid(hid&&) = delete;

    void update()
    {
        hidScanInput();
        kDown = hidKeysDown();
        kDownRepeat = hidKeysDownRepeat();
        kHeld= hidKeysHeld();
        kUp = hidKeysUp();

        hidCircleRead(&cpad);
        irrstCstickRead(&cstick);
        hidTouchRead(&touch);
    }

    struct input {
        u32 any(const u32 k = -1) const
        {
            return (keys & k);
        }
        bool all(const u32 k) const
        {
            return (keys & k) == k;
        }
        bool none(const u32 k) const
        {
            return (keys & k) == 0;
        }

        struct key {
            enum class mode {
                any,
                all,
                none,
            };

            key(const u32 k_)
                : key(k_, mode::any)
            {

            }

            key(const u32 k_, mode m_)
                : k(k_)
                , m(m_)
            {

            }

            key operator&(const key other) const
            {
                return {k | other.k, mode::all};
            }
            key operator|(const key other) const
            {
                return {k | other.k, mode::any};
            }
            key operator~() const
            {
                return {k, mode::none};
            }

        private:
            friend input;

            u32 k;
            mode m;
        };

        bool check(const key k) const
        {
            switch(k.m)
            {
            case key::mode::any:
                return any(k.k);
            case key::mode::all:
                return all(k.k);
            case key::mode::none:
                return none(k.k);
            default:
                return false;
            }
        }

    private:
        friend hid;

        input(const u32 k)
            : keys(k)
        {

        }

        u32 keys;
    };

    static input::key begin_combo()
    {
        return {0};
    }
    input pressed() const
    {
        return kDown;
    }
    input held() const
    {
        return kHeld;
    }
    input released() const
    {
        return kUp;
    }
    input repeated() const
    {
        return kDownRepeat;
    }

    bool touch_active() const
    {
        return kDownRepeat & KEY_TOUCH;
    }

    struct touch_point {
        u16 x, y;
    };
    bool touching(const touch_point pt) const
    {
        return kHeld & KEY_TOUCH && touch.px == pt.x && touch.py == pt.y;
    }

    // rectangle from [corner] to [opposite]
    bool touching_abs(const touch_point corner, const touch_point opposite) const
    {
        const bool touching_x = std::min(corner.x, opposite.x) <= touch.px && touch.px < std::max(corner.x, opposite.x);
        const bool touching_y = std::min(corner.y, opposite.y) <= touch.py && touch.py < std::max(corner.y, opposite.y);
        return kHeld & KEY_TOUCH && touching_x && touching_y;
    }

    // rectangle from [corner] to [corner + delta]
    bool touching_offset(const touch_point corner, const touch_point delta) const
    {
        const bool touching_x = corner.x <= touch.px && touch.px < (corner.x + delta.x);
        const bool touching_y = corner.y <= touch.py && touch.py < (corner.y + delta.y);
        return kHeld & KEY_TOUCH && touching_x && touching_y;
    }

    // rectangle from [corner - radius] to [corner + radius]
    bool touching_center(const touch_point corner, const touch_point radius) const
    {
        const bool touching_x = (corner.x - radius.x) <= touch.px && touch.px < (corner.x + radius.x);
        const bool touching_y = (corner.y - radius.y)  <= touch.py && touch.py < (corner.y + radius.y);
        return kHeld & KEY_TOUCH && touching_x && touching_y;
    }

    struct stick {
        int x() const
        {
            return pos.dx;
        }
        int y() const
        {
            return pos.dy;
        }

        int distance() const
        {
            return x() * x() + y() * y();
        }
        float angle() const
        {
            return std::atan2(x()/300.0f, y()/300.0f);
        }

    private:
        friend hid;

        stick(const circlePosition p)
            : pos(p)
        {

        }

        circlePosition pos;
    };

    stick left_stick() const
    {
        return cpad;
    }
    stick right_stick() const
    {
        return have_cstick ? cstick : circlePosition{0, 0};
    }

private:
    u32 kDown{}, kHeld{}, kUp{}, kDownRepeat{};
    circlePosition cpad{}, cstick{};
    touchPosition touch{};
    bool have_cstick{};
};

}

#endif
