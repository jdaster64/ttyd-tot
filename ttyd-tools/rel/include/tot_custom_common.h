#pragma once

#include "evt_cmd.h"

#include <gc/types.h>
#include <ttyd/evtmgr.h>

#include <cstdint>

namespace mod::tot::custom {

// Gets the number of enemy-type actors remaining (used for spawning clones).
// If a midboss is present in the fight, will always return at least 2.
EVT_DECLARE_USER_FUNC(evtTot_CheckNumEnemiesRemaining, 1);

// Selects unit + part targets for multi-hit 'barrage' moves.
// Used for Hammer Bro, Fire Bro, Piders' and Yuxes' scripts.
EVT_DECLARE_USER_FUNC(evtTot_SelectMultihitTargetsX2, 4);
EVT_DECLARE_USER_FUNC(evtTot_SelectMultihitTargetsX3, 6);
EVT_DECLARE_USER_FUNC(evtTot_SelectMultihitTargetsX4, 8);
EVT_DECLARE_USER_FUNC(evtTot_SelectMultihitTargetsX5, 10);
EVT_DECLARE_USER_FUNC(evtTot_SelectMultihitTargetsX6, 12);

// Returns the percentage of max HP a battle unit currently has.
EVT_DECLARE_USER_FUNC(evtTot_GetPercentOfMaxHP, 2)

// Returns whether a battle unit is of an enemy species, regardless of alliance.
EVT_DECLARE_USER_FUNC(evtTot_CheckSpeciesIsEnemy, 2)

}