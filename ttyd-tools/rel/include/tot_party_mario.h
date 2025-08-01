#pragma once

#include <cstdint>

namespace ttyd::battle {
struct BattleWork;
}
namespace ttyd::battle_database_common {
struct BattleWeapon;
}

namespace mod::tot::party_mario {

// Custom function to populate the weapon selection dialog.
int32_t MakeSelectWeaponTable(
    ttyd::battle::BattleWork* battleWork, int32_t table_type);

// Custom function to get First Strike attacks.   
ttyd::battle_database_common::BattleWeapon* GetFirstAttackWeapon(
    int32_t attack_type);

extern ttyd::battle_database_common::BattleWeapon* g_CustomJumpWeapons[8];
extern ttyd::battle_database_common::BattleWeapon* g_CustomHammerWeapons[8];
extern ttyd::battle_database_common::BattleWeapon* g_CustomHammerThrowWeapons[8];

extern ttyd::battle_database_common::BattleWeapon customWeapon_SuperHammerRecoil;
extern ttyd::battle_database_common::BattleWeapon customWeapon_UltraHammerRecoil;

}