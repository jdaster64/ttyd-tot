#include "patches_battle.h"

#include "custom_enemy.h"
#include "evt_cmd.h"
#include "mod.h"
#include "mod_state.h"
#include "patch.h"
#include "patches_mario_move.h"
#include "tot_move_manager.h"

#include <ttyd/battle.h>
#include <ttyd/battle_ac.h>
#include <ttyd/battle_actrecord.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_damage.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_unit.h>
#include <ttyd/evtmgr.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>

#include <cstdint>
#include <cstring>

// Assembly patch functions.
extern "C" {
    // audience_level_patches.s
    void StartSetTargetAudienceCount();
    void BranchBackSetTargetAudienceCount();
    // permanent_status_patches.s
    void StartCheckRecoveryStatus();
    void BranchBackCheckRecoveryStatus();
    void ConditionalBranchCheckRecoveryStatus();
    void StartStatusIconDisplay();
    void BranchBackStatusIconDisplay();
    void ConditionalBranchStatusIconDisplay();
    // star_power_patches.s
    void StartApplySpRegenMultiplierNoBingo();
    void BranchBackApplySpRegenMultiplierNoBingo();
    void StartApplySpRegenMultiplierBingo();
    void BranchBackApplySpRegenMultiplierBingo();
    
    void setTargetAudienceCount() {
        mod::infinite_pit::battle::SetTargetAudienceAmount();
    }
    double applySpRegenMultiplier(double base_regen) {
        return mod::infinite_pit::battle::ApplySpRegenMultiplier(base_regen);
    }
}

