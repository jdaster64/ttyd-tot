#pragma once

#include "evt_cmd.h"

#include <ttyd/evtmgr.h>

#include <cstdint>

namespace mod::tot {

namespace AchievementId {
    enum e {
        AGG_STYLISH_20 = 0,
        AGG_BUY_ITEMS_50,
        AGG_NPC_DEALS_10,
        AGG_ALL_PARTNERS,
        AGG_USE_LV_3_MOVES_10,
        AGG_MIDBOSS_TYPES_50,
        AGG_SUPERGUARD_100,
        AGG_COINS_10000,
        AGG_DAMAGE_50000,

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
        RUN_DOUBLE_HP_ATK,
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
        META_ITEMS_BADGES_10,
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
        SECRET_INFATUATE,
        SECRET_ZERO_STATS_3,
        SECRET_DAMAGE,

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
    const char* reward_msg;     // Falls back to CosmeticData.name_msg if null.
    int32_t reward_type;
    uint32_t reward_id;         // For option type / indexing into CosmeticData.
};

class AchievementsManager {
public:
    // Code that runs every frame.
    static void Update();
    // Code that runs drawing-related code every frame.
    static void Draw();

    // Gets achievement data.
    static const AchievementData* GetData(int32_t ach);

    // Checks whether an option is unlocked.
    static bool CheckOptionUnlocked(uint32_t option);
    // Marks off an achievement.
    static void MarkCompleted(int32_t ach);
    // Checks whether an achievement should be newly met, and marks it if so.
    static void CheckCompleted(int32_t ach);
    // Gets the progress for an achievement.
    static bool GetProgress(int32_t ach, int32_t* done, int32_t* total);
};

// Marks an achievement as complete.
EVT_DECLARE_USER_FUNC(evtTot_MarkCompletedAchievement, 1)
// Checks whether an achievement should be newly met, and marks it if so.
EVT_DECLARE_USER_FUNC(evtTot_CheckCompletedAchievement, 1)
 
}