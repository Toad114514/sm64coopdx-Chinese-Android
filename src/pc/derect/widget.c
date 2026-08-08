// Derect Client custom Vape-Like widgets
#ifndef CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#endif

#include "../cimgui/cimgui.h"
#include "widget.h"
#include "module.h"
#include "config.h"

// 渲染单个模块开关组件
// label: 模块名, active: 开启状态, shortcut: 快捷键文本(可为NULL), is_cyan: 是否使用青色高亮(如GUIBlur)
bool VapeUI_ModuleToggle(Module* mod) {
    if (!mod) return false;  // callback
    
    igPushID_Str(mod->name); // aloneID
    
    ////////// Active
    
    ImVec4 active_color = (ImVec4){0.00f, 0.65f, 0.60f, 1.00f};
    if (mod->color.w > 0.0f) {
        active_color = mod->color;
    }

    if (mod->enabled) {
        igPushStyleColor_Vec4(ImGuiCol_Header, active_color);
        igPushStyleColor_Vec4(ImGuiCol_HeaderHovered, (ImVec4){active_color.x * 1.1f, active_color.y * 1.1f, active_color.z * 1.1f, 1.0f});
    } else {
        igPushStyleColor_Vec4(ImGuiCol_Header, (ImVec4){1.0f, 1.0f, 1.0f, 1.0f});
        igPushStyleColor_Vec4(ImGuiCol_HeaderHovered, (ImVec4){0.14f, 0.14f, 0.18f, 0.80f});
    }
    
    igPushStyleVar_Vec2(ImGuiStyleVar_ItemSpacing, (ImVec2){0.0f, 6.0f});
    
    
    /////////nb Click
    bool clicked = igSelectable_Bool(mod->name, mod->enabled, ImGuiSelectableFlags_None, (ImVec2){0.0f, 30.0f});
    if (clicked) {
        mod->enabled = !(mod->enabled);
        if (mod->enabled) {
            if (mod->on_enable) mod->on_enable();
        } else {
            if (mod->on_disable) mod->on_disable();
        }
        
        Config_Save("/storage/emulated/0/com.toad1145.derectcoopdxcn/config.ini");
    }
    
    float window_w = igGetWindowWidth();
    igSameLine(window_w - 12.0f, 0.0f);
    
    // Popups
    //igSameLine(0.0f, 4.0f);
    ImGuiDir arrow_dir = mod->expanded ? ImGuiDir_Down : ImGuiDir_Right;
    if (igArrowButton("##expandBtn", arrow_dir)) {
        mod->expanded = !mod->expanded;
    }

    // 4. 【核心逻辑】如果处于展开状态且有配置项，直接在下方内嵌渲染
    if (mod->expanded && mod->config) {
        igSpacing();
        
        igIndent(12.0f);
        igBeginGroup();
        igPushStyleColor_Vec4(ImGuiCol_ChildBg, (ImVec4){0.1f, 0.1f, 0.1f, 0.5f});
        
        // 渲染模块自带的配置项（Slider/Combo/Checkbox 等）
        mod->config();

        igPopStyleColor(1);
        igEndGroup();

        igUnindent(12.0f);
        igSpacing();
        igSeparator();
    }
    
    igPopStyleVar(1);
    igPopStyleColor(2);

    igPopID();
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
        (ImVec2){290.0f, -1.0f},
        (ImVec2){290.0f, -1.0f},
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
                VapeUI_ModuleToggle(&modules[i]);
            }
        }
    }

    igEnd();
}