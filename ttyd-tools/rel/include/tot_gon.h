#pragma once

#include "evt_cmd.h"

#include <ttyd/evtmgr.h>

#include <cstdint>

namespace ttyd::battle_database_common {
struct BattleSetupData;
}

namespace mod::tot::gon {

// Runs prolog info for the custom area maps / events.
void Prolog();

// Returns a pointer to the battle setups, to be mutated by tower scripts.
EVT_DECLARE_USER_FUNC(evtTot_GetGonBattleDatabasePtr, 1)

}