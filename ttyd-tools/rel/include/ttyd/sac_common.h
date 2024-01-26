#pragma once

#include "evtmgr.h"

#include <cstdint>

namespace ttyd::sac_common {

extern "C" {

// sac_ac_help_off
// sac_ac_help_on
EVT_DECLARE_USER_FUNC(sac_enemy_slide_return, 0)
// sac_enemy_slide_go
// sac_cheer_end
// sac_cheer
// sac_handbeat_end
// sac_handbeat
EVT_DECLARE_USER_FUNC(sac_wao, 0)

}

}