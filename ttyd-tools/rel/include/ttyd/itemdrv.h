#pragma once

#include <gc/types.h>

#include <cstdint>

namespace ttyd::itemdrv {

extern "C" {

// itemPickUp
// itemStatus
// itemStatusOn
// itemFlag
// itemFlagOff
// itemFlagOn
// itemNokoForceGet
// itemForceGet
// itemHitCheckSide
// itemSetPosition
// itemseq_Bound
// itemseq_GetItem
// winFullDisp
// winHelpDisp
// winHelpMain
// winNameDisp2
// winNameDisp
// itemNearDistCheck
// itemHitCheck
// itemNameToPtr
// itemDelete
void* itemEntry(
    const char* name, int32_t item, float x, float y, float z, uint32_t mode,
    int32_t collection_gswf, void* pickup_script);
// itemModeChange
// itemMain
// N_itemPickUpFromFieldCheck
// itemGetNokoCheck
// itemGetCheck
// itemCoinDrop
// itemReInit
// itemInit

}

}