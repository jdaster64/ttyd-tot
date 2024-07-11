#pragma once

#include <cstdint>

#include "evtmgr.h"

namespace ttyd::evt_party {

extern "C" {

EVT_DECLARE_USER_FUNC(N_evt_party_cloud_lock_animations_on_off, 2)
EVT_DECLARE_USER_FUNC(evt_party_cloud_breathout, 2)
EVT_DECLARE_USER_FUNC(evt_party_nokotaro_get_status, 2)
EVT_DECLARE_USER_FUNC(evt_party_nokotaro_hold_cancel, 1)
EVT_DECLARE_USER_FUNC(evt_party_nokotaro_kick_hold, 1)
EVT_DECLARE_USER_FUNC(evt_party_nokotaro_kick, 1)
EVT_DECLARE_USER_FUNC(evt_party_yoshi_fly, 3)
EVT_DECLARE_USER_FUNC(evt_party_yoshi_ride, 2)
EVT_DECLARE_USER_FUNC(L_evt_party_vivian_tail, 2)
EVT_DECLARE_USER_FUNC(evt_party_vivian_unhold, 1)
EVT_DECLARE_USER_FUNC(evt_party_vivian_hold, 1)
EVT_DECLARE_USER_FUNC(evt_party_thunders_use, 1)
EVT_DECLARE_USER_FUNC(evt_party_get_name_hitobj_ride, 2)
EVT_DECLARE_USER_FUNC(evt_party_get_name_hitobj_head, 2)
EVT_DECLARE_USER_FUNC(evt_party_get_status, 2)
EVT_DECLARE_USER_FUNC(L_evt_party_dokan, 2)
EVT_DECLARE_USER_FUNC(evt_party_set_breed_pose, 2)
EVT_DECLARE_USER_FUNC(evt_party_sleep_off, 1)
EVT_DECLARE_USER_FUNC(evt_party_sleep_on, 1)
EVT_DECLARE_USER_FUNC(evt_party_set_pose, 2)
EVT_DECLARE_USER_FUNC(evt_party_set_homing_dist, 2)
EVT_DECLARE_USER_FUNC(evt_party_move_beside_mario, 1)
EVT_DECLARE_USER_FUNC(evt_party_move_behind_mario, 1)
EVT_DECLARE_USER_FUNC(evt_party_jump_pos, 7)
EVT_DECLARE_USER_FUNC(evt_party_wait_landon, 1)
EVT_DECLARE_USER_FUNC(evt_party_move_pos2, 4)
EVT_DECLARE_USER_FUNC(evt_party_move_pos, 4)
EVT_DECLARE_USER_FUNC(evt_party_run, 1)
EVT_DECLARE_USER_FUNC(evt_party_stop, 1)
EVT_DECLARE_USER_FUNC(evt_party_set_dispdir, 2)
EVT_DECLARE_USER_FUNC(evt_party_get_dispdir, 2)
EVT_DECLARE_USER_FUNC(evt_party_set_dir_pos, 3)
EVT_DECLARE_USER_FUNC(evt_party_set_dir_npc, 2)
EVT_DECLARE_USER_FUNC(evt_party_set_dir, 3)
EVT_DECLARE_USER_FUNC(evt_party_get_pos, 4)
EVT_DECLARE_USER_FUNC(evt_party_set_hosei_xyz, 4)
EVT_DECLARE_USER_FUNC(evt_party_set_pos, 4)
EVT_DECLARE_USER_FUNC(evt_party_outofscreen, 2)
EVT_DECLARE_USER_FUNC(evt_party_force_reset_outofscreen, 1)
EVT_DECLARE_USER_FUNC(evt_party_set_camid, 2)
EVT_DECLARE_USER_FUNC(evt_party_init_camid, 1)
EVT_DECLARE_USER_FUNC(evt_party_cont_onoff, 2)
// unk_JP_US_EU_27_800eb9cc
EVT_DECLARE_USER_FUNC(evt_party_dispflg_onoff, 3)
EVT_DECLARE_USER_FUNC(evt_party_flg_onoff, 3)

}

}