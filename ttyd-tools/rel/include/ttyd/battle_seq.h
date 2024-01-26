#pragma once

#include <cstdint>

namespace ttyd::battle {
struct BattleWork;
}
namespace ttyd::battle_database_common {
struct BattleWeapon;
}

namespace ttyd::battle_seq {

extern "C" {

// _set_haikei_entry_scale
// _mapobj_data_touch_scale
uint32_t BattleCheckConcluded(ttyd::battle::BattleWork* battleWork);
// BattleWaitAllActiveEvtEnd_NoBgSetEndWait
// BattleWaitAllActiveEvtEnd
// battleMakePhaseEvtTable
// battleSortPhaseMoveTable
// btlseqPhaseFirstProcess
// btlseqTurnFirstProcess
// btlseqAct
// BattlePhaseEndCheck
// btlseqMove
// btlseqPhase
void _rule_disp();
// btlseqTurn
// _set_effect_luck
// btlseqFirstAct
ttyd::battle_database_common::BattleWeapon* _GetFirstAttackWeapon(
    int32_t attack_type);
// btlseqInit
// BattleCheckAllPinchStatus
// BattleSequenceManager

}

}