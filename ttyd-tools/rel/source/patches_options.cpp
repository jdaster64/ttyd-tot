#include "patches_options.h"

#include "common_functions.h"
#include "mod.h"
#include "patch.h"
#include "tot_generate_item.h"

#include <ttyd/battle.h>
#include <ttyd/battle_actrecord.h>
#include <ttyd/battle_audience.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_event_default.h>
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
    
    int32_t getAudienceItem(int32_t item_type) {
        return mod::tot::patch::options::GetRandomAudienceItem(item_type);
    }
    uint32_t audienceFixItemSpaceCheck(
        uint32_t empty_item_slots, uint32_t item_type) {
        return mod::tot::patch::options::FixAudienceItemSpaceCheck(
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
        mod::tot::patch::options::SetPinchThreshold(kind, max_hp, peril);
        return kind;
    }
    bool checkStarPowersEnabled() {
        // Star Powers will always be enabled in v2.
        return true;
    }
}

namespace mod::tot::patch {

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
extern int32_t (*g__get_escape_rate_trampoline)(EvtEntry*, bool);
extern void (*g_BattleAudienceSetThrowItemMax_trampoline)();
// Patch addresses.
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
    g_btlevtcmd_WeaponAftereffect_trampoline = mod::hookFunction(
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

    // Guarantee running away from battle if enabled.
    g__get_escape_rate_trampoline = mod::hookFunction(
        ttyd::battle_event_default::_get_escape_rate,
        [](EvtEntry* evt, bool isFirstCall) {
            // Call original logic.
            g__get_escape_rate_trampoline(evt, isFirstCall);
            // Override with 100% chance if enabled.
            if (g_Mod->state_.CheckOptionValue(OPTVAL_RUN_AWAY_GUARANTEED)) {
                evtSetValue(evt, evt->evtArguments[0], 101);
            }
            return 2;
        });
        
    // Guarantee that 1 item can be thrown every turn if randomized audience
    // items are enabled.
    g_BattleAudienceSetThrowItemMax_trampoline = mod::hookFunction(
        ttyd::battle_audience::BattleAudienceSetThrowItemMax, [](){
            g_BattleAudienceSetThrowItemMax_trampoline();
            if (g_Mod->state_.GetOption(OPT_AUDIENCE_RANDOM_THROWS)) {
                auto& audience_work = ttyd::battle::g_BattleWork->audience_work;
                audience_work.max_items_this_turn = 1;
            }
        });
        
    // Override the default Danger / Peril threshold checks for all actors.
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_BtlUnit_CheckPinchStatus_DangerThreshold_BH),
        reinterpret_cast<void*>(StartSetDangerThreshold),
        reinterpret_cast<void*>(BranchBackSetDangerThreshold));
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_BtlUnit_CheckPinchStatus_PerilThreshold_BH),
        reinterpret_cast<void*>(StartSetPerilThreshold),
        reinterpret_cast<void*>(BranchBackSetPerilThreshold));

    // Fix visual indicators of Mario Danger / Peril.
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_DrawMenuMarioPinchMark_CheckThreshold_BH),
        reinterpret_cast<void*>(StartCheckMarioPinchDisp),
        reinterpret_cast<void*>(BranchBackCheckMarioPinchDisp));
    mod::writePatch(
        reinterpret_cast<void*>(g_DrawMenuMarioPinchMark_Patch_CheckResult1),
        0x7c032000U /* cmpw r3, r4 */);
    mod::writePatch(
        reinterpret_cast<void*>(g_DrawMenuMarioPinchMark_Patch_CheckResult2),
        0x4181005cU /* bgt- 0x5c */);
        
    // Fix visual indicators of partners' Danger / Peril.
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_DrawMenuPartyPinchMark_CheckThreshold_BH),
        reinterpret_cast<void*>(StartCheckPartnerPinchDisp),
        reinterpret_cast<void*>(BranchBackCheckPartnerPinchDisp));
    mod::writePatch(
        reinterpret_cast<void*>(g_DrawMenuPartyPinchMark_Patch_CheckResult1),
        0x7c032000U /* cmpw r3, r4 */);
    mod::writePatch(
        reinterpret_cast<void*>(g_DrawMenuPartyPinchMark_Patch_CheckResult2),
        0x4181005cU /* bgt- 0x5c */);
        
    // Enable Star Power features always, if the option is set.
    mod::writeBranchPair(
        reinterpret_cast<void*>(g__btlcmd_MakeOperationTable_AppealAlways_BH),
        reinterpret_cast<void*>(StartEnableAppealCheck),
        reinterpret_cast<void*>(BranchBackEnableAppealCheck));
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_BattleAudienceAddAudienceNum_EnableAlways_BH),
        reinterpret_cast<void*>(StartAddAudienceCheck),
        reinterpret_cast<void*>(BranchBackAddAudienceCheck));
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_BattleAudience_Disp_EnableAlways_BH),
        reinterpret_cast<void*>(StartDisplayAudienceCheck),
        reinterpret_cast<void*>(BranchBackDisplayAudienceCheck));
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_BattleAudience_End_SaveAmountAlways_BH),
        reinterpret_cast<void*>(StartSaveAudienceCountCheck),
        reinterpret_cast<void*>(BranchBackSaveAudienceCountCheck));
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_BattleAudienceSettingAudience_EnableAlways_BH),
        reinterpret_cast<void*>(StartSetInitialAudienceCheck),
        reinterpret_cast<void*>(BranchBackSetInitialAudienceCheck));
    mod::writeBranchPair(
        reinterpret_cast<void*>(g__object_fall_attack_AudienceEnableAlways_BH),
        reinterpret_cast<void*>(StartObjectFallOnAudienceCheck),
        reinterpret_cast<void*>(BranchBackObjectFallOnAudienceCheck));
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_BattleAudienceAddPuni_EnableAlways_BH),
        reinterpret_cast<void*>(StartAddPuniToAudienceCheck),
        reinterpret_cast<void*>(BranchBackAddPuniToAudienceCheck));
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_BattleBreakSlot_PointInc_EnableAlways_BH),
        reinterpret_cast<void*>(StartEnableIncrementingBingoCheck),
        reinterpret_cast<void*>(BranchBackEnableIncrementingBingoCheck));
        
    // Enable random audience items.
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_BattleAudienceItemOn_RandomItem_BH),
        reinterpret_cast<void*>(StartAudienceItem),
        reinterpret_cast<void*>(BranchBackAudienceItem));
    // Make sure the right inventory is checked for getting items from audience.
    // Extend the range to check all item types:
    mod::writePatch(
        reinterpret_cast<void*>(
            g_BattleAudienceItemCtrlProcess_Patch_CheckItemValidRange),
        0x2c000153U /* cmpwi r0, 0x153 */);
    // Handle bad items and badges:
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_BattleAudienceItemCtrlProcess_CheckSpace_BH),
        reinterpret_cast<void*>(StartAudienceItemSpaceFix),
        reinterpret_cast<void*>(BranchBackAudienceItemSpaceFix));
    
    // Apply patch to give the player infinite BP, if enabled.
    g_pouchReviseMarioParam_trampoline = mod::hookFunction(
        ttyd::mario_pouch::pouchReviseMarioParam, [](){
            g_pouchReviseMarioParam_trampoline();
            if (g_Mod->state_.CheckOptionValue(OPTVAL_INFINITE_BP) &&
                g_Mod->state_.GetOption(OPT_RUN_STARTED)) {
                ttyd::mario_pouch::pouchGetPtr()->total_bp = 99;
                ttyd::mario_pouch::pouchGetPtr()->unallocated_bp = 99;
            }
        });
}

