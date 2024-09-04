#pragma once

#include "evtmgr.h"

#include <cstdint>
    
namespace ttyd::npcdrv {
struct NpcTribeDescription;
}

namespace ttyd::npc_data {

namespace NpcAiType {
    enum e {
        NONE = 0,
        GENERAL_IMMOVABLE,
        UNK_EVENTS,
        TEST_ENEMY,
        GOOMBA,
        SPIKY_GOOMBA,
        PARAGOOMBA,
        KOOPA_TROOPA,
        PARATROOPA,
        BUZZY,
        BUZZY_CEILING,
        SPIKE_TOP,
        SPIKE_TOP_CEILING,
        PARABUZZY,
        DULL_BONES,
        DRY_BONES,
        FUZZY,
        GOLD_FUZZY,
        BILL_BLASTER,
        BULLET_BILL,
        POKEY,
        BALD_CLEFT,
        CLEFT,
        BRISTLE,
        X_NAUT,  // Uses Goomba events
        YUX,
        X_YUX,
        PIDER,
        PIRANHA_PLANT,
        PUFF,
        BANDIT,  // Uses Goomba events
        SWOOPER_CEILING,
        SWOOPER_FLYING,
        BOO,
        CRAZEE_DAYZEE,
        AMAZY_DAYZEE,
        EMBER,
        BULKY_BOB_OMB,
        WIZZERD,
        X_NAUT_PHD,
        SPINIA,
        MAGIKOOPA_FLYING,
        MAGIKOOPA_GROUNDED,
        HAMMER_BRO,
        CHAIN_CHOMP,
        KOOPATROL,
        _2D_GENERAL_IMMOVABLE,
        _2D_GOOMBA,
        _2D_X_NAUT,
        _2D_X_NAUT_PIPE,
        _2D_X_NAUT_PIPE_GENERATOR,
        _2D_X_NAUT_DESCENDING,
        _2D_X_NAUT_AQUATIC,
        _2D_BLOOPER,
        _2D_X_NAUT_PHD,
        UNK_EVENTS_2,
    };
}

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
        HOOKTAIL        = 0x21,
        LUMPY           = 0x35,
        ZESS_T          = 0x51,
        DOOGAN_YELLOW   = 0x5f,
        MOVER           = 0x6b,
        GRUBBA          = 0x74,
        TOAD_SISTER_R   = 0x79,
        TOAD_SISTER_P   = 0x7a,
        TOAD_SISTER_G   = 0x7b,
        DOOPLISS        = 0x95,
        GLOOMTAIL       = 0xc6,
        PIDER           = 0x10a,
        ARANTULA        = 0x10b,
        BONETAIL        = 0x145,
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