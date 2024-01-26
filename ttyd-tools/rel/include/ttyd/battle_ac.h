#pragma once

#include <cstdint>

namespace ttyd::battle_database_common {
struct BattleWeapon;
}
namespace ttyd::battle_unit {
struct BattleWorkUnit;
}

namespace ttyd::battle_ac {

extern "C" {

// .text
// BattleAcGaugeSeDelete
// BattleAcGaugeSeUpdate
// BattleAcGaugeSeInit
// BattleAcDrawGauge
// BattleACGetButtonIcon
// BattleActionCommandGetPrizeLv
// BattleActionCommandSetDifficulty
// BattleActionCommandGetDifficulty
// BattleActionCommandResetDefenceResult
// BattleActionCommandGetDefenceResult
// BattleACPadCheckRecordTrigger
int32_t BattleActionCommandCheckDefence(
    battle_unit::BattleWorkUnit* unit,
    battle_database_common::BattleWeapon* weapon);
// BattleActionCommandStop
// BattleActionCommandStart
// BattleActionCommandSetup
// BattleActionCommandDeclareACResult
// BattleActionCommandResult
// BattleActionCommandManager
// BattleActionCommandManagerInit

// .data
extern int8_t guard_frames[8];
extern int8_t superguard_frames[8];

}

}