#include "patches_options.h"

#include "common_functions.h"
#include "custom_item.h"
#include "mod.h"
#include "mod_state.h"
#include "patch.h"

#include <ttyd/battle.h>
#include <ttyd/battle_actrecord.h>
#include <ttyd/battle_audience.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_mario.h>
#include <ttyd/battle_seq_end.h>
#include <ttyd/battle_stage_object.h>
#include <ttyd/battle_unit.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/system.h>

#include <cstdint>
#include <cstring>

// Assembly patch functions.
extern "C" {
    // action_menu_patches.s
    void StartSpendFpOnSwitchPartner();
    void BranchBackSpendFpOnSwitchPartner();
    // audience_item_patches.s
    void StartAudienceItem();
    void BranchBackAudienceItem();
    void StartAudienceItemSpaceFix();
    void BranchBackAudienceItemSpaceFix();
    // danger_threshold_patches.s
    void StartSetDangerThreshold();
    void BranchBackSetDangerThreshold();
    void StartSetPerilThreshold();
    void BranchBackSetPerilThreshold();
    void StartCheckMarioPinchDisp();
    void BranchBackCheckMarioPinchDisp();
    void StartCheckPartnerPinchDisp();
    void BranchBackCheckPartnerPinchDisp();
    // star_power_patches.s
    void StartEnableAppealCheck();
    void BranchBackEnableAppealCheck();
    void StartAddAudienceCheck();
    void BranchBackAddAudienceCheck();
    void StartDisplayAudienceCheck();
    void BranchBackDisplayAudienceCheck();
    void StartSaveAudienceCountCheck();
    void BranchBackSaveAudienceCountCheck();
    void StartSetInitialAudienceCheck();
    void BranchBackSetInitialAudienceCheck();
    void StartObjectFallOnAudienceCheck();
    void BranchBackObjectFallOnAudienceCheck();
    void StartAddPuniToAudienceCheck();
    void BranchBackAddPuniToAudienceCheck();
    void StartEnableIncrementingBingoCheck();
    void BranchBackEnableIncrementingBingoCheck();
    
    void spendFpOnSwitchPartner(ttyd::battle_unit::BattleWorkUnit* unit) {
        mod::infinite_pit::options::SpendFpOnSwitchingPartner(unit);
    }
    int32_t getAudienceItem(int32_t item_type) {
        return mod::infinite_pit::options::GetRandomAudienceItem(item_type);
    }
    uint32_t audienceFixItemSpaceCheck(
        uint32_t empty_item_slots, uint32_t item_type) {
        return mod::infinite_pit::options::FixAudienceItemSpaceCheck(
            empty_item_slots, item_type);
    }
    ttyd::battle_database_common::BattleUnitKind* setPinchThreshold(
        ttyd::battle_database_common::BattleUnitKind* kind,
        int32_t max_hp, int32_t base_max_hp, bool peril) {
        // Ignore HP Plus badges for enemies.
        if (kind->unit_type <= 
            ttyd::battle_database_common::BattleUnitType::BONETAIL) {
            max_hp = base_max_hp;
        }
        mod::infinite_pit::options::SetPinchThreshold(kind, max_hp, peril);
        return kind;
    }
    bool checkStarPowersEnabled() {
        // Star Powers will always be enabled in v2.
        return true;
    }
}

