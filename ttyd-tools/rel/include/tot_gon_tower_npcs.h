#pragma once

#include "evt_cmd.h"

#include <ttyd/evtmgr.h>

#include <cstdint>

namespace mod::tot::gon {

// Returns setup info for NPCs.
// out arg0 - NPC name, arg1 - tribe name, arg2 - NpcSetupInfo
EVT_DECLARE_USER_FUNC(evtTot_GetCharlietonNpcParams, 3)

}