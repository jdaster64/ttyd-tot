#include "tot_window_log.h"

#include "common_functions.h"
#include "common_types.h"
#include "mod.h"
#include "tot_generate_enemy.h"
#include "tot_gsw.h"
#include "tot_manager_achievements.h"
#include "tot_manager_cosmetics.h"
#include "tot_manager_progress.h"
#include "tot_state.h"

#include <gc/types.h>
#include <ttyd/gx/GXTev.h>
#include <ttyd/gx/GXTransform.h>
#include <ttyd/_core_language_libs.h>
#include <ttyd/animdrv.h>
#include <ttyd/battle_monosiri.h>
#include <ttyd/dispdrv.h>
#include <ttyd/filemgr.h>
#include <ttyd/fontmgr.h>
#include <ttyd/icondrv.h>
#include <ttyd/item_data.h>
#include <ttyd/mario.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/mario_party.h>
#include <ttyd/memory.h>
#include <ttyd/msgdrv.h>
#include <ttyd/pmario_sound.h>
#include <ttyd/sound.h>
#include <ttyd/swdrv.h>
#include <ttyd/system.h>
#include <ttyd/win_log.h>
#include <ttyd/win_main.h>
#include <ttyd/win_party.h>
#include <ttyd/win_root.h>
#include <ttyd/winmgr.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::tot::win {

