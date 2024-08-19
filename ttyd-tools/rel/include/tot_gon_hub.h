#pragma once

#include <cstdint>

namespace mod::tot::gon {

// Returns a pointer to the init event for the west-side lobby map (gon_10).
const int32_t* GetWestSideInitEvt();

// Returns a pointer to the init event for the east-side lobby map (gon_11).
const int32_t* GetEastSideInitEvt();

}