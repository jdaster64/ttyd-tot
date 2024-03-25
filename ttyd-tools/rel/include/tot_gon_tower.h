#pragma once

#include <ttyd/evtmgr.h>

#include <cstdint>

namespace mod::tot::gon {

// Returns a pointer to the init event for the standard tower map (gon_01).
const int32_t* GetTowerInitEvt();

// Updates the map the exit of the current room leads to based on the floor number.
void UpdateDestinationMap();

}