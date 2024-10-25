#include "patches_item.h"

#include "evt_cmd.h"
#include "mod.h"
#include "patch.h"
#include "patches_battle.h"
#include "patches_battle_seq.h"
#include "tot_generate_item.h"
#include "tot_manager_achievements.h"
#include "tot_manager_reward.h"
#include "tot_state.h"

#include <ttyd/battle.h>
#include <ttyd/battle_camera.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_event_default.h>
#include <ttyd/battle_item_data.h>
#include <ttyd/battle_mario.h>
#include <ttyd/battle_sub.h>
#include <ttyd/battle_unit.h>
#include <ttyd/battle_weapon_power.h>
#include <ttyd/evt_eff.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evt_sub.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/icondrv.h>
#include <ttyd/item_data.h>
#include <ttyd/itemdrv.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/swdrv.h>
#include <ttyd/system.h>

#include <cstdint>
#include <cstring>

namespace mod::tot::patch {

namespace {

// For convenience.
using namespace ::ttyd::battle_camera;
using namespace ::ttyd::battle_database_common;
using namespace ::ttyd::battle_event_cmd;
using namespace ::ttyd::evt_eff;
using namespace ::ttyd::evt_snd;
using namespace ::ttyd::evt_sub;

using ::ttyd::battle_unit::BattleWorkUnit;
using ::ttyd::evtmgr::EvtEntry;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;
using ::ttyd::item_data::itemDataTable;
using ::ttyd::item_data::ItemData;

namespace IconType = ::ttyd::icondrv::IconType;
namespace ItemType = ::ttyd::item_data::ItemType;
namespace ItemUseLocation = ::ttyd::item_data::ItemUseLocation_Flags;

bool g_FreezeFieldItems = false;

}

// Function hooks.
extern void* (*g_itemEntry_trampoline)(
    const char*, int32_t, float, float, float, uint32_t, int32_t, void*);
extern uint32_t (*g_pouchGetItem_trampoline)(int32_t);
extern int32_t (*g_btlevtcmd_GetItemRecoverParam_trampoline)(EvtEntry*, bool);
extern int32_t (*g__get_flower_suitoru_point_trampoline)(EvtEntry*, bool);
extern int32_t (*g__get_heart_suitoru_point_trampoline)(EvtEntry*, bool);
extern int32_t (*g_BattleItemData_hpfp_change_declare_1_trampoline)(EvtEntry*, bool);
extern int32_t (*g_BattleItemData_rank_up_trampoline)(EvtEntry*, bool);
// Patch addresses.
extern const int32_t g_itemMain_CheckItemFreeze_BH;
extern const int32_t g_pouchRemoveItemIndex_CheckMaxInv_BH;
extern const int32_t g_pouchRemoveItem_CheckMaxInv_BH;
extern const int32_t g_pouchGetItem_CheckMaxInv_BH;
extern const int32_t g_pouchGetEmptyHaveItemCnt_CheckMaxInv_BH;
extern const int32_t g_ItemEvent_LastDinner_Weapon;
extern const int32_t g_ItemEvent_Teki_Kyouka_ApplyStatusHook;
extern const int32_t g_ItemEvent_Support_NoEffect_TradeOffJumpPoint;
extern const int32_t g_ItemEvent_Poison_Kinoko_PoisonChance;
extern const int32_t g_fbatBattleMode_Patch_BumpAttackLevel;
extern const int32_t g_btlseqTurn_HappyHeartProc_BH;
extern const int32_t g_btlseqTurn_HappyHeartProc_EH;
extern const int32_t g_btlseqTurn_HappyFlowerProc_BH;
extern const int32_t g_btlseqTurn_HappyFlowerProc_EH;
extern const int32_t g_btlevtcmd_ConsumeItem_Patch_RefundPer;
extern const int32_t g_btlevtcmd_ConsumeItemReserve_Patch_RefundPer;
extern const int32_t g_btlevtcmd_ConsumeItem_Patch_RefundBase;
extern const int32_t g_btlevtcmd_ConsumeItemReserve_Patch_RefundBase;
extern const int32_t g_BattleDamageDirect_Patch_AddTotalDamage;
extern const int32_t g_BattleDamageDirect_Patch_PityFlowerChance;
extern const int32_t g_BattleAudience_Case_Appeal_Patch_AppealSp;

// Assembly patch functions.
extern "C" {
    // field_item_patches.s
    void StartCheckItemFreeze();
    void BranchBackCheckItemFreeze();
    // happy_badge_patches.s
    void StartHappyHeartProc();
    void BranchBackHappyHeartProc();
    void StartHappyFlowerProc();
    void BranchBackHappyFlowerProc();
    // item_inventory_patches.s
    void StartGetItemMax();
    void BranchBackGetItemMax();
    void StartRemoveItemMax();
    void BranchBackRemoveItemMax();
    void StartRemoveItemIndexMax();
    void BranchBackRemoveItemIndexMax();
    void StartGetEmptyItemSlotsMax();
    void BranchBackGetEmptyItemSlotsMax();

    bool checkItemFreeze() {
        return g_FreezeFieldItems;
    }
    
    int32_t getTotItemInventorySize() {
        return mod::tot::patch::item::GetItemInventorySize();
    }
}

namespace item {
    
namespace {

// Forward declarations.
EVT_DECLARE_USER_FUNC(ToggleCookingItemTypeToHeldItem, 1)
EVT_DECLARE_USER_FUNC(evtTot_GetDummyPoisonShroomWeapon, 1)
EVT_DECLARE_USER_FUNC(evtTot_RecoverStarPower, 0)
EVT_DECLARE_USER_FUNC(evtTot_StoreGradualStarPower, 0)
EVT_DECLARE_USER_FUNC(evtTot_PickRandomStatus, 1)

// Mystic Egg event; Recovers 1/3 of max SP.
EVT_BEGIN(MysticEggAttackEvent)
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(8))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(8))
    USER_FUNC(btlevtcmd_CheckCommandUnit, -2, LW(0))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(btlevtcmd_GetSelectEnemy, LW(10), LW(11))
    ELSE()
        USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
        USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(8))
        USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(8), LW(10), LW(11))
    END_IF()
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, -1)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    WAIT_MSEC(1000)
    USER_FUNC(btlevtcmd_GetFriendBelong, -2, LW(0))
    IF_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetBodyId, -2, LW(9))
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, LW(9), 40)
        USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.00))
        USER_FUNC(btlevtcmd_MovePosition, -2, -60, LW(1), LW(2), 0, -1, 0)
    END_IF()
    USER_FUNC(btlevtcmd_GetBodyId, -2, LW(9))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, LW(9), 52)
    USER_FUNC(btlevtcmd_CommandFlyItem, -2, 60)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, LW(9), 69)
    USER_FUNC(btlevtcmd_ConsumeItem, -2)
    USER_FUNC(btlevtcmd_CommandGetWeaponItemId, LW(9), LW(8))
    IF_EQUAL(LW(9), 0)
        USER_FUNC(btlevtcmd_ConsumeItem, LW(9))
    END_IF()
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, LW(10), -1)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)

    USER_FUNC(btlevtcmd_GetPos, LW(10), LW(0), LW(1), LW(2))
    USER_FUNC(evt_eff, PTR(""), PTR("stardust"), 2, LW(0), LW(1), LW(2), 50, 50, 50, 100, 0, 0, 0, 0)
    USER_FUNC(evt_snd_sfxon_3d, PTR("SFX_ITEM_RECOVERY_SHINE1"), LW(0), LW(1), LW(2), 0)
    USER_FUNC(evtTot_RecoverStarPower)

    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    WAIT_FRM(60)
    RUN_CHILD_EVT(ttyd::battle_item_data::_return_home_event)
    USER_FUNC(btlevtcmd_StartWaitEvent, LW(10))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

