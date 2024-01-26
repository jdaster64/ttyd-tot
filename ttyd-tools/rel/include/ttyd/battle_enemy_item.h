#pragma once

#include "battle_unit.h"

#include <cstdint>

namespace ttyd::battle_enemy_item {

extern "C" {

void* _check_status_recover_item(battle_unit::BattleWorkUnit* unit);
void* _check_status_support_item(battle_unit::BattleWorkUnit* unit);
void* _check_status_attack_item(battle_unit::BattleWorkUnit* unit);
void* _check_attack_item(battle_unit::BattleWorkUnit* unit);
void* _check_fp_recover_item(battle_unit::BattleWorkUnit* unit);
void* _check_hp_recover_item(battle_unit::BattleWorkUnit* unit);
void* BattleEnemyUseItemCheck(battle_unit::BattleWorkUnit* unit);

}

}