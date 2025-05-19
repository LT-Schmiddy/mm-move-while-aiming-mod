#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "macros.h"
#include "z64animation.h"
#include "objects/gameplay_keep/gameplay_keep.h"

#define OPPOSITE_SIGNS(x, y) (x / ABS(x) != y / ABS(y))

// Recomp Type Definitions:
typedef enum {
    RECOMP_AIMINIG_OVERRIDE_OFF = 0,
    RECOMP_AIMINIG_OVERRIDE_DISABLE_LEFT_STICK = 1,
    RECOMP_AIMINIG_OVERRIDE_FORCE_RIGHT_STICK = 2
} RecompAimingOverideMode;

extern LinkAnimationHeader gPlayerAnim_link_normal_Fclimb_hold2upL;
extern LinkAnimationHeader gPlayerAnim_link_normal_Fclimb_startA;

extern s32 sPlayerShapeYawToTouchedWall;
extern s32 sPlayerWorldYawToTouchedWall;
extern u32 sPlayerTouchedWallFlags;

extern Input* sPlayerControlInput;
// Function for footstep audio.
void func_8083EA44(Player* this, f32 arg1);
s32 func_8083D860(PlayState* play, Player* this);
void func_80837C20(PlayState* play, Player* this);
s32 func_80832558(PlayState* play, Player* this, PlayerFuncD58 arg2);
void func_8082DAD4(Player* this);
void func_8082E920(PlayState* play, Player* this, s32 moveFlags);
void Player_AnimationPlayOnce(PlayState* play, Player* this, PlayerAnimationHeader* anim);

// Player Action Functions:
void Player_Action_43(Player* this, PlayState* play); // Free Look, Hookshot, Bow, etc.
void Player_Action_52(Player* this, PlayState* play); //Riding Epona

s32 FirstPersonColliderCheck(Player* this, PlayState* play);
// My Methods:
bool should_move_while_aiming(PlayState* play, Player* this, bool in_free_look) {
    return (this->actionFunc == Player_Action_43) // Aiming should now only be allowed in non-minigame gameplay, and not riding epona.
        && (
            in_free_look 
            || ( // If it can hold one of these, let it move.
                (this->heldItemAction == PLAYER_IA_BOW) 
                || (this->heldItemAction == PLAYER_IA_BOW_FIRE) 
                || (this->heldItemAction == PLAYER_IA_BOW_ICE) 
                || (this->heldItemAction == PLAYER_IA_BOW_LIGHT) 
                || (this->heldItemAction == PLAYER_IA_HOOKSHOT)
            ) 
            || (this->transformation != PLAYER_FORM_ZORA)// Prevents movement with zora boomerangs
            // || (this->heldItemAction != PLAYER_IA_11)// Prevents movement with zora boomerangs
        ) 
        ;
}

// RECOMP_PATCH s32 func_80830F9C(PlayState* play) {
//     return (play->unk_1887C > 0) && (CHECK_BTN_ALL(sPlayerControlInput->press.button, BTN_B) || CHECK_BTN_ALL(sPlayerControlInput->press.button, BTN_R));
// }

RECOMP_CALLBACK("*", recomp_before_first_person_aiming_update_event) \
void disable_left_stick_callback(PlayState* play, Player* this, bool in_free_look, RecompAimingOverideMode* aiming_override) { 
    if (should_move_while_aiming(play, this, in_free_look) || this->actionFunc == Player_Action_52) {
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
    f32 relX = -(play->state.input[0].rel.stick_x / 60.0f);
    f32 relY = (play->state.input[0].rel.stick_y / 60.0f);

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
    f32 footstep_dist = sqrtf((relX2 * relX2) + (relY2 * relY2)) * movementSpeed;
    if (this->currentMask == PLAYER_MASK_DEKU) {
        footstep_dist *= 2.0f;
    }
    
    f32 wallPolyNormalX = 0.0f;
    f32 wallPolyNormalZ = 0.0f;

    if (
        (this->actor.bgCheckFlags & BGCHECKFLAG_WALL) 
        || (this->actor.bgCheckFlags & BGCHECKFLAG_PLAYER_WALL_INTERACT) 
    ) {

        SurfaceType_CheckWallFlag2(&play->colCtx, this->actor.wallPoly, this->actor.wallBgId);
        CollisionPoly* wallPoly = this->actor.wallPoly;

        wallPolyNormalX = COLPOLY_GET_NORMAL(wallPoly->normal.x);
        wallPolyNormalZ = COLPOLY_GET_NORMAL(wallPoly->normal.z);
    }
    
    func_8083EA44(this, footstep_dist / 4.5f);

    f32 motion_vector_x = (relX2 * movementSpeed);
    f32 motion_vector_z = (relY2 * movementSpeed);

    f32 collision_vector_x = (wallPolyNormalX * movementSpeed);
    f32 collision_vector_z = (wallPolyNormalZ * movementSpeed);

    // if (
    //     (this->actor.bgCheckFlags & BGCHECKFLAG_WALL) 
    //     || (this->actor.bgCheckFlags & BGCHECKFLAG_PLAYER_WALL_INTERACT) 
    // ) {
    //     recomp_printf("motion: X=%f Z=%f", motion_vector_x, motion_vector_z);
    //     recomp_printf(" collision: %f and %f", collision_vector_x, collision_vector_z);
    //     // recomp_printf(" displacement: X=%f Z=%f", this->actor.colChkInfo.displacement.x, this->actor.colChkInfo.displacement.z);
    //     recomp_printf("\n");
    // }

    this->actor.world.pos.x += motion_vector_x * (OPPOSITE_SIGNS(motion_vector_x, wallPolyNormalX) ? (1 - ABS(wallPolyNormalX)) : 1.0f);
    this->actor.world.pos.z += motion_vector_z * (OPPOSITE_SIGNS(motion_vector_z, wallPolyNormalZ) ? (1 - ABS(wallPolyNormalZ)) : 1.0f);
}
