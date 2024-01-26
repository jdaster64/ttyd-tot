#include "patches_enemy_fix.h"

#include "common_functions.h"
#include "common_types.h"
#include "evt_cmd.h"
#include "mod.h"
#include "mod_state.h"
#include "patch.h"

#include <ttyd/battle.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_item_data.h>
#include <ttyd/battle_seq.h>
#include <ttyd/battle_sub.h>
#include <ttyd/battle_unit.h>
#include <ttyd/dispdrv.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>

#include <cstdint>

// Assembly patch functions.
extern "C" {
    // audience_item_patches.s
    void StartAudienceCheckPlayerTarget();
    void BranchBackAudienceCheckPlayerTarget();
    // enemy_sampling_patches.s
    void StartSampleRandomTarget();
    void BranchBackSampleRandomTarget();
    // held_item_disp_patches.s
    void StartDispEnemyHeldItem();
    void BranchBackDispEnemyHeldItem();    
    
    int32_t sumWeaponTargetRandomWeights(int32_t* weights) {
        return mod::infinite_pit::enemy_fix::
            SumWeaponTargetRandomWeights(weights);
    }
    
    void dispEnemyHeldItem(
        ttyd::dispdrv::CameraId cameraId, uint8_t renderMode, float order,
        ttyd::dispdrv::PFN_dispCallback callback, void *user) {
        // Alias for convenience.
        namespace BattleUnitType = ::ttyd::battle_database_common::BattleUnitType;
            
        auto* battleWork = ttyd::battle::g_BattleWork;
        // Loop through all units, and skip drawing item if any clones.
        for (int32_t i = 0; i < 64; ++i) {
            auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, i);
            if (!unit) continue;
            switch (unit->current_kind) {
                case BattleUnitType::MAGIKOOPA_CLONE:
                case BattleUnitType::RED_MAGIKOOPA_CLONE:
                case BattleUnitType::WHITE_MAGIKOOPA_CLONE:
                case BattleUnitType::GREEN_MAGIKOOPA_CLONE:
                case BattleUnitType::DARK_WIZZERD_CLONE:
                case BattleUnitType::ELITE_WIZZERD_CLONE:
                    return;
                default:
                    break;
            }
        }
        // No clones present, display item as normal.
        ttyd::dispdrv::dispEntry(cameraId, renderMode, order, callback, user);
    }
}