namespace mod::infinite_pit {

namespace {

using ::ttyd::battle_database_common::BattleUnitKind;
using ::ttyd::battle_database_common::BattleWeapon;
using ::ttyd::evtmgr::EvtEntry;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;
using ::ttyd::item_data::itemDataTable;

namespace ItemType = ::ttyd::item_data::ItemType;

}

// Function hooks.
extern void (*g_pouchReviseMarioParam_trampoline)();
extern int32_t (*g_btlevtcmd_WeaponAftereffect_trampoline)(EvtEntry*, bool);
extern void (*g_BattleAudienceSetThrowItemMax_trampoline)();
// Patch addresses.
extern const int32_t g__btlcmd_SetAttackEvent_SwitchPartnerCost_BH;
extern const int32_t g_BtlUnit_CheckPinchStatus_DangerThreshold_BH;
extern const int32_t g_BtlUnit_CheckPinchStatus_PerilThreshold_BH;
extern const int32_t g_DrawMenuMarioPinchMark_CheckThreshold_BH;
extern const int32_t g_DrawMenuMarioPinchMark_Patch_CheckResult1;
extern const int32_t g_DrawMenuMarioPinchMark_Patch_CheckResult2;
extern const int32_t g_DrawMenuPartyPinchMark_CheckThreshold_BH;
extern const int32_t g_DrawMenuPartyPinchMark_Patch_CheckResult1;
extern const int32_t g_DrawMenuPartyPinchMark_Patch_CheckResult2;
extern const int32_t g__btlcmd_MakeOperationTable_AppealAlways_BH;
extern const int32_t g_BattleAudienceAddAudienceNum_EnableAlways_BH;
extern const int32_t g_BattleAudience_Disp_EnableAlways_BH;
extern const int32_t g_BattleAudience_End_SaveAmountAlways_BH;
extern const int32_t g_BattleAudienceSettingAudience_EnableAlways_BH;
extern const int32_t g__object_fall_attack_AudienceEnableAlways_BH;
extern const int32_t g_BattleAudienceAddPuni_EnableAlways_BH;
extern const int32_t g_BattleBreakSlot_PointInc_EnableAlways_BH;
extern const int32_t g_BattleAudienceItemOn_RandomItem_BH;
extern const int32_t g_BattleAudienceItemCtrlProcess_Patch_CheckItemValidRange;
extern const int32_t g_BattleAudienceItemCtrlProcess_CheckSpace_BH;

namespace options {

void ApplyFixedPatches() {
    // Change the chances of weapon-induced stage effects based on options.
    g_btlevtcmd_WeaponAftereffect_trampoline = patch::hookFunction(
        ttyd::battle_event_cmd::btlevtcmd_WeaponAftereffect,
        [](EvtEntry* evt, bool isFirstCall) {
            // Make sure the stage jet type is initialized, if possible.
            auto* battleWork = ttyd::battle::g_BattleWork;
            if (ttyd::mario_pouch::pouchGetPtr()->rank > 0) {
                ttyd::battle_stage_object::_nozzle_type_init();
            }
            
            int8_t stage_hazard_chances[12];
            // Store the original stage hazard chances.
            auto& weapon = *reinterpret_cast<BattleWeapon*>(
                evtGetValue(evt, evt->evtArguments[0]));
            memcpy(stage_hazard_chances, &weapon.bg_a1_a2_fall_weight, 12);
            
            // Get the percentage to scale original chances by.
            int32_t scale = 100;
            switch (g_Mod->state_.GetOptionValue(OPT_STAGE_HAZARDS)) {
                case OPTVAL_STAGE_HAZARDS_HIGH: {
                    scale = 250;
                    break;
                }
                case OPTVAL_STAGE_HAZARDS_LOW: {
                    scale = 50;
                    break;
                }
                case OPTVAL_STAGE_HAZARDS_OFF: {
                    scale = 0;
                    break;
                }
                case OPTVAL_STAGE_HAZARDS_NO_FOG: {
                    // If stage jets are uninitialized or fog-type jets,
                    // make them unable to fire.
                    if (battleWork->stage_hazard_work.current_stage_jet_type
                        <= 0) {
                        weapon.nozzle_turn_chance = 0;
                        weapon.nozzle_fire_chance = 0;
                    }
                    break;
                }
                default: break;
            }
            
            // Change the parameter values according to the selected scale.
            weapon.bg_a1_fall_weight = 
                Min((weapon.bg_a1_fall_weight * scale + 50) / 100, 100);
            weapon.bg_a2_fall_weight = 
                Min((weapon.bg_a2_fall_weight * scale + 50) / 100, 100);
            weapon.bg_b_fall_weight = 
                Min((weapon.bg_b_fall_weight * scale + 50) / 100, 100);
            weapon.nozzle_turn_chance = 
                Min((weapon.nozzle_turn_chance * scale + 50) / 100, 100);
            weapon.nozzle_fire_chance = 
                Min((weapon.nozzle_fire_chance * scale + 50) / 100, 100);
            weapon.ceiling_fall_chance = 
                Min((weapon.ceiling_fall_chance * scale + 50) / 100, 100);
            weapon.object_fall_chance = 
                Min((weapon.object_fall_chance * scale + 50) / 100, 100);
            weapon.unused_stage_hazard_chance = 
                Min((weapon.unused_stage_hazard_chance * scale + 50) / 100, 100);
            
            // Call the function that induces stage effects.
            g_btlevtcmd_WeaponAftereffect_trampoline(evt, isFirstCall);
            
            // Copy back original values.
            memcpy(&weapon.bg_a1_a2_fall_weight, stage_hazard_chances, 12);
            
            return 2;
        });
        
    // Guarantee that 1 item can be thrown every turn if randomized audience
    // items are enabled.
    g_BattleAudienceSetThrowItemMax_trampoline = mod::patch::hookFunction(
        ttyd::battle_audience::BattleAudienceSetThrowItemMax, [](){
            g_BattleAudienceSetThrowItemMax_trampoline();
            if (g_Mod->state_.GetOptionNumericValue(
                OPT_AUDIENCE_RANDOM_THROWS)) {
                // Always set 'max thrown items' value to 1.
                uintptr_t audience_work_base =
                    reinterpret_cast<uintptr_t>(
                        ttyd::battle::g_BattleWork->audience_work);
                *reinterpret_cast<int32_t*>(audience_work_base + 0x137f4) = 1;
            }
        });
    
    // Apply patch to give the player infinite BP if enabled.
    g_pouchReviseMarioParam_trampoline = mod::patch::hookFunction(
        ttyd::mario_pouch::pouchReviseMarioParam, [](){
            g_pouchReviseMarioParam_trampoline();
            if (g_Mod->state_.CheckOptionValue(OPTVAL_NO_EXP_MODE_INFINITE) &&
                !strcmp(GetCurrentArea(), "jon")) {
                ttyd::mario_pouch::pouchGetPtr()->unallocated_bp = 99;
            }
        });
        
    // Add code that subtracts FP for switching partners (if option enabled).
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g__btlcmd_SetAttackEvent_SwitchPartnerCost_BH),
        reinterpret_cast<void*>(StartSpendFpOnSwitchPartner),
        reinterpret_cast<void*>(BranchBackSpendFpOnSwitchPartner));
        
