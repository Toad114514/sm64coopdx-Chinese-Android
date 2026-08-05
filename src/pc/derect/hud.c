#ifndef CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#endif

#include "../cimgui/cimgui.h"
#include "hud.h"
#include "module.h"

void Derect_RenderHUD(void) {
    int count = 0;
    // moduless
    Module* modules = Module_GetAll(&count);
    ImGuiIO* io = igGetIO();
    
    igSetNextWindowPos((ImVec2){io->DisplaySize.x - 15.0f, 15.0f}, ImGuiCond_Always, (ImVec2){1.0f, 0.0f});
    
    // 2. HUD 必备窗口属性：无装饰、自动尺寸、无背景、不响应鼠标输入（透传到游戏）、不可移动
    ImGuiWindowFlags hud_flags = ImGuiWindowFlags_NoDecoration 
                               | ImGuiWindowFlags_AlwaysAutoResize 
                               | ImGuiWindowFlags_NoBackground 
                               | ImGuiWindowFlags_NoInputs 
                               | ImGuiWindowFlags_NoMove 
                               | ImGuiWindowFlags_NoSavedSettings 
                               | ImGuiWindowFlags_NoFocusOnAppearing;

    igBegin("##Vape_ArrayList_HUD", NULL, hud_flags);

    // 3. 遍历并渲染所有开启的模块
    for (int i = 0; i < count; i++) {
        if (modules[i].enabled) {
            igTextColored(modules[i].color, "%s", modules[i].name);
        }
    }

    igEnd();
}