namespace mod::infinite_pit {

namespace {
    
using ::ttyd::battle::BattleWork;
using ::ttyd::battle_database_common::BattleUnitKindPart;
using ::ttyd::battle_unit::BattleWorkUnit;
using ::ttyd::battle_unit::BtlUnit_CheckStatus;
using ::ttyd::evtmgr::EvtEntry;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;
using ::ttyd::mario_pouch::PouchData;

namespace BattleUnitType = ::ttyd::battle_database_common::BattleUnitType;
namespace ItemType = ::ttyd::item_data::ItemType;

}

// Function hooks.
extern int32_t (*g_btlevtcmd_GetSelectEnemy_trampoline)(EvtEntry*, bool);
extern int32_t (*g_btlevtcmd_CheckSpace_trampoline)(EvtEntry*, bool);
extern uint32_t (*g_BattleCheckConcluded_trampoline)(BattleWork*);
// Patch addresses.
extern const int32_t g_BattleAudienceDetectTargetPlayer_CheckPlayer_BH;
extern const int32_t g_BattleAudienceDetectTargetPlayer_CheckPlayer_EH;
extern const int32_t g_BattleChoiceSamplingEnemy_SumRandWeights_BH;
extern const int32_t g_BattleChoiceSamplingEnemy_SumRandWeights_EH;
extern const int32_t g_btlDispMain_DrawNormalHeldItem_BH;
extern const int32_t g_btlevtcmd_CheckSpace_Patch_CheckEnemyTypes;
extern const int32_t g_jon_BanditAttackEvt_CheckConfusionOffset;
extern const int32_t g_jon_DarkKoopatrolAttackEvt_NormalAttackReturnLblOffset;
extern const int32_t g_jon_DarkWizzerdAttackEvt_CheckEnemyNumOffset;
extern const int32_t g_jon_DarkWizzerdGaleForceDeathEvt_PatchOffset;
extern const int32_t g_jon_EliteWizzerdAttackEvt_CheckEnemyNumOffset;
extern const int32_t g_jon_EliteWizzerdGaleForceDeathEvt_PatchOffset;
extern const int32_t g_jon_BadgeBanditAttackEvt_CheckConfusionOffset;
extern const int32_t g_jon_PiderGaleForceDeathEvt_PatchOffset;
extern const int32_t g_jon_ArantulaGaleForceDeathEvt_PatchOffset;
extern const int32_t g_custom_AtomicBoo_BreathWeapon_Offset;
extern const int32_t g_custom_AtomicBooBreathSubEvt_Offset;
extern const int32_t g_custom_HammerBrosAttackEvt_CheckHpOffset;
extern const int32_t g_custom_BoomerangBrosAttackEvt_CheckHpOffset;
extern const int32_t g_custom_FireBrosAttackEvt_CheckHpOffset;
extern const int32_t g_custom_MagikoopaGaleForceDeathEvt_PatchOffset;
extern const int32_t g_custom_GrnMagikoopaGaleForceDeathEvt_PatchOffset;
extern const int32_t g_custom_RedMagikoopaGaleForceDeathEvt_PatchOffset;
extern const int32_t g_custom_WhtMagikoopaGaleForceDeathEvt_PatchOffset;
extern const int32_t g_custom_MagikoopaAttackEvt_CheckEnemyNumOffset;
extern const int32_t g_custom_GrnMagikoopaAttackEvt_CheckEnemyNumOffset;
extern const int32_t g_custom_RedMagikoopaAttackEvt_CheckEnemyNumOffset;
extern const int32_t g_custom_WhtMagikoopaAttackEvt_CheckEnemyNumOffset;
extern const int32_t g_custom_BigBanditAttackEvt_CheckConfusionOffset;
extern const int32_t g_custom_KoopatrolAttackEvt_NormalAttackReturnLblOffset;
extern const int32_t g_custom_XNautAttackEvt_NormalAttackReturnLblOffset;
extern const int32_t g_custom_XNautAttackEvt_JumpAttackReturnLblOffset;
extern const int32_t g_custom_EliteXNautAttackEvt_NormalAttackReturnLblOffset;
extern const int32_t g_custom_EliteXNautAttackEvt_JumpAttackReturnLblOffset;
extern const int32_t g_custom_ZYux_PrimaryKindPartOffset;
extern const int32_t g_custom_XYux_PrimaryKindPartOffset;
extern const int32_t g_custom_Yux_PrimaryKindPartOffset;
extern const int32_t g_custom_GrnMagikoopa_DefenseOffset;

namespace enemy_fix {

namespace {

// Returns the percentage of max HP a battle unit currently has.
EVT_DECLARE_USER_FUNC(GetPercentOfMaxHP, 2)
    
// Patch to disable the coins / EXP from Gale Force (replace with no-ops).
EVT_BEGIN(GaleForceKillPatch)
DEBUG_REM(0) DEBUG_REM(0) DEBUG_REM(0) DEBUG_REM(0)
DEBUG_REM(0) DEBUG_REM(0) DEBUG_REM(0)
EVT_PATCH_END()
static_assert(sizeof(GaleForceKillPatch) == 0x38);

// Modified Atomic Boo breath event that checks for both characters' guards.
EVT_BEGIN(AtomicBooBreathSubEvt)

USER_FUNC(
    ttyd::battle_event_cmd::btlevtcmd_PreCheckDamage, -2, LW(3), LW(4),
    REL_PTR(ModuleId::CUSTOM, g_custom_AtomicBoo_BreathWeapon_Offset), 0, LW(5))
SWITCH(LW(5))
    CASE_OR(4)
        GOTO(90)
        CASE_END()
    CASE_EQUAL(3)
        USER_FUNC(ttyd::battle_event_cmd::btlevtcmd_StartAvoid, LW(3), 38)
        GOTO(90)
        CASE_END()
    CASE_EQUAL(6)
        USER_FUNC(ttyd::battle_event_cmd::btlevtcmd_StartAvoid, LW(3), 39)
        GOTO(90)
        CASE_END()
    CASE_EQUAL(2)
        USER_FUNC(ttyd::battle_event_cmd::btlevtcmd_StartAvoid, LW(3), 40)
        GOTO(90)
        CASE_END()
    CASE_EQUAL(1)
        GOTO(91)
        CASE_END()
    CASE_ETC()
        GOTO(98)  // This is what it does in the original script?
        CASE_END()
END_SWITCH()
LBL(90)
    GOTO(99)
LBL(91)
    // Use a random variable not used in the Pit to track how many checks have
    // taken place; just setting this here could cause race-condition issues,
    // but for this particular attack hits happen ~simultaneously for all
    // targets so it shouldn't be an issue.
    SET(GSW(821), 0)
    
