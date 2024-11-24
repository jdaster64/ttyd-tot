#pragma once

#include "evt_cmd.h"

#include <ttyd/evtmgr.h>

#include <cstdint>

namespace mod::tot {

namespace AchievementId {
    enum e {
        AGG_STYLISH_15 = 0,
        AGG_BUY_ITEMS_25,
        AGG_NPC_DEALS_10,
        AGG_ALL_PARTNERS,
        AGG_USE_LV_3_MOVES_10,
        AGG_MIDBOSS_TYPES_30,
        AGG_SUPERGUARD_100,
        AGG_COINS_10000,
        AGG_DAMAGE_15000,

        RUN_HALF_FIRST,
        RUN_HALF_SPEED1,
        RUN_HALF_SPEED2,
        RUN_FULL_FIRST,
        RUN_FULL_SPEED1,
        RUN_FULL_SPEED2,
        RUN_EX_FIRST,
        RUN_EX_SPEED1,
        RUN_EX_SPEED2,
        RUN_NPC_DEALS_7,
        RUN_MAX_AC_DIFFICULTY,
        RUN_NO_PARTNERS,
        RUN_ALL_PARTNERS,
        RUN_NO_JUMP_HAMMER,
        RUN_NO_ITEMS,
        RUN_NO_BADGES,
        RUN_ALL_MOVES_MAXED,
        RUN_ALL_CONDITIONS_MET,
        RUN_ALL_FLOORS_3_TURN,
        RUN_NO_DAMAGE,
        RUN_HIGH_INTENSITY,
        RUN_ZERO_STAT_1,
        RUN_ZERO_STAT_2,
        
        MISC_FAINTED_PARTNER,
        MISC_BANDIT_STEAL,
        MISC_BOMB_SQUAD_FIRE,
        MISC_FROZEN_20,
        MISC_POISON_50,
        MISC_SHRUNK_OHKO,
        MISC_THUNDER_STORM_GREAT,
        MISC_MEGATON_BOMB,
        MISC_TRADE_OFF_BOSS,
        MISC_SUPERGUARD_BITE,
        MISC_CHARLIETON_OUT_OF_STOCK,
        MISC_DAZZLE_100_COINS,
        MISC_CHET_RIPPO_SELL_ALL,
        MISC_LUMPY_DOUBLE_2,
        MISC_KEY_SKIP_CHEST,
        MISC_ZESS_SIGNATURE,
        MISC_ZESS_POINT_SWAP,
        MISC_SHINES_10,
        MISC_RUN_COINS_999,

        META_COSMETICS_5,
        META_ALL_OPTIONS,
        META_ALL_KEY_ITEMS,
        META_ITEMS_BADGES_5,
        META_ITEMS_BADGES_ALL,
        META_ITEM_LOG_BASIC,
        META_ITEM_LOG_ALL,
        META_BADGE_LOG_ALL,
        META_TATTLE_LOG_BASIC,
        META_TATTLE_LOG_ALL,
        META_MOVE_LOG_ALL,
        META_SECRET_BOSS,
        META_RUNS_10,
        META_RUNS_25,
        META_ALL_ACHIEVEMENTS,

        SECRET_COINS,
        RUN_INFATUATE,
        SECRET_ZERO_STATS_3,
        SECRET_DAMAGE,

        // Added in 2.0.
        V2_AGG_USE_ALL_MOVES,
        V2_RUN_BASE_MOVES_ONLY,
        V2_AGG_RUN_AWAY_50,
        V2_MISC_STATUS_5,
        V2_MISC_ALLERGIC,
        V2_SECRET_RUN_HAMMERMAN,
        V2_MISC_BATTLE_COINS_100,
        V2_MISC_SP_TURN1,
        V2_RUN_COUNTDOWN_40,
        V2_AGG_ENEMY_TIMES_100,

        MAX_ACHIEVEMENT,
    };
}

namespace AchievementRewardType {
    enum e {
        OPTION = 0,
        KEY_ITEM,
        ATTACK_FX,
        MARIO_COSTUME,
        YOSHI_COSTUME,
        HAMMER,

        MAX_TYPE,
    };
}

struct AchievementData {
    const char* help_msg;
    const char* reward_msg;     // Falls back to CosmeticGroupData.name_msg if null.
    int16_t grid_index;
    int16_t reward_type;
    uint32_t reward_id;         // For option type / indexing into CosmeticGroupData.
};

class AchievementsManager {
public:
    // Code that runs every frame.
    static void Update();
    // Code that runs drawing-related code every frame.
    static void Draw();

    // Gets achievement data.
    static const AchievementData* GetData(int32_t ach);
    
    // Returns whether achievement is "secret" (not required for 100%).
    static bool IsSecret(int32_t ach);

    // Marks off an achievement.
    static void MarkCompleted(int32_t ach);
    // Grants the corresponding option / key item when animation plays.
    static void UnlockReward(int32_t ach);
    // Checks whether an achievement should be newly met, and marks it if so.
    static void CheckCompleted(int32_t ach);

    // Checks whether a run option is unlocked.
    static bool CheckOptionUnlocked(uint32_t option);
    // Checks whether a cosmetic group is unlocked (not necessarily purchased).
    static bool CheckCosmeticGroupUnlocked(int32_t reward_type, int32_t group_id);

    // Returns a pointer with a grid of achievement ids.
    static void GetAchievementGrid(int8_t const ** arr);
    // Returns an pointer to an array of achievement states (2 = completed, 1 = visible).
    // If suppress_queued is true, doesn't treat queued unlocks as completed.
    // Returns the earliest id of an achievement queued to unlock, if any.
    static int32_t GetAchievementStates(int8_t const ** arr, bool suppress_queued = true);
};

// Marks an achievement as complete.
EVT_DECLARE_USER_FUNC(evtTot_MarkCompletedAchievement, 1)
// Checks for marking off an achievement, and returns whether it was met.
// arg0 = achievement id
// out arg1 = whether the achievement is completed.
// out arg2 = whether the achievement was just completed for the first time.
EVT_DECLARE_USER_FUNC(evtTot_CheckCompletedAchievement, 3)
 
}