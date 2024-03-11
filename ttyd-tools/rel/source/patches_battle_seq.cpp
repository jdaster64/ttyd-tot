#include "patches_battle_seq.h"

#include "common_ui.h"
#include "custom_condition.h"
#include "evt_cmd.h"
#include "mod.h"
#include "mod_state.h"
#include "patch.h"
#include "patches_partner.h"
#include "tot_generate_reward.h"
#include "tot_move_manager.h"
#include "tot_party_mario.h"

#include <ttyd/battle.h>
#include <ttyd/battle_actrecord.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_information.h>
#include <ttyd/battle_monosiri.h>
#include <ttyd/battle_seq.h>
#include <ttyd/battle_seq_command.h>
#include <ttyd/battle_unit.h>
#include <ttyd/evt_item.h>
#include <ttyd/item_data.h>
#include <ttyd/npcdrv.h>
#include <ttyd/seq_battle.h>
#include <ttyd/system.h>

#include <cstdint>

// Assembly patch functions.
extern "C" {
    // action_menu_patches.s
    void StartFixMarioSingleMoveCheck();
    void BranchBackFixMarioSingleMoveCheck();
    // battle_end_patches.s
    void StartGivePlayerInvuln();
    void BranchBackGivePlayerInvuln();
    void StartBtlSeqEndJudgeRule();
    void BranchBackBtlSeqEndJudgeRule();
    // currency_patches.s
    void StartCheckDeleteFieldItem();
    void BranchBackCheckDeleteFieldItem();
    
    void checkMarioSingleJumpHammer() {
        auto* battleWork = ttyd::battle::g_BattleWork;
        battleWork->battle_flags |= 0x600;
        
        // If the only move available is base-rank level 1, skip move selection.
        for (int32_t i = 0; i < 8; ++i) {
            int32_t move = mod::tot::MoveType::JUMP_BASE + i;
            int32_t move_tier = mod::tot::MoveManager::GetMoveData(move)->move_tier;
            int32_t unlocked_level = mod::tot::MoveManager::GetUnlockedLevel(move);
            if ((move_tier > 0 && unlocked_level > 0) || unlocked_level > 1) {
                battleWork->battle_flags &= ~0x200;
                break;
            }
        }
        for (int32_t i = 0; i < 8; ++i) {
            int32_t move = mod::tot::MoveType::HAMMER_BASE + i;
            int32_t move_tier = mod::tot::MoveManager::GetMoveData(move)->move_tier;
            int32_t unlocked_level = mod::tot::MoveManager::GetUnlockedLevel(move);
            if ((move_tier > 0 && unlocked_level > 0) || unlocked_level > 1) {
                battleWork->battle_flags &= ~0x400;
                break;
            }
        }
    }
}

