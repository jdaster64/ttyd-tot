#pragma once

#include <cstdint>

namespace mod::infinite_pit {

class AchievementsManager {
public:
    // Checks whether to award key items for completing various tasks.
    static void Update();
    // Code that runs drawing-related code every frame.
    static void Draw();
    
    // Gets current and max completion points for each task.
    static int32_t GetCurrentCompletionPoints(int32_t task_type);
    static int32_t GetMaxCompletionPoints(int32_t task_type);
    
    // Called when loading an old file; marks off corresponding P badges for
    // currently collected Mario badges if playing in Mario-alone mode.
    static void UpdatePartnerVariantBadgesCollected();
    
    // Item types to award for completing achievement tasks.
    static const int32_t kChestRewardItem;
    static const int32_t kBadgeLogItem;
    static const int32_t kTattleLogItem;
};
 
}