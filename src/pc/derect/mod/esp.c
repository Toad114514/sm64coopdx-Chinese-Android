#include <math.h>
#include <stdbool.h>

#ifndef CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#endif
#include "../../cimgui/cimgui.h"

#include "game/camera.h"
#include "game/level_update.h"
#include "game/mario.h"
#include "pc/network/network_player.h"
#include "../module.h"

#define DEG2RAD(angle) ((angle) * 3.14159265358979323846f / 180.0f)

typedef struct {
    float x, y;
} ESPScrPos;

////////
// 3d -> 2d Pos

// $$X_{ndc} = \frac{X_{cam} \cdot \cot(\frac{FOV}{2})}{Z_{cam} \cdot Aspect}, \quad Y_{ndc} = \frac{Y_{cam} \cdot \cot(\frac{FOV}{2})}{Z_{cam}}$$


bool WorldToScreen(const float worldPos[3], ESPScrPos* screenPos) {
    ImGuiIO* io = igGetIO();
    float screenWidth = io->DisplaySize.x;
    float screenHeight = io->DisplaySize.y;

    // 1. 获取 Lakitu Pos
    float camX = gLakituState.curPos[0];
    float camY = gLakituState.curPos[1];
    float camZ = gLakituState.curPos[2];

    float focusX = gLakituState.curFocus[0];
    float focusY = gLakituState.curFocus[1];
    float focusZ = gLakituState.curFocus[2];

    // 2. Forward Vector 前向量
    //       Pfocus - Pcam
    // F = -------------------
    //     || Pfocus - Pcam || 
    float forwardX = focusX - camX;
    float forwardY = focusY - camY;
    float forwardZ = focusZ - camZ;
    float fLen = sqrtf(forwardX * forwardX + forwardY * forwardY + forwardZ * forwardZ);
    if (fLen < 0.0001f) return false;
    forwardX /= fLen; forwardY /= fLen; forwardZ /= fLen;

    // 3. Right Vec 右向量
    //       F x (0, 1, 0)
    // R = ------------------
    //     || F x (0, 1, 0) ||
    float rightX = -forwardZ;
    float rightY = 0.0f;
    float rightZ = forwardX;
    float rLen = sqrtf(rightX * rightX + rightZ * rightZ);
    if (rLen < 0.0001f) return false;
    rightX /= rLen; rightZ /= rLen;

    // 4. 上向量
    //  U = R x F
    float upX = rightY * forwardZ - rightZ * forwardY;
    float upY = rightZ * forwardX - rightX * forwardZ;
    float upZ = rightX * forwardY - rightY * forwardX;

    // 5. World Pos 偏移
    float relX = worldPos[0] - camX;
    float relY = worldPos[1] - camY;
    float relZ = worldPos[2] - camZ;

    // 6. 向量投影
    float camSpaceX = relX * rightX + relY * rightY + relZ * rightZ;
    float camSpaceY = relX * upX + relY * upY + relZ * upZ;
    float camSpaceZ = relX * forwardX + relY * forwardY + relZ * forwardZ;

    // cap 物品在背后，别画
    if (camSpaceZ <= 10.0f) {
        return false;
    }

    // 7. 相机空间 -> NDC
    float fov = 45.0f; // 默认45，目前无办法获取 Lakitu Fov
    // if (fov <= 0.0f) fov = 45.0f; // 默认 45 度视角
    // focalLength = 1/tan(fov/2)
    float fl = 1.0f / tanf(DEG2RAD(fov * 0.5f));
    // 屏幕比例
    float aspect = screenWidth / screenHeight;
    
    // NDCX and NDCY
    float ndcX = (camSpaceX * fl) / (camSpaceZ * aspect);
    float ndcY = (camSpaceY * fl) / camSpaceZ;

    // 8. 最后视口转换完成，ScreenPosX and ScreenPosY
    screenPos->x = (screenWidth * 0.5f) * (1.0f + ndcX);
    screenPos->y = (screenHeight * 0.5f) * (1.0f - ndcY);

    return true;
}

/// World -> screen done


#define IM_COL32(R, G, B, A) (((ImU32)(A) << 24) | ((ImU32)(B) << 16) | ((ImU32)(G) << 8) | (ImU32)(R))

// Mario_ESP 透视
void Mario_ESP_Render(void) {
    ImDrawList* drawList = igGetForegroundDrawList_ViewportPtr(NULL);
    ImGuiIO* io = igGetIO();
    float screenWidth = io->DisplaySize.x;
    float screenHeight = io->DisplaySize.y;
    
    for (int i = 0; i < MAX_PLAYERS; i++) {
        struct MarioState* m = &gMarioStates[i];
        struct NetworkPlayer* np = &gNetworkPlayers[i];
        
        if (!m || !m->marioObj || !np->connected ) continue;

        // 脚底和头顶
        float bottomWorld[3] = { m->pos[0], m->pos[1], m->pos[2] };
        float topWorld[3]    = { m->pos[0], m->pos[1] + 160.0f, m->pos[2] }; // 马尿身高160

        ESPScrPos bottomScreen, topScreen;

        // 坐标转换
        if (WorldToScreen(bottomWorld, &bottomScreen) && WorldToScreen(topWorld, &topScreen)) {
            // 根据转换后的 2D 高度计算方框宽度
            float boxHeight = bottomScreen.y - topScreen.y;
            float boxWidth  = boxHeight * 0.5f; // 宽高比为 1:2

            float x1 = topScreen.x - boxWidth * 0.5f;
            float y1 = topScreen.y;
            float x2 = topScreen.x + boxWidth * 0.5f;
            float y2 = bottomScreen.y;
            
            // if NaN then fuck you off
            if (isnan(x1) || isnan(y1) || isnan(x2) || isnan(y2) ||
                isinf(x1) || isinf(y1) || isinf(x2) || isinf(y2)) {
               return;
            }

            // gMarioStates[0] 本人为 0
            ImU32 color = (i == 0) ? IM_COL32(255, 255, 0, 255) : IM_COL32(255, 0, 135, 255);
            
            ImVec2 pm = { x1, y1 };
            ImVec2 px = { x2, y2 };
            
            // 框
            ImDrawList_AddRect(drawList, pm, px, color, 0.0f, 1.5f, 0);

            // 看我跟踪。
            ImDrawList_AddLine(
                drawList,
                (ImVec2){screenWidth * 0.5f, screenHeight},
                (ImVec2){bottomScreen.x, bottomScreen.y},
                color,
                1.5f
            );

            // 看我开户。
            char infoText[64];
            snprintf(infoText, sizeof(infoText), "%s | HP %d", np->name, m->health);
            ImDrawList_AddText_Vec2(
                drawList,
                (ImVec2){x1, y1 - 16.0f},
                IM_COL32(255, 255, 255, 255),
                infoText,
                NULL
            );
        }
    }
}

// Module Register
void module_esp(void){
    Module_Register("PlayerESP", CAT_RENDER, false, NULL, MOD_COLOR_GREEN, NULL, NULL, NULL);
    Module_HookRender("PlayerESP", Mario_ESP_Render);
}