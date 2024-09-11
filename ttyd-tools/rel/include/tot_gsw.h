
#pragma once

#include "evt_cmd.h"

#include <cstdint>

namespace mod::tot {

// GSW variables used for Tower of Trials-specific purposes.
enum GlobalWorkVars {
    // Used for tower progression, dialogue progression, etc.
    GSW_ToT_StoryProgression                = GSW(1000),
    GSW_Tower_ChestClaimed,
    GSW_Tower_DisplayChestIcons,
    GSW_Tower_ContinuingFromGameOver,
    GSW_Tower_TutorialClearAttempts,
    GSW_Tower_TutorialClears,
    GSW_Battle_AtomicBoo_BreathGuardCount,
    GSW_Hub_WelcomeKoopaCutsceneState,

    // Used for cosmetic choices that persist across runs.
    GSW_MarioCostume                        = GSW(1500),

    // Flags used for tower progression, dialogue progression, etc.
    GSWF_Chest_0                            = GSWF(6000),
    GSWF_Chest_1,
    GSWF_Chest_2,
    GSWF_Chest_3,
    GSWF_Lock,
    GSWF_Lobby_InConfirm,
    GSWF_Lobby_Confirmed,
    GSWF_Battle_Hooktail_BiteReactionSeen,
    GSWF_HubShopTutorial,
    GSWF_CosmeticShopTutorial,
    GSWF_RunOptionsTutorial,

    // Used for tracking things that persist across runs.
    GSWF_PeekabooEnabled                    = GSWF(7000),
    GSWF_SuperPeekabooEnabled,
    GSWF_TimingTutorEnabled,
    GSWF_BgmEnabled, 
    
    // Used for enabling cosmetic choices that persist across runs.
    GSWF_AttackFxFlags                      = GSWF(7500),
    GSWF_AttackFxFlags_End                  = GSWF_AttackFxFlags + 30,
    GSWF_YoshiColors                        = GSWF_AttackFxFlags_End,
    GSWF_YoshiColors_End                    = GSWF_YoshiColors + 30,
    GSWF_MarioColors                        = GSWF_YoshiColors_End,
    GSWF_MarioColors_End                    = GSWF_MarioColors + 30,
    // Used for queueing animations for achievement unlocking.
    GSWF_AchWinQueue                        = GSWF_MarioColors_End,
    GSWF_AchWinQueue_End                    = GSWF_AchWinQueue + 70,
    GSWF_AchUnlockQueue                     = GSWF_AchWinQueue_End,
    GSWF_AchUnlockQueue_End                 = GSWF_AchUnlockQueue + 70,
};

// Wrappers to ttyd::swdrv functions that handle base conversion.
int32_t GetSWByte(int32_t byte_id);
void SetSWByte(int32_t byte_id, int32_t value);
int32_t GetSWF(int32_t flag_id);
void SetSWF(int32_t flag_id, int32_t value = 1);
// Toggles on/off, and returns whether it finished in the 'on' state.
int32_t ToggleSWF(int32_t flag_id);

}  // namespace mod::tot