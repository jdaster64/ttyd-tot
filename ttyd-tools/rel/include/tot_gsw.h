
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
    GSW_NpcA_SpecialConversation,
    GSW_NpcF_CurrentConversation,
    GSW_Battle_Multihit_GuardCount,
    GSW_CountdownTimerState,
    GSW_Battle_DooplissMove,
    GSW_Tower_RevealUsedThisFloor,
    GSW_Tower_NpcTalkedThisFloor,
    GSW_Tower_FinalBossType,
    GSW_Tower_FinalBossSpecialDialogue,
    GSW_NpcG_CurrentStatBreakdown,
    GSW_Boss_Target1_Unit,
    GSW_Boss_Target1_Part,
    GSW_Boss_Target2_Unit,
    GSW_Boss_Target2_Part,

    // Used for cosmetic choices that persist across runs.
    GSW_MarioCostume                        = GSW(1500),

    // Flags used for tower progression, dialogue progression, etc.
    GSWF_Chest_0                            = GSWF(6000),
    GSWF_Chest_1,
    GSWF_Chest_2,
    GSWF_Chest_3,
    GSWF_Lock,
    GSWF_RunSettingsCleared,
    GSWF_Lobby_InConfirm,
    GSWF_Lobby_Confirmed,
    // Tutorial flags.
    GSWF_HubShopTutorial,
    GSWF_CosmeticShopTutorial,
    GSWF_RunOptionsTutorial,
    GSWF_Battle_Hooktail_BiteReactionSeen,
    // Flag for having chatted with NPC D for the first time.
    GSWF_NpcD_FirstTimeChat,
    // Flags for unique Bub-ulber conversations seen.
    GSWF_NpcF_Flags,
    GSWF_NpcF_Flags_End                     = GSWF_NpcF_Flags + 30,
    GSWF_NpcF_FirstTalk                     = GSWF_NpcF_Flags_End,
    GSWF_NpcF_SeedUnlocked,
    // Flag for having chatted with NPC G post-tutorial clears.
    GSWF_NpcG_PostTutorialChat,
    // Flag for having chatted with NPC I post-tutorial clears.
    GSWF_NpcI_PostTutorialChat,
    // Flag for having chatted with NPC K for the first time.
    GSWF_NpcK_FirstTimeChat,
    // Flag for having chatted with Mayor Kroop post-tutorial clears.
    GSWF_MayorKroop_PostTutorialChat,
    // Flag for having chatted with the gatekeeper for the first time.
    GSWF_Gatekeeper_FirstTimeChat,
    // Conversation flags for beating bosses for the first time.
    GSWF_Hooktail_FirstTimeChat,
    GSWF_Gloomtail_FirstTimeChat,
    GSWF_Bonetail_FirstTimeChat,
    // Flag for having chatted with the innkeeper for the first time.
    GSWF_Innkeeper_FirstTimeChat,
    // Flag for option unlock conversation with move enthusiast.
    GSWF_NpcK_CustomMovesUnlocked,
    // Flags for first + option unlock conversation with General White.
    GSWF_White_FirstTimeChat,
    GSWF_White_CountdownUnlocked,
    // Flags for beating each of the secret bosses.
    GSWF_SecretBoss1_Beaten,
    GSWF_SecretBoss2_Beaten,
    GSWF_SecretBoss3_Beaten,
    GSWF_RegularBoss_BeatenEX,
    GSWF_SecretBoss1_BeatenEX,
    GSWF_SecretBoss2_BeatenEX,
    GSWF_SecretBoss3_BeatenEX,
    // Flags for talking to NPC G about reward breakdowns.
    GSWF_NpcG_UnlockedRewardStats,
    GSWF_NpcG_TalkedThisVisit,
    // Flag for enabling NPC to save custom presets.
    GSWF_NpcPreset_Enabled,
    GSWF_NpcPreset_FirstTimeChat,

    // Used for tracking things that persist across runs.
    GSWF_PeekabooEnabled                    = GSWF(7000),
    GSWF_SuperPeekabooEnabled,
    GSWF_TimingTutorEnabled,
    GSWF_BgmEnabled, 
    
    // Used for enabling cosmetic choices that persist across runs.
    GSWF_AttackFxFlags                      = GSWF(7500),
    GSWF_AttackFxFlags_End                  = GSWF_AttackFxFlags + 30,
    GSWF_YoshiColors                        = GSWF_AttackFxFlags_End,
    GSWF_YoshiColors_End                    = GSWF_YoshiColors + 60,
    // Used for queueing animations for achievement unlocking.
    GSWF_AchWinQueue                        = GSWF_YoshiColors_End,
    GSWF_AchWinQueue_End                    = GSWF_AchWinQueue + 128,
    GSWF_AchUnlockQueue                     = GSWF_AchWinQueue_End,
    GSWF_AchUnlockQueue_End                 = GSWF_AchUnlockQueue + 128,
};

// Wrappers to ttyd::swdrv functions that handle base conversion.
int32_t GetSWByte(int32_t byte_id);
void SetSWByte(int32_t byte_id, int32_t value);
int32_t GetSWF(int32_t flag_id);
void SetSWF(int32_t flag_id, int32_t value = 1);
// Toggles on/off, and returns whether it finished in the 'on' state.
int32_t ToggleSWF(int32_t flag_id);

}  // namespace mod::tot