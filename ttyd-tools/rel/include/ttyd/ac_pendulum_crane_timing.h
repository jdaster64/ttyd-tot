#pragma once

#include <cstdint>

namespace ttyd::ac_pendulum_crane_timing {

extern "C" {

// .text
// _get_angle_hp
// _get_angle_rate
// actionCommandDisp
// battleAcDelete_PendulumCraneTiming
// battleAcDisp_PendulumCraneTiming
// battleAcResult_PendulumCraneTiming
// battleAcMain_PendulumCraneTiming

// .data
extern int16_t pendulumCrane_hp_tbl[14];

}

}