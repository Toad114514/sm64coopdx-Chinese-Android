#ifndef VAPE_UI_H
#define VAPE_UI_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// 初始化 Vape V4 样式与触屏参数适配
void derect_initStyle(void);

// 每帧渲染 Vape V4 主界面
void derect_panel_render(bool* open);

// 自定义模块 Toggle 控件接口 (纯 C 函数)
bool VapeUI_ModuleToggle(const char* label, bool* v, const char* desc);

#ifdef __cplusplus
}
#endif

#endif // VAPE_UI_H