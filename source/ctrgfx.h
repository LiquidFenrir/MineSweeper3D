#ifndef GFX3DS_INC
#define GFX3DS_INC

#include <3ds.h>
#include <citro3d.h>
#include <citro2d.h>
#include <optional>
#include <array>
#include <span>
#include <ranges>
#include "useful_utilities.h"

namespace ctr {

struct gfx {
    enum class screen_mode {
        regular,
        stereo,
        wide,
    };

    gfx(const screen_mode mode);
    ~gfx();

    gfx& operator=(const gfx&) = delete;
    gfx& operator=(gfx&&) = delete;
    gfx(const gfx&) = delete;
    gfx(gfx&&) = delete;

    const screen_mode current_mode;

    struct color {
        explicit operator u32() const
        {
            return C2D_Color32(r, g, b, a);
        }
        explicit operator C3D_FVec() const
        {
            return FVec4_Scale(FVec4_New(r, g, b, a), 1.0f/255.0f);
        }

        u8 r, g, b, a;
    };

    struct shader {
        shader(const shader&) = delete;
        shader(shader&&) = delete;
        shader& operator=(const shader&) = delete;
        shader& operator=(shader&&) = delete;

        ~shader()
        {
            shaderProgramFree(&program);
            DVLB_Free(shader_dvlb);
        }

        DVLB_s* shader_dvlb;
        shaderProgram_s program;
    
    protected:
        shader(u32* const shader_binary, const u32 shader_size)
        {
            shader_dvlb = DVLB_ParseFile(shader_binary, shader_size);
            shaderProgramInit(&program);
            shaderProgramSetVsh(&program, &shader_dvlb->DVLE[0]);
            if(shader_dvlb->numDVLE > 1)
            {
                shaderProgramSetGsh(&program, &shader_dvlb->DVLE[1], 0);
            }
        }
    };
    template<int uniform_count>
    struct shader_with_uniforms : public shader {
        shader_with_uniforms(u32* const shader_binary, const u32 shader_size, std::span<const std::pair<const char*, shaderInstance_s* shaderProgram_s::*>, uniform_count> uniform_names)
            : shader(shader_binary, shader_size)
        {
            for(int i = 0; i < uniform_count; ++i)
            {
                const auto& [name, shader_type] = uniform_names[i];
                uniforms[i] = shaderInstanceGetUniformLocation(program.*shader_type, name);
            }
        }

        std::array<int, uniform_count> uniforms;
    };

    struct tex3ds_deleter {
        void operator()(Tex3DS_Texture t3x)
        {
            Tex3DS_TextureFree(t3x);
        }
    };
    using tex3ds_ptr = util::ptr_for<Tex3DS_Texture, tex3ds_deleter>;

    struct target {
        void clear(const color clear_color)
        {
            C3D_RenderTargetClear(t, C3D_CLEAR_COLOR, __builtin_bswap32(u32(clear_color)), 0);
        }
        void clear(const u32 clear_depth)
        {
            C3D_RenderTargetClear(t, C3D_CLEAR_DEPTH, 0, clear_depth);
        }
        void clear(const color clear_color, const std::optional<u32> clear_depth = std::nullopt)
        {
            C3D_RenderTargetClear(t, clear_depth ? C3D_CLEAR_ALL : C3D_CLEAR_COLOR, __builtin_bswap32(u32(clear_color)), clear_depth ? *clear_depth : 0);
        }

        void focus()
        {
            C2D_SceneBegin(t);
        }

    private:
        friend gfx;
        friend struct texture;

        target(C3D_RenderTarget* t_)
            : t(t_)
        {

        }

        // non owning
        C3D_RenderTarget* t;
    };

    struct texture {
        ~texture()
        {
            if(render_target)
            {
                C3D_RenderTargetDelete(render_target);
            }
            if(tex)
            {
                C3D_TexDelete(&*tex);
            }
        }

        static std::optional<texture> load_from_file(const char* const path)
        {
            util::file_ptr f(fopen(path, "rb"));
            if (!f)
                return std::nullopt;

            texture out;
            tex3ds_ptr t3x(Tex3DS_TextureImportStdio(f.get(), &out.tex.emplace(), nullptr, false));
            if (!t3x)
            {
                out.tex.reset();
                return std::nullopt;
            }

            return out;
        }
        static texture create(const u16 width, const u16 height, const GPU_TEXCOLOR format)
        {
            return texture(width, height, format);
        }
        static texture create_renderable(const u16 width, const u16 height, const GPU_TEXCOLOR format, std::optional<GPU_DEPTHBUF> depthtype = std::nullopt)
        {
            return texture(width, height, format, depthtype);
        }

        std::optional<target> get_target()
        {
            return render_target ? std::make_optional<target>(render_target) : std::nullopt;
        }
        C3D_Tex* get_tex()
        {
            return tex ? &*tex : nullptr;
        }