namespace {

// For convenience.
using namespace ::ttyd::gx::GXTev;

using ::ttyd::dispdrv::CameraId;
using ::ttyd::item_data::itemDataTable;
using ::ttyd::msgdrv::msgSearch;
using ::ttyd::win_root::WinPauseMenu;
using ::ttyd::winmgr::WinMgrEntry;

namespace IconType = ::ttyd::icondrv::IconType;
namespace ItemType = ::ttyd::item_data::ItemType;
namespace ItemUseLocation = ::ttyd::item_data::ItemUseLocation_Flags;

enum RecordLogOptions {
    // All record-log specific options should have negative values.
    REC_EMPTY = -100000,
    REC_PLAY_TIME,
    REC_COMPLETION_PCT,
    REC_ITEM_PCT,
    REC_BADGE_PCT,
    REC_MOVE_PCT,
    REC_MOVE_CMP_PCT,
    REC_TATTLE_PCT,
    REC_ACHIEVEMENT_PCT,
    REC_HUB_PROGRESS_PCT,
    REC_HUB_ITEMS,
    REC_HUB_BADGES,
    REC_HUB_KEY_ITEMS,
    REC_HUB_MARIO_SKINS,
    REC_HUB_YOSHI_SKINS,
    REC_HUB_ATTACK_FX,
    REC_TOTAL_ATTEMPTS,
    REC_TOTAL_FINISHES,
    REC_UNIQUE_MIDBOSSES,
    REC_BEST_TIMES,
};

struct RecordLogEntry {
    int32_t option;
    const char* name_msg;
    const char* desc_msg;
};
const RecordLogEntry kRecordLogEntries[] = {
    { REC_EMPTY, "tot_recn_runstats_1", "tot_rech_runstats" },
    { STAT_RUN_TURNS_SPENT, "tot_optr_turnsspent", "tot_rech_runstats" },
    { STAT_RUN_TIMES_RAN_AWAY, "tot_optr_timesran", "tot_rech_runstats" },
    { STAT_RUN_ENEMY_DAMAGE, "tot_optr_enemydamage", "tot_rech_runstats" },
    { STAT_RUN_PLAYER_DAMAGE, "tot_optr_playerdamage", "tot_rech_runstats" },
    { STAT_RUN_COINS_EARNED, "tot_optr_coinsearned", "tot_rech_runstats" },
    { STAT_RUN_COINS_SPENT, "tot_optr_coinsspent", "tot_rech_runstats" },
    { STAT_RUN_STAR_PIECES, "tot_optr_starpieces", "tot_rech_runstats" },
    { STAT_RUN_SHINE_SPRITES, "tot_optr_shinesprites", "tot_rech_runstats" },
    { REC_EMPTY, "tot_recn_runstats_2", "tot_rech_runstats" },
    { STAT_RUN_ITEMS_USED, "tot_optr_itemsused", "tot_rech_runstats" },
    { STAT_RUN_ITEMS_BOUGHT, "tot_optr_itemsbought", "tot_rech_runstats" },
    { STAT_RUN_FP_SPENT, "tot_optr_fpspent", "tot_rech_runstats" },
    { STAT_RUN_SP_SPENT, "tot_optr_spspent", "tot_rech_runstats" },
    { STAT_RUN_SUPERGUARDS, "tot_optr_superguards", "tot_rech_runstats" },
    { STAT_RUN_CONDITIONS_MET, "tot_optr_conditionsmet", "tot_rech_runstats" },
    { STAT_RUN_NPCS_DEALT_WITH, "tot_optr_npcsmet", "tot_rech_runstats" },
    { STAT_RUN_CONTINUES, "tot_optr_continues", "tot_rech_runstats" },
    { REC_EMPTY, "tot_recn_overall", "tot_rech_progression" },
    { REC_COMPLETION_PCT, "tot_recn_completion_pct", "tot_rech_progression" },
    { REC_ACHIEVEMENT_PCT, "tot_recn_achievement_pct", "tot_rech_progression" },
    { REC_ITEM_PCT, "tot_recn_item_pct", "tot_rech_progression" },
    { REC_BADGE_PCT, "tot_recn_badge_pct", "tot_rech_progression" },
    { REC_TATTLE_PCT, "tot_recn_tattle_pct", "tot_rech_progression" },
    { REC_MOVE_PCT, "tot_recn_move_pct", "tot_rech_progression" },
    { REC_MOVE_CMP_PCT, "tot_recn_move_cmp_pct", "tot_rech_progression" },
    { REC_PLAY_TIME, "tot_recn_playtime", "tot_rech_progression" },
    { REC_EMPTY, "tot_recn_hub", "tot_rech_hub_pct" },
    { REC_HUB_PROGRESS_PCT, "tot_recn_hub_pct", "tot_rech_hub_pct" },
    { REC_HUB_KEY_ITEMS, "tot_recn_hub_keyitems", "tot_rech_hub_pct" },
    { REC_HUB_ITEMS, "tot_recn_hub_items", "tot_rech_hub_pct" },
    { REC_HUB_BADGES, "tot_recn_hub_badges", "tot_rech_hub_pct" },
    { REC_HUB_MARIO_SKINS, "tot_recn_hub_marioskins", "tot_rech_hub_pct" },
    { REC_HUB_YOSHI_SKINS, "tot_recn_hub_yoshiskins", "tot_rech_hub_pct" },
    { REC_HUB_ATTACK_FX, "tot_recn_hub_attackfx", "tot_rech_hub_pct" },
    { REC_EMPTY, nullptr, "tot_rech_wins" },
    { REC_EMPTY, "tot_recn_runs", "tot_rech_wins" },
    { STAT_PERM_HALF_FINISHES, "tot_recn_half_wins", "tot_rech_wins" },
    { STAT_PERM_FULL_FINISHES, "tot_recn_full_wins", "tot_rech_wins" },
    { STAT_PERM_EX_FINISHES, "tot_recn_ex_wins", "tot_rech_wins" },
    { STAT_PERM_MAX_INTENSITY, "tot_recn_intensity", "tot_rech_wins" },
    { REC_BEST_TIMES, "tot_recn_times", "tot_rech_wins" },
    { STAT_PERM_HALF_BEST_TIME, "tot_recn_half_time", "tot_rech_wins" },
    { STAT_PERM_FULL_BEST_TIME, "tot_recn_full_time", "tot_rech_wins" },
    { STAT_PERM_EX_BEST_TIME, "tot_recn_ex_time", "tot_rech_wins" },
    { REC_EMPTY, "tot_recn_aggstats_1", "tot_rech_aggstats" },
    { REC_TOTAL_ATTEMPTS, "tot_recn_attempts", "tot_rech_aggstats" },
    { REC_TOTAL_FINISHES, "tot_recn_clears", "tot_rech_aggstats" },
    { STAT_PERM_MAX_INTENSITY, "tot_recn_intensity", "tot_rech_aggstats" },
    { STAT_PERM_CONTINUES, "tot_recn_continues", "tot_rech_aggstats" },
    { STAT_PERM_FLOORS, "tot_recn_floors", "tot_rech_aggstats" },
    { STAT_PERM_TURNS_SPENT, "tot_recn_turns", "tot_rech_aggstats" },
    { STAT_PERM_TIMES_RAN_AWAY, "tot_recn_runaway", "tot_rech_aggstats" },
    { STAT_PERM_NPC_DEALS_TOTAL, "tot_optr_npcsmet", "tot_rech_aggstats" },
    { REC_EMPTY, "tot_recn_aggstats_2", "tot_rech_aggstats" },
    { STAT_PERM_ENEMIES_DEFEATED, "tot_recn_kills", "tot_rech_aggstats" },
    { REC_UNIQUE_MIDBOSSES, "tot_recn_midkills", "tot_rech_aggstats" },
    { STAT_PERM_ENEMY_DAMAGE, "tot_recn_edamage", "tot_rech_aggstats" },
    { STAT_PERM_PLAYER_DAMAGE, "tot_recn_pdamage", "tot_rech_aggstats" },
    { STAT_PERM_FP_SPENT, "tot_recn_fpspent", "tot_rech_aggstats" },
    { STAT_PERM_SP_SPENT, "tot_recn_spspent", "tot_rech_aggstats" },
    { STAT_PERM_CONDITIONS_MET, "tot_recn_conditions", "tot_rech_aggstats" },
    { STAT_PERM_SUPERGUARDS, "tot_recn_superguards", "tot_rech_aggstats" },
    { REC_EMPTY, "tot_recn_aggstats_3", "tot_rech_aggstats" },
    { STAT_PERM_COINS_EARNED, "tot_recn_rcoinsearned", "tot_rech_aggstats" },
    { STAT_PERM_COINS_SPENT, "tot_recn_rcoinsspent", "tot_rech_aggstats" },
    { STAT_PERM_STAR_PIECES, "tot_recn_starpieces", "tot_rech_aggstats" },
    { STAT_PERM_SHINE_SPRITES, "tot_recn_shinesprites", "tot_rech_aggstats" },
    { STAT_PERM_ITEMS_USED, "tot_recn_itemsused", "tot_rech_aggstats" },
    { STAT_PERM_ITEMS_BOUGHT, "tot_recn_itemsbought", "tot_rech_aggstats" },
    { STAT_PERM_META_COINS_EARNED, "tot_recn_mcoinsearned", "tot_rech_aggstats" },
    { STAT_PERM_META_SP_EARNED, "tot_recn_mspearned", "tot_rech_aggstats" },
};
const int32_t kNumRecordLogEntries = sizeof(kRecordLogEntries) / sizeof(RecordLogEntry);

int32_t CompBadgeItemABC(int16_t* lhs, int16_t* rhs) {
    return strcmp(
        msgSearch(itemDataTable[*lhs + 0x80].name),
        msgSearch(itemDataTable[*rhs + 0x80].name));
}

int32_t CompBadgeItemABC_R(int16_t* lhs, int16_t* rhs) {
    return strcmp(
        msgSearch(itemDataTable[*rhs + 0x80].name),
        msgSearch(itemDataTable[*lhs + 0x80].name));
}

int32_t CompBadgeItemId(int16_t* lhs, int16_t* rhs) {
    return itemDataTable[*lhs + 0x80].type_sort_order
         - itemDataTable[*rhs + 0x80].type_sort_order;
}

int32_t CompBadgeItemId_R(int16_t* lhs, int16_t* rhs) {
    return itemDataTable[*rhs + 0x80].type_sort_order
         - itemDataTable[*lhs + 0x80].type_sort_order;
}

int32_t CompBadgeBP(int16_t* lhs, int16_t* rhs) {
    // Sort by cost ascending, then type ascending.
    const int32_t lhs_sort =
        (itemDataTable[*lhs + 0x80].bp_cost << 16)
        + itemDataTable[*lhs + 0x80].type_sort_order;
    const int32_t rhs_sort =
        (itemDataTable[*rhs + 0x80].bp_cost << 16)
        + itemDataTable[*rhs + 0x80].type_sort_order;
    return lhs_sort - rhs_sort;
}

int32_t CompBadgeBP_R(int16_t* lhs, int16_t* rhs) {
    // Sort by cost descending, then type ascending.
    const int32_t lhs_sort =
        ((10 - itemDataTable[*lhs + 0x80].bp_cost) << 16)
        + itemDataTable[*lhs + 0x80].type_sort_order;
    const int32_t rhs_sort =
        ((10 - itemDataTable[*rhs + 0x80].bp_cost) << 16)
        + itemDataTable[*rhs + 0x80].type_sort_order;
    return lhs_sort - rhs_sort;
}

void SortBadgeLogABC(WinPauseMenu* menu) {
    if (menu->has_sorted == 0) {
        ttyd::system::qqsort(
            menu->badge_log_ids, menu->badge_log_total_count, 2,
            (void*)CompBadgeItemABC);
    } else {
        ttyd::system::qqsort(
            menu->badge_log_ids, menu->badge_log_total_count, 2,
            (void*)CompBadgeItemABC_R);
    }
    menu->has_sorted = !menu->has_sorted;
    menu->badge_log_cursor_idx = 0;
    menu->badge_log_page_num = 0;
}

void SortBadgeLogType(WinPauseMenu* menu) {
    if (menu->has_sorted == 0) {
        ttyd::system::qqsort(
            menu->badge_log_ids, menu->badge_log_total_count, 2,
            (void*)CompBadgeItemId);
    } else {
        ttyd::system::qqsort(
            menu->badge_log_ids, menu->badge_log_total_count, 2,
            (void*)CompBadgeItemId_R);
    }
    menu->has_sorted = !menu->has_sorted;
    menu->badge_log_cursor_idx = 0;
    menu->badge_log_page_num = 0;
}

void SortBadgeLogBP(WinPauseMenu* menu) {
    if (menu->has_sorted == 0) {
        ttyd::system::qqsort(
            menu->badge_log_ids, menu->badge_log_total_count, 2,
            (void*)CompBadgeBP);
    } else {
        ttyd::system::qqsort(
            menu->badge_log_ids, menu->badge_log_total_count, 2,
            (void*)CompBadgeBP_R);
    }
    menu->has_sorted = !menu->has_sorted;
    menu->badge_log_cursor_idx = 0;
    menu->badge_log_page_num = 0;
}

void SortItemLogABC(WinPauseMenu* menu) {
    if (menu->has_sorted == 0) {
        ttyd::system::qqsort(
            menu->recipe_log_ids, menu->recipe_log_total_count, 2,
            (void*)CompBadgeItemABC);
    } else {
        ttyd::system::qqsort(
            menu->recipe_log_ids, menu->recipe_log_total_count, 2,
            (void*)CompBadgeItemABC_R);
    }
    menu->has_sorted = !menu->has_sorted;
    menu->recipe_log_cursor_idx = 0;
    menu->recipe_log_page_num = 0;
}

void SortItemLogType(WinPauseMenu* menu) {
    if (menu->has_sorted == 0) {
        ttyd::system::qqsort(
            menu->recipe_log_ids, menu->recipe_log_total_count, 2,
            (void*)CompBadgeItemId);
    } else {
        ttyd::system::qqsort(
            menu->recipe_log_ids, menu->recipe_log_total_count, 2,
            (void*)CompBadgeItemId_R);
    }
    menu->has_sorted = !menu->has_sorted;
    menu->recipe_log_cursor_idx = 0;
    menu->recipe_log_page_num = 0;
}

bool IsUnbreakable(int32_t cursor) {
    int32_t row = cursor / 10;
    int32_t col = cursor % 10;
    return (row <= 1 || row >= 5) && (col <= 1 || col >= 8);
}

void GetAchievementRewardDetails(
    const int8_t* grid, int32_t cursor, int32_t* icon, const char** name_msg) {
    // Special case for selecting one of the unlocking hammers.
    if (cursor == -1) {
        if (icon) *icon = IconType::HAMMER;
        if (name_msg) *name_msg =
            ttyd::item_data::itemDataTable[ItemType::HAMMER].name;
        return;
    }
    const auto* data =  AchievementsManager::GetData(grid[cursor]);
    switch (data->reward_type) {
        case AchievementRewardType::OPTION:
            if (icon) *icon = IconType::VITAL_PAPER;
            if (name_msg) *name_msg = data->reward_msg;
            break;
        case AchievementRewardType::HAMMER:
            if (icon) *icon = IconType::HAMMER;
            if (name_msg) *name_msg = 
                ttyd::item_data::itemDataTable[ItemType::HAMMER].name;
            break;
        case AchievementRewardType::KEY_ITEM:
            if (icon) *icon =
                ttyd::item_data::itemDataTable[data->reward_id].icon_id;
            if (name_msg) *name_msg =
                ttyd::item_data::itemDataTable[data->reward_id].name;
            break;
        case AchievementRewardType::MARIO_COSTUME: {
            const auto* cosmetic_data = CosmeticsManager::GetGroupData(
                CosmeticType::MARIO_COSTUME, data->reward_id);
            if (icon) *icon = cosmetic_data->icon;
            if (name_msg) *name_msg = cosmetic_data->name_msg;
            break;
        }
        case AchievementRewardType::YOSHI_COSTUME: {
            const auto* cosmetic_data = CosmeticsManager::GetGroupData(
                CosmeticType::YOSHI_COSTUME, data->reward_id);
            if (icon) *icon = cosmetic_data->icon;
            if (name_msg) *name_msg = cosmetic_data->name_msg;
            break;
        }
        case AchievementRewardType::ATTACK_FX: {
            const auto* cosmetic_data = CosmeticsManager::GetGroupData(
                CosmeticType::ATTACK_FX, data->reward_id);
            if (icon) *icon = cosmetic_data->icon;
            if (name_msg) *name_msg = cosmetic_data->name_msg;
            break;
        }
    }
}

void DrawAchievementLog(WinPauseMenu* menu, float win_x, float win_y) {
    int8_t const* grid;
    int8_t const* states;
    AchievementsManager::GetAchievementGrid(&grid);
    AchievementsManager::GetAchievementStates(&states);

    ttyd::win_main::winTexInit(*menu->win_tpl->mpFileData);
    // Draw square drop shadows.
    for (int32_t i = 0; i < AchievementId::MAX_ACHIEVEMENT; ++i) {
        // Skip squares that don't have a visible shadow.
        if (i % 10 < 8 && i / 10 < 5);
        // Skip drawing corner squares if not completed.
        if (states[i] != 2 && grid[i] >= 66) continue;

        gc::vec3 position = {
            win_x + 105.0f + 36.0f * (i % 10),
            win_y - 14.0f - 36.0f * (i / 10),
            0.0f
        };
        gc::vec3 scale = { 0.642857f, 0.642857f, 0.642857f };
        uint32_t color = 0x00000080U;
        ttyd::win_main::winTexSet(0x7b, &position, &scale, &color);
    }
    // Draw main square backgrounds.
    for (int32_t i = 0; i < AchievementId::MAX_ACHIEVEMENT; ++i) {
        // Skip drawing corner squares if not completed.
        if (states[i] != 2 && grid[i] >= 66) continue;

        int32_t darker_shade = ((i ^ (i / 10)) & 1);
        int32_t success_shade = states[i] == 2 ? 13 : 0;
        uint32_t color = 0x505050FFU;
        switch (states[i]) {
            case 1:  color = 0xb0b0b0FFU;  break;
            case 2:  color = 0xFFFFFFFFU;  break;
        }
        gc::vec3 position = {
            win_x + 101.0f + 36.0f * (i % 10),
            win_y - 10.0f - 36.0f * (i / 10),
            0.0f
        };
        gc::vec3 scale = { 0.642857f, 0.642857f, 0.642857f };
        ttyd::win_main::winTexInit(*menu->win_tpl->mpFileData);
        ttyd::win_main::winTexSet(
            0x7b + darker_shade + success_shade, &position, &scale, &color);

        // Draw icon (or its shadow).
        if (states[i]) {
            int32_t icon = 0;
            GetAchievementRewardDetails(grid, i, &icon, nullptr);
            ttyd::win_main::winIconInit();
            scale = { 0.45f, 0.45f, 0.45f };
            color = states[i] == 2 ? 0xFFFFFFFFU : 0x000000B0U;
            ttyd::win_main::winIconSet(icon, &position, &scale, &color);
        }
    }

    // Draw large tile that shows current tile's reward.
    {
        int32_t cursor = menu->achievement_log_cursor_idx;
        int32_t cursor_state = cursor >= 0 ? states[cursor] : 2;
        uint32_t color = 0x505050FFU;
        switch (cursor_state) {
            case 1:  color = 0xb0b0b0FFU;  break;
            case 2:  color = 0xFFFFFFFFU;  break;
        }
        gc::vec3 position = { win_x - 21.0f, win_y - 50.0f, 0.0f };
        gc::vec3 scale = { 1.5f, 1.5f, 1.5f };
        ttyd::win_main::winTexInit(*menu->win_tpl->mpFileData);
        ttyd::win_main::winTexSet(
            0x7b + (cursor_state == 2 ? 13 : 0), &position, &scale, &color);

        if (cursor_state) {
            int32_t icon = 0;
            const char* name_msg = "";
            GetAchievementRewardDetails(grid, cursor, &icon, &name_msg);

            // Draw icon.
            ttyd::win_main::winIconInit();
            gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
            color = cursor_state == 2 ? 0xFFFFFFFFU : 0x000000FFU;
            ttyd::win_main::winIconSet(icon, &position, &scale, &color);

            // Draw nameplate.
            ttyd::win_root::winNameGX(
                position.x - 85.0f, position.y - 42.0f, 170.0f, 0.0f, menu, 1);

            if (cursor_state < 2) name_msg = "msg_menu_stone_none_help";
            const char* name = msgSearch(name_msg);
            int32_t width = ttyd::fontmgr::FontGetMessageWidth(name);
            float x_scale = 1.0f;
            float max_width = 180.0f;
            if (width > max_width) {
                x_scale = max_width / width;
                width = max_width;
            }

            ttyd::win_main::winFontInit();
            gc::vec3 text_position = { 
                position.x - width * 0.75f * 0.5f,
                position.y - 48.f,
                0.0f
            };
            gc::vec3 text_scale = { 0.75f * x_scale, 0.75f, 0.75f };
            uint32_t text_color = 0xFFFFFFFFU;
            ttyd::win_main::winFontSetEdgeWidth(
                &text_position, &text_scale, &text_color, max_width, name);
        }
    }

    // Draw hammers.
    for (int32_t i = 0; i < 5; ++i) {
        ttyd::win_main::winIconInit();
        gc::vec3 position = {
            win_x - 80.0f + 30.0f * i, win_y - 155.0f - 10.0f * (i & 1), 0.0f
        };
        gc::vec3 scale = { 0.5f, 0.5f, 0.5f };
        uint32_t color =
            i < menu->achievement_log_hammers ? 0x00000080U : 0x000000FFU;
        ttyd::win_main::winIconSet(IconType::HAMMER, &position, &scale, &color);

        if (i < menu->achievement_log_hammers) {
            position.x -= 2.0f;
            position.y += 2.0f;
            color = 0xFFFFFFFFU;
            // Darken the last hammer if currently in use.
            if (i == menu->achievement_log_hammers - 1 && 
                menu->achievement_log_using_hammer) {
                color = 0xB0B0B0FFU;
            }
            ttyd::win_main::winIconSet(IconType::HAMMER, &position, &scale, &color);
        }
    }

    // Draw window with number of completed achievements.
    ttyd::win_root::winKirinukiGX(
        win_x - 71.0f, win_y - 190.0f, 110.0f, 41.0f, menu, 0);
    {
        ttyd::win_main::winFontInit();
        char buf[8];
        sprintf(
            buf, "%" PRId32 "/%" PRId32,
            menu->achievement_log_completed_count,
            menu->achievement_log_total_count);
        int32_t width = ttyd::fontmgr::FontGetMessageWidth(buf);
        gc::vec3 position = {
            win_x - 0.0f - 0.75f * width * 0.5f,
            win_y - 201.0f,
            0.0f
        };
        gc::vec3 scale = { 0.75f, 0.75f, 0.75f };
        uint32_t color = 0x000000FFU;
        ttyd::win_main::winFontSet(&position, &scale, &color, buf);
    }
    {
        // Draw ribbon next to completed number.
        ttyd::win_main::winTexInit(*menu->win_tpl->mpFileData);
        gc::vec3 position = { win_x - 46.0f, win_y - 210.0f, 0.0f };
        gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
        uint32_t color = 0xFFFFFFFFU;
        ttyd::win_main::winTexSet(0xb0, &position, &scale, &color);
    }

    // Draw flashing red boxes on unbreakable tiles when using a hammer.
    if (menu->achievement_log_using_hammer &&
        !menu->achievement_log_unlock_timer) {
        // Pulse ~once every second, using absolute cosine curve.
        int32_t alpha = 0x20 + 0x60 * AbsF(ttyd::_core_cos((
                (ttyd::mariost::g_MarioSt->currentRetraceCount & 0x7f)
                * 3.14159265f / 64.0f
            )));

        for (int32_t i = 0; i < 70; ++i) {
            // Skip corners and already unlocked tiles.
            if (i == 0 || i == 9 || i == 60 || i == 69) continue;
            if (states[i] == 2) continue;
            // Skip tiles that are breakable, but not currently selected.
            if (!IsUnbreakable(i) && i != menu->achievement_log_cursor_idx)
                continue;
            
            ttyd::win_main::winIconInit();
            gc::vec3 position = {
                win_x + 101.0f + 36.0f * (i % 10),
                win_y - 10.0f - 36.0f * (i / 10),
                0.0f
            };
            gc::vec3 scale = { 1.125f, 1.125f, 1.125f };
            // Flash unbreakable tiles red, selected breakable one white.
            uint32_t color =
                (IsUnbreakable(i) ? 0xFF000000U : 0xFFFFFF00U) | alpha;
            ttyd::win_main::winIconSet(
                IconType::TOT_BLANK, &position, &scale, &color);
        }
    }
}

void DrawMoveLog(WinPauseMenu* menu, float win_x, float win_y) {
    const auto& state = g_Mod->state_;

    // Draw book background.
    ttyd::win_root::winBookGX(win_x, win_y, menu, 1);
    
    int32_t page_number = menu->move_log_cursor_idx < 24
        ? menu->move_log_cursor_idx / 8 : (menu->move_log_cursor_idx) / 6 - 1;
    int32_t page_first_entry =
        page_number < 3 ? page_number * 8 : (page_number + 1) * 6;
    int32_t page_last_entry = (page_number < 3 ? 8 : 6) + page_first_entry;

    int32_t page_unlocked = true;
    if (uint32_t partner_flags = state.GetOption(STAT_PERM_PARTNERS_OBTAINED);
        page_number >= 3 && !(partner_flags & (1 << (page_number - 2)))) {
        page_unlocked = false;
    }

    // Draw header for page.
    {
        const char* msg;
        int32_t costume = GetSWByte(GSW_MarioCostume);
        int32_t header_icon = CosmeticsManager::GetMarioCostumeData(costume)->icon;
        switch (page_number) {
            case 0:
                msg = "tot_movelog_jump";
                break;
            case 1:
                msg = "tot_movelog_hammer";
                break;
            case 2:
                msg = "tot_movelog_special";
                break;
            default: {
                if (page_unlocked) {
                    msg =
                        ttyd::win_party::g_winPartyDt[page_number - 3].name;
                    header_icon = 
                        ttyd::win_party::g_winPartyDt[page_number - 3].icon_id;
                    // Always display Yoshi as green if not currently in party.
                    if (page_number == 6 && 
                        !(ttyd::mario_pouch::pouchGetPtr()->party_data[4].flags & 1)) {
                        header_icon = IconType::YOSHI_GREEN;
                    }
                } else {
                    msg = "msg_menu_stone_none_help";
                    header_icon = -1;
                }
                break;
            }
        }

        {
            if (page_unlocked) {
                ttyd::win_main::winIconInit();
                gc::vec3 position = { win_x + 25.0f, win_y - 24.0f, 0.0f };
                gc::vec3 scale = { 0.6f, 0.6f, 0.6f };
                uint32_t color = 0xFFFFFFFFU;
                ttyd::win_main::winIconSet(header_icon, &position, &scale, &color);
            }

            ttyd::win_main::winFontInit();
            {
                gc::vec3 position = { win_x + 43.0f, win_y - 16.0f, 0.0f };
                gc::vec3 scale = { 0.75f, 0.75f, 0.75f };
                uint32_t color = 0x403030FFU;
                ttyd::win_main::winFontSet(&position, &scale, &color, msgSearch(msg));
            }
            {
                gc::vec3 position = { win_x + 245.0f, win_y - 16.0f, 0.0f };
                gc::vec3 scale = { 0.75f, 0.75f, 0.75f };
                uint32_t color = 0x403030FFU;
                ttyd::win_main::winFontSet(
                    &position, &scale, &color, msgSearch("tot_movelog_found"));
            }
            {
                gc::vec3 position = { win_x + 310.0f, win_y - 16.0f, 0.0f };
                gc::vec3 scale = { 0.75f, 0.75f, 0.75f };
                uint32_t color = 0x403030FFU;
                ttyd::win_main::winFontSet(
                    &position, &scale, &color, msgSearch("tot_movelog_used"));
            }
            {
                gc::vec3 position = { win_x + 370.0f, win_y - 16.0f, 0.0f };
                gc::vec3 scale = { 0.75f, 0.75f, 0.75f };
                uint32_t color = 0x403030FFU;
                ttyd::win_main::winFontSet(
                    &position, &scale, &color, msgSearch("tot_movelog_stylish"));
            }
        }
    }

    // Draw entries for each move.
    for (int32_t move = page_first_entry; move < page_last_entry; ++move) {
        float base_y = win_y - 16.0f - 24.0f * ((move - page_first_entry) + 1);
        const auto* move_data = MoveManager::GetMoveData(move);
        uint32_t move_flags = g_Mod->state_.GetOption(STAT_PERM_MOVE_LOG, move);
        if (!page_unlocked) move_flags = 0;

        // Print move name, and print icon if unlocked.
        {
            const char* name_msg = (move_flags & MoveLogFlags::UNLOCKED_LV_1) 
                ? move_data->name_msg : "msg_menu_stone_none_help";
            ttyd::win_main::winFontInit();
            gc::vec3 position = { win_x + 43.0f, base_y, 0.0f };
            gc::vec3 scale = { 0.75f, 0.75f, 0.75f };
            uint32_t color = 0x000000FFU;
            ttyd::win_main::winFontSet(
                &position, &scale, &color, msgSearch(name_msg));
        }
        ttyd::win_main::winIconInit();
        if (move_flags & MoveLogFlags::UNLOCKED_LV_1) {
            gc::vec3 position = { win_x + 25.0f, base_y - 8.0f, 0.0f };
            gc::vec3 scale = { 0.45f, 0.45f, 0.45f };
            uint32_t color = 0xFFFFFFFFU;
            ttyd::win_main::winIconSet(
                move_data->icon_id, &position, &scale, &color);
        }

        bool move_completed = true;

        // Print pips for levels unlocked, levels used, and Stylish.
        for (int32_t i = 0; i < move_data->max_level; ++i) {
            uint32_t flag = MoveLogFlags::UNLOCKED_LV_1;
            switch (i) {
                case 1: flag = MoveLogFlags::UNLOCKED_LV_2; break;
                case 2: flag = MoveLogFlags::UNLOCKED_LV_3; break;
            }
            int32_t icon = IconType::SP_ORB_EMPTY;
            if (move_flags & flag) {
                icon = IconType::AC_LIGHT_GREEN + i;
            } else {
                move_completed = false;
            }
            gc::vec3 position = { win_x + 250.0f + 18.0f * i, base_y - 8.0f, 0.0f };
            gc::vec3 scale = { 0.6f, 0.6f, 0.6f };
            uint32_t color = 0xFFFFFFFFU;
            ttyd::win_main::winIconSet(icon, &position, &scale, &color);
        }
        for (int32_t i = 0; i < move_data->max_level; ++i) {
            uint32_t flag = MoveLogFlags::USED_LV_1;
            switch (i) {
                case 1: flag = MoveLogFlags::USED_LV_2; break;
                case 2: flag = MoveLogFlags::USED_LV_3; break;
            }
            int32_t icon = IconType::SP_ORB_EMPTY;
            if (move_flags & flag) {
                icon = IconType::AC_LIGHT_GREEN + i;
            } else {
                move_completed = false;
            }
            gc::vec3 position = { win_x + 315.0f + 18.0f * i, base_y - 8.0f, 0.0f };
            gc::vec3 scale = { 0.6f, 0.6f, 0.6f };
            uint32_t color = 0xFFFFFFFFU;
            ttyd::win_main::winIconSet(icon, &position, &scale, &color);
        }
        if (page_number != 2 && move != MoveType::GOOMBELLA_RALLY_WINK) {
            uint32_t flag = MoveLogFlags::STYLISH_ALL;
            int32_t icon = IconType::SP_ORB_EMPTY;
            if ((move_flags & flag) == flag) {
                icon = IconType::AC_LIGHT_GREEN;
            } else {
                move_completed = false;
            }
            gc::vec3 position = { win_x + 385.0f, base_y - 8.0f, 0.0f };
            gc::vec3 scale = { 0.6f, 0.6f, 0.6f };
            uint32_t color = 0xFFFFFFFFU;
            ttyd::win_main::winIconSet(icon, &position, &scale, &color);
        }

        // Draw a ribbon if all flags for the move were completed.
        if (move_completed) {
            ttyd::win_main::winTexInit(*menu->win_tpl->mpFileData);
            gc::vec3 position = { win_x + 420.0f, base_y - 8.0f, 0.0f };
            gc::vec3 scale = { 0.8f, 0.8f, 0.8f };
            uint32_t color = 0xFFFFFFFFU;
            ttyd::win_main::winTexSet(0xb0, &position, &scale, &color);
        }
    }

    // Draw window with move obtained / completed counts.
    ttyd::win_root::winKirinukiGX(
        win_x - 110.0f, win_y - 140.0f, 100.0f, 81.0f, menu, 0);
    ttyd::win_main::winFontInit();
    {
        const char* msg = msgSearch("tot_movelog_movecount");
        int32_t width = ttyd::fontmgr::FontGetMessageWidth(msg);
        gc::vec3 position = {
            win_x - 60.0f - 0.5f * 0.75f * width,
            win_y - 148.0f,
            0.0f
        };
        gc::vec3 scale = { 0.75f, 0.75f, 0.75f };
        uint32_t color = 0x000000FFU;
        ttyd::win_main::winFontSet(&position, &scale, &color, msg);
    }
    {
        char buf[8];
        sprintf(
            buf, "%" PRId32 "/%" PRId32, 
            menu->move_log_obtained_count, MoveType::MOVE_TYPE_MAX);
        int32_t width = ttyd::fontmgr::FontGetMessageWidth(buf);
        gc::vec3 position = {
            win_x - 32.0f - 0.75f * width,
            win_y - 166.0f,
            0.0f
        };
        gc::vec3 scale = { 0.75f, 0.75f, 0.75f };
        uint32_t color = 0x000000FFU;
        ttyd::win_main::winFontSet(&position, &scale, &color, buf);
    }
    {
        char buf[4];
        sprintf(
            buf, "x%s%" PRId32,
            menu->move_log_completed_count < 10 ? " " : "",
            menu->move_log_completed_count);
        int32_t width = ttyd::fontmgr::FontGetMessageWidth(buf);
        gc::vec3 position = {
            win_x - 32.0f - 0.75f * width,
            win_y - 190.0f,
            0.0f
        };
        gc::vec3 scale = { 0.75f, 0.75f, 0.75f };
        uint32_t color = 0x000000FFU;
        ttyd::win_main::winFontSet(&position, &scale, &color, buf);
    }
    {
        // Draw ribbon next to completed number.
        ttyd::win_main::winTexInit(*menu->win_tpl->mpFileData);
        gc::vec3 position = { win_x - 80.0f, win_y - 200.0f, 0.0f };
        gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
        uint32_t color = 0xFFFFFFFFU;
        ttyd::win_main::winTexSet(0xb0, &position, &scale, &color);
    }

    // Draw L/R button icon / arrows.
    if (page_number != 0) {
        {
            ttyd::win_main::winTexInit(*menu->win_tpl->mpFileData);
            gc::vec3 position = { win_x + 440.0f, win_y + 18.0f, 0.0f };
            gc::vec3 scale = { -1.0f, -1.0f, -1.0f };
            uint32_t color = 0xFFFFFFFFU;
            ttyd::win_main::winTexSet(0x17, &position, &scale, &color);
        }
        {
            ttyd::win_main::winIconInit();        
            gc::vec3 position = { win_x + 440.0f, win_y, 0.0f };
            gc::vec3 scale = { 0.6f, 0.6f, 0.6f };
            uint32_t color = 0xFFFFFFFFU;
            ttyd::win_main::winIconSet(
                IconType::L_BUTTON, &position, &scale, &color);
        }
    }
    if (page_number != 9) {
        {
            ttyd::win_main::winTexInit(*menu->win_tpl->mpFileData);
            gc::vec3 position = { win_x + 440.0f, win_y - 255.0f, 0.0f };
            gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
            uint32_t color = 0xFFFFFFFFU;
            ttyd::win_main::winTexSet(0x17, &position, &scale, &color);
        }
        
        {
            ttyd::win_main::winIconInit();
            gc::vec3 position = { win_x + 440.0f, win_y - 240.0f, 0.0f };
            gc::vec3 scale = { 0.6f, 0.6f, 0.6f };
            uint32_t color = 0xFFFFFFFFU;
            ttyd::win_main::winIconSet(
                IconType::R_BUTTON, &position, &scale, &color);
        }
    }
}

void DrawRecordsLog(WinPauseMenu* menu, float win_x, float win_y) {
    const auto& state = g_Mod->state_;

    // Draw book background.
    ttyd::win_root::winBookGX(win_x, win_y, menu, 1);
    
    int32_t page_first_entry = menu->records_log_cursor_idx / 9 * 9;
    int32_t page_last_entry = page_first_entry + 9;

    ttyd::win_main::winFontInit();
    for (int32_t i = page_first_entry; i < page_last_entry; ++i) {
        const auto& record = kRecordLogEntries[i];
        const float pos_y = win_y - 16.0f - (i - page_first_entry) * 24.0f;

        // Draw entry's name text.
        {
            gc::vec3 position = { win_x + 30.0f, pos_y, 0.0f };
            gc::vec3 scale = { 0.75f, 0.75f, 0.75f };
            uint32_t color = 0x000000FFU;
            const char* name =
                record.name_msg ? msgSearch(record.name_msg) : "";

            // Hide some descriptions until certain progression thresholds,
            // unless using special filename modes.
            if (!state.CheckOptionValue(OPTVAL_RACE_MODE_ENABLED) &&
                !state.CheckOptionValue(OPTVAL_100_MODE_ENABLED)) {
                switch (record.option) {
                    case STAT_PERM_FULL_FINISHES:
                    case STAT_PERM_FULL_BEST_TIME:
                        if (state.GetOption(STAT_PERM_FULL_ATTEMPTS) < 1)
                            name = "???";
                        break;
                    case STAT_PERM_EX_FINISHES:
                    case STAT_PERM_EX_BEST_TIME:
                        if (state.GetOption(STAT_PERM_FULL_FINISHES) < 1)
                            name = "???";
                        break;
                    case STAT_PERM_MAX_INTENSITY:
                        if (!GetSWF(GSWF_RunOptionsTutorial)) 
                            name = "???";
                        break;
                    case STAT_RUN_NPCS_DEALT_WITH:
                        if (state.GetOption(STAT_PERM_NPC_DEALS_TOTAL) < 1) {
                            name = msgSearch("tot_recn_unkdeals");
                        }
                        break;
                    case STAT_PERM_NPC_DEALS_TOTAL:
                    case STAT_PERM_NPC_WONKY_TRADES:
                    case STAT_PERM_NPC_DAZZLE_TRADES:
                    case STAT_PERM_NPC_RIPPO_TRADES:
                    case STAT_PERM_NPC_LUMPY_TRADES:
                    case STAT_PERM_NPC_GRUBBA_DEAL:
                    case STAT_PERM_NPC_DOOPLISS_DEAL:
                    case STAT_PERM_NPC_MOVER_TRADES:
                    case STAT_PERM_NPC_ZESS_COOKS:
                        if (state.GetOption(record.option) < 1) {
                            name = msgSearch("tot_recn_unkdeals");
                        }
                        break;
                    case REC_HUB_ATTACK_FX: {
                        int32_t cur, tot;
                        ProgressManager::GetHubAttackFXProgress(&cur, &tot);
                        if (cur < 1) name = "???";
                        break;
                    }
                    case REC_HUB_MARIO_SKINS: {
                        int32_t cur, tot;
                        ProgressManager::GetHubMarioCostumeProgress(&cur, &tot);
                        if (cur < 1) name = "???";
                        break;
                    }
                    case REC_HUB_YOSHI_SKINS: {
                        int32_t cur, tot;
                        ProgressManager::GetHubYoshiCostumeProgress(&cur, &tot);
                        if (cur < 1) name = "???";
                        break;
                    }
                }
            }

            // Handle special coloring for section headers.
            switch (record.option) {    
                case REC_EMPTY: {
                    color = 0x403030FFU;
                    break;
                }
                case REC_BEST_TIMES: {
                    color = 0x403030FFU;
                    if (state.CheckOptionValue(OPTVAL_RACE_MODE_ENABLED)) {
                        name = msgSearch("tot_recn_times_rta");
                    }
                    break;
                }
            }

            ttyd::win_main::winFontSet(&position, &scale, &color, name);
        }

        // Draw entry's value text.
        if (record.option != REC_EMPTY) {
            gc::vec3 position = { win_x + 250.0f, pos_y, 0.0f };
            gc::vec3 scale = { 0.75f, 0.75f, 0.75f };
            uint32_t color = 0x000000FFU;

            static char buf[64];
            buf[0] = '\0';
            char* ptr = buf;
            switch (record.option) {
                case REC_PLAY_TIME: {
                    ptr += DurationTicksToFmtString(
                        ttyd::mariost::g_MarioSt->lastFrameRetraceLocalTime, ptr);
                    // Remove centiseconds from end of string.
                    ptr[-3] = '\0';
                    break;
                }
                case REC_COMPLETION_PCT: {
                    sprintf(
                        ptr, "%.1f%%", 
                        ProgressManager::GetOverallProgression() * 0.01f);
                    break;
                }
                case REC_ACHIEVEMENT_PCT: {
                    int32_t cur, tot;
                    ProgressManager::GetAchievementLogProgress(&cur, &tot);
                    sprintf(ptr, "%" PRId32 " / %" PRId32, cur, tot);
                    break;
                }
                case REC_ITEM_PCT: {
                    int32_t cur, tot;
                    ProgressManager::GetItemLogProgress(&cur, &tot);
                    sprintf(ptr, "%" PRId32 " / %" PRId32, cur, tot);
                    break;
                }
                case REC_BADGE_PCT: {
                    int32_t cur, tot;
                    ProgressManager::GetBadgeLogProgress(&cur, &tot);
                    sprintf(ptr, "%" PRId32 " / %" PRId32, cur, tot);
                    break;
                }
                case REC_MOVE_PCT: {
                    // Show number of moves obtained out of the total.
                    int32_t obt, tot;
                    ProgressManager::GetMoveLogProgress(&obt, nullptr, &tot);
                    sprintf(
                        ptr, "%" PRId32 " / %" PRId32, obt, tot);
                    break;
                }
                case REC_MOVE_CMP_PCT: {
                    // Show number of moves completed out of the total.
                    int32_t com, tot;
                    int32_t score =
                        ProgressManager::GetMoveLogProgress(nullptr, &com, &tot);
                    sprintf(
                        ptr, "%" PRId32 " / %" PRId32 " (%.1f%%)", 
                        com, tot, score * 0.01f);
                    break;
                }
                case REC_TATTLE_PCT: {
                    int32_t cur, tot;
                    ProgressManager::GetTattleLogProgress(&cur, &tot);
                    sprintf(ptr, "%" PRId32 " / %" PRId32, cur, tot);
                    break;
                }
                case REC_HUB_PROGRESS_PCT: {
                    sprintf(
                        ptr, "%.1f%%", 
                        ProgressManager::GetOverallHubProgression() * 0.01f);
                    break;
                }
                case REC_HUB_KEY_ITEMS: {
                    int32_t cur, tot;
                    ProgressManager::GetKeyItemProgress(&cur, &tot);
                    sprintf(ptr, "%" PRId32 " / %" PRId32, cur, tot);
                    break;
                }
                case REC_HUB_ITEMS: {
                    int32_t cur, tot;
                    ProgressManager::GetHubItemProgress(&cur, &tot);
                    sprintf(ptr, "%" PRId32 " / %" PRId32, cur, tot);
                    break;
                }
                case REC_HUB_BADGES: {
                    int32_t cur, tot;
                    ProgressManager::GetHubBadgeProgress(&cur, &tot);
                    sprintf(ptr, "%" PRId32 " / %" PRId32, cur, tot);
                    break;
                }
                case REC_HUB_ATTACK_FX: {
                    int32_t cur, tot;
                    ProgressManager::GetHubAttackFXProgress(&cur, &tot);
                    if (cur > 0) {
                        sprintf(ptr, "%" PRId32 " / %" PRId32, cur, tot);
                    } else {
                        sprintf(ptr, "???");
                    }
                    break;
                }
                case REC_HUB_MARIO_SKINS: {
                    int32_t cur, tot;
                    ProgressManager::GetHubMarioCostumeProgress(&cur, &tot);
                    if (cur > 0) {
                        sprintf(ptr, "%" PRId32 " / %" PRId32, cur, tot);
                    } else {
                        sprintf(ptr, "???");
                    }
                    break;
                }
                case REC_HUB_YOSHI_SKINS: {
                    int32_t cur, tot;
                    ProgressManager::GetHubYoshiCostumeProgress(&cur, &tot);
                    if (cur > 0) {
                        sprintf(ptr, "%" PRId32 " / %" PRId32, cur, tot);
                    } else {
                        sprintf(ptr, "???");
                    }
                    break;
                }
                case REC_TOTAL_ATTEMPTS: {
                    int32_t count = 0;
                    count += state.GetOption(STAT_PERM_HALF_ATTEMPTS);
                    count += state.GetOption(STAT_PERM_FULL_ATTEMPTS);
                    count += state.GetOption(STAT_PERM_EX_ATTEMPTS);
                    sprintf(ptr, "%" PRId32, count);
                    break;
                }
                case REC_TOTAL_FINISHES: {
                    int32_t count = 0;
                    count += state.GetOption(STAT_PERM_HALF_FINISHES);
                    count += state.GetOption(STAT_PERM_FULL_FINISHES);
                    count += state.GetOption(STAT_PERM_EX_FINISHES);
                    sprintf(ptr, "%" PRId32, count);
                    break;
                }
                case REC_UNIQUE_MIDBOSSES: {
                    int32_t count = 0;
                    for (int32_t i = 0; i < 128; ++i) {
                        count += state.GetOption(FLAGS_MIDBOSS_DEFEATED, i);
                    }
                    sprintf(ptr, "%" PRId32, count);
                    break;
                }
                case STAT_PERM_MAX_INTENSITY: {
                    if (GetSWF(GSWF_RunOptionsTutorial)) {
                        sprintf(
                            ptr, "%" PRId32 "%%", 
                            state.GetOption(STAT_PERM_MAX_INTENSITY));
                    } else {
                        sprintf(ptr, "???");
                    }
                    break;
                }
                case STAT_PERM_HALF_FINISHES: {
                    ptr += IntegerToFmtString(
                        state.GetOption(STAT_PERM_HALF_FINISHES), ptr);
                    ptr += sprintf(ptr, " / ");
                    ptr += IntegerToFmtString(
                        state.GetOption(STAT_PERM_HALF_ATTEMPTS), ptr);
                    break;
                }
                case STAT_PERM_FULL_FINISHES: {
                    ptr += IntegerToFmtString(
                        state.GetOption(STAT_PERM_FULL_FINISHES), ptr);
                    ptr += sprintf(ptr, " / ");
                    ptr += IntegerToFmtString(
                        state.GetOption(STAT_PERM_FULL_ATTEMPTS), ptr);
                    break;
                }
                case STAT_PERM_EX_FINISHES: {
                    ptr += IntegerToFmtString(
                        state.GetOption(STAT_PERM_EX_FINISHES), ptr);
                    ptr += sprintf(ptr, " / ");
                    ptr += IntegerToFmtString(
                        state.GetOption(STAT_PERM_EX_ATTEMPTS), ptr);
                    break;
                }
                case REC_BEST_TIMES: {
                    sprintf(ptr, msgSearch("tot_recn_times2"));
                    color = 0x403030FFU;
                    scale.x = 0.50;
                    scale.y = 0.50;
                    position.x -= 5;
                    position.y -= 5;
                    break;
                }
                case STAT_PERM_HALF_BEST_TIME:
                case STAT_PERM_FULL_BEST_TIME:
                case STAT_PERM_EX_BEST_TIME: {
                    ptr += DurationCentisToFmtString(
                        state.GetOption(record.option), ptr);
                    break;
                }
                case STAT_PERM_CONDITIONS_MET: {
                    ptr += IntegerToFmtString(
                        state.GetOption(STAT_PERM_CONDITIONS_MET), ptr);
                    ptr += sprintf(ptr, " / ");
                    ptr += IntegerToFmtString(
                        state.GetOption(STAT_PERM_CONDITIONS_TOTAL), ptr);
                    break;
                }
                case STAT_RUN_TURNS_SPENT: {
                    ptr += IntegerToFmtString(
                        state.GetOption(STAT_RUN_TURNS_SPENT), ptr);
                    sprintf(
                        ptr, " (Max: %" PRId32 ")",
                        state.GetOption(STAT_RUN_MOST_TURNS_RECORD));
                    break;
                }
                case STAT_RUN_CONDITIONS_MET: {
                    ptr += IntegerToFmtString(
                        state.GetOption(STAT_RUN_CONDITIONS_MET), ptr);
                    ptr += sprintf(ptr, " / ");
                    ptr += IntegerToFmtString(
                        state.GetOption(STAT_RUN_CONDITIONS_TOTAL), ptr);
                    break;
                }
                case STAT_RUN_NPCS_DEALT_WITH: {
                    int32_t deals = 0;
                    for (int32_t i = 0; i < 8; ++i) {
                        deals += !!state.GetOption(STAT_RUN_NPCS_DEALT_WITH, i);
                    }
                    sprintf(ptr, "%" PRId32, deals);
                    break;
                }
                default: {
                    ptr += IntegerToFmtString(
                        state.GetOption(record.option), ptr);
                    break;
                }
            }

            ttyd::win_main::winFontSet(&position, &scale, &color, buf);
        }
    }

    // Draw L/R button icon / arrows.
    int32_t min_entry = g_Mod->state_.GetOption(OPT_RUN_STARTED) ? 0 : 18;
    if (page_first_entry > min_entry) {
        {
            ttyd::win_main::winTexInit(*menu->win_tpl->mpFileData);
            gc::vec3 position = { win_x + 440.0f, win_y + 18.0f, 0.0f };
            gc::vec3 scale = { -1.0f, -1.0f, -1.0f };
            uint32_t color = 0xFFFFFFFFU;
            ttyd::win_main::winTexSet(0x17, &position, &scale, &color);
        }
        {
            ttyd::win_main::winIconInit();        
            gc::vec3 position = { win_x + 440.0f, win_y, 0.0f };
            gc::vec3 scale = { 0.6f, 0.6f, 0.6f };
            uint32_t color = 0xFFFFFFFFU;
            ttyd::win_main::winIconSet(
                IconType::L_BUTTON, &position, &scale, &color);
        }
    }
    if (page_last_entry < kNumRecordLogEntries) {
        {
            ttyd::win_main::winTexInit(*menu->win_tpl->mpFileData);
            gc::vec3 position = { win_x + 440.0f, win_y - 255.0f, 0.0f };
            gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
            uint32_t color = 0xFFFFFFFFU;
            ttyd::win_main::winTexSet(0x17, &position, &scale, &color);
        }
        
        {
            ttyd::win_main::winIconInit();
            gc::vec3 position = { win_x + 440.0f, win_y - 240.0f, 0.0f };
            gc::vec3 scale = { 0.6f, 0.6f, 0.6f };
            uint32_t color = 0xFFFFFFFFU;
            ttyd::win_main::winIconSet(
                IconType::R_BUTTON, &position, &scale, &color);
        }
    }
    
}

}  // namespace

