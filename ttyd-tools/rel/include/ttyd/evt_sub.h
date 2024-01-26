#pragma once

#include <cstdint>

#include "evtmgr.h"

namespace ttyd::evt_sub {

extern "C" {

EVT_DECLARE_USER_FUNC(mail_evt_gor_04_keyoff_check, 1)
EVT_DECLARE_USER_FUNC(irai_mail_check, 2)
EVT_DECLARE_USER_FUNC(evt_sub_check_intersect, 9)
EVT_DECLARE_USER_FUNC(unk_80053f10, 2)
EVT_DECLARE_USER_FUNC(bgm_start_wait, 0)
EVT_DECLARE_USER_FUNC(get_stop, 1)
EVT_DECLARE_USER_FUNC(set_stop, 1)
EVT_DECLARE_USER_FUNC(stageclear_wait, 0)
EVT_DECLARE_USER_FUNC(stone_bg_open, 1)
EVT_DECLARE_USER_FUNC(stone_bg, 1)
EVT_DECLARE_USER_FUNC(stone_ry, 2)
EVT_DECLARE_USER_FUNC(evt_sub_get_coin, 1)
// coingetDisp
EVT_DECLARE_USER_FUNC(evt_sub_countdown_set_restart_time, 1)
EVT_DECLARE_USER_FUNC(evt_sub_countdown_flag_onoff, 2)
EVT_DECLARE_USER_FUNC(evt_sub_countdown_end, 0)
EVT_DECLARE_USER_FUNC(evt_sub_countdown_get_status, 1)
EVT_DECLARE_USER_FUNC(evt_sub_countdown_start, 2)
EVT_DECLARE_USER_FUNC(evt_sub_animgroup_async, 1)
EVT_DECLARE_USER_FUNC(evt_sub_get_language, 1)
EVT_DECLARE_USER_FUNC(evt_sub_load_progresstime, 2)
EVT_DECLARE_USER_FUNC(evt_sub_save_playtime, 1)
EVT_DECLARE_USER_FUNC(N_evt_sub_status_gauge_force_update, 0)
EVT_DECLARE_USER_FUNC(evt_sub_status_gauge_check_update, 1)
EVT_DECLARE_USER_FUNC(evt_sub_status_gauge_force_close, 0)
EVT_DECLARE_USER_FUNC(evt_sub_status_gauge_force_open, 0)
EVT_DECLARE_USER_FUNC(evt_sub_get_areaname, 1)
EVT_DECLARE_USER_FUNC(evt_sub_get_mapname, 1)
EVT_DECLARE_USER_FUNC(evt_key_get_buttontrg, 2)
EVT_DECLARE_USER_FUNC(evt_key_get_button, 2)
EVT_DECLARE_USER_FUNC(evt_key_get_dir, 2)
EVT_DECLARE_USER_FUNC(evt_sub_system_flag_onoff, 2)
EVT_DECLARE_USER_FUNC(evt_sub_get_system_flag, 1)
EVT_DECLARE_USER_FUNC(evt_sub_get_dir, 5)
EVT_DECLARE_USER_FUNC(evt_sub_get_dist, 5)
EVT_DECLARE_USER_FUNC(evt_sub_area_check, 7)
EVT_DECLARE_USER_FUNC(evt_sub_get_stopwatch, 2)
EVT_DECLARE_USER_FUNC(evt_sub_random, 2)
EVT_DECLARE_USER_FUNC(evt_sub_rumble_onoff, 2)
EVT_DECLARE_USER_FUNC(evt_sub_get_sincos, 3)
EVT_DECLARE_USER_FUNC(evt_sub_spline_free, 0)
EVT_DECLARE_USER_FUNC(evt_sub_spline_get_value_manual, 4)
EVT_DECLARE_USER_FUNC(evt_sub_spline_get_value, 4)
EVT_DECLARE_USER_FUNC(evt_sub_spline_init, 6)
EVT_DECLARE_USER_FUNC(evt_sub_intpl_msec_get_value_para, 2)
EVT_DECLARE_USER_FUNC(evt_sub_intpl_msec_get_value, 0)
EVT_DECLARE_USER_FUNC(evt_sub_intpl_msec_init, 4)
EVT_DECLARE_USER_FUNC(evt_sub_intpl_get_float, 2)
EVT_DECLARE_USER_FUNC(evt_sub_intpl_init_float, 4)
EVT_DECLARE_USER_FUNC(evt_sub_intpl_get_value_para, 2)
EVT_DECLARE_USER_FUNC(evt_sub_intpl_get_value, 0)
EVT_DECLARE_USER_FUNC(evt_sub_intpl_init, 4)

}

}