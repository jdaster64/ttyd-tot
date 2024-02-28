#pragma once

#include "evt_cmd.h"

#include <ttyd/evtmgr.h>

#include <cstdint>

namespace mod::tot {
    
// Sets up battle loadout information for a given floor and returns properties
// for the corresponding enemy NPC:
//
// arg0 (inout) = BattleSetupData* database
// arg1 (inout) = NpcSetupInfo* npc
// arg2 = npc_tribe_description->modelName
// arg3 = npc_tribe_description->nameJp
// arg4~6 = Starting spawn position (x, y, z).
EVT_DECLARE_USER_FUNC(evtTot_GetEnemyNpcInfo, 7)

// Sets finalized battle info on the NPC, as well as any setting up any
// held items, battle conditions, etc.
//
// arg0 = npc name
// arg1 = battle id
EVT_DECLARE_USER_FUNC(evtTot_SetEnemyNpcBattleInfo, 2)

}