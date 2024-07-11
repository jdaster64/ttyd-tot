#pragma once

#include <cstdint>

#include "evtmgr.h"

namespace ttyd::evt_cam {

extern "C" {

EVT_DECLARE_USER_FUNC(evt_cam_letter_box_camid, 1)
EVT_DECLARE_USER_FUNC(evt_cam_letter_box_disable, 1)
EVT_DECLARE_USER_FUNC(evt_cam_letter_box_onoff, 2)
EVT_DECLARE_USER_FUNC(evt_cam_type_change, 2)
EVT_DECLARE_USER_FUNC(evt_cam_shift_reset, 0)
EVT_DECLARE_USER_FUNC(evt_cam_road_reset2, 3)
EVT_DECLARE_USER_FUNC(evt_cam_road_reset, 0)
EVT_DECLARE_USER_FUNC(evt_cam3d_event_from_road, 10)
EVT_DECLARE_USER_FUNC(evt_cam3d_evt_set_now, 0)
EVT_DECLARE_USER_FUNC(evt_cam3d_get_shift, 4)
EVT_DECLARE_USER_FUNC(evt_cam3d_evt_xyz_off, 2)
EVT_DECLARE_USER_FUNC(evt_cam3d_evt_set_xyz, 6)
EVT_DECLARE_USER_FUNC(evt_cam3d_road_shift_onoff, 1)
EVT_DECLARE_USER_FUNC(evt_cam3d_evt_off, 2)
EVT_DECLARE_USER_FUNC(evt_cam3d_evt_set_rel_dir, 8)
EVT_DECLARE_USER_FUNC(evt_cam3d_evt_set_npc_rel, 9)
EVT_DECLARE_USER_FUNC(evt_cam3d_evt_set_rel, 8)
EVT_DECLARE_USER_FUNC(evt_cam3d_evt_set, 8)
EVT_DECLARE_USER_FUNC(evt_cam3d_evt_set_at, 6)
EVT_DECLARE_USER_FUNC(evt_cam3d_evt_zoom_in, 4)
EVT_DECLARE_USER_FUNC(evt_cam_shake, 4)
EVT_DECLARE_USER_FUNC(evt_cam_get_at, 4)
EVT_DECLARE_USER_FUNC(evt_cam_get_pos, 4)
EVT_DECLARE_USER_FUNC(evt_cam_ctrl_onoff, 2)

}

}