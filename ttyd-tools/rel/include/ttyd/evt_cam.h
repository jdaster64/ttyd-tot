#pragma once

#include <cstdint>

#include "evtmgr.h"

namespace ttyd::evt_cam {

extern "C" {

// evt_cam_letter_box_camid
// evt_cam_letter_box_disable
// evt_cam_letter_box_onoff
// evt_cam_type_change
// evt_cam_shift_reset
// evt_cam_road_reset2
// evt_cam_road_reset
// evt_cam3d_event_from_road
// evt_cam3d_evt_set_now
// evt_cam3d_get_shift
// evt_cam3d_evt_xyz_off
// evt_cam3d_evt_set_xyz
// evt_cam3d_road_shift_onoff
// evt_cam3d_evt_off
// evt_cam3d_evt_set_rel_dir
// evt_cam3d_evt_set_npc_rel
// evt_cam3d_evt_set_rel
// evt_cam3d_evt_set
// evt_cam3d_evt_set_at
// evt_cam3d_evt_zoom_in
// evt_cam_shake
// evt_cam_get_at
// evt_cam_get_pos
EVT_DECLARE_USER_FUNC(evt_cam_ctrl_onoff, 2)

}

}