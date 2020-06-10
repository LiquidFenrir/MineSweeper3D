#include "verts.h"

#include "vshader_shbin.h"

#include <cassert>

namespace ProgramWide {
    DVLB_s* vshader_dvlb = nullptr;
    shaderProgram_s program;

    int uLoc_projection, uLoc_modelView;
    C3D_Mtx projection;
    C3D_Tex* sprites_tex = nullptr;

    void init(C3D_Tex* tex)
    {
        sprites_tex = tex;

        vshader_dvlb = DVLB_ParseFile((u32*)vshader_shbin, vshader_shbin_size);
        shaderProgramInit(&program);
        shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);

        uLoc_projection   = shaderInstanceGetUniformLocation(program.vertexShader, "projection");
        uLoc_modelView    = shaderInstanceGetUniformLocation(program.vertexShader, "modelView");

        Mtx_PerspTilt(&projection, C3D_AngleFromDegrees(60.0f), C3D_AspectRatioTop, 0.01f, 100.0f, false);

        // C3D_TexSetFilter(sprites_tex, GPU_LINEAR, GPU_NEAREST);
    }

    void exit()
    {
        if(vshader_dvlb)
        {
            shaderProgramFree(&program);
            DVLB_Free(vshader_dvlb);
            vshader_dvlb = nullptr;
            sprites_tex = nullptr;
        }
    }
};

namespace LevelWide {
    void* vbo_data = nullptr;
    size_t vertex_count = 0;

    C3D_FogLut fog_Lut;

    const C3D_Material material =
    {
        { 0.125f, 0.125f, 0.125f }, //ambient
        { 0.4f, 0.4f, 0.4f }, //diffuse
        { 0.5f, 0.5f, 0.5f }, //specular0
        { 0.0f, 0.0f, 0.0f }, //specular1
        { 0.0f, 0.0f, 0.0f }, //emission
    };

    C3D_LightEnv lightEnv;
    C3D_Light light;
    C3D_LightLut lut_Spec;
    C3D_FVec lightPos = FVec4_New(0.0f, 0.0f, 0.0f, 1.0f);

    C3D_AttrInfo vbo_attrInfo;
    C3D_BufInfo vbo_bufInfo;

    void init(const std::vector<Vertex>& vertices)
    {
        // Configure attributes for use with the vertex shader
        AttrInfo_Init(&vbo_attrInfo);
        AttrInfo_AddLoader(&vbo_attrInfo, 0, GPU_FLOAT, 3); // v0=position
        AttrInfo_AddLoader(&vbo_attrInfo, 1, GPU_FLOAT, 2); // v1=texcoord
        AttrInfo_AddLoader(&vbo_attrInfo, 2, GPU_FLOAT, 3); // v2=normal

        // Create and fill the VBO (vertex buffer object)
        vertex_count = vertices.size();
        vbo_data = linearAlloc(sizeof(Vertex) * vertex_count);

        // Configure buffers
        BufInfo_Init(&vbo_bufInfo);
        BufInfo_Add(&vbo_bufInfo, vbo_data, sizeof(Vertex), 3, 0x210);

        C3D_LightEnvInit(&lightEnv);
        C3D_LightEnvMaterial(&lightEnv, &material);

        LightLut_Phong(&lut_Spec, 5.0f);
        C3D_LightEnvLut(&lightEnv, GPU_LUT_D0, GPU_LUTINPUT_NH, false, &lut_Spec);

        C3D_LightInit(&light, &lightEnv);
        C3D_LightColor(&light, 1.0f, 1.0f, 1.0f);

        FogLut_Exp(&fog_Lut, 0.05f, 1.5f, 0.0025f, 20.0f);

        set_verts(vertices);
    }

    void set_verts(const std::vector<Vertex>& vertices)
    {
        assert(vertices.size() == vertex_count);
        memcpy(vbo_data, vertices.data(), sizeof(Vertex) * vertex_count);
    }

    void exit()
    {
        if(vbo_data)
        {
            linearFree(vbo_data);
            vbo_data = nullptr;
        }
    }
};

namespace ThreeD {
    void bind()
    {
        C3D_BindProgram(&ProgramWide::program);
        C3D_SetAttrInfo(&LevelWide::vbo_attrInfo);
        C3D_SetBufInfo(&LevelWide::vbo_bufInfo);
        C3D_LightEnvBind(&LevelWide::lightEnv);
        C3D_FogLutBind(&LevelWide::fog_Lut);
        C3D_FogGasMode(GPU_FOG, GPU_PLAIN_DENSITY, false);
        C3D_FogColor(0xD8B068);
        // C3D_DepthTest(true, GPU_GREATER, GPU_WRITE_DEPTH);
        C3D_CullFace(GPU_CULL_NONE);
        C3D_TexBind(0, ProgramWide::sprites_tex);

        // Configure the first fragment shading substage to blend the fragment primary color
        // with the fragment secondary color.
        // See https://www.opengl.org/sdk/docs/man2/xhtml/glTexEnv.xml for more insight
        C3D_TexEnv* env = C3D_GetTexEnv(0);
        C3D_TexEnvInit(env);
        C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0);
	    C3D_TexEnvColor(env, 0xFFFFFFFF);
        C3D_TexEnvFunc(env, C3D_RGB, GPU_REPLACE);
        C3D_TexEnvFunc(env, C3D_Alpha, GPU_MODULATE);

        // Clear out the other texenvs
        C3D_TexEnvInit(C3D_GetTexEnv(1));
        C3D_TexEnvInit(C3D_GetTexEnv(2));
        C3D_TexEnvInit(C3D_GetTexEnv(3));
        C3D_TexEnvInit(C3D_GetTexEnv(4));
        C3D_TexEnvInit(C3D_GetTexEnv(5));
    }

    void draw(float posX, float posZ, float angleX, float angleY, bool looking_at_floor)
    {
        // Calculate the modelView matrix
        C3D_Mtx modelView;
        Mtx_Identity(&modelView);
        const auto angle = C3D_AngleFromDegrees(angleX - 90);
        Mtx_RotateY(&modelView, angle, true);
        Mtx_Rotate(&modelView, FVec3_New(cosf(angle), 0.0f, sinf(angle)), C3D_AngleFromDegrees(-angleY), true);
        Mtx_Translate(&modelView, posX, 0.0f, posZ, true);

        LevelWide::lightPos.x = posX;
        LevelWide::lightPos.z = posZ;
        C3D_LightPosition(&LevelWide::light, &LevelWide::lightPos);

        // Update the uniforms
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, ProgramWide::uLoc_projection, &ProgramWide::projection);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, ProgramWide::uLoc_modelView,  &modelView);

        const int count = LevelWide::vertex_count - (looking_at_floor ? 0 : 6);
        C3D_DrawArrays(GPU_TRIANGLES, 0, count);
    }
};
