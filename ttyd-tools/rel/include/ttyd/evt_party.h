#pragma once

#include <cstdint>

#include "evtmgr.h"

namespace ttyd::evt_party {

extern "C" {

// N_evt_party_cloud_lock_animations_on_off
// evt_party_cloud_breathout
// evt_party_nokotaro_get_status
// evt_party_nokotaro_hold_cancel
// evt_party_nokotaro_kick_hold
// evt_party_nokotaro_kick
// evt_party_yoshi_fly
// evt_party_yoshi_ride
// L_evt_party_vivian_tail
// evt_party_vivian_unhold
// evt_party_vivian_hold
// evt_party_thunders_use
// evt_party_get_name_hitobj_ride
// evt_party_get_name_hitobj_head
// evt_party_get_status
// L_evt_party_dokan
// evt_party_set_breed_pose
// evt_party_sleep_off
// evt_party_sleep_on
// evt_party_set_pose
// evt_party_set_homing_dist
// evt_party_move_beside_mario
// evt_party_move_behind_mario
// evt_party_jump_pos
// evt_party_wait_landon
// evt_party_move_pos2
// evt_party_move_pos
EVT_DECLARE_USER_FUNC(evt_party_run, 1)
// evt_party_stop
// evt_party_set_dispdir
// evt_party_get_dispdir
// evt_party_set_dir_pos
// evt_party_set_dir_npc
// evt_party_set_dir
EVT_DECLARE_USER_FUNC(evt_party_get_pos, 4)
// evt_party_set_hosei_xyz
// evt_party_set_pos
// evt_party_outofscreen
// evt_party_force_reset_outofscreen
EVT_DECLARE_USER_FUNC(evt_party_set_camid, 2)
// evt_party_init_camid
// evt_party_cont_onoff
// unk_JP_US_EU_27_800eb9cc
// evt_party_dispflg_onoff
// evt_party_flg_onoff

}

}