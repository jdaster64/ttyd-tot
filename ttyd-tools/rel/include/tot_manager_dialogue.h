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

        // NPC B (Toad)
        NPC_B               = 2000,
        NPC_B_CVS_START     = 2100,
        NPC_B_CVS_END       = NPC_B_CVS_START + 10,

        // NPC C (Gamer Toad)
        NPC_C               = 3000,
        NPC_C_CVS_START     = 3100,
        NPC_C_CVS_END       = NPC_C_CVS_START + 6,
        NPC_C_CVS_SPECIAL   = 3200,

        // Innkeeper
        NPC_INN             = 4000,
        NPC_INN_CVS_START   = 4100,
        NPC_INN_CVS_END     = NPC_INN_CVS_START + 5,

        // NPC H (Koopa)
        NPC_H               = 5000,
        NPC_H_CVS_START     = 5100,
        NPC_H_CVS_END       = NPC_H_CVS_START + 7,

        // NPC I (Toad)
        NPC_I               = 6000,
        NPC_I_NO_CLEAR      = 6100,
        NPC_I_FIRST_CLEAR   = 6101,
        NPC_I_SECOND_CLEAR  = 6102,
        NPC_I_CVS_START     = 6200,
        NPC_I_CVS_END       = NPC_I_CVS_START + 6,
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