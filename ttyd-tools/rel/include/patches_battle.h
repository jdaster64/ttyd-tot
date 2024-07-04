#pragma once

#include "evt_cmd.h"

#include <ttyd/evtmgr.h>

#include <cstdint>


namespace ttyd::battle_damage {

struct CounterattackWork;

}
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

// Calculates the damage dealt by a counterattack.
void CalculateCounterDamage(
    ttyd::battle_damage::CounterattackWork* counter_work,
    ttyd::battle_unit::BattleWorkUnit* attacker,
    ttyd::battle_unit::BattleWorkUnit* target,
    ttyd::battle_unit::BattleWorkUnitPart* part,
    int32_t damage_dealt);

// Toggles off "Scoped" status for the target if attacked by the player.
void ToggleScopedStatus(
    ttyd::battle_unit::BattleWorkUnit* attacker, 
    ttyd::battle_unit::BattleWorkUnit* target,
    ttyd::battle_database_common::BattleWeapon* weapon);

// Queues a custom status announcement message.
void QueueCustomStatusMessage(
    ttyd::battle_unit::BattleWorkUnit* unit, const char* announce_msg);

// Gets the party switch cost in FP (0, or 1+ with Quick Change).
int32_t GetPartySwitchCost();
// Resets the party switch cost at the start of an encounter.
void ResetPartySwitchCost();
// Signals that a party switch should cost FP unless cancelled by Confusion.
void SignalPlayerInitiatedPartySwitch();

// Calculates an enemy's current ATK, including statuses.
// Uses the default reference ATK value if `reference_atk` < 0.
int32_t GetCurrentEnemyAtk(
    ttyd::battle_unit::BattleWorkUnit* unit, int32_t reference_atk = -1);
// Calculates an enemy's current DEF (to normal attacks), including statuses.
int32_t GetCurrentEnemyDef(
    ttyd::battle_unit::BattleWorkUnit* unit);

// For "select a side" type attacks.
// Handles user input to swap which side they are currently targeting.
void HandleSideSelection();
// Checks whether the currently selected actor is on the currently targeted
// side (for displaying cursors before selecting the move).
bool CheckOnSelectedSide(int32_t target_idx);

// Applies a custom status effect to the target.
// Params: unit, part, status_flag, color1 & color2 (rgb), sfx, announce_msg
EVT_DECLARE_USER_FUNC(evtTot_ApplyCustomStatus, 7)

}