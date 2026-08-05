#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "../cimgui/cimgui.h"
#include "ui.h"
#include "style.h"
#include "widget.h"

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
static const char* top_tabs[] = {"Modules", "Config", "GUI", "HUD", "Sound", "Search", "Profiles", "Apps", "About"};
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

    // ================= 1. 顶部导航栏 =================
    igSetNextWindowPos((ImVec2){io->DisplaySize.x * 0.5f, 20}, ImGuiCond_Always, (ImVec2){0.5f, 0.0f});
    igBegin("TopBar", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
    
    for (int i = 0; i < 9; i++) {
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
    igEnd();

    // ================= 2. Render 分类窗口 =================
    igSetNextWindowPos((ImVec2){200, 80}, ImGuiCond_FirstUseEver, (ImVec2){0, 0});
    igSetNextWindowSize((ImVec2){220, 450}, ImGuiCond_FirstUseEver);
    
    // 隐藏系统默认标题栏，使用自定义简洁头部
    igBegin("RenderPanel", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

    // 自定义窗口 Header
    igTextDisabled("^"); igSameLine(0, 5);
    igText("Render");
    igSameLine(igGetWindowWidth() - 20, 0);
    igTextDisabled("^");
    igSeparator();
    igDummy((ImVec2){0, 2});

    // 模块列表
    VapeUI_ModuleToggle("Arraylist", &mod_arraylist, NULL, false);
    VapeUI_ModuleToggle("Ambience", &(bool){false}, NULL, false);
    VapeUI_ModuleToggle("AudioVisualizer", &mod_audiovisualizer, NULL, false);
    VapeUI_ModuleToggle("BlackCapture", &mod_blackcapture, "LCtrl+NO", false);
    VapeUI_ModuleToggle("Keystrokes", &mod_keystrokes, "LAlt+K", false);
    VapeUI_ModuleToggle("Background", &mod_background, NULL, true); // 青色高亮
    VapeUI_ModuleToggle("GUIBlur", &mod_guiblur, NULL, true);         // 青色高亮

    igEnd();

    // ================= 3. 右侧 HUD / Arraylist 悬浮列表 =================
    igSetNextWindowPos((ImVec2){io->DisplaySize.x - 10, 80}, ImGuiCond_Always, (ImVec2){1.0f, 0.0f});
    igBegin("HUD_Arraylist", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);

    if (mod_arraylist) {
        igTextColored((ImVec4){0.10f, 0.85f, 0.30f, 1.0f}, "AudioVisualizer");
        igTextColored((ImVec4){0.00f, 0.75f, 0.70f, 1.0f}, "Background");
        igTextColored((ImVec4){0.00f, 0.75f, 0.70f, 1.0f}, "GUIBlur");
    }

    igEnd();
}