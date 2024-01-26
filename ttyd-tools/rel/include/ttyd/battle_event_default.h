#pragma once

#include "evtmgr.h"

#include <cstdint>

namespace ttyd::battle_event_default {

extern "C" {

// .text
EVT_DECLARE_USER_FUNC(_get_flower_suitoru_point, 2);
EVT_DECLARE_USER_FUNC(_get_heart_suitoru_point, 2);
// _get_escape_rate
// _set_escape_ac_hlp_msg
// _btl_escape_reset_move_color_lv_player_unit_all
// btlevtcmd_audience_code_escape_fail
// btlevtcmd_audience_code_escape_success
// btlevtcmd_audience_code_escape
// _check_pose_stay
// _anime_load_wait
// _anime_load
// _backfire

// .data
extern int32_t btldefaultevt_SuitoruBadgeEffect[1];

}

}