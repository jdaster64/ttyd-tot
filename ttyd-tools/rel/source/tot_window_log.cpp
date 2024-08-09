#include "tot_window_log.h"

#include "common_functions.h"
#include "common_types.h"
#include "mod.h"
#include "tot_generate_enemy.h"
#include "tot_state.h"

#include <gc/types.h>
#include <ttyd/gx/GXTev.h>
#include <ttyd/gx/GXTransform.h>
#include <ttyd/battle_monosiri.h>
#include <ttyd/animdrv.h>
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
    REC_TATTLE_PCT,
    REC_ACHIEVEMENT_PCT,
    REC_HUB_PROGRESS_PCT,
    REC_HUB_ITEMS,
    REC_HUB_BADGES,
    REC_HUB_KEY_ITEMS,
    REC_HUB_OPTIONS,
    REC_HUB_MARIO_SKINS,
    REC_HUB_YOSHI_SKINS,
    REC_HUB_ATTACK_FX,
};

struct RecordLogEntry {
    int32_t option;
    const char* name_msg;
    const char* desc_msg;
};
const RecordLogEntry kRecordLogEntries[] = {
    { REC_EMPTY, "tot_recn_overall", "tot_rech_progression" },
    { REC_PLAY_TIME, "tot_recn_playtime", "tot_rech_progression" },
    { REC_COMPLETION_PCT, "tot_recn_completion_pct", "tot_rech_progression" },
    { REC_ACHIEVEMENT_PCT, "tot_recn_achievement_pct", "tot_rech_progression" },
    { REC_ITEM_PCT, "tot_recn_item_pct", "tot_rech_progression" },
    { REC_BADGE_PCT, "tot_recn_badge_pct", "tot_rech_progression" },
    { REC_MOVE_PCT, "tot_recn_move_pct", "tot_rech_progression" },
    { REC_TATTLE_PCT, "tot_recn_tattle_pct", "tot_rech_progression" },
    { REC_HUB_PROGRESS_PCT, "tot_recn_hub_pct", "tot_rech_hub_pct" },
    { REC_EMPTY, "tot_recn_hub", "tot_rech_hub_pct" },
    { REC_HUB_ITEMS, "tot_recn_hub_items", "tot_rech_hub_pct" },
    { REC_HUB_BADGES, "tot_recn_hub_badges", "tot_rech_hub_pct" },
    { REC_HUB_KEY_ITEMS, "tot_recn_hub_keyitems", "tot_rech_hub_pct" },
    { REC_HUB_OPTIONS, "tot_recn_hub_options", "tot_rech_hub_pct" },
    { REC_HUB_MARIO_SKINS, "tot_recn_hub_marioskins", "tot_rech_hub_pct" },
    { REC_HUB_YOSHI_SKINS, "tot_recn_hub_yoshiskins", "tot_rech_hub_pct" },
    { REC_HUB_ATTACK_FX, "tot_recn_hub_attackfx", "tot_rech_hub_pct" },
    { REC_EMPTY, nullptr, "tot_rech_wins" },
    { REC_EMPTY, "tot_recn_runs", "tot_rech_wins" },
    { STAT_PERM_HALF_FINISHES, "tot_recn_half_wins", "tot_rech_wins" },
    { STAT_PERM_FULL_FINISHES, "tot_recn_full_wins", "tot_rech_wins" },
    { STAT_PERM_EX_FINISHES, "tot_recn_ex_wins", "tot_rech_wins" },
    { STAT_PERM_CONTINUES, "tot_recn_continues", "tot_rech_wins" },
    { REC_EMPTY, "tot_recn_times", "tot_rech_wins" },
    { STAT_PERM_HALF_BEST_TIME, "tot_recn_half_time", "tot_rech_wins" },
    { STAT_PERM_FULL_BEST_TIME, "tot_recn_full_time", "tot_rech_wins" },
    { STAT_PERM_EX_BEST_TIME, "tot_recn_ex_time", "tot_rech_wins" },
    { REC_EMPTY, "tot_recn_runstats_1", "tot_rech_runstats" },
    { STAT_PERM_FLOORS, "tot_recn_floors", "tot_rech_runstats" },
    { STAT_PERM_TURNS_SPENT, "tot_recn_turns", "tot_rech_runstats" },
    { STAT_PERM_TIMES_RAN_AWAY, "tot_recn_runaway", "tot_rech_runstats" },
    { STAT_PERM_ENEMIES_DEFEATED, "tot_recn_kills", "tot_rech_runstats" },
    { STAT_PERM_ENEMY_DAMAGE, "tot_recn_edamage", "tot_rech_runstats" },
    { STAT_PERM_PLAYER_DAMAGE, "tot_recn_pdamage", "tot_rech_runstats" },
    { STAT_PERM_SUPERGUARDS, "tot_recn_superguards", "tot_rech_runstats" },
    { STAT_PERM_CONDITIONS_MET, "tot_recn_conditions", "tot_rech_runstats" },
    { REC_EMPTY, "tot_recn_runstats_2", "tot_rech_runstats" },
    { STAT_PERM_FP_SPENT, "tot_recn_fpspent", "tot_rech_runstats" },
    { STAT_PERM_SP_SPENT, "tot_recn_spspent", "tot_rech_runstats" },
    { STAT_PERM_STAR_PIECES, "tot_recn_starpieces", "tot_rech_runstats" },
    { STAT_PERM_SHINE_SPRITES, "tot_recn_shinesprites", "tot_rech_runstats" },
    { STAT_PERM_COINS_EARNED, "tot_recn_coinsearned", "tot_rech_runstats" },
    { STAT_PERM_COINS_SPENT, "tot_recn_coinsspent", "tot_rech_runstats" },
    { STAT_PERM_ITEMS_USED, "tot_recn_itemsused", "tot_rech_runstats" },
    { STAT_PERM_ITEMS_BOUGHT, "tot_recn_itemsbought", "tot_rech_runstats" },
    { REC_EMPTY, "tot_recn_runstats_3", "tot_rech_runstats" },
    { STAT_PERM_NPC_WONKY_TRADES, "tot_recn_wonky", "tot_rech_runstats" },
    { STAT_PERM_NPC_DAZZLE_TRADES, "tot_recn_dazzle", "tot_rech_runstats" },
    { STAT_PERM_NPC_RIPPO_TRADES, "tot_recn_rippo", "tot_rech_runstats" },
    { STAT_PERM_NPC_LUMPY_TRADES, "tot_recn_lumpy", "tot_rech_runstats" },
    { STAT_PERM_NPC_GRUBBA_DEAL, "tot_recn_grubba", "tot_rech_runstats" },
    { STAT_PERM_NPC_DOOPLISS_DEAL, "tot_recn_doopliss", "tot_rech_runstats" },
    { STAT_PERM_NPC_MOVER_TRADES, "tot_recn_mover", "tot_rech_runstats" },
    { STAT_PERM_NPC_ZESS_COOKS, "tot_recn_zess", "tot_rech_runstats" },
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
        int32_t header_icon = IconType::MARIO_HEAD;
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
            uint32_t color = 
                record.option == REC_EMPTY ? 0x403030FFU : 0x000000FFU;
            const char* name =
                record.name_msg ? msgSearch(record.name_msg) : "";
            ttyd::win_main::winFontSet(&position, &scale, &color, name);
        }

        // Draw entry's value text.
        if (record.option != REC_EMPTY) {
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
                    sprintf(ptr, "???");
                    break;
                }
                case REC_ACHIEVEMENT_PCT: {
                    sprintf(ptr, "???");
                    break;
                }
                case REC_ITEM_PCT: {
                    sprintf(
                        ptr, "%" PRId32 " / %" PRId32,
                        menu->recipe_log_obtained_count,
                        menu->recipe_log_total_count);
                    break;
                }
                case REC_BADGE_PCT: {
                    sprintf(
                        ptr, "%" PRId32 " / %" PRId32,
                        menu->badge_log_obtained_count,
                        menu->badge_log_total_count);
                    break;
                }
                case REC_MOVE_PCT: {
                    sprintf(
                        ptr, "%" PRId32 " / %" PRId32,
                        menu->move_log_completed_count,
                        MoveType::MOVE_TYPE_MAX);
                    break;
                }
                case REC_TATTLE_PCT: {
                    sprintf(
                        ptr, "%" PRId32 " / %" PRId32,
                        menu->tattle_log_obtained_count,
                        menu->tattle_log_total_count);
                    break;
                }
                case REC_HUB_PROGRESS_PCT: {
                    sprintf(ptr, "???");
                    break;
                }
                case REC_HUB_ITEMS: {
                    sprintf(ptr, "???");
                    break;
                }
                case REC_HUB_BADGES: {
                    sprintf(ptr, "???");
                    break;
                }
                case REC_HUB_KEY_ITEMS: {
                    sprintf(ptr, "???");
                    break;
                }
                case REC_HUB_OPTIONS: {
                    sprintf(ptr, "???");
                    break;
                }
                case REC_HUB_MARIO_SKINS: {
                    sprintf(ptr, "???");
                    break;
                }
                case REC_HUB_YOSHI_SKINS: {
                    sprintf(ptr, "???");
                    break;
                }
                case REC_HUB_ATTACK_FX: {
                    sprintf(ptr, "???");
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
                default: {
                    ptr += IntegerToFmtString(
                        state.GetOption(record.option), ptr);
                    break;
                }
            }

            gc::vec3 position = { win_x + 250.0f, pos_y, 0.0f };
            gc::vec3 scale = { 0.75f, 0.75f, 0.75f };
            uint32_t color = 0x000000FFU;
            ttyd::win_main::winFontSet(&position, &scale, &color, buf);
        }
    }

    // Draw L/R button icon / arrows.
    if (page_first_entry != 0) {
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
    // Hardcode Journal menu to have three submenus: Tattles, Items, Badges.
    menu->log_menu_state = 0;
    menu->log_submenu_cursor_idx = 0;
    menu->log_submenu_count = 5;
    for (int32_t i = 0; i < menu->log_submenu_count; ++i) {
        auto& submenu = menu->log_submenu_info[i];
        submenu.x = 0.0f;
        submenu.target_x = 0.0f;
        const float banner_y = 45.0f * (menu->log_submenu_count * 0.5f - i);
        submenu.y = banner_y;
        submenu.target_y = banner_y;
        submenu.state = 0;
        submenu.timer = 0;
    }
    menu->log_submenu_info[0].id = 4;
    menu->log_submenu_info[0].help_msg = "msg_menu_kiroku_mono";
    menu->log_submenu_info[1].id = 3;
    menu->log_submenu_info[1].help_msg = "msg_menu_kiroku_ryori";
    menu->log_submenu_info[2].id = 2;
    menu->log_submenu_info[2].help_msg = "msg_menu_kiroku_badge";
    menu->log_submenu_info[3].id = 5;
    menu->log_submenu_info[3].help_msg = "msg_menu_move_log";
    menu->log_submenu_info[4].id = 7;
    menu->log_submenu_info[4].help_msg = "msg_menu_move_records";
  
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
  
    menu->tattle_log_total_count = 0;
    menu->tattle_log_obtained_count = 0;
    for (int32_t i = 1; i < 0xd8; ++i) {
        auto* monosiri = ttyd::battle_monosiri::battleGetUnitMonosiriPtr(i);
        if (monosiri->menu_tattle && tot::GetCustomTattleIndex(i) >= 0) {
            menu->tattle_logs[menu->tattle_log_total_count].order = 
                ttyd::win_root::enemy_monoshiri_sort_table[i];
            menu->tattle_logs[menu->tattle_log_total_count].id = i;
            
            // TODO: Restrict the total count to lower numbers based on which
            // Pits have been cleared / whether Gold Fuzzy has been encountered.
            ++menu->tattle_log_total_count;
            menu->tattle_log_obtained_count += ttyd::swdrv::swGet(i + 0x117a);
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
    menu->move_log_obtained_count = 0;
    menu->move_log_completed_count = 0;
    for (int32_t move = 0; move < MoveType::MOVE_TYPE_MAX; ++move) {
        uint32_t move_flags = g_Mod->state_.GetOption(STAT_PERM_MOVE_LOG, move);
        const auto* move_data = MoveManager::GetMoveData(move);
        if (move_flags & MoveLogFlags::UNLOCKED_LV_1)
            ++menu->move_log_obtained_count;

        // Check for whether the move has been unlocked (implicitly) and used
        // at every level, and had its Stylish commands performed, if any.
        bool completed = true;
        if (!(move_flags & MoveLogFlags::USED_LV_1))
            completed = false;
        if (move_data->max_level >= 2 && !(move_flags & MoveLogFlags::USED_LV_2))
            completed = false;
        if (move_data->max_level >= 3 && !(move_flags & MoveLogFlags::USED_LV_3))
            completed = false;
        if ((move < MoveType::SP_SWEET_TREAT || move > MoveType::SP_SUPERNOVA)
            && move != MoveType::GOOMBELLA_RALLY_WINK
            && (move_flags & MoveLogFlags::STYLISH_ALL) != MoveLogFlags::STYLISH_ALL)
            completed = false;

        if (completed) ++menu->move_log_completed_count;
    }
}

void LogMenuInit2(ttyd::win_root::WinPauseMenu* menu) {
    menu->main_cursor_target_x = -128.0f;
    menu->main_cursor_target_y = 8.0f +
        45.0f * (menu->log_submenu_count * 0.5f - menu->log_submenu_cursor_idx);

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
            }
            if (buttons_pressed & ButtonId::START) {
                return -2;
            }        
            if (menu->dirs_repeated & DirectionInputId::ANALOG_UP) {
                int32_t prev_idx = menu->log_submenu_cursor_idx;
                --menu->log_submenu_cursor_idx;
                if (menu->log_submenu_cursor_idx < 0) 
                    menu->log_submenu_cursor_idx = menu->log_submenu_count - 1;
                if (prev_idx != menu->log_submenu_cursor_idx)
                    ttyd::pmario_sound::psndSFXOn((char *)0x20005);
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_DOWN) {
                int32_t prev_idx = menu->log_submenu_cursor_idx;
                ++menu->log_submenu_cursor_idx;
                if (menu->log_submenu_cursor_idx >= menu->log_submenu_count) 
                    menu->log_submenu_cursor_idx = 0;
                if (prev_idx != menu->log_submenu_cursor_idx)
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
            
            menu->main_cursor_target_x = -128.0f;
            menu->main_cursor_target_y =
                 8.0f +
                 (45.0f * menu->log_submenu_count * 0.5f - 45.0f * menu->log_submenu_cursor_idx);
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
                if (menu->tattle_log_cursor_idx > 8)
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
        case 17: {
            // Move log.

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
                    if (--menu->records_log_cursor_idx < 0) {
                        menu->records_log_cursor_idx = 1;
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
                if (menu->records_log_cursor_idx < 0) {
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
                submenu.target_x = 0.0f;
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
                submenu.target_x = 0.0f;
                submenu.target_y = 140.0f;
                submenu.timer = 0;
                ++submenu.state;
                break;
            case 21:
                // Closing submenu, vertical slide.
                if (8 < ++submenu.timer) {
                    submenu.target_x = 0.0f;
                    submenu.target_y = 45.0f * (menu->log_submenu_count * 0.5f - i);
                    submenu.timer = 0;
                    ++submenu.state;
                }
                break;
            case 30:
                // Opened a different submenu; slide out.
                submenu.target_x = 0.0f;
                submenu.target_y = -160.0f;
                submenu.timer = 0;
                ++submenu.state;
                break;
            case 40:
                // Closed a different submenu; slide in.
                submenu.target_x = 0.0f;
                submenu.target_y = 45.0f * (menu->log_submenu_count * 0.5f - i);
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
  
    // Tattle log menu.
    switch (menu->log_menu_state) {
        case 14:
        case 20:
        case 21:
        case 22:
        case 1001:
             ttyd::win_log::monoshiriGX(win_x - 170.0f, win_y + 130.0f, menu);
             break;
    }

    // New menus.
    if (menu->log_menu_state == 15) {
        DrawMoveLog(menu, win_x - 170.0f, win_y + 130.0f);
    }
    if (menu->log_menu_state == 17) {
        DrawRecordsLog(menu, win_x - 170.0f, win_y + 130.0f);
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
    }
}

}  // namespace mod::tot::win