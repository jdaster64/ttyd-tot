#pragma once

#include <cstdint>

namespace ttyd::npcdrv {
struct FbatBattleInformation;
}

namespace ttyd::battle_information {

extern "C" {
    
void BattleInformationSetDropMaterial(npcdrv::FbatBattleInformation* fbat_info);
// BattleInformationGetResult
// BattleInformationSetResult
// BattleInfomationSetBattleSetupInfo
// BattleInformationInit
// BattleInformationSetFirstAttack
// BattleInformationSetParty
// BattleInformationSetMode

}

}