    // Override the default Danger / Peril threshold checks for all actors.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_BtlUnit_CheckPinchStatus_DangerThreshold_BH),
        reinterpret_cast<void*>(StartSetDangerThreshold),
        reinterpret_cast<void*>(BranchBackSetDangerThreshold));
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_BtlUnit_CheckPinchStatus_PerilThreshold_BH),
        reinterpret_cast<void*>(StartSetPerilThreshold),
        reinterpret_cast<void*>(BranchBackSetPerilThreshold));

    // Fix visual indicators of Mario Danger / Peril.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_DrawMenuMarioPinchMark_CheckThreshold_BH),
        reinterpret_cast<void*>(StartCheckMarioPinchDisp),
        reinterpret_cast<void*>(BranchBackCheckMarioPinchDisp));
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_DrawMenuMarioPinchMark_Patch_CheckResult1),
        0x7c032000U /* cmpw r3, r4 */);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_DrawMenuMarioPinchMark_Patch_CheckResult2),
        0x4181005cU /* bgt- 0x5c */);
        
    // Fix visual indicators of partners' Danger / Peril.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_DrawMenuPartyPinchMark_CheckThreshold_BH),
        reinterpret_cast<void*>(StartCheckPartnerPinchDisp),
        reinterpret_cast<void*>(BranchBackCheckPartnerPinchDisp));
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_DrawMenuPartyPinchMark_Patch_CheckResult1),
        0x7c032000U /* cmpw r3, r4 */);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_DrawMenuPartyPinchMark_Patch_CheckResult2),
        0x4181005cU /* bgt- 0x5c */);
        
    // Enable Star Power features always, if the option is set.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g__btlcmd_MakeOperationTable_AppealAlways_BH),
        reinterpret_cast<void*>(StartEnableAppealCheck),
        reinterpret_cast<void*>(BranchBackEnableAppealCheck));
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_BattleAudienceAddAudienceNum_EnableAlways_BH),
        reinterpret_cast<void*>(StartAddAudienceCheck),
        reinterpret_cast<void*>(BranchBackAddAudienceCheck));
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_BattleAudience_Disp_EnableAlways_BH),
        reinterpret_cast<void*>(StartDisplayAudienceCheck),
        reinterpret_cast<void*>(BranchBackDisplayAudienceCheck));
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_BattleAudience_End_SaveAmountAlways_BH),
        reinterpret_cast<void*>(StartSaveAudienceCountCheck),
        reinterpret_cast<void*>(BranchBackSaveAudienceCountCheck));
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_BattleAudienceSettingAudience_EnableAlways_BH),
        reinterpret_cast<void*>(StartSetInitialAudienceCheck),
        reinterpret_cast<void*>(BranchBackSetInitialAudienceCheck));
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g__object_fall_attack_AudienceEnableAlways_BH),
        reinterpret_cast<void*>(StartObjectFallOnAudienceCheck),
        reinterpret_cast<void*>(BranchBackObjectFallOnAudienceCheck));
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_BattleAudienceAddPuni_EnableAlways_BH),
        reinterpret_cast<void*>(StartAddPuniToAudienceCheck),
        reinterpret_cast<void*>(BranchBackAddPuniToAudienceCheck));
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_BattleBreakSlot_PointInc_EnableAlways_BH),
        reinterpret_cast<void*>(StartEnableIncrementingBingoCheck),
        reinterpret_cast<void*>(BranchBackEnableIncrementingBingoCheck));
        
    // Enable random audience items.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_BattleAudienceItemOn_RandomItem_BH),
        reinterpret_cast<void*>(StartAudienceItem),
        reinterpret_cast<void*>(BranchBackAudienceItem));
    // Make sure the right inventory is checked for getting items from audience.
    // Extend the range to check all item types:
    mod::patch::writePatch(
        reinterpret_cast<void*>(
            g_BattleAudienceItemCtrlProcess_Patch_CheckItemValidRange),
        0x2c000153U /* cmpwi r0, 0x153 */);
    // Handle bad items and badges:
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_BattleAudienceItemCtrlProcess_CheckSpace_BH),
        reinterpret_cast<void*>(StartAudienceItemSpaceFix),
        reinterpret_cast<void*>(BranchBackAudienceItemSpaceFix));
}

