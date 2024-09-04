#include "tot_manager_progress.h"

#include "mod.h"
#include "tot_generate_enemy.h"
#include "tot_manager_achievements.h"
#include "tot_manager_cosmetics.h"
#include "tot_manager_move.h"
#include "tot_state.h"

#include <ttyd/battle_monosiri.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/swdrv.h>

namespace mod::tot {

namespace {

using ::ttyd::item_data::itemDataTable;

namespace ItemType = ::ttyd::item_data::ItemType;

struct ProgressData {
    int32_t overall_score;

    int32_t moves_score;
    int32_t moves_obtained;
    int32_t moves_completed;
    int32_t moves_total;
    int32_t items_current;
    int32_t items_total;
    int32_t badges_current;
    int32_t badges_total;
    int32_t tattles_current;
    int32_t tattles_total;
    int32_t achievements_current;
    int32_t achievements_total;
    int32_t achievements_secret;

    int32_t hub_score;

    int32_t key_items_current;
    int32_t key_items_total;
    int32_t hub_items_current;
    int32_t hub_items_total;
    int32_t hub_badges_current;
    int32_t hub_badges_total;
    int32_t attack_fx_current;
    int32_t attack_fx_visible;
    int32_t attack_fx_total;
    int32_t attack_fx_secret;
    int32_t mario_costumes_current;
    int32_t mario_costumes_visible;
    int32_t mario_costumes_total;
    int32_t mario_costumes_secret;
    int32_t yoshi_costumes_current;
    int32_t yoshi_costumes_visible;
    int32_t yoshi_costumes_total;
    int32_t yoshi_costumes_secret;
};
ProgressData g_ProgressData;
bool g_Cached = false;

}

void ProgressManager::RefreshCache() {
    g_Cached = false;
}

int32_t ProgressManager::GetOverallProgression() {
    auto& data = g_ProgressData;
    int32_t& score = data.overall_score;

    if (!g_Cached) {
        score = 0;

        GetAchievementLogProgress(nullptr, nullptr);
        GetMoveLogProgress(nullptr, nullptr, nullptr);
        GetItemLogProgress(nullptr, nullptr);
        GetBadgeLogProgress(nullptr, nullptr);
        GetTattleLogProgress(nullptr, nullptr);
        GetOverallHubProgression();

        // Score:
        // 30% for Achievements,
        // 15% each for Move and Tattle logs,
        // 7.5% each for Items and Badge logs,
        // 25% for Hub progression,
        // 1% per bonus achievement.
        //
        // Maximum of 105% (100% + 4% from bonus achievements + 1% from hub bonus)

        int32_t a_cur = data.achievements_current - data.achievements_secret;
        int32_t a_tot = data.achievements_total - data.achievements_secret;

        score += 3000 * a_cur / a_tot;
        score += 1500 * data.moves_score / 10000;
        score += 1500 * data.tattles_current / 102;
        score += 750 * data.items_current / data.items_total;
        score += 750 * data.badges_current / data.badges_total;
        score += data.hub_score / 4;

        score += 100 * data.achievements_secret;
    }

    // Update the saved completion score.
    g_Mod->state_.completion_score_ = score;

    return score;
}

int32_t ProgressManager::GetMoveLogProgress(int32_t* obt, int32_t* com, int32_t* tot) {
    int32_t& obtained = g_ProgressData.moves_obtained;
    int32_t& completed = g_ProgressData.moves_completed;
    int32_t& total = g_ProgressData.moves_total;
    int32_t& score = g_ProgressData.moves_score;

    if (!g_Cached) {
        int32_t current_score = 0;
        int32_t total_score = 0;

        obtained = 0;
        completed = 0;
        total = MoveType::MOVE_TYPE_MAX;

        for (int32_t move = 0; move < MoveType::MOVE_TYPE_MAX; ++move) {
            uint32_t flags = g_Mod->state_.GetOption(STAT_PERM_MOVE_LOG, move);
            const auto* move_data = MoveManager::GetMoveData(move);

            bool has_stylish = false;
            int32_t move_max_score = move_data->max_level * 2;
            if (!(move >= MoveType::SP_SWEET_TREAT && 
                move <= MoveType::SP_SUPERNOVA) &&
                move != MoveType::GOOMBELLA_RALLY_WINK) {
                has_stylish = true;
                move_max_score += 2;
            }
            total_score += move_max_score;

            if (MoveManager::PartnerNeverObtained(move)) continue;

            int32_t move_score = 0;
            if (flags & MoveLogFlags::UNLOCKED_LV_1) {
                ++obtained;
                ++move_score;
            }
            if (move_data->max_level >= 2 && (flags & MoveLogFlags::UNLOCKED_LV_2))
                ++move_score;
            if (move_data->max_level >= 3 && (flags & MoveLogFlags::UNLOCKED_LV_3))
                ++move_score;
            if ((flags & MoveLogFlags::USED_LV_1))
                ++move_score;
            if (move_data->max_level >= 2 && (flags & MoveLogFlags::USED_LV_2))
                ++move_score;
            if (move_data->max_level >= 3 && (flags & MoveLogFlags::USED_LV_3))
                ++move_score;
            if (has_stylish && (flags & MoveLogFlags::STYLISH_ALL) 
                == MoveLogFlags::STYLISH_ALL)
                move_score += 2;

            if (move_score == move_max_score) ++completed;
            current_score += move_score;
        }
        
        score = 10000 * current_score / total_score;
    }

    if (obt) *obt = obtained;
    if (com) *com = completed;
    if (tot) *tot = total;
    return score;
}

void ProgressManager::GetItemLogProgress(int32_t* cur, int32_t* tot) {
    int32_t& current = g_ProgressData.items_current;
    int32_t& total = g_ProgressData.items_total;

    if (!g_Cached) {
        current = 0;
        total = 0;

        for (int32_t i = ItemType::THUNDER_BOLT; i < ItemType::POWER_JUMP; ++i) {
            if (itemDataTable[i].type_sort_order != -1) {
                ++total;
                if (g_Mod->state_.GetOption(FLAGS_ITEM_ENCOUNTERED, (i - 0x80))) {
                    ++current;
                }
            }
        }
    }

    if (cur) *cur = current;
    if (tot) *tot = total;
}

void ProgressManager::GetBadgeLogProgress(int32_t* cur, int32_t* tot) {
    int32_t& current = g_ProgressData.badges_current;
    int32_t& total = g_ProgressData.badges_total;

    if (!g_Cached) {
        current = 0;
        total = 0;

        for (int32_t i = ItemType::POWER_JUMP; i < ItemType::MAX_ITEM_TYPE; ++i) {
            if (itemDataTable[i].type_sort_order != -1) {
                ++total;
                if (g_Mod->state_.GetOption(FLAGS_ITEM_ENCOUNTERED, (i - 0x80))) {
                    ++current;
                }
            }
        }
    }

    if (cur) *cur = current;
    if (tot) *tot = total;
}

void ProgressManager::GetTattleLogProgress(int32_t* cur, int32_t* tot) {
    int32_t& current = g_ProgressData.tattles_current;
    int32_t& total = g_ProgressData.tattles_total;

    if (!g_Cached) {
        const auto& state = g_Mod->state_;

        // Set the "total" number of Tattles based on progression, by default.
        total = 95;
        if (state.GetOption(STAT_PERM_HALF_FINISHES)) {
            total = 96;
        }
        if (state.GetOption(STAT_PERM_FULL_FINISHES)) {
            total = 98;
        }
        if (state.GetOption(STAT_PERM_EX_FINISHES)) {
            total = 100;
        }
        if (state.GetOption(FLAGS_ACHIEVEMENT, AchievementId::META_SECRET_BOSS)) {
            total = 102;
        }

        // Count valid ToT enemies that were Tattled; if any enemy's index is
        // higher than the total shown by default, increase it accordingly
        // (e.g. the player can tattle a boss, then fail and end the run).
        current = 0;
        for (int32_t i = 1; i < 0xd8; ++i) {
            int32_t custom_tattle_idx = GetCustomTattleIndex(i);
            if (ttyd::swdrv::swGet(i + 0x117a) && custom_tattle_idx > 0) {
                ++current;
                if (custom_tattle_idx > total) total = custom_tattle_idx;
            }
        }
    }

    if (cur) *cur = current;
    if (tot) *tot = total;
}

void ProgressManager::GetAchievementLogProgress(int32_t* cur, int32_t* tot) {
    int32_t& current = g_ProgressData.achievements_current;
    int32_t& total = g_ProgressData.achievements_total;
    int32_t& secret = g_ProgressData.achievements_secret;

    if (!g_Cached) {
        current = 0;
        total = 0;
        secret = 0;

        for (int32_t i = 0; i < AchievementId::MAX_ACHIEVEMENT; ++i) {
            if (g_Mod->state_.GetOption(FLAGS_ACHIEVEMENT, i)) {
                ++current;
                ++total;
                if (i >= AchievementId::SECRET_COINS) ++secret;
            } else if (i < AchievementId::SECRET_COINS) {
                // Don't count up total for secret achievements.
                ++total;
            }
        }
    }

    if (cur) *cur = current;
    if (tot) *tot = total;
}

int32_t ProgressManager::GetOverallHubProgression() {
    // Calculate component parts, if not already cached.
    GetKeyItemProgress(nullptr, nullptr);
    GetHubItemProgress(nullptr, nullptr);
    GetHubBadgeProgress(nullptr, nullptr);
    GetHubAttackFXProgress(nullptr, nullptr);
    GetHubMarioCostumeProgress(nullptr, nullptr);
    GetHubYoshiCostumeProgress(nullptr, nullptr);

    auto& data = g_ProgressData;
    int32_t& score = data.hub_score;

    if (!g_Cached) {
        score = 0;

        int32_t a_cur = data.attack_fx_current - data.attack_fx_secret;
        int32_t a_tot = data.attack_fx_total - data.attack_fx_secret;
        int32_t m_cur = data.mario_costumes_current - data.mario_costumes_secret;
        int32_t m_tot = data.mario_costumes_total - data.mario_costumes_secret;
        int32_t y_cur = data.yoshi_costumes_current - data.yoshi_costumes_secret;
        int32_t y_tot = data.yoshi_costumes_total - data.yoshi_costumes_secret;

        // 20% each for hub item, badge, and each type of cosmetic purchase.
        // (Key Items are not factored into completion).

        score += 2000 * data.hub_items_current / data.hub_items_total;
        score += 2000 * data.hub_badges_current / data.hub_badges_total;
        score += 2000 * a_cur / a_tot;
        score += 2000 * m_cur / m_tot;
        score += 2000 * y_cur / y_tot;

        // Bonus 1% each for purchasing 4 secret comsetics.
        score += 100 *
            (data.attack_fx_secret + 
             data.mario_costumes_secret + 
             data.yoshi_costumes_secret);
    }

    return score;
}

void ProgressManager::GetKeyItemProgress(int32_t* cur, int32_t* tot) {
    int32_t& current = g_ProgressData.key_items_current;
    int32_t& total = g_ProgressData.key_items_total;

    if (!g_Cached) {
        current = 0;
        total = 0;

        for (int32_t i = ItemType::TOT_KEY_PEEKABOO; i < ItemType::TOT_KEY_ITEM_MAX; ++i) {
            ++total;
            if (ttyd::mario_pouch::pouchCheckItem(i)) {
                ++current;
            }
        }
    }

    if (cur) *cur = current;
    if (tot) *tot = total;
}

void ProgressManager::GetHubItemProgress(int32_t* cur, int32_t* tot) {
    int32_t& current = g_ProgressData.hub_items_current;
    int32_t& total = g_ProgressData.hub_items_total;

    if (!g_Cached) {
        current = 0;
        total = 0;

        for (int32_t i = ItemType::THUNDER_BOLT; i < ItemType::POWER_JUMP; ++i) {
            if (itemDataTable[i].type_sort_order != -1) {
                ++total;
                if (g_Mod->state_.GetOption(FLAGS_ITEM_PURCHASED, (i - 0x80))) {
                    ++current;
                }
            }
        }
    }

    if (cur) *cur = current;
    if (tot) *tot = total;
}

void ProgressManager::GetHubBadgeProgress(int32_t* cur, int32_t* tot) {
    int32_t& current = g_ProgressData.hub_badges_current;
    int32_t& total = g_ProgressData.hub_badges_total;

    if (!g_Cached) {
        current = 0;
        total = 0;

        for (int32_t i = ItemType::POWER_JUMP; i < ItemType::MAX_ITEM_TYPE; ++i) {
            if (itemDataTable[i].type_sort_order != -1) {
                ++total;
                if (g_Mod->state_.GetOption(FLAGS_ITEM_PURCHASED, (i - 0x80))) {
                    ++current;
                }
            }
        }
    }

    if (cur) *cur = current;
    if (tot) *tot = total;
}

void ProgressManager::GetHubAttackFXProgress(int32_t* cur, int32_t* tot) {
    int32_t& current = g_ProgressData.attack_fx_current;
    int32_t& visible = g_ProgressData.attack_fx_visible;
    int32_t& total = g_ProgressData.attack_fx_total;
    int32_t& secret = g_ProgressData.attack_fx_secret;

    if (!g_Cached) {
        current = 0;
        visible = 0;
        total = 0;
        secret = 0;

        for (int32_t i = 0; i < 30; ++i) {
            const auto* data = CosmeticsManager::GetAttackFxData(i);
            if (!data) continue;
            if (CosmeticsManager::IsAvailable(CosmeticType::ATTACK_FX, i)) {
                ++current;
                ++visible;
                ++total;
                if (data->secret) ++secret;
            } else {
                if (CosmeticsManager::IsPurchaseable(CosmeticType::ATTACK_FX, i))
                    ++visible;
                if (!data->secret) ++total;
            }
        }
    }

    if (cur) *cur = current;
    if (tot) *tot = visible;
}

void ProgressManager::GetHubMarioCostumeProgress(int32_t* cur, int32_t* tot) {
    int32_t& current = g_ProgressData.mario_costumes_current;
    int32_t& visible = g_ProgressData.mario_costumes_visible;
    int32_t& total = g_ProgressData.mario_costumes_total;
    int32_t& secret = g_ProgressData.mario_costumes_secret;

    if (!g_Cached) {
        current = 0;
        visible = 0;
        total = 0;
        secret = 0;

        for (int32_t i = 0; i < 30; ++i) {
            const auto* data = CosmeticsManager::GetMarioCostumeData(i);
            if (!data) continue;
            if (CosmeticsManager::IsAvailable(CosmeticType::MARIO_COSTUME, i)) {
                ++current;
                ++visible;
                ++total;
                if (data->secret) ++secret;
            } else {
                if (CosmeticsManager::IsPurchaseable(CosmeticType::MARIO_COSTUME, i))
                    ++visible;
                if (!data->secret) ++total;
            }
        }
        
        // Hide the default costume from the count.
        if (current == 1) current = 0;
    }

    if (cur) *cur = current;
    if (tot) *tot = visible;
}

void ProgressManager::GetHubYoshiCostumeProgress(int32_t* cur, int32_t* tot) {
    int32_t& current = g_ProgressData.yoshi_costumes_current;
    int32_t& visible = g_ProgressData.yoshi_costumes_visible;
    int32_t& total = g_ProgressData.yoshi_costumes_total;
    int32_t& secret = g_ProgressData.yoshi_costumes_secret;

    if (!g_Cached) {
        current = 0;
        visible = 0;
        total = 0;
        secret = 0;

        for (int32_t i = 0; i < 30; ++i) {
            const auto* data = CosmeticsManager::GetYoshiCostumeData(i);
            if (!data) continue;
            if (CosmeticsManager::IsAvailable(CosmeticType::YOSHI_COSTUME, i)) {
                ++current;
                ++visible;
                ++total;
                if (data->secret) ++secret;
            } else {
                if (CosmeticsManager::IsPurchaseable(CosmeticType::YOSHI_COSTUME, i))
                    ++visible;
                if (!data->secret) ++total;
            }
        }
        
        // Hide the default costume from the count.
        if (current == 1) current = 0;
    }

    if (cur) *cur = current;
    if (tot) *tot = visible;
}
 
}