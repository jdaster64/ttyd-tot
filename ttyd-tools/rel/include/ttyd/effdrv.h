#pragma once

#include <cstdint>

namespace ttyd::effdrv {
    
struct EffEntry {
    uint32_t        flags;
    uint32_t        bBattle;
    int32_t         eff_count;
    void*           eff_work;
    void*           main_func;
    const char*     type_name;
    char            name[16];
} __attribute__((__packed__));

static_assert(sizeof(EffEntry) == 0x28);

extern "C" {

// effDrawMayaPoly
// effCalcMayaAnim
// effPlayMayaAnim
// effDeleteMayaAnim
// effMayaAnimAlloc
// effCalcMayaAnimMatrix
// effGetSet
// effNameToPtr
// effSoftDelete
// effDelete
// effMain
// effSetName
// effEntry
// effAutoRelease
// effGetTexObj
// effTexSetup
// effInit
// _callback_tpl

}

}