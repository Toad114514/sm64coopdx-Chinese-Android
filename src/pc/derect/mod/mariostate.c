#include "game/mario.h"
#include "game/level_update.h"
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

void module_mario_state(void){
    Module_Register("God Mode",    CAT_MARIO, false, NULL, green, NULL, god_mode_disable,        god_mode_loop);
    Module_HookConfig("God Mode", god_op);
    Config_RegisterModuleOptions("God Mode", s_god_mode_option, GMOD_COUNT);
    
    Module_Register("Inf WCap",    CAT_MARIO, false, NULL, green, NULL, inf_wing_cap_disable,    inf_wing_cap_loop);
    Module_Register("Inf MCap",    CAT_MARIO, false, NULL, green, NULL, inf_metal_cap_disable,   inf_metal_cap_loop);
    Module_Register("Inf VCap",    CAT_MARIO, false, NULL, green, NULL, inf_vanish_cap_disable,  inf_vanish_cap_loop);
}