        void bind(int id)
        {
            if(tex)
            {
                C3D_TexBind(id, &*tex);
            }
        }

        texture(texture&& other)
            : tex(std::move(other.tex))
            , render_target(std::exchange(other.render_target, nullptr))
        {
            other.tex.reset();
        }
        texture& operator=(texture&& other)
        {
            tex = std::move(other.tex);
            other.tex.reset();
            render_target = std::exchange(other.render_target, nullptr);
            return *this;
        }

    private:
        texture() = default;
        texture(const u16 width, const u16 height, const GPU_TEXCOLOR format, std::optional<GPU_DEPTHBUF> depthtype)
        {
            C3D_TexInitVRAM(&tex.emplace(), width, height, format);
            render_target = C3D_RenderTargetCreateFromTex(&*tex, GPU_TEXFACE_2D, 0, depthtype ? *depthtype : -1);
        }
        texture(const u16 width, const u16 height, const GPU_TEXCOLOR format)
        {
            C3D_TexInit(&tex.emplace(), width, height, format);
        }

        std::optional<C3D_Tex> tex;
        C3D_RenderTarget* render_target{nullptr};
    };

    struct spritesheet {
        spritesheet(C2D_SpriteSheet&& s)
            : sheet(s)
        {

        }
        ~spritesheet()
        {
            C2D_SpriteSheetFree(sheet);
        }

        C2D_Image get_image(const std::size_t index)
        {
            return C2D_SpriteSheetGetImage(sheet, index);
        }

    private:
        C2D_SpriteSheet sheet;
    };

    struct font {
        C2D_Font fnt;

        font(const char* const path) : fnt(C2D_FontLoad(path))
        {

        }

        ~font()
        {
            C2D_FontFree(fnt);
        }
    };

    struct text_buf {
        C2D_TextBuf buf;

        text_buf(const std::size_t size) : buf(C2D_TextBufNew(size))
        {

        }

        ~text_buf()
        {
            C2D_TextBufDelete(buf);
        }

        void clear()
        {
            C2D_TextBufClear(buf);
        }
    };

    struct text_generator {

        text_generator(text_buf& b, font* f = nullptr)
            : buf{b}
            , fnt{f}
        {

        }

        void clear()
        {
            buf.clear();
        }

        C2D_Text parse(const char * s)
        {
            C2D_Text out;
            if(fnt)
                C2D_TextFontParse(&out, fnt->fnt, buf.buf, s);
            else
                C2D_TextParse(&out, buf.buf, s);
            C2D_TextOptimize(&out);
            return out;
        }
        std::pair<C2D_Text, const char*> parse_line(const char * &  s, u32 line)
        {
            C2D_Text out;
            const char* out_s = nullptr;
            if(fnt)
                out_s = C2D_TextFontParseLine(&out, fnt->fnt, buf.buf, s, line);
            else
                out_s = C2D_TextParseLine(&out, buf.buf, s, line);
            C2D_TextOptimize(&out);
            return {out, out_s};
        }

        text_buf& buf;
    private:
        font* fnt;
    };

    void begin_frame();
    void clear_screens(const color top, const color bottom);
    void end_frame();

    std::optional<target> get_screen(const gfxScreen_t screen, const gfx3dSide_t side = GFX_LEFT);
    struct dimensions {
        int width, height;
    };
    dimensions get_screen_dimensions(const gfxScreen_t screen) const;

    float stereo() const;

    void render_2d()
    {
        if(current_rendering_mode == rendering_mode::citro2d)
        {
            C2D_Flush();
        }
        else
        {
            C2D_Prepare();
        }
        current_rendering_mode = rendering_mode::citro2d;
    }

    void render_3d(shader& using_shader)
    {
        if(current_rendering_mode == rendering_mode::citro2d)
        {
            C2D_Flush();
        }
        current_rendering_mode = rendering_mode::citro3d;

        C3D_BindProgram(&using_shader.program);
        C3D_DepthTest(true, GPU_GREATER, GPU_WRITE_ALL);
        C3D_CullFace(GPU_CULL_BACK_CCW);

        C3D_TexEnvInit(C3D_GetTexEnv(0));
        C3D_TexEnvInit(C3D_GetTexEnv(1));
        C3D_TexEnvInit(C3D_GetTexEnv(2));
        C3D_TexEnvInit(C3D_GetTexEnv(3));
        C3D_TexEnvInit(C3D_GetTexEnv(4));
        C3D_TexEnvInit(C3D_GetTexEnv(5));
    }

private:
    enum class rendering_mode {
        none,
        citro2d,
        citro3d,
    };
    rendering_mode current_rendering_mode;

    C3D_RenderTarget* top_left_target;
    C3D_RenderTarget* top_right_target;
    C3D_RenderTarget* bottom_target;
};

}

#endif
