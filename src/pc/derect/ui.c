#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "../cimgui/cimgui.h"
#include "ui.h"

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

// 辅助宏：构建 C99 ImVec 结构体
#define VEC2(x, y) ((ImVec2){(float)(x), (float)(y)})
#define VEC4(r, g, b, a) ((ImVec4){(float)(r), (float)(g), (float)(b), (float)(a)})

// =========================================================================
// 1. Vape V4 主题样式定义 (纯 C)
// =========================================================================
void derect_initStyle(void) {
    ImGuiStyle* style = igGetStyle();

    // --- 触屏与尺寸适配 ---
    style->WindowRounding    = 8.0f;
    style->FrameRounding     = 6.0f;
    style->PopupRounding     = 6.0f;
    style->GrabRounding      = 6.0f;
    style->ScrollbarRounding = 6.0f;

    style->WindowPadding     = VEC2(14.0f, 14.0f);
    style->FramePadding      = VEC2(10.0f, 8.0f);
    style->ItemSpacing       = VEC2(10.0f, 10.0f);
    style->ItemInnerSpacing  = VEC2(8.0f, 8.0f);
    style->TouchExtraPadding = VEC2(4.0f, 4.0f);

    style->WindowBorderSize  = 1.0f;
    style->FrameBorderSize   = 0.0f;

    // --- 色彩 Scheme (Vape 紫) ---
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_WindowBg]             = VEC4(0.08f, 0.08f, 0.10f, 0.94f);
    colors[ImGuiCol_ChildBg]              = VEC4(0.12f, 0.12f, 0.15f, 0.85f);
    colors[ImGuiCol_PopupBg]              = VEC4(0.10f, 0.10f, 0.12f, 0.96f);
    colors[ImGuiCol_Border]               = VEC4(0.22f, 0.22f, 0.28f, 0.60f);

    colors[ImGuiCol_Text]                 = VEC4(0.95f, 0.95f, 0.98f, 1.00f);
    colors[ImGuiCol_TextDisabled]         = VEC4(0.45f, 0.45f, 0.52f, 1.00f);

    colors[ImGuiCol_FrameBg]              = VEC4(0.15f, 0.15f, 0.19f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]       = VEC4(0.20f, 0.20f, 0.26f, 1.00f);
    colors[ImGuiCol_FrameBgActive]        = VEC4(0.25f, 0.25f, 0.32f, 1.00f);

    ImVec4 accent                         = VEC4(0.55f, 0.32f, 0.98f, 1.00f);
    ImVec4 accentHovered                  = VEC4(0.65f, 0.42f, 1.00f, 1.00f);
    ImVec4 accentActive                   = VEC4(0.45f, 0.22f, 0.88f, 1.00f);

    colors[ImGuiCol_CheckMark]            = accent;
    colors[ImGuiCol_SliderGrab]           = accent;
    colors[ImGuiCol_SliderGrabActive]     = accentActive;

    colors[ImGuiCol_Button]               = VEC4(0.15f, 0.15f, 0.20f, 1.00f);
    colors[ImGuiCol_ButtonHovered]        = accent;
    colors[ImGuiCol_ButtonActive]         = accentActive;

    colors[ImGuiCol_Header]               = VEC4(0.18f, 0.18f, 0.24f, 1.00f);
    colors[ImGuiCol_HeaderHovered]        = accentHovered;
    colors[ImGuiCol_HeaderActive]         = accentActive;

    colors[ImGuiCol_ScrollbarBg]          = VEC4(0.08f, 0.08f, 0.10f, 0.30f);
    colors[ImGuiCol_ScrollbarGrab]        = VEC4(0.22f, 0.22f, 0.28f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = accent;
}

