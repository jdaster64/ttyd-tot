#include "mod_achievements.h"

#include "common_functions.h"
#include "common_ui.h"
#include "custom_enemy.h"
#include "custom_item.h"
#include "mod.h"
#include "mod_state.h"

#include <ttyd/battle_database_common.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/msgdrv.h>
#include <ttyd/sound.h>
#include <ttyd/swdrv.h>
#include <ttyd/system.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::infinite_pit {
    
namespace {
    
using ::ttyd::mario_pouch::PouchData;
    
namespace BattleUnitType = ::ttyd::battle_database_common::BattleUnitType;
namespace ItemType = ::ttyd::item_data::ItemType;

// Variables used to track achievement progress.
// TODO: Make cleaner if more achievements are ever added.
int32_t g_CurrentTaskPoints[]           = { 0, 0, 0 };
int32_t g_MaxTaskPoints[]               = { 0, 0, 0 };

// Variables used for drawing "Achievement unlocked" text.
const int32_t kDisplayItemMaxFrames     = 360;
const int32_t kDisplayItemFadeoutStart  = 40;
int32_t g_DisplayItem                   = 0;
int32_t g_DisplayFrameCount             = -1;

void GetAchievement(int32_t item_type) {
    if (!ttyd::mario_pouch::pouchCheckItem(item_type)) {
        ttyd::sound::SoundEfxPlayEx(0x265, 0, 0x64, 0x40);
        ttyd::mario_pouch::pouchGetItem(item_type);
        g_DisplayFrameCount = kDisplayItemMaxFrames;
        g_DisplayItem = item_type;
    }
}

}

// Definitions for class constants.
const int32_t AchievementsManager::kChestRewardItem   = 0x52;
const int32_t AchievementsManager::kBadgeLogItem      = 0x53;
const int32_t AchievementsManager::kTattleLogItem     = 0x54;

void AchievementsManager::Update() {
    StateManager_v2& state = g_Mod->state_;
    
    // Remove awarded items if debug mode was used at any point.
    if (state.GetOptionNumericValue(OPT_DEBUG_MODE_USED)) {
        ttyd::mario_pouch::pouchRemoveItem(kChestRewardItem);
        ttyd::mario_pouch::pouchRemoveItem(kBadgeLogItem);
        ttyd::mario_pouch::pouchRemoveItem(kTattleLogItem);
        return;
    }
    
    // Check to see if the player has received all unique chest rewards.
    int32_t num_rewards = CountSetBits(state.reward_flags_) - 2;
    for (int32_t i = 0; i < 8; ++i) {
        num_rewards += state.GetStarPowerLevel(i);
    }
    const int32_t total_rewards =
        state.CheckOptionValue(OPTVAL_PARTNERS_NEVER) ? 39 : 46;
    if (num_rewards == total_rewards) GetAchievement(kChestRewardItem);
    g_CurrentTaskPoints[0] = num_rewards;
    g_MaxTaskPoints[0] = total_rewards;
    
    // Check to see if the player has completed the badge log.
    int32_t num_badges = 0;
    for (int32_t i = ItemType::POWER_JUMP; i < ItemType::MAX_ITEM_TYPE; ++i) {
        // Ignore the Attack FX badges, since those are optional.
        if (i >= ItemType::ATTACK_FX_R && i <= ItemType::ATTACK_FX_P) continue;
        num_badges += ttyd::swdrv::swGet(0x80 + i - ItemType::POWER_JUMP);
    }
    if (num_badges == 80) GetAchievement(kBadgeLogItem);
    g_CurrentTaskPoints[1] = num_badges;
    g_MaxTaskPoints[1] = 80;
    
    // Check to see if the player has completed the Tattle log.
    int32_t num_tattles = 0;
    for (int32_t i = 0; i <= BattleUnitType::BONETAIL; ++i) {
        // Only check for enemies included in the Tattle log.
        if (GetCustomTattleIndex(i) < 0) continue;
        num_tattles += ttyd::swdrv::swGet(0x117a + i);
    }
    if (num_tattles == 94) GetAchievement(kTattleLogItem);
    g_CurrentTaskPoints[2] = num_tattles;
    g_MaxTaskPoints[2] = 94;
}

void AchievementsManager::Draw() {    
    if (InMainGameModes() && g_DisplayFrameCount > 0) {
        uint32_t alpha = 0xffU;
        if (g_DisplayFrameCount < kDisplayItemFadeoutStart) {
            alpha = 0xff * g_DisplayFrameCount / kDisplayItemFadeoutStart;
        }
        char buf[64];
        sprintf(
            buf, "Achievement unlocked: \"%s\"",
            ttyd::msgdrv::msgSearch(
                ttyd::item_data::itemDataTable[g_DisplayItem].name));
        DrawText(buf, -260, -176, alpha, true, ~0U, 0.75f, /* center-left */ 3);
        
        --g_DisplayFrameCount;
    }
    // Start fadeout early if pause menu is opened.
    if (InPauseMenu() && g_DisplayFrameCount > kDisplayItemFadeoutStart) {
        g_DisplayFrameCount = kDisplayItemFadeoutStart;
    }
}

int32_t AchievementsManager::GetCurrentCompletionPoints(int32_t task_type) {
    switch (task_type) {
        case kChestRewardItem:  return g_CurrentTaskPoints[0];
        case kBadgeLogItem:     return g_CurrentTaskPoints[1];
        case kTattleLogItem:    return g_CurrentTaskPoints[2];
        default:                return 0;
    }
}

int32_t AchievementsManager::GetMaxCompletionPoints(int32_t task_type) {
    switch (task_type) {
        case kChestRewardItem:  return g_MaxTaskPoints[0];
        case kBadgeLogItem:     return g_MaxTaskPoints[1];
        case kTattleLogItem:    return g_MaxTaskPoints[2];
        default:                return -1;
    }
}

void AchievementsManager::UpdatePartnerVariantBadgesCollected() {
    // Only mark P badges collected if playing without partners.
    if (!g_Mod->state_.CheckOptionValue(OPTVAL_PARTNERS_NEVER) ||
        g_Mod->state_.GetOptionNumericValue(OPT_FIRST_PARTNER)) return;
    
    for (int32_t i = ItemType::POWER_JUMP; i < ItemType::MAX_ITEM_TYPE; ++i) {
        bool collected = ttyd::swdrv::swGet(0x80 + i - ItemType::POWER_JUMP);
        if (collected && IsStackableMarioBadge(i)) {
            ttyd::swdrv::swSet(0x81 + i - ItemType::POWER_JUMP);
        }
    }
}

}