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

        // NPC B (Toad on west side)
        NPC_B               = 2000,
        NPC_B_CVS_START     = 2100,
        NPC_B_CVS_END       = NPC_B_CVS_START + 10,

        // NPC C (Little Toad)
        NPC_C               = 3000,
        NPC_C_CVS_START     = 3100,
        NPC_C_CVS_END       = NPC_C_CVS_START + 6,
        NPC_C_CVS_SPECIAL   = 3200,

        // Innkeeper
        NPC_INN             = 4000,
        NPC_INN_CVS_START   = 4100,
        NPC_INN_CVS_END     = NPC_INN_CVS_START + 5,

        // NPC H (Koopa on east side)
        NPC_H               = 5000,
        NPC_H_CVS_START     = 5100,
        NPC_H_CVS_END       = NPC_H_CVS_START + 7,

        // NPC I (Toad on east side)
        NPC_I               = 6000,
        NPC_I_NO_CLEAR      = 6100,
        NPC_I_FIRST_CLEAR   = 6101,
        NPC_I_SECOND_CLEAR  = 6102,
        NPC_I_CVS_START     = 6200,
        NPC_I_CVS_END       = NPC_I_CVS_START + 6,

        // NPC K (Toad in far east house)
        NPC_K               = 7000,
        NPC_K_SPRING_JUMP   = 7100,
        NPC_K_ULTRA_HAMMER,
        NPC_K_BADGE_MOVES,
        NPC_K_PARTNER_1,
        NPC_K_PARTNER_2,
        NPC_K_PARTNER_3,
        NPC_K_PARTNER_4,
        NPC_K_PARTNER_5,
        NPC_K_PARTNER_6,
        NPC_K_PARTNER_7,

        // Mayor Kroop
        NPC_KROOP           = 8000,
        NPC_KROOP_TUT_1     = 8100,
        NPC_KROOP_TUT_2     = 8101,
        NPC_KROOP_CVS_START = 8200,
        NPC_KROOP_CVS_END   = NPC_KROOP_CVS_START + 4,

        // NPC D (Koopa in house east of shop)
        NPC_D               = 9000,
        NPC_D_CVS_BASE      = 9100,
        NPC_D_NONE_ACTIVE   = 9300,
        NPC_D_ALL_DONE      = 9301,

        // NPC - East Petalburg gatekeeper
        NPC_GATEKEEPER      = 12000,
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