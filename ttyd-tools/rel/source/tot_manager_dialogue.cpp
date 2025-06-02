#include "tot_manager_dialogue.h"

#include "evt_cmd.h"
#include "mod.h"
#include "tot_generate_enemy.h"
#include "tot_gsw.h"
#include "tot_manager_achievements.h"
#include "tot_manager_move.h"
#include "tot_manager_reward.h"
#include "tot_state.h"

#include <ttyd/battle_database_common.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/mario_party.h>
#include <ttyd/system.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::tot {

namespace {

// For storing / sorting frequency of taking a particular reward type.
struct NpcGStats {
    int16_t reward_type;
    int16_t reward_rate_taken;
};
int32_t comp_NpcGStats(NpcGStats* lhs, NpcGStats* rhs) {
    return lhs->reward_rate_taken - rhs->reward_rate_taken;
}

using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

namespace BattleUnitType = ::ttyd::battle_database_common::BattleUnitType;

// Default conversation type: single text box from a speaker.
int8_t kDefaultConversation[] = { 0, -1 };

int8_t kActorPartnerDialogue[] = { 0, 1, -1 };
int8_t kPartnerConversation[] = { 1, -1 };

int32_t g_ConversationId = 0;
int8_t* g_ConversationPtr = nullptr;
int32_t g_ConversationStep = 0;

// Used to indicate where the next conversation should pick up, for
// conversations with programmatic beginnings / endings like NPC G's.
int32_t g_ConversationContinueId[3] = { 0, 0, 0 };

}  // namespace

