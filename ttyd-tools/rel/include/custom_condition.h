#pragma once

#include <ttyd/npcdrv.h>

#include <cstdint>

namespace mod::infinite_pit {
    
// Randomly sets parameters for a battle condition that grants a bonus item.
void SetBattleCondition(ttyd::npcdrv::NpcBattleInfo* npc_info, bool enable = true);

// Returns a string based on the current battle condition, if any.
void GetBattleConditionString(char* out_buf);

}