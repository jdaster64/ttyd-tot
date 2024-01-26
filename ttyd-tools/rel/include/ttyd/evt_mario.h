#pragma once

#include "evtmgr.h"

#include <cstdint>

namespace ttyd::evt_mario {

extern "C" {

// evt_mario_paper_plane
// L_evt_mario_keyon_wait
// evt_mario_get_name_hitobj_push
// evt_mario_get_name_hitobj_front
// evt_mario_get_name_hitobj_under
// evt_mario_get_name_hitobj_ride
// evt_mario_get_name_hitobj_head
// evt_mario_dump_item
// evt_mario_check_key_item
// evt_mario_sleep_off
// evt_mario_sleep_on
// evt_mario_push
// evt_mario_clear_party
EVT_DECLARE_USER_FUNC(evt_mario_kill_party, 1)
EVT_DECLARE_USER_FUNC(evt_mario_goodbye_party, 1)
// evt_mario_set_prev_party_pos
// evt_mario_set_prev_party
EVT_DECLARE_USER_FUNC(evt_mario_set_party_pos, 5)
// evt_mario_set_party
// evt_mario_hello_exparty_pos
// evt_mario_hello_party
// evt_mario_set_prev_party_dokan
// evt_mario_party_use_check
// evt_mario_chk_join_party
// evt_mario_get_exparty
// evt_mario_get_party
// evt_peach_set_condition
// evt_peach_transform_gundan_off
// evt_peach_transform_gundan_on
// evt_koopa_set_level
// evt_koopa_get_level
// evt_mario_set_mode
// evt_mario_get_mode
// evt_mario_get_state
// evt_mario_wait_movable
// evt_mario_wait_move_end
// evt_koopa_weary
// evt_koopa_chk_dead
// evt_koopa_fire
// evt_koopa_hip_attack
// evt_mario_check_landon
// evt_mario_wait_rideon
// evt_mario_wait_landon
// evt_mario_jump_jumpstand
// evt_mario_jump_pos
// evt_mario_party_bero_move
// evt_mario_party_door_move
// N_evt_mario_party_door_halve_hitbox
// evt_mario_mov_pos2
// evt_mario_mov_pos
// evt_mario_set_force_dir
// evt_mario_get_pose
// evt_mario_cancel_roll_motion
// evt_mario_set_talk_motion
// evt_mario_chk_hipbump
// evt_mario_set_motion
// evt_mario_get_motion
EVT_DECLARE_USER_FUNC(evt_mario_normalize, 0)
// evt_mario_set_pose
// evt_mario_set_normal_pose
// evt_mario_adjust_dir
// unk_JP_US_EU_26_800e6b1c
// evt_mario_set_dir_npc
// evt_mario_set_dispdir
// evt_mario_get_dispdir
// evt_mario_set_reardir
// evt_mario_set_dir
// evt_mario_get_dir
// evt_mario_get_mov_spd
// evt_mario_set_mov_spd
// evt_mario_set_hosei_xyz
// evt_mario_set_pos
// evt_mario_get_pos
// evt_mario_set_camid
// evt_mario_init_camid
// evt_mario_cam_y_off
// evt_mario_cam_y_on
// evt_mario_cam_keep_off
// evt_mario_cam_keep_on
// evt_mario_cam_off
// evt_mario_cam_on
// evt_mario_bgmode_off
// evt_mario_bgmode_on
// evt_mario_set_enable_key
EVT_DECLARE_USER_FUNC(evt_mario_key_onoff, 1)
// evt_mario_cont_onoff
// evt_mario_trigflag_onoff
// evt_mario_dispflag_onoff
// evt_mario_flag_onoff
// evt_mario_balloon_off
// evt_mario_balloon_tenten
// evt_mario_balloon_bikkuri
// evt_mario_paper_pose_off
// evt_mario_dokan_end
// evt_mario_paper_pose_dokan
// evt_mario_dokan_prepare

}

}