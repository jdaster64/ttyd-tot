#pragma once

#include "evtmgr.h"

#include <cstdint>

namespace ttyd::battle_icon {

extern "C" {

EVT_DECLARE_USER_FUNC(btlevtcmd_BtlIconJumpPosition, 5)
EVT_DECLARE_USER_FUNC(btlevtcmd_BtlIconSetFallAccel, 2)
EVT_DECLARE_USER_FUNC(btlevtcmd_BtlIconSetPosition, 4)
EVT_DECLARE_USER_FUNC(btlevtcmd_BtlIconDelete, 1)
EVT_DECLARE_USER_FUNC(btlevtcmd_BtlIconEntryItemId, 5)
EVT_DECLARE_USER_FUNC(btlevtcmd_BtlIconEntry, 5)
// BtlIcon_Delete
// BtlIcon_Entry
// BattleIconEnd
// BattleIconMain
// BattleIconInit
// BtlIconGetPtr

}

}