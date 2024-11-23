#include "modding.h"
#include "global.h"

// Function for footstep audio.
void func_8083EA44(Player* this, f32 arg1);

// RECOMP_DECLARE_EVENT(recomp_disable_left_stick_aiming(bool* should));
// Patches a function in the base game that's used to check if the player should quickspin.
RECOMP_CALLBACK("*", recomp_before_first_person_aiming_update_event) \
void disable_left_stick_callback(PlayState* play, Player* this, bool in_free_look, RecompAimingOverideMode* aiming_override) { 
    bool should = play->unk_1887C == 0 // Prevents movement in shooting gallery.
        && this->rideActor == NULL // Prevents movement on horseback.
        && (
            in_free_look 
            || this->currentMask != PLAYER_MASK_ZORA
        );

    if (should) {
        // *aiming_override = RECOMP_AIMINIG_OVERRIDE_DISABLE_LEFT_STICK;
        *aiming_override = RECOMP_AIMINIG_OVERRIDE_FORCE_RIGHT_STICK;
    }
}

RECOMP_CALLBACK("*", recomp_after_first_person_aiming_update_event) \
void recomp_on_aiming_callback(PlayState* play, Player* this, bool in_free_look) {

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

    func_8083EA44(this, distance / 4.5f);

    this->actor.world.pos.x += (relX2 * movementSpeed) + this->actor.colChkInfo.displacement.x;
    this->actor.world.pos.z += (relY2 * movementSpeed) + this->actor.colChkInfo.displacement.z;
}

