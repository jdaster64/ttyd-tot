#pragma once

#include <cstdint>

#include "evtmgr.h"

namespace ttyd::evt_mobj {

extern "C" {

// .text
// evt_mobj_koopa_pole
// mobj_koopa_pole_evt
// kpa_clear_jump_data
// kpa_disp_pole_init
// kpa_disp_pole_score_main
// kpa_disp_pole_score
// kpa_score_to_str
// kpa_get_width
// kpa_get_level
// kpa_get_hata_name
// kpa_get_pole_name
// evt_mobj_koopa_fireber_dodai
// mobj_koopa_fireber_dodai_evt
// evt_mobj_koopa_sango
// mobj_koopa_sango_evt
// fire_func
// evt_mobj_koopa_dokan
// mobj_koopa_dokan_evt
// evt_mobj_koopa_blk
// mobj_koopa_blk
// evt_mobj_koopa_hidden_blk
// evt_mobj_koopa_badgeblk
// mobj_koopa_badgeblk
// evt_mobj_koopa_brick
// mobj_koopa_brickblk
// evt_mobj_koopa_stone_blk
// mobj_koopa_stone_blk_evt
// evt_mobj_koopa_ojama_blk
// mobj_koopa_ojama_blk_evt
// kpa_powerup
// evt_mobj_kururing_floor
// mobj_kururing_floor
// kururing_capture
// evt_mobj_breaking_floor
// mobj_breaking_floor
// evt_mobj_brick
// mobj_10countblk
// mobj_brickblk
// evt_mobj_powerupblk
// mobj_powerupblk
// evt_mobj_badgeblk
// mobj_badgeblk
// evt_mobj_blk
// mobj_blk
// evt_mobj_save_blk
// mobj_save_blk
// evt_mobj_recovery_blk
// mobj_recovery_blk
EVT_DECLARE_USER_FUNC(evt_mobj_signboard, 6)
// mobj_signboard
// evt_mobj_itembox
// mobj_itembox
// evt_mobj_lock
// mobj_lock
// evt_mobj_lock_unlock
// evt_mobj_switch_float_blk
// mobj_switch_float_blk
// evt_mobj_float_blk
// mobj_float_blk
// evt_mobj_lv_blk
// mobj_lv_blk_evt
// evt_mobj_jumpstand_blue
// mobj_jumpstand_blue
// evt_mobj_jumpstand_red
// mobj_jumpstand_red
// evt_mobj_rideswitch_lightblue
// mobj_rideswitch_lightblue
// evt_mobj_timerswitch
// mobj_timerswitch
// evt_mobj_floatswitch_red
// evt_mobj_switch_red
// mobj_switch_red
// evt_mobj_tornadoswitch_blue
// evt_mobj_floatswitch_blue
// evt_mobj_switch_blue
// mobj_switch_blue
// evt_mobj_hit_onoff
// evt_mobj_hitevt_onoff
// evt_mobj_set_camid
EVT_DECLARE_USER_FUNC(evt_mobj_wait_animation_end, 1)
// evt_mobj_set_anim
// evt_mobj_set_gravity_bound
// evt_mobj_exec_cancel
// evt_mobj_set_z_position
// evt_mobj_set_y_position
// evt_mobj_set_x_position
// evt_mobj_set_position
// evt_mobj_get_z_position
// evt_mobj_get_y_position
// evt_mobj_get_x_position
EVT_DECLARE_USER_FUNC(evt_mobj_get_position, 4)
// evt_mobj_set_scale
// evt_mobj_get_kindname
// evt_mobj_flag_onoff
// evt_mobj_check
// evt_mobj_delete
// evt_mobj_entry

// .data
extern int32_t mobj_save_blk_sysevt[1];

}

}