    WAIT_FRM(15)
    IF_SMALL(GSW(821), 1)
        USER_FUNC(
            ttyd::battle_event_cmd::btlevtcmd_ResultACDefence, LW(3),
            REL_PTR(ModuleId::CUSTOM, g_custom_AtomicBoo_BreathWeapon_Offset))
        SET(GSW(821), 1)
    END_IF()
    USER_FUNC(
        ttyd::battle_event_cmd::btlevtcmd_CheckDamage, -2, LW(3), LW(4),
        REL_PTR(ModuleId::CUSTOM, g_custom_AtomicBoo_BreathWeapon_Offset),
        0, LW(5))
        
    WAIT_FRM(30)
    IF_SMALL(GSW(821), 2)
        USER_FUNC(
            ttyd::battle_event_cmd::btlevtcmd_ResultACDefence, LW(3),
            REL_PTR(ModuleId::CUSTOM, g_custom_AtomicBoo_BreathWeapon_Offset))
        SET(GSW(821), 2)
    END_IF()
    USER_FUNC(
        ttyd::battle_event_cmd::btlevtcmd_CheckDamage, -2, LW(3), LW(4),
        REL_PTR(ModuleId::CUSTOM, g_custom_AtomicBoo_BreathWeapon_Offset),
        0, LW(5))
    
    WAIT_FRM(30)
    IF_SMALL(GSW(821), 3)
        USER_FUNC(
            ttyd::battle_event_cmd::btlevtcmd_ResultACDefence, LW(3),
            REL_PTR(ModuleId::CUSTOM, g_custom_AtomicBoo_BreathWeapon_Offset))
        SET(GSW(821), 3)
    END_IF()
    USER_FUNC(
        ttyd::battle_event_cmd::btlevtcmd_CheckDamage, -2, LW(3), LW(4),
        REL_PTR(ModuleId::CUSTOM, g_custom_AtomicBoo_BreathWeapon_Offset),
        0x100, LW(5))
        
LBL(99)
RETURN()
EVT_END()

// Wrapper for modified Atomic Boo breath event.
EVT_BEGIN(AtomicBooBreathSubEvtHook)
RUN_CHILD_EVT(AtomicBooBreathSubEvt)
RETURN()
EVT_END()

// A fragment of an event to patch over Hammer/Boomerang/Fire Bros.' HP checks.
const int32_t HammerBrosHpCheck[] = {
    USER_FUNC(GetPercentOfMaxHP, -2, LW(0))
    IF_SMALL(LW(0), 50)
};

EVT_DEFINE_USER_FUNC(CheckNumEnemiesRemaining) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    int32_t num_enemies = 0;
    for (int32_t i = 0; i < 64; ++i) {
        BattleWorkUnit* unit = battleWork->battle_units[i];
        // Count enemies of either alliance that are still alive.
        if (unit && unit->current_kind <= BattleUnitType::BONETAIL &&
            unit->alliance <= 1 && !BtlUnit_CheckStatus(unit, 27))
            ++num_enemies;
    }
    evtSetValue(evt, evt->evtArguments[0], num_enemies);
    return 2;
}

EVT_DEFINE_USER_FUNC(CheckConfusedOrInfatuated) {
    // Check if Confused first (assumes token checked is 0x10).
    ttyd::battle_event_cmd::btlevtcmd_CheckToken(evt, isFirstCall);
    // Check if Infatuated.
    auto* battleWork = ttyd::battle::g_BattleWork;
    int32_t id = evtGetValue(evt, evt->evtArguments[0]);
    id = ttyd::battle_sub::BattleTransID(evt, id);
    auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, id);
    if (unit->alliance == 0) evtSetValue(evt, evt->evtArguments[2], 1);
    return 2;
}

EVT_DEFINE_USER_FUNC(GetPercentOfMaxHP) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    int32_t id = evtGetValue(evt, evt->evtArguments[0]);
    id = ttyd::battle_sub::BattleTransID(evt, id);
    auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, id);
    evtSetValue(
        evt, evt->evtArguments[1], unit->current_hp * 100 / unit->max_hp);
    return 2;
}

