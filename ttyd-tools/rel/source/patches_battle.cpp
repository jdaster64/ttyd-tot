#include "patches_battle.h"

#include "custom_enemy.h"
#include "evt_cmd.h"
#include "mod.h"
#include "mod_state.h"
#include "patch.h"
#include "patches_mario_move.h"

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

namespace battle {
    
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
                // Run vanilla logic.
                g__getSickStatusParam_trampoline(
                    unit, weapon, status_type, turn_count, strength);
                // If badge type and status type (DEF-Up) are correct,
                // change the effect strength based on the badges equipped.
                if (status_type == 14 && (
                    weapon->item_id == ItemType::SUPER_CHARGE ||
                    weapon->item_id == ItemType::SUPER_CHARGE_P)) {
                    bool is_mario = unit->current_kind == BattleUnitType::MARIO;
                    int8_t badges = mario_move::GetToughenUpLevel(is_mario);
                    *strength = badges + 1;
                }
                // If unit is an enemy and status is Charge (and not an item),
                // change its power in the same way as ATK / FP damage.
                if (status_type == 16 && !weapon->item_id &&
                    unit->current_kind <= BattleUnitType::BONETAIL) {
                    int32_t altered_charge;
                    GetEnemyStats(
                        unit->current_kind, nullptr, &altered_charge,
                        nullptr, nullptr, nullptr, *strength);
                    if (altered_charge > 99) altered_charge = 99;
                    *strength = altered_charge;
                }
            });
        
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

EVT_DEFINE_USER_FUNC(AwardStarPowerAndResetFaceDirection) {
    // Declare Star Power / bonuses earned via this attack.
    ttyd::battle_event_cmd::btlevtcmd_InviteApInfoReport(evt, isFirstCall);
    // Reset facing direction as usual.
    ttyd::battle_event_cmd::btlevtcmd_ResetFaceDirection(evt, isFirstCall);
    return 2;
}

}  // namespace battle
}  // namespace mod::infinite_pit