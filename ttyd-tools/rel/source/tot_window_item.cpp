#include "tot_window_item.h"

#include "common_types.h"
#include "mod.h"
#include "patches_item.h"
#include "tot_state.h"

#include <gc/types.h>
#include <ttyd/gx/GXTev.h>
#include <ttyd/gx/GXTransform.h>
#include <ttyd/dispdrv.h>
#include <ttyd/filemgr.h>
#include <ttyd/fontmgr.h>
#include <ttyd/icondrv.h>
#include <ttyd/item_data.h>
#include <ttyd/mario.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/mario_party.h>
#include <ttyd/msgdrv.h>
#include <ttyd/pmario_sound.h>
#include <ttyd/win_main.h>
#include <ttyd/win_mario.h>
#include <ttyd/win_party.h>
#include <ttyd/win_root.h>
#include <ttyd/winmgr.h>

namespace mod::tot::win {

namespace {

// For convenience.
using namespace ::ttyd::gx::GXTev;

using ::ttyd::item_data::itemDataTable;
using ::ttyd::msgdrv::msgSearch;
using ::ttyd::win_party::WinPartyData;
using ::ttyd::win_party::g_winPartyDt;
using ::ttyd::win_root::WinPauseMenu;
using ::ttyd::winmgr::WinMgrEntry;

namespace IconType = ::ttyd::icondrv::IconType;
namespace ItemType = ::ttyd::item_data::ItemType;
namespace ItemUseLocation = ::ttyd::item_data::ItemUseLocation_Flags;

void GetPartyMemberMenuOrder(WinPartyData** out_party_data) {
    WinPartyData* party_data = g_winPartyDt;
    // Get the currently active party member.
    const int32_t party_id = ttyd::mario_party::marioGetParty();
    
    // Put the currently active party member in the first slot.
    WinPartyData** current_order = out_party_data;
    for (int32_t i = 0; i < 7; ++i) {
        if (party_data[i].partner_id == party_id) {
            *current_order = party_data + i;
            ++current_order;
        }
    }
    // Put the remaining party members in the remaining slots, ordered by
    // the order they appear in g_winPartyDt.
    ttyd::mario_pouch::PouchPartyData* pouch_data = 
        ttyd::mario_pouch::pouchGetPtr()->party_data;
    for (int32_t i = 0; i < 7; ++i) {
        int32_t id = party_data[i].partner_id;
        if ((pouch_data[id].flags & 1) && id != party_id) {
            *current_order = party_data + i;
            ++current_order;
        }
    }
}

void DrawItemGrid(WinPauseMenu* menu, float win_x, float win_y) {  
    int32_t num_items = menu->item_submenu_id == 0 
        ? ttyd::mario_pouch::pouchGetHaveItemCnt() : menu->key_items_count;
    int32_t page_num = menu->items_page_num[menu->item_submenu_id];
  
    ttyd::gx::GXTransform::GXSetScissor(0, 109, 608, 192);
  
    for (int32_t i = 0; i < num_items; ++i) {
        int32_t item_type = menu->item_submenu_id == 0
            ? ttyd::mario_pouch::pouchHaveItem(i) : menu->key_items[i];
        
        float items_base_y = menu->items_offset_y[menu->item_submenu_id];
         
        bool item_active =
            menu->item_submenu_id != 0 ||
            (itemDataTable[item_type].usable_locations & ItemUseLocation::kField);
        
        float item_y = 
            items_base_y + 20.0f + 100.0f + win_y - (i / 2) * 38 - 10.0f;
        
        if (item_y <= 240.0f) {
            if (item_y < -240.0f) break;
            
            // Draw item icon.
            if (item_active) {
                ttyd::win_main::winIconInit();
            } else {
                ttyd::win_main::winIconGrayInit();
            }

            {
                gc::vec3 position = {
                    win_x - 125.0f + (i & 1) * 204,
                    item_y,
                    0.0f
                };
                float base_scale = menu->item_submenu_id == 0 ? 0.8f : 0.666667f;
                gc::vec3 scale = { base_scale, base_scale, base_scale };
                uint32_t color = 0xFFFFFFFFU;
                ttyd::win_main::winIconSet(
                    itemDataTable[item_type].icon_id, &position, &scale, &color);
            }

            // Draw item name.
            {
                ttyd::win_main::winFontInit();
                gc::vec3 position = {
                    win_x - 105.0f + (i & 1) * 204,
                    item_y + 10.0f,
                    0.0f
                };
                gc::vec3 scale = { 0.9f, 0.9f, 0.9f };
                uint32_t color = item_active ? 0x000000FFU : 0x808080FFU;
                const char* msg = msgSearch(itemDataTable[item_type].name);
                ttyd::win_main::winFontSetWidth(
                    &position, &scale, &color, 155.0f, msg);
            }
        }
    }

    ttyd::gx::GXTransform::GXSetScissor(0, 0, 608, 480);
    
    // Draw L button if not on first page of items.
    if (page_num > 0) {
        {
            ttyd::win_main::winTexInit(*menu->win_tpl->mpFileData);
            gc::vec3 position = {
                250.0f + win_x,
                20.0f + 18.0f + 110.0f + win_y,
                0.0f
            };
            gc::vec3 scale = { -1.0f, -1.0f, -1.0f };
            uint32_t color = 0xFFFFFFFFU;
            ttyd::win_main::winTexSet(0x17, &position, &scale, &color);
        }
        
        {
            ttyd::win_main::winIconInit();        
            gc::vec3 position = {
                250.0f + win_x,
                20.0f + 113.0f + win_y,
                0.0f
            };
            gc::vec3 scale = { 0.6f, 0.6f, 0.6f };
            uint32_t color = 0xFFFFFFFFU;
            ttyd::win_main::winIconSet(
                IconType::L_BUTTON, &position, &scale, &color);
        }
    }
  
    // Draw R button if not on final page of items.
    if (page_num * 10 + 10 < num_items) {
        {
            ttyd::win_main::winTexInit(*menu->win_tpl->mpFileData);
            gc::vec3 position = {
                250.0f + win_x,
                20.0f + win_y - 90.0f - 15.0f,
                0.0f
            };
            gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
            uint32_t color = 0xFFFFFFFFU;
            ttyd::win_main::winTexSet(0x17, &position, &scale, &color);
        }
        
        {
            ttyd::win_main::winIconInit();
            gc::vec3 position = {
                250.0f + win_x,
                20.0f + win_y - 90.0f,
                0.0f
            };
            gc::vec3 scale = { 0.6f, 0.6f, 0.6f };
            uint32_t color = 0xFFFFFFFFU;
            ttyd::win_main::winIconSet(
                IconType::R_BUTTON, &position, &scale, &color);
        }
    }
    
    // "Sort Methods" text
    const char* msg = msgSearch("msg_menu_sort_narabikae");
    int32_t sort_width = ttyd::fontmgr::FontGetMessageWidth(msg);
    {
        ttyd::win_main::winFontInit();
        gc::vec3 position = {
            win_x + 220.0f - 0.7f * sort_width,
            20.0f + win_y - 92.0f - 10.0f,
            0.0f
        };
        gc::vec3 scale = { 0.7f, 0.7f, 0.7f };
        uint32_t color = 0x000000FFU;
        ttyd::win_main::winFontSet(&position, &scale, &color, msg);
    }
    
    {
        // Z button icon
        ttyd::win_main::winIconInit();
        gc::vec3 position = {
            win_x + 220.0f - 0.7f * sort_width - 30.0f,
            20.0f + win_y - 100.0f - 10.0f,
            0.0f
        };
        gc::vec3 scale = { 0.6f, 0.6f, 0.6f };
        uint32_t color = 0xFFFFFFFFU;
        ttyd::win_main::winIconSet(
            IconType::FLAT_Z_BUTTON, &position, &scale, &color);
    }
}



void DrawItemInventorySize(WinPauseMenu* menu, float win_x, float win_y) {    
    // "Constant" colors.
    static uint32_t kBlack = 0x000000FFU;
    // static uint32_t kWhite = 0xFFFFFFFFU;
    static uint32_t kRed   = 0xA00000FFU;
    
    // Temporary variables, used across draw calls.
    gc::vec3 pos = { 0.0f, 0.0f, 0.0f };
    gc::vec3 scale = { 0.9f, 0.9f, 0.9f };
    int32_t width;
    
    // Top-left corner of the window, for reference.
    gc::vec3 win_pos = { win_x - 268.0f, win_y + 20.0f, 0.0f };
    gc::vec3 win_dim = { 100.0f, 40.0f, 0.0f };

    // Draw recessed window for item count.
    ttyd::win_root::winKirinukiGX(
        win_pos.x, win_pos.y, win_dim.x, win_dim.y, menu, 0);

    ttyd::win_main::winFontInit();
    
    // Draw slash between current and maximum item count.
    width = ttyd::fontmgr::FontGetMessageWidth("/");
    pos.x = win_pos.x + 50.0f - 0.5f * width;
    pos.y = win_pos.y - 9.0f;
    ttyd::win_main::winFontSet(&pos, &scale, &kBlack, "/");
    
    int32_t current_items = ttyd::mario_pouch::pouchGetHaveItemCnt();
    int32_t max_items = mod::infinite_pit::item::GetItemInventorySize();
    
    // Draw current and max inventory numbers.
    const char* temp_current_items = ttyd::win_mario::winZenkakuStr(current_items);
    width = ttyd::fontmgr::FontGetMessageWidth(temp_current_items);
    pos.x = win_pos.x + 30.0f - 0.5f * width;
    uint32_t* color = current_items < max_items ? &kBlack : &kRed;
    ttyd::win_main::winFontSet(&pos, &scale, color, "%s", temp_current_items);

    const char* temp_max_items = ttyd::win_mario::winZenkakuStr(max_items);
    width = ttyd::fontmgr::FontGetMessageWidth(temp_max_items);
    pos.x = win_pos.x + 70.0f - 0.5f * width;
    ttyd::win_main::winFontSet(&pos, &scale, &kBlack, "%s", temp_max_items);
    
    // Draw "Space used" string.
    const char* space_used = msgSearch("tot_menu_spaceused");
    width = ttyd::fontmgr::FontGetMessageWidth(space_used);
    scale.x = 0.6f;
    scale.y = 0.6f;
    scale.z = 0.6f;
    pos.x = win_pos.x + (win_dim.x - scale.x * width) * 0.5f ;
    pos.y = win_pos.y + scale.y * 26.0f + 2.0f;
    ttyd::win_main::winFontSet(&pos, &scale, &kBlack, space_used);
}

}  // namespace

int32_t ItemMenuMain(WinPauseMenu* menu) {
    
    switch (menu->item_menu_state) {
        case 0: {
            if (menu->buttons_pressed & ButtonId::START) {
                return -2;
            } else if (menu->buttons_pressed & ButtonId::B) {
                ttyd::pmario_sound::psndSFXOn((char *)0x20013);
                return -1;
            } else if (menu->buttons_pressed & ButtonId::A) {
                if (menu->item_submenu_id == 0 &&
                    ttyd::mario_pouch::pouchGetHaveItemCnt() == 0)
                    return 0;
                if (menu->item_submenu_id == 1 && menu->key_items_count == 0)
                    return 0;
                menu->item_menu_state = 10;
                ttyd::pmario_sound::psndSFXOn((char *)0x20012);
                return 0;
            } else if (menu->buttons_pressed & ButtonId::Z) {
                ttyd::win_root::winSortEntry(-310.0f, 150.0f, menu, menu->item_submenu_id);
                menu->parent_menu_state = menu->item_menu_state;
                ttyd::pmario_sound::psndSFXOn((char *)0x20012);
                menu->item_menu_state = 1000;
            } else if (menu->dirs_repeated & (DirectionInputId::ANALOG_UP | DirectionInputId::ANALOG_DOWN)) {
                menu->item_submenu_id ^= 1;
                ttyd::pmario_sound::psndSFXOn((char *)0x20005);
            }
            
            menu->main_cursor_target_x = -275.0f;
            menu->main_cursor_target_y = 125 - menu->item_submenu_id * 50;
            if (menu->item_submenu_id == 0) {
                ttyd::win_root::winMsgEntry(menu, 0, "msg_menu_mochi_item", 0);
            } else {
                ttyd::win_root::winMsgEntry(menu, 0, "msg_menu_mochi_daiji", 0);
            }
            break;
        }
        case 10: {
            int32_t num_items = menu->item_submenu_id == 0
                ? ttyd::mario_pouch::pouchGetHaveItemCnt() : menu->key_items_count;
            int32_t& cursor_idx = menu->items_cursor_idx[menu->item_submenu_id];
            int32_t& page_num = menu->items_page_num[menu->item_submenu_id];
            int32_t item = menu->item_submenu_id == 0
                ? ttyd::mario_pouch::pouchHaveItem(cursor_idx)
                : menu->key_items[cursor_idx];
            
            if (menu->buttons_pressed & ButtonId::B) {
                ttyd::pmario_sound::psndSFXOn((char *)0x20013);
                menu->item_menu_state = 0;
            } else if (menu->buttons_pressed & ButtonId::A) {
                if (menu->item_submenu_id == 0) {
                    if (itemDataTable[item].usable_locations & ItemUseLocation::kField) {
                        menu->use_item_type = item;
                        menu->use_item_idx = cursor_idx;
                        menu->item_menu_state = 300;
                        menu->use_item_menu_cursor_idx = 0;
                        menu->use_item_timer = 0;
                        ttyd::pmario_sound::psndSFXOn((char *)0x20012);
                        
                        auto* win = ttyd::winmgr::winMgrGetPtr(menu->winmgr_entry_1);
                        ttyd::winmgr::winMgrSetSize(
                            menu->winmgr_entry_1, win->x, win->y,
                            win->width, menu->party_count * 28 + 68);
                        ttyd::winmgr::winMgrOpen(menu->winmgr_entry_1);
                        ttyd::winmgr::winMgrOpen(menu->winmgr_entry_2);
                    }
                } else {
                    // TODO: Handle selecting key items.
                }
            } else if (menu->buttons_pressed & ButtonId::L) {
                if (--page_num < 0) page_num = 0;
                cursor_idx = page_num * 10;
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if (menu->buttons_pressed & ButtonId::R) {
                if (++page_num >= (num_items + 9) / 10) --page_num;
                cursor_idx = page_num * 10;
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if (menu->dirs_repeated & (DirectionInputId::ANALOG_LEFT | DirectionInputId::ANALOG_RIGHT)) {
                cursor_idx ^= 1;
                if (cursor_idx >= num_items) cursor_idx = num_items - 1;
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_UP) {
                if (cursor_idx >= 2) cursor_idx -= 2;
                page_num = cursor_idx / 10;
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_DOWN) {
                if (cursor_idx + 2 < num_items) {
                    cursor_idx += 2;
                } else if (cursor_idx == num_items - 2 && (num_items & 1)) {
                    cursor_idx += 1;
                }
                page_num = cursor_idx / 10;
                ttyd::pmario_sound::psndSFXOn((char *)0x20035);
            } else if (menu->buttons_pressed & ButtonId::Z) {
                ttyd::win_root::winSortEntry(-310.0f, 150.0f, menu, menu->item_submenu_id);
                menu->parent_menu_state = menu->item_menu_state;
                ttyd::pmario_sound::psndSFXOn((char *)0x20012);
                menu->item_menu_state = 1000;
            } else if (menu->buttons_pressed & ButtonId::START) {
                return -2;
            }
            
            menu->main_cursor_target_x = (cursor_idx & 1) * 204 - 155;
            menu->main_cursor_target_y = (cursor_idx % 10) / 2 * -38 + 110;
            ttyd::win_root::winMsgEntry(menu, item, itemDataTable[item].description, 0);
            
            break;
        }
        // case 1xx : Super Luigi menu (unused).
        // case 2xx : Email menu (unused).
        case 300: {
            if (menu->buttons_pressed & ButtonId::START) {
                menu->use_item_type = 0;
                ttyd::winmgr::winMgrClose(menu->winmgr_entry_1);
                ttyd::winmgr::winMgrClose(menu->winmgr_entry_2);
            } else if (menu->buttons_pressed & ButtonId::B) {
                ttyd::pmario_sound::psndSFXOn((char *)0x20013);
                menu->item_menu_state = 10;
                menu->use_item_type = 0;
                ttyd::winmgr::winMgrClose(menu->winmgr_entry_1);
                ttyd::winmgr::winMgrClose(menu->winmgr_entry_2);
            } else if (menu->buttons_pressed & ButtonId::A) {
                ttyd::pmario_sound::psndSFXOn((char *)0x20012);

                // Original game does check for whether to kick out of the menu
                // here, but I want to stay in the menu always anyway.
                
                // Sort party members: Mario, current member, remaining members.
                WinPartyData* party_members[8] = { nullptr };
                GetPartyMemberMenuOrder(party_members + 1);
                
                // Use item on selected party member.
                const auto& item_data = itemDataTable[menu->use_item_type];
                int32_t hp = item_data.hp_restored;
                int32_t fp = item_data.fp_restored;
                if (menu->use_item_type == ItemType::CAKE) {
                    hp += mod::infinite_pit::item::GetBonusCakeRestoration();
                    fp += mod::infinite_pit::item::GetBonusCakeRestoration();
                }

                if (menu->use_item_menu_cursor_idx < 1) {
                    ttyd::mario_pouch::pouchSetHP(
                        ttyd::mario_pouch::pouchGetHP() + hp);
                    ttyd::mario_pouch::pouchSetFP(
                        ttyd::mario_pouch::pouchGetFP() + fp);
                } else {
                    int32_t partner_id =
                        party_members[menu->use_item_menu_cursor_idx]->partner_id;
                    ttyd::mario_pouch::pouchSetPartyHP(
                        partner_id,
                        ttyd::mario_pouch::pouchGetPartyHP(partner_id) + hp);
                    ttyd::mario_pouch::pouchSetFP(
                        ttyd::mario_pouch::pouchGetFP() + fp);
                }

                // Track items used in the menu.
                g_Mod->state_.ChangeOption(STAT_RUN_ITEMS_USED);
                
                menu->use_item_timer = 0;
                menu->item_menu_state = 301;
                return 0;
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_UP) {
                if (--menu->use_item_menu_cursor_idx < 0)
                    menu->use_item_menu_cursor_idx = menu->party_count;
                if (menu->party_count)
                    ttyd::pmario_sound::psndSFXOn((char *)0x20005);
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_DOWN) {
                if (++menu->use_item_menu_cursor_idx > menu->party_count)
                    menu->use_item_menu_cursor_idx = 0;
                if (menu->party_count)
                    ttyd::pmario_sound::psndSFXOn((char *)0x20005);
            }
            
            // Set cursor position.
            int32_t cursor_idx = menu->use_item_menu_cursor_idx;
            menu->main_cursor_target_x = -80.0f;
            float y_offset = 0.0f;
            if (cursor_idx == 1) {
                y_offset = -16.0f;
            } else if (cursor_idx > 0) {
                y_offset -= 28.0f;
            }
            menu->main_cursor_target_y = 130.0f - 28.0f * cursor_idx + y_offset;
            
            break;
        }
        case 301: {
            if (++menu->use_item_timer > 120) {
                menu->use_item_timer = 0;
                ttyd::mario_pouch::pouchRemoveItemIndex(
                    menu->use_item_type, menu->use_item_idx);
                menu->item_menu_state = 10;
                menu->use_item_type = 0;
                if (ttyd::mario_pouch::pouchGetHaveItemCnt() == 0) {
                    menu->item_menu_state = 0;
                } else {
                    int32_t num_items = ttyd::mario_pouch::pouchGetHaveItemCnt();
                    if (num_items <= menu->items_cursor_idx[0]) {
                        menu->items_cursor_idx[0] = num_items - 1;
                    }
                }
                ttyd::winmgr::winMgrClose(menu->winmgr_entry_1);
                ttyd::winmgr::winMgrClose(menu->winmgr_entry_2);
            }
            break;
        }
        case 1000: {
            if (ttyd::win_root::winSortWait(menu) == 0) {
                menu->item_menu_state = menu->parent_menu_state;
            }
            if (menu->buttons_pressed & ButtonId::START) {
                return -2;
            }
            break;
        }
    }
    
    return 0;
}

void ItemMenuMain2(ttyd::win_root::WinPauseMenu* menu) {
    menu->items_target_y[0] = menu->items_page_num[0] * 190.0f;
    menu->items_offset_y[0] +=
        (menu->items_target_y[0] - menu->items_offset_y[0]) / 4.0f;

    menu->items_target_y[1] = menu->items_page_num[1] * 190.0f;
    menu->items_offset_y[1] +=
        (menu->items_target_y[1] - menu->items_offset_y[1]) / 4.0f;
}

void ItemMenuDisp(
    ttyd::dispdrv::CameraId camera_id, WinPauseMenu* menu, int32_t tab_number) {
    float win_x = menu->tab_body_info[tab_number].x;
    float win_y = menu->tab_body_info[tab_number].y;
  
    ttyd::win_root::winBgGX(win_x, win_y, menu, 2);
    ttyd::win_root::winKirinukiGX(
        win_x - 150.0f, 20.0f + 120.0f + win_y, 415.0f, 210.0f, menu, 0);
        
    // Draw icons for "Items" and "Important Items".
    for (int32_t i = 0; i < 2; ++i) {
        if (i == menu->item_submenu_id) {
            // Draw icon's drop shadow.
            ttyd::win_main::winTexInit(*menu->win_tpl->mpFileData);
            gc::vec3 position = {
                win_x - 215.0f,
                win_y + 20.0f + 105.0f - i * 50,
                0.0f
            };
            gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
            uint32_t color = 0x00000080U;
            ttyd::win_main::winTexSet(0xb3, &position, &scale, &color);
          
            // Draw icon.
            ttyd::win_main::winTexInit_x2(*menu->win_tpl->mpFileData);
            position.x -= 4.0f;
            position.y += 4.0f;
            color = 0xFFFFFFFFU;
            ttyd::win_main::winTexSet_x2(i + 0x18, 0xb3, &position, &scale, &color);
        } else {
            // Draw icon, greyed out.
            ttyd::win_main::winTexInit_x2(*menu->win_tpl->mpFileData);      
            gc::vec3 position = {
                win_x - 215.0f,
                win_y + 20.0f + 105.0f - i * 50,
                0.0f
            };
            gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
            uint32_t color = 0x808080FFU;
            ttyd::win_main::winTexSet_x2(i + 0x18, 0xb3, &position, &scale, &color);
        }
    }

    // Draw item icons.
    DrawItemGrid(menu, win_x, win_y);

    // Draw item inventory size if in regular item inventory.
    // Don't display if not in the middle of a run.
    if (!g_Mod->state_.GetOption(tot::OPT_RUN_STARTED)) return;
    // Only display if on the regular item inventory screen.
    if (menu->item_submenu_id != 0) return;
    DrawItemInventorySize(menu, win_x, win_y);
}

void ItemSubdialogMain1(WinMgrEntry* winmgr_entry) {
    
    auto* menu = (WinPauseMenu*)ttyd::win_main::winGetPtr();
    auto& pouch = *ttyd::mario_pouch::pouchGetPtr();
  
    if (ttyd::winmgr::winMgrAction(menu->winmgr_entry_1) == 0) {

        // Sort party members: Mario, current member, remaining members.
        WinPartyData* party_members[8] = { nullptr };
        GetPartyMemberMenuOrder(party_members + 1);
        
        for (int32_t i = 0; i < menu->party_count + 1; ++i) {
            float y_offset = 0.0f;
            if (i == 1) {
                y_offset = -16.0f;
            } else if (i != 0) {
                y_offset = -28.0f;
            }
          
            bool item_disabled = false;
            uint32_t text_color = 0x000000FFU;
            if (menu->use_item_timer != 0 && i != menu->use_item_menu_cursor_idx) {
                item_disabled = true;
                text_color = 0x808080FFU;
            }
          
            // If Mario...
            if (i == 0) {            
                if (menu->use_item_timer % 20 < 16 || i != menu->use_item_menu_cursor_idx) {
              
                    // Draw Mario head icon.
                  
                    if (item_disabled) {
                        ttyd::win_main::winIconGrayInit();
                    } else {
                        ttyd::win_main::winIconInit();
                    }
                    {
                        gc::vec3 position = {
                            winmgr_entry->x + 30.0f,
                            winmgr_entry->y - 20.0f - (i * 22),
                            0.0f
                        };              
                        gc::vec3 scale = { 0.65f, 0.65f, 0.65f };
                        uint32_t color = 0xFFFFFFFFU;
                        ttyd::win_main::winIconSet(
                            IconType::MARIO_HEAD, &position, &scale, &color);
                    }
                  
                    // Color spots behind "HP" and "FP".
                  
                    ttyd::win_main::winTexInit(*menu->win_tpl->mpFileData);
                    ttyd::gx::GXTev::GXSetTevColorIn(
                        GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
                    ttyd::gx::GXTev::GXSetTevAlphaIn(
                        GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_TEXA, GX_CA_ZERO);
                    {
                        int32_t width = ttyd::fontmgr::FontGetMessageWidth("HP");
                        gc::vec3 position = {
                            0.9f * width * 0.5f + winmgr_entry->x + 220,
                            winmgr_entry->y - 19.0f - (i * 22),
                            0.0f
                        };
                        gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
                        uint32_t color = 0xCB8CA2FEU;
                        ttyd::win_main::winTexSet(0xaf, &position, &scale, &color);
                    }
                    {
                        int32_t width = ttyd::fontmgr::FontGetMessageWidth("FP");
                        gc::vec3 position = {
                            0.9f * width * 0.5f + winmgr_entry->x + 220,
                            winmgr_entry->y - 41.0f,
                            0.0f
                        };
                        gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
                        uint32_t color = 0xCBBE89FEU;
                        ttyd::win_main::winTexSet(0xaf, &position, &scale, &color);
                    }
                  
                    // Text for Mario's HP and FP.
                  
                    ttyd::win_main::winFontInit();
                    {
                        gc::vec3 position = {
                            winmgr_entry->x + 50.0f,
                            winmgr_entry->y - 8.0f - (i * 22),
                            0.0f
                        };
                        gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
                        uint32_t color = text_color;
                        const char* msg = msgSearch("name_mario");
                        ttyd::win_main::winFontSet(&position, &scale, &color, msg);
                    }
                    {
                        gc::vec3 position = {
                            winmgr_entry->x + 220.0f,
                            winmgr_entry->y - 8.0f - (i * 22),
                            0.0f
                        };
                        gc::vec3 scale = { 0.9f, 0.9f, 0.9f };
                        uint32_t color = text_color;
                        ttyd::win_main::winFontSet(&position, &scale, &color, "HP");
                    }
                    {
                        gc::vec3 position = {
                            winmgr_entry->x + 245.0f,
                            winmgr_entry->y - 8.0f - (i * 22),
                            0.0f
                        };    
                        gc::vec3 scale = { 0.9f, 0.9f, 0.9f };
                        uint32_t color = text_color;
                        ttyd::win_main::winFontSetR(
                            &position, &scale, &color, "%d", 
                            ttyd::mario_pouch::pouchGetHP());
                    }
                    {
                        gc::vec3 position = {
                            winmgr_entry->x + 267.0f,
                            winmgr_entry->y - 8.0f - (i * 22),
                            0.0f
                        };    
                        gc::vec3 scale = { 0.8f, 0.8f, 0.8f };
                        uint32_t color = text_color;
                        ttyd::win_main::winFontSetR(&position, &scale, &color, "/");
                    }
                    { 
                        gc::vec3 position = {
                            winmgr_entry->x + 310.0f,
                            winmgr_entry->y - 10.0f - (i * 22),
                            0.0f
                        };
                        gc::vec3 scale = { 0.8f, 0.8f, 0.8f };
                        uint32_t color = text_color;
                        ttyd::win_main::winFontSet(
                            &position, &scale, &color, "%d", 
                            ttyd::mario_pouch::pouchGetMaxHP());
                    }

                    // Skip drawing FP if item was just used on party member
                    // and the text should be flashing.
                    if (itemDataTable[menu->use_item_type].fp_restored != 0) {
                        if (menu->use_item_timer % 20 >= 16)
                            continue;
                        text_color = 0x000000FFU;
                    }

                    ttyd::win_main::winFontInit();
                    {
                        gc::vec3 position = {
                            winmgr_entry->x + 220.0f,
                            winmgr_entry->y - 30.0f,
                            0.0f
                        };
                        gc::vec3 scale = { 0.9f, 0.9f, 0.9f };
                        uint32_t color = text_color;
                        ttyd::win_main::winFontSet(&position, &scale, &color, "FP");
                    }
                    {
                        gc::vec3 position = {
                            winmgr_entry->x + 245.0f,
                            winmgr_entry->y - 30.0f,
                            0.0f
                        };
                        gc::vec3 scale = { 0.9f, 0.9f, 0.9f };
                        uint32_t color = text_color;
                        ttyd::win_main::winFontSetR(
                            &position, &scale, &color, "%d", 
                            ttyd::mario_pouch::pouchGetFP()); 
                    }             
                    {
                        gc::vec3 position = {
                            winmgr_entry->x + 267.0f,
                            winmgr_entry->y - 30.0f,
                            0.0f
                        };
                        gc::vec3 scale = { 0.8f, 0.8f, 0.8f };
                        uint32_t color = text_color;
                        ttyd::win_main::winFontSetR(&position, &scale, &color, "/");
                    }
                    {
                        gc::vec3 position = {
                            winmgr_entry->x + 310.0f,
                            winmgr_entry->y - 30.0f,
                            0.0f
                        };
                        gc::vec3 scale = { 0.8f, 0.8f, 0.8f };
                        uint32_t color = text_color;
                        ttyd::win_main::winFontSet(
                            &position, &scale, &color, "%d", 
                            ttyd::mario_pouch::pouchGetMaxFP());
                    }
                }
            } else {
                // Otherwise, partner...
                int32_t partner_id = party_members[i]->partner_id;
                const char* party_name;
                if (partner_id == 4) {
                    party_name = ttyd::mario_pouch::pouchGetYoshiName();
                } else {
                    party_name = msgSearch(party_members[i]->name);
                }
            
                if (menu->use_item_timer % 20 < 16 || i != menu->use_item_menu_cursor_idx) {
                    // Color spots behind "HP" and "FP".
                    ttyd::win_main::winTexInit(*menu->win_tpl->mpFileData);
                    ttyd::gx::GXTev::GXSetTevColorIn(
                        GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
                    ttyd::gx::GXTev::GXSetTevAlphaIn(
                        GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_TEXA, GX_CA_ZERO);
                    {
                        int32_t width = ttyd::fontmgr::FontGetMessageWidth("HP");
                        gc::vec3 position = {
                            0.9f * width * 0.5f + winmgr_entry->x + 220,
                            y_offset + winmgr_entry->y - 8 - (i * 28) - 14.0f,
                            0.0f
                        };
                        gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
                        uint32_t color = 0xCB8CA2FEU;
                        ttyd::win_main::winTexSet(0xaf, &position, &scale, &color);
                    }

                    // Partner icon.
                    if (item_disabled) {
                        ttyd::win_main::winIconGrayInit();
                    }
                    else {
                        ttyd::win_main::winIconInit();
                    }
                    {
                        gc::vec3 position = {
                            winmgr_entry->x + 30.0f,
                            y_offset + winmgr_entry->y - 20.0f - (i * 28),
                            0.0f
                        };              
                        gc::vec3 scale = { 0.65f, 0.65f, 0.65f };
                        uint32_t color = 0xFFFFFFFFU;
                        ttyd::win_main::winIconSet(
                            party_members[i]->icon_id, &position, &scale, &color);
                    }

                    // Text for partner's HP.
                  
                    ttyd::win_main::winFontInit();
                    {
                        int32_t width = ttyd::fontmgr::FontGetMessageWidth(party_name);
                        float x_scale = 1.0f;
                        if (160.0f <= width) {
                            x_scale = 160.0f / width;
                        }
                        gc::vec3 position = {
                            winmgr_entry->x + 50.0f,
                            y_offset + winmgr_entry->y - 8 - (i * 28),
                            0.0f
                        };
                        gc::vec3 scale = { x_scale, 1.0f, 1.0f };
                        uint32_t color = text_color;
                        ttyd::win_main::winFontSet(&position, &scale, &color, party_name);
                    }
                    {
                        gc::vec3 position = {
                            winmgr_entry->x + 220.0f,
                            y_offset + winmgr_entry->y - 8 - (i * 28),
                            0.0f
                        };
                        gc::vec3 scale = { 0.9f, 0.9f, 0.9f };
                        uint32_t color = text_color;
                        ttyd::win_main::winFontSet(&position, &scale, &color, "HP");
                    }
                    {    
                        gc::vec3 position = {
                            winmgr_entry->x + 245.0f,
                            y_offset + winmgr_entry->y - 8 - (i * 28),
                            0.0f
                        };
                        gc::vec3 scale = { 0.9f, 0.9f, 0.9f };
                        uint32_t color = text_color;
                        ttyd::win_main::winFontSetR(
                            &position, &scale, &color, "%d",
                            pouch.party_data[partner_id].current_hp);              
                    }
                    {   
                        gc::vec3 position = {
                            winmgr_entry->x + 267.0f,
                            y_offset + winmgr_entry->y - 8 - (i * 28),
                            0.0f
                        };
                        gc::vec3 scale = { 0.8f, 0.8f, 0.8f };
                        uint32_t color = text_color;
                        ttyd::win_main::winFontSetR(&position, &scale, &color, "/");
                    }
                    {
                        gc::vec3 position = {
                            winmgr_entry->x + 310.0f,
                            y_offset + winmgr_entry->y - 8 - (i * 28),
                            0.0f
                        };
                        gc::vec3 scale = { 0.8f, 0.8f, 0.8f };
                        uint32_t color = text_color;
                        ttyd::win_main::winFontSet(
                            &position, &scale, &color, "%d",
                            pouch.party_data[partner_id].max_hp);
                    }
                }
            }
        }
        
        if (menu->party_count > 1) {
            // Draw dotted line under first partner.
            ttyd::win_main::winIconInit();
            for (int32_t i = 2; i < 30; ++i) {            
                gc::vec3 position = {
                    winmgr_entry->x + (float)(winmgr_entry->width * i / 31),
                    winmgr_entry->y - 84.0f,
                    0.0f
                };
                gc::vec3 scale = { 0.15f, 0.15f, 0.15f };
                uint32_t color = 0x000000FFU;
                ttyd::win_main::winIconSet(
                    IconType::SP_ORB_YELLOW, &position, &scale, &color);
            }
        }
    }
  
    // Redraw main cursor + its drop shadow over the window.
    ttyd::win_main::winTexInit(*menu->win_tpl->mpFileData);
    {
        gc::vec3 position = {
            menu->main_cursor_x + 2.0f,
            menu->main_cursor_y - 2.0f,
            0.0f
        };
        gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
        uint32_t color = 0x00000080U;
        ttyd::win_main::winTexSet(0, &position, &scale, &color);
    }
    {
        gc::vec3 position = {
            menu->main_cursor_x,
            menu->main_cursor_y,
            0.0f
        };
        gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
        uint32_t color = 0xFFFFFFFFU;
        ttyd::win_main::winTexSet(0, &position, &scale, &color);
    }
}

void ItemSubdialogMain2(WinMgrEntry* winmgr_entry) {
    auto* menu = (WinPauseMenu*)ttyd::win_main::winGetPtr();
    if (ttyd::winmgr::winMgrAction(menu->winmgr_entry_2) == 0) {
        // "Use it on whom?"
        const char* msg = msgSearch("msg_window_select_5");
        uint16_t lines;
        int32_t text_length = ttyd::fontmgr::FontGetMessageWidthLine(msg, &lines);
        if (winmgr_entry->width - 20 < text_length) {
            text_length = winmgr_entry->width - 20;
        }
    
        ttyd::winmgr::winMgrSetSize(
            menu->winmgr_entry_2, winmgr_entry->x, winmgr_entry->y,
            winmgr_entry->width, winmgr_entry->desc->height + lines * 22);
            
        ttyd::win_main::winFontInit();
        
        gc::vec3 position = {
            (winmgr_entry->width - text_length) * 0.5f + winmgr_entry->x,
            134.0f,
            0.0f
        };
        gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
        uint32_t color = 0x000000FFU;
        ttyd::win_main::winFontSetWidth(
            &position, &scale, &color, winmgr_entry->width - 20, msg);
    }
}

}  // namespace mod::tot::win