namespace mod::infinite_pit {

namespace {

using ::ttyd::battle_database_common::BattleWeapon;
using ::ttyd::battle_unit::BattleWorkUnit;

namespace BattleUnitType = ::ttyd::battle_database_common::BattleUnitType;
namespace ItemType = ::ttyd::item_data::ItemType;
namespace StatusEffectType = ::ttyd::battle_database_common::StatusEffectType;

}

// Function hooks.
extern int32_t (*g_BattleActionCommandCheckDefence_trampoline)(
    BattleWorkUnit*, BattleWeapon*);
extern void (*g__getSickStatusParam_trampoline)(
    BattleWorkUnit*, BattleWeapon*, int32_t, int8_t*, int8_t*);
// Patch addresses.
extern const int32_t g_BattleCheckDamage_Patch_PaybackDivisor;
extern const int32_t g_BattleCheckDamage_Patch_HoldFastDivisor;
extern const int32_t g_BattleCheckDamage_Patch_ReturnPostageDivisor;
extern const int32_t g_BattleAudience_ApRecoveryBuild_NoBingoRegen_BH;
extern const int32_t g_BattleAudience_ApRecoveryBuild_BingoRegen_BH;
extern const int32_t g_BattleAudience_SetTargetAmount_BH;
extern const int32_t g_BtlUnit_CheckRecoveryStatus_PermanentStatus_BH;
extern const int32_t g_BtlUnit_CheckRecoveryStatus_PermanentStatus_EH;
extern const int32_t g_BtlUnit_CheckRecoveryStatus_PermanentStatus_CH1;
extern const int32_t g_battle_status_icon_SkipIconForPermanentStatus_BH;
extern const int32_t g_battle_status_icon_SkipIconForPermanentStatus_EH;
extern const int32_t g_battle_status_icon_SkipIconForPermanentStatus_CH1;

namespace battle {
    
void GetStatusParams(
    BattleWorkUnit* unit, BattleWeapon* weapon, int32_t status_type,
    int8_t* turn_count, int8_t* strength) {
    int32_t turns_temporary = 0;
    int32_t strength_temporary = 0;
    
    switch (status_type) {
        case StatusEffectType::ALLERGIC:
            turns_temporary = weapon->allergic_time;
            break;
        case StatusEffectType::SLEEP:
            turns_temporary = weapon->sleep_time;
            break;
        case StatusEffectType::STOP:
            turns_temporary = weapon->stop_time;
            break;
        case StatusEffectType::DIZZY:
            turns_temporary = weapon->dizzy_time;
            break;
        case StatusEffectType::POISON:
            turns_temporary = weapon->poison_time;
            strength_temporary = weapon->poison_strength;
            break;
        case StatusEffectType::CONFUSE:
            turns_temporary = weapon->confuse_time;
            break;
        case StatusEffectType::ELECTRIC:
            turns_temporary = weapon->electric_time;
            break;
        case StatusEffectType::DODGY:
            turns_temporary = weapon->dodgy_time;
            break;
        case StatusEffectType::BURN:
            turns_temporary = weapon->burn_time;
            break;
        case StatusEffectType::FREEZE:
            turns_temporary = weapon->freeze_time;
            break;
        case StatusEffectType::HUGE:
            if (weapon->size_change_strength > 0) {
                turns_temporary = weapon->size_change_time;
                strength_temporary = weapon->size_change_strength;
            }
            break;
        case StatusEffectType::TINY:
            if (weapon->size_change_strength < 0) {
                turns_temporary = weapon->size_change_time;
                strength_temporary = weapon->size_change_strength;
            }
            break;
        case StatusEffectType::ATTACK_UP:
            if (weapon->atk_change_strength > 0) {
                turns_temporary = weapon->atk_change_time;
                strength_temporary = weapon->atk_change_strength;
            }
            break;
        case StatusEffectType::ATTACK_DOWN:
            if (weapon->atk_change_strength < 0) {
                turns_temporary = weapon->atk_change_time;
                strength_temporary = weapon->atk_change_strength;
            }
            break;
        case StatusEffectType::DEFENSE_UP:
            if (weapon->def_change_strength > 0) {
                turns_temporary = weapon->def_change_time;
                strength_temporary = weapon->def_change_strength;
            }
            break;
        case StatusEffectType::DEFENSE_DOWN:
            if (weapon->def_change_strength < 0) {
                turns_temporary = weapon->def_change_time;
                strength_temporary = weapon->def_change_strength;
            }
            break;
        case StatusEffectType::CHARGE:
            strength_temporary = weapon->charge_strength;
            break;
        case StatusEffectType::INVISIBLE:
            turns_temporary = weapon->invisible_time;
            break;
        case StatusEffectType::FAST:
            turns_temporary = weapon->fast_time;
            break;
        case StatusEffectType::SLOW:
            turns_temporary = weapon->slow_time;
            break;
        case StatusEffectType::PAYBACK:
            turns_temporary = weapon->payback_time;
            break;
        case StatusEffectType::HOLD_FAST:
            turns_temporary = weapon->hold_fast_time;
            break;
        case StatusEffectType::HP_REGEN:
            turns_temporary = weapon->hp_regen_time;
            strength_temporary = weapon->hp_regen_strength;
            break;
        case StatusEffectType::FP_REGEN:
            turns_temporary = weapon->fp_regen_time;
            strength_temporary = weapon->fp_regen_strength;
            break;
        case StatusEffectType::OHKO:
            turns_temporary = weapon->ohko_chance;
            break;
    }
    
    switch (weapon->item_id) {
        case ItemType::POWER_JUMP:
            if (status_type == StatusEffectType::DEFENSE_DOWN) {
                turns_temporary += tot::MoveManager::GetSelectedLevel(
                    tot::MoveType::JUMP_POWER_JUMP) * 2 - 2;
            }
            break;
        case ItemType::SLEEPY_STOMP:
            if (status_type == StatusEffectType::SLEEP) {
                turns_temporary += tot::MoveManager::GetSelectedLevel(
                    tot::MoveType::JUMP_SLEEPY_STOMP) * 2 - 2;
            }
            break;
        case ItemType::HEAD_RATTLE:
            if (status_type == StatusEffectType::TINY) {
                turns_temporary += tot::MoveManager::GetSelectedLevel(
                    tot::MoveType::HAMMER_SHRINK_SMASH) * 2 - 2;
            }
            break;
        case ItemType::ICE_SMASH:
            if (status_type == StatusEffectType::FREEZE) {
                turns_temporary += tot::MoveManager::GetSelectedLevel(
                    tot::MoveType::HAMMER_ICE_SMASH) * 2 - 2;
            }
            break;
        case ItemType::CHARGE:
        case ItemType::CHARGE_P:
            if (status_type == StatusEffectType::CHARGE) {
                strength_temporary += mario_move::GetStrategyBadgeLevel(
                    /* is_charge = */ true,
                    unit->current_kind == BattleUnitType::MARIO) - 1;
            }
            break;
        case ItemType::SUPER_CHARGE:
        case ItemType::SUPER_CHARGE_P:
            if (status_type == StatusEffectType::DEFENSE_UP) {
                strength_temporary += mario_move::GetStrategyBadgeLevel(
                    /* is_charge = */ false,
                    unit->current_kind == BattleUnitType::MARIO) - 1;
            }
            break;
    }

    // If unit is an enemy and status is Charge (and not an item),
    // change its power in the same way as ATK / FP damage.
    if (status_type == StatusEffectType::CHARGE && !weapon->item_id &&
        unit->current_kind <= BattleUnitType::BONETAIL) {
        int32_t altered_charge;
        GetEnemyStats(
            unit->current_kind, nullptr, &altered_charge,
            nullptr, nullptr, nullptr, strength_temporary);
        if (altered_charge > 99) altered_charge = 99;
        strength_temporary = altered_charge;
    }
    
    *turn_count = turns_temporary;
    *strength = strength_temporary;
}         
    
void ApplyFixedPatches() {
    g_BattleActionCommandCheckDefence_trampoline = patch::hookFunction(
        ttyd::battle_ac::BattleActionCommandCheckDefence,
        [](BattleWorkUnit* unit, BattleWeapon* weapon) {
            // Run normal logic if option turned off.
            const int32_t sp_cost =
                g_Mod->state_.GetOptionValue(OPTNUM_SUPERGUARD_SP_COST);
            if (sp_cost <= 0) {
                const int32_t defense_result =
                    g_BattleActionCommandCheckDefence_trampoline(unit, weapon);
                if (defense_result == 5) {
                    // Successful Superguard, track in play stats.
                    g_Mod->state_.ChangeOption(STAT_SUPERGUARDS);
                }
                return defense_result;
            }
            
            int8_t superguard_frames[7];
            bool restore_superguard_frames = false;
            // Temporarily disable Superguarding if SP is too low.
            if (ttyd::mario_pouch::pouchGetAP() < sp_cost) {
                restore_superguard_frames = true;
                memcpy(superguard_frames, ttyd::battle_ac::superguard_frames, 7);
                for (int32_t i = 0; i < 7; ++i) {
                    ttyd::battle_ac::superguard_frames[i] = 0;
                }
            }
            const int32_t defense_result =
                g_BattleActionCommandCheckDefence_trampoline(unit, weapon);
            if (defense_result == 5) {
                // Successful Superguard, subtract SP and track in play stats.
                ttyd::mario_pouch::pouchAddAP(-sp_cost);
                g_Mod->state_.ChangeOption(STAT_SUPERGUARDS);
            }
            if (restore_superguard_frames) {
                memcpy(ttyd::battle_ac::superguard_frames, superguard_frames, 7);
            }
            return defense_result;
        });
            
    g__getSickStatusParam_trampoline = patch::hookFunction(
        ttyd::battle_damage::_getSickStatusParam, [](
            BattleWorkUnit* unit, BattleWeapon* weapon, int32_t status_type,
            int8_t* turn_count, int8_t* strength) {
                // Replaces vanilla logic completely.
                GetStatusParams(unit, weapon, status_type, turn_count, strength);
            });
            
    // Add support for "permanent statuses" (turn count >= 100).
    // Don't tick down turn count if 100 or over:
    mod::patch::writeBranch(
        reinterpret_cast<void*>(
            g_BtlUnit_CheckRecoveryStatus_PermanentStatus_BH),
        reinterpret_cast<void*>(StartCheckRecoveryStatus));
    mod::patch::writeBranch(
        reinterpret_cast<void*>(BranchBackCheckRecoveryStatus),
        reinterpret_cast<void*>(
            g_BtlUnit_CheckRecoveryStatus_PermanentStatus_EH));
    mod::patch::writeBranch(
        reinterpret_cast<void*>(ConditionalBranchCheckRecoveryStatus),
        reinterpret_cast<void*>(
            g_BtlUnit_CheckRecoveryStatus_PermanentStatus_CH1));
    // Skip drawing status icon if 100 or over:
    mod::patch::writeBranch(
        reinterpret_cast<void*>(
            g_battle_status_icon_SkipIconForPermanentStatus_BH),
        reinterpret_cast<void*>(StartStatusIconDisplay));
    mod::patch::writeBranch(
        reinterpret_cast<void*>(BranchBackStatusIconDisplay),
        reinterpret_cast<void*>(
            g_battle_status_icon_SkipIconForPermanentStatus_EH));
    mod::patch::writeBranch(
        reinterpret_cast<void*>(ConditionalBranchStatusIconDisplay),
        reinterpret_cast<void*>(
            g_battle_status_icon_SkipIconForPermanentStatus_CH1));
        
    // Increase all forms of Payback-esque status returned damage to 1x.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_BattleCheckDamage_Patch_PaybackDivisor),
        0x38000032U /* li r0, 50 */);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_BattleCheckDamage_Patch_HoldFastDivisor),
        0x38000032U /* li r0, 50 */);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_BattleCheckDamage_Patch_ReturnPostageDivisor),
        0x38000032U /* li r0, 50 */);
        
    // Change frame windows for guarding / Superguarding at different levels
    // of Simplifiers / Unsimplifiers to be more symmetric.
    const int8_t kGuardFrames[] =     { 12, 10, 9, 8, 7, 6, 5, 0 };
    const int8_t kSuperguardFrames[]  = { 5, 4, 4, 3, 2, 2, 1, 0 };
    mod::patch::writePatch(
        ttyd::battle_ac::guard_frames, kGuardFrames, sizeof(kGuardFrames));
    mod::patch::writePatch(
        ttyd::battle_ac::superguard_frames, kSuperguardFrames, 
        sizeof(kSuperguardFrames));
        
    // Override the default target audience size.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_BattleAudience_SetTargetAmount_BH),
        reinterpret_cast<void*>(StartSetTargetAudienceCount),
        reinterpret_cast<void*>(BranchBackSetTargetAudienceCount));
        
    // Apply the option to change the amount of SP regained from attacks.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_BattleAudience_ApRecoveryBuild_NoBingoRegen_BH),
        reinterpret_cast<void*>(StartApplySpRegenMultiplierNoBingo),
        reinterpret_cast<void*>(BranchBackApplySpRegenMultiplierNoBingo));
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_BattleAudience_ApRecoveryBuild_BingoRegen_BH),
        reinterpret_cast<void*>(StartApplySpRegenMultiplierBingo),
        reinterpret_cast<void*>(BranchBackApplySpRegenMultiplierBingo));
}

void SetTargetAudienceAmount() {
    uintptr_t audience_work_base =
        reinterpret_cast<uintptr_t>(
            ttyd::battle::g_BattleWork->audience_work);
    float target_amount = 200.0f;
    // If set to rank up by progression, make the target audience follow suit;
    // otherwise, keep the target fixed at max capacity.
    if (g_Mod->state_.GetOptionValue(OPTVAL_STAGE_RANK_30_FLOORS)) {
        const int32_t floor = g_Mod->state_.floor_;
        target_amount = floor >= 195 ? 200.0f : floor + 5.0f;
    }
    *reinterpret_cast<float*>(audience_work_base + 0x13778) = target_amount;
}

double ApplySpRegenMultiplier(double base_regen) {
    return base_regen * 
        g_Mod->state_.GetOptionNumericValue(OPTNUM_SP_REGEN_MODIFIER) / 20.0;
}

}  // namespace battle
}  // namespace mod::infinite_pit