void DialogueManager::SetConversation(int32_t id) {
    auto& state = g_Mod->state_;

    g_ConversationId = id;
    g_ConversationPtr = kDefaultConversation;
    g_ConversationStep = 0;

    // Used for most NPCs.
    int32_t completed_runs =
        state.GetOption(STAT_PERM_HALF_FINISHES) +
        state.GetOption(STAT_PERM_FULL_FINISHES) +
        state.GetOption(STAT_PERM_EX_FINISHES);
    // If using a special filename mode, treat it like a post-tutorial state,
    // but don't allow the secret boss hint to be mentioned.
    if (state.CheckOptionValue(OPTVAL_RACE_MODE_ENABLED) ||
        state.CheckOptionValue(OPTVAL_100_MODE_ENABLED))
        completed_runs = 4;

    // Commonly-used sources of pseudo-random conversation choice.
    int32_t damage_dealt = state.GetOption(STAT_PERM_ENEMY_DAMAGE);
    int32_t coins_earned = state.GetOption(STAT_PERM_META_COINS_EARNED);

    // For some NPCs, override the default conversation type based on
    // pseudo-random factors like run stats, or other conditions.
    switch (id) {
        case ConversationId::NPC_A: {
            if (g_ConversationContinueId[0]) {
                // Add predetermined ending to previous conversation.
                g_ConversationId = g_ConversationContinueId[0];
                g_ConversationContinueId[0] = 0;
                break;
            }

            int32_t cv_type = GetSWByte(GSW_NpcA_SpecialConversation);
            switch (cv_type) {
                case 10:
                    // Generic win message.
                    g_ConversationId = ConversationId::NPC_A_W;
                    break;
                case 11:
                    // First EX difficulty clear.
                    g_ConversationId = ConversationId::NPC_A_W_EXDIFF;
                    break;
                case 12:
                    // New personal best run time.
                    g_ConversationId = ConversationId::NPC_A_W_TIME;
                    break;
                case 13:
                    // New personal intensity record cleared.
                    g_ConversationId = ConversationId::NPC_A_W_INTENSITY;
                    break;
                case 40:
                    // Lost to countdown timer timeout.
                    g_ConversationId = ConversationId::NPC_A_L_TIMEUP;
                    break;
                case 20: {
                    // Generic loss message.
                    g_ConversationId = ConversationId::NPC_A_L;

                    int32_t attacker = state.GetOption(STAT_PERM_LAST_ATTACKER);
                    switch (attacker) {
                        case BattleUnitType::HOOKTAIL:
                            g_ConversationId = ConversationId::NPC_A_L_HOOK;
                            break;
                        case BattleUnitType::GLOOMTAIL:
                            g_ConversationId = ConversationId::NPC_A_L_GLOOM;
                            break;
                        case BattleUnitType::BONETAIL:
                            g_ConversationId = ConversationId::NPC_A_L_BONE;
                            break;
                        case BattleUnitType::GOLD_FUZZY:
                        case BattleUnitType::FUZZY_HORDE:
                            g_ConversationId = ConversationId::NPC_A_L_SBOSS1;
                            break;
                        case BattleUnitType::BOWSER_CH_8:
                            g_ConversationId = ConversationId::NPC_A_L_SBOSS2A;
                            break;
                        case BattleUnitType::KAMMY_KOOPA:
                            g_ConversationId = ConversationId::NPC_A_L_SBOSS2B;
                            break;
                        case BattleUnitType::DOOPLISS_CH_8:
                        case BattleUnitType::DOOPLISS_CH_8_FAKE_MARIO:
                        case BattleUnitType::DOOPLISS_CH_8_GOOMBELLA:
                        case BattleUnitType::DOOPLISS_CH_8_KOOPS:
                        case BattleUnitType::DOOPLISS_CH_8_FLURRIE:
                        case BattleUnitType::DOOPLISS_CH_8_YOSHI:
                        case BattleUnitType::DOOPLISS_CH_8_VIVIAN:
                        case BattleUnitType::DOOPLISS_CH_8_BOBBERY:
                        case BattleUnitType::DOOPLISS_CH_8_MS_MOWZ:
                            g_ConversationId = ConversationId::NPC_A_L_SBOSS3;
                            break;
                        case BattleUnitType::BOMB_SQUAD_BOMB:
                            g_ConversationId = ConversationId::NPC_A_L_BOMB;
                            break;
                        case 0:
                        case BattleUnitType::SYSTEM:
                        case BattleUnitType::MARIO:
                        case BattleUnitType::GOOMBELLA:
                        case BattleUnitType::KOOPS:
                        case BattleUnitType::FLURRIE:
                        case BattleUnitType::YOSHI:
                        case BattleUnitType::VIVIAN:
                        case BattleUnitType::BOBBERY:
                        case BattleUnitType::MS_MOWZ:
                            break;
                        default: {
                            // Loss to a regular enemy, pick message at random.
                            int32_t num_cvs =
                                ConversationId::NPC_A_L_ENEMY_END -
                                ConversationId::NPC_A_L_ENEMY_START;
                            int32_t cv_id = ttyd::system::irand(num_cvs);
                            g_ConversationId =
                                ConversationId::NPC_A_L_ENEMY_START + cv_id;

                            // Pick a response with an appropriate level of
                            // sympathy based on how tough the enemy was.
                            int32_t level;
                            GetEnemyStats(attacker,
                                nullptr, nullptr, nullptr, nullptr, &level);
                            
                            int32_t num_endings =
                                ConversationId::NPC_A_L_RESP_END -
                                ConversationId::NPC_A_L_RESP_START;
                            level = (level + ttyd::system::irand(5) - 2) / 2;
                            if (level >= num_endings) level = num_endings - 1;
                            // Cue response for after this conversation ends.
                            g_ConversationContinueId[0] =
                                ConversationId::NPC_A_L_RESP_START + level;

                            break;
                        }
                    }
                    break;
                }
                default:
                    break;
            }

            // Clear special conversation flag.
            SetSWByte(GSW_NpcA_SpecialConversation, 0);

            break;
        }
        case ConversationId::NPC_A_FIRST_VISIT:
        case ConversationId::NPC_A_FIRST_CLEAR:
        case ConversationId::NPC_A_SECOND_CLEAR: {
            // Special-occasion cutscenes always have three dialogue boxes.
            static int8_t NpcA_CutsceneConversation[] =  { 0, 0, 0, -1 };
            g_ConversationPtr = NpcA_CutsceneConversation;
            break;
        }
        case ConversationId::NPC_B: {
            if (completed_runs < 1) break;
            
            int32_t num_cvs =
                ConversationId::NPC_B_CVS_END -
                ConversationId::NPC_B_CVS_START;
            int32_t cv_id = coins_earned % num_cvs;
            g_ConversationId = ConversationId::NPC_B_CVS_START + cv_id;
            break;
        }
        case ConversationId::NPC_C: {
            if (completed_runs < 1) break;
            
            int32_t num_cvs =
                ConversationId::NPC_C_CVS_END -
                ConversationId::NPC_C_CVS_START;
            int32_t cv_id = coins_earned % num_cvs;
            g_ConversationId = ConversationId::NPC_C_CVS_START + cv_id;
            
            if (completed_runs >= 5 && coins_earned % 11 == 0) {
                g_ConversationId = ConversationId::NPC_C_CVS_SPECIAL;
            }
            break;
        }
        case ConversationId::NPC_D: {
            if (completed_runs < 1 || !GetSWF(GSWF_NpcD_FirstTimeChat)) {
                SetSWF(GSWF_NpcD_FirstTimeChat);
                break;
            }

            int8_t const* grid;
            int8_t const* states;
            AchievementsManager::GetAchievementGrid(&grid);
            AchievementsManager::GetAchievementStates(&states, false);

            int16_t ids[AchievementId::MAX_ACHIEVEMENT + 10];
            int32_t num_cvs = 0;
            int32_t num_achievements = 0;
            int32_t num_total_achievements = 0;

            for (int32_t i = 0; i < AchievementId::MAX_ACHIEVEMENT; ++i) {
                if (AchievementsManager::IsSecret(grid[i])) continue;
                ++num_total_achievements;

                if (states[i] == 2) {
                    ++num_achievements;
                } else if (states[i] == 1) {
                    // Achievement visible but not completed; if there is
                    // a dialogue option for it, add it to the possibility list.
                    switch (grid[i]) {
                        case AchievementId::RUN_NPC_DEALS_7:
                        case AchievementId::RUN_NO_JUMP_HAMMER:
                        case AchievementId::RUN_ALL_MOVES_MAXED:
                        case AchievementId::RUN_ALL_CONDITIONS_MET:
                        case AchievementId::RUN_ALL_FLOORS_3_TURN:
                        case AchievementId::RUN_NO_DAMAGE:
                        case AchievementId::RUN_HIGH_INTENSITY:
                        case AchievementId::RUN_ZERO_STAT_1:
                        case AchievementId::RUN_ZERO_STAT_2:
                        case AchievementId::MISC_TRADE_OFF_BOSS:
                        case AchievementId::MISC_SUPERGUARD_BITE:
                        case AchievementId::MISC_SHINES_10:
                        case AchievementId::MISC_RUN_COINS_999:
                        case AchievementId::V2_MISC_SP_TURN1:
                        case AchievementId::V3_RUN_DEFEAT_7_TYPES:
                            ids[num_cvs++] =
                                ConversationId::NPC_D_CVS_BASE + grid[i];
                            break;
                        case AchievementId::META_ITEM_LOG_BASIC:
                            // Only hint after Zess T. isn't a spoiler.
                            if (completed_runs >= 3)
                                ids[num_cvs++] =
                                    ConversationId::NPC_D_CVS_BASE + grid[i];
                            break;
                        case AchievementId::META_ALL_OPTIONS: {
                            // Give individual hints for NPC options that
                            // aren't yet unlocked.
                            if (!GetSWF(GSWF_NpcF_SeedUnlocked)) {
                                ids[num_cvs++] =
                                    ConversationId::NPC_D_OPTIONS_SEED;
                            }
                            if (!GetSWF(GSWF_NpcK_CustomMovesUnlocked)) {
                                ids[num_cvs++] =
                                    ConversationId::NPC_D_OPTIONS_MOVES;
                            }
                            if (!GetSWF(GSWF_White_CountdownUnlocked)) {
                                ids[num_cvs++] =
                                    ConversationId::NPC_D_OPTIONS_TIMER;
                            }
                            
                            int32_t bosses_beaten =
                                GetSWF(GSWF_SecretBoss1_Beaten) +
                                GetSWF(GSWF_SecretBoss2_Beaten) +
                                GetSWF(GSWF_SecretBoss3_Beaten);
                            if (bosses_beaten > 0 && bosses_beaten < 3) {
                                ids[num_cvs++] =
                                    ConversationId::NPC_D_OPTIONS_BOSS;
                            }

                            break;
                        }
                        default:
                            break;
                    }
                }
            }

            if (num_achievements == num_total_achievements) {
                g_ConversationId = ConversationId::NPC_D_ALL_DONE;
            } else if (num_cvs < 1) {
                g_ConversationId = ConversationId::NPC_D_NONE_ACTIVE;
            } else {
                int32_t cv_id = coins_earned % num_cvs;
                g_ConversationId = ids[cv_id];
            }
            break;
        }
        case ConversationId::NPC_F: {
            int32_t num_cvs =
                ConversationId::NPC_F_CVS_END -
                ConversationId::NPC_F_CVS_START;
            int32_t cv_id = damage_dealt % num_cvs;
            g_ConversationId = ConversationId::NPC_F_CVS_START + cv_id;

            SetSWByte(GSW_NpcF_CurrentConversation, cv_id);
            break;
        }
        case ConversationId::NPC_INN: {
            if (completed_runs < 1 || !GetSWF(GSWF_Innkeeper_FirstTimeChat)) {
                SetSWF(GSWF_Innkeeper_FirstTimeChat);
                break;
            }
            
            int32_t num_cvs =
                ConversationId::NPC_INN_CVS_END -
                ConversationId::NPC_INN_CVS_START;
            int32_t cv_id = damage_dealt % num_cvs;
            g_ConversationId = ConversationId::NPC_INN_CVS_START + cv_id;
            break;
        }
        case ConversationId::NPC_G: {
            if (completed_runs < 1) {
                g_ConversationId = ConversationId::NPC_G_NO_CLEAR;
                break;
            } else if (!GetSWF(GSWF_NpcG_PostTutorialChat)) {
                g_ConversationId = ConversationId::NPC_G_FIRST_CLEAR;
                SetSWF(GSWF_NpcG_PostTutorialChat);
                break;
            } else if (g_ConversationContinueId[0]) {
                // Queue next conversation.
                g_ConversationId = g_ConversationContinueId[0];
                g_ConversationContinueId[0] = g_ConversationContinueId[1];
                g_ConversationContinueId[1] = 0;
                break;
            }

            NpcGStats stats[RewardStatId::MAX_REWARD_STAT];
            int32_t num_stats = 0;
            bool all_offered = true;

            for (int32_t i = 0; i < RewardStatId::MAX_REWARD_STAT; ++i) {
                switch (i) {
                    case RewardStatId::MOVE_FLURRIE:
                    case RewardStatId::MOVE_YOSHI:
                    case RewardStatId::MOVE_VIVIAN:
                    case RewardStatId::MOVE_BOBBERY:
                        if (completed_runs < 2) break;
                        // Intentional fallthrough.
                    default: {
                        // Calculate how often this reward was taken.
                        int32_t num_offered =
                            state.GetOption(STAT_PERM_REWARDS_OFFERED, i);
                        int32_t num_taken =
                            state.GetOption(STAT_PERM_REWARDS_TAKEN, i);

                        stats[num_stats].reward_type = i;
                        stats[num_stats].reward_rate_taken =
                            num_offered ? num_taken * 10000LL / num_offered : 0;
                        ++num_stats;

                        if (!num_offered) all_offered = false;
                            
                        break;
                    }
                }
            }

            // Pick conversation.
            int32_t cv_id = damage_dealt % num_stats;
            int32_t reward_type = stats[cv_id].reward_type;
            g_ConversationId = ConversationId::NPC_G_REWARD_START + reward_type;

            // Sort reward types by frequency taken, and find index.
            ttyd::system::qqsort(
                stats, num_stats, sizeof(NpcGStats), (void*)comp_NpcGStats);
            int32_t index = 0;
            for (; stats[index].reward_type != reward_type; ++index);
            
            // Set which ending the conversation should use based on how
            // frequently you've picked a particular type of reward.
            if (stats[index].reward_rate_taken < 500) {
                // If taken < 5% of the time:
                g_ConversationContinueId[0] = ConversationId::NPC_G_ENDING_NONE;
            } else if (stats[index].reward_rate_taken > 7500 || num_stats - index <= 3) {
                // If taken > 75% of the time, or in top three types taken:
                g_ConversationContinueId[0] = ConversationId::NPC_G_ENDING_HIGH;
            } else if (stats[index].reward_rate_taken < 2000 || index < 8) {
                // If taken < 20% of the time, or in bottom 40% of types:
                g_ConversationContinueId[0] = ConversationId::NPC_G_ENDING_LOW;
            } else {
                g_ConversationContinueId[0] = ConversationId::NPC_G_ENDING_MED;
            }

            // Prompt follow-up conversation, asking about individual stats.
            if (completed_runs >= 2 && !GetSWF(GSWF_NpcG_UnlockedRewardStats)) {
                if (all_offered) {
                    g_ConversationContinueId[1] = ConversationId::NPC_G_STATS_UNLOCK;
                    SetSWF(GSWF_NpcG_UnlockedRewardStats, 1);
                }
            } else if (GetSWF(GSWF_NpcG_UnlockedRewardStats)) {
                g_ConversationContinueId[1] = ConversationId::NPC_G_STATS_AFTER;
            }

            break;
        }
        case ConversationId::NPC_G_STATS: {
            if (g_ConversationContinueId[0]) {
                // Queue next conversation.
                g_ConversationId = g_ConversationContinueId[0];
                g_ConversationContinueId[0] = 0;
                break;
            }

            NpcGStats stats[RewardStatId::MAX_REWARD_STAT];
            int32_t num_stats = 0;

            for (int32_t i = 0; i < RewardStatId::MAX_REWARD_STAT; ++i) {
                // Calculate how often this reward was taken.
                int32_t num_offered =
                    state.GetOption(STAT_PERM_REWARDS_OFFERED, i);
                int32_t num_taken =
                    state.GetOption(STAT_PERM_REWARDS_TAKEN, i);

                stats[num_stats].reward_type = i;
                stats[num_stats].reward_rate_taken =
                    num_offered ? num_taken * 10000LL / num_offered : 0;
                ++num_stats;
            }

            // Sort reward types by frequency taken, and find index.
            ttyd::system::qqsort(
                stats, num_stats, sizeof(NpcGStats), (void*)comp_NpcGStats);
            int32_t index = 0;
            for (; stats[index].reward_type != GetSWByte(GSW_NpcG_CurrentStatBreakdown); ++index);
            
            // Set which ending the conversation should use based on how
            // frequently you've picked a particular type of reward.
            g_ConversationId = ConversationId::NPC_G_ENDING_MED;

            if (stats[index].reward_rate_taken < 500) {
                // If taken < 5% of the time:
                g_ConversationId = ConversationId::NPC_G_ENDING_NONE;
            } else if (stats[index].reward_rate_taken > 7500 || num_stats - index <= 3) {
                // If taken > 75% of the time, or in top three types taken:
                g_ConversationId = ConversationId::NPC_G_ENDING_HIGH;
            } else if (stats[index].reward_rate_taken < 2000 || index < 8) {
                // If taken < 20% of the time, or in bottom 40% of types:
                g_ConversationId = ConversationId::NPC_G_ENDING_LOW;
            }

            // Follow-up conversation asking about other stats.
            g_ConversationContinueId[0] = ConversationId::NPC_G_STATS_AFTER;

            break;
        }
        case ConversationId::NPC_H: {
            if (completed_runs < 1) break;
            
            int32_t num_cvs =
                ConversationId::NPC_H_CVS_END -
                ConversationId::NPC_H_CVS_START;
            int32_t cv_id = coins_earned % num_cvs;
            g_ConversationId = ConversationId::NPC_H_CVS_START + cv_id;
            break;
        }
        case ConversationId::NPC_I: {
            if (completed_runs < 1) {
                g_ConversationId = ConversationId::NPC_I_NO_CLEAR;
                break;
            } else if (completed_runs == 1) {
                g_ConversationId = ConversationId::NPC_I_FIRST_CLEAR;
                break;
            } else if (!GetSWF(GSWF_NpcI_PostTutorialChat)) {
                g_ConversationId = ConversationId::NPC_I_SECOND_CLEAR;
                SetSWF(GSWF_NpcI_PostTutorialChat);
                break;
            }
            
            int32_t num_cvs =
                ConversationId::NPC_I_CVS_END -
                ConversationId::NPC_I_CVS_START;
            int32_t cv_id = coins_earned % num_cvs;
            g_ConversationId = ConversationId::NPC_I_CVS_START + cv_id;
            break;
        }
        case ConversationId::NPC_K: {
            if (completed_runs < 1 || !GetSWF(GSWF_NpcK_FirstTimeChat)) {
                SetSWF(GSWF_NpcK_FirstTimeChat);
                break;
            }

            // Unlock move selector once all moves have been used.
            if (!GetSWF(GSWF_NpcK_CustomMovesUnlocked)) {
                bool all_moves_used = true;
                for (int32_t i = 0; i < MoveType::MOVE_TYPE_MAX; ++i) {
                    uint32_t flags = state.GetOption(STAT_PERM_MOVE_LOG, i);
                    if (!(flags & MoveLogFlags::USED_ALL)) {
                        all_moves_used = false;
                        break;
                    }
                }
                if (all_moves_used) {
                    g_ConversationId = ConversationId::NPC_K_OPTION_UNLOCK;
                    SetSWF(GSWF_NpcK_CustomMovesUnlocked);
                    break;
                }
            }

            int32_t ids[10] = { ConversationId::NPC_K_BADGE_MOVES };
            int32_t num_cvs = 1;

            if (state.GetOption(STAT_PERM_MOVE_LOG, MoveType::JUMP_SPRING) 
                && MoveLogFlags::UNLOCKED_LV_1) {
                ids[num_cvs++] = ConversationId::NPC_K_SPRING_JUMP;
            }
            if (state.GetOption(STAT_PERM_MOVE_LOG, MoveType::HAMMER_ULTRA) 
                && MoveLogFlags::UNLOCKED_LV_1) {
                ids[num_cvs++] = ConversationId::NPC_K_ULTRA_HAMMER;
            }
            for (int32_t i = 1; i <= 7; ++i) {
                if (state.GetOption(STAT_PERM_PARTNERS_OBTAINED) & (1 << i)) {
                    ids[num_cvs++] = ConversationId::NPC_K_PARTNER_1 + i - 1;
                }
            }

            int32_t cv_id = coins_earned % num_cvs;
            g_ConversationId = ids[cv_id];
            break;
        }
        case ConversationId::NPC_KROOP: {
            if (completed_runs < 1) {
                g_ConversationId = ConversationId::NPC_KROOP_TUT_1;
                break;
            } else if (!GetSWF(GSWF_MayorKroop_PostTutorialChat)) {
                g_ConversationId = ConversationId::NPC_KROOP_TUT_2;
                SetSWF(GSWF_MayorKroop_PostTutorialChat);
                break;
            }
            
            int32_t num_cvs =
                ConversationId::NPC_KROOP_CVS_END -
                ConversationId::NPC_KROOP_CVS_START;
            int32_t cv_id = coins_earned % num_cvs;
            g_ConversationId = ConversationId::NPC_KROOP_CVS_START + cv_id;
            break;
        }
        case ConversationId::NPC_WHITE: {
            if (GetSWF(GSWF_White_CountdownUnlocked)) {
                // Use default conversation.
                break;
            }

            if (!GetSWF(GSWF_White_FirstTimeChat)) {
                SetSWF(GSWF_White_FirstTimeChat);
                g_ConversationId = ConversationId::NPC_WHITE_FIRST;
                break;
            }

            // Count number of speedrun achievements the player has finished.
            int32_t timer_achievements = 0;
            if (state.GetOption(
                FLAGS_ACHIEVEMENT, AchievementId::RUN_HALF_SPEED1))
                ++timer_achievements;
            if (state.GetOption(
                FLAGS_ACHIEVEMENT, AchievementId::RUN_HALF_SPEED2))
                ++timer_achievements;
            if (state.GetOption(
                FLAGS_ACHIEVEMENT, AchievementId::RUN_FULL_SPEED1))
                ++timer_achievements;
            if (state.GetOption(
                FLAGS_ACHIEVEMENT, AchievementId::RUN_FULL_SPEED2))
                ++timer_achievements;
            if (state.GetOption(
                FLAGS_ACHIEVEMENT, AchievementId::RUN_EX_SPEED1))
                ++timer_achievements;
            if (state.GetOption(
                FLAGS_ACHIEVEMENT, AchievementId::RUN_EX_SPEED2))
                ++timer_achievements;

            if (timer_achievements >= 4) {
                SetSWF(GSWF_White_CountdownUnlocked);
                g_ConversationId = ConversationId::NPC_WHITE_UNLOCK;
            } else {
                g_ConversationId =
                    ConversationId::NPC_WHITE_PROGRESS + timer_achievements;
            }

            break;
        }
        case ConversationId::NPC_GATEKEEPER: {
            if (completed_runs < 1 || !GetSWF(GSWF_Gatekeeper_FirstTimeChat)) {
                SetSWF(GSWF_Gatekeeper_FirstTimeChat);
                break;
            }
            g_ConversationId = ConversationId::NPC_GATEKEEPER_POST;
            break;
        }
        case ConversationId::HOOK_F1:
        case ConversationId::HOOK_ENTRY:
        case ConversationId::HOOK_DEATH: {
            // Swap for different conversation after first successful clear.
            if (GetSWF(GSWF_Hooktail_FirstTimeChat)) {
                g_ConversationId += 10;
            }
            break;
        }
        case ConversationId::GLOOM_F1:
        case ConversationId::GLOOM_ENTRY:
        case ConversationId::GLOOM_DEATH: {
            // Swap for different conversation after first successful clear.
            if (GetSWF(GSWF_Gloomtail_FirstTimeChat)) {
                g_ConversationId += 10;
            }
            break;
        }
        case ConversationId::BONE_ENTRY: {
            // Swap for different conversation after first successful clear.
            // (only the initial one has a partner reaction).
            if (GetSWF(GSWF_Bonetail_FirstTimeChat)) {
                g_ConversationId += 10;
            } else {
                g_ConversationPtr = kActorPartnerDialogue;
            }
            break;
        }
        case ConversationId::HOOK_BITE2: {
            // Dialogue is always just the partner.
            g_ConversationPtr = kPartnerConversation;
            break;
        }
        case ConversationId::BONE_LOW_HP: {
            // Dialogue is always Bonetail, then the partner reacting.
            g_ConversationPtr = kActorPartnerDialogue;
            break;
        }
        case ConversationId::GLOOM_MEGA: {
            if (GetSWF(GSWF_Gloomtail_FirstTimeChat)) {
                // Small random chance of an alternate Megabreath message.
                const int32_t num_cvs =
                    ConversationId::GLOOM_MEGA_END -
                    ConversationId::GLOOM_MEGA_START;
                if (int32_t cv_id = state.Rand(100); cv_id < num_cvs)
                    g_ConversationId = ConversationId::GLOOM_MEGA_START + cv_id;
            }
            break;
        }
        case ConversationId::BOSS_2_F:
        case ConversationId::BOSS_3_F: {
            // TODO: Adjust as necessary, add variants for FX encounters, etc.
            // Boss 2 and 3 cutscenes support up to four dialogue boxes.
            static int8_t AltBosses_CutsceneConversation[] =  { 0, 0, 0, 0, 0, -1 };
            g_ConversationPtr = AltBosses_CutsceneConversation;
            break;
        }
    }
}

