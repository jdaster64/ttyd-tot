#pragma once

#include "common_types.h"

#include <cstdint>

namespace mod::infinite_pit::enemy_fix {

// Apply patches for various fixes to issues with enemies in battle.
void ApplyFixedPatches();

// Apply patches to module code.
void ApplyModuleLevelPatches(void* module_ptr, ModuleId::e module_id);

// Links / unlinks custom events that rely on code in TTYD's modules.
// Unlinks if `link` = false.
void LinkCustomEvts(void* module_ptr, ModuleId::e module_id, bool link);

// Sums attack target weights for attacks that target randomly,
// ensuring that individual target weights are always > 0.
int32_t SumWeaponTargetRandomWeights(int32_t* weights);

}