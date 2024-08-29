#pragma once

#include <cstdint>

namespace mod::tot {

// Tracks save file completion.
class ProgressManager {
public:
    // Forces the next poll for overall progression to be recalculated.
    static void RefreshCache();

    // Returns a progression score out of 10000.
    // Also caches all other stats until the next time RefreshStats() is called.
    static int32_t GetOverallProgression();

    // Can return binary completions or a progression score out of 10000.
    static int32_t GetMoveLogProgress(int32_t* obt, int32_t* com, int32_t* tot);

    static void GetItemLogProgress(int32_t* cur, int32_t* tot);
    static void GetBadgeLogProgress(int32_t* cur, int32_t* tot);
    static void GetTattleLogProgress(int32_t* cur, int32_t* tot);
    static void GetAchievementLogProgress(int32_t* cur, int32_t* tot);

    // Returns a progression score out of 10000.
    static int32_t GetOverallHubProgression();

    static void GetKeyItemProgress(int32_t* cur, int32_t* tot);
    static void GetHubItemProgress(int32_t* cur, int32_t* tot);
    static void GetHubBadgeProgress(int32_t* cur, int32_t* tot);
    static void GetHubAttackFXProgress(int32_t* cur, int32_t* tot);
    static void GetHubMarioCostumeProgress(int32_t* cur, int32_t* tot);
    static void GetHubYoshiCostumeProgress(int32_t* cur, int32_t* tot);
};
 
}