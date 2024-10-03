#pragma once

#include "evt_cmd.h"

#include <ttyd/evtmgr.h>

#include <cstdint>

namespace mod::tot {

namespace ConversationId {
    enum e {
        INVALID = 0,

        // NPC F (Pink Bub-ulb)
        NPC_F               = 1000,
        NPC_F_CVS_START     = 1100,
        NPC_F_CVS_END       = NPC_F_CVS_START + 10,  // TBD; can't exceed 30.

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

        // NPC G (Blue Bub-ulb)
        NPC_G               = 10000,
        NPC_G_NO_CLEAR      = 10100,
        NPC_G_FIRST_CLEAR   = 10101,
        NPC_G_REWARD_START  = 10200,
        NPC_G_REWARD_END    = NPC_G_REWARD_START + 20,
        NPC_G_ENDING_NONE   = 10300,
        NPC_G_ENDING_LOW,
        NPC_G_ENDING_MED,
        NPC_G_ENDING_HIGH,

        // NPC A - West Petalburg Koopa.
        NPC_A               = 11000,
        NPC_A_FIRST_VISIT   = 11100,
        NPC_A_FIRST_CLEAR   = 11200,
        NPC_A_SECOND_CLEAR  = 11300,
        NPC_A_L             = 11400,
        NPC_A_L_HOOK,
        NPC_A_L_GLOOM,
        NPC_A_L_BONE,
        NPC_A_L_SECRET,
        NPC_A_L_BOMB,
        NPC_A_L_ENEMY_START = 11410,
        NPC_A_L_ENEMY_END   = NPC_A_L_ENEMY_START + 3,
        NPC_A_L_RESP_START  = 11420,
        NPC_A_L_RESP_END    = NPC_A_L_RESP_START + 6,
        NPC_A_W             = 11430,
        NPC_A_W_EXDIFF,
        NPC_A_W_TIME,
        NPC_A_W_INTENSITY,

        // NPC - East Petalburg gatekeeper
        NPC_GATEKEEPER      = 12000,
        NPC_GATEKEEPER_POST = 12001,

        // Boss battles.

        // Hooktail
        HOOK            = 30000,
        HOOK_F1         = 30100,
            HOOK_F1_START   = 30110,
            HOOK_F1_END     = HOOK_F1_START + 1,
        HOOK_F2         = 30200,
        HOOK_F3         = 30300,
        HOOK_ENTRY      = 31000,
        HOOK_LOW_HP     = 31100,
        HOOK_P2         = 31200,
        HOOK_P3         = 31300,
        HOOK_MEGA       = 31400,
        HOOK_HEAL       = 31500,
        HOOK_BITE1      = 31600,
        HOOK_BITE2      = 31700,
        HOOK_DEATH      = 31800,
        HOOK_FAKEOUT    = 31900,
        // Gloomtail
        GLOOM               = 40000,
        GLOOM_F1            = 40100,
            GLOOM_F1_START      = 40110,
            GLOOM_F1_END        = GLOOM_F1_START + 1,
        GLOOM_F2            = 40200,
        GLOOM_F3            = 40300,
        GLOOM_ENTRY         = 41000,
        GLOOM_LOW_HP        = 41100,
        GLOOM_P2            = 41200,
        GLOOM_P3            = 41300,
        GLOOM_MEGA          = 41400,
        GLOOM_HEAL          = 41500,
        GLOOM_BITE1         = 41600,
        GLOOM_BITE2         = 41700,
        GLOOM_DEATH         = 41800,
        GLOOM_FAKEOUT       = 41900,
        // Bonetail
        BONE                = 50000,
        BONE_F1             = 50100,
        BONE_F2             = 50200,
        BONE_F3             = 50300,
        BONE_ENTRY          = 51000,
        BONE_LOW_HP         = 51100,
        BONE_P2             = 51200,
        BONE_P3             = 51300,
        BONE_MEGA           = 51400,
        BONE_HEAL           = 51500,
        BONE_BITE1          = 51600,
        BONE_BITE2          = 51700,
        BONE_DEATH          = 51800,
        BONE_FAKEOUT        = 51900,
        // Extra
        SBOSS               = 60000,
        SBOSS_F1            = 60100,
        SBOSS_F2            = 60200,
        SBOSS_F3            = 60300,
        SBOSS_PHASE2        = 61000,
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

// Returns whether or not there is another conversation queued for afterward.
EVT_DECLARE_USER_FUNC(evtTot_HasConversationQueued, 1)
 
}