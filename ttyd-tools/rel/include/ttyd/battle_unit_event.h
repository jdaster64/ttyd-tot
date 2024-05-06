#pragma once

#include <cstdint>

namespace ttyd::battle {
struct BattleWork;
}
namespace ttyd::battle_unit {
struct BattleWorkUnit;
}

namespace ttyd::battle_unit_event {

extern "C" {

// BattleRunWaitEvent
// BattlePhaseEventStartDeclare
// BattleRunPhaseEvent

int32_t BattleRunHitEvent(
    ttyd::battle_unit::BattleWorkUnit* unit, uint32_t damage_code);

// BattleRunHitEventDirect
bool BattleCheckEndUnitInitEvent(ttyd::battle::BattleWork* battleWork);

}

}