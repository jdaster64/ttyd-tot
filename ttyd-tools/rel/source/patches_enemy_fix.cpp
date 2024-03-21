#include "patches_enemy_fix.h"

#include "common_functions.h"
#include "common_types.h"
#include "evt_cmd.h"
#include "mod.h"
#include "mod_state.h"
#include "patch.h"
#include "tot_custom_rel.h"

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

// For convenience.
using namespace ::mod::tot::custom;
    
using ::ttyd::battle::BattleWork;
using ::ttyd::battle_database_common::BattleUnitKind;
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

// A fragment of an event to patch over Hammer/Boomerang/Fire Bros.' HP checks.
const int32_t HammerBrosHpCheck[] = {
    USER_FUNC(GetPercentOfMaxHP, -2, LW(0))
    IF_SMALL(LW(0), 50)
};

EVT_DEFINE_USER_FUNC(CheckNumEnemiesRemaining) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    int32_t num_enemies = 0;
    bool is_midboss = false;
    for (int32_t i = 0; i < 64; ++i) {
        BattleWorkUnit* unit = battleWork->battle_units[i];
        // Count enemies of either alliance that are still alive.
        if (unit && unit->current_kind <= BattleUnitType::BONETAIL &&
            unit->alliance <= 1 && !BtlUnit_CheckStatus(unit, 27)) {
            ++num_enemies;
            if (unit->size_change_turns > 99) is_midboss = true;
        }
    }
    // If there is a midboss, force enemies to use their AI for 2+ enemies.
    if (is_midboss && num_enemies < 2) num_enemies = 2;
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
                // If there is a midboss on the field, always fail.
                for (int32_t i = 0; i < 64; ++i) {
                    auto* unit = battleWork->battle_units[i];
                    if (unit && !BtlUnit_CheckStatus(unit, 27) &&
                        unit->size_change_turns > 99) {
                        // Treat the spot as full.
                        evtSetValue(evt, evt->evtArguments[0], 1);
                        return 2;
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
        
    // Individual enemy behavior, etc. patches.
    
    // Patch Gale Force coins / EXP out for enemies that had special logic
    // for handling it in the original game (mostly cloning enemies).
    mod::patch::writePatch(
        reinterpret_cast<void*>(evt_DarkWizzerd_GaleForceDeath_PatchLoc),
        GaleForceKillPatch, sizeof(GaleForceKillPatch));
    mod::patch::writePatch(
        reinterpret_cast<void*>(evt_EliteWizzerd_GaleForceDeath_PatchLoc),
        GaleForceKillPatch, sizeof(GaleForceKillPatch));
    mod::patch::writePatch(
        reinterpret_cast<void*>(evt_Magikoopa_GaleForceDeath_PatchLoc),
        GaleForceKillPatch, sizeof(GaleForceKillPatch));
    mod::patch::writePatch(
        reinterpret_cast<void*>(evt_RedMagikoopa_GaleForceDeath_PatchLoc),
        GaleForceKillPatch, sizeof(GaleForceKillPatch));
    mod::patch::writePatch(
        reinterpret_cast<void*>(evt_WhiteMagikoopa_GaleForceDeath_PatchLoc),
        GaleForceKillPatch, sizeof(GaleForceKillPatch));
    mod::patch::writePatch(
        reinterpret_cast<void*>(evt_GreenMagikoopa_GaleForceDeath_PatchLoc),
        GaleForceKillPatch, sizeof(GaleForceKillPatch));
    mod::patch::writePatch(
        reinterpret_cast<void*>(evt_Pider_GaleForceDeath_PatchLoc),
        GaleForceKillPatch, sizeof(GaleForceKillPatch));
    mod::patch::writePatch(
        reinterpret_cast<void*>(evt_Arantula_GaleForceDeath_PatchLoc),
        GaleForceKillPatch, sizeof(GaleForceKillPatch));
        
    // Patch cloning Wizzerds' / Magikoopas' remaining enemies checks
    // to consider all enemy actors, regardless of alliance.
    mod::patch::writePatch(
        reinterpret_cast<void*>(evt_DarkWizzerd_CheckNumEnemies_PatchLoc),
        reinterpret_cast<uint32_t>(CheckNumEnemiesRemaining));
    mod::patch::writePatch(
        reinterpret_cast<void*>(evt_EliteWizzerd_CheckNumEnemies_PatchLoc),
        reinterpret_cast<uint32_t>(CheckNumEnemiesRemaining));
    mod::patch::writePatch(
        reinterpret_cast<void*>(evt_Magikoopa_CheckNumEnemies_PatchLoc),
        reinterpret_cast<uint32_t>(CheckNumEnemiesRemaining));
    mod::patch::writePatch(
        reinterpret_cast<void*>(evt_RedMagikoopa_CheckNumEnemies_PatchLoc),
        reinterpret_cast<uint32_t>(CheckNumEnemiesRemaining));
    mod::patch::writePatch(
        reinterpret_cast<void*>(evt_WhiteMagikoopa_CheckNumEnemies_PatchLoc),
        reinterpret_cast<uint32_t>(CheckNumEnemiesRemaining));
    mod::patch::writePatch(
        reinterpret_cast<void*>(evt_GreenMagikoopa_CheckNumEnemies_PatchLoc),
        reinterpret_cast<uint32_t>(CheckNumEnemiesRemaining));
        
    // Patch over Bandits' confusion check for whether to steal.
    mod::patch::writePatch(
        reinterpret_cast<void*>(evt_Bandit_CheckConfusion_PatchLoc),
        reinterpret_cast<uint32_t>(CheckConfusedOrInfatuated));
    mod::patch::writePatch(
        reinterpret_cast<void*>(evt_BigBandit_CheckConfusion_PatchLoc),
        reinterpret_cast<uint32_t>(CheckConfusedOrInfatuated));
    mod::patch::writePatch(
        reinterpret_cast<void*>(evt_BadgeBandit_CheckConfusion_PatchLoc),
        reinterpret_cast<uint32_t>(CheckConfusedOrInfatuated));
            
    // Patch over Hammer, Boomerang, and Fire Bros.' low-HP checks.
    mod::patch::writePatch(
        reinterpret_cast<void*>(evt_HammerBros_CheckHp_PatchLoc),
        HammerBrosHpCheck, sizeof(HammerBrosHpCheck));
    mod::patch::writePatch(
        reinterpret_cast<void*>(evt_BoomerangBros_CheckHp_PatchLoc),
        HammerBrosHpCheck, sizeof(HammerBrosHpCheck));
    mod::patch::writePatch(
        reinterpret_cast<void*>(evt_FireBros_CheckHp_PatchLoc),
        HammerBrosHpCheck, sizeof(HammerBrosHpCheck));
        
    // Fix branch labels for attacks that softlock if there are no valid targets.
    mod::patch::writePatch(
        reinterpret_cast<void*>(evt_Koopatrol_NormalAttackReturnLbl_PatchLoc), 98);
    mod::patch::writePatch(
        reinterpret_cast<void*>(evt_DarkKoopatrol_NormalAttackReturnLbl_PatchLoc), 98);
    mod::patch::writePatch(
        reinterpret_cast<void*>(evt_XNaut_NormalAttackReturnLbl_PatchLoc), 98);
    mod::patch::writePatch(
        reinterpret_cast<void*>(evt_XNaut_JumpAttackReturnLbl_PatchLoc), 98);
    mod::patch::writePatch(
        reinterpret_cast<void*>(evt_EliteXNaut_NormalAttackReturnLbl_PatchLoc), 98);
    mod::patch::writePatch(
        reinterpret_cast<void*>(evt_EliteXNaut_JumpAttackReturnLbl_PatchLoc), 98);
            
    // Make all varieties of Yux able to be hit by grounded attacks,
    // that way any partner is able to attack them.
    part_Yux_Main.attribute_flags   &= ~0x600000;
    part_ZYux_Main.attribute_flags  &= ~0x600000;
    part_XYux_Main.attribute_flags  &= ~0x600000;
    
    // Make all varieties of Yux unable to be swallowed.
    unit_Yux.swallow_chance     = -1;
    unit_ZYux.swallow_chance    = -1;
    unit_XYux.swallow_chance    = -1;
    
    // Give Green Magikoopas nonzero base DEF so they can have 1 DEF in the mod.
    for (int32_t i = 0; i < 5; ++i) defense_GreenMagikoopa[i] = 1;
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