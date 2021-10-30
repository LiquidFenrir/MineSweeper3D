#include "ctraudio.h"
#include "ctrthread.h"
#include "debugging.h"
#include <exception>
#include <charconv>

namespace ctr {

audio::audio()
{
    can_play = R_SUCCEEDED(ndspInit());
    if(!can_play) return;

    ndspSetOutputMode(NDSP_OUTPUT_STEREO);

    ndspChnReset(0);
    ndspChnSetFormat(0, NDSP_FORMAT_STEREO_PCM16);

    for(int i = 1; i <= MAX_SFX; ++i)
    {
        ndspChnReset(i);
        ndspChnSetFormat(i, NDSP_FORMAT_MONO_PCM16);
    }

    keep_running.test_and_set();
    ctr::thread::meta meta{
        32 * 1024,
        0x38,
        0
    };
    playing_thread = ctr::thread(meta, &audio::playing_thread_func, this);
}
audio::~audio()
{
    if(can_play)
    {
        keep_running.clear();
        playing_thread.join();
        if(music.active_buf != -1)
        {
            ndspChnReset(0);
        }
        for(int i = 1; i <= MAX_SFX; ++i)
        {
            auto& s = sounds[i - 1];
            if(!(s.wave_buf.status == NDSP_WBUF_DONE || s.wave_buf.status == NDSP_WBUF_FREE))
            {
                ndspChnReset(i);
            }
        }
        ndspExit();
    }
}

void audio::set_bgm_enable(bool enable)
{
    bgm_enabled = enable;
}
void audio::set_sfx_enable(bool enable)
{
    sfx_enabled = enable;
}

void audio::play_bgm(std::string_view name, bool looping)
{
    if(!can_play) return;
    if(!bgm_enabled) return;

    if(!name.empty())
    {
        auto new_name = fmt::format("{}{}.ogg", BGM_ROOT, name);
        if(music.name != new_name)
        {
            std::lock_guard guard(music.mutex);
            music.name = std::move(new_name);

            music.source.emplace(music.name.c_str());
            if(!music.source->valid)
            {
                music.name.clear();
                music.source.reset();
                music.play = false;
                return;
            }

            vorbis_info *vi = ov_info(&music.source->vf, -1);
            ndspChnWaveBufClear(0);
            ndspChnSetRate(0, vi->rate);// Set sample rate to what's read from the ogg file

            if(music.active_buf == -1)
            {
                music.wave_buf[0].status = (music.wave_buf[1].status = NDSP_WBUF_DONE);
                music.samples[0].reset(linearAlloc(bgm::NUM_SAMPLES * sizeof(s16)));
                music.samples[1].reset(linearAlloc(bgm::NUM_SAMPLES * sizeof(s16)));
                music.wave_buf[0].data_vaddr = music.samples[0].get();
                music.wave_buf[1].data_vaddr = music.samples[1].get();
                music.active_buf = 0;
            }

            music.loop_start = 0;
            if(looping)
            {
                auto vc = ov_comment(&music.source->vf, -1);
                for(int i = 0; i < vc->comments; ++i)
                {
                    std::string_view comment(vc->user_comments[i], vc->comment_lengths[i]);
                    constexpr std::string_view looking_for("LOOPSTART=");
                    if(comment.starts_with(looking_for))
                    {
                        comment.substr(looking_for.size());
                        if(std::from_chars(comment.data(), comment.data() + comment.size(), music.loop_start, 10).ec != std::errc())
                        {
                            music.loop_start = 0;
                        }
                        break;
                    }
                }
            }

            music.offset = 0;
            music.loop = looping;
            music.play = true;
        }
        else
        {
            std::lock_guard guard(music.mutex);
            music.loop = looping;
            music.play = true;
            ov_pcm_seek(&music.source->vf, music.loop_start);
        }
    }
    else
    {
        std::lock_guard guard(music.mutex);
        music.play = false;
        ndspChnWaveBufClear(0);
    }
}
void audio::play_sfx(std::string_view name, unsigned force_channnel)
{
    if(!can_play) return;
    if(!sfx_enabled) return;

    if(!name.empty())
    {
        auto new_name = fmt::format("{}{}.ogg", SFX_ROOT, name);

        const auto do_channel = [&](int idx) -> int {
            auto& s = sounds[idx - 1];
            if(s.wave_buf.status == NDSP_WBUF_DONE || s.wave_buf.status == NDSP_WBUF_FREE)
            {
                if(s.info.name == new_name) // reuse old data, since we want to play the same
                {
                    ndspChnWaveBufAdd(idx, &s.wave_buf);
                    return 0;
                }
                else
                {
                    ogg source(new_name.c_str());
                    if(!source.valid)
                        return -2;

                    s.info.name = std::move(new_name);
                    vorbis_info *vi = ov_info(&source.vf, -1);
                    const auto new_rate = vi->rate;
                    const auto new_samples = ov_pcm_total(&source.vf, -1);

                    debugging::log("new_name, new_rate, new_samples: {}, {}, {}\n", name, new_rate, new_samples);
                    ndspChnWaveBufClear(idx);
                    ndspChnSetRate(idx, new_rate);// Set sample rate to what's read from the ogg file
                    s.info.samples_size = new_samples;
                    s.wave_buf.nsamples = new_samples;
                    if(new_samples > s.info.samples_capacity)
                    {
                        s.info.samples_capacity = new_samples;
                        s.info.samples.reset(linearAlloc(new_samples * sizeof(s16)));
                        s.wave_buf.data_vaddr = s.info.samples.get();
                    }

                    void* data_buf = s.info.samples.get();
                    char* data_buf_output = static_cast<char*>(data_buf);

                    long rd = 0;
                    long remaining = new_samples * sizeof(s16);
                    long offset = 0;
                    int bitstream;
                    while((rd = ov_read(&source.vf, data_buf_output + offset, remaining, &bitstream)) != 0)
                    {
                        // errors can't happen..? :)
                        remaining -= rd;
                        offset += rd;
                    }
                    DSP_FlushDataCache(data_buf, new_samples * sizeof(s16));

                    ndspChnWaveBufAdd(idx, &s.wave_buf);
                    return 0;
                }
            }
            return -1;
        };

        if(force_channnel)
        {
            do_channel(force_channnel);
        }
        else
        {
            for(int i = 1; i <= MAX_SFX; ++i)
            {
                if(const int res = do_channel(i); res == -2 || res == 0)
                    break;
            }
        }
    }
    else
    {
        if(force_channnel)
        {
            ndspChnWaveBufClear(force_channnel);
        }
        else
        {
            for(int i = 1; i <= MAX_SFX; ++i)
            {
                ndspChnWaveBufClear(i);
            }
        }
    }
}

void audio::playing_thread_func()
{
    while(keep_running.test_and_set())
    {
        if(music.play && music.wave_buf[music.active_buf].status == NDSP_WBUF_DONE) // only run if the current selected buffer has already finished playing
        {
            std::lock_guard lock(music.mutex);
            int bitstream;

            auto& buf = music.wave_buf[music.active_buf];
            long rd = ov_read(&music.source->vf, (char*)buf.data_pcm16, bgm::NUM_SAMPLES * sizeof(s16), &bitstream);

            if(rd < 0)
            {
                throw std::runtime_error("BGM READ ERROR");
            }
            else if (rd == 0) // EoF or error
            {
                debugging::log("BGM reached end\n");
                if(music.loop)
                    ov_pcm_seek(&music.source->vf, music.loop_start);
                else
                    ov_pcm_seek(&music.source->vf, 0);
            }
            else 
            {
                buf.nsamples = rd / 4;
                ndspChnWaveBufAdd(0, &buf); // Add buffer to channel
                music.active_buf = 1 - music.active_buf; // switch to other buffer to load and prepare it while the current one is playing
            }
        }
        using namespace std::chrono_literals;
        ctr::this_thread.sleep_for(750'000ns);
    }
}

}
