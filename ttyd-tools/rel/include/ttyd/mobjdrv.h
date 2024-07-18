#pragma once

#include <cstdint>

namespace ttyd::mobjdrv {

extern "C" {

// mobjNearDistCheck2
// mobjCheckItemboxOpen
// mobjCheckExec
// mobjGetHint
// mobjCalcMtx2
// mobjCalcMtx
// mobjRunEvent
// mobjHitObjPtrToPtr
void* mobjNameToPtrNoAssert(const char* name);
// mobjNameToPtr
// mobjMain
// mobjSetPosition
// mobjDelete
// mobjEntry
// mobjHitEntry
// mobjReset
// mobjInit
// mobjDisp_OffscreenXLU
// mobjDisp
// mobjDispXLU
// mobjKoopaOn

}

}