void ReplaceLogSortMethods() {
    // Replace sort functions for item/recipe logs to cover entire item range.
    ttyd::win_root::sort_5[0].func = (void*)SortBadgeLogABC;
    ttyd::win_root::sort_5[1].func = (void*)SortBadgeLogType;
    ttyd::win_root::sort_5[2].func = (void*)SortBadgeLogBP;
    ttyd::win_root::sort_6[0].func = (void*)SortItemLogABC;
    ttyd::win_root::sort_6[1].func = (void*)SortItemLogType;

    // Remove "by location" sort for Tattle log.
    // Could eventually replace with "# defeated" if that's implemented?
    ttyd::win_root::sort_7[1].type = "msg_menu_sort_aiueo";
    ttyd::win_root::sort_7[1].func = (void*)ttyd::win_root::sort_7_3_func;
    ttyd::win_root::sort_7[2].type = nullptr;
    ttyd::win_root::sort_7[2].func = nullptr;
}

void LogMenuInit(ttyd::win_root::WinPauseMenu* menu) {
    ProgressManager::RefreshCache();

    menu->log_menu_state = 0;
    menu->log_submenu_cursor_idx = 0;
    menu->log_submenu_count = 6;
    for (int32_t i = 0; i < menu->log_submenu_count; ++i) {
        auto& submenu = menu->log_submenu_info[i];
        const float banner_x = 120.f * (i < 3 ? -1 : 1);
        const float banner_y = 45.0f * (1.5f - i % 3);
        submenu.x = banner_x;
        submenu.target_x = banner_x;
        submenu.y = banner_y;
        submenu.target_y = banner_y;
        submenu.state = 0;
        submenu.timer = 0;
    }
    menu->log_submenu_info[0].id = 6;
    menu->log_submenu_info[0].help_msg = "msg_menu_achievements";
    menu->log_submenu_info[1].id = 3;
    menu->log_submenu_info[1].help_msg = "msg_menu_kiroku_ryori";
    menu->log_submenu_info[2].id = 2;
    menu->log_submenu_info[2].help_msg = "msg_menu_kiroku_badge";
    menu->log_submenu_info[3].id = 5;
    menu->log_submenu_info[3].help_msg = "msg_menu_move_log";
    menu->log_submenu_info[4].id = 4;
    menu->log_submenu_info[4].help_msg = "msg_menu_kiroku_mono";
    menu->log_submenu_info[5].id = 7;
    menu->log_submenu_info[5].help_msg = "msg_menu_records";
  
    menu->badge_log_total_count = 0;
    menu->badge_log_obtained_count = 0;
    for (int32_t i = ItemType::POWER_JUMP; i < ItemType::MAX_ITEM_TYPE; ++i) {
        if (itemDataTable[i].type_sort_order != -1) {
            menu->badge_log_ids[menu->badge_log_total_count++] = i - 0x80;
            if (g_Mod->state_.GetOption(FLAGS_ITEM_ENCOUNTERED, (i - 0x80)))
                ++menu->badge_log_obtained_count;
        }
    }
    ttyd::system::qqsort(
        menu->badge_log_ids, menu->badge_log_total_count, 2, (void*)CompBadgeItemId);
    menu->badge_log_cursor_idx = 0;
    menu->badge_log_page_num = 0;
    menu->badge_log_win_offset_x = 320.0f;
    menu->badge_log_win_target_x = 320.0f;
    menu->badge_log_win_offset_y = -240.0f;
    menu->badge_log_win_target_y = -240.0f;
    menu->badge_log_scroll_y = 0.0f;
    menu->badge_log_scroll_target_y = 0.0f;
    menu->badge_log_showcased_x = -200.0f;
    menu->badge_log_showcased_target_x = -200.0f;
    menu->badge_log_showcased_y = -300.0f;
    menu->badge_log_showcased_target_y = -300.0f;

    menu->recipe_log_total_count = 0;
    menu->recipe_log_obtained_count = 0;
    for (int32_t i = ItemType::THUNDER_BOLT; i <= ItemType::FRESH_JUICE; ++i) {
        if (itemDataTable[i].type_sort_order != -1) {
            menu->recipe_log_ids[menu->recipe_log_total_count++] = i - 0x80;
            if (g_Mod->state_.GetOption(FLAGS_ITEM_ENCOUNTERED, (i - 0x80)))
                ++menu->recipe_log_obtained_count;
        }
    }
    ttyd::system::qqsort(
        menu->recipe_log_ids, menu->recipe_log_total_count, 2, (void*)CompBadgeItemId);
    menu->recipe_log_cursor_idx = 0;
    menu->recipe_log_page_num = 0;
    menu->recipe_log_win_offset_x = 320.0f;
    menu->recipe_log_win_target_x = 320.0f;
    menu->recipe_log_win_offset_y = -240.0f;
    menu->recipe_log_win_target_y = -240.0f;
    menu->recipe_log_scroll_y = 0.0f;
    menu->recipe_log_scroll_target_y = 0.0f;
    menu->recipe_log_showcased_x = -200.0f;
    menu->recipe_log_showcased_target_x = -200.0f;
    menu->recipe_log_showcased_y = -300.0f;
    menu->recipe_log_showcased_target_y = -300.0f;
  
    ProgressManager::GetTattleLogProgress(
        &menu->tattle_log_obtained_count, &menu->tattle_log_total_count);

    int32_t num_tattles = 0;
    for (int32_t i = 1; i < 0xd8; ++i) {
        int32_t custom_tattle_idx = GetCustomTattleIndex(i);
        if (custom_tattle_idx >= 0 &&
            custom_tattle_idx <= menu->tattle_log_total_count) {
            menu->tattle_logs[num_tattles].order = 
                ttyd::win_root::enemy_monoshiri_sort_table[i];
            menu->tattle_logs[num_tattles].id = i;
            ++num_tattles;
        }
    }
    // Bubble sort tattle log entries by type-sort order.
    for (int32_t i = 0; i < menu->tattle_log_total_count; ++i) {
        for (int32_t j = i + 1; j < menu->tattle_log_total_count; ++j) {
            if (menu->tattle_logs[j].order < menu->tattle_logs[i].order) {
                int32_t tmp_sort = menu->tattle_logs[j].order;
                int32_t tmp_type = menu->tattle_logs[j].id;
                menu->tattle_logs[j].order = menu->tattle_logs[i].order;
                menu->tattle_logs[j].id = menu->tattle_logs[i].id;
                menu->tattle_logs[i].order = tmp_sort;
                menu->tattle_logs[i].id = tmp_type;
            }
        }
    }
    menu->tattle_log_cursor_idx = 0;
    menu->tattle_log_page_num = 0;

    menu->move_log_cursor_idx = 0;
    ProgressManager::GetMoveLogProgress(
        &menu->move_log_obtained_count, &menu->move_log_completed_count, nullptr);

    // Records log has current run stats first, if currently in a run.
    menu->records_log_cursor_idx =
        g_Mod->state_.GetOption(OPT_RUN_STARTED) ? 1 : 19;

    menu->achievement_log_cursor_idx = 33;
    menu->achievement_log_completed_count = 0;
    menu->achievement_log_total_count = 0;
    for (int32_t i = 0; i < AchievementId::MAX_ACHIEVEMENT; ++i) {
        bool completed =
            g_Mod->state_.GetOption(FLAGS_ACHIEVEMENT, i) &&
            !GetSWF(GSWF_AchUnlockQueue + i);
        if (completed) ++menu->achievement_log_completed_count;
        if (i <= AchievementId::META_ALL_ACHIEVEMENTS || completed) {
            ++menu->achievement_log_total_count;
        }
    }
    menu->achievement_log_hammers = g_Mod->state_.GetOption(STAT_PERM_ACH_HAMMERS);
    menu->achievement_log_using_hammer = 0;
    menu->achievement_log_unlock_timer = 0;
    menu->achievement_log_cam_shake_offset = 0.0f;
}

