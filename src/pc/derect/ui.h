#ifndef VAPE_UI_H
#define VAPE_UI_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// 每帧渲染 Vape V4 主界面
void derect_panel_render(bool* open);

#ifdef __cplusplus
}
#endif

#endif // VAPE_UI_H