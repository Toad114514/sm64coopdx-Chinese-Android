#include "game/mario.h"
#include "game/level_update.h"
#include "pc/network/network_player.h"
#include "types.h"
#include "sm64.h"

#include "../module.h"
#include "../config.h"

// Player0HP
void player_hp_loop(void) {
    for (int i=0;i < MAX_PLAYERS;i++) {
        struct NetworkPlayer* np = &gNetworkPlayers[i];
        struct MarioState* m = &gMarioStates[i];
        if (!m || !m->marioObj) continue;
        if (i == 0 || !np->connected) continue;
        
        m->health = 0x0000;
        m->invincTimer = 0;
    }
}

void module_network(void) {
    Module_Register("Player0HP",   CAT_WEB,  false,  NULL,  MOD_COLOR_RED, NULL, NULL, player_hp_loop);
}