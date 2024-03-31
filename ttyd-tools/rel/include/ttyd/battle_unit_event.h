#pragma once

#include <cstdint>

namespace ttyd::battle {
struct BattleWork;
}

namespace ttyd::battle_unit_event {

extern "C" {

// BattleRunWaitEvent
// BattlePhaseEventStartDeclare
// BattleRunPhaseEvent
// BattleRunHitEvent
// BattleRunHitEventDirect
bool BattleCheckEndUnitInitEvent(ttyd::battle::BattleWork* battleWork);

}

}