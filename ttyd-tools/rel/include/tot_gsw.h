
#pragma once

#include "evt_cmd.h"

#include <cstdint>

namespace mod::tot {

// GSW variables used for Tower of Trials-specific purposes.
enum GlobalWorkVars {
    // Used for tower + overall progression.
    GSW_ToT_StoryProgression                = GSW(1000),
    GSW_Tower_ChestClaimed,
    GSW_Tower_DisplayChestIcons,
    GSW_Tower_ContinuingFromGameOver,
    GSW_Battle_AtomicBoo_BreathGuardCount,
    GSW_Battle_Hooktail_BiteReactionSeen,

    // Used for tracking things that persist across runs.
    GSW_Enemy_Defeated_Count                = GSW(1500),
    GSW_Enemy_Defeated_Count_End            = GSW_Enemy_Defeated_Count + 110*2,

    // Flags used for tower progression.
    GSWF_Chest_0                            = GSWF(6000),
    GSWF_Chest_1,
    GSWF_Chest_2,
    GSWF_Chest_3,
    GSWF_Lock,
    GSWF_Lobby_InConfirm,
    GSWF_Lobby_Confirmed,

    // Used for tracking things that persist across runs.
    GSWF_PeekabooEnabled                    = GSWF(7000),
    GSWF_SuperPeekabooEnabled,
    GSWF_TimingTutorEnabled,
    GSWF_BgmEnabled,
    
    // Used for enabling cosmetic options that persist across runs.
    GSWF_AttackFxFlags                      = GSWF(7500),
    GSWF_AttackFxFlags_End                  = GSWF_AttackFxFlags + 30,
    GSWF_YoshiColors                        = GSWF_AttackFxFlags_End,
    GSWF_YoshiColors_End                    = GSWF_YoshiColors + 16,
};

// Wrappers to ttyd::swdrv functions that handle base conversion.
uint32_t GetSWF(int32_t flag_id);
uint32_t GetSWByte(int32_t flag_id);
void SetSWF(int32_t flag_id, int32_t value = 1);
uint32_t ToggleSWF(int32_t flag_id);

}  // namespace mod::tot