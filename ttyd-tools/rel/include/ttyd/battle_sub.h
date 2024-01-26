#pragma once

#include "evtmgr.h"

#include <cstdint>

namespace ttyd::battle_sub {

extern "C" {

int32_t BattleTransID(evtmgr::EvtEntry* evt, int32_t id);
// BtlCompForwardLv
// intpl_sub
// btlMovePos
// atan2f_safety
// cosfd
// sinfd

}

}