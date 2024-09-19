#pragma once

#include "evt_cmd.h"

#include <ttyd/evtmgr.h>

#include <cstdint>

namespace mod::tot {

namespace ConversationId {
    enum e {
        INVALID = 0,

        // NPC F (Pink Bub-ulb)
        BUBULB_P            = 1000,
        BUBULB_P_CVS_START  = 1100,
        BUBULB_P_CVS_END    = BUBULB_P_CVS_START + 10,  // TBD; can't exceed 30.

        // NPC C (Gamer Toad)
        NPC_C               = 3000,
        NPC_C_CVS_START     = 3100,
        NPC_C_CVS_END       = NPC_C_CVS_START + 6,
        NPC_C_CVS_SPECIAL   = 3200,
    };
}

namespace SpeakerType {
    enum e {
        SPEAKER = 0,
        PARTNER,
        SYSTEM,

        CONVERSATION_END = -1,
    };
}

class DialogueManager {
public:
    static void SetConversation(int32_t id);

    static bool GetNextMessage(const char** msg, int32_t* speaker_type);
};

// Wrapper to SetConversation.
EVT_DECLARE_USER_FUNC(evtTot_SetConversation, 1)

// Wrapper to GetNextMessage.
EVT_DECLARE_USER_FUNC(evtTot_GetNextMessage, 2)
 
}