#pragma once

#include <cstdint>

namespace ttyd::battle {
struct BattleWork;
}

namespace ttyd::ac_button_down {

extern "C" {

// actionCommandDisp
// battleAcDelete_ButtonDown
// battleAcDisp_ButtonDown
uint32_t battleAcResult_ButtonDown(ttyd::battle::BattleWork* battleWork);
uint32_t battleAcMain_ButtonDown(ttyd::battle::BattleWork* battleWork);

}

}