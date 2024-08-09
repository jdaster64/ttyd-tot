#include "patches_battle_seq.h"

#include "common_functions.h"
#include "common_ui.h"
#include "evt_cmd.h"
#include "mod.h"
#include "mod_state.h"
#include "patch.h"
#include "patches_battle.h"
#include "patches_partner.h"
#include "tot_generate_condition.h"
#include "tot_generate_reward.h"
#include "tot_gsw.h"
#include "tot_manager_achievements.h"
#include "tot_manager_move.h"
#include "tot_party_mario.h"

#include <ttyd/battle.h>
#include <ttyd/battle_actrecord.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_information.h>
#include <ttyd/battle_monosiri.h>
#include <ttyd/battle_seq.h>
#include <ttyd/battle_seq_command.h>
#include <ttyd/battle_unit.h>
#include <ttyd/evt_eff.h>
#include <ttyd/evt_item.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/item_data.h>
#include <ttyd/mariost.h>
#include <ttyd/npcdrv.h>
#include <ttyd/seq_battle.h>
#include <ttyd/swdrv.h>
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
    void StartCalculateCoinDrops();
    void BranchBackCalculateCoinDrops();
    void StartSkipBanditEscapedCheck();
    void BranchBackSkipBanditEscapedCheck();
    void ConditionalBranchBackSkipBanditEscapedCheck();
    // currency_patches.s
    void StartCheckDeleteFieldItem();
    void BranchBackCheckDeleteFieldItem();
    // turn_seq_patches.s
    void StartCheckGradualSpRecovery();
    void BranchBackCheckGradualSpRecovery();
    
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

    void checkGradualSpRecovery() {
        mod::infinite_pit::battle_seq::CheckGradualSpRegenEffect();
    }
    
    int32_t calculateCoinDrops(
        ttyd::npcdrv::FbatBattleInformation* battleInfo,
        ttyd::npcdrv::NpcEntry* npc) {
        return mod::infinite_pit::battle_seq::CalculateCoinDrops(battleInfo, npc);
    }

    int32_t checkLetBanditEscape() {
        return mod::g_Mod->state_.CheckOptionValue(mod::tot::OPTVAL_BANDIT_NO_REFIGHT);
    }
}

namespace mod::infinite_pit {

namespace {

using ::ttyd::battle::BattleWork;
using ::ttyd::battle_database_common::BattleGroupSetup;
using ::ttyd::battle_database_common::BattleWeapon;
using ::ttyd::battle_database_common::PointDropData;
using ::ttyd::battle_unit::BattleWorkUnit;
using ::ttyd::evtmgr::EvtEntry;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;
using ::ttyd::npcdrv::FbatBattleInformation;
using ::ttyd::npcdrv::NpcBattleInfo;
using ::ttyd::npcdrv::NpcEntry;

namespace BattleUnitType = ::ttyd::battle_database_common::BattleUnitType;
namespace ItemType = ::ttyd::item_data::ItemType;

int32_t g_GradualSpRecoveryTurns = 0;

}

// Function hooks.
extern void (*g_seq_battleInit_trampoline)(void);
extern void (*g_fbatBattleMode_trampoline)(void);
extern void (*g_Btl_UnitSetup_trampoline)(BattleWork*);
extern void (*g_BtlActRec_JudgeRuleKeep_trampoline)(void);
extern void (*g__rule_disp_trampoline)(void);
extern BattleWeapon* (*g__GetFirstAttackWeapon_trampoline)(int32_t);
extern int32_t (*g_BattleSeqCmd_get_msg_trampoline)(EvtEntry*, bool);
extern void (*g__btlcmd_UpdateSelectWeaponTable_trampoline)(
    BattleWork*, int32_t);
extern int32_t (*g__btlcmd_MakeSelectWeaponTable_trampoline)(
    BattleWork*, int32_t);
extern void (*g_BattleCommandInit_trampoline)(BattleWork*);
extern void (*g_BattleInformationSetDropMaterial_trampoline)(
    FbatBattleInformation*);
// Patch addresses.
extern const int32_t g_fbatBattleMode_SkipStolenCheck_BH;
extern const int32_t g_fbatBattleMode_SkipStolenCheck_EH;
extern const int32_t g_fbatBattleMode_SkipStolenCheck_CH1;
extern const int32_t g_fbatBattleMode_CalculateCoinDrops_BH;
extern const int32_t g_fbatBattleMode_CalculateCoinDrops_EH;
extern const int32_t g_fbatBattleMode_GivePlayerInvuln_BH;
extern const int32_t g_fbatBattleMode_GivePlayerInvuln_EH;
extern const int32_t g_marioMain_Patch_SkipRunawayCoinDrop;
extern const int32_t g_btlSeqMove_FixMarioSingleMoveCheck_BH;
extern const int32_t g_btlSeqMove_FixMarioSingleMoveCheck_EH;
extern const int32_t g_btlseqTurn_Patch_RuleDispShowLonger;
extern const int32_t g_btlseqTurn_Patch_RuleDispDismissOnlyWithB;
extern const int32_t g_btlseqTurn_SpGradualRecoveryProc_BH;
extern const int32_t g_BtlUnit_Entry_Patch_BattleWorkUnitAlloc;
extern const int32_t g_BtlUnit_Entry_Patch_BattleWorkUnitMemset;
extern const int32_t g_btlseqEnd_JudgeRuleEarly_BH;
extern const int32_t g_btlseqEnd_Patch_RemoveJudgeRule;
extern const int32_t g_itemEntry_CheckDeleteFieldItem_BH;
extern const int32_t g_itemseq_Bound_Patch_BounceRange;
extern const int32_t g_enemy_common_dead_event_SpawnCoinsHook;

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
    auto& state = g_Mod->state_;
    state.ChangeOption(
        tot::STAT_RUN_TURNS_SPENT, ttyd::battle::g_BattleWork->turn_count);
    state.ChangeOption(
        tot::STAT_PERM_TURNS_SPENT, ttyd::battle::g_BattleWork->turn_count);
    state.ChangeOption(
        tot::STAT_RUN_MOST_TURNS_CURRENT, ttyd::battle::g_BattleWork->turn_count);
    if (state.GetOption(tot::STAT_RUN_MOST_TURNS_CURRENT) >
        state.GetOption(tot::STAT_RUN_MOST_TURNS_RECORD)) {
        // Update max turn count record.
        state.SetOption(tot::STAT_RUN_MOST_TURNS_RECORD,
            state.GetOption(tot::STAT_RUN_MOST_TURNS_CURRENT));
        state.SetOption(tot::STAT_RUN_MOST_TURNS_FLOOR, state.floor_);
    }
    if (fbat_info->wResult != 1) {
        state.ChangeOption(tot::STAT_RUN_TIMES_RAN_AWAY);
        state.ChangeOption(tot::STAT_PERM_TIMES_RAN_AWAY);
    }
    
