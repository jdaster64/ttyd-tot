#pragma once

#include <cstdint>

#include "evtmgr.h"

namespace ttyd::evt_fade {

extern "C" {

// evt_fade_reset
// evt_fade_softfocus_onoff
// evt_fade_tec_onoff
EVT_DECLARE_USER_FUNC(evt_fade_set_mapchange_type, 5)
// evt_fade_set_anim_virtual_pos
// evt_fade_set_anim_ofs_pos
// evt_fade_set_spot_pos
// evt_fade_end_wait
// evt_fade_entry
// evt_fade_out
// evt_fade_in

}

}