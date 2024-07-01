#pragma once

#include <cstdint>

namespace mod::infinite_pit::mario_move {

// Apply patches related to Mario's moves and move selection in battle.
void ApplyFixedPatches();

// Initializes Mario's moves' selected power level when entering/exiting battle.
void OnEnterExitBattle(bool is_start);
// Gets the current move level of Charge or Toughen Up for Mario or partner.
int8_t GetStrategyBadgeLevel(bool is_charge, bool is_mario);

// Sets up how many targets appear of each type for Sweet Treat / Feast.
void SweetTreatSetUpTargets();
// Replaces the logic for numbers displaying / blinking for Sweet Treat / Feast.
void SweetTreatBlinkNumbers();
// Returns how many action command bars should appear for Earth Tremor.
int32_t GetEarthTremorNumberOfBars();
// Returns the attack power for Art Attack given the circled percentage.
int32_t GetArtAttackPower(int32_t circled_percent);

}