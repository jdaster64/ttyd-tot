#pragma once

#include "evtmgr.h"

#include <cstdint>

namespace ttyd::battle_message {

extern "C" {

EVT_DECLARE_USER_FUNC(btlevtcmd_AnnounceSetParam, 2)
// _disp
EVT_DECLARE_USER_FUNC(btlevtcmd_AnnounceMessage, 5)

}

}