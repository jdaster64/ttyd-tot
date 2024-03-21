#pragma once

#include <cstdint>

#include "evtmgr.h"

namespace ttyd::evt_ext {

extern "C" {

EVT_DECLARE_USER_FUNC(evt_ext_reset, 0)

// arg0 = count
// arg1 = ext_data
// arg2 = init func
// arg3 = main func
// arg4 = disp func
EVT_DECLARE_USER_FUNC(evt_ext_entry, 5)

}

}