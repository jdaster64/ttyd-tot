#pragma once

#include <ttyd/battle_unit.h>

#include <cstdint>

namespace mod::infinite_pit::item {

// Apply patches related to item balance, etc.
void ApplyFixedPatches();

// Replaces the vanilla logic for evasion badges; returns true if Lucky.
bool CheckEvasionBadges(ttyd::battle_unit::BattleWorkUnit* unit);

// Returns how much extra HP or FP to restore when using Strawberry Cake,
// above the base 5.
int32_t GetBonusCakeRestoration();

}