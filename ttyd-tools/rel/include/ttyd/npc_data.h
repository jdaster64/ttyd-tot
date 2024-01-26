#pragma once

#include "evtmgr.h"

#include <cstdint>
    
namespace ttyd::npcdrv {
struct NpcTribeDescription;
}

namespace ttyd::npc_data {
    
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
} __attribute__((__packed__));

static_assert(sizeof(NpcAiTypeTable) == 0x24);

extern "C" {

extern ttyd::npcdrv::NpcTribeDescription npcTribe[1];
extern NpcAiTypeTable npc_ai_type_table[57];

// npc_define_territory_type

}

}