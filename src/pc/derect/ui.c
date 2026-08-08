#ifndef CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#endif

#include "../cimgui/cimgui.h"
#include "ui.h"
#include "style.h"
#include "widget.h"
#include "module.h"

// col32
#ifndef IM_COL32
#define IM_COL32_R_SHIFT 0
#define IM_COL32_G_SHIFT 8
#define IM_COL32_B_SHIFT 16
#define IM_COL32_A_SHIFT 24
#define IM_COL32(R,G,B,A) (((ImU32)(A)<<IM_COL32_A_SHIFT) | ((ImU32)(B)<<IM_COL32_B_SHIFT) | ((ImU32)(G)<<IM_COL32_G_SHIFT) | ((ImU32)(R)<<IM_COL32_R_SHIFT))
#endif

// More col32
#ifndef IM_COL32_WHITE
#define IM_COL32_WHITE IM_COL32(255,255,255,255)
#define IM_COL32_BLACK IM_COL32(0,0,0,255)
#define IM_COL32_BLACK_TRANS IM_COL32(0,0,0,0)
#endif

// =========================================================================
// 3. 主界面渲染逻辑 (纯 C)
// =========================================================================
// 顶栏菜单项结构
static const char* top_tabs[] = {"Modules", "Config", "HUD", "Search", "Profiles", "About"};
static int active_tab = 0;

// 模拟模块状态
static bool mod_arraylist = true;
static bool mod_audiovisualizer = true;
static bool mod_blackcapture = false;
static bool mod_keystrokes = false;
static bool mod_background = true;
static bool mod_guiblur = true;

void derect_panel_render(bool* p_open) {
    if (!*p_open) return;

    ImGuiIO* io = igGetIO();
    
    // ============ 其他模块的组件渲染 ===============
    
    // >> Background Blur
    Module* bgblur = Module_Find("BackgroundBlur");
    
    if (bgblur && bgblur->enabled && bgblur->on_render) {
        bgblur->on_render();
    }

    // ================= 顶部导航栏 =================
    igSetNextWindowPos((ImVec2){io->DisplaySize.x * 0.5f, 20}, ImGuiCond_Always, (ImVec2){0.5f, 0.0f});
    igBegin("TopBar", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
    
    int tab_count = sizeof(top_tabs) / sizeof(top_tabs[0]);
    
    for (int i = 0; i < tab_count; i++) {
        if (i > 0) igSameLine(0, 8);
        if (active_tab == i) {
            igPushStyleColor_Vec4(ImGuiCol_Button, (ImVec4){0.8f, 0.2f, 0.2f, 1.0f}); // 选中红框/红块
        } else {
            igPushStyleColor_Vec4(ImGuiCol_Button, (ImVec4){0.0f, 0.0f, 0.0f, 0.0f});
        }
        
        if (igButton(top_tabs[i], (ImVec2){0, 0})) {
            active_tab = i;
        }
        igPopStyleColor(1);
    }
    
    igSameLine(0, 16);

    // Close
    igPushStyleColor_Vec4(ImGuiCol_Button, (ImVec4){0.0f, 0.0f, 0.0f, 0.0f});
    igPushStyleColor_Vec4(ImGuiCol_ButtonHovered, (ImVec4){0.8f, 0.1f, 0.1f, 0.8f});
    igPushStyleColor_Vec4(ImGuiCol_ButtonActive, (ImVec4){0.6f, 0.0f, 0.0f, 1.0f});
    igPushStyleColor_Vec4(ImGuiCol_Text, (ImVec4){0.7f, 0.7f, 0.7f, 1.0f});
    
    if (igButton("X##global_close", (ImVec2){20, 0})) {
        *p_open = false;
    }
    igPopStyleColor(4);

    igEnd();

    // ================= Render 分类窗口 =================
    
    float start_x = 50.0f;
    float start_y = 70.0f;
    float panel_width = 300.0f; // 每个面板间隔宽度

    for (int cat = 0; cat < CAT_COUNT; cat++) {
        ImVec2 panel_pos = (ImVec2){start_x + cat * panel_width, start_y};
        VapeUI_RenderCategoryPanel((ModuleCategory)cat, panel_pos);
    }
    
    //igEnd();
}