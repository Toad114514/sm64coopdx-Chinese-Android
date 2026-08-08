#include <stdbool.h>
#include <math.h>

#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>

#ifndef CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#endif
#include "../../cimgui/cimgui.h"

#include "../module.h"
#include "../config.h"

#define IM_COL32(R, G, B, A) (((ImU32)(A) << 24) | ((ImU32)(B) << 16) | ((ImU32)(G) << 8) | (ImU32)(R))

// GL
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

// GL Handle
static GLuint s_blur_fbo = 0;
static GLuint s_blur_tex = 0;
static int s_fbo_w = 0, s_fbo_h = 0;

// config
static float g_blur_darkness = 0.55f;
static float g_blur_radius   = 5.0f;  // 强度

static const ConfigOption bgblur_options[] = {
    BIND_FLOAT("darkness", "Darkness",    &g_blur_darkness, 0.0f,  1.0f, "%.2f"),
    BIND_FLOAT("radius",   "Blur Radius", &g_blur_radius,   15.0f, 100.0f, "%.1f")
};
#define BGBLUR_OPTION_COUNT sizeof(bgblur_options) / sizeof(bgblur_options[0])

void bgblur_config(void) {
    Config_RenderOptions(bgblur_options, BGBLUR_OPTION_COUNT);
}

static void UpdateBlurFBO(int low_w, int low_h) {
    if (s_fbo_w == low_w && s_fbo_h == low_h && s_blur_fbo != 0) return;

    s_fbo_w = low_w;
    s_fbo_h = low_h;

    // 清理旧资源
    if (s_blur_fbo) glDeleteFramebuffers(1, &s_blur_fbo);
    if (s_blur_tex) glDeleteTextures(1, &s_blur_tex);

    // 1. 创建低分辨率纹理
    glGenTextures(1, &s_blur_tex);
    glBindTexture(GL_TEXTURE_2D, s_blur_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, low_w, low_h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    
    // 【关键】：开启 GL_LINEAR 线性过滤，放大时自动生成弥散模糊
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // 2. 绑定到专用 FBO
    glGenFramebuffers(1, &s_blur_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, s_blur_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s_blur_tex, 0);

    // 还原默认帧缓冲区绑定
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}


void bgblur_render(void) {
    ImGuiIO* io = igGetIO();
    if (!io) return;

    int screen_w = (int)io->DisplaySize.x;
    int screen_h = (int)io->DisplaySize.y;
    if (screen_w <= 0 || screen_h <= 0) return;

    // low.w/low.h
    int low_w = (int)(screen_w / g_blur_radius);
    int low_h = (int)(screen_h / g_blur_radius);
    if (low_w < 16) low_w = 16;
    if (low_h < 16) low_h = 16;

    UpdateBlurFBO(low_w, low_h);

    // FBO 保存
    GLint old_read_fbo = 0, old_draw_fbo = 0;
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &old_read_fbo);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &old_draw_fbo);

    // fuckoff
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, s_blur_fbo);

    // GL_LINEAR 
    glBlitFramebuffer(
        0, 0, screen_w, screen_h,
        0, 0, low_w, low_h,
        GL_COLOR_BUFFER_BIT, GL_LINEAR
    );
    
    glBindFramebuffer(GL_READ_FRAMEBUFFER, old_read_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, old_draw_fbo);
    
    ImDrawList* bgList = igGetBackgroundDrawList(NULL);
    if (bgList && s_blur_tex != 0) {
        // 倒灌
        ImVec2 uv0 = {0.0f, 1.0f};
        ImVec2 uv1 = {1.0f, 0.0f};
        
        ImTextureRef_c tex_ref = (ImTextureRef_c){ NULL, (ImTextureID)(uintptr_t)s_blur_tex };
        
        ImDrawList_AddImage(
            bgList,
            tex_ref,
            (ImVec2){0.0f, 0.0f},
            (ImVec2){(float)screen_w, (float)screen_h},
            uv0, uv1,
            0xFFFFFFFF
        );

        // darkness overlay
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

void module_bgblur(void){
    Module_Register("BackgroundBlur", CAT_RENDER, false, NULL, MOD_COLOR_BLUE, NULL, NULL, NULL);
    Module_HookConfig("BackgroundBlur", bgblur_config);
    Module_HookRender("BackgroundBlur", bgblur_render);
    Config_RegisterModuleOptions("BackgroundBlur", bgblur_options, BGBLUR_OPTION_COUNT);
}