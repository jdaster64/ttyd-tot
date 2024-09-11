#pragma once

#include <cstdint>

#include "evtmgr.h"

namespace ttyd::evt_fade {

extern "C" {

EVT_DECLARE_USER_FUNC(evt_fade_reset, 1)
EVT_DECLARE_USER_FUNC(evt_fade_softfocus_onoff, 1)
EVT_DECLARE_USER_FUNC(evt_fade_tec_onoff, 2)
EVT_DECLARE_USER_FUNC(evt_fade_set_mapchange_type, 5)
EVT_DECLARE_USER_FUNC(evt_fade_set_anim_virtual_pos, 3)
EVT_DECLARE_USER_FUNC(evt_fade_set_anim_ofs_pos, 2)
EVT_DECLARE_USER_FUNC(evt_fade_set_spot_pos, 3)
EVT_DECLARE_USER_FUNC(evt_fade_end_wait, 0)
EVT_DECLARE_USER_FUNC(evt_fade_entry, 5)
EVT_DECLARE_USER_FUNC(evt_fade_out, 1)
EVT_DECLARE_USER_FUNC(evt_fade_in, 1)

}

}