#include "tot_manager_achievements.h"

#include "common_functions.h"
#include "mod.h"
#include "tot_generate_enemy.h"
#include "tot_gsw.h"
#include "tot_manager_cosmetics.h"
#include "tot_manager_reward.h"
#include "tot_state.h"

#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/pmario_sound.h>
#include <ttyd/swdrv.h>
#include <ttyd/win_main.h>
#include <ttyd/win_root.h>

namespace mod::tot {

namespace {

using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

namespace ItemType = ttyd::item_data::ItemType;

const AchievementData g_AchievementData[] = {
    { "tot_achd_00", nullptr, 54, AchievementRewardType::KEY_ITEM, ItemType::TOT_KEY_TIMING_TUTOR },
    { "tot_achd_01", "tot_acho_charlieton", 53, AchievementRewardType::OPTION, OPT_CHARLIETON_STOCK },
    { "tot_achd_02", "tot_acho_npcchoice", 36, AchievementRewardType::OPTION, OPT_NPC_CHOICE_1 },
    { "tot_achd_03", "tot_acho_partner", 33, AchievementRewardType::OPTION, OPT_PARTNER },
    { "tot_achd_04", "tot_acho_acdiff", 44, AchievementRewardType::OPTION, OPT_AC_DIFFICULTY },
    { "tot_achd_05", nullptr, 83, AchievementRewardType::KEY_ITEM, ItemType::TOT_KEY_SUPER_PEEKABOO },
    { "tot_achd_06", "tot_acho_superguardcost", 56, AchievementRewardType::OPTION, OPTNUM_SUPERGUARD_SP_COST },
    { "tot_achd_07", nullptr, 27, AchievementRewardType::ATTACK_FX, 10 },
    { "tot_achd_08", "tot_acho_randomdamage", 16, AchievementRewardType::OPTION, OPT_RANDOM_DAMAGE },
    { "tot_achd_09", nullptr, 43, AchievementRewardType::KEY_ITEM, ItemType::TOT_KEY_PEEKABOO },
    { "tot_achd_10", nullptr, 42, AchievementRewardType::ATTACK_FX, 9 },
    { "tot_achd_11", nullptr, 32, AchievementRewardType::ATTACK_FX, 14 },
    { "tot_achd_12", nullptr, 46, AchievementRewardType::KEY_ITEM, ItemType::TOT_KEY_BGM_TOGGLE },
    { "tot_achd_13", nullptr, 47, AchievementRewardType::YOSHI_COSTUME, 7 },
    { "tot_achd_14", nullptr, 37, AchievementRewardType::YOSHI_COSTUME, 6 },
    { "tot_achd_15", nullptr, 38, AchievementRewardType::MARIO_COSTUME, 12 },
    { "tot_achd_16", nullptr, 28, AchievementRewardType::MARIO_COSTUME, 13 },
    { "tot_achd_17", nullptr, 18, AchievementRewardType::MARIO_COSTUME, 14 },
    { "tot_achd_18", nullptr, 35, AchievementRewardType::HAMMER, 0 },
    { "tot_achd_19", nullptr, 52, AchievementRewardType::ATTACK_FX, 12 },
    { "tot_achd_20", nullptr, 23, AchievementRewardType::MARIO_COSTUME, 11 },
    { "tot_achd_21", nullptr, 34, AchievementRewardType::YOSHI_COSTUME, 10 },
    { "tot_achd_22", nullptr, 55, AchievementRewardType::HAMMER, 0 },
    { "tot_achd_23", "tot_acho_audiencethrow", 74, AchievementRewardType::OPTION, OPT_AUDIENCE_RANDOM_THROWS },
    { "tot_achd_24", "tot_acho_infinitebp", 60, AchievementRewardType::OPTION, OPTVAL_INFINITE_BP },
    { "tot_achd_25", nullptr, 31, AchievementRewardType::HAMMER, 0 },
    { "tot_achd_26", nullptr, 29, AchievementRewardType::ATTACK_FX, 8 },
    { "tot_achd_27", nullptr, 19, AchievementRewardType::MARIO_COSTUME, 17 },
    { "tot_achd_28", nullptr, 70, AchievementRewardType::YOSHI_COSTUME, 20 },
    { "tot_achd_29", nullptr, 17, AchievementRewardType::YOSHI_COSTUME, 9 },
    { "tot_achd_30", nullptr, 78, AchievementRewardType::MARIO_COSTUME, 24 },
    { "tot_achd_31", nullptr, 79, AchievementRewardType::MARIO_COSTUME, 25 },
    { "tot_achd_32", "tot_acho_revive", 64, AchievementRewardType::OPTION, OPT_REVIVE_PARTNERS },
    { "tot_achd_33", "tot_acho_bandit", 25, AchievementRewardType::OPTION, OPT_BANDIT_ESCAPE },
    { "tot_achd_34", nullptr, 24, AchievementRewardType::MARIO_COSTUME, 5 },
    { "tot_achd_35", nullptr, 57, AchievementRewardType::MARIO_COSTUME, 6 },
    { "tot_achd_36", nullptr, 75, AchievementRewardType::MARIO_COSTUME, 7 },
    { "tot_achd_37", nullptr, 68, AchievementRewardType::YOSHI_COSTUME, 16 },
    { "tot_achd_38", nullptr, 76, AchievementRewardType::YOSHI_COSTUME, 15 },
    { "tot_achd_39", nullptr, 48, AchievementRewardType::ATTACK_FX, 11 },
    { "tot_achd_40", nullptr, 86, AchievementRewardType::MARIO_COSTUME, 9 },
    { "tot_achd_41", nullptr, 6, AchievementRewardType::HAMMER, 0 },
    { "tot_achd_42", "tot_acho_invincrease", 63, AchievementRewardType::OPTION, OPT_INVENTORY_SACK_SIZE },
    { "tot_achd_43", nullptr, 5, AchievementRewardType::MARIO_COSTUME, 16 },
    { "tot_achd_44", nullptr, 50, AchievementRewardType::MARIO_COSTUME, 18 },
    { "tot_achd_45", nullptr, 41, AchievementRewardType::YOSHI_COSTUME, 11 },
    { "tot_achd_46", nullptr, 40, AchievementRewardType::YOSHI_COSTUME, 17 },
    { "tot_achd_47", nullptr, 15, AchievementRewardType::ATTACK_FX, 7 },
    { "tot_achd_48", nullptr, 2, AchievementRewardType::MARIO_COSTUME, 10 },
    { "tot_achd_49", nullptr, 4, AchievementRewardType::ATTACK_FX, 13 },
    { "tot_achd_50", nullptr, 3, AchievementRewardType::MARIO_COSTUME, 20 },
    { "tot_achd_51", nullptr, 87, AchievementRewardType::MARIO_COSTUME, 15 },
    { "tot_achd_52", nullptr, 49, AchievementRewardType::MARIO_COSTUME, 19 },
    { "tot_achd_53", nullptr, 62, AchievementRewardType::YOSHI_COSTUME, 14 },
    { "tot_achd_54", "tot_acho_customloadout", 45, AchievementRewardType::OPTION, OPTVAL_STARTER_ITEMS_CUSTOM },
    { "tot_achd_55", nullptr, 71, AchievementRewardType::MARIO_COSTUME, 8 },
    { "tot_achd_56", "tot_acho_obfuscated", 58, AchievementRewardType::OPTION, OPT_OBFUSCATE_ITEMS },
    { "tot_achd_57", nullptr, 61, AchievementRewardType::MARIO_COSTUME, 21 },
    { "tot_achd_58", nullptr, 72, AchievementRewardType::YOSHI_COSTUME, 13 },
    { "tot_achd_59", nullptr, 59, AchievementRewardType::ATTACK_FX, 6 },
    { "tot_achd_60", nullptr, 21, AchievementRewardType::YOSHI_COSTUME, 8 },
    { "tot_achd_61", nullptr, 20, AchievementRewardType::YOSHI_COSTUME, 12 },
    { "tot_achd_62", "tot_acho_secretboss", 11, AchievementRewardType::OPTION, OPT_SECRET_BOSS },
    { "tot_achd_63", nullptr, 77, AchievementRewardType::HAMMER, 0 },
    { "tot_achd_64", nullptr, 69, AchievementRewardType::YOSHI_COSTUME, 19 },
    { "tot_achd_65", nullptr, 88, AchievementRewardType::YOSHI_COSTUME, 21 },
    { "tot_achd_66", nullptr, 80, AchievementRewardType::MARIO_COSTUME, 22 },
    { "tot_achd_67", nullptr, 0, AchievementRewardType::MARIO_COSTUME, 23 },
    { "tot_achd_68", nullptr, 89, AchievementRewardType::MARIO_COSTUME, 26 },
    { "tot_achd_69", nullptr, 9, AchievementRewardType::YOSHI_COSTUME, 18 },
    { "tot_achd_70", "tot_acho_movea", 22, AchievementRewardType::OPTION, OPT_MOVE_AVAILABILITY },
    { "tot_achd_71", "tot_acho_movel", 65, AchievementRewardType::OPTION, OPT_MOVE_LIMIT },
    { "tot_achd_72", "tot_acho_runaway", 66, AchievementRewardType::OPTION, OPT_RUN_AWAY },
    { "tot_achd_73", "tot_acho_stageh", 51, AchievementRewardType::OPTION, OPT_STAGE_HAZARDS },
    { "tot_achd_74", nullptr, 67, AchievementRewardType::MARIO_COSTUME, 27 },
    { "tot_achd_75", nullptr, 30, AchievementRewardType::MARIO_COSTUME, 28 },
    { "tot_achd_76", nullptr, 13, AchievementRewardType::YOSHI_COSTUME, 23 },
    { "tot_achd_77", nullptr, 14, AchievementRewardType::YOSHI_COSTUME, 22 },
    { "tot_achd_78", nullptr, 8, AchievementRewardType::YOSHI_COSTUME, 24 },
    { "tot_achd_79", nullptr, 12, AchievementRewardType::YOSHI_COSTUME, 25 },
    { "tot_achd_80", "tot_acho_midboss", 84, AchievementRewardType::OPTION, OPT_MIDBOSSES },
    { "tot_achd_81", "tot_acho_rerolls", 26, AchievementRewardType::OPTION, OPT_CHEST_CHOICE },
    { "tot_achd_82", nullptr, 82, AchievementRewardType::MARIO_COSTUME, 29 },
    { "tot_achd_83", nullptr, 7, AchievementRewardType::MARIO_COSTUME, 30 },
    { "tot_achd_84", nullptr, 81, AchievementRewardType::MARIO_COSTUME, 31 },
    { "tot_achd_85", nullptr, 39, AchievementRewardType::YOSHI_COSTUME, 26 },
    { "tot_achd_86", nullptr, 85, AchievementRewardType::YOSHI_COSTUME, 27 },
    { "tot_achd_87", nullptr, 1, AchievementRewardType::YOSHI_COSTUME, 28 },
    { "tot_achd_88", nullptr, 73, AchievementRewardType::YOSHI_COSTUME, 29 },
    { "tot_achd_89", nullptr, 10, AchievementRewardType::YOSHI_COSTUME, 30 },
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

bool AchievementsManager::IsSecret(int32_t ach) {
    switch (ach) {
        case AchievementId::SECRET_COINS:
        case AchievementId::SECRET_DAMAGE:
        case AchievementId::SECRET_BADGE_COMBO:
        case AchievementId::SECRET_ZERO_STATS_3:
            return true;
    }
    return false;
}

bool AchievementsManager::CheckCosmeticGroupUnlocked(
    int32_t reward_type, int32_t group_id) {
    // Special case for cosmetics that are always available for purchase.
    if (group_id == 0) return true;

    // Treat most cosmetic groups as always unlocked in race mode.
    if (g_Mod->state_.CheckOptionValue(OPTVAL_RACE_MODE_ENABLED)) {
        switch (reward_type) {
            case AchievementRewardType::ATTACK_FX:
                return true;
            case AchievementRewardType::MARIO_COSTUME: 
                return group_id <= 21 || group_id >= 27;
            case AchievementRewardType::YOSHI_COSTUME:
                return group_id <= 17 || group_id >= 22;
        }
    }
        
    for (int32_t i = 0; i < AchievementId::MAX_ACHIEVEMENT; ++i) {
        if (g_AchievementData[i].reward_type == reward_type &&
            static_cast<int32_t>(g_AchievementData[i].reward_id) == group_id) {
            return g_Mod->state_.GetOption(FLAGS_ACHIEVEMENT, i);
        }
    }
    return false;
}

bool AchievementsManager::CheckOptionUnlocked(uint32_t option) {
    // Always treat options as unlocked on special files.
    switch (g_Mod->state_.GetOptionValue(OPT_SPECIAL_FILE_MODE)) {
        case OPTVAL_RACE_MODE_ENABLED:
            return option != OPT_SECRET_BOSS;
        case OPTVAL_100_MODE_ENABLED:
            return true;
    }

    for (int32_t i = 0; i < AchievementId::MAX_ACHIEVEMENT; ++i) {
        if (g_AchievementData[i].reward_type == AchievementRewardType::OPTION &&
            g_AchievementData[i].reward_id == option) {
            return g_Mod->state_.GetOption(FLAGS_OPTION_UNLOCKED, i);
        }
    }
    // No achievement has the option tied to it, so always return true.
    return true;
}

void AchievementsManager::MarkCompleted(int32_t ach) {
    // Disable unlocking achievements on special files.
    switch (g_Mod->state_.GetOptionValue(OPT_SPECIAL_FILE_MODE)) {
        case OPTVAL_RACE_MODE_ENABLED:
            return;
        case OPTVAL_100_MODE_ENABLED:
            if (IsSecret(ach)) return;
    }

    if (!g_Mod->state_.GetOption(FLAGS_ACHIEVEMENT, ach)) {
        g_Mod->state_.SetOption(FLAGS_ACHIEVEMENT, ach);

        // Queue animations for unlocking.
        SetSWF(GSWF_AchWinQueue + ach);
        SetSWF(GSWF_AchUnlockQueue + ach);

        // TODO: Move sound effect to the window popup, if creating one.
        ttyd::pmario_sound::psndSFXOn("SFX_MOBJ_BLOCK_POWER_SHINE1");
    }

    if (!g_Mod->state_.GetOption(
        FLAGS_ACHIEVEMENT, AchievementId::META_ALL_ACHIEVEMENTS)) {
        CheckCompleted(AchievementId::META_ALL_ACHIEVEMENTS);
    }
}

void AchievementsManager::UnlockReward(int32_t ach) {
    switch (g_AchievementData[ach].reward_type) {
        case AchievementRewardType::OPTION: {
            g_Mod->state_.SetOption(FLAGS_OPTION_UNLOCKED, ach);
            // Check to see if all extra options are unlocked.
            CheckCompleted(AchievementId::META_ALL_OPTIONS);
            break;
        }
        case AchievementRewardType::KEY_ITEM: {
            int32_t item = g_AchievementData[ach].reward_id;
            if (!ttyd::mario_pouch::pouchCheckItem(item)) {
                // Award item and enable corresponding 'activated' flag.
                ttyd::mario_pouch::pouchGetItem(item);
                switch (item) {
                    case ItemType::TOT_KEY_PEEKABOO:
                        SetSWF(GSWF_PeekabooEnabled);
                        break;
                    case ItemType::TOT_KEY_SUPER_PEEKABOO:
                        SetSWF(GSWF_PeekabooEnabled);
                        SetSWF(GSWF_SuperPeekabooEnabled);
                        break;
                    case ItemType::TOT_KEY_TIMING_TUTOR:
                        SetSWF(GSWF_TimingTutorEnabled);
                        break;
                    case ItemType::TOT_KEY_BGM_TOGGLE:
                        SetSWF(GSWF_BgmEnabled);
                        break;
                }
                auto* menu = ttyd::win_main::winGetPtr();
                if (menu) {
                    // Refresh Important items display to show new item.
                    int32_t key_items_count = 0;
                    for (int32_t item = ItemType::TOT_KEY_PEEKABOO;
                        item < ItemType::TOT_KEY_ITEM_MAX; ++item) {
                        if (ttyd::mario_pouch::pouchCheckItem(item)) {
                            menu->key_items[key_items_count++] = item;
                        }
                    }
                    menu->key_items_count = key_items_count;
                    menu->items_cursor_idx[1] = 0;
                }
            }
            break;
        }
        case AchievementRewardType::HAMMER: {
            g_Mod->state_.ChangeOption(STAT_PERM_ACH_HAMMERS, 1);
            break;
        }
    }
}

void AchievementsManager::CheckCompleted(int32_t ach) {
    const auto& state = g_Mod->state_;
    switch (ach) {
        case AchievementId::V2_AGG_ENEMY_TIMES_100: {
            // Only checked here when updating a file from pre-v2.00.
            for (int32_t i = 1; i <= 105; ++i) {
                if (state.GetOption(STAT_PERM_ENEMY_KILLS, i) >= 100) {
                    MarkCompleted(ach);
                    return;
                }
            }
            break;
        }
        case AchievementId::V2_AGG_RUN_AWAY_30: {
            // Only checked here when updating a file from pre-v2.00.
            if (state.GetOption(STAT_PERM_TIMES_RAN_AWAY) >= 30) {
                MarkCompleted(ach);
            }
            break;
        }
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
        case AchievementId::V3_AGG_MERLON_10: {
            if (state.GetOption(STAT_PERM_NPC_MERLON_DEALS) >= 10)
                MarkCompleted(ach);
            break;
        }
        case AchievementId::V3_AGG_REWARD_5_EACH: {
            for (int32_t i = 0; i < RewardStatId::MAX_REWARD_STAT; ++i) {
                if (state.GetOption(STAT_PERM_REWARDS_TAKEN, i) < 5) return;
            }
            MarkCompleted(ach);
            break;
        }
        case AchievementId::META_COSMETICS_5: {
            int32_t cosmetics = 0;
            for (int32_t type = 0; type <= 2; ++type) {
                for (int32_t i = 1; i < 32; ++i) {
                    if (state.GetOption(FLAGS_COSMETIC_PURCHASED, type * 32 + i))
                        ++cosmetics;
                }
                if (cosmetics < 5) return;
                cosmetics = 0;
            }
            MarkCompleted(ach);
            break;
        }
        case AchievementId::META_ALL_OPTIONS: {
            // Check for completion of all achievements that reward options.
            for (int32_t i = 0; i < AchievementId::MAX_ACHIEVEMENT; ++i) {
                if (g_AchievementData[i].reward_type == 
                    AchievementRewardType::OPTION) {
                    if (!state.GetOption(FLAGS_OPTION_UNLOCKED, i)) return;
                }
            }

            // Check for having unlocked options tied to NPCs.
            if (!GetSWF(GSWF_NpcF_SeedUnlocked)) return;
            if (!GetSWF(GSWF_NpcK_CustomMovesUnlocked)) return;
            if (!GetSWF(GSWF_White_CountdownUnlocked)) return;
            // Check for having beaten each secret boss once.
            if (!GetSWF(GSWF_SecretBoss1_Beaten)) return;
            if (!GetSWF(GSWF_SecretBoss2_Beaten)) return;
            if (!GetSWF(GSWF_SecretBoss3_Beaten)) return;

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
        case AchievementId::META_ITEMS_BADGES_5: {
            int32_t num_items = 0;
            for (int32_t i = ItemType::THUNDER_BOLT; i < ItemType::POWER_JUMP; ++i) {
                if (state.GetOption(FLAGS_ITEM_PURCHASED, i - ItemType::THUNDER_BOLT) &&
                    ttyd::item_data::itemDataTable[i].type_sort_order >= 0)
                    ++num_items;
            }
            if (num_items < 5) return;
            int32_t num_badges = 0;
            for (int32_t i = ItemType::POWER_JUMP; i < ItemType::MAX_ITEM_TYPE; ++i) {
                if (state.GetOption(FLAGS_ITEM_PURCHASED, i - ItemType::THUNDER_BOLT) &&
                    ttyd::item_data::itemDataTable[i].type_sort_order >= 0)
                    ++num_badges;
            }
            if (num_badges < 5) return;
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
        case AchievementId::META_TATTLE_LOG_50: {
            int32_t tattles = 0;
            for (int32_t i = 1; i < 0xd8; ++i) {
                int32_t custom_tattle_idx = GetCustomTattleIndex(i);
                if (custom_tattle_idx >= 1 && custom_tattle_idx <= 105) {
                    if (ttyd::swdrv::swGet(i + 0x117a)) ++tattles;
                }
            }
            if (tattles >= 50) MarkCompleted(ach);
            break;
        }
        case AchievementId::META_TATTLE_LOG_ALL: {
            for (int32_t i = 1; i < 0xd8; ++i) {
                int32_t custom_tattle_idx = GetCustomTattleIndex(i);
                if (custom_tattle_idx >= 1 && custom_tattle_idx <= 105) {
                    if (!ttyd::swdrv::swGet(i + 0x117a)) return;
                }
            }
            MarkCompleted(ach);
            break;
        }
        case AchievementId::V2_META_USE_50_MOVES: {
            int32_t moves_used = 0;
            for (int32_t i = 0; i < MoveType::MOVE_TYPE_MAX; ++i) {
                uint32_t flags = state.GetOption(STAT_PERM_MOVE_LOG, i);
                // Check if any of the "used" flags are set.
                if ((flags & MoveLogFlags::USED_ALL)) ++moves_used;
            }
            if (moves_used >= 50) MarkCompleted(ach);
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
            for (int32_t i = 0; i < AchievementId::MAX_ACHIEVEMENT; ++i) {
                // Verify all non-secret achievements other than this are done.
                if (i != ach && !IsSecret(i) &&
                    !state.GetOption(FLAGS_ACHIEVEMENT, i)) return;
            }
            MarkCompleted(ach);
            break;
        }
    }
}

void AchievementsManager::GetProgress(
    int32_t ach, int32_t& progress, int32_t& total) {
    const auto& state = g_Mod->state_;

    // By default, set both to a sentinel of -1.
    progress = -1;
    total = -1;

    switch (ach) {
        // Single-run progression achievements.
        case AchievementId::V3_RUN_INFATUATE: {
            progress = state.GetOption(STAT_RUN_INFATUATE_DAMAGE);
            total = 500;
            break;
        }
        case AchievementId::RUN_NPC_DEALS_7: {
            total = 7;
            progress = 0;
            for (int32_t i = 0; i < 8; ++i) {
                if (state.GetOption(STAT_RUN_NPCS_DEALT_WITH, i)) ++progress;
            }
            break;
        }
        case AchievementId::RUN_ALL_FLOORS_3_TURN: {
            // If currently haven't failed, print the floor progress.
            if (state.GetOption(STAT_RUN_MOST_TURNS_RECORD) <= 3) {
                total = state.GetNumFloors();
                progress =
                    state.floor_ - (GetSWByte(GSW_Tower_ChestClaimed) ? 0 : 1);
            }
            break;
        }
        case AchievementId::RUN_ALL_CONDITIONS_MET: {
            // If currently haven't failed, print the number of met conditions.
            if (!state.GetOption(STAT_RUN_CONDITIONS_FAILED)) {
                total = 10;
                progress = state.GetOption(STAT_RUN_CONDITIONS_MET);
            }
            break;
        }
        case AchievementId::V2_RUN_HAMMERMAN: {
            // If currently haven't failed, print the floor progress.
            if (!state.GetOption(STAT_RUN_HAMMERMAN_FAILED)) {
                total = state.GetNumFloors();
                progress =
                    state.floor_ - (GetSWByte(GSW_Tower_ChestClaimed) ? 0 : 1);
            }
            break;
        }
        case AchievementId::V3_RUN_GRUBBA: {
            // If Grubba active and streak hasn't ended, print progress.
            if (state.GetOption(STAT_RUN_NPC_GRUBBA_FLOOR)) {
                int32_t grubba_combo = state.GetOption(STAT_RUN_NPC_GRUBBA_COMBO);
                if (grubba_combo <= 7) {
                    progress = grubba_combo;
                    total = 7;
                }
            }
            break;
        }

        // Multiple-run progression achievements.
        case AchievementId::META_RUNS_10: {
            total = 10;
            progress = 0;
            progress += state.GetOption(STAT_PERM_HALF_FINISHES);
            progress += state.GetOption(STAT_PERM_FULL_FINISHES);
            progress += state.GetOption(STAT_PERM_EX_FINISHES);
            break;
        }
        case AchievementId::META_RUNS_25: {
            total = 25;
            progress = 0;
            progress += state.GetOption(STAT_PERM_HALF_FINISHES);
            progress += state.GetOption(STAT_PERM_FULL_FINISHES);
            progress += state.GetOption(STAT_PERM_EX_FINISHES);
            break;
        }
        case AchievementId::AGG_NPC_DEALS_10: {
            progress = state.GetOption(STAT_PERM_NPC_DEALS_TOTAL);
            total = 10;
            break;
        }
        case AchievementId::V3_AGG_MERLON_10: {
            progress = state.GetOption(STAT_PERM_NPC_MERLON_DEALS);
            total = 10;
            break;
        }
        case AchievementId::AGG_BUY_ITEMS_25: {
            progress = state.GetOption(STAT_PERM_ITEMS_BOUGHT);
            total = 25;
            break;
        }
        case AchievementId::AGG_SUPERGUARD_100: {
            progress = state.GetOption(STAT_PERM_SUPERGUARDS);
            total = 100;
            break;
        }
        case AchievementId::AGG_COINS_10000: {
            progress = state.GetOption(STAT_PERM_META_COINS_EARNED);
            total = 10000;
            break;
        }
        case AchievementId::AGG_DAMAGE_15000: {
            progress = state.GetOption(STAT_PERM_ENEMY_DAMAGE);
            total = 15000;
            break;
        }
        case AchievementId::V2_AGG_RUN_AWAY_30: {
            progress = state.GetOption(STAT_PERM_TIMES_RAN_AWAY);
            total = 30;
            break;
        }
        case AchievementId::V3_AGG_REWARD_5_EACH: {
            total = RewardStatId::MAX_REWARD_STAT;
            progress = 0;
            for (int32_t i = 0; i < RewardStatId::MAX_REWARD_STAT; ++i) {
                if (state.GetOption(STAT_PERM_REWARDS_TAKEN, i) >= 5) ++progress;
            }
            break;
        }
        case AchievementId::AGG_ALL_PARTNERS: {
            total = 7;
            progress = 0;
            int32_t obtained = state.GetOption(STAT_PERM_PARTNERS_OBTAINED);
            for (int32_t i = 1; i <= 7; ++i) {
                progress += (obtained & (1 << i)) ? 1 : 0;
            }
            break;
        }
        case AchievementId::V3_AGG_BOSS_ALL_EX: {
            total = 4;
            progress = 0;
            progress += GetSWF(GSWF_RegularBoss_BeatenEX) ? 1 : 0;
            progress += GetSWF(GSWF_SecretBoss1_BeatenEX) ? 1 : 0;
            progress += GetSWF(GSWF_SecretBoss2_BeatenEX) ? 1 : 0;
            progress += GetSWF(GSWF_SecretBoss3_BeatenEX) ? 1 : 0;
            break;
        }

        // Move, midboss, etc. Journal progression.
        case AchievementId::V2_META_USE_50_MOVES: {
            total = 50;
            progress = 0;
            for (int32_t i = 0; i < MoveType::MOVE_TYPE_MAX; ++i) {
                uint32_t value = g_Mod->state_.GetOption(STAT_PERM_MOVE_LOG, i);
                if (value & MoveLogFlags::USED_ALL) ++progress;
            }
            break;
        }
        case AchievementId::AGG_USE_LV_3_MOVES_10: {
            total = 10;
            progress = 0;
            for (int32_t i = 0; i < MoveType::MOVE_TYPE_MAX; ++i) {
                uint32_t value = g_Mod->state_.GetOption(STAT_PERM_MOVE_LOG, i);
                if (value & MoveLogFlags::USED_LV_3) ++progress;
            }
            break;
        }
        case AchievementId::AGG_STYLISH_15: {
            total = 15;
            progress = 0;
            for (int32_t i = 0; i < MoveType::MOVE_TYPE_MAX; ++i) {
                uint32_t value = g_Mod->state_.GetOption(STAT_PERM_MOVE_LOG, i);
                if ((value & MoveLogFlags::STYLISH_ALL) == MoveLogFlags::STYLISH_ALL) {
                    ++progress;
                }
            }
            break;
        }
        case AchievementId::V2_AGG_ENEMY_TIMES_100: {
            total = 100;
            progress = 0;
            for (int32_t i = 0; i < 128; ++i) {
                int32_t kills = state.GetOption(STAT_PERM_ENEMY_KILLS, i);
                if (kills > progress) progress = kills;
            }
            break;
        }
        case AchievementId::AGG_MIDBOSS_TYPES_30: {
            total = 30;
            progress = 0;
            for (int32_t i = 0; i < 128; ++i) {
                if (state.GetOption(FLAGS_MIDBOSS_DEFEATED, i)) ++progress;
            }
            break;
        }
        case AchievementId::V3_AGG_MIDBOSS_TYPES_ALL: {
            total = 75;
            progress = 0;
            for (int32_t i = 0; i < 128; ++i) {
                if (state.GetOption(FLAGS_MIDBOSS_DEFEATED, i)) ++progress;
            }
            break;
        }

        // default: no progress to track.
    }
}

// Backing arrays for "achievement grid" interface.
int8_t g_AchievementGrid[AchievementId::MAX_ACHIEVEMENT + 1] = { -1 };
int8_t g_AchievementStates[AchievementId::MAX_ACHIEVEMENT + 1] = { -1 };

void AchievementsManager::GetAchievementGrid(int8_t const ** arr) {
    for (int32_t ach = 0; ach < AchievementId::MAX_ACHIEVEMENT; ++ach) {
        int32_t index = g_AchievementData[ach].grid_index;
        g_AchievementGrid[index] = ach;
    }
    g_AchievementGrid[AchievementId::MAX_ACHIEVEMENT] = -1;
    
    *arr = g_AchievementGrid;
}

int32_t AchievementsManager::GetAchievementStates(int8_t const ** arr, bool suppress_queued) {
    const auto& state = g_Mod->state_;
    int32_t queued_unlock = -1;
    int32_t queued_unlock_ach = -1;
    for (int32_t i = 0; i < AchievementId::MAX_ACHIEVEMENT; ++i) {
        int32_t ach = g_AchievementGrid[i];
        g_AchievementStates[i] = state.GetOption(FLAGS_ACHIEVEMENT, ach) ? 2 : 0;
    }
    if (suppress_queued) {
        for (int32_t i = 0; i < AchievementId::MAX_ACHIEVEMENT; ++i) {
            int32_t ach = g_AchievementGrid[i];
            if (GetSWF(GSWF_AchUnlockQueue + ach)) {
                // Suppress cell from being drawn as unlocked.
                g_AchievementStates[i] = 0;
                if (queued_unlock < 0 || ach < queued_unlock_ach) {
                    queued_unlock = i;
                    queued_unlock_ach = ach;
                }
            }
        }
    }
    for (int32_t i = 0; i < AchievementId::MAX_ACHIEVEMENT; ++i) {
        int32_t ach = g_AchievementGrid[i];
        if (g_AchievementStates[i] == 0 && !IsSecret(ach) && (
            (i % 10 > 0 && g_AchievementStates[i -  1] == 2) ||
            (i % 10 < 9 && g_AchievementStates[i +  1] == 2) ||
            (i / 10 > 0 && g_AchievementStates[i - 10] == 2) ||
            (i / 10 < 8 && g_AchievementStates[i + 10] == 2))) {
            g_AchievementStates[i] = 1;
        }
    }
    g_AchievementStates[AchievementId::MAX_ACHIEVEMENT] = -1;

    *arr = g_AchievementStates;
    return queued_unlock;
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
    bool completed = g_Mod->state_.GetOption(FLAGS_ACHIEVEMENT, ach);
    bool newly_completed = completed && !prev_completed;
    evtSetValue(evt, evt->evtArguments[1], completed);
    evtSetValue(evt, evt->evtArguments[2], newly_completed);
    return 2;
}
 
}