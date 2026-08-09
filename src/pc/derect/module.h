#ifndef MODULE_REGISTRY_H
#define MODULE_REGISTRY_H

#ifndef CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#endif

#include "../cimgui/cimgui.h"

// 模块颜色选择
#define MOD_COLOR_RED   ((ImVec4){1.0f,  0.20f, 0.12f, 1.0f})
#define MOD_COLOR_GREEN ((ImVec4){0.10f, 0.85f, 0.30f, 1.0f})
#define MOD_COLOR_BLUE  ((ImVec4){0.00f, 0.20f, 1.0f,  1.0f})
#define MOD_COLOR_CYAN  ((ImVec4){0.00f, 0.75f, 0.70f, 1.0f})
#define MOD_COLOR_PINK  ((ImVec4){1.0f,  0.00f, 0.78f, 1.0f})

// 模块分类定义
typedef enum {
    CAT_CORE = 0,
    CAT_MARIO,
    CAT_RENDER,
    CAT_CONTROL,
    CAT_WEB,
    CAT_MISC,
    CAT_DEMO,
    CAT_COUNT
} ModuleCategory;

typedef void (*ModuleCallBack)(void);

// 单个模块数据结构
typedef struct {
    const char* name;        // 模块名称
    ModuleCategory category; // 所属分类
    bool enabled;            // 开启状态
    bool expanded;           // 展开/关闭
    char shortcut[32];       // 快捷键文本 (如 "LAlt+K"，无则为空串)
    ImVec4 color;            // HUD 显色

    int bind_key;            // 解析后的触发键 (ImGuiKey，无绑定为 ImGuiKey_None)
    int bind_mods;           // 解析后的修饰键位掩码 (ImGuiMod_*)

    ModuleCallBack on_enable;
    ModuleCallBack on_disable;
    ModuleCallBack on_loop;
    ModuleCallBack on_render;
    
    ModuleCallBack config;
} Module;

#define MAX_MODULES 128
extern Module g_modules[MAX_MODULES];
extern int g_module_count;

// API 声明
void Module_InitRegistry(void);
void Module_Register(const char* name, ModuleCategory category, bool default_enabled, const char* shortcut, ImVec4 color, ModuleCallBack on_enable, ModuleCallBack on_disable, ModuleCallBack on_loop);
void Module_HookRender(const char* name, ModuleCallBack on_render);
void Module_HookConfig(const char* name, ModuleCallBack config);

Module* Module_GetAll(int* out_count);
Module* Module_Find(const char* name);
const char* Module_GetCategoryName(ModuleCategory cat);

// callback
void Module_Update(void);
void Module_Render(void);

// 快捷键
void Module_Toggle(Module* mod);
void Module_HandleShortcuts(void);
void Module_BeginKeybind(Module* mod);
void Module_CancelKeybind(void);
Module* Module_GetBindingModule(void);
void Module_SetShortcut(Module* mod, const char* shortcut);
void Module_FormatShortcut(const Module* mod, char* buf, size_t buf_size);

#endif