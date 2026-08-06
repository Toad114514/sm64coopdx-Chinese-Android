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
    if (!label || !active) return false;  // callback
    
    ImVec2 avail;
    igGetContentRegionAvail();
    ImVec2 item_size = (ImVec2){avail.x, 24.0f};

    // 设置激活时的背景色（鲜绿色 或 青色）
    ImVec4 active_color = is_cyan 
        ? (ImVec4){0.00f, 0.65f, 0.60f, 1.00f}   // 青色 (Teal)
        : (ImVec4){0.10f, 0.70f, 0.20f, 1.00f};  // 绿色 (Green)
    

    if (*active) {
        igPushStyleColor_Vec4(ImGuiCol_Header, active_color);
        igPushStyleColor_Vec4(ImGuiCol_HeaderHovered, (ImVec4){active_color.x * 1.1f, active_color.y * 1.1f, active_color.z * 1.1f, 1.0f});
    } else {
        igPushStyleColor_Vec4(ImGuiCol_Header, (ImVec4){1.0f, 1.0f, 1.0f, 1.0f});
        igPushStyleColor_Vec4(ImGuiCol_HeaderHovered, (ImVec4){0.14f, 0.14f, 0.18f, 0.80f});
    }
    
    igPushStyleVar_Vec2(ImGuiStyleVar_ItemSpacing, (ImVec2){0.0f, 6.0f});

    // 绘制可点击区域
    bool clicked = igSelectable_Bool(label, *active, ImGuiSelectableFlags_None, (ImVec2){0.0f, 30.0f});
    if (clicked) {
        *active = !(*active);
    }

    // 3. 靠右绘制快捷键与冒号（使用安全窗口相对坐标）
    float window_w = igGetWindowWidth();
    if (shortcut && shortcut[0] != '\0') {
        igSameLine(window_w - 80.0f, 0.0f);
        igTextDisabled("%s", shortcut);
        igSameLine(window_w - 18.0f, 0.0f);
        igTextDisabled(":");
    } else {
        igSameLine(window_w - 18.0f, 0.0f);
        igTextDisabled(":");
    }

    igPopStyleColor(2);
    return clicked;
}

// 单分类面板动态渲染
void VapeUI_RenderCategoryPanel(ModuleCategory target_cat, ImVec2 pos) {
    int count = 0;
    Module* modules = Module_GetAll(&count);
    
    if (!modules || count <= 0) return; // if not then fuck

    igSetNextWindowPos(pos, ImGuiCond_FirstUseEver, (ImVec2){0, 0});
    // igSetNextWindowSize((ImVec2){200, 400}, ImGuiCond_FirstUseEver);
    igSetNextWindowSizeConstraints(
        (ImVec2){220.0f, -1.0f},
        (ImVec2){220.0f, -1.0f},
        NULL, NULL
    );
    const char* cat_name = Module_GetCategoryName(target_cat);
    
    // flags
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;
    bool visible = igBegin(cat_name, NULL, flags);
    
    // 自定义分类 Header
    if (visible) {
        // 自定义 Header
        
        igTextDisabled("^");
        igSameLine(0, 6);
        igText("%s", cat_name);

        float window_w = igGetWindowWidth();
        igSameLine(window_w - 20.0f, 0);
        igTextDisabled("^");

        igSeparator();
        igDummy((ImVec2){0.0f, 2.0f});

        // 遍历当前分类下的模块
        for (int i = 0; i < count; i++) {
            if (modules[i].category == target_cat && modules[i].name != NULL) {
                VapeUI_ModuleToggle(
                    modules[i].name,
                    &modules[i].enabled,
                    modules[i].shortcut,
                    modules[i].is_cyan
                );
            }
        }
    }

    igEnd();
}