#include <string.h>

#ifndef CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#endif

#include "../cimgui/cimgui.h"
#include "style.h"

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

void derect_initStyle(void) {
    ImGuiStyle* style = igGetStyle();
    ImVec4* colors = style->Colors;
    
    ImGuiStyle_ScaleAllSizes(style, 1.8f);

    // 布局尺寸调整
    style->WindowPadding = (ImVec2){6, 6};
    style->FramePadding = (ImVec2){8, 4};
    style->ItemSpacing = (ImVec2){4, 2};
    style->ItemInnerSpacing = (ImVec2){4, 4};
    style->WindowRounding = 4.0f;
    style->FrameRounding = 2.0f;
    style->WindowBorderSize = 1.0f;

    // 核心颜色配色
    colors[ImGuiCol_WindowBg]             = (ImVec4){0.05f, 0.05f, 0.06f, 0.92f}; // 主深灰背景
    colors[ImGuiCol_Header]               = (ImVec4){0.12f, 0.12f, 0.15f, 0.60f}; // 未激活悬浮
    colors[ImGuiCol_HeaderHovered]        = (ImVec4){0.18f, 0.18f, 0.22f, 0.80f};
    colors[ImGuiCol_HeaderActive]         = (ImVec4){0.10f, 0.70f, 0.25f, 1.00f};
    colors[ImGuiCol_Button]               = (ImVec4){0.08f, 0.08f, 0.10f, 0.80f};
    colors[ImGuiCol_ButtonHovered]        = (ImVec4){0.15f, 0.15f, 0.18f, 1.00f};
    colors[ImGuiCol_Border]               = (ImVec4){0.12f, 0.12f, 0.15f, 0.50f};
    colors[ImGuiCol_Text]                 = (ImVec4){0.92f, 0.92f, 0.95f, 1.00f};
    colors[ImGuiCol_TextDisabled]         = (ImVec4){0.45f, 0.45f, 0.50f, 1.00f};
    
    printf("[Derect] Style Inited");
}