// Changes the order that certain attacks select their targets in
// (selecting the user last, if the user is included).
void ReorderWeaponTargets() {
    auto& twork = ttyd::battle::g_BattleWork->weapon_targets_work;
    
    // If Trade Off, reorder targets so attacker (if present) is targeted last.
    // TODO: Apply this change for any other weapons with similar issues.
    if (twork.weapon == &ttyd::battle_item_data::ItemWeaponData_Teki_Kyouka) {
        if (twork.num_targets > 1) {
            for (int32_t i = 0; i < twork.num_targets - 1; ++i) {
                int32_t target_unit_idx = 
                    twork.targets[twork.target_indices[i]].unit_idx;
                if (target_unit_idx == twork.attacker_idx) {
                    // Swap with last target.
                    int32_t tmp = twork.target_indices[i];
                    twork.target_indices[i] = 
                        twork.target_indices[twork.num_targets - 1];
                    twork.target_indices[twork.num_targets - 1] = tmp;
                    return;
                }
            }
        }
    }
}

// Checks if all player characters are defeated (excluding enemies).
bool CheckIfPlayerDefeated() {
    for (int32_t ai = 0; ai < 3; ++ai) {
        auto* battleWork = ttyd::battle::g_BattleWork;
        auto* alliances = battleWork->alliance_information;
        if (alliances[ai].identifier == 2) {
            int32_t idx = 0;
            for (; idx < 64; ++idx) {
                BattleWorkUnit* unit = battleWork->battle_units[idx];
                // For all non-player allied actors that aren't enemies...
                // (e.g. just Mario and partner)
                if (unit && unit->alliance == 0 &&
                    unit->true_kind > BattleUnitType::BONETAIL &&
                    (unit->attribute_flags & 0x40000)) {
                    // Break early if any are alive.
                    if (!ttyd::battle_unit::BtlUnit_CheckStatus(unit, 27) &&
                        !(unit->attribute_flags & 0x10000000)) break;
                }
            }
            if (idx == 64) {  // Didn't break early (i.e. none are alive)
                alliances[ai].loss_condition_met = true;
                return true;
            }
        }
    }
    return false;
}

}

