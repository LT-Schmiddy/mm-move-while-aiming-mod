#include "modding.h"

#include "prevent_bss_reordering.h"
#include "global.h"
#include "z64horse.h"
#include "z64quake.h"
#include "z64rumble.h"
#include "z64shrink_window.h"

#include "overlays/actors/ovl_En_Horse_Game_Check/z_en_horse_game_check.h"
#include "overlays/actors/ovl_En_Horse/z_en_horse.h"
// #include "overlays/actors/ovl_En_In/z_en_in.h"


RECOMP_PATCH s32 EnHorse_PlayerCanMove(EnHorse* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    if ( 
        false 
        // (player->stateFlags1 & PLAYER_STATE1_1) 
        // || (func_800B7128(GET_PLAYER(play)) == true) 
        // || (player->stateFlags1 & PLAYER_STATE1_100000) 
         || (((this->stateFlags & ENHORSE_FLAG_19) || (this->stateFlags & ENHORSE_FLAG_29)) && !this->inRace) 
        // || (this->action == ENHORSE_ACTION_HBA) 
        || (player->actor.flags & ACTOR_FLAG_TALK_REQUESTED) 
        // || (play->csCtx.state != CS_STATE_IDLE) 
        || (CutsceneManager_GetCurrentCsId() != CS_ID_NONE) 
        // || (player->stateFlags1 & PLAYER_STATE1_20) 
        // || (player->csAction != PLAYER_CSACTION_NONE)
    ) {
        return false;
    }
    return true;
}
