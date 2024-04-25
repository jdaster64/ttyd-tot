#pragma once

#include "evtmgr.h"

#include <cstdint>

namespace ttyd::battle_message {

extern "C" {

EVT_DECLARE_USER_FUNC(btlevtcmd_AnnounceSetParam, 2)
void battle_message_disp(void);
EVT_DECLARE_USER_FUNC(btlevtcmd_AnnounceMessage, 5)

}

}