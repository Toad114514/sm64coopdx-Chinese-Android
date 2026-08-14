#include <string.h>

#include "game/mario.h"
#include "game/level_update.h"
#include "surface_terrains.h"
#include "engine/surface_collision.h"
#include "types.h"
#include "sm64.h"
//#include "PR/os_cont.h"
#include "../module.h"

#include "../config.h"

ImVec4 green = (ImVec4){0.10f, 0.85f, 0.30f, 1.0f};

// Mario God Mode
static bool god_lockHealth = true;
static bool god_invc = false;
static int  god_healBack = 4;

static const ConfigOption s_god_mode_option[] = {
    BIND_BOOL ("lockhealth",     "Lock Player Health",     &god_lockHealth),
    BIND_BOOL ("invinc",         "Invinc Player",          &god_invc),
    BIND_INT  ("healcount",      "Health Counter",         &god_healBack,    0,   8,  "%d")
};
#define GMOD_COUNT (sizeof(s_god_mode_option) / sizeof(s_god_mode_option[0]))

static void god_mode_loop(void) {
    struct MarioState* m = &gMarioStates[0];
    if (!m || !m->marioObj) return;

    if (god_lockHealth) m->health = 0x0880;
    if (god_invc) m->invincTimer = 30;
    m->healCounter = god_healBack;
}

static void god_mode_disable(void) {
    struct MarioState* m = &gMarioStates[0];
    if (m) {
        m->invincTimer = 0;
    }
}

static void god_op(void) {
    Config_RenderOptions(s_god_mode_option, GMOD_COUNT);
}

// Mario Cap Status
static void inf_wing_cap_loop(void) {
    struct MarioState* m = &gMarioStates[0];
    if (m) {
        m->flags |= MARIO_WING_CAP | MARIO_CAP_ON_HEAD;
        m->capTimer = 678;
    }
}

static void inf_wing_cap_disable(void) {
    struct MarioState* m = &gMarioStates[0];
    if (m) {
        m->flags &= ~MARIO_WING_CAP;
        m->capTimer = 0;
    }
}

static void inf_metal_cap_loop(void) {
    struct MarioState* m = &gMarioStates[0];
    if (m) {
        m->flags |= MARIO_METAL_CAP | MARIO_CAP_ON_HEAD;
        m->capTimer = 678;
    }
}

static void inf_metal_cap_disable(void) {
    struct MarioState* m = &gMarioStates[0];
    if (m) {
        m->flags &= ~MARIO_METAL_CAP;
        m->capTimer = 0;
    }
}

static void inf_vanish_cap_loop(void) {
    struct MarioState* m = &gMarioStates[0];
    if (m) {
        m->flags |= MARIO_VANISH_CAP | MARIO_CAP_ON_HEAD;
        m->capTimer = 678;
    }
}

static void inf_vanish_cap_disable(void) {
    struct MarioState* m = &gMarioStates[0];
    if (m) {
        m->flags &= ~MARIO_VANISH_CAP;
        m->capTimer = 0;
    }
}

/// AntiDeathFloor

static bool adf_lavas = true;
static bool adf_slipper = true;
static bool adf_death_plane = false;
static bool adf_all = false;

static const ConfigOption s_anti_death_floor_config[] = {
    BIND_BOOL ("lavas",       "Fuck Lava/QuickSand",     &adf_lavas),
    BIND_BOOL ("slipper",     "Fuck Ice/Slippery",       &adf_slipper),
    BIND_BOOL ("death_plane", "Fuck DEATH_PLANE",        &adf_death_plane),
    BIND_BOOL ("all",         "Fuck All",                &adf_all)
};
#define ADF_COUNT (sizeof(s_anti_death_floor_config) / sizeof(s_anti_death_floor_config[0]))

static void adf_config(void) {
    Config_RenderOptions(s_anti_death_floor_config, ADF_COUNT);
}

// jsksksmmdmdlx check
static bool adf_is_dangerous(s16 type) {
    // 岩浆 / 流沙
    if (adf_lavas) {
        switch (type) {
            case SURFACE_BURNING:
            case SURFACE_DEEP_QUICKSAND:
            case SURFACE_SHALLOW_QUICKSAND:
            case SURFACE_QUICKSAND:
            case SURFACE_MOVING_QUICKSAND:
            case SURFACE_SHALLOW_MOVING_QUICKSAND:
            case SURFACE_DEEP_MOVING_QUICKSAND:
            case SURFACE_INSTANT_QUICKSAND:
            case SURFACE_INSTANT_MOVING_QUICKSAND:
                return true;
            default:
                break;
        }
    }
    
    // 滑地
    if (adf_slipper) {
        switch (type) {
            case SURFACE_SLIPPERY:
            case SURFACE_VERY_SLIPPERY:
            case SURFACE_ICE:
            case SURFACE_HARD_SLIPPERY:
            case SURFACE_HARD_VERY_SLIPPERY:
            case SURFACE_NOISE_SLIPPERY:
            case SURFACE_NOISE_VERY_SLIPPERY:
            case SURFACE_NO_CAM_COL_SLIPPERY:
            case SURFACE_NO_CAM_COL_VERY_SLIPPERY:
                return true;
            default:
                break;
        }
    }

    // 陈死亡区域
    if (adf_death_plane) {
        switch (type) {
            case SURFACE_DEATH_PLANE:
            case SURFACE_VERTICAL_WIND:
                return true;
            default:
                break;
        }
    }

    return false;
}

