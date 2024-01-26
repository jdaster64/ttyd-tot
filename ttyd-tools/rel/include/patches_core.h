#pragma once

#include <cstdint>

namespace mod::infinite_pit::core {

// Apply patches to core game systems, such as save file I/O, file and module
// loading, and string message lookup.
void ApplyFixedPatches();

// Replaces the existing logic for loading a map.
// Returns 1 if the map is not finished loading, and 2 if it is.
int32_t LoadMap();
// Code that runs immediately before unloading a map.
void OnMapUnloaded();

// Returns a pointer to the currently loaded Pit module, if it is loaded.
uintptr_t GetPitModulePtr();
// Gets/sets whether or not the player should currently be prompted to save.
bool GetShouldPromptSave();
void SetShouldPromptSave(bool should_save);

}