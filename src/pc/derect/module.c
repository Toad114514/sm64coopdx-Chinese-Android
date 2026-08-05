#include "module.h"
#include <stddef.h>

#define MAX_MODULES 128

static Module g_modules[MAX_MODULES];
static int g_module_count = 0;

static const char* g_category_names[] = {
    "File", "Render", "Control", "Web", "Misc", "Drivers"
};

const char* Module_GetCategoryName(ModuleCategory cat) {
    if (cat >= 0 && cat < CAT_COUNT) {
        return g_category_names[cat];
    }
    return "Unknown";
}

// 注册新模块的公共方法
void Module_Register(const char* name, ModuleCategory category, bool default_enabled, const char* shortcut, ImVec4 color, bool is_cyan) {
    if (g_module_count >= MAX_MODULES) return;

    g_modules[g_module_count] = (Module){
        .name = name,
        .category = category,
        .enabled = default_enabled,
        .shortcut = shortcut,
        .color = color,
        .is_cyan = is_cyan
    };
    g_module_count++;
}

// 统一在此处注册所有游戏/应用模块
void Module_InitRegistry(void) {
    g_module_count = 0;

    // 预设常用颜色
    ImVec4 green = (ImVec4){0.10f, 0.85f, 0.30f, 1.0f};
    ImVec4 cyan  = (ImVec4){0.00f, 0.75f, 0.70f, 1.0f};

    // ==================== 1. Render 分类 ====================
    Module_Register("Arraylist",       CAT_RENDER, true,  NULL,       green, false);
    Module_Register("AudioVisualizer", CAT_RENDER, true,  NULL,       green, false);
    Module_Register("BlackCapture",    CAT_RENDER, false, "LCtrl+NO", green, false);
    Module_Register("Keystrokes",      CAT_RENDER, false, "LAlt+K",   green, false);
    Module_Register("Background",      CAT_RENDER, true,  NULL,       cyan,  true);
    Module_Register("GUIBlur",         CAT_RENDER, true,  NULL,       cyan,  true);

    // ==================== 2. Web 分类 ====================
    Module_Register("AntiRickroll",    CAT_WEB,    true,  NULL,       green, false);
    Module_Register("LiveStream",      CAT_WEB,    true,  NULL,       green, false);
    Module_Register("QuakeWarning",    CAT_WEB,    true,  NULL,       green, false);
    Module_Register("BiliFans",        CAT_WEB,    true,  NULL,       green, false);

    // ==================== 3. Misc 分类 ====================
    Module_Register("AutoSpeak",       CAT_MISC,   true,  NULL,       green, false);
    Module_Register("MemeTrigger",     CAT_MISC,   true,  NULL,       green, false);
    Module_Register("Volume",          CAT_MISC,   true,  "RAlt+Del", green, false);
}

// 获取已注册模块列表指针
Module* Module_GetAll(int* out_count) {
    if (out_count) *out_count = g_module_count;
    return g_modules;
}