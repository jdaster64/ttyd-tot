#pragma once

#include "evt_cmd.h"

#include <ttyd/evtmgr.h>

#include <cstdint>

namespace mod::tot::gon {

// Returns setup structs for NPCs.
EVT_DECLARE_USER_FUNC(evtTot_GetCharlietonNpcSetup, 1)

}