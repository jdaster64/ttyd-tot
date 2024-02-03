#pragma once

#include <cstdint>

#include "evtmgr.h"

namespace ttyd::evt_batstage {

extern "C" {

EVT_DECLARE_USER_FUNC(evt_batstage_light_set_player_off, 0)
EVT_DECLARE_USER_FUNC(evt_batstage_light_set_player_on, 1)
EVT_DECLARE_USER_FUNC(evt_batstage_return_aud_dark_base, 2)
EVT_DECLARE_USER_FUNC(evt_batstage_set_aud_dark_base, 3)
EVT_DECLARE_USER_FUNC(evt_batstage_set_aud_dark, 3)
EVT_DECLARE_USER_FUNC(evt_batstage_return_stg_dark_base, 2)
EVT_DECLARE_USER_FUNC(evt_batstage_set_stg_dark_base, 3)
EVT_DECLARE_USER_FUNC(evt_batstage_set_stg_dark, 3)

}

}