// patch
static void adf_patch_surface(struct Surface* floor) {
    if (!floor) return;
    if (floor->type == SURFACE_DEFAULT) return;

    if (adf_all || adf_is_dangerous(floor->type)) {
        floor->type = SURFACE_DEFAULT;
    }
}

// 在物理更新(读取 m->floor)之前替换危险地面
static void adf_loop(void) {
    struct MarioState* m = &gMarioStates[0];
    if (!m) return;
    
    adf_patch_surface(m->floor);
    
    struct Surface* floor = NULL;
    find_floor(m->pos[0], m->pos[1], m->pos[2], &floor);
    adf_patch_surface(floor);
    
    static const f32 kOffsets[] = { -160.0f, -80.0f, 0.0f, 80.0f, 160.0f };
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            struct Surface* near = NULL;
            find_floor(m->pos[0] + kOffsets[i], m->pos[1] + 100.0f, m->pos[2] + kOffsets[j], &near);
            adf_patch_surface(near);
        }
    }
}

// AnyBLJ
//if m.action == ACT_LONG_JUMP and
//   m.controller.buttonDown & Z_TRIG ~= 0 and
//      m.forwardVel < -15 then
//      m.vel.y = -30
//end
static void anyblj_loop(void){
    struct MarioState* m = &gMarioStates[0];
    if (m->action == ACT_LONG_JUMP && m->forwardVel < -15 && (m->controller->buttonDown & Z_TRIG) != 0) {
        m->vel[1] = -30;
    }
}

// m.numLives = 100
static void maxlife_loop(void) {
    struct MarioState* m = &gMarioStates[0];
    m->numLives = 100;
}

// speed
static int speedx = 4;

static const ConfigOption s_speed_config[] = {
    BIND_INT ("speed", "Multiple Speed", &speedx, 0, 20, "%d")
};
#define SPEEDX_COUNT (sizeof(s_speed_config) / sizeof(s_speed_config[0]))

static void speed_config(void) {
    Config_RenderOptions(s_speed_config, SPEEDX_COUNT);
}
static void speed_loop(void) {
    struct MarioState* m = &gMarioStates[0];
    if (m->action != ACT_BUBBLED && m->action != ACT_WATER_JUMP && m->action != ACT_HOLD_WATER_JUMP) {
        m->vel[0] = m->vel[0] * speedx;
        m->vel[2] = m->vel[2] * speedx;
    }
}

// FreezePos

static Vec3f orig_pos   = {0.00f, 0.00f, 0.00f};
static Vec3f freeze_pos = {0.00f, 0.00f, 0.00f};

static float fp_plusx = 100.00f;
static float fp_plusy = 100.00f;
static float fp_plusz = 100.00f;
static bool fp_dontlockxz = true;
static bool fp_fall = false;

static const ConfigOption s_fp_option[] = {
    BIND_FLOAT ("plusx", "+X to",  &fp_plusx,  -10000.00f, 10000.00f, "%f"),
    BIND_FLOAT ("plusy", "+Y to",  &fp_plusy,  -10000.00f, 10000.00f, "%f"),
    BIND_FLOAT ("plusz", "+Z to",  &fp_plusz,  -10000.00f, 10000.00f, "%f"),
    BIND_BOOL  ("dlockxz", "Dont Lock X/Z",   &fp_dontlockxz),
    BIND_BOOL  ("fall",    "Falldown",        &fp_fall),
};
#define FPOS_COUNT (sizeof(s_fp_option) / sizeof(s_fp_option[0]))

static void fp_config(void) {
    Config_RenderOptions(s_fp_option, FPOS_COUNT);
}

static void fp_enable(void) {
    struct MarioState* m = &gMarioStates[0];
    if (!m) return;
    
    memcpy(orig_pos, m->pos, sizeof(Vec3f));
}