    // Did not win the fight (e.g. ran away).
    if (fbat_info->wResult != 1) return;

    for (int32_t i = 0; i < 8; ++i) {
        if (npc_info->wStolenItems[i] != 0) {
            if (state.CheckOptionValue(tot::OPTVAL_BANDIT_FORCE_REFIGHT)) {
                // Did not win the fight (an enemy still has a stolen item).
                return;
            }
            // If no forced refights, achievement for getting away.
            tot::AchievementsManager::MarkCompleted(
                tot::AchievementId::MISC_BANDIT_STEAL);
            break;
        }
    }
    
    // If condition is a success and rule is not 0, add a bonus item.
    if (fbat_info->wBtlActRecCondition && fbat_info->wRuleKeepResult == 6) {
        int32_t item_reward = 0;
        if (state.CheckOptionValue(tot::OPTVAL_DROP_HELD_FROM_BONUS)) {
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
        state.ChangeOption(tot::STAT_RUN_CONDITIONS_MET);
        state.ChangeOption(tot::STAT_PERM_CONDITIONS_MET);
    }
    
    // If battle reward mode is "drop all held", award items other than the
    // natural drop ones until there are no "recovered items" slots left.
    if (state.CheckOptionValue(tot::OPTVAL_DROP_ALL_HELD)) {
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
}

// Displays text associated with the battle condition.
void DisplayBattleCondition() {
    char buf[128];
    tot::GetBattleConditionString(buf);
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
    
    switch (g_Mod->state_.GetOptionValue(tot::OPT_BATTLE_DROPS)) {
        // If using default battle drop behavior, select the item drop based on
        // the previously determined enemy held item index.
        case tot::OPTVAL_DROP_STANDARD:
        case tot::OPTVAL_DROP_ALL_HELD: {
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
            // Init badge move levels and reset selected move levels.
            tot::MoveManager::InitBadgeMoveLevels();
            tot::MoveManager::ResetSelectedLevels();
            // Force enemy ATK/DEF tattles to display at start of encounter.
            partner::RefreshExtraTattleStats();
            // Reset cost of Quick Change switches.
            battle::ResetPartySwitchCost();
            // Reset SP regen status from prior fights.
            g_GradualSpRecoveryTurns = 0;
            // Reset current Star Power to 0 + 50 per copy of Super Start.
            const int32_t sp = Min(
                ttyd::mario_pouch::pouchEquipCheckBadge(
                    ItemType::TOT_SUPER_START) * 50,
                static_cast<int32_t>(ttyd::mario_pouch::pouchGetPtr()->max_sp));
            ttyd::mario_pouch::pouchGetPtr()->current_sp = sp;
            g_seq_battleInit_trampoline();
        });

    g_fbatBattleMode_trampoline = patch::hookFunction(
        ttyd::npcdrv::fbatBattleMode, []() {
            bool post_battle_state = ttyd::npcdrv::fbatGetPointer()->state == 4;
            g_fbatBattleMode_trampoline();
            // Copy information back to parent npc after battle, if applicable.
            if (post_battle_state) CopyChildBattleInfo(/* to_child = */ false);
        });

    // Enlarge size of BattleWorkUnit struct to add ToT-relevant fields.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_BtlUnit_Entry_Patch_BattleWorkUnitAlloc),
        0x38600b50U /* li r3, 0xb34 -> 0xb50 */);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_BtlUnit_Entry_Patch_BattleWorkUnitMemset),
        0x38a00b50U /* li r5, 0xb34 -> 0xb50 */);

    g_Btl_UnitSetup_trampoline = patch::hookFunction(
        ttyd::battle::Btl_UnitSetup, [](BattleWork* battleWork) {
            // Run original logic.
            g_Btl_UnitSetup_trampoline(battleWork);
            // Set "badge equipped" flags based on new key item flags.
            if (tot::GetSWF(tot::GSWF_PeekabooEnabled)) {
                battleWork->badge_equipped_flags |= 2;
            }
            if (tot::GetSWF(tot::GSWF_TimingTutorEnabled)) {
                battleWork->badge_equipped_flags |= 4;
            }
        });
        
    g_BtlActRec_JudgeRuleKeep_trampoline = patch::hookFunction(
        ttyd::battle_actrecord::BtlActRec_JudgeRuleKeep, []() {
            g_BtlActRec_JudgeRuleKeep_trampoline();
            // Handle item drops from conditions / "all held drops" mode.
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
        
    // Hook to disable moves under special circumstances.
    g__btlcmd_UpdateSelectWeaponTable_trampoline = patch::hookFunction(
        ttyd::battle_seq_command::_btlcmd_UpdateSelectWeaponTable, [](
            BattleWork* battleWork, int32_t table_type) {
            // Run original logic.
            g__btlcmd_UpdateSelectWeaponTable_trampoline(battleWork, table_type);
            
            ttyd::battle::BattleWorkCommandCursor* cursor;
            ttyd::battle_seq_command::_btlcmd_GetCursorPtr(
                &battleWork->command_work, table_type, &cursor);
            if (!cursor) return;
            // Run extra logic to disable moves under special circumstances.
            for (int32_t i = 0; i < cursor->num_options; ++i) {
                auto& weapon = battleWork->command_work.weapon_table[i];
                if (!weapon.weapon || weapon.item_id) continue;
                switch (weapon.index) {
                    case tot::MoveType::VIVIAN_INFATUATE: {
                        // Disable if there is already an Infatuated enemy.
                        bool disable = false;
                        for (int32_t i = 0; i < 64; ++i) {
                            auto* unit = battleWork->battle_units[i];
                            if (unit && unit->alliance == 0 &&
                                unit->current_kind <= BattleUnitType::BONETAIL) {
                                disable = true;
                                break;
                            }
                        }
                        if (disable) {
                            weapon.unk_04 = 0;
                            weapon.unk_18 = 10;
                        }
                        break;
                    }
                    case tot::MoveType::BOBBERY_MEGATON_BOMB: {
                        // Disable if there is already an active Megaton Bomb.
                        bool disable = false;
                        for (int32_t i = 0; i < 64; ++i) {
                            auto* unit = battleWork->battle_units[i];
                            if (unit &&
                                unit->current_kind == BattleUnitType::BOMB_SQUAD_BOMB &&
                                unit->unit_work[2] == 2) {
                                disable = true;
                                break;
                            }
                        }
                        if (disable) {
                            weapon.unk_04 = 0;
                            weapon.unk_18 = 11;
                        }
                        break;
                    }
                }
            }
        });
        
    // Handle additional "can't select move" message types.
    g_BattleSeqCmd_get_msg_trampoline = patch::hookFunction(
        ttyd::battle_seq_command::BattleSeqCmd_get_msg,
        [](EvtEntry* evt, bool isFirstCall) {
            // Check for new message types.
            auto* battleWork = ttyd::battle::g_BattleWork;
            switch (battleWork->command_work.selection_error_msg) {
                case 10:
                    evtSetValue(
                        evt, evt->evtArguments[0], PTR("tot_selerr_infatuate"));
                    return 2;
                case 11:
                    evtSetValue(
                        evt, evt->evtArguments[0], PTR("tot_selerr_megaton_bomb"));
                    return 2;
            }
            // Run vanilla logic.
            return g_BattleSeqCmd_get_msg_trampoline(evt, isFirstCall);
        });
    
    g_BattleCommandInit_trampoline = patch::hookFunction(
        ttyd::battle_seq_command::BattleCommandInit, [](BattleWork* battleWork) {
            // Reset selected move levels before every player action.
            tot::MoveManager::ResetSelectedLevels();
            // Run original logic.
            g_BattleCommandInit_trampoline(battleWork);
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

    // Disable forced re-fights with enemies w/ stolen items, if option enabled.
    mod::patch::writeBranch(
        reinterpret_cast<void*>(g_fbatBattleMode_SkipStolenCheck_BH),
        reinterpret_cast<void*>(StartSkipBanditEscapedCheck));
    mod::patch::writeBranch(
        reinterpret_cast<void*>(BranchBackSkipBanditEscapedCheck),
        reinterpret_cast<void*>(g_fbatBattleMode_SkipStolenCheck_EH));
    mod::patch::writeBranch(
        reinterpret_cast<void*>(ConditionalBranchBackSkipBanditEscapedCheck),
        reinterpret_cast<void*>(g_fbatBattleMode_SkipStolenCheck_CH1));

    // Disable dropping coins after running away from a battle.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_marioMain_Patch_SkipRunawayCoinDrop),
        0x60000000U /* nop */);

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
        
    // Override logic to calculate coin drops to fix the original's overflow,
    // as well as dividing the result by 2 and capping to 100.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_fbatBattleMode_CalculateCoinDrops_BH),
        reinterpret_cast<void*>(g_fbatBattleMode_CalculateCoinDrops_EH),
        reinterpret_cast<void*>(StartCalculateCoinDrops),
        reinterpret_cast<void*>(BranchBackCalculateCoinDrops));
        
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
    
    // Handle per-turn SP recovery.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_btlseqTurn_SpGradualRecoveryProc_BH),
        reinterpret_cast<void*>(StartCheckGradualSpRecovery),
        reinterpret_cast<void*>(BranchBackCheckGradualSpRecovery));
}