void ApplySettingBasedPatches() {
    // Change stage rank-up levels (set to 100 to force no rank-up on level-up).
    auto* rankup_data = ttyd::battle_seq_end::_rank_up_data;
    switch (g_Mod->state_.GetOptionValue(OPT_STAGE_RANK)) {
        case OPTVAL_STAGE_RANK_30_FLOORS:
        case OPTVAL_STAGE_RANK_ALWAYSMAX: {
            rankup_data[1].level = 100;
            rankup_data[2].level = 100;
            rankup_data[3].level = 100;
            break;
        }
    }
    
    // Increase some move badge BP costs if playing with larger max move levels.
    if (g_Mod->state_.CheckOptionValue(OPTVAL_BADGE_MOVE_1X)) {
        itemDataTable[ItemType::POWER_JUMP].bp_cost = 1;
        itemDataTable[ItemType::POWER_SMASH].bp_cost = 1;
        itemDataTable[ItemType::MULTIBOUNCE].bp_cost = 1;
        itemDataTable[ItemType::QUAKE_HAMMER].bp_cost = 2;
        itemDataTable[ItemType::FIRE_DRIVE].bp_cost = 2;
        itemDataTable[ItemType::CHARGE].bp_cost = 1;
        itemDataTable[ItemType::CHARGE_P].bp_cost = 1;
    } else {
        itemDataTable[ItemType::POWER_JUMP].bp_cost = 2;
        itemDataTable[ItemType::POWER_SMASH].bp_cost = 2;
        itemDataTable[ItemType::MULTIBOUNCE].bp_cost = 3;
        itemDataTable[ItemType::QUAKE_HAMMER].bp_cost = 3;
        itemDataTable[ItemType::FIRE_DRIVE].bp_cost = 3;
        itemDataTable[ItemType::CHARGE].bp_cost = 2;
        itemDataTable[ItemType::CHARGE_P].bp_cost = 2;
    }
    
    // Increase badge BP cost and shop prices if HP/FP Drains heal per hit.
    if (g_Mod->state_.GetOptionNumericValue(OPT_64_STYLE_HP_FP_DRAIN)) {
        itemDataTable[ItemType::HP_DRAIN].bp_cost = 3;
        itemDataTable[ItemType::HP_DRAIN_P].bp_cost = 3;
        itemDataTable[ItemType::FP_DRAIN].bp_cost = 6;
        itemDataTable[ItemType::FP_DRAIN_P].bp_cost = 6;
        
        itemDataTable[ItemType::HP_DRAIN].buy_price = 150;
        itemDataTable[ItemType::HP_DRAIN_P].buy_price = 150;
        itemDataTable[ItemType::FP_DRAIN].buy_price = 150;
        itemDataTable[ItemType::FP_DRAIN_P].buy_price = 150;
    } else {
        itemDataTable[ItemType::HP_DRAIN].bp_cost = 1;
        itemDataTable[ItemType::HP_DRAIN_P].bp_cost = 1;
        itemDataTable[ItemType::FP_DRAIN].bp_cost = 1;
        itemDataTable[ItemType::FP_DRAIN_P].bp_cost = 1;
        
        itemDataTable[ItemType::HP_DRAIN].buy_price = 70;
        itemDataTable[ItemType::HP_DRAIN_P].buy_price = 70;
        itemDataTable[ItemType::FP_DRAIN].buy_price = 125;
        itemDataTable[ItemType::FP_DRAIN_P].buy_price = 125;
    }
    
    if (g_Mod->state_.GetOptionNumericValue(OPT_OBFUSCATE_ITEMS)) {
        ObfuscateItems(true);
    }
}

