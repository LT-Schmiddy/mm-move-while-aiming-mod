#include "modding.h"
#include "recomputils.h"
#include "recompconfig.h"

#include "global.h"
#include "functions.h"

#define PLAYER_STATE1_SHIELDING PLAYER_STATE1_400000
#define CHECK_ITEM_IS_BOW(item) ((item == ITEM_BOW) || ((item >= ITEM_BOW_FIRE) && (item <= ITEM_BOW_LIGHT)))
#define CHECK_ITEM_IS_HOOKSHOT(item) (item == ITEM_HOOKSHOT)

typedef enum {
    OFF = 0,
    FIRST_PERSON = 1,
    AIMING = 2,
    HOLDING = 3
} RFiringBehavior;

// order matches EquipSlot enum
static u16 slot_to_btn_id[4] = {
    BTN_B,
    BTN_CLEFT,
    BTN_CDOWN,
    BTN_CRIGHT
};

static Input* sInput = NULL;
static bool special_r_pressed = false;
static bool special_r_held = false;

s32 Player_UpperAction_7(Player* thisx, PlayState* play);
s32 Player_UpperAction_8(Player* thisx, PlayState* play);
void Player_Action_43(Player* this, PlayState* play); // Free Look, Hookshot, Bow, etc.
void Player_Action_52(Player* this, PlayState* play); //Riding Epona
void Player_Action_81(Player* this, PlayState* play); // Shooting Gallery, Cremia's Milk Run

bool Player_isHoldingBow(Player* this) {
    return (
        this->heldItemAction == PLAYER_IA_BOW
        || this->heldItemAction == PLAYER_IA_BOW_FIRE
        || this->heldItemAction == PLAYER_IA_BOW_ICE
        || this->heldItemAction == PLAYER_IA_BOW_LIGHT
        || this->actionFunc == Player_Action_81 // For Bow Mini-Games
        );
}

bool Player_IsAimingBow(Player* this, PlayState* play) {
    return (Player_isHoldingBow(this)) &&
        (
            this->upperActionFunc == Player_UpperAction_8
            || this->upperActionFunc == Player_UpperAction_7
        );
}

bool Player_IsAimingHookshot(Player* this, PlayState* play) {
    return (Player_IsHoldingHookshot(this)) &&
        (
            this->upperActionFunc == Player_UpperAction_8
            || this->upperActionFunc == Player_UpperAction_7
        );
}

bool Player_IsFirstPersonBow(Player* this, PlayState* play) {
    return (Player_isHoldingBow(this)) &&
        (
            this->actionFunc == Player_Action_43
            || this->actionFunc == Player_Action_52
            || this->actionFunc == Player_Action_81
        );
}

bool Player_IsFirstPersonHookshot(Player* this, PlayState* play) {
    return (Player_IsHoldingHookshot(this)) &&
        (
            this->actionFunc == Player_Action_43
            || this->actionFunc == Player_Action_52
            || this->actionFunc == Player_Action_81
        );
}

bool ShouldAllowRFiring(Player* this, PlayState* play) {
    RFiringBehavior bow_r = recomp_get_config_u32("bow-fire-with-r");
    RFiringBehavior hookshot_r = recomp_get_config_u32("hookshot-fire-with-r");
    return (
        (bow_r == FIRST_PERSON && Player_IsFirstPersonBow(this, play))
        || (bow_r == AIMING && Player_IsAimingBow(this, play))
        || (bow_r == HOLDING && Player_isHoldingBow(this))
        || (hookshot_r == FIRST_PERSON && Player_IsFirstPersonHookshot(this, play))
        || (hookshot_r == AIMING && Player_IsAimingHookshot(this, play))
        || (hookshot_r == HOLDING && Player_IsHoldingHookshot(this))
    );
}

RECOMP_HOOK("Player_UpdateCommon") void pre_Player_UpdateCommon(Player* this, PlayState* play, Input* input) {
    // Kafei Prevention:
    if (this->actor.id != ACTOR_PLAYER) {
        return;
    }

    // Checking if we need to do anything.
    if (ShouldAllowRFiring(this, play)) { 
        this->stateFlags1 &= ~PLAYER_STATE1_SHIELDING; 
        special_r_pressed = input->press.button & BTN_R;
        special_r_held = input->cur.button & BTN_R;
        input->press.button &= ~BTN_R;
        input->cur.button &= ~BTN_R;

        // Spoof the C Input:
        EquipSlot spoof_button = EQUIP_SLOT_NONE;
        for (EquipSlot i = EQUIP_SLOT_B; i <= EQUIP_SLOT_C_RIGHT; i++) {
            u8 equippedItem = gSaveContext.save.saveInfo.equips.buttonItems[0][i];
            if ((Player_isHoldingBow(this) && CHECK_ITEM_IS_BOW(equippedItem)) 
                || (Player_IsHoldingHookshot(this) && CHECK_ITEM_IS_HOOKSHOT(equippedItem))) {
                spoof_button = i;
                break;
            }
        }

        if (special_r_pressed) {
            input->press.button |= slot_to_btn_id[spoof_button];
        }
        if (special_r_held) {
            input->cur.button |=  slot_to_btn_id[spoof_button];
        }

    } else {
        this->stateFlags1 |= PLAYER_STATE1_SHIELDING; 
    }
}
