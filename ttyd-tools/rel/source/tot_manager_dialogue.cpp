#include "tot_manager_dialogue.h"

#include "evt_cmd.h"
#include "mod.h"
#include "tot_generate_enemy.h"
#include "tot_generate_reward.h"
#include "tot_gsw.h"
#include "tot_manager_achievements.h"
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
int32_t g_ConversationContinueId = 0;

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

    // Commonly-used sources of pseudo-random conversation choice.
    int32_t damage_dealt = state.GetOption(STAT_PERM_ENEMY_DAMAGE);
    int32_t coins_earned = state.GetOption(STAT_PERM_META_COINS_EARNED);

    // For some NPCs, override the default conversation type based on
    // pseudo-random factors like run stats, or other conditions.
    switch (id) {
        case ConversationId::NPC_A: {
            if (g_ConversationContinueId) {
                // Add predetermined ending to previous conversation.
                g_ConversationId = g_ConversationContinueId;
                g_ConversationContinueId = 0;
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
                            g_ConversationId = ConversationId::NPC_A_L_SECRET;
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
                            g_ConversationContinueId =
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

            int8_t ids[AchievementId::MAX_ACHIEVEMENT + 1];
            int32_t num_cvs = 0;
            int32_t num_achievements = 0;

            for (int32_t i = 0; i < 70; ++i) {
                if (grid[i] >= AchievementId::SECRET_COINS) continue;

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
                        case AchievementId::META_ALL_OPTIONS:
                        case AchievementId::META_ITEM_LOG_BASIC:
                            ids[num_cvs++] = grid[i];
                            break;
                        default:
                            break;
                    }
                }
            }

            if (num_achievements == AchievementId::SECRET_COINS) {
                g_ConversationId = ConversationId::NPC_D_ALL_DONE;
            } else if (num_cvs < 1) {
                g_ConversationId = ConversationId::NPC_D_NONE_ACTIVE;
            } else {
                int32_t cv_id = coins_earned % num_cvs;
                g_ConversationId = ConversationId::NPC_D_CVS_BASE + ids[cv_id];
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
            if (completed_runs < 1) break;
            
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
            } else if (g_ConversationContinueId) {
                // Add predetermined ending to previous conversation.
                g_ConversationId = g_ConversationContinueId;
                g_ConversationContinueId = 0;
                break;
            }

            NpcGStats stats[RewardStatId::MAX_REWARD_STAT];
            int32_t num_stats = 0;

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
                g_ConversationContinueId = ConversationId::NPC_G_ENDING_NONE;
            } else if (stats[index].reward_rate_taken > 7500 || num_stats - index <= 3) {
                // If taken > 75% of the time, or in top three types taken:
                g_ConversationContinueId = ConversationId::NPC_G_ENDING_HIGH;
            } else if (stats[index].reward_rate_taken < 2000 || index < 8) {
                // If taken < 20% of the time, or in bottom 40% of types:
                g_ConversationContinueId = ConversationId::NPC_G_ENDING_LOW;
            } else {
                g_ConversationContinueId = ConversationId::NPC_G_ENDING_MED;
            }

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
            if (state.GetOption(STAT_PERM_HALF_FINISHES) >= 1) {
                g_ConversationId += 10;
            }
            break;
        }
        case ConversationId::GLOOM_F1:
        case ConversationId::GLOOM_ENTRY:
        case ConversationId::GLOOM_DEATH: {
            // Swap for different conversation after first successful clear.
            if (state.GetOption(STAT_PERM_FULL_FINISHES) >= 1) {
                g_ConversationId += 10;
            }
            break;
        }
        case ConversationId::BONE_ENTRY: {
            // Swap for different conversation after first successful clear.
            // (only the initial one has a partner reaction).
            if (state.GetOption(STAT_PERM_EX_FINISHES) >= 1) {
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
            if (state.GetOption(STAT_PERM_FULL_FINISHES) >= 1) {
                // Small random chance of an alternate Megabreath message.
                const int32_t num_cvs =
                    ConversationId::GLOOM_MEGA_END -
                    ConversationId::GLOOM_MEGA_START;
                if (int32_t cv_id = state.Rand(100); cv_id < num_cvs)
                    g_ConversationId = ConversationId::GLOOM_MEGA_START + cv_id;
            }
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
    evtSetValue(evt, evt->evtArguments[0], g_ConversationContinueId != 0);
    return 2;
}

}  // namespace mod::tot