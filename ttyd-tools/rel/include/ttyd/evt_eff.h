#pragma once

#include <cstdint>

#include "evtmgr.h"

namespace ttyd::evt_eff {

extern "C" {

EVT_DECLARE_USER_FUNC(evt_eff_fukidashi, 11)
EVT_DECLARE_USER_FUNC(evt_eff_delete_ptr, 1)
EVT_DECLARE_USER_FUNC(evt_eff_softdelete_ptr, 1)
EVT_DECLARE_USER_FUNC(evt_eff_delete, 1)
EVT_DECLARE_USER_FUNC(evt_eff_softdelete, 1)
EVT_DECLARE_USER_FUNC(evt_eff64, 14)
EVT_DECLARE_USER_FUNC(evt_eff, 14)

}

}