#include "tot_manager_achievements.h"

#include "common_functions.h"
#include "mod.h"
#include "tot_gsw.h"
#include "tot_state.h"

#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/pmario_sound.h>
#include <ttyd/swdrv.h>

namespace mod::tot {

namespace {

using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

namespace ItemType = ttyd::item_data::ItemType;

const AchievementData g_AchievementData[] = {
    { "tot_achd_00", nullptr, AchievementRewardType::KEY_ITEM, ItemType::TOT_KEY_TIMING_TUTOR },
    { "tot_achd_01", "tot_acho_charlieton", AchievementRewardType::OPTION, OPT_CHARLIETON_STOCK },
    { "tot_achd_02", "tot_acho_npcchoice", AchievementRewardType::OPTION, OPT_NPC_CHOICE_1 },
    { "tot_achd_03", "tot_acho_partner", AchievementRewardType::OPTION, OPT_PARTNER },
    { "tot_achd_04", "tot_acho_acdiff", AchievementRewardType::OPTION, OPT_AC_DIFFICULTY },
    { "tot_achd_05", nullptr, AchievementRewardType::KEY_ITEM, ItemType::TOT_KEY_SUPER_PEEKABOO },
    { "tot_achd_06", "tot_acho_superguardcost", AchievementRewardType::OPTION, OPTNUM_SUPERGUARD_SP_COST },
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
    { "tot_achd_23", "tot_acho_audiencethrow", AchievementRewardType::OPTION, OPT_AUDIENCE_RANDOM_THROWS },
    { "tot_achd_24", "tot_acho_infinitebp", AchievementRewardType::OPTION, OPTVAL_INFINITE_BP },
    { "tot_achd_25", nullptr, AchievementRewardType::HAMMER, 0 },
    { "tot_achd_26", nullptr, AchievementRewardType::ATTACK_FX, 9 },
    { "tot_achd_27", nullptr, AchievementRewardType::MARIO_COSTUME, 16 },
    { "tot_achd_28", nullptr, AchievementRewardType::YOSHI_COSTUME, 19 },
    { "tot_achd_29", "tot_acho_randomdamage", AchievementRewardType::OPTION, OPT_RANDOM_DAMAGE },
    { "tot_achd_30", nullptr, AchievementRewardType::MARIO_COSTUME, 24 },
    { "tot_achd_31", nullptr, AchievementRewardType::MARIO_COSTUME, 25 },
    { "tot_achd_32", "tot_acho_revive", AchievementRewardType::OPTION, OPT_REVIVE_PARTNERS },
    { "tot_achd_33", "tot_acho_bandit", AchievementRewardType::OPTION, OPT_BANDIT_ESCAPE },
    { "tot_achd_34", nullptr, AchievementRewardType::MARIO_COSTUME, 5 },
    { "tot_achd_35", nullptr, AchievementRewardType::MARIO_COSTUME, 6 },
    { "tot_achd_36", nullptr, AchievementRewardType::MARIO_COSTUME, 7 },
    { "tot_achd_37", nullptr, AchievementRewardType::YOSHI_COSTUME, 15 },
    { "tot_achd_38", nullptr, AchievementRewardType::YOSHI_COSTUME, 9 },
    { "tot_achd_39", nullptr, AchievementRewardType::ATTACK_FX, 12 },
    { "tot_achd_40", nullptr, AchievementRewardType::MARIO_COSTUME, 8 },
    { "tot_achd_41", nullptr, AchievementRewardType::HAMMER, 0 },
    { "tot_achd_42", "tot_acho_invincrease", AchievementRewardType::OPTION, OPT_INVENTORY_SACK_SIZE },
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
    { "tot_achd_54", "tot_acho_customloadout", AchievementRewardType::OPTION, OPTVAL_STARTER_ITEMS_CUSTOM },
    { "tot_achd_55", nullptr, AchievementRewardType::MARIO_COSTUME, 21 },
    { "tot_achd_56", "tot_acho_obfuscated", AchievementRewardType::OPTION, OPT_OBFUSCATE_ITEMS },
    { "tot_achd_57", nullptr, AchievementRewardType::YOSHI_COSTUME, 13 },
    { "tot_achd_58", nullptr, AchievementRewardType::MARIO_COSTUME, 19 },
    { "tot_achd_59", nullptr, AchievementRewardType::ATTACK_FX, 15 },
    { "tot_achd_60", nullptr, AchievementRewardType::YOSHI_COSTUME, 12 },
    { "tot_achd_61", nullptr, AchievementRewardType::YOSHI_COSTUME, 11 },
    { "tot_achd_62", "tot_acho_secretboss", AchievementRewardType::OPTION, OPT_SECRET_BOSS },
    { "tot_achd_63", nullptr, AchievementRewardType::HAMMER, 0 },
    { "tot_achd_64", nullptr, AchievementRewardType::YOSHI_COSTUME, 18 },
    { "tot_achd_65", nullptr, AchievementRewardType::YOSHI_COSTUME, 20 },
    { "tot_achd_66", nullptr, AchievementRewardType::MARIO_COSTUME, 22 },
    { "tot_achd_67", nullptr, AchievementRewardType::MARIO_COSTUME, 23 },
    { "tot_achd_68", nullptr, AchievementRewardType::MARIO_COSTUME, 26 },
    { "tot_achd_69", nullptr, AchievementRewardType::YOSHI_COSTUME, 17 },
};

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

bool AchievementsManager::CheckCosmeticGroupUnlocked(
    int32_t reward_type, int32_t group_id) {
        
    for (int32_t i = 0; i < AchievementId::MAX_ACHIEVEMENT; ++i) {
        if (g_AchievementData[i].reward_type == reward_type &&
            static_cast<int32_t>(g_AchievementData[i].reward_id) == group_id) {
            return true;
        }
    }
    return false;
}

bool AchievementsManager::CheckOptionUnlocked(uint32_t option) {
    for (int32_t i = 0; i < AchievementId::MAX_ACHIEVEMENT; ++i) {
        if (g_AchievementData[i].reward_type == AchievementRewardType::OPTION &&
            g_AchievementData[i].reward_id == option) {
            return g_Mod->state_.GetOption(FLAGS_OPTION_UNLOCKED, i);
        }
    }
    return false;
}

void AchievementsManager::MarkCompleted(int32_t ach) {
    if (!g_Mod->state_.GetOption(FLAGS_ACHIEVEMENT, ach)) {
        g_Mod->state_.SetOption(FLAGS_ACHIEVEMENT, ach);

        // Queue animations for unlocking.
        SetSWF(GSWF_AchWinQueue + ach);
        SetSWF(GSWF_AchUnlockQueue + ach);

        // TODO: Move sound effect to the window popup, if creating one.
        ttyd::pmario_sound::psndSFXOn("SFX_MOBJ_BLOCK_POWER_SHINE1");

        // For options, automatically unlock their reward (no purchase needed).
        switch (g_AchievementData[ach].reward_type) {
            case AchievementRewardType::OPTION: {
                g_Mod->state_.SetOption(FLAGS_OPTION_UNLOCKED, ach);
                // Check to see if all extra options are unlocked.
                CheckCompleted(AchievementId::META_ALL_OPTIONS);
                break;
            }
        }
    }

    if (!g_Mod->state_.GetOption(
        FLAGS_ACHIEVEMENT, AchievementId::META_ALL_ACHIEVEMENTS)) {
        CheckCompleted(AchievementId::META_ALL_ACHIEVEMENTS);
    }
}

void AchievementsManager::CheckCompleted(int32_t ach) {
    const auto& state = g_Mod->state_;
    switch (ach) {
        case AchievementId::MISC_RUN_COINS_999: {
            if (ttyd::mario_pouch::pouchGetPtr()->coins == 999 &&
                state.GetOption(OPT_RUN_STARTED)) {
                MarkCompleted(ach);
            }
            break;
        }
        case AchievementId::MISC_CHET_RIPPO_SELL_ALL: {
            if (state.hp_level_ > 0) return;
            if (state.fp_level_ > 0) return;
            if (state.bp_level_ > 0) return;
            if (state.hp_p_level_ > 0 && GetNumActivePartners()) return;
            MarkCompleted(ach);
            break;
        }
        case AchievementId::MISC_LUMPY_DOUBLE_2: {
            int32_t lumpy_deals = 0;
            for (int32_t i = 0; i < 8; ++i) {
                if (state.GetOption(STAT_RUN_NPCS_SELECTED, i) == 3 &&
                    state.GetOption(STAT_RUN_NPCS_DEALT_WITH, i))
                    ++lumpy_deals;
            }
            if (lumpy_deals >= 2) MarkCompleted(ach);
            break;
        }
        case AchievementId::META_COSMETICS_5: {
            int32_t cosmetics = 0;
            for (int32_t i = 0; i < AchievementId::MAX_ACHIEVEMENT; ++i) {
                if (g_AchievementData[i].reward_type == 
                    AchievementRewardType::MARIO_COSTUME) {
                    if (state.GetOption(FLAGS_OPTION_UNLOCKED, i)) ++cosmetics;
                }
            }
            if (cosmetics < 5) return;
            cosmetics = 0;
            for (int32_t i = 0; i < AchievementId::MAX_ACHIEVEMENT; ++i) {
                if (g_AchievementData[i].reward_type == 
                    AchievementRewardType::YOSHI_COSTUME) {
                    if (state.GetOption(FLAGS_OPTION_UNLOCKED, i)) ++cosmetics;
                }
            }
            if (cosmetics < 5) return;
            cosmetics = 0;
            for (int32_t i = 0; i < AchievementId::MAX_ACHIEVEMENT; ++i) {
                if (g_AchievementData[i].reward_type == 
                    AchievementRewardType::ATTACK_FX) {
                    if (state.GetOption(FLAGS_OPTION_UNLOCKED, i)) ++cosmetics;
                }
            }
            if (cosmetics < 5) return;
            MarkCompleted(ach);
            break;
        }
        case AchievementId::META_ALL_OPTIONS: {
            for (int32_t i = 0; i < AchievementId::MAX_ACHIEVEMENT; ++i) {
                if (g_AchievementData[i].reward_type == 
                    AchievementRewardType::OPTION) {
                    if (!state.GetOption(FLAGS_OPTION_UNLOCKED, i)) return;
                }
            }
            MarkCompleted(ach);
            break;
        }
        case AchievementId::META_ALL_KEY_ITEMS: {
            for (int32_t i = ItemType::TOT_KEY_PEEKABOO; 
                i < ItemType::TOT_KEY_ITEM_MAX; ++i) {
                if (!ttyd::mario_pouch::pouchCheckItem(i)) return;
            }
            MarkCompleted(ach);
            break;
        }
        case AchievementId::META_ITEMS_BADGES_10: {
            int32_t num_items = 0;
            for (int32_t i = ItemType::THUNDER_BOLT; i < ItemType::POWER_JUMP; ++i) {
                if (state.GetOption(FLAGS_ITEM_PURCHASED, i - ItemType::THUNDER_BOLT) &&
                    ttyd::item_data::itemDataTable[i].type_sort_order >= 0)
                    ++num_items;
            }
            if (num_items < 10) return;
            int32_t num_badges = 0;
            for (int32_t i = ItemType::POWER_JUMP; i < ItemType::MAX_ITEM_TYPE; ++i) {
                if (state.GetOption(FLAGS_ITEM_PURCHASED, i - ItemType::THUNDER_BOLT) &&
                    ttyd::item_data::itemDataTable[i].type_sort_order >= 0)
                    ++num_badges;
            }
            if (num_badges < 10) return;
            MarkCompleted(ach);
            break;
        }
        case AchievementId::META_ITEMS_BADGES_ALL: {
            int32_t num_items = 0;
            for (int32_t i = ItemType::THUNDER_BOLT; i < ItemType::POWER_JUMP; ++i) {
                if (state.GetOption(FLAGS_ITEM_PURCHASED, i - ItemType::THUNDER_BOLT) &&
                    ttyd::item_data::itemDataTable[i].type_sort_order >= 0)
                    ++num_items;
            }
            if (num_items < 73) return;
            int32_t num_badges = 0;
            for (int32_t i = ItemType::POWER_JUMP; i < ItemType::MAX_ITEM_TYPE; ++i) {
                if (state.GetOption(FLAGS_ITEM_PURCHASED, i - ItemType::THUNDER_BOLT) &&
                    ttyd::item_data::itemDataTable[i].type_sort_order >= 0)
                    ++num_badges;
            }
            if (num_badges < 63) return;
            MarkCompleted(ach);
            break;
        }
        case AchievementId::META_ITEM_LOG_BASIC: {
            int32_t num_items = 0;
            for (int32_t i = ItemType::THUNDER_BOLT; i < ItemType::POWER_JUMP; ++i) {
                if (state.GetOption(FLAGS_ITEM_ENCOUNTERED, i - ItemType::THUNDER_BOLT) &&
                    ttyd::item_data::itemDataTable[i].type_sort_order >= 1 &&
                    ttyd::item_data::itemDataTable[i].type_sort_order <= 68)
                    ++num_items;
            }
            if (num_items < 68) return;
            MarkCompleted(ach);
            break;
        }
        case AchievementId::META_ITEM_LOG_ALL: {
            int32_t num_items = 0;
            for (int32_t i = ItemType::THUNDER_BOLT; i < ItemType::POWER_JUMP; ++i) {
                if (state.GetOption(FLAGS_ITEM_ENCOUNTERED, i - ItemType::THUNDER_BOLT) &&
                    ttyd::item_data::itemDataTable[i].type_sort_order >= 0)
                    ++num_items;
            }
            if (num_items < 73) return;
            MarkCompleted(ach);
            break;
        }
        case AchievementId::META_BADGE_LOG_ALL: {
            int32_t num_badges = 0;
            for (int32_t i = ItemType::POWER_JUMP; i < ItemType::MAX_ITEM_TYPE; ++i) {
                if (state.GetOption(FLAGS_ITEM_ENCOUNTERED, i - ItemType::THUNDER_BOLT) &&
                    ttyd::item_data::itemDataTable[i].type_sort_order >= 0)
                    ++num_badges;
            }
            if (num_badges < 63) return;
            MarkCompleted(ach);
            break;
        }
        case AchievementId::META_TATTLE_LOG_BASIC: {
            for (int32_t i = 1; i <= 95; ++i) {
                if (!ttyd::swdrv::swGet(i + 0x117a)) return;
            }
            MarkCompleted(ach);
            break;
        }
        case AchievementId::META_TATTLE_LOG_ALL: {
            for (int32_t i = 1; i <= 100; ++i) {
                if (!ttyd::swdrv::swGet(i + 0x117a)) return;
            }
            MarkCompleted(ach);
            break;
        }
        case AchievementId::META_MOVE_LOG_ALL: {
            for (int32_t i = 0; i < MoveType::MOVE_TYPE_MAX; ++i) {
                uint32_t flags = state.GetOption(STAT_PERM_MOVE_LOG, i);
                const auto* data = MoveManager::GetMoveData(i);

                // Check "used" (and implicitly, "obtained") flags.
                if (!(flags & MoveLogFlags::USED_LV_1)) return;
                if (data->max_level >= 2 && !(flags & MoveLogFlags::USED_LV_2)) return;
                if (data->max_level >= 3 && !(flags & MoveLogFlags::USED_LV_3)) return;

                // Check Stylish completion for moves that have one.
                if (i == MoveType::GOOMBELLA_RALLY_WINK ||
                    (i >= MoveType::SP_SWEET_TREAT && i <= MoveType::SP_SUPERNOVA))
                    continue;
                if ((flags & MoveLogFlags::STYLISH_ALL) != MoveLogFlags::STYLISH_ALL)
                    return;
            }
            MarkCompleted(ach);
            break;
        }
        case AchievementId::META_ALL_ACHIEVEMENTS: {
            for (int32_t i = 0; i < AchievementId::META_ALL_ACHIEVEMENTS; ++i) {
                if (!state.GetOption(FLAGS_ACHIEVEMENT, i)) return;
            }
            MarkCompleted(ach);
            break;
        }
    }
}

bool AchievementsManager::GetProgress(int32_t ach, int32_t* done, int32_t* total) {
    // TODO: Implement.
    return false;
}

EVT_DEFINE_USER_FUNC(evtTot_MarkCompletedAchievement) {
    int32_t ach = evtGetValue(evt, evt->evtArguments[0]);
    AchievementsManager::MarkCompleted(ach);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_CheckCompletedAchievement) {
    int32_t ach = evtGetValue(evt, evt->evtArguments[0]);
    bool prev_completed = g_Mod->state_.GetOption(FLAGS_ACHIEVEMENT, ach);
    AchievementsManager::CheckCompleted(ach);
    bool new_completed = 
        !prev_completed && g_Mod->state_.GetOption(FLAGS_ACHIEVEMENT, ach);
    evtSetValue(evt, evt->evtArguments[1], new_completed);
    return 2;
}
 
}