void SpendFpOnSwitchingPartner(ttyd::battle_unit::BattleWorkUnit* unit) {
    int32_t switch_fp_cost =
        g_Mod->state_.GetOptionValue(OPTNUM_SWITCH_PARTY_FP_COST);
    if (switch_fp_cost > 0) {
        switch_fp_cost -= unit->badges_equipped.flower_saver;
        if (switch_fp_cost < 1) switch_fp_cost = 1;
        
        // Spend FP (and track total FP spent in BattleActRec).
        int32_t fp = ttyd::battle_unit::BtlUnit_GetFp(unit);
        ttyd::battle_unit::BtlUnit_SetFp(unit, fp - switch_fp_cost);
        ttyd::battle_actrecord::BtlActRec_AddPoint(
            &ttyd::battle::g_BattleWork->act_record_work.mario_fp_spent,
            switch_fp_cost);
    }
}

int32_t GetPinchThresholdForMaxHp(int32_t max_hp, bool peril) {
    if (g_Mod->state_.GetOptionNumericValue(OPT_PERCENT_BASED_DANGER)) {
        // Danger = 1/3 of max, Peril = 1/10 of max (rounded normally).
        int32_t threshold = peril ? (max_hp + 5) / 10 : (max_hp + 1) / 3;
        // Clamp to range [1, 99] to ensure the pinch range exists,
        // but doesn't exceed the range of a signed byte.
        if (threshold < 1) threshold = 1;
        if (threshold > 99) threshold = 99;
        return threshold;
    }
    // If not using %-based, return a fixed 1 / 5 HP (including for enemies).
    return peril ? 1 : 5;
}

void SetPinchThreshold(BattleUnitKind* kind, int32_t max_hp, bool peril) {
    int32_t threshold = GetPinchThresholdForMaxHp(max_hp, peril);
    if (peril) {
        kind->peril_hp = threshold;
    } else {
        kind->danger_hp = threshold;
    }
}

int32_t GetRandomAudienceItem(int32_t item_type) {
    if (g_Mod->state_.GetOptionNumericValue(OPT_AUDIENCE_RANDOM_THROWS)) {
        item_type = PickRandomItem(RNG_AUDIENCE_ITEM, 20, 10, 5, 15);
        if (item_type <= 0) {
            // Pick a coin, heart, flower, or random bad item if "none" selected.
            switch (g_Mod->state_.Rand(10, RNG_AUDIENCE_ITEM)) {
                case 0:  return ItemType::AUDIENCE_CAN;
                case 1:  return ItemType::AUDIENCE_ROCK;
                case 2:  return ItemType::AUDIENCE_BONE;
                case 3:  return ItemType::AUDIENCE_HAMMER;
                case 4:  
                case 5:  return ItemType::HEART_PICKUP;
                case 6:
                case 7:  return ItemType::FLOWER_PICKUP;
                default: return ItemType::COIN;
            }
        }
    }
    return item_type;
}

uint32_t FixAudienceItemSpaceCheck(uint32_t empty_item_slots, uint32_t item_type) {
    if (item_type >= ItemType::POWER_JUMP) {
        // Return 1 if there are any spaces left for badges.
        return ttyd::mario_pouch::pouchGetHaveBadgeCnt() < 200 ? 1 : 0; 
    } else if (item_type < ItemType::AUDIENCE_CAN) {
        // The number of empty item slots was already checked.
        return empty_item_slots;
    } else {
        // Item is a "harmful item" (can, etc.); skip the space check.
        return 1;
    }
}

}  // namespace options
}  // namespace mod::infinite_pit