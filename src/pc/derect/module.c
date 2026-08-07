#include "module.h"
#include <stddef.h>

#define MAX_MODULES 128

static Module g_modules[MAX_MODULES];
static int g_module_count = 0;

static const char* g_category_names[] = {
    "File", "Mario", "Render", "Control", "Coopnet/Web", "Misc", "Demo"
};

const char* Module_GetCategoryName(ModuleCategory cat) {
    if (cat >= 0 && cat < CAT_COUNT) {
        return g_category_names[cat];
    }
    return "Unknown";
}

// 注册新模块的公共方法
void Module_Register(const char* name, ModuleCategory category, bool default_enabled, const char* shortcut, ImVec4 color, ModuleCallBack on_enable, ModuleCallBack on_disable, ModuleCallBack on_loop) {
    if (g_module_count >= MAX_MODULES) return;

    g_modules[g_module_count] = (Module){
        .name = name,
        .category = category,
        .enabled = default_enabled,
        .shortcut = shortcut,
        .color = color,
        .on_enable = on_enable,
        .on_disable = on_disable,
        .on_loop = on_loop,
        .on_render = NULL,
    };
    g_module_count++;
    
    printf("[Derect] Resigned Module %s \n", name);
}

void Module_HookRender(const char* name, ModuleCallBack on_render) {
    int count = sizeof(g_modules) / sizeof(g_modules[0]);
    for (int i = 0; i < count; i++) {
        if (name == g_modules[i].name) {
            g_modules[i].on_render = on_render;
            printf("[Derect] Resigned Module %s on_render hook", name);
        }
    }
}


// Module text
static void printf_test_on_enable(){
    printf("[Test] test enable");
}

static void printf_test_on_disable(){
    printf("[Test] Bye");
}

static void printf_test_good_work(){
    printf("[Test] HolyMoly");
}

static bool demo_close_btn = true;

static void demo_loop(){
    igShowDemoWindow(&demo_close_btn);
}

static void demo_close(){
    demo_close_btn = false;
}

static void demo_show() {
    demo_close_btn = true;
}

// 统一在此处注册所有游戏/应用模块
void Module_InitRegistry(void) {
    g_module_count = 0;

    // 预设常用颜色
    ImVec4 green = (ImVec4){0.10f, 0.85f, 0.30f, 1.0f};
    ImVec4 cyan  = (ImVec4){0.00f, 0.75f, 0.70f, 1.0f};

    // ==================== 1. Render 分类 ====================
    Module_Register("Arraylist",       CAT_RENDER, true,  NULL,       green, NULL, NULL, NULL);
    Module_Register("AudioVisualizer", CAT_RENDER, true,  NULL,       green, NULL, NULL, NULL);
    Module_Register("BlackCapture",    CAT_RENDER, false, "LCtrl+NO", green, NULL, NULL, NULL);
    Module_Register("Keystrokes",      CAT_RENDER, false, "LAlt+K",   green, NULL, NULL, NULL);
    Module_Register("Background",      CAT_RENDER, true,  NULL,       cyan,  NULL, NULL, NULL);
    Module_Register("GUIBlur",         CAT_RENDER, true,  NULL,       cyan,  NULL, NULL, NULL);
    Module_Register("这是中文合成效果",   CAT_RENDER, false, NULL,       cyan,  NULL, NULL, NULL);

    // ==================== 2. Web 分类 ====================
    Module_Register("AntiRickroll",    CAT_WEB,    true,  NULL,       green, NULL, NULL, NULL);
    Module_Register("LiveStream",      CAT_WEB,    true,  NULL,       green, NULL, NULL, NULL);
    Module_Register("QuakeWarning",    CAT_WEB,    true,  NULL,       green, NULL, NULL, NULL);
    Module_Register("BiliFans",        CAT_WEB,    true,  NULL,       green, NULL, NULL, NULL);

    // ==================== 3. Misc 分类 ====================
    Module_Register("AutoSpeak",       CAT_MISC,   true,  NULL,       green, NULL, NULL, NULL);
    Module_Register("MemeTrigger",     CAT_MISC,   true,  NULL,       green, NULL, NULL, NULL);
    Module_Register("Volume",          CAT_MISC,   true,  "RAlt+Del", green, NULL, NULL, NULL);
    
    // Demo Sections
    Module_Register("Notification",    CAT_DEMO,   false, NULL,       green, NULL, NULL, NULL);
    Module_Register("Function Printf", CAT_DEMO,   false, NULL,       green, printf_test_on_enable, printf_test_on_disable, printf_test_good_work);

    Module_Register("ImGUI Demo",      CAT_DEMO,   false, NULL,       green, demo_show, demo_close, demo_loop);
    Module_HookRender("ImGUI Demo",    demo_loop);
}
    

// 获取已注册模块列表指针
Module* Module_GetAll(int* out_count) {
    if (out_count) *out_count = g_module_count;
    return g_modules;
}

// will call on game/game_init.c
void Module_Update(void) {
    int count = sizeof(g_modules) / sizeof(g_modules[0]);
    for (int i = 0; i < count; i++) {
        if (g_modules[i].enabled && g_modules[i].on_loop) {
            g_modules[i].on_loop();
        }
    }
}

// will call on derect/ui Render
void Module_Render(void) {
    int count = sizeof(g_modules) / sizeof(g_modules[0]);
    for (int i = 0; i < count; i++) {
        if (g_modules[i].enabled && g_modules[i].on_render) {
            g_modules[i].on_render();
        }
    }
}
