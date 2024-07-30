#include "tot_window_log.h"

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
        - itemDataTable[*lhs + 0x80].type_sort_order;
    const int32_t rhs_sort =
        (itemDataTable[*rhs + 0x80].bp_cost << 16)
        - itemDataTable[*rhs + 0x80].type_sort_order;
    return lhs_sort - rhs_sort;
}

int32_t CompBadgeBP_R(int16_t* lhs, int16_t* rhs) {
    // Sort by cost descending, then type ascending.
    const int32_t lhs_sort =
        ((10 - itemDataTable[*lhs + 0x80].bp_cost) << 16)
        - itemDataTable[*lhs + 0x80].type_sort_order;
    const int32_t rhs_sort =
        ((10 - itemDataTable[*rhs + 0x80].bp_cost) << 16)
        - itemDataTable[*rhs + 0x80].type_sort_order;
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
    menu->log_submenu_count = 3;
    for (int32_t i = 0; i < 3; ++i) {
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
                if (menu->badge_log_cursor_idx % 7 < 6 &&
                    menu->badge_log_cursor_idx + 1 < total)
                    ++menu->badge_log_cursor_idx;
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_LEFT) {
                if (menu->badge_log_cursor_idx % 7 > 0)
                    --menu->badge_log_cursor_idx;
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
            menu->main_cursor_target_x = -140 + (menu->badge_log_cursor_idx % 7) * 56;
            menu->main_cursor_target_y = 90 - ((menu->badge_log_cursor_idx % 28) / 7) * 56;
      
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
                if (menu->recipe_log_cursor_idx % 7 < 6 &&
                    menu->recipe_log_cursor_idx + 1 < total)
                    ++menu->recipe_log_cursor_idx;
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_LEFT) {
                if (menu->recipe_log_cursor_idx % 7 > 0)
                    --menu->recipe_log_cursor_idx;
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
            menu->main_cursor_target_x = -140 + (menu->recipe_log_cursor_idx % 7) * 56;
            menu->main_cursor_target_y = 90 - ((menu->recipe_log_cursor_idx % 28) / 7) * 56;
      
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
                if (++menu->tattle_log_cursor_idx > total)
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
  
    // Draw banners for submenus.
    for (int32_t i = 0; i < menu->log_submenu_count; ++i) {
        const auto& submenu = menu->log_submenu_info[i];
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
                ttyd::win_main::winTexSet_x2(submenu.id + 0x1f, 0xb5, &position, &scale, &color);
            }
        } else {
            ttyd::win_main::winTexInit_x2(*menu->win_tpl->mpFileData);
            gc::vec3 position = { win_x + submenu.x, win_y + submenu.y, 0.0f };
            gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
            uint32_t color = 0x808080FFU;
            ttyd::win_main::winTexSet_x2(submenu.id + 0x1f, 0xb5, &position, &scale, &color);
        }
    }
}

}  // namespace mod::tot::win