void LogMenuInit2(ttyd::win_root::WinPauseMenu* menu) {
    menu->main_cursor_target_x = 
        -128.0f + 120.0f * (menu->log_submenu_cursor_idx < 3 ? -1 : 1);
    menu->main_cursor_target_y =
        8.0f + 45.0f * (1.5f - menu->log_submenu_cursor_idx % 3);

    menu->badge_log_win_offset_x = 320.0f;
    menu->badge_log_win_target_x = 320.0f;
    menu->badge_log_win_offset_y = -240.0f;
    menu->badge_log_win_target_y = -240.0f;
    menu->badge_log_scroll_y = menu->badge_log_page_num * 232;
    menu->badge_log_scroll_target_y = menu->badge_log_page_num * 232;
    menu->badge_log_showcased_x = -200.0f;
    menu->badge_log_showcased_target_x = -200.0f;
    menu->badge_log_showcased_y = -300.0f;
    menu->badge_log_showcased_target_y = -300.0f;

    menu->recipe_log_win_offset_x = 320.0f;
    menu->recipe_log_win_target_x = 320.0f;
    menu->recipe_log_win_offset_y = -240.0f;
    menu->recipe_log_win_target_y = -240.0f;
    menu->recipe_log_scroll_y = menu->recipe_log_page_num * 232;
    menu->recipe_log_scroll_target_y = menu->recipe_log_page_num * 232;
    menu->recipe_log_showcased_x = -200.0f;
    menu->recipe_log_showcased_target_x = -200.0f;
    menu->recipe_log_showcased_y = -300.0f;
    menu->recipe_log_showcased_target_y = -300.0f;

    ttyd::win_root::winMsgEntry(
        menu, 0, menu->log_submenu_info[menu->log_submenu_cursor_idx].help_msg, 0);
}

