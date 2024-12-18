#pragma once

#include <cstdint>

namespace mod::tot::patch::core {

// Apply patches to core game systems, such as save file I/O, file and module
// loading, and string message lookup.
void ApplyFixedPatches();

// Replaces the existing logic for loading a map.
// Returns 1 if the map is not finished loading, and 2 if it is.
int32_t LoadMap();
// Code that runs immediately before unloading a map.
void OnMapUnloaded();

}