// Meteor Meal event; applies status and banks SP regen for 3 turns.
EVT_BEGIN(MeteorMealAttackEvent)
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(8))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(8))
    USER_FUNC(btlevtcmd_CheckCommandUnit, -2, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_GetSelectEnemy, LW(10), LW(11))
    ELSE()
        USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
        USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(8))
        USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(8), LW(10), LW(11))
    END_IF()
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, -1)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    WAIT_MSEC(1000)
    USER_FUNC(btlevtcmd_GetFriendBelong, -2, LW(0))
    IF_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetBodyId, -2, LW(9))
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, LW(9), 40)
        USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.0))
        USER_FUNC(btlevtcmd_MovePosition, -2, -60, LW(1), LW(2), 0, -1, 0)
    END_IF()
    USER_FUNC(btlevtcmd_GetBodyId, -2, LW(9))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, LW(9), 52)
    USER_FUNC(btlevtcmd_CommandFlyItem, -2, 60)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, LW(9), 69)
    USER_FUNC(btlevtcmd_ConsumeItem, -2)
    USER_FUNC(btlevtcmd_CommandGetWeaponItemId, LW(9), LW(8))
    IF_EQUAL(LW(9), 0)
        USER_FUNC(btlevtcmd_GetConsumeItem, LW(9))
    END_IF()
    RUN_CHILD_EVT(ttyd::battle_item_data::ItemEvent_Recovery_Core_Effect)
    SET(LW(12), LW(8))
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(10), LW(11), LW(12), 256, LW(5))
    SWITCH(LW(5))
        CASE_EQUAL(3)
            USER_FUNC(btlevtcmd_StartAvoid, LW(10), 38)
        CASE_EQUAL(6)
            USER_FUNC(btlevtcmd_StartAvoid, LW(10), 39)
        CASE_EQUAL(2)
            USER_FUNC(btlevtcmd_StartAvoid, LW(10), 40)
        CASE_EQUAL(1)
            USER_FUNC(btlevtcmd_GetPos, LW(10), LW(0), LW(1), LW(2))
            USER_FUNC(evt_eff, PTR(""), PTR("stardust"), 2, LW(0), LW(1), LW(2), 50, 50, 50, 100, 0, 0, 0, 0)
            USER_FUNC(evtTot_StoreGradualStarPower)
            USER_FUNC(patch::battle::evtTot_ApplyCustomStatus,
                LW(10), LW(11), 0,
                /* splash colors */ 0xccf4ff, 0xffa0ff,
                PTR("SFX_CONDITION_REST_HP_SLOW1"), 
                PTR("tot_sp_regen_effect"))
            USER_FUNC(btlevtcmd_CheckDamage, -2, LW(10), LW(11), LW(12), 256, LW(5))
    END_SWITCH()
    RUN_CHILD_EVT(ttyd::battle_item_data::_return_home_event)
    USER_FUNC(btlevtcmd_StartWaitEvent, LW(10))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

// Wrapper to Poison Shroom event that makes it have no effect on Fuzzy Horde.
EVT_BEGIN(PoisonShroomWrapperEvent)
    SET(LW(12), PTR(&ttyd::battle_item_data::ItemWeaponData_PoisonKinoko))
    RUN_CHILD_EVT(ttyd::battle_item_data::ItemEvent_GetTarget)
    IF_EQUAL(LW(10), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_GetUnitKind, LW(10), LW(0))
    IF_NOT_EQUAL(LW(0), (int32_t)BattleUnitType::FUZZY_HORDE)
        // Run original weapon logic.
        RUN_CHILD_EVT(ttyd::battle_item_data::ItemEvent_Poison_Kinoko)
        GOTO(99)
    END_IF()

    // Special handling: for Fuzzy Horde, do 0-damage hit, since the normal
    // effect trivializes the fight and the healing could cause issues.
    USER_FUNC(evtTot_GetDummyPoisonShroomWeapon, LW(12))
    RUN_CHILD_EVT(ttyd::battle_item_data::ItemEvent_Support_Sub_UseDeclere)
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    RUN_CHILD_EVT(ttyd::battle_item_data::ItemEvent_Support_Sub_Effect)
    USER_FUNC(evt_snd_sfxon_3d, PTR("SFX_BTL_ATTACK_MISS2"), 0, 0, 0, 0)
    WAIT_FRM(40)
    RUN_CHILD_EVT(ttyd::battle_item_data::_return_home_event)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    
LBL(99)
    RETURN()
EVT_END()

// Custom attack event for Love Pudding and Peach Tart.
EVT_BEGIN(RngStatusAttackEvent)
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    USER_FUNC(evtTot_PickRandomStatus, LW(12))
    RUN_CHILD_EVT(ttyd::battle_item_data::ItemEvent_Support_NoEffect)
    RETURN()
EVT_END()

// Custom attack event for CookingItems that temporarily sets CookingItem's type
// to the attacker's held item, so it can be used for target weighting.
EVT_BEGIN(CookingItemAttackEvent)
    USER_FUNC(ToggleCookingItemTypeToHeldItem, 1)
    RUN_CHILD_EVT(PTR(ttyd::battle_item_data::ItemEvent_Recovery))
    USER_FUNC(ToggleCookingItemTypeToHeldItem, 0)
    RETURN()
EVT_END()

// Patch over the end of the existing Trade Off item script so it actually
// calls the part of the code associated with applying its status.
EVT_BEGIN(TradeOffPatch)
    SET(LW(12), PTR(&ttyd::battle_item_data::ItemWeaponData_Teki_Kyouka))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    // Run the end of ItemEvent_Support_NoEffect's evt.
    RUN_CHILD_EVT(static_cast<int32_t>(g_ItemEvent_Support_NoEffect_TradeOffJumpPoint))
    RETURN()
EVT_END()

// Toggles on/off setting CookingItem's item type to the attacker's held item.
// This allows, e.g., target weighting for pure-FP items to be properly used.
EVT_DEFINE_USER_FUNC(ToggleCookingItemTypeToHeldItem) {
    BattleWeapon& weapon = ttyd::battle_item_data::ItemWeaponData_CookingItem;
    if (evtGetValue(evt, evt->evtArguments[0])) {
        // Get the held item from the current attacker.
        if (evt->wActorThisPtr) {
            BattleWorkUnit* unit = ttyd::battle::BattleGetUnitPtr(
                ttyd::battle::g_BattleWork,
                reinterpret_cast<uint32_t>(evt->wActorThisPtr));
            if (unit && unit->held_item > 0) {
                weapon.item_id = unit->held_item;
            }
        }
    } else {
        weapon.item_id = 0;
    }
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_GetDummyPoisonShroomWeapon) {
    static BattleWeapon weapon;
    memcpy(
        &weapon, &ttyd::battle_item_data::ItemWeaponData_PoisonKinoko, 
        sizeof(BattleWeapon));
    weapon.damage_function = (void*)ttyd::battle_weapon_power::weaponGetPowerDefault;
    weapon.damage_function_params[0] = 0;
    weapon.poison_strength = 0;
    weapon.poison_chance = 0;
    weapon.poison_time = 0;

    evtSetValue(evt, evt->evtArguments[0], PTR(&weapon));
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_RecoverStarPower) {
    auto& pouch = *ttyd::mario_pouch::pouchGetPtr();
    pouch.current_sp += pouch.max_sp / 3;
    if (pouch.current_sp > pouch.max_sp) pouch.current_sp = pouch.max_sp;
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_StoreGradualStarPower) {
    patch::battle_seq::StoreGradualSpRegenEffect();
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_PickRandomStatus) {
    auto* weapon = (BattleWeapon*)evtGetValue(evt, evt->evtArguments[0]);
    // Nullify all status chances and damage.
    weapon->damage_function = nullptr;
    weapon->damage_function_params[0] = 0;
    memset(&weapon->sleep_chance, 0, 0x30);
    switch (weapon->item_id) {
        case ItemType::LOVE_PUDDING: {
            switch(ttyd::system::irand(5)) {
                case 0:
                    weapon->atk_change_strength = 5;
                    weapon->atk_change_chance = 100;
                    weapon->atk_change_time = 3;
                    break;
                case 1:
                    weapon->def_change_strength = 5;
                    weapon->def_change_chance = 100;
                    weapon->def_change_time = 3;
                    break;
                case 2:
                    weapon->hp_regen_strength = 5;
                    weapon->hp_regen_time = 5;
                    break;
                case 3:
                    weapon->fp_regen_strength = 5;
                    weapon->fp_regen_time = 5;
                    break;
                case 4:
                    weapon->invisible_chance = 100;
                    weapon->invisible_time = 3;
                    break;
            }
            break;
        }
        case ItemType::PEACH_TART: {
            switch(ttyd::system::irand(5)) {
                case 0:
                    weapon->damage_function = 
                        (void*)ttyd::battle_weapon_power::weaponGetPowerDefault;
                    weapon->damage_function_params[0] = 5;
                    weapon->poison_chance = 100;
                    weapon->poison_strength = 1;
                    weapon->poison_time = 5;
                    break;
                case 1:
                    weapon->sleep_chance = 100;
                    weapon->sleep_time = 3;
                    break;
                case 2:
                    weapon->confuse_chance = 100;
                    weapon->confuse_time = 3;
                    break;
                case 3:
                    weapon->freeze_chance = 100;
                    weapon->freeze_time = 3;
                    break;
                case 4:
                    weapon->size_change_strength = -2;
                    weapon->size_change_chance = 100;
                    weapon->size_change_time = 3;
                    break;
            }
            break;
        }
    }
    return 2;
}

