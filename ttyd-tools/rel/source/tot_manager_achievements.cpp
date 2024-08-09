#include "tot_manager_achievements.h"

#include "mod.h"
#include "tot_state.h"

#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/item_data.h>
#include <ttyd/pmario_sound.h>

namespace mod::tot {

namespace {

using ::ttyd::evtmgr_cmd::evtGetValue;

namespace ItemType = ttyd::item_data::ItemType;



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

// Queues for popping up "Achievement unlocked" dialogs and marking off
// completed achievements in the pause menu.
int8_t g_WinQueue[AchievementId::MAX_ACHIEVEMENT + 1] = { -1 };
int8_t g_MarkQueue[AchievementId::MAX_ACHIEVEMENT + 1] = { -1 };

}  // namespace

void AchievementsManager::Update() {
    // TODO: Check for new completions every frame, if needed.
    // TODO: Implement window dequeueing system.
}

void AchievementsManager::Draw() {
    // TODO: Implement, if needed.
}

const AchievementData* AchievementsManager::GetData(int32_t ach) {
    return &g_AchievementData[ach];
}

bool AchievementsManager::CheckCompleted(int32_t ach) {
    return g_Mod->state_.GetOption(FLAGS_ACHIEVEMENT, ach);
}

bool AchievementsManager::CheckOptionUnlocked(uint32_t option) {
    for (int32_t i = 0; i < AchievementId::MAX_ACHIEVEMENT; ++i) {
        if (g_AchievementData[i].reward_type == AchievementRewardType::OPTION &&
            static_cast<uint32_t>(g_AchievementData[i].reward_type) == option) {
            return g_Mod->state_.GetOption(FLAGS_OPTION_UNLOCKED, i);
        }
    }
    return false;
}

void AchievementsManager::MarkCompleted(int32_t ach) {
    // TODO: Move this to the window popup.
    ttyd::pmario_sound::psndSFXOn("SFX_MOBJ_BLOCK_POWER_SHINE1");

    if (!CheckCompleted(ach)) {
        g_Mod->state_.SetOption(FLAGS_ACHIEVEMENT, ach);
        if (g_AchievementData[ach].reward_type == AchievementRewardType::OPTION)
            g_Mod->state_.SetOption(FLAGS_OPTION_UNLOCKED, ach);
        
        for (int32_t i = 0; i <= AchievementId::MAX_ACHIEVEMENT; ++i) {
            if (g_WinQueue[i] == -1) {
                g_WinQueue[i] = ach;
                g_WinQueue[i + 1] = -1;
            }
        }
        for (int32_t i = 0; i <= AchievementId::MAX_ACHIEVEMENT; ++i) {
            if (g_MarkQueue[i] == -1) {
                g_MarkQueue[i] = ach;
                g_MarkQueue[i + 1] = -1;
            }
        }
    }
}

// Marks an achievement as complete.
EVT_DEFINE_USER_FUNC(evtTot_MarkCompleted) {
    int32_t ach = evtGetValue(evt, evt->evtArguments[0]);
    AchievementsManager::MarkCompleted(ach);
    return 2;
}
 
}