#pragma once

#include "evtmgr.h"

#include <cstdint>

namespace ttyd::battle_event_subset {

extern "C" {

// .text
// _binta_effect
// _check_through_pos
// unk_80113f1c
// _add_star_point_disp_offset
// _check_at_dm_event_wait
// _disable_restore_command_cursor

// .data
extern int32_t unk_evt_803537c4[1];

}

}