// Returns altered item restoration parameters.
EVT_DEFINE_USER_FUNC(GetAlteredItemRestorationParams) {
    int32_t item_id = evtGetValue(evt, evt->evtArguments[0]);
    if (item_id == ItemType::CAKE) {
        int32_t hp = 
            evtGetValue(evt, evt->evtArguments[1]) + GetBonusCakeRestoration();
        int32_t fp =
            evtGetValue(evt, evt->evtArguments[2]) + GetBonusCakeRestoration();
        evtSetValue(evt, evt->evtArguments[1], hp);
        evtSetValue(evt, evt->evtArguments[2], fp);
    }
    return 2;
}

// Replaces the vanilla logic for HP or FP Drain restoration.
int32_t GetDrainRestoration(EvtEntry* evt, bool hp_drain) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    int32_t id = evtGetValue(evt, evt->evtArguments[0]);
    id = ttyd::battle_sub::BattleTransID(evt, id);
    auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, id);
    
    bool use_64_style_drain = false;
    int32_t drain = 0;
    if (unit) {
        int32_t num_badges = 0;
        if (hp_drain) {
            num_badges = unit->badges_equipped.hp_drain;
        } else {
            num_badges = unit->badges_equipped.fp_drain;
        }
        if (use_64_style_drain) {
            // 1 point per damaging hit x num badges, max of 5.
            drain = unit->total_damage_dealt_this_attack * num_badges;
            if (drain > 5) drain = 5;
        } else {
            // 1 per badge if any damaging hits were dealt.
            drain = !!unit->total_damage_dealt_this_attack * num_badges;
        }
    }
    evtSetValue(evt, evt->evtArguments[1], drain);
    return 2;
}

inline void SetItemRestoration(int32_t item, int32_t hp = 0, int32_t fp = 0) {
    itemDataTable[item].hp_restored = hp;
    itemDataTable[item].fp_restored = fp;
}

}
    