int32_t CalculateCoinDrops(FbatBattleInformation* battleInfo, NpcEntry* npc) {
    int32_t result = npc->wBaseCoinCount;
    if (battleInfo->wResult == 1) {
        int32_t base_coins = battleInfo->wCoinDropCount;
        int32_t multiplier =
            1 + ttyd::mario_pouch::pouchEquipCheckBadge(ItemType::DOUBLE_PAIN);
        
        // Merlee coin multiplier; x2 instead of +2 to multiplier.
        if (*(int8_t*)((uintptr_t)ttyd::mariost::g_MarioSt->fbatData + 0x54c)) {
            multiplier *= 2;
        }
        
        // Divide ultimate result by 2.
        result = base_coins * multiplier / 2;

        // If Grubba's conditions are active, double or nothing, based on
        // whether the condition was met.
        int32_t grubba_floor =
            g_Mod->state_.GetOption(tot::STAT_RUN_NPC_GRUBBA_FLOOR);
        if (grubba_floor && g_Mod->state_.floor_ - grubba_floor < 8) {
            if (battleInfo->wRuleKeepResult == 6) {
                result *= 2;
            } else {
                result = 0;
            }
        }
    }

    // Check for dead partners, and give achievement if any exist.
    for (int32_t i = 1; i <= 7; ++i) {
        const auto& data = ttyd::mario_pouch::pouchGetPtr()->party_data[i];
        if ((data.flags & 1) && data.current_hp == 0) {
            tot::AchievementsManager::MarkCompleted(
                tot::AchievementId::MISC_FAINTED_PARTNER);
        }
    }

    return result < 100 ? result : 100;
}

void StoreGradualSpRegenEffect() {
    g_GradualSpRecoveryTurns = 3;
}

EVT_BEGIN(ApplyStardustEffectToMario)
    USER_FUNC(
        ttyd::battle_event_cmd::btlevtcmd_GetPos, -3, LW(0), LW(1), LW(2))
    USER_FUNC(
        ttyd::evt_eff::evt_eff, PTR(""), PTR("stardust"), 2,
        LW(0), LW(1), LW(2), 50, 50, 50, 100, 0, 0, 0, 0)
    RETURN()
EVT_END()

void CheckGradualSpRegenEffect() {
    if (g_GradualSpRecoveryTurns > 0) {
        --g_GradualSpRecoveryTurns;
        auto& pouch = *ttyd::mario_pouch::pouchGetPtr();
        pouch.current_sp += pouch.max_sp / 3;
        if (pouch.current_sp > pouch.max_sp) pouch.current_sp = pouch.max_sp;
        ttyd::evtmgr::evtEntry(
            const_cast<int32_t*>(ApplyStardustEffectToMario), 0, 0);
    }
}

}  // namespace battle_seq
}  // namespace mod::infinite_pit