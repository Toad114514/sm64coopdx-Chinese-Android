#include "types.h"
#include "game/mario.h"
#include "game/object_helpers.h"
#include "game/level_update.h"          // 包含 gMarioStates 声明
#include "game/object_list_processor.h"
#include "engine/behavior_script.h"
#include "behavior_data.h"
#include "engine/math_util.h"
#include "audio/external.h"
#include "object_fields.h"
#include "sm64.h"

#include "../module.h"

// 自动打库巴使能开关
static bool g_auto_bowser_enabled = true;

// 找最近的指定 Behavior 结构体对象
static struct Object* find_nearest_behavior(struct Object *parent, const BehaviorScript *behavior) {
    uintptr_t *behaviorAddr = (uintptr_t *)behavior;
    struct Object *nearest = NULL;
    f32 minDst = 999999.0f;

    struct Object *obj = (struct Object *)gObjectListArray[OBJ_LIST_GENACTOR].next;
    while (obj && obj != (struct Object *)&gObjectListArray[OBJ_LIST_GENACTOR]) {
        if (obj->behavior == behaviorAddr && obj->activeFlags != ACTIVE_FLAG_DEACTIVATED) {
            f32 dx = obj->oPosX - parent->oPosX;
            f32 dy = obj->oPosY - parent->oPosY;
            f32 dz = obj->oPosZ - parent->oPosZ;
            f32 dist = sqrtf(dx * dx + dy * dy + dz * dz);
            if (dist < minDst) {
                minDst = dist;
                nearest = obj;
            }
        }
        obj = (struct Object *)obj->header.next;
    }
    return nearest;
}

void autobowser_loop(void) {
    struct MarioState* m = &gMarioStates[0];
    if (!g_auto_bowser_enabled || !m) return;

    // 1. 寻找场景中的库巴
    struct Object *bowser = find_nearest_behavior(m->marioObj, bhvBowser);
    if (!bowser) return;

    // 状态 A：尚未抓到库巴尾巴 (近身 & 自动抓尾)
    if (m->action != ACT_HOLDING_BOWSER) {
        // 计算库巴尾巴的相对位置 (库巴后方约 220 单位)
        s16 bowserYaw = bowser->oFaceAngleYaw;
        f32 tailX = bowser->oPosX - sins(bowserYaw) * 220.0f;
        f32 tailZ = bowser->oPosZ - coss(bowserYaw) * 220.0f;
        f32 tailY = bowser->oPosY;

        // 将马里奥平滑吸附到尾巴坐标
        m->pos[0] = tailX;
        m->pos[1] = tailY;
        m->pos[2] = tailZ;
        m->faceAngle[1] = bowserYaw; // 面对库巴

        // 迫使库巴进入被抓取状态/判定碰撞
        // if (m->controller) {
            //m->controller->buttonPressed |= B_BUTTON; // 触发按 B 抓取
            //m->controller->buttonDown |= B_BUTTON;
        // }
        // return;
    }

    // 状态 B：已经抓到库巴 (进入旋转与瞄准逻辑)
    if (m->action == ACT_HOLDING_BOWSER) {
        
        // 1. 查找距离最近的炸弹 (bhvBowserBomb)
        struct Object *bomb = find_nearest_behavior(m->marioObj, bhvBowserBomb);

        // 2. 持续增加转速 (Spinning)
        if (m->angleVel[1] < 0x800) {
            m->angleVel[1] += 0x80; // 快速加速旋转
        }
        m->faceAngle[1] += m->angleVel[1];

        // 3. 如果找到了炸弹，计算目标角度并精准出手
        if (bomb) {
            // 计算马里奥到炸弹的绝对夹角
            f32 dx = bomb->oPosX - m->pos[0];
            f32 dz = bomb->oPosZ - m->pos[2];
            s16 targetYaw = atan2s(dz, dx);

            // 计算当前旋转角度与目标炸弹角度的残差
            s16 angleDiff = m->faceAngle[1] - targetYaw;

            // 家家家。
            if (m->angleVel[1] >= 0x1200 && absi(angleDiff) < 0x1200) {
                set_mario_action(m, ACT_RELEASING_BOWSER, 0);
                // 飞飞飞
                m->forwardVel = 260.0f;
                
                //play_mario_sound(m, SOUND_ACTION_TERRAIN_JUMP, MARIO_SOUND_SO_LONG_BOWSER);
                play_character_sound(m, CHAR_SOUND_SO_LONGA_BOWSER);
            }
        } else {
            // 如果没找到炸弹（比如阶段转换中），保持高速度旋转盲甩
            if (m->angleVel[1] >= 0x1800) {
                set_mario_action(m, ACT_RELEASING_BOWSER, 0);
                m->forwardVel = 220.0f;
                play_character_sound(m, CHAR_SOUND_SO_LONGA_BOWSER);
            }
        }
    }
}

void module_autobowser(void) {
    Module_Register("AutoFuckBowser", CAT_CONTROL, false, NULL, MOD_COLOR_RED, NULL, NULL, autobowser_loop);
}