void LogMenuExit(ttyd::win_root::WinPauseMenu* menu) {
    // No cleanup needed, since Map / Crystal Stars menus don't exist.
    return;
}

int32_t LogMenuMain(ttyd::win_root::WinPauseMenu* menu) {
    switch (menu->log_menu_state) {
        case 0: {
            // Main menu.
            uint32_t buttons_pressed = menu->buttons_pressed;
            if (buttons_pressed & ButtonId::B) {
                ttyd::pmario_sound::psndSFXOn((char *)0x20013);
                return -1;
            } else if (buttons_pressed & ButtonId::START) {
                return -2;
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_UP) {
                menu->log_submenu_cursor_idx
                    += (menu->log_submenu_cursor_idx % 3 == 0 ? 2 : -1);
                ttyd::pmario_sound::psndSFXOn((char *)0x20005);
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_DOWN) {
                menu->log_submenu_cursor_idx
                    += (menu->log_submenu_cursor_idx % 3 == 2 ? -2 : 1);
                ttyd::pmario_sound::psndSFXOn((char *)0x20005);
            } else if ((menu->dirs_repeated & DirectionInputId::ANALOG_LEFT) ||
                (menu->dirs_repeated & DirectionInputId::ANALOG_RIGHT)) {
                menu->log_submenu_cursor_idx
                    += (menu->log_submenu_cursor_idx >= 3 ? -3 : 3);
                ttyd::pmario_sound::psndSFXOn((char *)0x20005);
            } else if (buttons_pressed & ButtonId::A) {
                for (int32_t i = 0; i < menu->log_submenu_count; ++i) {
                    menu->log_submenu_info[i].state = 30;
                }
                auto& submenu = menu->log_submenu_info[menu->log_submenu_cursor_idx];
                submenu.state = 10;
                menu->log_menu_state = submenu.id + 10;
                
                switch (submenu.id) {
                  case 2:
                      menu->badge_log_win_target_x = -130.0f;
                      menu->badge_log_win_target_y = 120.0f;
                      menu->badge_log_showcased_target_x = -200.0f;
                      menu->badge_log_showcased_target_y =  60.0f;
                      break;
                  case 3:
                      menu->recipe_log_win_target_x = -130.0f;
                      menu->recipe_log_win_target_y = 120.0f;
                      menu->recipe_log_showcased_target_x = -200.0f;
                      menu->recipe_log_showcased_target_y =  60.0f;
                      break;
                }
                ttyd::pmario_sound::psndSFXOn((char *)0x20012);
            }
            
            menu->main_cursor_target_x =
                -128.0f + 120.0f * (menu->log_submenu_cursor_idx < 3 ? -1 : 1);
            menu->main_cursor_target_y =
                 8.0f + 45.0f * (1.5f - menu->log_submenu_cursor_idx % 3);
            ttyd::win_root::winMsgEntry(
                menu, 0, menu->log_submenu_info[menu->log_submenu_cursor_idx].help_msg, 0);
            
            break;
        }
        case 12: {
            // Badges menu.
            int32_t total = menu->badge_log_total_count;
            int32_t last_row_start = total / 7 * 7;
            int32_t pages = (total + 28 - 1) / 28;
            
            if (menu->buttons_pressed & ButtonId::B) {
                ttyd::pmario_sound::psndSFXOn((char *)0x20013);
                for (int32_t i = 0; i < menu->log_submenu_count; ++i) {
                    menu->log_submenu_info[i].state = 40;
                }
                menu->log_submenu_info[menu->log_submenu_cursor_idx].state = 20;
                menu->log_menu_state = 0;
                menu->badge_log_win_target_x = 330.0f;
                menu->badge_log_win_target_y = -250.0f;
                menu->badge_log_showcased_target_x = -200.0f;
                menu->badge_log_showcased_target_y = -300.0f;
            } else if (menu->buttons_repeated & ButtonId::L) {
                if (--menu->badge_log_page_num < 0)
                    menu->badge_log_page_num = 0;
                menu->badge_log_cursor_idx = menu->badge_log_page_num * 28;
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if (menu->buttons_repeated & ButtonId::R) {
                if (++menu->badge_log_page_num >= pages)
                    menu->badge_log_page_num = pages - 1;
                menu->badge_log_cursor_idx = menu->badge_log_page_num * 28;
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_RIGHT) {
                if (menu->badge_log_cursor_idx >= last_row_start) {
                    if (menu->badge_log_cursor_idx + 1 == total) {
                        menu->badge_log_cursor_idx = last_row_start;
                    } else {
                        ++menu->badge_log_cursor_idx;
                    }
                } else {
                    if (++menu->badge_log_cursor_idx % 7 == 0)
                        menu->badge_log_cursor_idx -= 7;
                }
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_LEFT) {
                if (menu->badge_log_cursor_idx >= last_row_start) {
                    if (menu->badge_log_cursor_idx == last_row_start) {
                        menu->badge_log_cursor_idx = total - 1;
                    } else {
                        --menu->badge_log_cursor_idx;
                    }
                } else {
                    if (menu->badge_log_cursor_idx % 7 == 0) {
                        menu->badge_log_cursor_idx += 6;
                    } else {
                        --menu->badge_log_cursor_idx;
                    }
                }
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_UP) {
                if (menu->badge_log_cursor_idx >= 7)
                    menu->badge_log_cursor_idx -= 7;
                menu->badge_log_page_num = menu->badge_log_cursor_idx / 28;
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_DOWN) {
                if (menu->badge_log_cursor_idx + 7 >= total) {
                    menu->badge_log_cursor_idx = total - 1;
                } else {
                    menu->badge_log_cursor_idx += 7;
                }
                menu->badge_log_page_num = menu->badge_log_cursor_idx / 28;
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if (menu->buttons_pressed & ButtonId::Z) {
                ttyd::win_root::winSortEntry(-310.0f, 150.0f, menu, 4);
                menu->parent_menu_state = menu->log_menu_state;
                ttyd::pmario_sound::psndSFXOn((char *)0x20012);
                menu->log_menu_state = 1000;
            } else if (menu->buttons_pressed & ButtonId::START) {
                return -2;
            }

            // Set cursor position based on selected badge.
            menu->main_cursor_target_x = -140.0f + (menu->badge_log_cursor_idx % 7) * 56;
            menu->main_cursor_target_y = 90.0f - ((menu->badge_log_cursor_idx % 28) / 7) * 56;
      
            if (!g_Mod->state_.GetOption(
                    FLAGS_ITEM_ENCOUNTERED,
                    menu->badge_log_ids[menu->badge_log_cursor_idx])) {
                ttyd::win_root::winMsgEntry(
                    menu, -2, 
                    "msg_menu_stone_none_help", "msg_menu_stone_none_name");
            } else {
                ttyd::win_root::winMsgEntry(
                    menu,-2,
                    itemDataTable[menu->badge_log_ids[menu->badge_log_cursor_idx] + 0x80].menu_description,
                    itemDataTable[menu->badge_log_ids[menu->badge_log_cursor_idx] + 0x80].name);
            }
            
            break;
        }
        case 13: {
            // Recipes menu.
            int32_t total = menu->recipe_log_total_count;
            int32_t last_row_start = total / 7 * 7;
            int32_t pages = (total + 28 - 1) / 28;
            
            if (menu->buttons_pressed & ButtonId::B) {
                ttyd::pmario_sound::psndSFXOn((char *)0x20013);
                for (int32_t i = 0; i < menu->log_submenu_count; ++i) {
                    menu->log_submenu_info[i].state = 40;
                }
                menu->log_submenu_info[menu->log_submenu_cursor_idx].state = 20;
                menu->log_menu_state = 0;
                menu->recipe_log_win_target_x = 330.0f;
                menu->recipe_log_win_target_y = -250.0f;
                menu->recipe_log_showcased_target_x = -200.0f;
                menu->recipe_log_showcased_target_y = -300.0f;
            } else if (menu->buttons_repeated & ButtonId::L) {
                if (--menu->recipe_log_page_num < 0)
                    menu->recipe_log_page_num = 0;
                menu->recipe_log_cursor_idx = menu->recipe_log_page_num * 28;
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if (menu->buttons_repeated & ButtonId::R) {
                if (++menu->recipe_log_page_num >= pages)
                    menu->recipe_log_page_num = pages - 1;
                menu->recipe_log_cursor_idx = menu->recipe_log_page_num * 28;
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_RIGHT) {
                if (menu->recipe_log_cursor_idx >= last_row_start) {
                    if (menu->recipe_log_cursor_idx + 1 == total) {
                        menu->recipe_log_cursor_idx = last_row_start;
                    } else {
                        ++menu->recipe_log_cursor_idx;
                    }
                } else {
                    if (++menu->recipe_log_cursor_idx % 7 == 0)
                        menu->recipe_log_cursor_idx -= 7;
                }
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_LEFT) {
                if (menu->recipe_log_cursor_idx >= last_row_start) {
                    if (menu->recipe_log_cursor_idx == last_row_start) {
                        menu->recipe_log_cursor_idx = total - 1;
                    } else {
                        --menu->recipe_log_cursor_idx;
                    }
                } else {
                    if (menu->recipe_log_cursor_idx % 7 == 0) {
                        menu->recipe_log_cursor_idx += 6;
                    } else {
                        --menu->recipe_log_cursor_idx;
                    }
                }
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_UP) {
                if (menu->recipe_log_cursor_idx >= 7)
                    menu->recipe_log_cursor_idx -= 7;
                menu->recipe_log_page_num = menu->recipe_log_cursor_idx / 28;
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_DOWN) {
                if (menu->recipe_log_cursor_idx + 7 >= total) {
                    menu->recipe_log_cursor_idx = total - 1;
                } else {
                    menu->recipe_log_cursor_idx += 7;
                }
                menu->recipe_log_page_num = menu->recipe_log_cursor_idx / 28;
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if (menu->buttons_pressed & ButtonId::Z) {
                ttyd::win_root::winSortEntry(-310.0f, 150.0f, menu, 5);
                menu->parent_menu_state = menu->log_menu_state;
                ttyd::pmario_sound::psndSFXOn((char *)0x20012);
                menu->log_menu_state = 1000;
            } else if (menu->buttons_pressed & ButtonId::START) {
                return -2;
            }
                         
            // Set cursor position based on selected recipe.
            menu->main_cursor_target_x = -140.0f + (menu->recipe_log_cursor_idx % 7) * 56;
            menu->main_cursor_target_y = 90.0f - ((menu->recipe_log_cursor_idx % 28) / 7) * 56;
      
            if (!g_Mod->state_.GetOption(
                    FLAGS_ITEM_ENCOUNTERED,
                    menu->recipe_log_ids[menu->recipe_log_cursor_idx])) {
                ttyd::win_root::winMsgEntry(
                    menu, -2, 
                    "msg_menu_stone_none_help", "msg_menu_stone_none_name");
            } else {
                ttyd::win_root::winMsgEntry(
                    menu,-2,
                    itemDataTable[menu->recipe_log_ids[menu->recipe_log_cursor_idx] + 0x80].menu_description,
                    itemDataTable[menu->recipe_log_ids[menu->recipe_log_cursor_idx] + 0x80].name);
            }
            
            break;
        }
        case 14: {
            // Tattle log.
            
            int32_t total = menu->tattle_log_total_count;
            int32_t pages = (total + 16 - 1) / 16;
            
            if (menu->buttons_pressed & ButtonId::B) {
                ttyd::pmario_sound::psndSFXOn((char *)0x20013);
                for (int32_t i = 0; i < menu->log_submenu_count; ++i) {
                    menu->log_submenu_info[i].state = 40;
                }
                menu->log_submenu_info[menu->log_submenu_cursor_idx].state = 20;
                menu->log_menu_state = 0;
            } else if (menu->buttons_repeated & ButtonId::L) {
                if (--menu->tattle_log_page_num < 0)
                    menu->tattle_log_page_num = 0;
                menu->tattle_log_cursor_idx = menu->tattle_log_page_num * 16;
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if (menu->buttons_repeated & ButtonId::R) {
                if (++menu->tattle_log_page_num >= pages)
                    menu->tattle_log_page_num = pages - 1;
                menu->tattle_log_cursor_idx = menu->tattle_log_page_num * 16;
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_RIGHT) {
                // Changing behavior because I never liked it in the OG, lol.
                // Left/right scrolls by -/+8, up/down by -/+1, and all
                // directions will turn pages when appropriate.
                if (menu->tattle_log_cursor_idx + 8 >= total) {
                    menu->tattle_log_cursor_idx = total - 1;
                } else {
                    menu->tattle_log_cursor_idx += 8;
                }
                menu->tattle_log_page_num = menu->tattle_log_cursor_idx / 16;
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_LEFT) {
                if (menu->tattle_log_cursor_idx >= 8)
                    menu->tattle_log_cursor_idx -= 8;
                menu->tattle_log_page_num = menu->tattle_log_cursor_idx / 16;
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_UP) {
                if (--menu->tattle_log_cursor_idx < 0)
                    menu->tattle_log_cursor_idx = 0;
                menu->tattle_log_page_num = menu->tattle_log_cursor_idx / 16;
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_DOWN) {
                if (++menu->tattle_log_cursor_idx >= total)
                    menu->tattle_log_cursor_idx = total - 1;
                menu->tattle_log_page_num = menu->tattle_log_cursor_idx / 16;
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if (menu->buttons_pressed & ButtonId::A) {
                if (ttyd::swdrv::swGet(menu->tattle_logs[menu->tattle_log_cursor_idx].id + 0x117a)) {
                    ttyd::pmario_sound::psndSFXOn((char *)0x20012);
                    menu->log_menu_state = 21;                    
                    auto* work = (ttyd::win_root::WinLogTattleMenuWork*)
                        ttyd::memory::_mapAlloc(
                            ttyd::memory::mapalloc_base_ptr,
                            sizeof(ttyd::win_root::WinLogTattleMenuWork));
                    memset(work, 0, sizeof(ttyd::win_root::WinLogTattleMenuWork));
                    work->state = 0;
                    work->enemy_id = menu->tattle_logs[menu->tattle_log_cursor_idx].id;
                    work->x = 500.0f;
                    work->y = -300.0f;
                    work->target_x = -60.0f;
                    work->target_y = 150.0f;
                    work->anim_pose = -1;
                    work->anim_pose_2 = -1;
                    work->texture_obj = nullptr;
                    work->alpha = 0;
                    menu->tattle_log_menu_work = work;
                }
            } else if (menu->buttons_pressed & ButtonId::Z) {
                ttyd::win_root::winSortEntry(-310.0f, 150.0f, menu, 6);
                menu->parent_menu_state = menu->log_menu_state;
                ttyd::pmario_sound::psndSFXOn((char *)0x20012);
                menu->log_menu_state = 1001;
            } else if (menu->buttons_pressed & ButtonId::START) {
                return -2;
            }

            // Set cursor position based on selected enemy.
            menu->main_cursor_target_x = (menu->tattle_log_cursor_idx % 16) < 8 ? -180 : 36;
            menu->main_cursor_target_y = 106 - (menu->tattle_log_cursor_idx % 8) * 24;
      
            if (!ttyd::swdrv::swGet(menu->tattle_logs[menu->tattle_log_cursor_idx].id + 0x117a)) {
                ttyd::win_root::winMsgEntry(menu, 0, "msg_menu_monosiri_enemy", 0);
            }
            else {
                auto* monosiri = ttyd::battle_monosiri::battleGetUnitMonosiriPtr(
                    menu->tattle_logs[menu->tattle_log_cursor_idx].id);
                ttyd::win_root::winMsgEntry(menu, 0, monosiri->menu_tattle, 0);
            }
            
            break;
        }
        case 15: {
            // Move log.

            const int32_t num_moves = MoveType::MOVE_TYPE_MAX;
            int32_t page_number = menu->move_log_cursor_idx < 24
                ? menu->move_log_cursor_idx / 8
                : (menu->move_log_cursor_idx) / 6 - 1;
            int32_t page_first_entry =
                page_number < 3 ? page_number * 8 : (page_number + 1) * 6;
            int32_t page_cur_entry =
                menu->move_log_cursor_idx - page_first_entry;
            int32_t last_pos = menu->move_log_cursor_idx;

            if (menu->buttons_pressed & ButtonId::START) {
                return -2;
            } else if (menu->buttons_pressed & ButtonId::B) {
                ttyd::pmario_sound::psndSFXOn((char *)0x20013);
                for (int32_t i = 0; i < menu->log_submenu_count; ++i) {
                    menu->log_submenu_info[i].state = 40;
                }
                menu->log_submenu_info[menu->log_submenu_cursor_idx].state = 20;
                menu->log_menu_state = 0;
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_UP) {
                if (--menu->move_log_cursor_idx < 0)
                    menu->move_log_cursor_idx = 0;
                if (last_pos != menu->move_log_cursor_idx) {
                    // Force an update.
                    ttyd::win_root::winMsgEntry(menu, 0, "", 0);
                }
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_DOWN) {
                if (++menu->move_log_cursor_idx >= num_moves)
                    menu->move_log_cursor_idx = num_moves - 1;
                if (last_pos != menu->move_log_cursor_idx) {
                    // Force an update.
                    ttyd::win_root::winMsgEntry(menu, 0, "", 0);
                }
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if ((menu->dirs_repeated & DirectionInputId::ANALOG_LEFT) ||
                       (menu->buttons_repeated & ButtonId::L)) {
                if (--page_number < 0) page_number = 0;
                page_first_entry = 
                    page_number < 3 ? page_number * 8 : (page_number + 1) * 6;
                menu->move_log_cursor_idx = page_first_entry;
                page_cur_entry = 0;
                if (last_pos != menu->move_log_cursor_idx) {
                    // Force an update.
                    ttyd::win_root::winMsgEntry(menu, 0, "", 0);
                }
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if ((menu->dirs_repeated & DirectionInputId::ANALOG_RIGHT) ||
                       (menu->buttons_repeated & ButtonId::R)) {
                if (++page_number > 9) page_number = 9;
                page_first_entry = 
                    page_number < 3 ? page_number * 8 : (page_number + 1) * 6;
                menu->move_log_cursor_idx = page_first_entry;
                page_cur_entry = 0;
                if (last_pos != menu->move_log_cursor_idx) {
                    // Force an update.
                    ttyd::win_root::winMsgEntry(menu, 0, "", 0);
                }
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            }

            // Set window description based on hovered move.
            uint32_t partner_flags =
                g_Mod->state_.GetOption(STAT_PERM_PARTNERS_OBTAINED);
            uint32_t move_flags =
                g_Mod->state_.GetOption(STAT_PERM_MOVE_LOG, menu->move_log_cursor_idx);
            if ((move_flags & MoveLogFlags::UNLOCKED_LV_1) &&
                (page_number < 3 || (partner_flags & (1 << (page_number - 2))))) {
                // Create dynamic lookup key, so move description can
                // change to include move level information as levels unlock.
                ttyd::win_root::winMsgEntry(menu, 0, "tot_movelog_desc_dyn", 0);
            } else {
                // Skip non-unlocked moves or moves for non-unlocked partners.
                ttyd::win_root::winMsgEntry(menu, 0, "msg_menu_stone_none_help", 0);
            }

            // Set cursor position based on selected entry.
            menu->main_cursor_target_x = -180.0f;
            menu->main_cursor_target_y = 82.0f - page_cur_entry * 24;

            break;
        }
        case 16: {
            // Achievement log.

            int8_t const* grid;
            int8_t const* states;
            AchievementsManager::GetAchievementGrid(&grid);
            int32_t unlock_cell = AchievementsManager::GetAchievementStates(&states);
            if (unlock_cell != -1) {
                menu->log_menu_state = 60;
                menu->achievement_log_unlock_timer = -60;
                break;
            }

            int32_t cursor = menu->achievement_log_cursor_idx;

            if (menu->buttons_pressed & ButtonId::START) {
                return -2;
            } else if (menu->buttons_pressed & ButtonId::B) {
                ttyd::pmario_sound::psndSFXOn((char *)0x20013);
                if (menu->achievement_log_using_hammer) {
                    // Exit "using hammer" mode.
                    menu->achievement_log_using_hammer = 0;
                    menu->achievement_log_cursor_idx = -1;
                } else {
                    for (int32_t i = 0; i < menu->log_submenu_count; ++i) {
                        menu->log_submenu_info[i].state = 40;
                    }
                    menu->log_submenu_info[menu->log_submenu_cursor_idx].state = 20;
                    menu->log_menu_state = 0;
                }
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_UP) {
                if (cursor != -1 && cursor > 9 && 
                    !((cursor == 10 || cursor == 19) && !states[cursor - 10])) {
                    menu->achievement_log_cursor_idx -= 10;
                    cursor = menu->achievement_log_cursor_idx;
                }
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_DOWN) {
                if (cursor != -1 && cursor < 60 && 
                    !((cursor == 50 || cursor == 59) && !states[cursor + 10])) {
                    menu->achievement_log_cursor_idx += 10;
                    cursor = menu->achievement_log_cursor_idx;
                }
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_LEFT) {
                if (cursor % 10 == 0 && !menu->achievement_log_using_hammer &&
                    menu->achievement_log_hammers > 0) {
                    menu->achievement_log_cursor_idx = -1;
                    cursor = -1;
                } else if (cursor % 10 > 0 && 
                    !((cursor == 1 || cursor == 61) && !states[cursor - 1])) {
                    cursor = --menu->achievement_log_cursor_idx;
                }
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_RIGHT) {
                if (cursor == -1) {
                    menu->achievement_log_cursor_idx = 30;
                    cursor = 30;
                } else if (cursor % 10 < 9 && 
                    !((cursor == 8 || cursor == 68) && !states[cursor + 1])) {
                    cursor = ++menu->achievement_log_cursor_idx;
                }
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if (menu->buttons_pressed & ButtonId::A) {
                if (menu->achievement_log_using_hammer) {
                    if (IsUnbreakable(cursor) || states[cursor] == 2) {
                        ttyd::sound::SoundEfxPlayEx(0x266, 0, 0x64, 0x40);
                    } else {
                        int32_t ach = grid[cursor];
                        AchievementsManager::MarkCompleted(ach);
                        g_Mod->state_.ChangeOption(STAT_PERM_ACH_HAMMERS, -1);
                        menu->log_menu_state = 60;
                        menu->achievement_log_unlock_timer = -1;
                        break;
                    }
                } else if (cursor == -1 &&
                    menu->achievement_log_completed_count < 
                    menu->achievement_log_total_count) {
                    // Go into "hammer use" mode, only if there are
                    // achievements left to unlock.
                    menu->achievement_log_using_hammer = 1;
                    menu->achievement_log_cursor_idx = 34;
                    cursor = 34;
                    ttyd::pmario_sound::psndSFXOn((char *)0x20012);
                }
            }

            if (cursor == -1) {
                // Instructions on using the hammer.
                ttyd::win_root::winMsgEntry(menu, 0, "tot_ach_usehammer", 0);
                // Set cursor position based on selected hammer.
                int32_t hammer_index = menu->achievement_log_hammers - 1;
                menu->main_cursor_target_x = -286.0f + 30.0f * hammer_index;
                menu->main_cursor_target_y = -25.0f - 10.0f * (hammer_index & 1);
            } else if (
                menu->achievement_log_using_hammer && IsUnbreakable(cursor) &&
                states[cursor] < 2) {
                // Let the player know they cannot use the hammer.
                ttyd::win_root::winMsgEntry(menu, 0, "tot_ach_unbreakable", 0);
            } else if (states[cursor] > 0) {
                int32_t ach = grid[cursor];
                ttyd::win_root::winMsgEntry(
                    menu, 0, AchievementsManager::GetData(ach)->help_msg, 0);
            } else {
                ttyd::win_root::winMsgEntry(
                    menu, 0, "msg_menu_stone_none_help", 0);
            }

            if (cursor != -1) {
                // Set cursor position based on selected entry.
                menu->main_cursor_target_x = -105.0f + 36.0f * (cursor % 10);
                menu->main_cursor_target_y = 120.0f - 36.0f * (cursor / 10);
            }

            break;
        }
        case 17: {
            // Records log.
            
            // Records log has current run stats first, if currently in a run.
            const int32_t min_entry =
                g_Mod->state_.GetOption(OPT_RUN_STARTED) ? 0 : 18;

            if (menu->buttons_pressed & ButtonId::START) {
                return -2;
            } else if (menu->buttons_pressed & ButtonId::B) {
                ttyd::pmario_sound::psndSFXOn((char *)0x20013);
                for (int32_t i = 0; i < menu->log_submenu_count; ++i) {
                    menu->log_submenu_info[i].state = 40;
                }
                menu->log_submenu_info[menu->log_submenu_cursor_idx].state = 20;
                menu->log_menu_state = 0;
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_UP) {
                while (true) {
                    if (--menu->records_log_cursor_idx < min_entry) {
                        menu->records_log_cursor_idx = min_entry + 1;
                        break;
                    }
                    if (kRecordLogEntries[menu->records_log_cursor_idx].option 
                        != REC_EMPTY)
                        break;
                }
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_DOWN) {
                while (true) {
                    if (++menu->records_log_cursor_idx >= kNumRecordLogEntries) {
                        menu->records_log_cursor_idx = kNumRecordLogEntries - 1;
                        break;
                    }
                    if (kRecordLogEntries[menu->records_log_cursor_idx].option 
                        != REC_EMPTY)
                        break;
                }
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if ((menu->dirs_repeated & DirectionInputId::ANALOG_LEFT) ||
                       (menu->buttons_repeated & ButtonId::L)) {
                menu->records_log_cursor_idx = 
                    menu->records_log_cursor_idx / 9 * 9 - 9;
                if (menu->records_log_cursor_idx < min_entry) {
                    menu->records_log_cursor_idx += 9;
                }
                ++menu->records_log_cursor_idx;
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if ((menu->dirs_repeated & DirectionInputId::ANALOG_RIGHT) ||
                       (menu->buttons_repeated & ButtonId::R)) {
                menu->records_log_cursor_idx = 
                    menu->records_log_cursor_idx / 9 * 9 + 9;
                if (menu->records_log_cursor_idx >= kNumRecordLogEntries) {
                    menu->records_log_cursor_idx -= 9;
                }
                ++menu->records_log_cursor_idx;
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            }

            // Set window description based on hovered option.
            ttyd::win_root::winMsgEntry(
                menu, 0, kRecordLogEntries[menu->records_log_cursor_idx].desc_msg, 0);

            // Set cursor position based on selected entry.
            menu->main_cursor_target_x = -180.0f;
            menu->main_cursor_target_y = 106.0f - (menu->records_log_cursor_idx % 9) * 24;

            break;
        }
        case 21: {
            // Loading Tattle scene.
            if (menu->buttons_pressed & ButtonId::B) {
                ttyd::pmario_sound::psndSFXOn((char *)0x20013);
                menu->log_menu_state = 22;
                if (menu->tattle_log_menu_work) {
                    menu->tattle_log_menu_work->state = 2;
                }
            } else {
                if (menu->buttons_pressed & ButtonId::START) {
                    if (menu->tattle_log_menu_work != nullptr) {
                        if (menu->tattle_log_menu_work->win_tpl) {
                            ttyd::filemgr::fileFree(menu->tattle_log_menu_work->win_tpl);
                            menu->tattle_log_menu_work->win_tpl = nullptr;
                        }
                        if (menu->tattle_log_menu_work->mono_dat) {
                            ttyd::filemgr::fileFree(menu->tattle_log_menu_work->mono_dat);
                            menu->tattle_log_menu_work->mono_dat = nullptr;
                        }
                        if (menu->tattle_log_menu_work->anim_pose != -1) {
                            ttyd::animdrv::animPoseRelease(menu->tattle_log_menu_work->anim_pose);
                            menu->tattle_log_menu_work->anim_pose = -1;
                        }
                        if (menu->tattle_log_menu_work->anim_pose_2 != -1) {
                            ttyd::animdrv::animPoseRelease(menu->tattle_log_menu_work->anim_pose_2);
                            menu->tattle_log_menu_work->anim_pose_2 = -1;
                        }
                        if (menu->tattle_log_menu_work->texture_obj) {
                            ttyd::memory::smartFree(menu->tattle_log_menu_work->texture_obj);
                            menu->tattle_log_menu_work->texture_obj = nullptr;
                        }
                        ttyd::memory::_mapFree(
                            ttyd::memory::mapalloc_base_ptr, 
                            menu->tattle_log_menu_work);
                    }
                    return -2;
                }
                ttyd::win_log::monosiriMain(menu->tattle_log_menu_work);
            }
            break;      
        }
        case 22: {
            // Check for unloading Tattle scene.
            bool unload;
            if (menu->tattle_log_menu_work == nullptr) {
                unload = true;
            } else if (
                menu->tattle_log_menu_work->x <= 500.0f ||
                menu->tattle_log_menu_work->y >= -300.0f) {
                unload = false;
            } else {
                unload = true;
            }
            
            if (unload) {
                menu->log_menu_state = 14;
                
                if (menu->tattle_log_menu_work != nullptr) {
                    if (menu->tattle_log_menu_work->win_tpl) {
                        ttyd::filemgr::fileFree(menu->tattle_log_menu_work->win_tpl);
                        menu->tattle_log_menu_work->win_tpl = nullptr;
                    }
                    if (menu->tattle_log_menu_work->mono_dat) {
                        ttyd::filemgr::fileFree(menu->tattle_log_menu_work->mono_dat);
                        menu->tattle_log_menu_work->mono_dat = nullptr;
                    }
                    if (menu->tattle_log_menu_work->anim_pose != -1) {
                        ttyd::animdrv::animPoseRelease(menu->tattle_log_menu_work->anim_pose);
                        menu->tattle_log_menu_work->anim_pose = -1;
                    }
                    if (menu->tattle_log_menu_work->anim_pose_2 != -1) {
                        ttyd::animdrv::animPoseRelease(menu->tattle_log_menu_work->anim_pose_2);
                        menu->tattle_log_menu_work->anim_pose_2 = -1;
                    }
                    if (menu->tattle_log_menu_work->texture_obj) {
                        ttyd::memory::smartFree(menu->tattle_log_menu_work->texture_obj);
                        menu->tattle_log_menu_work->texture_obj = nullptr;
                    }
                    ttyd::memory::_mapFree(
                        ttyd::memory::mapalloc_base_ptr, 
                        menu->tattle_log_menu_work);
                }
            } else {
                ttyd::win_log::monosiriMain(menu->tattle_log_menu_work);
            }
            break;
        }
        case 60: {
            // Unlocking animation for achievements menu.

            int8_t const* grid;
            int8_t const* states;
            AchievementsManager::GetAchievementGrid(&grid);
            int32_t unlock_cell = AchievementsManager::GetAchievementStates(&states);

            int32_t cursor = menu->achievement_log_cursor_idx;

            if (menu->achievement_log_unlock_timer < 0) {
                ++menu->achievement_log_unlock_timer;
            } else {
                --menu->achievement_log_unlock_timer;

                // Calculate camera shake offset (cosine w/quadratic damping).
                float shake_time =
                    2.0f - menu->achievement_log_unlock_timer / 30.0f;
                if (shake_time > 1.0f) shake_time = 1.0f;
                float shake_offset = 3.0f;
                shake_offset *= ttyd::_core_cos(shake_time * 8 * 3.14159265f);
                shake_offset *= (1.0f - shake_time) * (1.0f - shake_time);
                menu->achievement_log_cam_shake_offset = shake_offset;

                // Allow speeding up animation if holding down the A or B button.
                if ((ttyd::system::keyGetButton(0) & (ButtonId::A | ButtonId::B)) &&
                    menu->achievement_log_unlock_timer < 30) {
                    menu->achievement_log_unlock_timer = 0;
                }
            }

            if (menu->achievement_log_unlock_timer == 60 - 7 &&
                menu->achievement_log_using_hammer) {
                // Play second half of hammer swinging sound.
                ttyd::sound::SoundEfxPlayEx(0x37f, 0, 0x64, 0x40);
                // Leave hammer-use mode.
                menu->achievement_log_using_hammer = 0;
            }

            if (menu->achievement_log_unlock_timer == 0) {
                if (unlock_cell == -1) {
                    menu->log_menu_state = 16;
                    break;
                }

                int32_t ach = grid[unlock_cell];
                SetSWF(GSWF_AchUnlockQueue + ach, 0);
                menu->achievement_log_cursor_idx = unlock_cell;
                cursor = menu->achievement_log_cursor_idx;

                // Update number of achievements unlocked / in total.
                menu->achievement_log_completed_count = 0;
                menu->achievement_log_total_count = 0;
                for (int32_t i = 0; i < AchievementId::MAX_ACHIEVEMENT; ++i) {
                    bool completed =
                        g_Mod->state_.GetOption(FLAGS_ACHIEVEMENT, i) &&
                        !GetSWF(GSWF_AchUnlockQueue + i);
                    if (completed) ++menu->achievement_log_completed_count;
                    if (i <= AchievementId::META_ALL_ACHIEVEMENTS || completed) {
                        ++menu->achievement_log_total_count;
                    }
                }

                // Update number of hammers if necessary.
                if (AchievementsManager::GetData(ach)->reward_type ==
                    AchievementRewardType::HAMMER) {
                    g_Mod->state_.ChangeOption(STAT_PERM_ACH_HAMMERS, 1);
                }
                menu->achievement_log_hammers =
                    g_Mod->state_.GetOption(STAT_PERM_ACH_HAMMERS);
                
                // Play explosion / hammer swinging sound.
                if (menu->achievement_log_using_hammer) {
                    ttyd::sound::SoundEfxPlayEx(0x384, 0, 0x64, 0x40);
                } else {
                    ttyd::sound::SoundEfxPlayEx(0x3f7, 0, 0x64, 0x40);
                }

                menu->achievement_log_unlock_timer = 60;
            }

            if (states[cursor] > 0) {
                int32_t ach = grid[cursor];
                ttyd::win_root::winMsgEntry(
                    menu, 0, AchievementsManager::GetData(ach)->help_msg, 0);
            } else {
                ttyd::win_root::winMsgEntry(
                    menu, 0, "msg_menu_stone_none_help", 0);
            }

            // Lock cursor position based on selected entry.
            menu->main_cursor_target_x = -105.0f + 36.0f * (cursor % 10);
            menu->main_cursor_target_y = 120.0f - 36.0f * (cursor / 10);
            menu->main_cursor_x = menu->main_cursor_target_x;
            menu->main_cursor_y = menu->main_cursor_target_y;

            break;
        }
        case 1000:
        case 1001: {
            if (ttyd::win_root::winSortWait(menu) == 0) {
                menu->log_menu_state = menu->parent_menu_state;
            }
            if (menu->buttons_pressed & ButtonId::START) {
                return -2;
            }
            break;
        }
    }
    
    return 0;
}

void LogMenuMain2(ttyd::win_root::WinPauseMenu* menu) {
    
    for (int32_t i = 0; i < menu->log_submenu_count; ++i) {
        auto& submenu = menu->log_submenu_info[i];
        // Handle animating submenu banner.
        switch(submenu.state) {
            case 10:
                // Opening submenu, vertical slide.
                submenu.target_x = 120.0f * (i < 3 ? -1 : 1);
                submenu.target_y = 140.0f;
                submenu.timer = 0;
                ++submenu.state;
                break;
            case 11:
                // Opening submenu, horizontal slide.
                if (8 < ++submenu.timer) {
                    submenu.target_x = -160.0f;
                    submenu.target_y = 140.0f;
                    submenu.timer = 0;
                    ++submenu.state;
                }
                break;
            case 20:
                // Closing submenu, horizontal slide.
                submenu.target_x = 120.0f * (i < 3 ? -1 : 1);
                submenu.target_y = 140.0f;
                submenu.timer = 0;
                ++submenu.state;
                break;
            case 21:
                // Closing submenu, vertical slide.
                if (8 < ++submenu.timer) {
                    submenu.target_x = 120.0f * (i < 3 ? -1 : 1);
                    submenu.target_y = 45.0f * (1.5f - i % 3);
                    submenu.timer = 0;
                    ++submenu.state;
                }
                break;
            case 30:
                // Opened a different submenu; slide out.
                submenu.target_x = 120.0f * (i < 3 ? -1 : 1);
                submenu.target_y = -160.0f;
                submenu.timer = 0;
                ++submenu.state;
                break;
            case 40:
                // Closed a different submenu; slide in.
                submenu.target_x = 120.0f * (i < 3 ? -1 : 1);
                submenu.target_y = 45.0f * (1.5f - i % 3);
                submenu.timer = 0;
                ++submenu.state;
                break;
        }
        submenu.x += (submenu.target_x - submenu.x) / 8.0f;
        submenu.y += (submenu.target_y - submenu.y) / 8.0f;
    }

    menu->badge_log_scroll_target_y = menu->badge_log_page_num * 232;
    menu->badge_log_scroll_y +=
        (menu->badge_log_scroll_target_y - menu->badge_log_scroll_y) / 6.0f;
    menu->badge_log_win_offset_x +=
        (menu->badge_log_win_target_x - menu->badge_log_win_offset_x) / 6.0f;
    menu->badge_log_win_offset_y +=
        (menu->badge_log_win_target_y - menu->badge_log_win_offset_y) / 6.0f;
    menu->badge_log_showcased_x +=
        (menu->badge_log_showcased_target_x - menu->badge_log_showcased_x) / 6.0f;
    menu->badge_log_showcased_y +=
        (menu->badge_log_showcased_target_y - menu->badge_log_showcased_y) / 6.0f;

    menu->recipe_log_scroll_target_y = menu->recipe_log_page_num * 232;
    menu->recipe_log_scroll_y +=
        (menu->recipe_log_scroll_target_y - menu->recipe_log_scroll_y) / 6.0f;
    menu->recipe_log_win_offset_x +=
        (menu->recipe_log_win_target_x - menu->recipe_log_win_offset_x) / 6.0f;
    menu->recipe_log_win_offset_y +=
        (menu->recipe_log_win_target_y - menu->recipe_log_win_offset_y) / 6.0f;
    menu->recipe_log_showcased_x +=
        (menu->recipe_log_showcased_target_x - menu->recipe_log_showcased_x) / 6.0f;
    menu->recipe_log_showcased_y +=
        (menu->recipe_log_showcased_target_y - menu->recipe_log_showcased_y) / 6.0f;
}

void LogMenuDisp(CameraId camera_id, WinPauseMenu* menu, int32_t tab_number) {
    float win_x = menu->tab_body_info[tab_number].x;
    float win_y = menu->tab_body_info[tab_number].y;
  
    // Draw basic BG.
    ttyd::win_root::winBgGX(win_x, win_y, menu, 4);
  
    // Badge log window.
    if (menu->badge_log_win_offset_x < 320.0f && -240.0f < menu->badge_log_win_offset_y) {
        ttyd::win_root::winHakoGX(
            win_x + menu->badge_log_win_offset_x,
            win_y + menu->badge_log_win_offset_y, menu, 0);
        ttyd::win_root::winKirinukiGX(
            win_x + menu->badge_log_showcased_x - 50.0f,
            win_y + menu->badge_log_showcased_y - 120.0f,
            100.0f, 30.0f, menu, 0);

        {
            ttyd::win_main::winFontInit();
            char buf[16];
            sprintf(buf, "%d/%d", menu->badge_log_obtained_count, menu->badge_log_total_count);
            int32_t length = ttyd::fontmgr::FontGetMessageWidth(buf);
            gc::vec3 position = {
                win_x + menu->badge_log_showcased_x - length * 0.375f,
                win_y + menu->badge_log_showcased_y - 128.0f,
                0.0f };
            gc::vec3 scale = { 0.75f, 0.75f, 0.75f };
            uint32_t color = 0x000000FFU;
            ttyd::win_main::winFontSet(&position, &scale, &color, buf);
        }
        {
            ttyd::win_main::winTexInit(*menu->win_tpl->mpFileData);
            gc::vec3 position = {
                win_x + menu->badge_log_showcased_x,
                win_y + menu->badge_log_showcased_y,
                0.0f };
            gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
            uint32_t color = 0xFFFFFFFFU;
            ttyd::win_main::winTexSet(0x95, &position, &scale, &color);
        }
        // Draw badge icon only if currently unlocked.
        if (g_Mod->state_.GetOption(
                FLAGS_ITEM_ENCOUNTERED, 
                menu->badge_log_ids[menu->badge_log_cursor_idx])) {
            ttyd::win_main::winIconInit();
            gc::vec3 position = {
                win_x + menu->badge_log_showcased_x,
                win_y + menu->badge_log_showcased_y - 10.0f,
                0.0f };
            gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
            uint32_t color = 0xFFFFFFFFU;
            ttyd::win_main::winIconSet(
                itemDataTable[menu->badge_log_ids[menu->badge_log_cursor_idx] + 0x80].icon_id,
                &position, &scale, &color);
        }
    }
  
    // Recipe log window.
    if (menu->recipe_log_win_offset_x < 320.0f && -240.0f < menu->recipe_log_win_offset_y) {
        ttyd::win_root::winHakoGX(
            win_x + menu->recipe_log_win_offset_x,
            win_y + menu->recipe_log_win_offset_y, menu, 1);
        ttyd::win_root::winKirinukiGX(
            win_x + menu->recipe_log_showcased_x - 50.0f,
            win_y + menu->recipe_log_showcased_y - 120.0f,
            100.0f, 30.0f, menu, 0);
                
        {
            ttyd::win_main::winFontInit();
            char buf[16];
            sprintf(buf, "%d/%d", menu->recipe_log_obtained_count, menu->recipe_log_total_count);
            int32_t length = ttyd::fontmgr::FontGetMessageWidth(buf);    
            gc::vec3 position = {
                win_x + menu->recipe_log_showcased_x - length * 0.375f,
                win_y + menu->recipe_log_showcased_y - 128.0f,
                0.0f };
            gc::vec3 scale = { 0.75f, 0.75f, 0.75f };
            uint32_t color = 0x000000FFU;
            ttyd::win_main::winFontSet(&position, &scale, &color, buf);
        }
        {
            ttyd::win_main::winTexInit(*menu->win_tpl->mpFileData);
            gc::vec3 position = {
                win_x + menu->recipe_log_showcased_x,
                win_y + menu->recipe_log_showcased_y,
                0.0f };
            gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
            uint32_t color = 0xFFFFFFFFU;
            // Change background to the pedestal one from the badge menu.
            ttyd::win_main::winTexSet(0x95, &position, &scale, &color);
        }
        // Draw recipe icon only if currently unlocked.
        if (g_Mod->state_.GetOption(
                FLAGS_ITEM_ENCOUNTERED,
                menu->recipe_log_ids[menu->recipe_log_cursor_idx])) {
            ttyd::win_main::winIconInit();
            gc::vec3 position = {
                win_x + menu->recipe_log_showcased_x,
                win_y + menu->recipe_log_showcased_y - 10.0f,
                0.0f };
            gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
            uint32_t color = 0xFFFFFFFFU;
            ttyd::win_main::winIconSet(
                itemDataTable[menu->recipe_log_ids[menu->recipe_log_cursor_idx] + 0x80].icon_id,
                &position, &scale, &color);
        }
    }
  
    // Other menus.
    switch (menu->log_menu_state) {
        case 14:
        case 20:
        case 21:
        case 22:
        case 1001:
             ttyd::win_log::monoshiriGX(win_x - 170.0f, win_y + 130.0f, menu);
             break;
        case 15:
            DrawMoveLog(menu, win_x - 170.0f, win_y + 130.0f);
            break;
        case 16:
        case 60:
            DrawAchievementLog(
                menu,
                win_x - 170.0f + menu->achievement_log_cam_shake_offset,
                win_y + 130.0f - menu->achievement_log_cam_shake_offset);
            break;
        case 17:
            DrawRecordsLog(menu, win_x - 170.0f, win_y + 130.0f);
            break;
    }
  
    // Draw banners for submenus.
    for (int32_t i = 0; i < menu->log_submenu_count; ++i) {
        const auto& submenu = menu->log_submenu_info[i];
        int32_t banner_id;
        switch (submenu.id) {
            case 2:
                banner_id = 0x21;   // Badges
                break;
            case 3:
                banner_id = 0xc2;   // Items
                break;
            case 4:
                banner_id = 0x23;   // Tattle Log
                break;
            case 5:
                banner_id = 0xc4;   // Moves
                break;
            case 6:
                banner_id = 0xc1;   // Achievements
                break;
            case 7:
                banner_id = 0xc3;   // Records
                break;
            default:
                // Should not be reached.
                banner_id = 0x20;
                break;
        }
        if (i == menu->log_submenu_cursor_idx) {
            {    
                ttyd::win_main::winTexInit(*menu->win_tpl->mpFileData);
                gc::vec3 position = { win_x + submenu.x, win_y + submenu.y, 0.0f };
                gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
                uint32_t color = 0x00000080U;
                ttyd::win_main::winTexSet(0xb5, &position, &scale, &color);
            }
            {
                ttyd::win_main::winTexInit_x2(*menu->win_tpl->mpFileData);
                gc::vec3 position = { win_x + submenu.x - 8.0f, win_y + submenu.y + 8.0f, 0.0f };
                gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
                uint32_t color = 0xFFFFFFFFU;
                ttyd::win_main::winTexSet_x2(banner_id, 0xb5, &position, &scale, &color);
            }
        } else {
            ttyd::win_main::winTexInit_x2(*menu->win_tpl->mpFileData);
            gc::vec3 position = { win_x + submenu.x, win_y + submenu.y, 0.0f };
            gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
            uint32_t color = 0x808080FFU;
            ttyd::win_main::winTexSet_x2(banner_id, 0xb5, &position, &scale, &color);
        }

        // Draw "!" icon above achievements banner if there's a pending unlock.
        if (submenu.id == 6 && menu->log_menu_state == 0) {
            bool achievement_queued = false;
            for (int32_t i = 0; i < 70; ++i) {
                if (GetSWF(GSWF_AchUnlockQueue + i)) {
                    achievement_queued = true;
                    break;
                }
            }
            if (achievement_queued) {
                float offset = i == menu->log_submenu_cursor_idx ? 8.0f : 0.0f;
                ttyd::win_main::winIconInit();
                gc::vec3 position = {
                    win_x + submenu.x + 116.0f - offset,
                    win_y + submenu.y + 24.0f + offset,
                    0.0f
                };
                gc::vec3 scale = { 0.8f, 0.8f, 0.8f };
                uint32_t color = 0xFFFFFFFFU;
                ttyd::win_main::winIconSet(
                    IconType::STATUS_EXCLAMATION_BUTTON,
                    &position, &scale, &color);
            }
        }
    }
}

}  // namespace mod::tot::win