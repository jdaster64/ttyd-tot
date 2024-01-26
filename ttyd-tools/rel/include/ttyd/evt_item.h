#pragma once

#include <cstdint>

#include "evtmgr.h"

namespace ttyd::evt_item {

extern "C" {

// evt_item_get_important_item
// evt_item_move_3d_position
// evt_item_set_alpha
// evt_item_set_scale
// evt_item_get_position
// evt_item_set_position
// evt_item_set_bound_next_dir
// evt_item_set_bound_next_speed
// evt_item_change_mode
// evt_item_get_item_end_wait
EVT_DECLARE_USER_FUNC(evt_item_get_item, 1)
// evt_item_set_bound_limit
// evt_item_set_bound_rate
// evt_item_set_gravity
// evt_item_set_jump_power
// evt_item_set_move_dir_speed
// evt_item_status_on
// evt_item_flag_off
// evt_item_flag_on
// evt_item_delete_check
// evt_item_delete
EVT_DECLARE_USER_FUNC(evt_item_entry, 8)

}

}