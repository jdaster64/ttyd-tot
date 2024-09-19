#include "tot_manager_dialogue.h"

#include "evt_cmd.h"
#include "mod.h"
#include "tot_gsw.h"
#include "tot_state.h"

#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/mario_party.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::tot {

namespace {

using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

// Default conversation type: single text box from a speaker.
int8_t kDefaultConversation[] = { 0, -1 };

int32_t g_ConversationId = 0;
int8_t* g_ConversationPtr = nullptr;
int32_t g_ConversationStep = 0;

}  // namespace

void DialogueManager::SetConversation(int32_t id) {
    const auto& state = g_Mod->state_;

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
        case ConversationId::BUBULB_P: {
            int32_t num_cvs =
                ConversationId::BUBULB_P_CVS_END -
                ConversationId::BUBULB_P_CVS_START;
            int32_t cv_id = damage_dealt % num_cvs;
            g_ConversationId = ConversationId::BUBULB_P_CVS_START + cv_id;

            SetSWByte(GSW_Hub_BubulbP_CurrentConversation, cv_id);
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
        case ConversationId::NPC_INN: {
            if (completed_runs < 1) break;
            
            int32_t num_cvs =
                ConversationId::NPC_INN_CVS_END -
                ConversationId::NPC_INN_CVS_START;
            int32_t cv_id = damage_dealt % num_cvs;
            g_ConversationId = ConversationId::NPC_INN_CVS_START + cv_id;
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

}  // namespace mod::tot