#pragma once

#include "evt_cmd.h"

#include <ttyd/evtmgr.h>

#include <cstdint>

namespace mod::tot::gon {

// Returns setup info for NPCs.
// out arg0 - NPC name, arg1 - tribe name, arg2 - model (Charlieton),
// out arg3 - NPC name, arg4 - tribe name, arg5 - model (secondary),
// out arg6 - NpcSetupInfo
EVT_DECLARE_USER_FUNC(evtTot_GetTowerNpcParams, 7)

// Selects the secondary NPCs that will show up throughout the run.
EVT_DECLARE_USER_FUNC(evtTot_SelectSecondaryNpcs, 0)

}