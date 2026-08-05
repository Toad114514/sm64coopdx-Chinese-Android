// Derect Client custom Vape-Like widgets
#ifndef CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#endif

#include "../cimgui/cimgui.h"
#include "widget.h"
#include "module.h"

// 渲染单个模块开关组件
// label: 模块名, active: 开启状态, shortcut: 快捷键文本(可为NULL), is_cyan: 是否使用青色高亮(如GUIBlur)
bool VapeUI_ModuleToggle(const char* label, bool* active, const char* shortcut, bool is_cyan) {
    ImVec2 avail;
    igGetContentRegionAvail(&avail);
    ImVec2 item_size = (ImVec2){avail.x, 24.0f};

    // 设置激活时的背景色（鲜绿色 或 青色）
    ImVec4 active_color = is_cyan 
        ? (ImVec4){0.00f, 0.65f, 0.60f, 1.00f}   // 青色 (Teal)
        : (ImVec4){0.10f, 0.70f, 0.20f, 1.00f};  // 绿色 (Green)

    if (*active) {
        igPushStyleColor_Vec4(ImGuiCol_Header, active_color);
        igPushStyleColor_Vec4(ImGuiCol_HeaderHovered, (ImVec4){active_color.x * 1.1f, active_color.y * 1.1f, active_color.z * 1.1f, 1.0f});
    } else {
        igPushStyleColor_Vec4(ImGuiCol_Header, (ImVec4){0.07f, 0.07f, 0.09f, 0.60f});
        igPushStyleColor_Vec4(ImGuiCol_HeaderHovered, (ImVec4){0.14f, 0.14f, 0.18f, 0.80f});
    }

    // 绘制可点击区域
    bool clicked = igSelectable_Bool(label, *active, ImGuiSelectableFlags_None, item_size);
    if (clicked) {
        *active = !(*active);
    }

    // 绘制右侧的快捷键和三点菜单按钮 (:)
    float start_x = igGetItemRectMin().x;
    float end_x = igGetItemRectMax().x;
    float top_y = igGetItemRectMin().y + 4.0f;

    if (shortcut && shortcut[0] != '\0') {
        ImVec2 sc_size;
        igCalcTextSize(&sc_size, shortcut, NULL, false, -1.0f);
        igSetCursorScreenPos((ImVec2){end_x - sc_size.x - 18.0f, top_y});
        igTextDisabled("%s", shortcut);
    }

    // 右侧三点图标
    igSetCursorScreenPos((ImVec2){end_x - 12.0f, top_y});
    igTextDisabled(":");

    igPopStyleColor(2);
    return clicked;
}

// 单分类面板动态渲染
void VapeUI_RenderCategoryPanel(ModuleCategory target_cat, ImVec2 pos) {
    int count = 0;
    Module* modules = Module_GetAll(&count);

    igSetNextWindowPos(pos, ImGuiCond_FirstUseEver, (ImVec2){0, 0});
    igSetNextWindowSize((ImVec2){200, 400}, ImGuiCond_FirstUseEver);

    const char* cat_name = Module_GetCategoryName(target_cat);
    
    igBegin(cat_name, NULL, ImGuiWindowFlags_NoTitleBar);

    // 自定义分类 Header
    igTextDisabled("^"); igSameLine(0, 5);
    igText("%s", cat_name);
    igSeparator();

    // 动态渲染属于当前分类的模块
    for (int i = 0; i < count; i++) {
        if (modules[i].category == target_cat) {
            VapeUI_ModuleToggle(
                modules[i].name, 
                &modules[i].enabled, 
                modules[i].shortcut, 
                modules[i].is_cyan
            );
        }
    }

    igEnd();
}