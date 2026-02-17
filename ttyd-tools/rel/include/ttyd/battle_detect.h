#pragma once

#include <cstdint>

namespace ttyd::battle {
struct BattleWorkWeaponTargets;
}

namespace ttyd::battle_detect {

extern "C" {

// BattleGetFirstAttackUnit
// BattleChoiceSamplingEnemy
// BattleSamplingEnemyUpdate
// BattleSamplingEnemy
void _btlSamplingEnemy(battle::BattleWorkWeaponTargets* targets_work);

}

}