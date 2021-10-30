#ifndef GFXAUDIO_INC
#define GFXAUDIO_INC

#include <3ds.h>
#include <optional>
#include <array>
#include <atomic>
#include <tremor/ivorbisfile.h>
#include <tremor/ivorbiscodec.h>
#include "ctrthread.h"
#include "useful_utilities.h"

namespace ctr {

struct audio {
    audio();
    ~audio();

    audio& operator=(const audio&) = delete;
    audio& operator=(audio&&) = delete;
    audio(const audio&) = delete;
    audio(audio&&) = delete;

    constexpr static inline std::string_view BGM_ROOT = "romfs:/bgm/";
    constexpr static inline std::string_view SFX_ROOT = "romfs:/sfx/";
    constexpr static inline int MAX_SFX = 9;

    void set_bgm_enable(bool enable);
    void set_sfx_enable(bool enable);

    // empty string will stop playback
    void play_bgm(std::string_view name = "", bool looping = false);
    // passing a path will look for the first not in use channel and play there
    // passing empty string will stop all sfx currently being played
    // passing a path and a second argument (>= 1) will use that channel if not in use, otherwise will do nothing
    // passing empty string and a second argument (>= 1) will only stop that channel
    void play_sfx(std::string_view name = "", unsigned force_channnel = 0);

private:
    void playing_thread_func();

    struct linear_deleter {
        void operator()(void* ptr)
        {
            linearFree(ptr);
        }
    };
    using linear_ptr = util::ptr_for<void*, linear_deleter>;
    
    struct ogg {
        ogg(const char* path)
        {
            util::file_ptr fh(fopen(path, "rb"));
            const int e = ov_open(fh.get(), &vf, nullptr, 0);
            if (e < 0) 
            {
                return;
            }
            fh.release();
            valid = true;
        }
        ~ogg()
        {
            if(valid)
                ov_clear(&vf);
        }

        bool valid{false};
        OggVorbis_File vf{};
    };

    struct bgm {
        constexpr static inline int NUM_SAMPLES = 4096;
        std::string name;
        linear_ptr samples[2];
        ndspWaveBuf wave_buf[2]{};
        std::optional<ogg> source;
        ctr::mutex mutex;
        ogg_int64_t loop_start{0};
        ogg_int64_t offset{0};
        int active_buf{-1};
        bool play{false};
        bool loop{false};
    };
    bgm music;

    struct sfx {
        struct data {
            std::string name;
            linear_ptr samples;
            ogg_int64_t samples_size{};
            ogg_int64_t samples_capacity{};
            long rate{};
        };
        data info;
        ndspWaveBuf wave_buf{};
    };
    std::array<sfx, MAX_SFX> sounds;

    bool can_play{false};
    bool bgm_enabled{true}, sfx_enabled{true};
    std::atomic_flag keep_running{ATOMIC_FLAG_INIT};
    ctr::thread playing_thread;
};

}

#endif
