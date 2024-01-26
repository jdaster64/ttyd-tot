#pragma once

#include "common_types.h"

#include <cstdint>

namespace mod::infinite_pit::field_start {

// Apply patches to field-related events in the starting room outside the Pit.
void ApplyFixedPatches();

// Apply patches to module code.
void ApplyModuleLevelPatches(void* module_ptr, ModuleId::e module_id);

}