#pragma once

#include "evt_cmd.h"

#include <ttyd/evtmgr.h>

#include <cstdint>

namespace ttyd::battle_unit {
struct BattleWorkUnit;
}

namespace mod::tot {

// Sets up battle loadout information for a given floor and returns properties
// for the corresponding enemy NPC:
//
// arg0 (inout) = BattleSetupData* database
// arg1 (inout) = NpcSetupInfo* npc
// arg2 = npc_tribe_description->modelName
// arg3 = npc_tribe_description->nameJp
// arg4~6 = Starting spawn position (x, y, z).
EVT_DECLARE_USER_FUNC(evtTot_GetEnemyNpcInfo, 7)

// Sets finalized battle info on the NPC, as well as any setting up any
// held items, battle conditions, etc.
//
// arg0 = npc name
// arg1 = battle id
EVT_DECLARE_USER_FUNC(evtTot_SetEnemyNpcBattleInfo, 2)

// Clears enemy loadout information.
EVT_DECLARE_USER_FUNC(evtTot_ClearEnemyInfo, 0)

// Selects the midboss that will be used for a given floor.
void SelectMidboss(int32_t floor, bool reroll);
// Selects the type of minion a midboss spawns.
int32_t SelectMidbossMinion(int32_t unit_type);

// Gets replacement stats for an enemy, based on the enemy type and current
// floor (determined by the mod's state).
// Will return false if no stats were found for the given enemy type.
// If an out pointer is passed as nullptr, that stat will be skipped.
// If out_level returns a negative number, that should be used as bonus EXP.
// Should not be called for ATK/DEF if replacing a vanilla ATK/DEF of 0.
bool GetEnemyStats(
    int32_t unit_type, int32_t* out_hp, int32_t* out_atk, int32_t* out_def, 
    int32_t* out_level, int32_t* out_coinlvl, int32_t base_attack_power = 0);
    
// EVT wrapper for above function (args in same order).
EVT_DECLARE_USER_FUNC(evtTot_GetEnemyStats, 7)

// Returns how many chests to offer based on the battle's relative difficulty.
int32_t GetBattleRewardTier();

// Returns the attack script for a midboss, based on its original script.
void* GetMidbossAttackScript(void* original_script);
    
// Gets/sets a custom Tattle message based on the enemy's parameters.
// In-battle:
const char* GetCustomTattle();
const char* SetCustomTattle(
    ttyd::battle_unit::BattleWorkUnit* unit, const char* original_tattle_msg);
const char* SetCustomMenuTattle(int32_t unit_type);
// Returns a custom ordering for Tattles that only considers enemies in the Pit.
int8_t GetCustomTattleIndex(int32_t unit_type);
// Returns the custom attack and defense "stat" for an enemy type, as used by
// the in-battle Tattle display. Returns false if no stats exist.
bool GetTattleDisplayStats(int32_t unit_type, int32_t* atk, int32_t* def);

// Returns whether a battle unit type can be a midboss.
bool IsEligibleMidboss(int32_t unit_type);
// Returns whether a battle unit type can be the first enemy in a loadout.
bool IsEligibleFrontEnemy(int32_t unit_type);
// Returns whether a battle unit type can be a back enemy in a loadout.
bool IsEligibleBackEnemy(int32_t unit_type);

}