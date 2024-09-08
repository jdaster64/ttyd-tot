#include "tot_manager_dialogue.h"

#include "evt_cmd.h"

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
    g_ConversationId = id;
    g_ConversationPtr = kDefaultConversation;

    // TODO: For some NPCs, pick from different conversations based on
    // randomness, or certain conditions.
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

    static char lookup_buf[16];
    sprintf(lookup_buf, "tot_di%4" PRId32 "%s", g_ConversationStep, party_str);

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