namespace mod::infinite_pit {

namespace {

using ::ttyd::battle::BattleWork;
using ::ttyd::battle_database_common::BattleGroupSetup;
using ::ttyd::battle_database_common::BattleWeapon;
using ::ttyd::battle_database_common::PointDropData;
using ::ttyd::battle_unit::BattleWorkUnit;
using ::ttyd::npcdrv::FbatBattleInformation;
using ::ttyd::npcdrv::NpcBattleInfo;
using ::ttyd::npcdrv::NpcEntry;

namespace BattleUnitType = ::ttyd::battle_database_common::BattleUnitType;
namespace ItemType = ::ttyd::item_data::ItemType;

}

// Function hooks.
extern void (*g_seq_battleInit_trampoline)(void);
extern void (*g_fbatBattleMode_trampoline)(void);
extern void (*g_BtlActRec_JudgeRuleKeep_trampoline)(void);
extern void (*g__rule_disp_trampoline)(void);
extern BattleWeapon* (*g__GetFirstAttackWeapon_trampoline)(int32_t);
extern int32_t (*g__btlcmd_MakeSelectWeaponTable_trampoline)(
    BattleWork*, int32_t);
extern void (*g_BattleInformationSetDropMaterial_trampoline)(
    FbatBattleInformation*);
// Patch addresses.
extern const int32_t g_fbatBattleMode_GivePlayerInvuln_BH;
extern const int32_t g_fbatBattleMode_GivePlayerInvuln_EH;
extern const int32_t g_btlSeqMove_FixMarioSingleMoveCheck_BH;
extern const int32_t g_btlSeqMove_FixMarioSingleMoveCheck_EH;
extern const int32_t g_btlseqTurn_Patch_RuleDispShowLonger;
extern const int32_t g_btlseqTurn_Patch_RuleDispDismissOnlyWithB;
extern const int32_t g_btlseqEnd_JudgeRuleEarly_BH;
extern const int32_t g_btlseqEnd_Patch_RemoveJudgeRule;
extern const int32_t g_itemEntry_CheckDeleteFieldItem_BH;
extern const int32_t g_itemseq_Bound_Patch_BounceRange;
extern const int32_t g_enemy_common_dead_event_SpawnCoinsHook;
extern const int32_t g_enemy_common_dead_event_SpawnItemDropHook;

namespace battle_seq {
    
namespace {

// Custom evt to spawn different denominations of coins.
EVT_BEGIN(SpawnCoinsEvt)
IF_LARGE(LW(3), 100)
    SET(LW(3), 100)
END_IF()
LBL(5)
IF_LARGE_EQUAL(LW(3), 5)
    SUB(LW(3), 5)
    USER_FUNC(
        ttyd::evt_item::evt_item_entry, 0, ItemType::PIANTA,
        LW(0), LW(1), LW(2), 10, -1, 0)
    GOTO(5)
END_IF()
IF_LARGE_EQUAL(LW(3), 1)
    SUB(LW(3), 1)
    USER_FUNC(
        ttyd::evt_item::evt_item_entry, 0, ItemType::COIN,
        LW(0), LW(1), LW(2), 10, -1, 0)
    GOTO(5)
END_IF()
RETURN()
EVT_END()

EVT_BEGIN(SpawnCoinsEvtHook)
RUN_CHILD_EVT(SpawnCoinsEvt)
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
EVT_PATCH_END()

// Custom event to spawn item drops.
EVT_BEGIN(SpawnItemDropEvt)
IF_EQUAL(LW(3), (int32_t)ItemType::STAR_PIECE)
    // Use custom pickup evt for Star Pieces to handle upgrading moves.
    USER_FUNC(tot::evtTot_GetStarPiecePickupEvt, LW(4))
ELSE()
    SET(LW(4), 0)
END_IF()
USER_FUNC(
    ttyd::evt_item::evt_item_entry, 0, LW(3), LW(0), LW(1), LW(2), 10, -1, LW(4))
RETURN()
EVT_END()

EVT_BEGIN(SpawnItemDropHook)
RUN_CHILD_EVT(SpawnItemDropEvt)
0, 0, 0, 0, 0, 0, 0, 0,
EVT_PATCH_END()

// Copies NPC battle information to / from children of a parent NPC
// (e.g. Piranha Plants, projectiles) when starting or ending a battle.
void CopyChildBattleInfo(bool to_child) {
    auto* npc = ttyd::npcdrv::fbatGetPointer()->pBattleNpc;
    // Only copy if the NPC is valid and has a parent.
    if (npc && npc->master) {
        NpcEntry *dest, *src;
        if (to_child) {
            dest = npc;
            src = npc->master;
        } else {
            src = npc;
            dest = npc->master;
        }
        mod::patch::writePatch(
            &dest->battleInfo, &src->battleInfo, sizeof(NpcBattleInfo));
    }
}

// Checks whether the battle condition was satisfied, and if so,
// adds a bonus item to the "recovered items" pool.
void CheckBattleCondition() {
    auto* fbat_info = ttyd::battle::g_BattleWork->fbat_info;
    NpcBattleInfo* npc_info = fbat_info->wBattleInfo;

    // Track the number of turns spent / number of run aways at fight's end.
    StateManager_v2& state = g_Mod->state_;
    state.ChangeOption(
        STAT_TURNS_SPENT, ttyd::battle::g_BattleWork->turn_count);
    state.ChangeOption(
        STAT_MOST_TURNS_CURRENT, ttyd::battle::g_BattleWork->turn_count);
    if (state.GetOptionValue(STAT_MOST_TURNS_CURRENT) >
        state.GetOptionValue(STAT_MOST_TURNS_RECORD)) {
        // Update max turn count record (use 1-indexed floor count).
        state.SetOption(STAT_MOST_TURNS_RECORD,
            state.GetOptionValue(STAT_MOST_TURNS_CURRENT));
        state.SetOption(STAT_MOST_TURNS_FLOOR, state.floor_ + 1);
    }
    if (fbat_info->wResult != 1) {
        state.ChangeOption(STAT_TIMES_RAN_AWAY);
    }
    
    // Did not win the fight (e.g. ran away).
    if (fbat_info->wResult != 1) return;
    
    // Did not win the fight (an enemy still has a stolen item).
    for (int32_t i = 0; i < 8; ++i) {
        if (npc_info->wStolenItems[i] != 0) return;
    }
    
    // If condition is a success and rule is not 0, add a bonus item.
    if (fbat_info->wBtlActRecCondition && fbat_info->wRuleKeepResult == 6) {
        int32_t item_reward = 0;
        if (g_Mod->state_.CheckOptionValue(OPTVAL_DROP_HELD_FROM_BONUS)) {
            // If using "drop gated by bonus" option, use the held item that
            // would otherwise normally drop instead of the random item.
            // (If that item was stolen, the player receives nothing.)
            item_reward = npc_info->wHeldItems[
                npc_info->pConfiguration->held_item_weight];
        } else {
            item_reward = npc_info->pConfiguration->random_item_weight;
        }
        for (int32_t i = 0; i < 8; ++i) {
            if (npc_info->wBackItemIds[i] == 0) {
                npc_info->wBackItemIds[i] = item_reward;
                break;
            }
        }
        // Increment the count of successful challenges.
        state.ChangeOption(STAT_CONDITIONS_MET);
    }
    
    // If battle reward mode is "drop all held", award items other than the
    // natural drop ones until there are no "recovered items" slots left.
    if (g_Mod->state_.CheckOptionValue(OPTVAL_DROP_ALL_HELD)) {
        for (int32_t i = 0; i < 8; ++i) {
            const int32_t held_item = npc_info->wHeldItems[i];
            // If there is a held item, and this isn't the natural drop...
            if (held_item && i != npc_info->pConfiguration->held_item_weight) {
                for (int32_t j = 0; j < 8; ++j) {
                    if (npc_info->wBackItemIds[j] == 0) {
                        npc_info->wBackItemIds[j] = held_item;
                        break;
                    }
                }
            }
        }
    }
    
    // If playing with the "no partners" option, give the player the Tattle logs 
    // for all enemies present at the start of the fight.
    if (g_Mod->state_.CheckOptionValue(OPTVAL_PARTNERS_NEVER) &&
        !g_Mod->state_.GetOptionNumericValue(OPT_FIRST_PARTNER)) {
        const auto* group = npc_info->pConfiguration;
        for (int32_t i = 0; i < group->num_enemies; ++i) {
            int32_t type = group->enemy_data[i].unit_kind_params->unit_type;
            switch (type) {
                case BattleUnitType::LAKITU:
                case BattleUnitType::DARK_LAKITU:
                case BattleUnitType::YUX:
                case BattleUnitType::Z_YUX:
                case BattleUnitType::X_YUX:
                    // For enemy types that spawn minions, assume that they
                    // were also present (as they're never there at the start,
                    // and as such would otherwise never be marked).
                    ttyd::battle_monosiri::battleSetUnitMonosiriFlag(type + 1);
                    // fallthrough to default case...
                default:
                    ttyd::battle_monosiri::battleSetUnitMonosiriFlag(type);
            }
        }
    }
}

// Displays text associated with the battle condition.
void DisplayBattleCondition() {
    char buf[128];
    GetBattleConditionString(buf);
    DrawCenteredTextWindow(
        buf, 0, 60, 0xFFu, false, 0x000000FFu, 0.75f, 0xFFFFFFE5u, 15, 10);
}

// Replaces the logic for getting HP, FP, and item drops after a battle.
void GetDropMaterials(FbatBattleInformation* fbat_info) {
    NpcBattleInfo* battle_info = fbat_info->wBattleInfo;
    const BattleGroupSetup* party_setup = battle_info->pConfiguration;
    const PointDropData* hp_drop = party_setup->hp_drop_table;
    const PointDropData* fp_drop = party_setup->fp_drop_table;
    
    // Get natural heart and flower drops based on Mario's health, as usual.
    auto* battleWork = ttyd::battle::g_BattleWork;
    const BattleWorkUnit* mario = ttyd::battle::BattleGetMarioPtr(battleWork);
    
    int32_t mario_hp_pct = mario->current_hp * 100 / mario->max_hp;
    for (; true; ++hp_drop) {
        if (mario_hp_pct <= hp_drop->max_stat_percent) {
            if (static_cast<int32_t>(ttyd::system::irand(100))
                    < hp_drop->overall_drop_rate) {
                for (int32_t i = 0; i < hp_drop->drop_count; ++i) {
                    if (static_cast<int32_t>(ttyd::system::irand(100))
                            < hp_drop->individual_drop_rate) {
                        ++battle_info->wHeartsDroppedBaseCount;
                    }
                }
            }
            break;
        }
    }
    int32_t mario_fp_pct = mario->current_fp * 100 / mario->max_fp;
    for (; true; ++fp_drop) {
        if (mario_fp_pct <= fp_drop->max_stat_percent) {
            if (static_cast<int32_t>(ttyd::system::irand(100))
                    < fp_drop->overall_drop_rate) {
                for (int32_t i = 0; i < fp_drop->drop_count; ++i) {
                    if (static_cast<int32_t>(ttyd::system::irand(100))
                            < fp_drop->individual_drop_rate) {
                        ++battle_info->wFlowersDroppedBaseCount;
                    }
                }
            }
            break;
        }
    }
    
    switch (g_Mod->state_.GetOptionValue(OPT_BATTLE_REWARD_MODE)) {
        // If using default battle drop behavior, select the item drop based on
        // the previously determined enemy held item index.
        case OPTVAL_DROP_STANDARD:
        case OPTVAL_DROP_ALL_HELD: {
            battle_info->wItemDropped = 
                battle_info->wHeldItems[party_setup->held_item_weight];
            break;
        }
    }
}

}
    
void ApplyFixedPatches() {
    g_seq_battleInit_trampoline = patch::hookFunction(
        ttyd::seq_battle::seq_battleInit, []() {
            // Copy information from parent npc before battle, if applicable.
            CopyChildBattleInfo(/* to_child = */ true);
            // Force enemy ATK/DEF tattles to display at start of encounter.
            partner::RefreshExtraTattleStats();
            // Reset move selected levels at the start of encounter.
            tot::MoveManager::ResetSelectedLevels();
            g_seq_battleInit_trampoline();
        });

    g_fbatBattleMode_trampoline = patch::hookFunction(
        ttyd::npcdrv::fbatBattleMode, []() {
            bool post_battle_state = ttyd::npcdrv::fbatGetPointer()->state == 4;
            g_fbatBattleMode_trampoline();
            // Copy information back to parent npc after battle, if applicable.
            if (post_battle_state) CopyChildBattleInfo(/* to_child = */ false);
        });
        
    g_BtlActRec_JudgeRuleKeep_trampoline = patch::hookFunction(
        ttyd::battle_actrecord::BtlActRec_JudgeRuleKeep, []() {
            g_BtlActRec_JudgeRuleKeep_trampoline();
            // Handle item drops from conditions / ALL_HELD_ITEMS mode,
            // as well as marking Tattles in PARTNERS_NEVER mode.
            CheckBattleCondition();
        });
        
    g__rule_disp_trampoline = patch::hookFunction(
        ttyd::battle_seq::_rule_disp, []() {
            // Replaces the original logic completely.
            DisplayBattleCondition();
        });
        
    g__GetFirstAttackWeapon_trampoline = patch::hookFunction(
        ttyd::battle_seq::_GetFirstAttackWeapon, [](int32_t attack_type) {
            // Replaces the original logic completely.
            return tot::party_mario::GetFirstAttackWeapon(attack_type);
        });
        
    g__btlcmd_MakeSelectWeaponTable_trampoline = patch::hookFunction(
        ttyd::battle_seq_command::_btlcmd_MakeSelectWeaponTable, [](
            BattleWork* battleWork, int32_t table_type) {
            // Replaces the original logic completely.
            return tot::party_mario::MakeSelectWeaponTable(
                battleWork, table_type);
        });
        
    g_BattleInformationSetDropMaterial_trampoline = patch::hookFunction(
        ttyd::battle_information::BattleInformationSetDropMaterial,
        [](FbatBattleInformation* fbat_info) {
            // Replaces the original logic completely.
            GetDropMaterials(fbat_info);
        });
        
    // Make Mario's jump/hammer menus always show the full weapon sub-menu.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_btlSeqMove_FixMarioSingleMoveCheck_BH),
        reinterpret_cast<void*>(g_btlSeqMove_FixMarioSingleMoveCheck_EH),
        reinterpret_cast<void*>(StartFixMarioSingleMoveCheck),
        reinterpret_cast<void*>(BranchBackFixMarioSingleMoveCheck));
        
