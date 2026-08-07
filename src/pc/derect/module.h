#ifndef MODULE_REGISTRY_H
#define MODULE_REGISTRY_H

#ifndef CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#endif

#include "../cimgui/cimgui.h"

// 模块分类定义
typedef enum {
    CAT_FILE = 0,
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
    const char* shortcut;    // 快捷键提示 (无则为 NULL)
    ImVec4 color;            // HUD 显色
    
    ModuleCallBack on_enable;
    ModuleCallBack on_disable;
    ModuleCallBack on_loop;
    ModuleCallBack on_render;
} Module;

// API 声明
void Module_InitRegistry(void);
void Module_Register(const char* name, ModuleCategory category, bool default_enabled, const char* shortcut, ImVec4 color, ModuleCallBack on_enable, ModuleCallBack on_disable, ModuleCallBack on_loop);
void Module_HookRender(const char* name, ModuleCallBack on_render);

Module* Module_GetAll(int* out_count);
const char* Module_GetCategoryName(ModuleCategory cat);

// callback
void Module_Update(void);
void Module_Render(void);

#endif