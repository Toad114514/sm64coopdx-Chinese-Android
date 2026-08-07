#include "game/mario.h"
#include "game/level_update.h"
#include "types.h"
#include "../module.h"

ImVec4 green = (ImVec4){0.10f, 0.85f, 0.30f, 1.0f};

// Mario God Mode
static void god_mode_loop(void) {
    struct MarioState* m = &gMarioStates[0];
    if (!m || !m->marioObj) return;

    m->health = 0x0880;
    m->invincTimer = 30;
    m->healCounter = 4;
}

static void god_mode_disable(void) {
    struct MarioState* m = &gMarioStates[0];
    if (m) {
        m->invincTimer = 0;
    }
}

static void module_mario_state(void){
    Module_Register("God Mode", CAT_MARIO, false, NULL, green, NULL, god_mode_disable, god_mode_loop);
}