static void fp_loop(void) {
    struct MarioState* m = &gMarioStates[0];
    if (!m) return;
    
    if (!fp_dontlockxz) freeze_pos[0] = orig_pos[0] + fp_plusx;
                        freeze_pos[1] = orig_pos[1] + fp_plusy;
    if (!fp_dontlockxz) freeze_pos[2] = orig_pos[2] + fp_plusz;
    
    if (fp_dontlockxz) {
        freeze_pos[0] = orig_pos[0];
        freeze_pos[2] = orig_pos[2];
    }
    
    memcpy(m->pos, freeze_pos, sizeof(Vec3f));
}

static void fp_disable(void) {
    struct MarioState* m = &gMarioStates[0];
    if (!m) return;
    
    if (!fp_fall) memcpy(m->pos, orig_pos, sizeof(Vec3f));
}

// 按 (？？) 起飞
// local function moon_jump_update(m)
    // if m.controller.buttonDown & L_TRIG ~= 0 then
        // m.faceAngle.y = m.intendedYaw - approach_s32(s16(m.intendedYaw - m.faceAngle.y), 0, 0x800, 0x800)
        // m.vel.y = 40

        // if m.action == ACT_FORWARD_GROUND_KB or
           // m.action == ACT_BACKWARD_GROUND_KB or
           // m.action == ACT_SOFT_FORWARD_GROUND_KB or
           // m.action == ACT_HARD_BACKWARD_GROUND_KB or
           // m.action == ACT_FORWARD_AIR_KB or
           // m.action == ACT_BACKWARD_AIR_KB or
           // m.action == ACT_HARD_FORWARD_AIR_KB or
           // m.action == ACT_HARD_BACKWARD_AIR_KB or
           // m.action == ACT_AIR_HIT_WALL then
            // set_mario_action(m, ACT_FREEFALL, 0)
        // end
    // end
// end

void moon_flyer_loop(void) {
    struct MarioState* m = &gMarioStates[0];
    
    if (m && (m->controller->buttonDown & L_TRIG) != 0) {
        m->faceAngle[1] = m->intendedYaw - approach_s32(s16(m.intendedYaw - m.faceAngle[1]), 0, 0x800, 0x800);
        m.vel[1] = 40;
        
        switch (m->action) {
            case ACT_FORWARD_GROUND_KB:
            case ACT_BACKWARD_GROUND_KB:
            case ACT_SOFT_FORWARD_GROUND_KB:
            case ACT_HARD_BACKWARD_GROUND_KB:
            case ACT_FORWARD_AIR_KB:
            case ACT_BACKWARD_AIR_KB:
            case ACT_HARD_FORWARD_AIR_KB:
            case ACT_HARD_BACKWARD_AIR_KB:
            case ACT_AIR_HIT_WALL:
                set_mario_action(m, ACT_FREEFALL, 0);
        }
    }
}

void module_mario_state(void){
    Module_Register("GodMode",    CAT_MARIO, false, NULL, green, NULL, god_mode_disable,        god_mode_loop);
    Module_HookConfig("GodMode", god_op);
    Config_RegisterModuleOptions("GodMode", s_god_mode_option, GMOD_COUNT);
    
    Module_Register("MaxLives",   CAT_MARIO, false, NULL, green, NULL, NULL, maxlife_loop);
    
    Module_Register("MultipleSpeed", CAT_MARIO, false, NULL, green, NULL, NULL, speed_loop);
    Module_HookConfig("MultipleSpeed", speed_config);
    Config_RegisterModuleOptions("MultipleSpeed", s_speed_config, SPEEDX_COUNT);
    
    Module_Register("MoonFlyer",  CAT_MARIO, false, NULL, gree, NULL, NULL, moon_flyer_loop);
    
    Module_Register("InfWCap",    CAT_MARIO, false, NULL, MOD_COLOR_RED,   NULL, inf_wing_cap_disable,    inf_wing_cap_loop);
    Module_Register("InfMCap",    CAT_MARIO, false, NULL, MOD_COLOR_GREEN, NULL, inf_metal_cap_disable,   inf_metal_cap_loop);
    Module_Register("InfVCap",    CAT_MARIO, false, NULL, MOD_COLOR_BLUE,  NULL, inf_vanish_cap_disable,  inf_vanish_cap_loop);
    
    Module_Register("AntiDeathFloor",  CAT_MARIO, false, NULL, green, NULL, NULL, adf_loop);
    Module_HookConfig("AntiDeathFloor", adf_config);
    Config_RegisterModuleOptions("AntiDeathFloor", s_anti_death_floor_config, ADF_COUNT);
    
    Module_Register("AnyBLJ",     CAT_MARIO, false, NULL, green, NULL, NULL, anyblj_loop);
    
    Module_Register("FreezePos",  CAT_MARIO, false, NULL, green, fp_enable, fp_disable, fp_loop);
    Module_HookConfig("FreezePos", fp_config);
    Config_RegisterModuleOptions("FreezePos", s_fp_option, FPOS_COUNT);
}