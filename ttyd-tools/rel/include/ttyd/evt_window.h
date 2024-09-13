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
EVT_DECLARE_USER_FUNC(evt_win_one_message, 1)
// oneMessageDisp
EVT_DECLARE_USER_FUNC(evt_win_nameent_off, 0)
EVT_DECLARE_USER_FUNC(evt_win_nameent_name, 1)
EVT_DECLARE_USER_FUNC(evt_win_nameent_wait, 0)
EVT_DECLARE_USER_FUNC(evt_win_nameent_on, 1)
// evt_win_item_desttable
// evt_win_item_maketable
EVT_DECLARE_USER_FUNC(evt_win_other_select, 1)
EVT_DECLARE_USER_FUNC(evt_win_item_select, 4)

}

}