int32_t GetPinchThresholdForMaxHp(int32_t max_hp, bool peril) {
    // 10% of max health for Peril, 30% of max health for Danger, rounded 5/4.
    int32_t threshold =  (max_hp * (peril ? 1 : 3) + 5) / 10;
    // Clamp to range [1, 99] to ensure the pinch range exists,
    // but doesn't exceed the range of a signed byte.
    if (threshold < 1) threshold = 1;
    if (threshold > 99) threshold = 99;
    return threshold;
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
    if (g_Mod->state_.GetOption(OPT_AUDIENCE_RANDOM_THROWS)) {
        item_type = PickRandomItem(RNG_AUDIENCE_ITEM, 25, 5, 5, 15);
        if (item_type <= 0) {
            // Pick a coin, heart, flower, or random bad item if "none" selected.
            switch (g_Mod->state_.Rand(10, RNG_AUDIENCE_ITEM)) {
                case 0:  item_type = ItemType::AUDIENCE_CAN;    break;
                case 1:  item_type = ItemType::AUDIENCE_ROCK;   break;
                case 2:  item_type = ItemType::AUDIENCE_BONE;   break;
                case 3:  item_type = ItemType::AUDIENCE_HAMMER; break;
                case 4:  
                case 5:  item_type = ItemType::HEART_PICKUP;    break;
                case 6:
                case 7:  item_type = ItemType::FLOWER_PICKUP;   break;
                default: item_type = ItemType::COIN;            break;
            }
        }
        // Set to 0 for helpful, 1 for harmful, since badges normally are unset.
        ttyd::battle_audience::BattleAudience_SetPresentItemType(
            item_type >= 0xec && item_type <= 0xef);
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
}  // namespace mod::tot::patch