    // Make defeating a group of enemies still holding stolen items always make
    // you have temporary intangibility, even if you recovered some of them, to
    // prevent projectiles from first-striking you again if you recover items.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_fbatBattleMode_GivePlayerInvuln_BH),
        reinterpret_cast<void*>(g_fbatBattleMode_GivePlayerInvuln_EH),
        reinterpret_cast<void*>(StartGivePlayerInvuln),
        reinterpret_cast<void*>(BranchBackGivePlayerInvuln));

    // Check for battle conditions at the start of processing the battle end,
    // not the end; this way level-up heals don't factor into "final HP".
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_btlseqEnd_JudgeRuleEarly_BH),
        reinterpret_cast<void*>(StartBtlSeqEndJudgeRule),
        reinterpret_cast<void*>(BranchBackBtlSeqEndJudgeRule));
    // Remove the original check for battle conditions at the end of battle.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_btlseqEnd_Patch_RemoveJudgeRule), 
        0x60000000U /* nop */);

    // Make the battle condition message display longer (5s instead of 2s),
    // and only be dismissable by the B button, rather than A or B.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_btlseqTurn_Patch_RuleDispShowLonger), 
        0x3800012cU /* li r0, 300 */);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_btlseqTurn_Patch_RuleDispDismissOnlyWithB), 
        0x38600200U /* li r3, 0x200 (B button) */);
        
    // Support Star Piece item drops behaving like reward, upgrading a move.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_enemy_common_dead_event_SpawnItemDropHook),
        SpawnItemDropHook, sizeof(SpawnItemDropHook));
        
    // Support multiple demonimations of coins dropping at the end of a fight.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_enemy_common_dead_event_SpawnCoinsHook),
        SpawnCoinsEvtHook, sizeof(SpawnCoinsEvtHook));
    // Allow big coins to be overridden by other items on the field.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_itemEntry_CheckDeleteFieldItem_BH),
        reinterpret_cast<void*>(StartCheckDeleteFieldItem),
        reinterpret_cast<void*>(BranchBackCheckDeleteFieldItem));     
    // Make bigger coins bounce the same distance as regular coins, not
    // vanilla Piantas (which have the range of hearts, flowers, or items).
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_itemseq_Bound_Patch_BounceRange),
        0x2c00007b /* cmpwi r0,0x7b (heart, not Pianta) */);
}

}  // namespace battle_seq
}  // namespace mod::infinite_pit