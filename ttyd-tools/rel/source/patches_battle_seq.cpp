#include "patches_battle_seq.h"

#include "common_functions.h"
#include "common_ui.h"
#include "evt_cmd.h"
#include "mod.h"
#include "patch.h"
#include "patches_battle.h"
#include "patches_partner.h"
#include "tot_generate_condition.h"
#include "tot_gsw.h"
#include "tot_manager_achievements.h"
#include "tot_manager_move.h"
#include "tot_manager_reward.h"
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
#include <ttyd/evt_snd.h>
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
        mod::tot::patch::battle_seq::CheckGradualSpRegenEffect();
    }
    
    int32_t calculateCoinDrops(
        ttyd::npcdrv::FbatBattleInformation* battleInfo,
        ttyd::npcdrv::NpcEntry* npc) {
        return mod::tot::patch::battle_seq::CalculateCoinDrops(battleInfo, npc);
    }

    int32_t checkLetBanditEscape() {
        return mod::g_Mod->state_.CheckOptionValue(mod::tot::OPTVAL_BANDIT_NO_REFIGHT);
    }
}

namespace mod::tot::patch {

namespace {

// For convenience.
using namespace ::ttyd::battle_event_cmd;

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
extern int32_t (*g_BattleWaitAllActiveEvtEnd_trampoline)(BattleWork*);
extern void (*g__rule_disp_trampoline)(void);
extern BattleWeapon* (*g__GetFirstAttackWeapon_trampoline)(int32_t);
extern int32_t (*g_BattleSeqCmd_get_msg_trampoline)(EvtEntry*, bool);
extern int32_t (*g_BattleCommandInput_trampoline)(BattleWork*);
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

EVT_BEGIN(CountdownKillEvt)
    USER_FUNC(ttyd::evt_snd::evt_snd_sfxon, PTR("SFX_BTL_THUNDERS_ATTACK4"), 0)
    USER_FUNC(ttyd::evt_eff::evt_eff, PTR(""), PTR("sandars"), 
        3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(btlevtcmd_DamageDirect, -3, 1, 99, 0x100, 3, 0)
    USER_FUNC(btlevtcmd_GetUnitId, -4, LW(15))
    IF_NOT_EQUAL(LW(15), -1)
        USER_FUNC(btlevtcmd_GetHp, -4, LW(15))
        IF_LARGE(LW(15), 0)
            USER_FUNC(btlevtcmd_DamageDirect, -4, 1, 99, 0x100, 3, 0)
        END_IF()
    END_IF()
    RETURN()
EVT_END()

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
        mod::writePatch(
            &dest->battleInfo, &src->battleInfo, sizeof(NpcBattleInfo));
    }
}

// Checks whether the battle condition was satisfied, and if so,
// adds a bonus item to the "recovered items" pool.
void CheckBattleCondition() {
    auto* battleWork = ttyd::battle::g_BattleWork;
    auto* fbat_info = ttyd::battle::g_BattleWork->fbat_info;
    const auto& actrec = ttyd::battle::g_BattleWork->act_record_work;
    NpcBattleInfo* npc_info = fbat_info->wBattleInfo;

    // Track the number of turns spent / number of run aways at fight's end.
    auto& state = g_Mod->state_;
    state.ChangeOption(STAT_RUN_TURNS_SPENT, battleWork->turn_count);
    state.ChangeOption(STAT_PERM_TURNS_SPENT, battleWork->turn_count);
    state.ChangeOption(STAT_RUN_MOST_TURNS_CURRENT, battleWork->turn_count);
    if (state.GetOption(STAT_RUN_MOST_TURNS_CURRENT) >
        state.GetOption(STAT_RUN_MOST_TURNS_RECORD)) {
        // Update max turn count record.
        state.SetOption(STAT_RUN_MOST_TURNS_RECORD,
            state.GetOption(STAT_RUN_MOST_TURNS_CURRENT));
        state.SetOption(STAT_RUN_MOST_TURNS_FLOOR, state.floor_);
    }
    if (fbat_info->wResult != 1) {
        state.ChangeOption(STAT_RUN_TIMES_RAN_AWAY);
        state.ChangeOption(STAT_PERM_TIMES_RAN_AWAY);
        if (state.GetOption(STAT_PERM_TIMES_RAN_AWAY) >= 30) {
            AchievementsManager::MarkCompleted(AchievementId::V2_AGG_RUN_AWAY_30);
        }
    }

    // Track whether Mario used any actions other than Hammer or Tactics.
    if (actrec.mario_times_jump_moves_used +
        actrec.mario_times_attacking_special_moves_used +
        actrec.mario_times_non_attacking_special_moves_used +
        actrec.mario_num_times_attack_items_used +
        actrec.mario_num_times_non_attack_items_used > 0) {
        state.ChangeOption(STAT_RUN_HAMMERMAN_FAILED);
    }
    
    // Did not win the fight (e.g. ran away).
    if (fbat_info->wResult != 1) return;

    for (int32_t i = 0; i < 8; ++i) {
        if (npc_info->wStolenItems[i] != 0) {
            if (state.CheckOptionValue(OPTVAL_BANDIT_FORCE_REFIGHT)) {
                // Did not win the fight (an enemy still has a stolen item).
                return;
            }
            // If no forced refights, achievement for getting away.
            AchievementsManager::MarkCompleted(AchievementId::MISC_BANDIT_STEAL);
            break;
        }
    }
    
    // If condition is a success and rule is not 0, add a bonus item.
    if (fbat_info->wBtlActRecCondition && fbat_info->wRuleKeepResult == 6) {
        int32_t item_reward = 0;
        if (state.CheckOptionValue(OPTVAL_DROP_HELD_FROM_BONUS)) {
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
        state.ChangeOption(STAT_RUN_CONDITIONS_MET);
        state.ChangeOption(STAT_PERM_CONDITIONS_MET);

        // Track whether all of Grubba's conditions for a set have been met.
        int32_t grubba_floor = state.GetOption(STAT_RUN_NPC_GRUBBA_FLOOR);
        if (grubba_floor && state.floor_ - grubba_floor < 8) {
            state.ChangeOption(STAT_RUN_NPC_GRUBBA_COMBO);
            if (state.GetOption(STAT_RUN_NPC_GRUBBA_COMBO) == 7) {
                AchievementsManager::MarkCompleted(AchievementId::V3_RUN_GRUBBA);
            }
        }
    } else {
        // Set to placeholder so progress on achievement isn't shown.
        state.SetOption(STAT_RUN_NPC_GRUBBA_COMBO, 99);
        state.ChangeOption(STAT_RUN_CONDITIONS_FAILED);
    }
    
    // If battle reward mode is "drop all held", award items other than the
    // natural drop ones until there are no "recovered items" slots left.
    if (state.CheckOptionValue(OPTVAL_DROP_ALL_HELD)) {
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
    
    switch (g_Mod->state_.GetOptionValue(OPT_BATTLE_DROPS)) {
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
    g_seq_battleInit_trampoline = mod::hookFunction(
        ttyd::seq_battle::seq_battleInit, []() {
            // Copy information from parent npc before battle, if applicable.
            CopyChildBattleInfo(/* to_child = */ true);
            // Init badge move levels and reset selected move levels.
            MoveManager::InitBadgeMoveLevels();
            MoveManager::ResetSelectedLevels();
            // Reset cost of Quick Change switches.
            battle::ResetPartySwitchCost();
            // Reset SP regen status from prior fights.
            g_GradualSpRecoveryTurns = 0;
            // Reset Star Power to 0, or 50 + 25 per extra copy of Super Start.
            int32_t starting_sp = 0;
            if (int32_t super_starts = ttyd::mario_pouch::pouchEquipCheckBadge(
                    ItemType::TOT_SUPER_START); super_starts > 0) {
                starting_sp = (super_starts + 1) * 25;
            }
            ttyd::mario_pouch::pouchGetPtr()->current_sp = Min(
                starting_sp,
                static_cast<int32_t>(ttyd::mario_pouch::pouchGetPtr()->max_sp));
            // Reset types of enemies defeated this fight.
            for (int32_t i = 0; i < 8; ++i) {
                g_Mod->state_.SetOption(STAT_RUN_TYPES_THIS_FIGHT, 0, i);
            }
            // Reset midboss helper spawn RNG.
            g_Mod->state_.rng_states_[RNG_MIDBOSS_MOB] = 0;

            g_seq_battleInit_trampoline();
        });

    g_fbatBattleMode_trampoline = mod::hookFunction(
        ttyd::npcdrv::fbatBattleMode, []() {
            bool post_battle_state = ttyd::npcdrv::fbatGetPointer()->state == 4;
            g_fbatBattleMode_trampoline();
            // Copy information back to parent npc after battle, if applicable.
            if (post_battle_state) CopyChildBattleInfo(/* to_child = */ false);
        });

    // Enlarge size of BattleWorkUnit struct to add ToT-relevant fields.
    mod::writePatch(
        reinterpret_cast<void*>(g_BtlUnit_Entry_Patch_BattleWorkUnitAlloc),
        0x38600b50U /* li r3, 0xb34 -> 0xb50 */);
    mod::writePatch(
        reinterpret_cast<void*>(g_BtlUnit_Entry_Patch_BattleWorkUnitMemset),
        0x38a00b50U /* li r5, 0xb34 -> 0xb50 */);

    g_Btl_UnitSetup_trampoline = mod::hookFunction(
        ttyd::battle::Btl_UnitSetup, [](BattleWork* battleWork) {
            // Run original logic.
            g_Btl_UnitSetup_trampoline(battleWork);
            // Set "badge equipped" flags based on new key item flags.
            if (GetSWF(GSWF_PeekabooEnabled)) {
                battleWork->badge_equipped_flags |= 2;
            }
            if (GetSWF(GSWF_TimingTutorEnabled)) {
                battleWork->badge_equipped_flags |= 4;
            }
        });
        
    g_BtlActRec_JudgeRuleKeep_trampoline = mod::hookFunction(
        ttyd::battle_actrecord::BtlActRec_JudgeRuleKeep, []() {
            g_BtlActRec_JudgeRuleKeep_trampoline();
            // Handle item drops from conditions / "all held drops" mode.
            CheckBattleCondition();
        });
        
    g__rule_disp_trampoline = mod::hookFunction(
        ttyd::battle_seq::_rule_disp, []() {
            // Replaces the original logic completely.
            DisplayBattleCondition();
        });
        
    g__GetFirstAttackWeapon_trampoline = mod::hookFunction(
        ttyd::battle_seq::_GetFirstAttackWeapon, [](int32_t attack_type) {
            // Replaces the original logic completely.
            return tot::party_mario::GetFirstAttackWeapon(attack_type);
        });
        
    g__btlcmd_MakeSelectWeaponTable_trampoline = mod::hookFunction(
        ttyd::battle_seq_command::_btlcmd_MakeSelectWeaponTable, [](
            BattleWork* battleWork, int32_t table_type) {
            // Replaces the original logic completely.
            return tot::party_mario::MakeSelectWeaponTable(
                battleWork, table_type);
        });
        
    // Hook to disable moves under special circumstances.
    g__btlcmd_UpdateSelectWeaponTable_trampoline = mod::hookFunction(
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
                    case MoveType::VIVIAN_INFATUATE: {
                        // Disable if there is already an Infatuated enemy.
                        bool disable = false;
                        for (int32_t i = 0; i < 64; ++i) {
                            auto* unit = battleWork->battle_units[i];
                            if (unit &&
                                unit->current_kind <= BattleUnitType::BONETAIL &&
                                unit->alliance == 0 && unit->current_hp > 0) {
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
                    case MoveType::BOBBERY_MEGATON_BOMB: {
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
    g_BattleSeqCmd_get_msg_trampoline = mod::hookFunction(
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
    
    g_BattleCommandInit_trampoline = mod::hookFunction(
        ttyd::battle_seq_command::BattleCommandInit, [](BattleWork* battleWork) {
            // Reset selected move levels before every player action.
            MoveManager::ResetSelectedLevels();
            // Run original logic.
            g_BattleCommandInit_trampoline(battleWork);
        });
        
    g_BattleInformationSetDropMaterial_trampoline = mod::hookFunction(
        ttyd::battle_information::BattleInformationSetDropMaterial,
        [](FbatBattleInformation* fbat_info) {
            // Replaces the original logic completely.
            GetDropMaterials(fbat_info);
        });
        
    // Make Mario's jump/hammer menus always show the full weapon sub-menu.
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_btlSeqMove_FixMarioSingleMoveCheck_BH),
        reinterpret_cast<void*>(g_btlSeqMove_FixMarioSingleMoveCheck_EH),
        reinterpret_cast<void*>(StartFixMarioSingleMoveCheck),
        reinterpret_cast<void*>(BranchBackFixMarioSingleMoveCheck));
        
    // Make defeating a group of enemies still holding stolen items always make
    // you have temporary intangibility, even if you recovered some of them, to
    // prevent projectiles from first-striking you again if you recover items.
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_fbatBattleMode_GivePlayerInvuln_BH),
        reinterpret_cast<void*>(g_fbatBattleMode_GivePlayerInvuln_EH),
        reinterpret_cast<void*>(StartGivePlayerInvuln),
        reinterpret_cast<void*>(BranchBackGivePlayerInvuln));

    // Disable forced re-fights with enemies w/ stolen items, if option enabled.
    mod::writeBranch(
        reinterpret_cast<void*>(g_fbatBattleMode_SkipStolenCheck_BH),
        reinterpret_cast<void*>(StartSkipBanditEscapedCheck));
    mod::writeBranch(
        reinterpret_cast<void*>(BranchBackSkipBanditEscapedCheck),
        reinterpret_cast<void*>(g_fbatBattleMode_SkipStolenCheck_EH));
    mod::writeBranch(
        reinterpret_cast<void*>(ConditionalBranchBackSkipBanditEscapedCheck),
        reinterpret_cast<void*>(g_fbatBattleMode_SkipStolenCheck_CH1));

    // Disable dropping coins after running away from a battle.
    mod::writePatch(
        reinterpret_cast<void*>(g_marioMain_Patch_SkipRunawayCoinDrop),
        0x60000000U /* nop */);

    // Check for battle conditions at the start of processing the battle end,
    // not the end; this way level-up heals don't factor into "final HP".
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_btlseqEnd_JudgeRuleEarly_BH),
        reinterpret_cast<void*>(StartBtlSeqEndJudgeRule),
        reinterpret_cast<void*>(BranchBackBtlSeqEndJudgeRule));
    // Remove the original check for battle conditions at the end of battle.
    mod::writePatch(
        reinterpret_cast<void*>(g_btlseqEnd_Patch_RemoveJudgeRule), 
        0x60000000U /* nop */);

    // Make the battle condition message display longer (5s instead of 2s),
    // and only be dismissable by the B button, rather than A or B.
    mod::writePatch(
        reinterpret_cast<void*>(g_btlseqTurn_Patch_RuleDispShowLonger), 
        0x3800012cU /* li r0, 300 */);
    mod::writePatch(
        reinterpret_cast<void*>(g_btlseqTurn_Patch_RuleDispDismissOnlyWithB), 
        0x38600200U /* li r3, 0x200 (B button) */);
        
    // Override logic to calculate coin drops to fix the original's overflow,
    // as well as dividing the result by 2 and capping to 100.
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_fbatBattleMode_CalculateCoinDrops_BH),
        reinterpret_cast<void*>(g_fbatBattleMode_CalculateCoinDrops_EH),
        reinterpret_cast<void*>(StartCalculateCoinDrops),
        reinterpret_cast<void*>(BranchBackCalculateCoinDrops));
        
    // Support multiple demonimations of coins dropping at the end of a fight.
    mod::writePatch(
        reinterpret_cast<void*>(g_enemy_common_dead_event_SpawnCoinsHook),
        SpawnCoinsEvtHook, sizeof(SpawnCoinsEvtHook));
    // Allow big coins to be overridden by other items on the field.
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_itemEntry_CheckDeleteFieldItem_BH),
        reinterpret_cast<void*>(StartCheckDeleteFieldItem),
        reinterpret_cast<void*>(BranchBackCheckDeleteFieldItem));     
    // Make bigger coins bounce the same distance as regular coins, not
    // vanilla Piantas (which have the range of hearts, flowers, or items).
    mod::writePatch(
        reinterpret_cast<void*>(g_itemseq_Bound_Patch_BounceRange),
        0x2c00007b /* cmpwi r0,0x7b (heart, not Pianta) */);
    
