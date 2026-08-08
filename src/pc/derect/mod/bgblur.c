#include <stdbool.h>
#include <math.h>

#ifndef CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#endif
#include "../../cimgui/cimgui.h"

#include "../module.h"
#include "../config.h"

#define IM_COL32(R, G, B, A) (((ImU32)(A) << 24) | ((ImU32)(B) << 16) | ((ImU32)(G) << 8) | (ImU32)(R))

// config
static float g_blur_darkness = 0.55f;
static float g_blur_radius   = 5.0f;  // 强度

static const ConfigOption bgblur_options[] = {
    BIND_FLOAT("darkness", "Darkness",    &g_blur_darkness, 0.0f, 1.0f, "%.2f"),
    BIND_FLOAT("radius",   "Blur Radius", &g_blur_radius,   1.0f, 10.0f, "%.1f")
};
#define BGBLUR_OPTION_COUNT sizeof(bgblur_options) / sizeof(bgblur_options[0])

void bgblur_config(void) {
    Config_RenderOptions(bgblur_options, BGBLUR_OPTION_COUNT);
}

void bgblur_render(void) {
    ImDrawList* bgDrawList = igGetBackgroundDrawList(NULL);
    ImGuiIO* io = igGetIO();
    if (!bgDrawList || !io) return;
    
    ImVec2 screenSize = io->DisplaySize;
    if (screenSize.x <= 0 || screenSize.y <= 0) return;
    
    int alpha = (int)(g_blur_darkness * 220.0f);
    if (alpha > 255) alpha = 255;
    if (alpha < 0)   alpha = 0;
    
    ImU32 overlayColor = IM_COL32(10, 12, 16, alpha);
    ImDrawList_AddRectFilled(bgDrawList, (ImVec2){0, 0}, screenSize, overlayColor, 0.0f, 0);
    
    int passes = (int)g_blur_radius;
    if (passes > 1) {
        float step = g_blur_radius * 0.6f;
        ImU32 blurColor = IM_COL32(255, 255, 255, (int)(12.0f / passes));

        for (int i = 1; i <= passes; i++) {
            float offset = i * step;
            ImDrawList_AddRectFilled(
                bgDrawList,
                (ImVec2){-offset, -offset},
                (ImVec2){screenSize.x + offset, screenSize.y + offset},
                blurColor,
                0.0f, 0
            );
        }
    }
}

void module_bgblur(void){
    Module_Register("BackgroundBlur", CAT_RENDER, false, NULL, MOD_COLOR_BLUE, NULL, NULL, NULL);
    Module_HookConfig("BackgroundBlur", bgblur_config);
    Module_HookRender("BackgroundBlur", bgblur_render);
    Config_RegisterModuleOptions("BackgroundBlur", bgblur_options, BGBLUR_OPTION_COUNT);
}