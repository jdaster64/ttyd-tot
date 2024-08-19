#pragma once

#include <cstdint>

#include "evtmgr.h"

namespace ttyd::evt_damage {

extern "C" {

// .text
EVT_DECLARE_USER_FUNC(evt_gazigazi_move_position, 9)

// .data
extern int32_t evt_gazigazi_entry[1];
extern int32_t evt_gazigazi_anim_loop[1];
extern int32_t evt_gazigazi_appear[1];

}

}