// =========================================================================
// 2. 自定义 Vape 模块 Toggle 控件 (纯 C 实现)
// =========================================================================
bool VapeUI_ModuleToggle(const char* label, bool* v, const char* desc) {
    ImVec2 avail;
    igGetContentRegionAvail();
    float width = avail.x;
    float height = 48.0f;

    ImVec2 p;
    igGetCursorScreenPos();

    // 隐形按钮响应触屏和点击
    bool pressed = igInvisibleButton(label, VEC2(width, height), 0);
    if (pressed && v) {
        *v = !(*v);
    }

    bool hovered = igIsItemHovered(0);
    ImDrawList* draw_list = igGetWindowDrawList();

    ImVec2 p_min = p;
    ImVec2 p_max = VEC2(p.x + width, p.y + height);

    bool is_active = (v && *v);
    ImU32 bgColor = is_active ? IM_COL32(32, 25, 48, 240) : (hovered ? IM_COL32(26, 26, 32, 240) : IM_COL32(20, 20, 25, 240));
    ImU32 borderColor = is_active ? IM_COL32(140, 82, 255, 255) : IM_COL32(42, 42, 52, 200);

    // 1. 背景与边框
    ImDrawList_AddRectFilled(draw_list, p_min, p_max, bgColor, 6.0f, 0);
    ImDrawList_AddRect(draw_list, p_min, p_max, borderColor, 6.0f, 0, 1.2f);

    // 2. 文本标签与描述
    ImVec2 label_size;
    igCalcTextSize(label, NULL, false, -1.0f);

    ImVec2 text_pos = VEC2(p_min.x + 14.0f, p_min.y + (desc ? 8.0f : (height - label_size.y) * 0.5f));
    ImU32 textColor = is_active ? IM_COL32(255, 255, 255, 255) : IM_COL32(180, 180, 190, 255);
    ImDrawList_AddText_Vec2(draw_list, text_pos, textColor, label, NULL);

    if (desc) {
        ImVec2 desc_pos = VEC2(p_min.x + 14.0f, text_pos.y + label_size.y + 2.0f);
        ImDrawList_AddText_Vec2(draw_list, desc_pos, IM_COL32(110, 110, 125, 255), desc, NULL);
    }

    // 3. 右侧胶囊开关灯
    float toggle_w = 32.0f, toggle_h = 16.0f;
    ImVec2 toggle_pos = VEC2(p_max.x - 14.0f - toggle_w, p_min.y + (height - toggle_h) * 0.5f);

    ImU32 switchBg = is_active ? IM_COL32(140, 82, 255, 255) : IM_COL32(50, 50, 62, 255);
    ImDrawList_AddRectFilled(draw_list, toggle_pos, VEC2(toggle_pos.x + toggle_w, toggle_pos.y + toggle_h), switchBg, 10.0f, 0);

    float circle_x = is_active ? (toggle_pos.x + toggle_w - 8.0f) : (toggle_pos.x + 8.0f);
    ImDrawList_AddCircleFilled(draw_list, VEC2(circle_x, toggle_pos.y + 8.0f), 6.0f, IM_COL32(255, 255, 255, 255), 0);

    return pressed;
}

// =========================================================================
// 3. 主界面渲染逻辑 (纯 C)
// =========================================================================
void derect_panel_render(bool* open) {
    if (!open || !(*open)) return;

    igSetNextWindowSize(VEC2(720, 440), ImGuiCond_FirstUseEver);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoResize;

    if (igBegin("VapeV4_Android_Main", open, flags)) {
        // --- A. 左侧 Sidebar 导航 ---
        if (igBeginChild_Str("Sidebar", VEC2(150, 0), true, 0)) {
            igSetCursorPosY(12);
            igTextColored(VEC4(0.6f, 0.35f, 1.0f, 1.0f), "  VAPE ");
            igSameLine(0, -1);
            igTextDisabled("v4");

            igSpacing();
            igSeparator();
            igSpacing();

            static int current_tab = 0;
            const char* tabs[] = { " Combat", " Movement", " Player", " Visuals", " Settings" };

            for (int i = 0; i < 5; i++) {
                bool is_selected = (current_tab == i);
                if (is_selected) {
                    igPushStyleColor_Vec4(ImGuiCol_Button, VEC4(0.55f, 0.32f, 0.98f, 1.00f));
                }

                if (igButton(tabs[i], VEC2(-1, 40))) {
                    current_tab = i;
                }

                if (is_selected) {
                    igPopStyleColor(1);
                }
            }
        }
        igEndChild();

        igSameLine(0, -1);

        // --- B. 右侧功能面板 ---
        if (igBeginChild_Str("ContentArea", VEC2(0, 0), false, 0)) {
            igTextDisabled("MODULES & CONFIGS");

            ImVec2 avail;
            igGetContentRegionAvail();
            igSameLine(avail.x - 20, -1);
            if (igButton("X", VEC2(24, 24))) {
                *open = false;
            }

            igSeparator();
            igSpacing();

            // 变量声明
            static bool killaura = false;
            static bool moonjump = false;
            static bool speedhack = false;
            static bool esp = true;

            // 渲染模块卡片
            VapeUI_ModuleToggle("Auto Attack", &killaura, "Automatically attacks nearby entities");
            VapeUI_ModuleToggle("Moon Jump", &moonjump, "Higher jump gravity override");
            VapeUI_ModuleToggle("Speed Hack", &speedhack, "Adjust Mario movement multiplier");
            VapeUI_ModuleToggle("Player ESP", &esp, "Draw 2D bounding box on players");

            if (moonjump || speedhack) {
                igSpacing();
                igTextDisabled("PARAMETER TWEAKS");
                if (igBeginChild_Str("SubSettings", VEC2(0, 100), true, 0)) {
                    static float jump_force = 1.5f;
                    static float move_speed = 2.0f;

                    igSliderFloat("Jump Multiplier", &jump_force, 1.0f, 3.0f, "%.1fx", 0);
                    igSliderFloat("Move Speed", &move_speed, 1.0f, 5.0f, "%.1fx", 0);
                }
                igEndChild();
            }
        }
        igEndChild();
    }
    igEnd();
}