#pragma once

#include <cstdint>

namespace ttyd::battle {
struct BattleWorkCommand;
}
namespace ttyd::battle_database_common {
struct BattleWeapon;
}

namespace mod::tot::party_vivian {

// Custom function to populate the partner's weapon selection dialog.
void MakeSelectWeaponTable(
    ttyd::battle::BattleWorkCommand* command_work, int32_t* num_options);

// Returns a pointer to a custom override of Vivian's unhiding event.
void* GetVivianUnhideEvt();

}