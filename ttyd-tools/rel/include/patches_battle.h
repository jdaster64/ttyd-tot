#pragma once

#include "evt_cmd.h"

#include <ttyd/evtmgr.h>

#include <cstdint>

namespace mod::infinite_pit::battle {

// Apply patches to various battle features.
void ApplyFixedPatches();

// Overrides the default target audience amount to be based on Pit progression.
void SetTargetAudienceAmount();
// Applies the option to change the SP amount regained from attacks.
double ApplySpRegenMultiplier(double base_regen);

}