#include "game/mario.h"
#include "game/level_update.h"
#include "surface_terrains.h"
#include "types.h"
#include "sm64.h"
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
static boll adf_death_plane = false;
static bool adf_all = false;

static struct Surface ready_to_place;

static const ConfigOption s_anti_death_floor_config[] = {
    BIND_BOOL ("lavas",       "Fuck Lava/QuickSand",     &adf_lavas),
    BIND_BOOL ("slipper",     "Fuck Ice/Slippery",       &adf_slipper),
    BIND_BOOL ("death_plane", "Fuck DEATH_PLANE",        &adf_death_plane),
    BIND_BOOL ("all",         "Fuck All",                &adf_all)
};
#define ADF_COUNT (sizeof(s_anti_death_floor_config) / sizeof(s_anti_death_floor_config[0]))

static void adf_config(void) {
    Config_RenderOptions(s_anti_death_floor_config, ADF_COUNT)
}

static void adf_loop(void) {
    struct MarioState* m = &gMarioStates[0];
    if (!m || !m->floor) return;
    
    s16 mario_floor = m->floor->type;
    bool willReplace = true;
    
    if (adf_all) {
        willReplace = true;
    }
    else {
        // 防岩浆/QuickSand
        if (adf_lavas) {
            switch (mario_floor) {
                case SURFACE_BURNING:
                case SURFACE_LAVA:
                case SURFACE_DEEP_QUICKSAND:
                case SURFACE_SHALLOW_QUICKSAND:
                case SURFACE_MOVING_QUICKSAND:
                case SURFACE_INSTANT_QUICKSAND:
                case SURFACE_DEEP_MOVING_QUICKSAND:
                case SURFACE_INSTANT_MOVING_QUICKSAND:
                    willReplace = true;
                    //break;
            }
        }
        
        // 防滑
        if (adf_slipper && !willReplace) {
            switch (mario_floor) {
                case SURFACE_SLIPPERY:
                case SURFACE_VERY_SLIPPERY:
                case SURFACE_ICE:
                case SURFACE_HARD_SLIPPERY:
                case SURFACE_HARD_VERY_SLIPPERY:
                //case SURFACE_HARD_NOT_SLIPPERY:
                    willReplace = true;
            }
        }
        
        // 达成一定条件可直接导致陈死亡的地面
        if (adf_death_plane && !willReplace) {
            switch (mario_floor) {
                case SURFACE_DEATH_PLANE:
                case SURFACE_VERTICAL_WIND:
                    willReplace = true;
            }
        }
    }
    
    // 重定向
    if (willReplace) {
        ready_to_place = *m->floor;
        ready_to_place.type = SURFACE_DEFAULT;
        m->floor = &ready_to_place;
    }
}

void module_mario_state(void){
    Module_Register("GodMode",    CAT_MARIO, false, NULL, green, NULL, god_mode_disable,        god_mode_loop);
    Module_HookConfig("GodMode", god_op);
    Config_RegisterModuleOptions("GodMode", s_god_mode_option, GMOD_COUNT);
    
    Module_Register("InfWCap",    CAT_MARIO, false, NULL, green, NULL, inf_wing_cap_disable,    inf_wing_cap_loop);
    Module_Register("InfMCap",    CAT_MARIO, false, NULL, green, NULL, inf_metal_cap_disable,   inf_metal_cap_loop);
    Module_Register("InfVCap",    CAT_MARIO, false, NULL, green, NULL, inf_vanish_cap_disable,  inf_vanish_cap_loop);
    
    Module_Register("AntiDeathFloor",  CAT_MARIO, false, NULL, green, NULL, NULL, adf_loop);
    Module_HookConfig("AntiDeathFloor", adf_config);
    Config_RegisterModuleOptions("AntiDeathFloor", s_anti_death_floor_config, ADF_COUNT);
}