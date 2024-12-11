#include "modding.h"
#include "global.h"


extern s32 sPlayerShapeYawToTouchedWall;
// Function for footstep audio.
void func_8083EA44(Player* this, f32 arg1);
s32 func_8083D860(PlayState* play, Player* this);

// Player Action Functions:
void Player_Action_43(Player* this, PlayState* play); // Free Look, Hookshot, Bow, etc.

// My Methods
bool should_move_while_aiming(PlayState* play, Player* this, bool in_free_look) {
    
    return this->actionFunc == Player_Action_43 // Aiming should now only be allowed in non-minigame gameplay, and not riding epona.
        && (
            in_free_look 
            || this->currentMask != PLAYER_MASK_ZORA
        ) // Prevents movement with zora boomerangs
        ;
}

RECOMP_CALLBACK("*", recomp_before_first_person_aiming_update_event) \
void disable_left_stick_callback(PlayState* play, Player* this, bool in_free_look, RecompAimingOverideMode* aiming_override) { 
   if (should_move_while_aiming(play, this, in_free_look)) {
        // *aiming_override = RECOMP_AIMINIG_OVERRIDE_DISABLE_LEFT_STICK;
        *aiming_override = RECOMP_AIMINIG_OVERRIDE_FORCE_RIGHT_STICK;
    }
}

RECOMP_CALLBACK("*", recomp_after_first_person_aiming_update_event) \
void recomp_on_aiming_callback(PlayState* play, Player* this, bool in_free_look) {
    if (!should_move_while_aiming(play, this, in_free_look)) {
        return;
    }

    f32 movementSpeed = 8.25f; // account for mask
    if (this->currentMask == PLAYER_MASK_BUNNY) {
        movementSpeed *= 1.5f;
    }

    //f32 relX = (sPlayerControlInput->rel.stick_x / 10);
    //f32 relY = (sPlayerControlInput->rel.stick_y / 10);
    f32 relX = -(play->state.input[0].rel.stick_x / 10);
    f32 relY = (play->state.input[0].rel.stick_y / 10);

    // Normalize so that diagonal movement isn't faster
    f32 relMag = sqrtf((relX * relX) + (relY * relY));
    if (relMag > 1.0f) {
        relX /= relMag;
        relY /= relMag;
    }

    // Determine what left and right mean based on camera angle
    f32 relX2 = relX * Math_CosS(this->actor.focus.rot.y) + relY * Math_SinS(this->actor.focus.rot.y);
    f32 relY2 = relY * Math_CosS(this->actor.focus.rot.y) - relX * Math_SinS(this->actor.focus.rot.y);

    // Calculate distance for footstep sound
    f32 distance = sqrtf((relX2 * relX2) + (relY2 * relY2)) * movementSpeed;
    if (this->currentMask == PLAYER_MASK_DEKU) {
        distance *= 2.0f;
    }
    
    if (
            !(this->stateFlags1 & PLAYER_STATE1_800) 
            && (this->actor.bgCheckFlags & BGCHECKFLAG_PLAYER_WALL_INTERACT) 
            && (sPlayerShapeYawToTouchedWall < 0x3000)
            && relY >= 0.0f
        ) {
        s32 wall_check = func_8083D860(play, this);
        return;
    }
    func_8083EA44(this, distance / 4.5f);
    this->actor.world.pos.x += (relX2 * movementSpeed) + this->actor.colChkInfo.displacement.x;
    this->actor.world.pos.z += (relY2 * movementSpeed) + this->actor.colChkInfo.displacement.z;
}

