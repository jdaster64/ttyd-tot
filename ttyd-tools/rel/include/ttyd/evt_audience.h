#pragma once

#include <cstdint>

#include "evtmgr.h"

namespace ttyd::evt_audience {

extern "C" {

EVT_DECLARE_USER_FUNC(evt_audience_joy_ending, 0)
EVT_DECLARE_USER_FUNC(evt_audience_acrobat_notry, 0)
EVT_DECLARE_USER_FUNC(evt_audience_num_updown_in_event, 1)
EVT_DECLARE_USER_FUNC(evt_audience_sound_cheer, 2)
EVT_DECLARE_USER_FUNC(evt_audience_abnormal_natural_all, 1)
EVT_DECLARE_USER_FUNC(evt_audience_reflesh_natural_all, 1)
EVT_DECLARE_USER_FUNC(evt_audience_ap_recovery, 0)
EVT_DECLARE_USER_FUNC(evt_audience_delete, 1)
EVT_DECLARE_USER_FUNC(evt_audience_entry, 2)
EVT_DECLARE_USER_FUNC(evt_audience_set_animpose, 3)
EVT_DECLARE_USER_FUNC(evt_audience_jump_position_gravity, 6)
EVT_DECLARE_USER_FUNC(evt_audience_jump_position_firstsp, 6)
EVT_DECLARE_USER_FUNC(evt_audience_move_position_speed, 5)
EVT_DECLARE_USER_FUNC(evt_audience_move_position_frame, 6)
EVT_DECLARE_USER_FUNC(evt_audience_set_rotate_offset, 4)
EVT_DECLARE_USER_FUNC(evt_audience_set_rotate, 4)
EVT_DECLARE_USER_FUNC(evt_audience_get_position, 4)
EVT_DECLARE_USER_FUNC(evt_audience_set_position, 4)
EVT_DECLARE_USER_FUNC(evt_audience_flag_off, 2)
EVT_DECLARE_USER_FUNC(evt_audience_flag_on, 2)
EVT_DECLARE_USER_FUNC(evt_audience_flag_check, 3)
EVT_DECLARE_USER_FUNC(evt_audience_base_flag_on, 1)

}

}