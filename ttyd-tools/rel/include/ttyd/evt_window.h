#pragma once

#include <cstdint>

#include "evtmgr.h"

namespace ttyd::evt_window {

extern "C" {

// mainwindow_opendisable
// mainwindow_openenable
EVT_DECLARE_USER_FUNC(evt_win_coin_wait, 1)
EVT_DECLARE_USER_FUNC(evt_win_coin_off, 1)
EVT_DECLARE_USER_FUNC(evt_win_coin_on, 2)
// coin_disp
// evt_win_one_message
// oneMessageDisp
// evt_win_nameent_off
// evt_win_nameent_name
// evt_win_nameent_wait
// evt_win_nameent_on
// evt_win_item_desttable
// evt_win_item_maketable
EVT_DECLARE_USER_FUNC(evt_win_other_select, 1)
EVT_DECLARE_USER_FUNC(evt_win_item_select, 4)


// evt_snd_bgm_scope
// evt_snd_bgm_freq
// evt_snd_bgmoff_f
EVT_DECLARE_USER_FUNC(evt_snd_bgmoff, 1)
// unk_JP_US_EU_37_801524c8
EVT_DECLARE_USER_FUNC(evt_snd_bgmon_f, 3)
EVT_DECLARE_USER_FUNC(evt_snd_bgmon, 2)

}

}