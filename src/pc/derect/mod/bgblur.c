#include <stdbool.h>
#include <math.h>
#include <stdio.h>

#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>

#ifndef CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#endif
#include "../../cimgui/cimgui.h"

#include "../module.h"
#include "../config.h"

#define IM_COL32(R, G, B, A) (((ImU32)(A) << 24) | ((ImU32)(B) << 16) | ((ImU32)(G) << 8) | (ImU32)(R))

// GL Framebuffer Defines
#ifndef GL_READ_FRAMEBUFFER
#define GL_READ_FRAMEBUFFER 0x8CA8
#endif
#ifndef GL_DRAW_FRAMEBUFFER
#define GL_DRAW_FRAMEBUFFER 0x8CA9
#endif
#ifndef GL_READ_FRAMEBUFFER_BINDING
#define GL_READ_FRAMEBUFFER_BINDING 0x8CAA
#endif
#ifndef GL_DRAW_FRAMEBUFFER_BINDING
#define GL_DRAW_FRAMEBUFFER_BINDING 0x8CA7
#endif

// GL Shader & FBO Handles
static GLuint s_shader_program = 0;
static GLint  s_u_dir_loc       = -1;
static GLint  s_u_radius_loc    = -1;
static GLint  s_u_tex_loc       = -1;

static GLuint s_fbo_raw  = 0, s_tex_raw  = 0;
static GLuint s_fbo_ping = 0, s_tex_ping = 0;
static GLuint s_fbo_pong = 0, s_tex_pong = 0;

static int s_buf_w = 0, s_buf_h = 0;

// Config
static float g_blur_darkness = 0.55f;
static float g_blur_radius   = 3.0f;  // 高斯采样半径

static const ConfigOption bgblur_options[] = {
    BIND_FLOAT("darkness", "Darkness",    &g_blur_darkness, 0.0f, 1.0f, "%.2f"),
    BIND_FLOAT("radius",   "Blur Radius", &g_blur_radius,   0.5f, 10.0f, "%.1f")
};
#define BGBLUR_OPTION_COUNT (sizeof(bgblur_options) / sizeof(bgblur_options[0]))

// --- GLSL Shaders (GLES 3.0) ---
static const char* g_vs_src =
    "#version 300 es\n"
    "out vec2 vTexCoord;\n"
    "void main() {\n"
    "    // 无需 VAO/VBO，直接根据 VertexID 生成覆盖全屏的三角形\n"
    "    float x = -1.0 + float((gl_VertexID & 1) << 2);\n"
    "    float y = -1.0 + float((gl_VertexID & 2) << 1);\n"
    "    vTexCoord = vec2((x + 1.0) * 0.5, (y + 1.0) * 0.5);\n"
    "    gl_Position = vec4(x, y, 0.0, 1.0);\n"
    "}\n";

static const char* g_fs_src =
    "#version 300 es\n"
    "precision mediump float;\n"
    "in vec2 vTexCoord;\n"
    "out vec4 FragColor;\n"
    "\n"
    "uniform sampler2D u_texture;\n"
    "uniform vec2 u_direction;\n"
    "uniform float u_radius;\n"
    "\n"
    "void main() {\n"
    "    // 9-Tap 核心高斯权重分布\n"
    "    float weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);\n"
    "    vec4 result = texture(u_texture, vTexCoord) * weights[0];\n"
    "    for (int i = 1; i < 5; i++) {\n"
    "        vec2 offset = u_direction * (float(i) * u_radius);\n"
    "        result += texture(u_texture, vTexCoord + offset) * weights[i];\n"
    "        result += texture(u_texture, vTexCoord - offset) * weights[i];\n"
    "    }\n"
    "    FragColor = result;\n"
    "}\n";

static GLuint CompileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static void InitShader(void) {
    if (s_shader_program != 0) return;

    GLuint vs = CompileShader(GL_VERTEX_SHADER, g_vs_src);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, g_fs_src);
    if (!vs || !fs) return;

    s_shader_program = glCreateProgram();
    glAttachShader(s_shader_program, vs);
    glAttachShader(s_shader_program, fs);
    glLinkProgram(s_shader_program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    s_u_dir_loc    = glGetUniformLocation(s_shader_program, "u_direction");
    s_u_radius_loc = glGetUniformLocation(s_shader_program, "u_radius");
    s_u_tex_loc    = glGetUniformLocation(s_shader_program, "u_texture");
}

