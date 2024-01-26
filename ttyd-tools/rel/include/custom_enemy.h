#pragma once

#include "common_types.h"

#include <ttyd/battle_unit.h>
#include <ttyd/npcdrv.h>

#include <cstdint>

namespace mod::infinite_pit {

// Selects the loadout of enemies to spawn on a given floor.
void SelectEnemies(int32_t floor);
    
// Procedurally builds a NpcSetupInfo and BattleSetupData based on the floor #.
// Returns the salient NPC info (the battle is constructed in place over
// the battle that would normally be loaded), as well as the lead enemy's type.
void BuildBattle(
    uintptr_t pit_module_ptr, int32_t floor,
    ttyd::npcdrv::NpcTribeDescription** out_npc_tribe_description,
    ttyd::npcdrv::NpcSetupInfo** out_npc_setup_info, int32_t* out_lead_type);
    
// Gets replacement stats for an enemy, based on the enemy type and current
// floor (determined by the mod's state).
// Will return false if no stats were found for the given enemy type.
// If an out pointer is passed as nullptr, that stat will be skipped.
// If out_level returns a negative number, that should be used as bonus EXP.
// Should not be called for ATK/DEF if replacing a vanilla ATK/DEF of 0.
bool GetEnemyStats(
    int32_t unit_type, int32_t* out_hp, int32_t* out_atk, int32_t* out_def, 
    int32_t* out_level, int32_t* out_coinlvl, int32_t base_attack_power = 0);
    
// Gets/sets a custom Tattle message based on the enemy's parameters.
// In-battle:
const char* GetCustomTattle();
const char* SetCustomTattle(
    ttyd::battle_unit::BattleWorkUnit* unit, const char* original_tattle_msg);
const char* SetCustomMenuTattle(const char* original_tattle_msg);
// Returns a custom ordering for Tattles that only considers enemies in the Pit.
int8_t GetCustomTattleIndex(int32_t unit_type);
// Returns the custom attack and defense "stat" for an enemy type, as used by
// the in-battle Tattle display. Returns false if no stats exist.
bool GetTattleDisplayStats(int32_t unit_type, int32_t* atk, int32_t* def);

// Used for debugging purposes:
// Returns whether a battle unit type can included in a starting loadout.
bool IsEligibleLoadoutEnemy(int32_t unit_type);
// Returns whether a battle unit type can be the first enemy in a loadout.
bool IsEligibleFrontEnemy(int32_t unit_type);

}