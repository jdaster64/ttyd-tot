#pragma once

#include "evtmgr.h"

#include <cstdint>

namespace ttyd::evt_yuugijou {

extern "C" {

// evtYuuWindowEndChk
// evtYuuWindow
// unk_JP_US_EU_57_802295a8
// unk_JP_US_EU_58_802295b0
// yuuminigame_end
// yuuminigame_window_end
// yuugijou_set_yupflag
// yuugijou_get_takeoutflag
// yuugijou_montemedal_return
// yuugijou_montemedal_keep
// yuugijou_montemedal_wait
// yuugijou_montemedal_disp_onoff
// yuugijou_add_montemedal_moto
// yuugijou_get_montemedal_keep
// yuugijou_get_montemedal
// yuugijou_add_montemedal
// monteCountDisp
void yuugijou_init();

}

}