static void CreateFBO(GLuint* fbo, GLuint* tex, int w, int h) {
    if (*fbo) glDeleteFramebuffers(1, fbo);
    if (*tex) glDeleteTextures(1, tex);

    glGenTextures(1, tex);
    glBindTexture(GL_TEXTURE_2D, *tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenFramebuffers(1, fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, *fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *tex, 0);
}

static void UpdateFBOs(int w, int h) {
    if (s_buf_w == w && s_buf_h == h && s_fbo_raw != 0) return;

    s_buf_w = w;
    s_buf_h = h;

    CreateFBO(&s_fbo_raw,  &s_tex_raw,  w, h);
    CreateFBO(&s_fbo_ping, &s_tex_ping, w, h);
    CreateFBO(&s_fbo_pong, &s_tex_pong, w, h);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void bgblur_config(void) {
    Config_RenderOptions(bgblur_options, BGBLUR_OPTION_COUNT);
}

void bgblur_render(void) {
    ImGuiIO* io = igGetIO();
    if (!io) return;

    int screen_w = (int)io->DisplaySize.x;
    int screen_h = (int)io->DisplaySize.y;
    if (screen_w <= 0 || screen_h <= 0) return;

    InitShader();
    if (!s_shader_program) return;

    UpdateFBOs(screen_w, screen_h);

    // 1. 备份 GL 渲染状态
    GLint old_program = 0, old_active_tex = 0, old_tex_binding = 0;
    GLint old_read_fbo = 0, old_draw_fbo = 0, old_viewport[4];
    
    glGetIntegerv(GL_CURRENT_PROGRAM, &old_program);
    glGetIntegerv(GL_ACTIVE_TEXTURE, &old_active_tex);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &old_tex_binding);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &old_read_fbo);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &old_draw_fbo);
    glGetIntegerv(GL_VIEWPORT, old_viewport);

    GLboolean scissor_was_enabled = glIsEnabled(GL_SCISSOR_TEST);
    if (scissor_was_enabled) glDisable(GL_SCISSOR_TEST);

    // 2. 拷贝当前屏幕 Framebuffer (0) -> s_fbo_raw
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, s_fbo_raw);
    glBlitFramebuffer(0, 0, screen_w, screen_h, 0, 0, screen_w, screen_h, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // 准备 Shader 绘制
    glUseProgram(s_shader_program);
    glUniform1i(s_u_tex_loc, 0);
    glUniform1f(s_u_radius_loc, g_blur_radius);
    glViewport(0, 0, screen_w, screen_h);

    glActiveTexture(GL_TEXTURE0);

    // 3. Pass 1: 水平方向高斯模糊 (s_tex_raw -> s_fbo_ping)
    glBindFramebuffer(GL_FRAMEBUFFER, s_fbo_ping);
    glBindTexture(GL_TEXTURE_2D, s_tex_raw);
    glUniform2f(s_u_dir_loc, 1.0f / (float)screen_w, 0.0f);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // 4. Pass 2: 垂直方向高斯模糊 (s_tex_ping -> s_fbo_pong)
    glBindFramebuffer(GL_FRAMEBUFFER, s_fbo_pong);
    glBindTexture(GL_TEXTURE_2D, s_tex_ping);
    glUniform2f(s_u_dir_loc, 0.0f, 1.0f / (float)screen_h);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // 5. 还原 GL 状态
    glUseProgram(old_program);
    glActiveTexture(old_active_tex);
    glBindTexture(GL_TEXTURE_2D, old_tex_binding);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, old_read_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, old_draw_fbo);
    glViewport(old_viewport[0], old_viewport[1], old_viewport[2], old_viewport[3]);
    if (scissor_was_enabled) glEnable(GL_SCISSOR_TEST);

    // 6. 将最终高斯模糊纹理 (s_tex_pong) 提交给 ImGui 绘制
    ImDrawList* bgList = igGetBackgroundDrawList(NULL);
    if (bgList && s_tex_pong != 0) {
        ImVec2 uv0 = {0.0f, 1.0f};
        ImVec2 uv1 = {1.0f, 0.0f};

        ImTextureRef_c tex_ref = (ImTextureRef_c){ NULL, (ImTextureID)(uintptr_t)s_tex_pong };

        ImDrawList_AddImage(
            bgList,
            tex_ref,
            (ImVec2){0.0f, 0.0f},
            (ImVec2){(float)screen_w, (float)screen_h},
            uv0, uv1,
            0xFFFFFFFF
        );

        // 7. Darkness 叠加遮罩
        int alpha = (int)(g_blur_darkness * 255.0f);
        if (alpha > 255) alpha = 255;
        if (alpha < 0)   alpha = 0;

        ImU32 overlayColor = ((ImU32)alpha << 24) | 0x000A0A0E;
        ImDrawList_AddRectFilled(
            bgList,
            (ImVec2){0.0f, 0.0f},
            (ImVec2){(float)screen_w, (float)screen_h},
            overlayColor,
            0.0f, 0
        );
    }
}

void bgblur_cleanup(void) {
    if (s_shader_program) { glDeleteProgram(s_shader_program); s_shader_program = 0; }
    if (s_fbo_raw)  { glDeleteFramebuffers(1, &s_fbo_raw);  s_fbo_raw = 0; }
    if (s_fbo_ping) { glDeleteFramebuffers(1, &s_fbo_ping); s_fbo_ping = 0; }
    if (s_fbo_pong) { glDeleteFramebuffers(1, &s_fbo_pong); s_fbo_pong = 0; }
    if (s_tex_raw)  { glDeleteTextures(1, &s_tex_raw);  s_tex_raw = 0; }
    if (s_tex_ping) { glDeleteTextures(1, &s_tex_ping); s_tex_ping = 0; }
    if (s_tex_pong) { glDeleteTextures(1, &s_tex_pong); s_tex_pong = 0; }
    s_buf_w = 0;
    s_buf_h = 0;
}

void module_bgblur(void){
    Module_Register("BackgroundBlur", CAT_RENDER, false, NULL, MOD_COLOR_BLUE, NULL, NULL, bgblur_cleanup);
    Module_HookConfig("BackgroundBlur", bgblur_config);
    Module_HookRender("BackgroundBlur", bgblur_render);
    Config_RegisterModuleOptions("BackgroundBlur", bgblur_options, BGBLUR_OPTION_COUNT);
}