void ApplyFixedPatches() {        
    // Changes targeting order for certain attacks so the user hits themselves
    // after all other targets.
    g_btlevtcmd_GetSelectEnemy_trampoline = patch::hookFunction(
        ttyd::battle_event_cmd::btlevtcmd_GetSelectEnemy,
        [](EvtEntry* evt, bool isFirstCall) {
            ReorderWeaponTargets();
            return g_btlevtcmd_GetSelectEnemy_trampoline(evt, isFirstCall);
        });

    // Disallows audience from throwing items at Infatuated enemies.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_BattleAudienceDetectTargetPlayer_CheckPlayer_BH),
        reinterpret_cast<void*>(g_BattleAudienceDetectTargetPlayer_CheckPlayer_EH),
        reinterpret_cast<void*>(StartAudienceCheckPlayerTarget),
        reinterpret_cast<void*>(BranchBackAudienceCheckPlayerTarget));

    // Hooks drawing held item code, skipping it if any clone enemies exist.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_btlDispMain_DrawNormalHeldItem_BH),
        reinterpret_cast<void*>(StartDispEnemyHeldItem),
        reinterpret_cast<void*>(BranchBackDispEnemyHeldItem));

    // Sums weapon targets' random weights, ensuring that each weight is > 0.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_BattleChoiceSamplingEnemy_SumRandWeights_BH),
        reinterpret_cast<void*>(g_BattleChoiceSamplingEnemy_SumRandWeights_EH),
        reinterpret_cast<void*>(StartSampleRandomTarget),
        reinterpret_cast<void*>(BranchBackSampleRandomTarget));
        
    // Force friendly enemies to never call for backup, and certain enemies
    // to stop calling for backup after turn 5.
    g_btlevtcmd_CheckSpace_trampoline = patch::hookFunction(
        ttyd::battle_event_cmd::btlevtcmd_CheckSpace,
        [](EvtEntry* evt, bool isFirstCall) {
            auto* battleWork = ttyd::battle::g_BattleWork;
            uint32_t idx = reinterpret_cast<uint32_t>(evt->wActorThisPtr);
            if (idx) {
                auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, idx);
                if (unit && unit->alliance == 0) {
                    // If desired pos is way out of range, multiply it by -1
                    // (to prevent infinite looping).
                    int32_t target_pos = evtGetValue(evt, evt->evtArguments[1]);
                    if (target_pos < -300 || target_pos > 300) {
                        evtSetValue(evt, evt->evtArguments[1], -target_pos);
                    }
                    // Treat the spot as full.
                    evtSetValue(evt, evt->evtArguments[0], 1);
                    return 2;
                } else if (battleWork->turn_count > 5) {
                    switch (unit->current_kind) {
                        case BattleUnitType::POKEY:
                        case BattleUnitType::POISON_POKEY:
                        case BattleUnitType::DULL_BONES:
                        case BattleUnitType::RED_BONES:
                        case BattleUnitType::DRY_BONES:
                        case BattleUnitType::DARK_BONES:
                        case BattleUnitType::LAKITU:
                        case BattleUnitType::DARK_LAKITU:
                        case BattleUnitType::GREEN_FUZZY:
                        case BattleUnitType::KOOPATROL:
                        case BattleUnitType::DARK_KOOPATROL:
                            // Treat the spot as full.
                            evtSetValue(evt, evt->evtArguments[0], 1);
                            return 2;
                        default:
                            break;
                    }
                }
            }
            return g_btlevtcmd_CheckSpace_trampoline(evt, isFirstCall);
        });
        
    // Make btlevtcmd_CheckSpace consider enemies only, regardless of alliance.
    // lwz r0, 8 (r3); cmpwi r0, 0xab; bgt- 0xd0 (Branch if not an enemy)
    const uint32_t kCheckSpaceAllianceCheckOps[] = {
        0x80030008, (0x2c000000 | BattleUnitType::BONETAIL), 0x418100d0
    };
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_btlevtcmd_CheckSpace_Patch_CheckEnemyTypes),
        kCheckSpaceAllianceCheckOps, sizeof(kCheckSpaceAllianceCheckOps));
        
    // Add additional check for player's side losing battle that doesn't
    // take Infatuated enemies into account.
    g_BattleCheckConcluded_trampoline = patch::hookFunction(
        ttyd::battle_seq::BattleCheckConcluded, [](BattleWork* battleWork) {
            uint32_t result = g_BattleCheckConcluded_trampoline(battleWork);
            if (!result) result = CheckIfPlayerDefeated();
            return result;
        });
}

