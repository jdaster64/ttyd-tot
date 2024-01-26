#pragma once

#include "common_types.h"

#include <cstdint>

namespace mod::infinite_pit::field {

// Apply patches to field-related events in the Pit.
void ApplyFixedPatches();

// Apply patches to module code.
void ApplyModuleLevelPatches(void* module_ptr, ModuleId::e module_id);

// Links / unlinks custom events that rely on code in TTYD's modules.
// Unlinks if `link` = false.
void LinkCustomEvts(void* module_ptr, ModuleId::e module_id, bool link);

// Updates the destination of the current floor's exit pipe to be consistent
// with a change in floor number.
void UpdateExitDestination();

}