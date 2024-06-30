#pragma once

#include "evt_cmd.h"

#include <ttyd/battle_unit.h>
#include <ttyd/evtmgr.h>

#include <cstdint>

namespace mod::infinite_pit::item {

// Apply patches related to item balance, etc.
void ApplyFixedPatches();

// Replaces the vanilla logic for evasion badges; returns true if Lucky.
bool CheckEvasionBadges(ttyd::battle_unit::BattleWorkUnit* unit);

// Returns how much extra HP or FP to restore when using Strawberry Cake,
// above the base 5.
int32_t GetBonusCakeRestoration();

// Gets the current size of the player's item inventory.
int32_t GetItemInventorySize();

// Toggles whether or not to stop field item drops from despawning.
EVT_DECLARE_USER_FUNC(evtTot_FreezeFieldItemTimers, 1)

}