void ApplyFixedPatches() {
    // Rebalanced price tiers for items & badges (non-pool items may have 0s).
    static const constexpr uint32_t kPriceTiers[] = {
        // Items / recipes.
        0x5a866873, 0x5a335335, 0xb7423343, 0x44474215, 0x03001676,
        0x00800006, 0x40864330, 0xa0005000, 0x036bba80, 0x08086650,
        0x00760057, 0x00000bb6, 0x85455809, 0x00000060,
        // Badges.
        0xb8a88dbb, 0xdd9d8a8b, 0xeeddcccc, 0xcceeffff, 0xbbccccdd,
        0xbbbebeeb, 0xaaaacdfc, 0xcaacccca, 0xd00edd7c, 0x0000000d,
        0x0a770007, 0xddee0000, 0x00000bbd,
    };
    // Prices corresponding to the price tiers in the above array.
    static const constexpr uint8_t kPrices[] = {
        5, 10, 15, 20, 25, 30, 40, 50, 60, 70, 80, 100, 125, 150, 200, 250
    };
    static const constexpr uint32_t kBpCost[] = {
        0x11111111, 0x44111111, 0x22662211, 0x22111144, 0x11224411,
        0x33331441, 0x12226220, 0x62211111, 0x40032216, 0x00000004,
        0x03110001, 0x24220100, 0x00000112,
    };
    static const constexpr int8_t kSortOrders[] = {
        // Items (0x80 - 0xef).
        12, 18, 19, 17, 16, 56, 36, 34, 35, 23, 20, 15, 21, 22, 32, 33,
        13, 28, 25, 26,  1,  2,  3, 29, 30, 31,  4,  5,  6, 10, 11,  7,
         8, 39, 27, 40, -1, -1, 14, -1,  9, -1, -1, -1, -1, 38, -1, -1,
        -1, 37, 24, 41, 42, 43, -1, 47, -1, -1, -1, 48, -1, -1, -1, 49,
        -1, 69, 70, 71, 72, 45, 65, -1, -1, 58, 54, 55, 50, -1, 51, -1,
        63, 53, -1, -1, 59, 66, -1, -1, 67, 64, 73, -1, -1, -1, -1, -1,
        62, -1, 68, 61, 57, 44, 52, 46, -1, 60, -1, -1, -1, -1, -1, -1,
        // Badges (0xf0 - 0x152).
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 33, 34,
        29, 30, 27, 28,  1,  2,  9, 10,  3,  4, 20, 21, 18, 19, 11, 12,
        22, 23, 13, 14, 15, 16, -1, -1, -1, 43, 44, 57, 58, 61, 62, 59,
        17,  7,  8, 60, 40, 41, 42, 50, 51, 52, 53, 24, 25, 35, 36, 37,
        38, 56, 45, 46, 26, -1, -1, 54, 55, -1, -1, -1, -1, -1, -1, -1,
        63, -1, -1, -1, -1, -1, 39, -1, -1, -1, -1, -1,  5,  6, 49, 47,
        48, 31, 32,
    };
    
    // - Set coin buy & sell (for Refund) prices based on above tiers.
    // - Set healing items' weapons to CookingItem if they don't have one.
    // - Fix unused items' and badges' sort order.
    for (int32_t i = ItemType::GOLD_BAR; i < ItemType::MAX_ITEM_TYPE; ++i) {
        ItemData& item = itemDataTable[i];

        if (i >= ItemType::THUNDER_BOLT) {
            // Assign new buy / sell prices.
            const int32_t word_index = (i - ItemType::THUNDER_BOLT) >> 3;
            const int32_t nybble_index = (i - ItemType::THUNDER_BOLT) & 7;
            const int32_t tier =
                (kPriceTiers[word_index] >> (nybble_index << 2)) & 15;
            item.buy_price = kPrices[tier];
            if (i >= ItemType::POWER_JUMP) {
                item.sell_price = kPrices[tier] / 10;
            } else {
                item.sell_price = kPrices[tier] / 5;
            }
            if (item.sell_price < 1) item.sell_price = 1;
            // Assign new sort order (for items / badge sorts / log).
            item.type_sort_order = kSortOrders[i - ItemType::THUNDER_BOLT];
        }
        
        if (i < ItemType::POWER_JUMP) {
            // For all items that restore HP or FP, assign the "cooked item"
            // weapon struct if they don't already have a weapon assigned.
            if (!item.weapon_params && (item.hp_restored || item.fp_restored)) {
                item.weapon_params =
                    &ttyd::battle_item_data::ItemWeaponData_CookingItem;
            } else if (item.weapon_params && item.hp_restored) {
                // For HP restoration items with weapon structs, give them
                // Mushroom-like target weighting (heal the least healthy).
                item.weapon_params->target_weighting_flags =
                    ttyd::battle_item_data::ItemWeaponData_Kinoko.
                        target_weighting_flags;
            }
        } else {
            // Assign new sort order and BP cost.
            const int32_t word_index = (i - ItemType::POWER_JUMP) >> 3;
            const int32_t nybble_index = (i - ItemType::POWER_JUMP) & 7;
            item.bp_cost = (kBpCost[word_index] >> (nybble_index << 2)) & 15;
        }
    }
    
    // Patch data for TOT reward items.
    RewardManager::PatchRewardItemData();
    
    // Change icon for blue coins.
    itemDataTable[ItemType::PIANTA].icon_id = IconType::TOT_COIN_BLUE;
    
    // Set Star Piece buy price.
    // Currently unused, but might make purchaseable from shops in future.
    itemDataTable[ItemType::STAR_PIECE].buy_price = 50;
    
    // Changed pickup messages for Super / Ultra boots and hammer.
    itemDataTable[ItemType::SUPER_BOOTS].description = "msg_custom_super_boots";
    itemDataTable[ItemType::ULTRA_BOOTS].description = "msg_custom_ultra_boots";
    itemDataTable[ItemType::SUPER_HAMMER].description = "msg_custom_super_hammer";
    itemDataTable[ItemType::ULTRA_HAMMER].description = "msg_custom_ultra_hammer";
    
    // Changed messages, appearance and sort order for Tower Key items.
    itemDataTable[ItemType::TOT_TOWER_KEY].name = "tot_key_name";
    itemDataTable[ItemType::TOT_TOWER_KEY].description = "tot_key_desc";
    itemDataTable[ItemType::TOT_TOWER_KEY].menu_description = "tot_key_desc";
    itemDataTable[ItemType::TOT_TOWER_KEY].type_sort_order = 98;
    itemDataTable[ItemType::TOT_TOWER_KEY].sell_price = 25;
    itemDataTable[ItemType::TOT_TOWER_KEY].icon_id = IconType::STORAGE_KEY;
    itemDataTable[ItemType::TOT_MASTER_KEY].name = "tot_mkey_name";
    itemDataTable[ItemType::TOT_MASTER_KEY].description = "tot_mkey_desc";
    itemDataTable[ItemType::TOT_MASTER_KEY].menu_description = "tot_mkey_desc";
    itemDataTable[ItemType::TOT_MASTER_KEY].type_sort_order = 99;
    itemDataTable[ItemType::TOT_MASTER_KEY].sell_price = 50;
    itemDataTable[ItemType::TOT_MASTER_KEY].icon_id = IconType::STATION_KEY;

    // Key items...

    itemDataTable[ItemType::TOT_KEY_PEEKABOO].name = "in_HP_mieru";
    itemDataTable[ItemType::TOT_KEY_PEEKABOO].description = "msg_HP_mieru";
    itemDataTable[ItemType::TOT_KEY_PEEKABOO].menu_description = "msg_HP_mieru";
    itemDataTable[ItemType::TOT_KEY_PEEKABOO].icon_id = IconType::PEEKABOO;
    itemDataTable[ItemType::TOT_KEY_PEEKABOO].type_sort_order = 1;

    itemDataTable[ItemType::TOT_KEY_SUPER_PEEKABOO].name = "in_super_peekaboo";
    itemDataTable[ItemType::TOT_KEY_SUPER_PEEKABOO].description = "msg_super_peekaboo";
    itemDataTable[ItemType::TOT_KEY_SUPER_PEEKABOO].menu_description = "msg_super_peekaboo";
    itemDataTable[ItemType::TOT_KEY_SUPER_PEEKABOO].icon_id = IconType::SUPER_PEEKABOO;
    itemDataTable[ItemType::TOT_KEY_SUPER_PEEKABOO].type_sort_order = 2;

    itemDataTable[ItemType::TOT_KEY_TIMING_TUTOR].name = "in_timing_oshieru";
    itemDataTable[ItemType::TOT_KEY_TIMING_TUTOR].description = "msg_timing_oshieru";
    itemDataTable[ItemType::TOT_KEY_TIMING_TUTOR].menu_description = "msg_timing_oshieru";
    itemDataTable[ItemType::TOT_KEY_TIMING_TUTOR].icon_id = IconType::TIMING_TUTOR;
    itemDataTable[ItemType::TOT_KEY_TIMING_TUTOR].type_sort_order = 3;

    itemDataTable[ItemType::TOT_KEY_BGM_TOGGLE].name = "in_bgm";
    itemDataTable[ItemType::TOT_KEY_BGM_TOGGLE].description = "msg_bgm";
    itemDataTable[ItemType::TOT_KEY_BGM_TOGGLE].menu_description = "msg_bgm";
    itemDataTable[ItemType::TOT_KEY_BGM_TOGGLE].icon_id = IconType::BGM_MUTE;
    itemDataTable[ItemType::TOT_KEY_BGM_TOGGLE].type_sort_order = 4;

    itemDataTable[ItemType::TOT_KEY_ITEM_SELECTOR].name = "in_itemselect";
    itemDataTable[ItemType::TOT_KEY_ITEM_SELECTOR].description = "msg_itemselect";
    itemDataTable[ItemType::TOT_KEY_ITEM_SELECTOR].menu_description = "msg_itemselect";
    itemDataTable[ItemType::TOT_KEY_ITEM_SELECTOR].icon_id = IconType::ITEM_HOG;
    itemDataTable[ItemType::TOT_KEY_ITEM_SELECTOR].type_sort_order = 5;

    itemDataTable[ItemType::TOT_KEY_BADGE_SELECTOR].name = "in_badgeselect";
    itemDataTable[ItemType::TOT_KEY_BADGE_SELECTOR].description = "msg_badgeselect";
    itemDataTable[ItemType::TOT_KEY_BADGE_SELECTOR].menu_description = "msg_badgeselect";
    itemDataTable[ItemType::TOT_KEY_BADGE_SELECTOR].icon_id = IconType::BADGE_SELECTOR;
    itemDataTable[ItemType::TOT_KEY_BADGE_SELECTOR].type_sort_order = 6;

    itemDataTable[ItemType::TOT_KEY_MARIO_COSTUME].name = "in_m_emblem";
    itemDataTable[ItemType::TOT_KEY_MARIO_COSTUME].description = "msg_m_emblem";
    itemDataTable[ItemType::TOT_KEY_MARIO_COSTUME].menu_description = "msg_m_emblem";
    itemDataTable[ItemType::TOT_KEY_MARIO_COSTUME].icon_id = IconType::MARIO_EMBLEM;
    itemDataTable[ItemType::TOT_KEY_MARIO_COSTUME].type_sort_order = 7;

    itemDataTable[ItemType::TOT_KEY_YOSHI_COSTUME].name = "in_y_emblem";
    itemDataTable[ItemType::TOT_KEY_YOSHI_COSTUME].description = "msg_y_emblem";
    itemDataTable[ItemType::TOT_KEY_YOSHI_COSTUME].menu_description = "msg_y_emblem";
    itemDataTable[ItemType::TOT_KEY_YOSHI_COSTUME].icon_id = IconType::YOSHI_EMBLEM;
    itemDataTable[ItemType::TOT_KEY_YOSHI_COSTUME].type_sort_order = 8;

    itemDataTable[ItemType::TOT_KEY_ATTACK_FX].name = "in_attack_fx";
    itemDataTable[ItemType::TOT_KEY_ATTACK_FX].description = "msg_attack_fx";
    itemDataTable[ItemType::TOT_KEY_ATTACK_FX].menu_description = "msg_attack_fx";
    itemDataTable[ItemType::TOT_KEY_ATTACK_FX].icon_id = IconType::ATTACK_FX_R;
    itemDataTable[ItemType::TOT_KEY_ATTACK_FX].type_sort_order = 9;

    // Balance changes for individual items...

    // Various items have rebalanced recovery.
    SetItemRestoration(ItemType::ULTRA_SHROOM, 30, 0);
    SetItemRestoration(ItemType::JAMMIN_JELLY, 0, 30);
    SetItemRestoration(ItemType::SHROOM_FRY, 8, 2);
    SetItemRestoration(ItemType::SHROOM_STEAK, 45, 10);
    SetItemRestoration(ItemType::JELLY_ULTRA, 30, 30);
    SetItemRestoration(ItemType::ZESS_DINNER, 15, 15);
    SetItemRestoration(ItemType::ZESS_SPECIAL, 25, 25);
    SetItemRestoration(ItemType::ZESS_DELUXE, 50, 50);
    SetItemRestoration(ItemType::ZESS_TEA, 0, 20);
    SetItemRestoration(ItemType::SNOW_BUNNY, 30, 0);
    SetItemRestoration(ItemType::SHROOM_CAKE, 30, 10);
    SetItemRestoration(ItemType::MOUSSE_CAKE, 10, 30);
    SetItemRestoration(ItemType::SHROOM_BROTH, 20, 20);
    SetItemRestoration(ItemType::SPICY_PASTA, 7, 7);
    SetItemRestoration(ItemType::FIRE_POP, 0, 30);
    SetItemRestoration(ItemType::HONEY_CANDY, 0, 15);
    SetItemRestoration(ItemType::JELLY_CANDY, 0, 50);
    SetItemRestoration(ItemType::HEALTHY_SALAD, 10, 10);

    // Fire Flower now burns, analogous with Ice Storm.
    ttyd::battle_item_data::ItemWeaponData_Fire_Flower.burn_time = 2;
    ttyd::battle_item_data::ItemWeaponData_Fire_Flower.burn_chance = 100;

    // Earth Quake now flips enemies, like POW Block, and deals more damage.
    ttyd::battle_item_data::ItemWeaponData_Yurayura_Jishin.
        damage_function_params[0] = 6;
    ttyd::battle_item_data::ItemWeaponData_Yurayura_Jishin.
        special_property_flags |= AttackSpecialProperty_Flags::FLIPS_SHELLED;
    ttyd::battle_item_data::ItemWeaponData_Yurayura_Jishin.
        special_property_flags |= AttackSpecialProperty_Flags::FLIPS_BOMB;
    // Reference new list description.
    itemDataTable[ItemType::EARTH_QUAKE].menu_description = "list_yurayura_jishin";
        
    // Make Trade Off usable only on the enemy party.
    ttyd::battle_item_data::ItemWeaponData_Teki_Kyouka.target_class_flags =
        0x02100063;
    // Make it inflict +ATK for 9 turns.
    ttyd::battle_item_data::ItemWeaponData_Teki_Kyouka.atk_change_chance = 100;
    ttyd::battle_item_data::ItemWeaponData_Teki_Kyouka.atk_change_time     = 9;
    ttyd::battle_item_data::ItemWeaponData_Teki_Kyouka.atk_change_strength = 3;
    // Patch in evt code to actually apply the item's newly granted status.
    mod::writePatch(
        reinterpret_cast<void*>(g_ItemEvent_Teki_Kyouka_ApplyStatusHook),
        TradeOffPatch, sizeof(TradeOffPatch));
    // Change "rank up" code to increase coins instead of level.
    g_BattleItemData_rank_up_trampoline = mod::hookFunction(
        ttyd::battle_item_data::BattleItemData_rank_up,
        [](EvtEntry* evt, bool isFirstCall) {
            //  Replace original logic.
            int32_t unit_id = evtGetValue(evt, evt->evtArguments[0]);
            unit_id = ttyd::battle_sub::BattleTransID(evt, unit_id);
            auto* unit = ttyd::battle::BattleGetUnitPtr(
                ttyd::battle::g_BattleWork, unit_id);
            // Use padding byte as storage for extra coins.
            unit->pad_00f += 10;
            if (unit->pad_00f > 50) unit->pad_00f = 50;
            return 2;
        });
    
    // Slow Shroom and Gradual Syrup have stronger effect, but only last 3 turns.
    ttyd::battle_item_data::ItemWeaponData_Jiwajiwa_Kinoko.hp_regen_strength = 5;
    ttyd::battle_item_data::ItemWeaponData_Jiwajiwa_Kinoko.hp_regen_time = 3;
    ttyd::battle_item_data::ItemWeaponData_Jiwajiwa_Syrup.fp_regen_strength = 5;
    ttyd::battle_item_data::ItemWeaponData_Jiwajiwa_Syrup.fp_regen_time = 3;

    // Tasty Tonic and Healthy Salad cure ATK-Down.
    ttyd::battle_item_data::ItemWeaponData_Sukkiri_Drink.atk_change_strength = 0;
    ttyd::battle_item_data::ItemWeaponData_Sukkiri_Drink.atk_change_time = 0;
    ttyd::battle_item_data::ItemWeaponData_Sukkiri_Drink.atk_change_chance = 100;
    ttyd::battle_item_data::ItemWeaponData_HealthySalad.atk_change_strength = 0;
    ttyd::battle_item_data::ItemWeaponData_HealthySalad.atk_change_time = 0;
    ttyd::battle_item_data::ItemWeaponData_HealthySalad.atk_change_chance = 100;
        
    // Strawberry Cake now restores a random amount of HP/FP (5,5 base).
    SetItemRestoration(ItemType::CAKE, 5, 5);
    // Run extra logic to increase the HP and FP restored at random.
    g_btlevtcmd_GetItemRecoverParam_trampoline = mod::hookFunction(
        ttyd::battle_event_cmd::btlevtcmd_GetItemRecoverParam,
        [](EvtEntry* evt, bool isFirstCall) {
            g_btlevtcmd_GetItemRecoverParam_trampoline(evt, isFirstCall);
            // Run custom behavior to replace the recovery params in some cases.
            return GetAlteredItemRestorationParams(evt, isFirstCall);
        });

    // Coconut is now a thrown damaging item that deals 6 non-piercing damage.
    static BattleWeapon kCoconutParams;
    memcpy(&kCoconutParams,
           &ttyd::battle_item_data::ItemWeaponData_CoconutsBomb,
           sizeof(BattleWeapon));
    kCoconutParams.item_id = ItemType::COCONUT;
    kCoconutParams.damage_function =
        reinterpret_cast<void*>(
            &ttyd::battle_weapon_power::weaponGetPowerDefault);
    kCoconutParams.damage_function_params[0] = 6;
    kCoconutParams.element = AttackElement::NORMAL;
    kCoconutParams.special_property_flags &=
        ~AttackSpecialProperty_Flags::DEFENSE_PIERCING;
    itemDataTable[ItemType::COCONUT].weapon_params = &kCoconutParams;
    // Can only be used in battle.
    itemDataTable[ItemType::COCONUT].usable_locations &= ~ItemUseLocation::kField;

    // Mystic Egg now restores 1/3 of max Star Power (in-battle only).
    SetItemRestoration(ItemType::MYSTIC_EGG, 0, 0);
    static BattleWeapon kMysticEggParams;
    memcpy(&kMysticEggParams,
           &ttyd::battle_item_data::ItemWeaponData_Tankobu,
           sizeof(BattleWeapon));
    kMysticEggParams.item_id = ItemType::MYSTIC_EGG;
    kMysticEggParams.attack_evt_code = const_cast<int32_t*>(MysticEggAttackEvent);
    itemDataTable[ItemType::MYSTIC_EGG].weapon_params = &kMysticEggParams;
    // Can only be used in battle.
    itemDataTable[ItemType::MYSTIC_EGG].usable_locations &= ~ItemUseLocation::kField;

    // Reduce Sleepy Sheep turn count to 3, to reduce frustration AND cheese.
    ttyd::battle_item_data::ItemWeaponData_Nemure_Yoikoyo.sleep_time = 3;

    // Hot Sauce charges by +3.
    ttyd::battle_item_data::ItemWeaponData_RedKararing.charge_strength = 3;
        
    // Make Koopa Curse multi-target, and no longer unable to miss.
    ttyd::battle_item_data::ItemWeaponData_Kameno_Noroi.
        target_class_flags =
            AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH |
            AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
            AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
            AttackTargetClass_Flags::CANNOT_TARGET_SELF |
            AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
            AttackTargetClass_Flags::MULTIPLE_TARGET;
    ttyd::battle_item_data::ItemWeaponData_Kameno_Noroi.
        special_property_flags &= ~AttackSpecialProperty_Flags::CANNOT_MISS;
    // Also, fix its missing icon.
    itemDataTable[ItemType::KOOPA_CURSE].icon_id = IconType::KOOPA_CURSE;

    // Zess Dynamite's damage increased, since it's harder to get.
    ttyd::battle_item_data::ItemWeaponData_NancyDynamite.
        damage_function_params[0] = 10;

    // Space Food no longer restores HP, but guarantees Allergic status.
    SetItemRestoration(ItemType::SPACE_FOOD, 0, 0);
    ttyd::battle_item_data::ItemWeaponData_SpaceFood.allergic_chance = 100;
    ttyd::battle_item_data::ItemWeaponData_SpaceFood.special_property_flags
        |= AttackSpecialProperty_Flags::IGNORES_STATUS_CHANCE;
    // Can only be used in battle.
    itemDataTable[ItemType::SPACE_FOOD].usable_locations
        &= ~ItemUseLocation::kField;

    // Snow Bunny guarantees one-turn Freeze and deals base 0 Ice damage.
    ttyd::battle_item_data::ItemWeaponData_SnowRabbit.damage_function =
        reinterpret_cast<void*>(
            &ttyd::battle_weapon_power::weaponGetPowerDefault);
    ttyd::battle_item_data::ItemWeaponData_SnowRabbit.
        damage_function_params[0] = 0;
    ttyd::battle_item_data::ItemWeaponData_SnowRabbit.element = 
        AttackElement::ICE;
    ttyd::battle_item_data::ItemWeaponData_SnowRabbit.special_property_flags 
        |= AttackSpecialProperty_Flags::DEFENSE_PIERCING;
    ttyd::battle_item_data::ItemWeaponData_SnowRabbit.special_property_flags
        |= AttackSpecialProperty_Flags::IGNORES_STATUS_CHANCE;
    ttyd::battle_item_data::ItemWeaponData_SnowRabbit.freeze_chance = 100;
    ttyd::battle_item_data::ItemWeaponData_SnowRabbit.freeze_time = 2;
    // Set target weighting flags to be same as basic HP items.
    ttyd::battle_item_data::ItemWeaponData_SnowRabbit.target_weighting_flags
        = ttyd::battle_item_data::ItemWeaponData_Kinoko.target_weighting_flags;
    // Can only be used in battle, to ensure you deal with the negative effect.
    itemDataTable[ItemType::SNOW_BUNNY].usable_locations 
        &= ~ItemUseLocation::kField;
    
    // Fire Pop guarantees one-turn Burn and deals base 0 Fire damage.
    static BattleWeapon kFirePopParams;
    memcpy(&kFirePopParams,
           &ttyd::battle_item_data::ItemWeaponData_BiribiriCandy,
           sizeof(BattleWeapon));
    kFirePopParams.item_id = ItemType::FIRE_POP;
    kFirePopParams.damage_function =
        reinterpret_cast<void*>(
            &ttyd::battle_weapon_power::weaponGetPowerDefault);
    kFirePopParams.damage_function_params[0] = 0;
    kFirePopParams.element = AttackElement::FIRE;
    kFirePopParams.special_property_flags 
        |= AttackSpecialProperty_Flags::DEFENSE_PIERCING;
    kFirePopParams.special_property_flags
        |= AttackSpecialProperty_Flags::IGNORES_STATUS_CHANCE;
    kFirePopParams.electric_chance = 0;
    kFirePopParams.electric_time = 0;
    kFirePopParams.burn_chance = 100;
    kFirePopParams.burn_time = 2;
    itemDataTable[ItemType::FIRE_POP].weapon_params = &kFirePopParams;
    // Can only be used in battle, to ensure you deal with the negative effect.
    itemDataTable[ItemType::FIRE_POP].usable_locations 
        &= ~ItemUseLocation::kField;

    // Shroom Broth drops ATK and DEF by 2 for 3 turns, has no HP-regen.
    SetItemRestoration(ItemType::SHROOM_BROTH, 20, 20);
    ttyd::battle_item_data::ItemWeaponData_TeaKinoko.atk_change_strength = -2;
    ttyd::battle_item_data::ItemWeaponData_TeaKinoko.atk_change_chance = 100;
    ttyd::battle_item_data::ItemWeaponData_TeaKinoko.atk_change_time = 3;
    ttyd::battle_item_data::ItemWeaponData_TeaKinoko.def_change_strength = -2;
    ttyd::battle_item_data::ItemWeaponData_TeaKinoko.def_change_chance = 100;
    ttyd::battle_item_data::ItemWeaponData_TeaKinoko.def_change_time = 3;
    ttyd::battle_item_data::ItemWeaponData_TeaKinoko.hp_regen_strength = 0;
    ttyd::battle_item_data::ItemWeaponData_TeaKinoko.hp_regen_time = 0;
    // Set target weighting flags to be same as basic HP items.
    ttyd::battle_item_data::ItemWeaponData_TeaKinoko.target_weighting_flags
        = ttyd::battle_item_data::ItemWeaponData_Kinoko.target_weighting_flags;
    // Can only be used in battle, to ensure you deal with the negative effect.
    itemDataTable[ItemType::SHROOM_BROTH].usable_locations 
        &= ~ItemUseLocation::kField;

    // Courage Meal drops enemy DEF by 3.
    ttyd::battle_item_data::ItemWeaponData_KachikachiDish.def_change_strength = -3;
    ttyd::battle_item_data::ItemWeaponData_KachikachiDish.def_change_chance = 100;
    ttyd::battle_item_data::ItemWeaponData_KachikachiDish.def_change_time = 3;

    // Fruit Parfait restores HP and FP immediately + over time.
    SetItemRestoration(ItemType::FRUIT_PARFAIT, 5, 5);
    static BattleWeapon kFruitParfaitParams;
    memcpy(&kFruitParfaitParams,
           &ttyd::battle_item_data::ItemWeaponData_BiribiriCandy,
           sizeof(BattleWeapon));
    kFruitParfaitParams.item_id = ItemType::FRUIT_PARFAIT;
    kFruitParfaitParams.electric_chance = 0;
    kFruitParfaitParams.electric_time = 0;
    kFruitParfaitParams.hp_regen_strength = 5;
    kFruitParfaitParams.hp_regen_time = 3;
    kFruitParfaitParams.fp_regen_strength = 5;
    kFruitParfaitParams.fp_regen_time = 3;
    itemDataTable[ItemType::FRUIT_PARFAIT].weapon_params = &kFruitParfaitParams;

    // Make Poison Shrooms able to target anyone, and make enemies prefer
    // to target Mario's team or characters that are in Peril.
    ttyd::battle_item_data::ItemWeaponData_PoisonKinoko.target_class_flags = 
        0x01100060;
    ttyd::battle_item_data::ItemWeaponData_PoisonKinoko.target_weighting_flags =
        0x80001023;
    // Make Poison Mushrooms poison & halve HP 67% of the time instead of 80%.
    mod::writePatch(
        reinterpret_cast<void*>(g_ItemEvent_Poison_Kinoko_PoisonChance), 67);
    // Add a wrapper to attack event that makes Fuzzy Horde unaffected.
    ttyd::battle_item_data::ItemWeaponData_PoisonKinoko.attack_evt_code =
        const_cast<int32_t*>(PoisonShroomWrapperEvent);

    // Meteor Meal restores HP and FP over time.
    SetItemRestoration(ItemType::METEOR_MEAL, 0, 0);
    ttyd::battle_item_data::ItemWeaponData_StarryDinner.hp_regen_strength = 5;
    ttyd::battle_item_data::ItemWeaponData_StarryDinner.fp_regen_strength = 5;
    ttyd::battle_item_data::ItemWeaponData_StarryDinner.hp_regen_time = 3;
    ttyd::battle_item_data::ItemWeaponData_StarryDinner.fp_regen_time = 3;
    // Special attack event also causes SP restoration over time.
    ttyd::battle_item_data::ItemWeaponData_StarryDinner.attack_evt_code 
        = const_cast<int32_t*>(MeteorMealAttackEvent);
    // Can only be used in battle.
    itemDataTable[ItemType::METEOR_MEAL].usable_locations &= ~ItemUseLocation::kField;

    // Hottest Dog restores less HP,FP than Spicy Pasta used to but Charges +3.
    SetItemRestoration(ItemType::SPICY_PASTA, 7, 7);
    ttyd::battle_item_data::ItemWeaponData_KararinaPasta.charge_strength = 3;
    // Use new icon.
    itemDataTable[ItemType::SPICY_PASTA].icon_id = IconType::HOTTEST_DOG;
        
    // Make Point Swap and Trial Stew only target Mario or his partner.
    ttyd::battle_item_data::ItemWeaponData_Irekaeeru.target_class_flags = 
        0x01100070;
    ttyd::battle_item_data::ItemWeaponData_LastDinner.target_class_flags = 
        0x01100070;
    // Clear the "last Mario attacker" play stat when Point Swap is used,
    // so a self-destruct doesn't get falsely attributed to an enemy.
    g_BattleItemData_hpfp_change_declare_1_trampoline = mod::hookFunction(
        ttyd::battle_item_data::BattleItemData_hpfp_change_declare_1,
        [](EvtEntry* evt, bool isFirstCall) {
            g_Mod->state_.SetOption(STAT_PERM_LAST_ATTACKER, 0);
            return g_BattleItemData_hpfp_change_declare_1_trampoline(evt, isFirstCall);
        });
        
    // Make Trial Stew's event use the correct weapon params.
    BattleWeapon* kLastDinnerWeaponAddr = 
        &ttyd::battle_item_data::ItemWeaponData_LastDinner;
    mod::writePatch(
        reinterpret_cast<void*>(g_ItemEvent_LastDinner_Weapon),
        &kLastDinnerWeaponAddr, sizeof(BattleWeapon*));

    // Love Pudding - Applies a random positive status to your alliance.
    static BattleWeapon kLovePuddingParams;
    memcpy(&kLovePuddingParams,
           &ttyd::battle_item_data::ItemWeaponData_Kameno_Noroi,
           sizeof(BattleWeapon));
    kLovePuddingParams.item_id = ItemType::LOVE_PUDDING;
    kLovePuddingParams.target_class_flags =
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_OPPOSING_ALLIANCE |
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::MULTIPLE_TARGET;
    kLovePuddingParams.special_property_flags |=
        AttackSpecialProperty_Flags::CANNOT_MISS |
        AttackSpecialProperty_Flags::IGNORES_STATUS_CHANCE |
        AttackSpecialProperty_Flags::USABLE_IF_CONFUSED;
    kLovePuddingParams.attack_evt_code = 
        const_cast<int32_t*>(RngStatusAttackEvent);
    itemDataTable[ItemType::LOVE_PUDDING].weapon_params = &kLovePuddingParams;

    // Peach Tart - Applies a random negative status to opposing alliance.
    static BattleWeapon kPeachTartParams;
    memcpy(&kPeachTartParams, &kLovePuddingParams, sizeof(BattleWeapon));
    kPeachTartParams.item_id = ItemType::PEACH_TART;
    kPeachTartParams.target_class_flags =
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::MULTIPLE_TARGET;
    kPeachTartParams.special_property_flags |=
        AttackSpecialProperty_Flags::IGNORES_STATUS_CHANCE |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING |
        AttackSpecialProperty_Flags::USABLE_IF_CONFUSED;
    itemDataTable[ItemType::PEACH_TART].weapon_params = &kPeachTartParams;


    // Badge changes...
    
    // New badges:
    // Toughen Up (P): a move that grants a single-turn +DEF buff.
    itemDataTable[ItemType::TOT_TOUGHEN_UP].icon_id = IconType::DEFEND_BADGE;
    itemDataTable[ItemType::TOT_TOUGHEN_UP].name = "in_toughen_up";
    itemDataTable[ItemType::TOT_TOUGHEN_UP].description = "msg_toughen_up";
    itemDataTable[ItemType::TOT_TOUGHEN_UP].menu_description = "msg_toughen_up_menu";
    itemDataTable[ItemType::TOT_TOUGHEN_UP_P].icon_id = IconType::DEFEND_BADGE_P;
    itemDataTable[ItemType::TOT_TOUGHEN_UP_P].name = "in_toughen_up_p";
    itemDataTable[ItemType::TOT_TOUGHEN_UP_P].description = "msg_toughen_up_p";
    itemDataTable[ItemType::TOT_TOUGHEN_UP_P].menu_description = "msg_toughen_up_p_menu";
    // Perfect Power (P): gives +1 power for being at full health.
    itemDataTable[ItemType::TOT_PERFECT_POWER].icon_id = IconType::PERFECT_POWER;
    itemDataTable[ItemType::TOT_PERFECT_POWER].name = "in_perfect_power";
    itemDataTable[ItemType::TOT_PERFECT_POWER].description = "msg_perfect_power";
    itemDataTable[ItemType::TOT_PERFECT_POWER].menu_description = "msg_perfect_power";
    itemDataTable[ItemType::TOT_PERFECT_POWER_P].icon_id = IconType::PERFECT_POWER_P;
    itemDataTable[ItemType::TOT_PERFECT_POWER_P].name = "in_perfect_power_p";
    itemDataTable[ItemType::TOT_PERFECT_POWER_P].description = "msg_perfect_power_p";
    itemDataTable[ItemType::TOT_PERFECT_POWER_P].menu_description = "msg_perfect_power_p";
    // Pity Star (P): gives extra Star Power from taking hits from enemies.
    itemDataTable[ItemType::TOT_PITY_STAR].icon_id = IconType::PITY_STAR;
    itemDataTable[ItemType::TOT_PITY_STAR].name = "in_pity_star";
    itemDataTable[ItemType::TOT_PITY_STAR].description = "msg_pity_star";
    itemDataTable[ItemType::TOT_PITY_STAR].menu_description = "msg_pity_star";
    itemDataTable[ItemType::TOT_PITY_STAR_P].icon_id = IconType::PITY_STAR_P;
    itemDataTable[ItemType::TOT_PITY_STAR_P].name = "in_pity_star_p";
    itemDataTable[ItemType::TOT_PITY_STAR_P].description = "msg_pity_star_p";
    itemDataTable[ItemType::TOT_PITY_STAR_P].menu_description = "msg_pity_star_p";
    // Super Start: gives +0.50 SP at the start of a battle.
    itemDataTable[ItemType::TOT_SUPER_START].icon_id = IconType::SUPER_START;
    itemDataTable[ItemType::TOT_SUPER_START].name = "in_super_start";
    itemDataTable[ItemType::TOT_SUPER_START].description = "msg_super_start";
    itemDataTable[ItemType::TOT_SUPER_START].menu_description = "msg_super_start";
        
    // Change Super Charge (P) weapons into Toughen Up (P) move.
    // Currently unused.
    ttyd::battle_mario::badgeWeapon_SuperCharge.base_fp_cost = 1;
    ttyd::battle_mario::badgeWeapon_SuperCharge.charge_strength = 0;
    ttyd::battle_mario::badgeWeapon_SuperCharge.def_change_chance = 100;
    ttyd::battle_mario::badgeWeapon_SuperCharge.def_change_time = 1;
    ttyd::battle_mario::badgeWeapon_SuperCharge.def_change_strength = 2;
    ttyd::battle_mario::badgeWeapon_SuperCharge.icon = IconType::DEFEND_BADGE;
    ttyd::battle_mario::badgeWeapon_SuperCharge.name = "in_toughen_up";
    ttyd::battle_mario::badgeWeapon_SuperChargeP.base_fp_cost = 1;
    ttyd::battle_mario::badgeWeapon_SuperChargeP.charge_strength = 0;
    ttyd::battle_mario::badgeWeapon_SuperChargeP.def_change_chance = 100;
    ttyd::battle_mario::badgeWeapon_SuperChargeP.def_change_time = 1;
    ttyd::battle_mario::badgeWeapon_SuperChargeP.def_change_strength = 2;
    ttyd::battle_mario::badgeWeapon_SuperChargeP.icon = IconType::DEFEND_BADGE_P;
    ttyd::battle_mario::badgeWeapon_SuperChargeP.name = "in_toughen_up";
        
    // Change base FP cost of Charge badges.
    ttyd::battle_mario::badgeWeapon_Charge.base_fp_cost = 2;
    ttyd::battle_mario::badgeWeapon_ChargeP.base_fp_cost = 2;
    
    // For testing purposes, Bump Attack ignores level check.
    mod::writePatch(
        reinterpret_cast<void*>(g_fbatBattleMode_Patch_BumpAttackLevel),
        0x60000000U /* nop */);
        
    // Super Appeal (P) give +0.50 SP per copy instead of +0.25.
    mod::writePatch(
        reinterpret_cast<void*>(g_BattleAudience_Case_Appeal_Patch_AppealSp),
        0x1c000032U /* mulli r0, r0, 50 */);
        
    // Happy badges are guaranteed to proc on even turns only.
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_btlseqTurn_HappyHeartProc_BH),
        reinterpret_cast<void*>(g_btlseqTurn_HappyHeartProc_EH),
        reinterpret_cast<void*>(StartHappyHeartProc),
        reinterpret_cast<void*>(BranchBackHappyHeartProc));
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_btlseqTurn_HappyFlowerProc_BH),
        reinterpret_cast<void*>(g_btlseqTurn_HappyFlowerProc_EH),
        reinterpret_cast<void*>(StartHappyFlowerProc),
        reinterpret_cast<void*>(BranchBackHappyFlowerProc));
        
    // Pity Flower (P) guarantees 1 FP recovery on each damaging hit.
    mod::writePatch(
        reinterpret_cast<void*>(g_BattleDamageDirect_Patch_PityFlowerChance),
        0x2c030064U /* cmpwi r3, 100 */);
        
    // Refund grants 100% of sell price, plus 25% per additional badge.
    mod::writePatch(
        reinterpret_cast<void*>(g_btlevtcmd_ConsumeItem_Patch_RefundPer),
        0x1ca00019U /* mulli r5, r0, 25 */);
    mod::writePatch(
        reinterpret_cast<void*>(g_btlevtcmd_ConsumeItemReserve_Patch_RefundPer),
        0x1ca00019U /* mulli r5, r0, 25 */);
    mod::writePatch(
        reinterpret_cast<void*>(g_btlevtcmd_ConsumeItem_Patch_RefundBase),
        0x38a5004bU /* addi r5, r5, 75 */);
    mod::writePatch(
        reinterpret_cast<void*>(g_btlevtcmd_ConsumeItemReserve_Patch_RefundBase),
        0x38a5004bU /* addi r5, r5, 75 */);
    
    // Replace HP/FP Drain logic; counts the number of intended damaging hits
    // and restores 1 HP per badge if there were any (or 1 per hit, to a max
    // of 5, if the PM64-style option is enabled).
    g__get_heart_suitoru_point_trampoline = mod::hookFunction(
        ttyd::battle_event_default::_get_heart_suitoru_point,
        [](EvtEntry* evt, bool isFirstCall) {
            return GetDrainRestoration(evt, /* hp_drain = */ true);
        });
    g__get_flower_suitoru_point_trampoline = mod::hookFunction(
        ttyd::battle_event_default::_get_flower_suitoru_point,
        [](EvtEntry* evt, bool isFirstCall) {
            return GetDrainRestoration(evt, /* hp_drain = */ false);
        });
    // Disable the instruction that normally adds to damage dealt, since that
    // field is now used as a boolean for "has attacked with a damaging move".
    mod::writePatch(
        reinterpret_cast<void*>(g_BattleDamageDirect_Patch_AddTotalDamage),
        0x60000000U /* nop */);


    // General hooks...
    
    // Make enemies prefer to use CookingItems like standard healing items.
    // (i.e. they use them on characters with less HP)
    ttyd::battle_item_data::ItemWeaponData_CookingItem.target_weighting_flags =
        ttyd::battle_item_data::ItemWeaponData_Kinoko.target_weighting_flags;
    // Change CookingItem's attack event to CookingItemAttack (wrapper to
    // ItemEvent_Recovery that passes in the item type being used for enemies).
    ttyd::battle_item_data::ItemWeaponData_CookingItem.attack_evt_code =
        const_cast<int32_t*>(CookingItemAttackEvent);
        
    // Set max inventory size based on number of sack upgrades claimed in ToT.
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_pouchGetItem_CheckMaxInv_BH),
        reinterpret_cast<void*>(StartGetItemMax),
        reinterpret_cast<void*>(BranchBackGetItemMax));
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_pouchRemoveItem_CheckMaxInv_BH),
        reinterpret_cast<void*>(StartRemoveItemMax),
        reinterpret_cast<void*>(BranchBackRemoveItemMax));
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_pouchRemoveItemIndex_CheckMaxInv_BH),
        reinterpret_cast<void*>(StartRemoveItemIndexMax),
        reinterpret_cast<void*>(BranchBackRemoveItemIndexMax));
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_pouchGetEmptyHaveItemCnt_CheckMaxInv_BH),
        reinterpret_cast<void*>(StartGetEmptyItemSlotsMax),
        reinterpret_cast<void*>(BranchBackGetEmptyItemSlotsMax));
        
    // Override item pickup script for special items that spawn as item drops.
    g_itemEntry_trampoline = mod::hookFunction(
        ttyd::itemdrv::itemEntry,
        [](const char* name, int32_t item, float x, float y, float z,
           uint32_t mode, int32_t collection_gswf, void* pickup_script) {
            if (item == ItemType::STAR_PIECE && !pickup_script) {
                pickup_script = RewardManager::GetStarPieceItemDropEvt();
            }
            if (item == ItemType::SHINE_SPRITE && !pickup_script) {
                pickup_script = RewardManager::GetShineSpriteItemDropEvt();
            }
            return g_itemEntry_trampoline(
                name, item, x, y, z, mode, collection_gswf, pickup_script);
        });
    
    // Override item-get logic for special items.
    g_pouchGetItem_trampoline = mod::hookFunction(
        ttyd::mario_pouch::pouchGetItem, [](int32_t item_type) {            
            // Handle items with special effects in ToT.
            if (RewardManager::HandleRewardItemPickup(item_type)) 
                return 1U;
            
            uint32_t return_value = g_pouchGetItem_trampoline(item_type);
            
            // Track coins, Star Pieces, and Shine Sprites gained.
            if (item_type == ItemType::COIN) {
                g_Mod->state_.ChangeOption(STAT_RUN_COINS_EARNED);
                g_Mod->state_.ChangeOption(STAT_PERM_COINS_EARNED);
                AchievementsManager::CheckCompleted(
                    AchievementId::MISC_RUN_COINS_999);
            }
            if (item_type == ItemType::STAR_PIECE) {
                g_Mod->state_.ChangeOption(STAT_RUN_STAR_PIECES);
                g_Mod->state_.ChangeOption(STAT_PERM_STAR_PIECES);
            }
            if (item_type == ItemType::SHINE_SPRITE) {
                g_Mod->state_.ChangeOption(STAT_RUN_SHINE_SPRITES);
                g_Mod->state_.ChangeOption(STAT_PERM_SHINE_SPRITES);
                if (g_Mod->state_.GetOption(STAT_RUN_SHINE_SPRITES) > 9) {
                    AchievementsManager::MarkCompleted(
                        AchievementId::MISC_SHINES_10);
                }
            }

            // Mark unique items as being collected.
            if (return_value) {
                RewardManager::MarkUniqueItemCollected(item_type);
            }

            // Mark regular items / badges as being encountered.
            if (return_value && item_type >= ItemType::THUNDER_BOLT) {
                g_Mod->state_.SetOption(FLAGS_ITEM_ENCOUNTERED, item_type - 0x80);

                // Check for Item / Badge log completion.
                AchievementsManager::CheckCompleted(AchievementId::META_ITEM_LOG_BASIC);
                AchievementsManager::CheckCompleted(AchievementId::META_ITEM_LOG_ALL);
                AchievementsManager::CheckCompleted(AchievementId::META_BADGE_LOG_ALL);
            }

            // If getting a key item, sort by type + check for all collected.
            if (item_type >= ItemType::TOT_KEY_PEEKABOO && 
                item_type < ItemType::TOT_KEY_ITEM_MAX) {
                // Sort key items by type.
                ttyd::mario_pouch::pouchSortItem(3);
                AchievementsManager::CheckCompleted(AchievementId::META_ALL_KEY_ITEMS);
            }
            
            return return_value;
        });

    // Add additional check for items not despawning while a Star Piece
    // or Shine Sprite is being collected on the field.
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_itemMain_CheckItemFreeze_BH),
        reinterpret_cast<void*>(StartCheckItemFreeze),
        reinterpret_cast<void*>(BranchBackCheckItemFreeze));
}

int32_t GetBonusCakeRestoration() {
    // Returns one of 0, 5, 10, ..., 25 at random (to be added to the base 5).
    return ttyd::system::irand(6) * 5;
}

int32_t GetItemInventorySize() {
    // Returns the current inventory size based on Strange Sack upgrades.
    int32_t items = g_Mod->state_.max_inventory_;
    return items < 20 ? items : 20;
}

EVT_DEFINE_USER_FUNC(evtTot_FreezeFieldItemTimers) {
    g_FreezeFieldItems = evtGetValue(evt, evt->evtArguments[0]);
    return 2;
}

}  // namespace item
}  // namespace mod::tot::patch