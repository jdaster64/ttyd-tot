#pragma once

#include "evtmgr.h"

#include <cstdint>

namespace ttyd::battle_sub {

extern "C" {

int32_t BattleTransID(evtmgr::EvtEntry* evt, int32_t id);
// BtlCompForwardLv
// intpl_sub
void btlMovePos(float distance, float angle, float* x, float* z);
// atan2f_safety
// cosfd
// sinfd

}

}