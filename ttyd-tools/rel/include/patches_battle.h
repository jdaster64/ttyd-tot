#pragma once

#include "evt_cmd.h"

#include <ttyd/evtmgr.h>

#include <cstdint>


namespace ttyd::battle_database_common {

struct BattleWeapon;

}
namespace ttyd::battle_unit {

struct BattleWorkUnit;
struct BattleWorkUnitPart;

}

namespace mod::infinite_pit::battle {

// Apply patches to various battle features.
void ApplyFixedPatches();

// Overrides the default target audience amount to be based on Pit progression.
void SetTargetAudienceAmount();
// Applies the option to change the SP amount regained from attacks.
double ApplySpRegenMultiplier(double base_regen);
// Toggles off "Scoped" status for the target if attacked by the player.
void ToggleScopedStatus(
    ttyd::battle_unit::BattleWorkUnit* attacker, 
    ttyd::battle_unit::BattleWorkUnit* target,
    ttyd::battle_database_common::BattleWeapon* weapon);

// Calculates the base damage for an attack, replacing the original TTYD func.
int32_t CalculateBaseDamage(
    ttyd::battle_unit::BattleWorkUnit* attacker, 
    ttyd::battle_unit::BattleWorkUnit* target, 
    ttyd::battle_unit::BattleWorkUnitPart* part,
    ttyd::battle_database_common::BattleWeapon* weapon, 
    uint32_t* unk0, uint32_t unk1);

// Applies a custom status effect to the target.
// Params: unit, part, status_flag, color1 & color2 (rgb), sfx, announce_msg
EVT_DECLARE_USER_FUNC(evtTot_ApplyCustomStatus, 7)

}