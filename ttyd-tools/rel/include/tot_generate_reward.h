#pragma once

#include "evt_cmd.h"

#include <ttyd/evtmgr.h>

#include <cstdint>

namespace mod::tot {

// Generates chest contents.
EVT_DECLARE_USER_FUNC(evtTot_GenerateChestContents, 0)

// Returns information about a chest.
// arg0     = (in) index
// arg1~3   = position
// arg4     = item
// arg5     = item pickup callback script
EVT_DECLARE_USER_FUNC(evtTot_GetChestData, 6)

// Draws icons above chests.
EVT_DECLARE_USER_FUNC(evtTot_DisplayChestIcons, 0)

}