void ApplyModuleLevelPatches(void* module_ptr, ModuleId::e module_id) {
    if (!module_ptr) return;
    const uint32_t module_start = reinterpret_cast<uint32_t>(module_ptr);
    
    if (module_id == ModuleId::JON) {
        // Patch Gale Force coins / EXP out for Wizzerds & Piders.
        mod::patch::writePatch(
            reinterpret_cast<void*>(
                module_start + g_jon_DarkWizzerdGaleForceDeathEvt_PatchOffset),
            GaleForceKillPatch, sizeof(GaleForceKillPatch));
        mod::patch::writePatch(
            reinterpret_cast<void*>(
                module_start + g_jon_EliteWizzerdGaleForceDeathEvt_PatchOffset),
            GaleForceKillPatch, sizeof(GaleForceKillPatch));
        mod::patch::writePatch(
            reinterpret_cast<void*>(
                module_start + g_jon_PiderGaleForceDeathEvt_PatchOffset),
            GaleForceKillPatch, sizeof(GaleForceKillPatch));
        mod::patch::writePatch(
            reinterpret_cast<void*>(
                module_start + g_jon_ArantulaGaleForceDeathEvt_PatchOffset),
            GaleForceKillPatch, sizeof(GaleForceKillPatch));
        // Patch over cloning Wizzerds' num enemies check.
        mod::patch::writePatch(
            reinterpret_cast<void*>(
                module_start + g_jon_DarkWizzerdAttackEvt_CheckEnemyNumOffset),
            reinterpret_cast<uint32_t>(CheckNumEnemiesRemaining));
        mod::patch::writePatch(
            reinterpret_cast<void*>(
                module_start + g_jon_EliteWizzerdAttackEvt_CheckEnemyNumOffset),
            reinterpret_cast<uint32_t>(CheckNumEnemiesRemaining));
        // Patch over Bandit, Badge Bandit confusion check for whether to steal.
        mod::patch::writePatch(
            reinterpret_cast<void*>(
                module_start + g_jon_BanditAttackEvt_CheckConfusionOffset),
            reinterpret_cast<uint32_t>(CheckConfusedOrInfatuated));
        mod::patch::writePatch(
            reinterpret_cast<void*>(
                module_start + g_jon_BadgeBanditAttackEvt_CheckConfusionOffset),
            reinterpret_cast<uint32_t>(CheckConfusedOrInfatuated));
        // Fix Dark Koopatrol's normal attack if there are no valid targets.
        mod::patch::writePatch(
            reinterpret_cast<void*>(
                module_start +
                g_jon_DarkKoopatrolAttackEvt_NormalAttackReturnLblOffset), 98);
    } else if (module_id == ModuleId::CUSTOM) {
        // Custom logic for Atomic Boo's breath attack that allows guards with
        // both characters at once.
        mod::patch::writePatch(
            reinterpret_cast<void*>(
                module_start + g_custom_AtomicBooBreathSubEvt_Offset),
            AtomicBooBreathSubEvtHook, sizeof(AtomicBooBreathSubEvtHook));
        // Patch over Hammer, Boomerang, and Fire Bros.' HP checks.
        mod::patch::writePatch(
            reinterpret_cast<void*>(
                module_start + g_custom_HammerBrosAttackEvt_CheckHpOffset),
            HammerBrosHpCheck, sizeof(HammerBrosHpCheck));
        mod::patch::writePatch(
            reinterpret_cast<void*>(
                module_start + g_custom_BoomerangBrosAttackEvt_CheckHpOffset),
            HammerBrosHpCheck, sizeof(HammerBrosHpCheck));
        mod::patch::writePatch(
            reinterpret_cast<void*>(
                module_start + g_custom_FireBrosAttackEvt_CheckHpOffset),
            HammerBrosHpCheck, sizeof(HammerBrosHpCheck));
        // Patch Gale Force coins / EXP out for Magikoopa.
        mod::patch::writePatch(
            reinterpret_cast<void*>(
                module_start + g_custom_MagikoopaGaleForceDeathEvt_PatchOffset),
            GaleForceKillPatch, sizeof(GaleForceKillPatch));
        mod::patch::writePatch(
            reinterpret_cast<void*>(
                module_start + g_custom_GrnMagikoopaGaleForceDeathEvt_PatchOffset),
            GaleForceKillPatch, sizeof(GaleForceKillPatch));
        mod::patch::writePatch(
            reinterpret_cast<void*>(
                module_start + g_custom_RedMagikoopaGaleForceDeathEvt_PatchOffset),
            GaleForceKillPatch, sizeof(GaleForceKillPatch));
        mod::patch::writePatch(
            reinterpret_cast<void*>(
                module_start + g_custom_WhtMagikoopaGaleForceDeathEvt_PatchOffset),
            GaleForceKillPatch, sizeof(GaleForceKillPatch));
        // Patch over Magikoopa's num enemies check.
        mod::patch::writePatch(
            reinterpret_cast<void*>(
                module_start + g_custom_MagikoopaAttackEvt_CheckEnemyNumOffset),
            reinterpret_cast<uint32_t>(CheckNumEnemiesRemaining));
        mod::patch::writePatch(
            reinterpret_cast<void*>(
                module_start + g_custom_GrnMagikoopaAttackEvt_CheckEnemyNumOffset),
            reinterpret_cast<uint32_t>(CheckNumEnemiesRemaining));
        mod::patch::writePatch(
            reinterpret_cast<void*>(
                module_start + g_custom_RedMagikoopaAttackEvt_CheckEnemyNumOffset),
            reinterpret_cast<uint32_t>(CheckNumEnemiesRemaining));
        mod::patch::writePatch(
            reinterpret_cast<void*>(
                module_start + g_custom_WhtMagikoopaAttackEvt_CheckEnemyNumOffset),
            reinterpret_cast<uint32_t>(CheckNumEnemiesRemaining));
        // Patch over Big Bandit confusion check for whether to steal items.
        mod::patch::writePatch(
            reinterpret_cast<void*>(
                module_start + g_custom_BigBanditAttackEvt_CheckConfusionOffset),
            reinterpret_cast<uint32_t>(CheckConfusedOrInfatuated));
        // Fix Koopatrol's normal attack if there are no valid targets.
        mod::patch::writePatch(
            reinterpret_cast<void*>(
                module_start +
                g_custom_KoopatrolAttackEvt_NormalAttackReturnLblOffset), 98);
        // Fix X-Naut & Elite X-Nauts' attacks if there are no valid targets.
        mod::patch::writePatch(
            reinterpret_cast<void*>(
                module_start +
                g_custom_XNautAttackEvt_NormalAttackReturnLblOffset), 98);
        mod::patch::writePatch(
            reinterpret_cast<void*>(
                module_start +
                g_custom_XNautAttackEvt_JumpAttackReturnLblOffset), 98);
        mod::patch::writePatch(
            reinterpret_cast<void*>(
                module_start +
                g_custom_EliteXNautAttackEvt_NormalAttackReturnLblOffset), 98);
        mod::patch::writePatch(
            reinterpret_cast<void*>(
                module_start +
                g_custom_EliteXNautAttackEvt_JumpAttackReturnLblOffset), 98);
        // Make all varieties of Yux able to be hit by grounded attacks,
        // that way any partner is able to attack them.
        auto* z_yux =
            reinterpret_cast<BattleUnitKindPart*>(
                module_start + g_custom_ZYux_PrimaryKindPartOffset);
        auto* x_yux =
            reinterpret_cast<BattleUnitKindPart*>(
                module_start + g_custom_XYux_PrimaryKindPartOffset);
        auto* yux =
            reinterpret_cast<BattleUnitKindPart*>(
                module_start + g_custom_Yux_PrimaryKindPartOffset);
        z_yux->attribute_flags  &= ~0x600000;
        x_yux->attribute_flags  &= ~0x600000;
        yux->attribute_flags    &= ~0x600000;
        // Give Green Magikoopas nonzero DEF so they can have 1 DEF in the mod.
        const int8_t kDefenseArr[5] = { 1, 1, 1, 1, 1 };
        mod::patch::writePatch(
            reinterpret_cast<void*>(
                module_start + g_custom_GrnMagikoopa_DefenseOffset),
            kDefenseArr, sizeof(kDefenseArr));
    }
}

void LinkCustomEvts(void* module_ptr, ModuleId::e module_id, bool link) {
    if (module_id != ModuleId::CUSTOM || !module_ptr) return;
    
    if (link) {
        LinkCustomEvt(
            module_id, module_ptr, const_cast<int32_t*>(AtomicBooBreathSubEvt));
    } else {
        UnlinkCustomEvt(
            module_id, module_ptr, const_cast<int32_t*>(AtomicBooBreathSubEvt));
    }
}

int32_t SumWeaponTargetRandomWeights(int32_t* weights) {
    int32_t sum = 0;
    auto& twork = ttyd::battle::g_BattleWork->weapon_targets_work;
    for (int32_t i = 0; i < twork.num_targets; ++i) {
        if (weights[i] <= 0) weights[i] = 1;
        sum += weights[i];
    }
    return sum;
}

}  // namespace enemy_fix
}  // namespace mod::infinite_pit