#pragma once

#include <ttyd/battle_database_common.h>
#include <ttyd/battle_unit.h>

#include <cstdint>

namespace mod::infinite_pit::options {

// Apply patches to core game systems, such as save file I/O, file and module
// loading, and string message lookup.
void ApplyFixedPatches();

// Apply balance changes that are based on the current file's settings.
void ApplySettingBasedPatches();

// Spends FP for switching partners, if the option is enabled.
void SpendFpOnSwitchingPartner(ttyd::battle_unit::BattleWorkUnit* unit);
    
// Gets the Danger / Peril threshold to use based on a max HP value.
int32_t GetPinchThresholdForMaxHp(int32_t max_hp, bool peril);
// Overrides the default Danger / Peril thresholds for a battle unit.
void SetPinchThreshold(ttyd::battle_database_common::BattleUnitKind* kind, 
    int32_t max_hp, bool peril);
    
// Choose a random item for the audience to throw, if the option is enabled;
// otherwise, returns the originally selected item.
int32_t GetRandomAudienceItem(int32_t item_type);
// Correctly returns whether there is space for an audience-thrown item
// regardless of its type (using item / badge inventory, or none for cans, etc.)
uint32_t FixAudienceItemSpaceCheck(uint32_t empty_item_slots, uint32_t item_type);

}