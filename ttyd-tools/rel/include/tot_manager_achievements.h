#pragma once

#include <cstdint>

namespace mod::tot {

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
    static const AchievementData* GetData(int32_t idx);
};
 
}