#pragma once

#include "evtmgr.h"

#include <cstdint>
    
namespace ttyd::npcdrv {
struct NpcTribeDescription;
}

namespace ttyd::npc_data {

// Non-exhaustive; only ones that are considered for spawning in ToT.
namespace NpcTribeType {
    enum e {
        MERLON          = 0x10,
        CHET_RIPPO      = 0x11,
        DAZZLE          = 0x12,
        MERLUVLEE       = 0x13,
        MERLEE          = 0x14,
        WONKY           = 0x15,
        GRIFTY          = 0x16,
        CHARLIETON      = 0x17,
        HOWZ_SELLER     = 0x1a,
        LUMPY           = 0x35,
        DOOGAN_YELLOW   = 0x5f,
        MOVER           = 0x6b,
        GRUBBA          = 0x74,
        DOOPLISS        = 0x95,
    };
}
    
struct NpcAiTypeTable {
	const char* aiTypeName;
	uint32_t    flags;
    void*       initEvtCode;
    void*       moveEvtCode;
    void*       deadEvtCode;
    void*       findEvtCode;
    void*       lostEvtCode;
    void*       returnEvtCode;
    void*       blowEvtCode;
} ;

static_assert(sizeof(NpcAiTypeTable) == 0x24);

extern "C" {

extern ttyd::npcdrv::NpcTribeDescription npcTribe[328];
extern NpcAiTypeTable npc_ai_type_table[57];

// npc_define_territory_type

}

}