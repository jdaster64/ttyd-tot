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

// Announces Star Power restoration and resets facing direction simultaneously;
// used to fix issues with moves in vanilla that had Stylishes placed after
// the move was "completed" for SP regeneration purposes.
EVT_DECLARE_USER_FUNC(AwardStarPowerAndResetFaceDirection, 1)

}