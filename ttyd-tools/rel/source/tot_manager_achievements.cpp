#include "tot_manager_achievements.h"

#include "tot_state.h"

#include <ttyd/item_data.h>

namespace mod::tot {

namespace {

namespace ItemType = ttyd::item_data::ItemType;

}  // namespace

const AchievementData g_AchievementData[] = {
    { "tot_achd_00", nullptr, AchievementRewardType::KEY_ITEM, ItemType::TOT_KEY_TIMING_TUTOR },
    { "tot_achd_01", nullptr, AchievementRewardType::OPTION, OPT_CHARLIETON_STOCK },
    { "tot_achd_02", nullptr, AchievementRewardType::OPTION, OPT_NPC_CHOICE_1 },
    { "tot_achd_03", nullptr, AchievementRewardType::OPTION, OPT_PARTNER },
    { "tot_achd_04", nullptr, AchievementRewardType::OPTION, OPT_AC_DIFFICULTY },
    { "tot_achd_05", nullptr, AchievementRewardType::KEY_ITEM, ItemType::TOT_KEY_SUPER_PEEKABOO },
    { "tot_achd_06", nullptr, AchievementRewardType::OPTION, OPTNUM_SUPERGUARD_SP_COST },
    { "tot_achd_07", nullptr, AchievementRewardType::ATTACK_FX, 6 },
    { "tot_achd_08", nullptr, AchievementRewardType::MARIO_COSTUME, 11 },
    { "tot_achd_09", nullptr, AchievementRewardType::KEY_ITEM, ItemType::TOT_KEY_PEEKABOO },
    { "tot_achd_10", nullptr, AchievementRewardType::ATTACK_FX, 8 },
    { "tot_achd_11", nullptr, AchievementRewardType::ATTACK_FX, 10 },
    { "tot_achd_12", nullptr, AchievementRewardType::KEY_ITEM, ItemType::TOT_KEY_BGM_TOGGLE },
    { "tot_achd_13", nullptr, AchievementRewardType::YOSHI_COSTUME, 7 },
    { "tot_achd_14", nullptr, AchievementRewardType::YOSHI_COSTUME, 6 },
    { "tot_achd_15", nullptr, AchievementRewardType::MARIO_COSTUME, 13 },
    { "tot_achd_16", nullptr, AchievementRewardType::MARIO_COSTUME, 12 },
    { "tot_achd_17", nullptr, AchievementRewardType::MARIO_COSTUME, 18 },
    { "tot_achd_18", nullptr, AchievementRewardType::HAMMER, 0 },
    { "tot_achd_19", nullptr, AchievementRewardType::ATTACK_FX, 7 },
    { "tot_achd_20", nullptr, AchievementRewardType::MARIO_COSTUME, 10 },
    { "tot_achd_21", nullptr, AchievementRewardType::YOSHI_COSTUME, 14 },
    { "tot_achd_22", nullptr, AchievementRewardType::HAMMER, 0 },
    { "tot_achd_23", nullptr, AchievementRewardType::OPTION, OPT_AUDIENCE_RANDOM_THROWS },
    { "tot_achd_24", nullptr, AchievementRewardType::OPTION, OPTVAL_INFINITE_BP },
    { "tot_achd_25", nullptr, AchievementRewardType::HAMMER, 0 },
    { "tot_achd_26", nullptr, AchievementRewardType::ATTACK_FX, 9 },
    { "tot_achd_27", nullptr, AchievementRewardType::MARIO_COSTUME, 16 },
    { "tot_achd_28", nullptr, AchievementRewardType::YOSHI_COSTUME, 19 },
    { "tot_achd_29", nullptr, AchievementRewardType::OPTION, OPT_RANDOM_DAMAGE },
    { "tot_achd_30", nullptr, AchievementRewardType::MARIO_COSTUME, 24 },
    { "tot_achd_31", nullptr, AchievementRewardType::MARIO_COSTUME, 25 },
    { "tot_achd_32", nullptr, AchievementRewardType::OPTION, OPT_REVIVE_PARTNERS },
    { "tot_achd_33", nullptr, AchievementRewardType::OPTION, OPT_BANDIT_ESCAPE },
    { "tot_achd_34", nullptr, AchievementRewardType::MARIO_COSTUME, 5 },
    { "tot_achd_35", nullptr, AchievementRewardType::MARIO_COSTUME, 6 },
    { "tot_achd_36", nullptr, AchievementRewardType::MARIO_COSTUME, 7 },
    { "tot_achd_37", nullptr, AchievementRewardType::YOSHI_COSTUME, 15 },
    { "tot_achd_38", nullptr, AchievementRewardType::YOSHI_COSTUME, 9 },
    { "tot_achd_39", nullptr, AchievementRewardType::ATTACK_FX, 12 },
    { "tot_achd_40", nullptr, AchievementRewardType::MARIO_COSTUME, 8 },
    { "tot_achd_41", nullptr, AchievementRewardType::HAMMER, 0 },
    { "tot_achd_42", nullptr, AchievementRewardType::OPTION, OPT_INVENTORY_SACK_SIZE },
    { "tot_achd_43", nullptr, AchievementRewardType::MARIO_COSTUME, 15 },
    { "tot_achd_44", nullptr, AchievementRewardType::MARIO_COSTUME, 17 },
    { "tot_achd_45", nullptr, AchievementRewardType::YOSHI_COSTUME, 10 },
    { "tot_achd_46", nullptr, AchievementRewardType::YOSHI_COSTUME, 8 },
    { "tot_achd_47", nullptr, AchievementRewardType::ATTACK_FX, 11 },
    { "tot_achd_48", nullptr, AchievementRewardType::MARIO_COSTUME, 9 },
    { "tot_achd_49", nullptr, AchievementRewardType::ATTACK_FX, 13 },
    { "tot_achd_50", nullptr, AchievementRewardType::MARIO_COSTUME, 20 },
    { "tot_achd_51", nullptr, AchievementRewardType::MARIO_COSTUME, 14 },
    { "tot_achd_52", nullptr, AchievementRewardType::YOSHI_COSTUME, 16 },
    { "tot_achd_53", nullptr, AchievementRewardType::ATTACK_FX, 14 },
    { "tot_achd_54", nullptr, AchievementRewardType::OPTION, OPTVAL_STARTER_ITEMS_CUSTOM },
    { "tot_achd_55", nullptr, AchievementRewardType::MARIO_COSTUME, 21 },
    { "tot_achd_56", nullptr, AchievementRewardType::OPTION, OPT_OBFUSCATE_ITEMS },
    { "tot_achd_57", nullptr, AchievementRewardType::YOSHI_COSTUME, 13 },
    { "tot_achd_58", nullptr, AchievementRewardType::MARIO_COSTUME, 19 },
    { "tot_achd_59", nullptr, AchievementRewardType::ATTACK_FX, 15 },
    { "tot_achd_60", nullptr, AchievementRewardType::YOSHI_COSTUME, 12 },
    { "tot_achd_61", nullptr, AchievementRewardType::YOSHI_COSTUME, 11 },
    { "tot_achd_62", nullptr, AchievementRewardType::OPTION, OPT_SECRET_BOSS },
    { "tot_achd_63", nullptr, AchievementRewardType::HAMMER, 0 },
    { "tot_achd_64", nullptr, AchievementRewardType::YOSHI_COSTUME, 18 },
    { "tot_achd_65", nullptr, AchievementRewardType::YOSHI_COSTUME, 20 },
    { "tot_achd_66", nullptr, AchievementRewardType::MARIO_COSTUME, 22 },
    { "tot_achd_67", nullptr, AchievementRewardType::MARIO_COSTUME, 23 },
    { "tot_achd_68", nullptr, AchievementRewardType::MARIO_COSTUME, 26 },
    { "tot_achd_69", nullptr, AchievementRewardType::YOSHI_COSTUME, 17 },
};

void AchievementsManager::Update() {
    // TODO: Implement.
}

void AchievementsManager::Draw() {
    // TODO: Implement.
}

const AchievementData* AchievementsManager::GetData(int32_t idx) {
    return &g_AchievementData[idx];
}
 
}