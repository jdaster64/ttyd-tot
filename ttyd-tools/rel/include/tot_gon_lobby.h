#pragma once

#include <cstdint>

namespace mod::tot::gon {

// Returns a pointer to the init event for the lobby map (gon_00).
const int32_t* GetLobbyInitEvt();

}