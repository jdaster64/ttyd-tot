#pragma once

#include "evt_cmd.h"

#include <ttyd/evtmgr.h>

#include <cstdint>

namespace mod::tot::gon {

namespace SecondaryNpcType {
    enum e {
        NONE = -1,
        WONKY = 0,
        DAZZLE,
        CHET_RIPPO,
        LUMPY,
        DOOPLISS,
        GRUBBA,
        MOVER,
        ZESS_T,
        MERLON,
        
        NUM_NPC_TYPES,
        // Used for interpreting run options.
        CHOICE_RANDOM   = NUM_NPC_TYPES,
        CHOICE_NONE     = NUM_NPC_TYPES + 1,
    };
}

void GetNpcMsgs(int32_t type, const char** out_name, const char** out_help);

// Returns setup info for NPCs.
// out arg0 - NPC name, arg1 - tribe name, arg2 - model (Charlieton),
// out arg3 - NPC name, arg4 - tribe name, arg5 - model (secondary),
// out arg6 - NpcSetupInfo
EVT_DECLARE_USER_FUNC(evtTot_GetTowerNpcParams, 7)

// Selects the secondary NPCs that will show up throughout the run.
EVT_DECLARE_USER_FUNC(evtTot_SelectSecondaryNpcs, 0)

}