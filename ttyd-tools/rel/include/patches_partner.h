#pragma once

#include "evt_cmd.h"

#include <gc/types.h>
#include <ttyd/evtmgr.h>

#include <cstdint>

namespace ttyd::battle_unit {
struct BattleWorkUnit;
}

namespace mod::infinite_pit::partner {

// Apply patches related to partner balance changes, etc.
void ApplyFixedPatches();

// Displays the HP stat for an enemy, and extra stats under some circumstances.
void DisplayTattleStats(
    gc::mtx34* matrix, int32_t number, int32_t is_small, uint32_t* color,
    ttyd::battle_unit::BattleWorkUnit* unit);
    
// Forces ATK/DEF to be shown at the start of an encounter.
void RefreshExtraTattleStats();

// Initializes + fully heals the selected party member.
EVT_DECLARE_USER_FUNC(InitializePartyMember, 1)
// Calculates the Action Command prize tier of Love Slap based on AC result.
EVT_DECLARE_USER_FUNC(GetLoveSlapPrizeTier, 1)

}