    // Handle per-turn SP recovery.
    mod::writeBranchPair(
        reinterpret_cast<void*>(g_btlseqTurn_SpGradualRecoveryProc_BH),
        reinterpret_cast<void*>(StartCheckGradualSpRecovery),
        reinterpret_cast<void*>(BranchBackCheckGradualSpRecovery));

    // Handle Countdown Timer in-battle...
    // On player's turn, wrest control if timer elapses.
    g_BattleCommandInput_trampoline = mod::hookFunction(
        ttyd::battle_seq_command::BattleCommandInput, [](BattleWork* battleWork) {
            // Run original logic.
            int32_t result = g_BattleCommandInput_trampoline(battleWork);
            // Check for countdown timer elapsing, and end turn if so.
            if (GetSWByte(GSW_CountdownTimerState) >= 2) {
                auto* command_work = &battleWork->command_work;
                command_work->unk_54c[0x558 - 0x54c] = 1;
                if (command_work->state == 12 || command_work->state == 13) {
                    command_work->unk_54c[0x558 - 0x54c] = 2;
                }
                ttyd::battle_seq_command::BattleCommandDisplay_AllEnd(battleWork);
                command_work->unk_544 = command_work->current_cursor_type;
                // Out of range, will end attack immediately.
                command_work->current_cursor_type = 14;
                command_work->state = 29;
            }
            return result;
        });
    // When blocking on an event after an action, if timer is up, kill player.
    g_BattleWaitAllActiveEvtEnd_trampoline = mod::hookFunction(
        ttyd::battle_seq::BattleWaitAllActiveEvtEnd, [](BattleWork* battleWork) {
            // Run original logic.
            int32_t result = g_BattleWaitAllActiveEvtEnd_trampoline(battleWork);
            if (result && GetSWByte(GSW_CountdownTimerState) >= 2) {
                // If attack ending, and battle would not otherwise end...
                if (ttyd::battle::BattleGetSeq(battleWork, 6) == 0x6000007 &&
                    !ttyd::battle_seq::BattleCheckConcluded(battleWork)) {
                    // Remove Life Shrooms and do direct lethal damage to party.
                    while (ttyd::mario_pouch::pouchCheckItem(ItemType::LIFE_SHROOM)) {
                        ttyd::mario_pouch::pouchRemoveItem(ItemType::LIFE_SHROOM);
                    }
                    ttyd::evtmgr::evtEntry(
                        const_cast<int32_t*>(CountdownKillEvt), 10, 0);
                    ttyd::battle::BattleSetSeq(battleWork, 0, 3);
                    ttyd::battle::BattleSetSeq(battleWork, 7, 0x7000000);
                    battleWork->fbat_info->wResult = 3;
                    SetSWByte(GSW_CountdownTimerState, 3);
                    return 1;
                }
            }
            return result;
        });
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
            g_Mod->state_.GetOption(STAT_RUN_NPC_GRUBBA_FLOOR);
        if (grubba_floor && g_Mod->state_.floor_ - grubba_floor < 8) {
            if (battleInfo->wRuleKeepResult == 6) {
                result *= 2;
            } else {
                result = 0;
            }
        }

        if (result >= 100) {
            AchievementsManager::MarkCompleted(
                AchievementId::V2_MISC_BATTLE_COINS_100);
        }
    }

    // Check for dead partners, and give achievement if any exist.
    for (int32_t i = 1; i <= 7; ++i) {
        const auto& data = ttyd::mario_pouch::pouchGetPtr()->party_data[i];
        if ((data.flags & 1) && data.current_hp == 0) {
            AchievementsManager::MarkCompleted(AchievementId::MISC_FAINTED_PARTNER);
        }
    }

    return result < 100 ? result : 100;
}

void StoreGradualSpRegenEffect(int32_t turn_count) {
    g_GradualSpRecoveryTurns = turn_count;
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
}  // namespace mod::tot::patch