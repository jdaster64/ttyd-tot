#pragma once

#include "evtmgr.h"

#include <cstdint>

namespace ttyd::battle_camera {

extern "C" {

EVT_DECLARE_USER_FUNC(evt_btl_camera_wait_move_end, 0)
EVT_DECLARE_USER_FUNC(evt_btl_camera_off_posoffset_manual, 1)
// btl_camera_off_posoffset_manual
EVT_DECLARE_USER_FUNC(evt_btl_camera_set_posoffset, 4)
// btl_camera_set_posoffset
EVT_DECLARE_USER_FUNC(evt_btl_camera_nomove_z_onoff, 2)
// btl_camera_nomove_z_onoff
EVT_DECLARE_USER_FUNC(evt_btl_camera_nomove_y_onoff, 2)
// btl_camera_nomove_y_onoff
EVT_DECLARE_USER_FUNC(evt_btl_camera_nomove_x_onoff, 2)
// btl_camera_nomove_x_onoff
EVT_DECLARE_USER_FUNC(evt_btl_camera_noshake, 1)
// btl_camera_noshake
EVT_DECLARE_USER_FUNC(evt_btl_camera_shake_h, 5)
// btl_camera_shake_h
EVT_DECLARE_USER_FUNC(evt_btl_camera_shake_w, 5)
// btl_camera_shake_w
EVT_DECLARE_USER_FUNC(evt_btl_camera_set_moveto, 9)
// btl_camera_set_moveto
EVT_DECLARE_USER_FUNC(evt_btl_camera_set_zoomSpeedLv, 2)
EVT_DECLARE_USER_FUNC(evt_btl_camera_set_moveSpeedLv, 2)
// btl_camera_set_zoomSpeedLv
// btl_camera_set_moveSpeedLv
EVT_DECLARE_USER_FUNC(evt_btl_camera_add_zoom, 2)
EVT_DECLARE_USER_FUNC(evt_btl_camera_set_zoom, 2)
// btl_camera_add_zoom
// btl_camera_set_zoom
EVT_DECLARE_USER_FUNC(evt_btl_camera_set_homing_unit_audience, 4)
EVT_DECLARE_USER_FUNC(evt_btl_camera_set_homing_unitparts, 5)
EVT_DECLARE_USER_FUNC(evt_btl_camera_set_homing_unit, 3)
EVT_DECLARE_USER_FUNC(evt_btl_camera_set_mode, 2)
// btl_camera_set_mode
EVT_DECLARE_USER_FUNC(evt_btl_camera_set_prilimit, 1)
// btl_camera_set_prilimit
// battleCameraGetPosMoveSpeed
// battleCameraMoveTo
// battleCameraMain
// battleCameraInit

}

}