bool DialogueManager::GetNextMessage(const char** msg, int32_t* speaker_type) {
    const char* party_str = nullptr;
    switch (ttyd::mario_party::marioGetParty()) {
        case 1: party_str = "_kur"; break;
        case 2: party_str = "_nok"; break;
        case 3: party_str = "_bom"; break;
        case 4: party_str = "_yos"; break;
        case 5: party_str = "_win"; break;
        case 6: party_str = "_viv"; break;
        case 7: party_str = "_chu"; break;
    }

    while (true) {
        // Check for conversation end.
        if (*g_ConversationPtr == SpeakerType::CONVERSATION_END) {
            *msg = nullptr;
            *speaker_type = SpeakerType::CONVERSATION_END;
            return false;
        }
        // Advance to first non-partner dialogue if no partner currently out.
        if (*g_ConversationPtr != SpeakerType::PARTNER || party_str) break;

        ++g_ConversationPtr;
        ++g_ConversationStep;
    }

    // Don't add party string to end unless partner is speaking, on the field.
    if (*g_ConversationPtr != SpeakerType::PARTNER || 
        ttyd::mariost::g_MarioSt->bInBattle) party_str = "";

    static char lookup_buf[24];
    sprintf(lookup_buf,
        "tot_di%06" PRId32 "_%02" PRId32 "%s",
        g_ConversationId, g_ConversationStep, party_str);

    // Set the output variables.
    *msg = lookup_buf;
    *speaker_type = *g_ConversationPtr;

    // Advance to the next step of the conversation.
    ++g_ConversationPtr;
    ++g_ConversationStep;

    return true;
}

EVT_DEFINE_USER_FUNC(evtTot_SetConversation) {
    DialogueManager::SetConversation(evtGetValue(evt, evt->evtArguments[0]));
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_GetNextMessage) {
    const char* msg;
    int32_t speaker_type;
    DialogueManager::GetNextMessage(&msg, &speaker_type);

    evtSetValue(evt, evt->evtArguments[0], PTR(msg));
    evtSetValue(evt, evt->evtArguments[1], speaker_type);

    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_HasConversationQueued) {
    evtSetValue(evt, evt->evtArguments[0], g_ConversationContinueId[0] != 0);
    return 2;
}

}  // namespace mod::tot