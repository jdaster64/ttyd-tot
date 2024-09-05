#include "tot_window_item.h"

#include "common_functions.h"
#include "common_types.h"
#include "mod.h"
#include "patches_item.h"
#include "tot_generate_item.h"
#include "tot_generate_reward.h"
#include "tot_gsw.h"
#include "tot_manager_cosmetics.h"
#include "tot_state.h"

#include <gc/types.h>
#include <ttyd/gx/GXTev.h>
#include <ttyd/gx/GXTransform.h>
#include <ttyd/animdrv.h>
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
#include <ttyd/sound.h>
#include <ttyd/system.h>
#include <ttyd/win_badge.h>
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

// Array used for item selector dialog (no reason to shoehorn into menu struct.)
int16_t g_SelectorItems[85] = { -1 };

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
         
        bool item_active = true;
        if (menu->item_submenu_id == 0) {
            if (!(itemDataTable[item_type].usable_locations 
                & ItemUseLocation::kField))
                item_active = false;
        } else {
            switch (item_type) {
                case ItemType::TOT_KEY_PEEKABOO:
                    item_active = GetSWF(GSWF_PeekabooEnabled);
                    break;
                case ItemType::TOT_KEY_SUPER_PEEKABOO:
                    item_active = GetSWF(GSWF_SuperPeekabooEnabled);
                    break;
                case ItemType::TOT_KEY_TIMING_TUTOR:
                    item_active = GetSWF(GSWF_TimingTutorEnabled);
                    break;
                case ItemType::TOT_KEY_BGM_TOGGLE:
                    item_active = GetSWF(GSWF_BgmEnabled);
                    break;
                case ItemType::TOT_KEY_YOSHI_COSTUME:
                case ItemType::TOT_KEY_ITEM_SELECTOR:
                case ItemType::TOT_KEY_BADGE_SELECTOR:
                    item_active = !g_Mod->state_.GetOption(OPT_RUN_STARTED);
                    break;
            }
        }
        
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

void DrawUseItemDialog(WinMgrEntry* winmgr_entry) {
    auto* menu = (WinPauseMenu*)ttyd::win_main::winGetPtr();
    auto& pouch = *ttyd::mario_pouch::pouchGetPtr();

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
                    int32_t costume = GetSWByte(GSW_MarioCostume);
                    int32_t icon = CosmeticsManager::GetMarioCostumeData(costume)->icon;
                    ttyd::win_main::winIconSet(icon, &position, &scale, &color);
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

void DrawCosmeticSelectionDialog(WinMgrEntry* winmgr_entry) {
    auto* menu = (WinPauseMenu*)ttyd::win_main::winGetPtr();

    // Limit drawing space to within the dialog.
    ttyd::gx::GXTransform::GXSetScissor(
        winmgr_entry->x + 304, -winmgr_entry->y + 16 + 240,
        winmgr_entry->width, winmgr_entry->height - 32);

    for (int32_t i = 0; i < menu->cosmetic_num_options; ++i) {
        int32_t id = menu->cosmetic_options[i];

        const char* text = "???";
        int32_t icon = 0;
        switch (menu->item_menu_state) {
            case 400: {
                auto* data = CosmeticsManager::GetAttackFxData(id);
                text = msgSearch(data->name_msg);
                icon = data->icon;
                break;
            }
            case 401: {
                auto* data = CosmeticsManager::GetMarioCostumeData(id);
                text = msgSearch(data->name_msg);
                icon = data->icon;
                break;
            }
            case 402: {
                auto* data = CosmeticsManager::GetYoshiCostumeData(id);
                text = msgSearch(data->name_msg);
                icon = data->icon;
                break;
            }
        }

        float y_trans = 
            winmgr_entry->y + menu->cosmetic_menu_scroll_y - i * 24.0f - 28.0f;

        // Skip entry if out of visible window.
        if (y_trans - 32.0f > winmgr_entry->y ||
            y_trans + 32.0f < winmgr_entry->y - winmgr_entry->height)
            continue;
        
        // Placeholders.
        ttyd::win_main::winFontInit();
        gc::vec3 text_pos = { winmgr_entry->x + 55.0f, y_trans + 12.0f, 0.0f };
        gc::vec3 text_scale = { 1.0f, 1.0f, 1.0f };
        uint32_t text_color = 0x000000FFU;
        ttyd::win_main::winFontSet(&text_pos, &text_scale, &text_color, text);
        
        ttyd::win_main::winIconInit();
        {
            gc::vec3 pos = { winmgr_entry->x + 30.0f, y_trans, 0.0f };
            gc::vec3 scale = { 0.5f, 0.5f, 0.5f };
            uint32_t icon_color = 0xFFFFFFFFU;
            ttyd::win_main::winIconSet(icon, &pos, &scale, &icon_color);
        }
        {
            // "Equipped" indicator.
            int32_t equipped =
                CosmeticsManager::IsEquipped(menu->item_menu_state - 400, id)
                ? IconType::AC_LIGHT_GREEN : IconType::SP_ORB_EMPTY;
            gc::vec3 pos = { 
                winmgr_entry->x + winmgr_entry->width - 24.0f,
                y_trans, 
                0.0f
            };
            gc::vec3 scale = { 0.6f, 0.6f, 0.6f };
            uint32_t icon_color = 0xFFFFFFFFU;
            ttyd::win_main::winIconSet(equipped, &pos, &scale, &icon_color);
        }
    }

    ttyd::gx::GXTransform::GXSetScissor(0, 0, 608, 480);
}

int32_t PopulateSelectorItems(int32_t type) {
    int32_t num_items = 0;
    int32_t start_item =
        type == 0 ? ItemType::THUNDER_BOLT : ItemType::POWER_JUMP;
    int32_t end_item =
        type == 0 ? ItemType::POWER_JUMP : ItemType::MAX_ITEM_TYPE;

    for (int32_t item = start_item; item < end_item; ++item) {
        // If item is valid & purchased, add to array.
        if (g_Mod->state_.GetOption(
                FLAGS_ITEM_PURCHASED, item - ItemType::THUNDER_BOLT) &&
            ttyd::item_data::itemDataTable[item].type_sort_order >= 0) {
            g_SelectorItems[num_items++] = item;
        }
    }
    g_SelectorItems[num_items] = -1;
    
    ttyd::system::qqsort(
        g_SelectorItems, num_items, sizeof(int16_t),
        (void*)TypeSortOrderComparator);

    return num_items;
}

void SetCurrentItemLoadoutEquipped(WinPauseMenu* menu, bool toggle) {
    uint32_t size_option = menu->item_menu_state == 500
        ? STAT_PERM_ITEM_LOAD_SIZE : STAT_PERM_BADGE_LOAD_SIZE;
    uint32_t loadout_option = menu->item_menu_state == 500
        ? STAT_PERM_ITEM_LOADOUT : STAT_PERM_BADGE_LOADOUT;

    int32_t size = g_Mod->state_.GetOption(size_option);
    if (toggle) {
        int32_t current_id = g_SelectorItems[menu->cosmetic_cursor_idx];

        // Don't allow equipping multiple copies of unique badges.
        bool already_equipped = false;
        if (RewardManager::IsUniqueBadge(current_id)) {
            for (int32_t i = 0; i < size; ++i) {
                int32_t item = g_Mod->state_.GetOption(loadout_option, i)
                    + ItemType::THUNDER_BOLT;
                if (item == current_id) {
                    already_equipped = true;
                    break;
                }
            }
        }

        if (!already_equipped && size < 6) {
            // Equip the currently selected item.
            g_Mod->state_.SetOption(
                loadout_option, current_id - ItemType::THUNDER_BOLT, size);
            g_Mod->state_.ChangeOption(size_option, 1);
            ttyd::pmario_sound::psndSFXOn((char *)0x20038);
        } else {
            ttyd::sound::SoundEfxPlayEx(0x266, 0, 0x64, 0x40);
        }
    } else {
        if (size > 0) {
            // Remove the last added item.
            g_Mod->state_.ChangeOption(size_option, -1);
            ttyd::pmario_sound::psndSFXOn((char *)0x20013);
        } else {
            ttyd::sound::SoundEfxPlayEx(0x266, 0, 0x64, 0x40);
        }
    }
}

void DrawLoadoutSelectionDialog(WinMgrEntry* winmgr_entry) {
    auto* menu = (WinPauseMenu*)ttyd::win_main::winGetPtr();

    uint32_t size_option = menu->item_menu_state == 500
        ? STAT_PERM_ITEM_LOAD_SIZE : STAT_PERM_BADGE_LOAD_SIZE;
    uint32_t loadout_option = menu->item_menu_state == 500
        ? STAT_PERM_ITEM_LOADOUT : STAT_PERM_BADGE_LOADOUT;

    // Draw item icons.
    ttyd::win_main::winIconInit();
    for (int32_t i = 0; i < menu->cosmetic_num_options; ++i) {
        int32_t icon = ttyd::item_data::itemDataTable[g_SelectorItems[i]].icon_id;
        gc::vec3 pos = { 
            -62.0f + 30.0f * (i % 13 - 6),
            124.0f - 30.0f * (i / 13),
            0.0f
        };
        gc::vec3 scale = { 0.6f, 0.6f, 0.6f };
        uint32_t icon_color = 0xFFFFFFFFU;
        ttyd::win_main::winIconSet(icon, &pos, &scale, &icon_color);
    }

    // Draw selected loadout icons.
    for (int32_t i = 0; i < g_Mod->state_.GetOption(size_option); ++i) {
        int32_t item = g_Mod->state_.GetOption(loadout_option, i)
            + ItemType::THUNDER_BOLT;

        int32_t icon = ttyd::item_data::itemDataTable[item].icon_id;
        gc::vec3 pos = { 50.0f * (i - 2.5f), -91.0f, 0.0f };
        gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
        uint32_t icon_color = 0xFFFFFFFFU;
        ttyd::win_main::winIconSet(icon, &pos, &scale, &icon_color);
    }

    // Draw tutorial instructions on side.
    const float tut_x = 0.89f;
    const float tut_icon_y = 0.13f;
    const float tut_text_y = 0.21f;
    const float tut_spread = 0.31f;
    const int32_t tut_icons[] = {
        IconType::A_BUTTON, IconType::X_BUTTON, IconType::B_BUTTON
    };
    const char* tut_msg[] = {
        "tot_loadoutsel_add", "tot_loadoutsel_remove", "tot_loadoutsel_back"
    };

    for (int32_t i = 0; i < 3; ++i) {
        gc::vec3 pos = { 
            winmgr_entry->x + winmgr_entry->width * tut_x,
            winmgr_entry->y - winmgr_entry->height *(tut_icon_y + tut_spread * i),
            0.0f
        };
        gc::vec3 scale = { 0.6f, 0.6f, 0.6f };
        uint32_t icon_color = 0xFFFFFFFFU;
        ttyd::win_main::winIconSet(tut_icons[i], &pos, &scale, &icon_color);
    }
    
    ttyd::win_main::winFontInit();
    for (int32_t i = 0; i < 3; ++i) {
        const char* text = msgSearch(tut_msg[i]);
        int16_t width = ttyd::fontmgr::FontGetMessageWidth(text);
        if (width > 120) width = 120;
        gc::vec3 pos = {
            winmgr_entry->x + winmgr_entry->width * tut_x - width * 0.375f,
            winmgr_entry->y - winmgr_entry->height * (tut_text_y + tut_spread * i),
            0.0f
        };
        gc::vec3 scale = { 0.75f, 0.75f, 0.75f };
        uint32_t color = 0x000000FFU;
        ttyd::win_main::winFontSet(&pos, &scale, &color, text);
    }
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
                        
                        ttyd::winmgr::winMgrSetSize(
                            menu->winmgr_entry_1, -46.f, 150.f, 350.f, 
                            menu->party_count * 28 + 68);
                        ttyd::winmgr::winMgrSetSize(
                            menu->winmgr_entry_2, -280.f, 145.f, 200.f, 50.f);

                        ttyd::winmgr::winMgrOpen(menu->winmgr_entry_1);
                        ttyd::winmgr::winMgrOpen(menu->winmgr_entry_2);
                    }
                } else {
                    // Open selection menu for cosmetic selectors.
                    int32_t cosmetic_type = -1;
                    switch (item) {
                        case ItemType::TOT_KEY_ATTACK_FX:
                            cosmetic_type = CosmeticType::ATTACK_FX;
                            break;
                        case ItemType::TOT_KEY_MARIO_COSTUME:
                            cosmetic_type = CosmeticType::MARIO_COSTUME;
                            break;
                        case ItemType::TOT_KEY_YOSHI_COSTUME:
                            if (!g_Mod->state_.GetOption(OPT_RUN_STARTED)) {
                                cosmetic_type = CosmeticType::YOSHI_COSTUME;
                            } else {
                                ttyd::sound::SoundEfxPlayEx(0x266, 0, 0x64, 0x40);
                            }
                            break;
                    }
                    if (cosmetic_type != -1) {
                        menu->cosmetic_num_options = 0;
                        for (int32_t i = 0; i < 30; ++i) {
                            if (CosmeticsManager::IsAvailable(
                                cosmetic_type, i)) {
                                menu->cosmetic_options[
                                    menu->cosmetic_num_options++] = i;
                            }
                        }
                        menu->cosmetic_cursor_idx = 0;
                        menu->cosmetic_menu_offset = 0;
                        menu->cosmetic_menu_scroll_y = 0;
                        menu->cosmetic_menu_scroll_target_y = 0;

                        menu->item_menu_state = 400 + cosmetic_type;
                        ttyd::pmario_sound::psndSFXOn((char *)0x20012);
                        
                        int32_t height =
                            Min(menu->cosmetic_num_options, 8) * 24 + 32;
                        
                        ttyd::winmgr::winMgrSetSize(
                            menu->winmgr_entry_1, -46.f, 150.f, 350.f, height);
                        ttyd::winmgr::winMgrSetSize(
                            menu->winmgr_entry_2, -280.f, 145.f, 200.f, 50.f);
                        ttyd::winmgr::winMgrOpen(menu->winmgr_entry_1);
                        ttyd::winmgr::winMgrOpen(menu->winmgr_entry_2);
                    }

                    // Open selection menu for item / badge loadout.
                    int32_t selector_type = -1;
                    switch (item) {
                        case ItemType::TOT_KEY_ITEM_SELECTOR:
                            if (!g_Mod->state_.GetOption(OPT_RUN_STARTED)) {
                                selector_type = 0;
                            } else {
                                ttyd::sound::SoundEfxPlayEx(0x266, 0, 0x64, 0x40);
                            }
                            break;
                        case ItemType::TOT_KEY_BADGE_SELECTOR:
                            if (!g_Mod->state_.GetOption(OPT_RUN_STARTED)) {
                                selector_type = 1;
                            } else {
                                ttyd::sound::SoundEfxPlayEx(0x266, 0, 0x64, 0x40);
                            }
                            break;
                    }
                    if (selector_type != -1) {
                        int32_t num_selections = 
                            PopulateSelectorItems(selector_type);
                        if (num_selections > 0) {
                            menu->cosmetic_cursor_idx = 0;
                            menu->cosmetic_num_options = num_selections;
                            menu->item_menu_state = 500 + selector_type;
                            ttyd::pmario_sound::psndSFXOn((char *)0x20012);

                            ttyd::winmgr::winMgrSetSize(
                                menu->winmgr_entry_1, -270.f, 148.f, 540.f, 198.f);
                            ttyd::winmgr::winMgrSetSize(
                                menu->winmgr_entry_2, -160.f, -57.f, 320.f, 68.f);
                            ttyd::winmgr::winMgrOpen(menu->winmgr_entry_1);
                            ttyd::winmgr::winMgrOpen(menu->winmgr_entry_2);
                        } else {
                            // No items / badges to select.
                            ttyd::sound::SoundEfxPlayEx(0x266, 0, 0x64, 0x40);
                        }
                    }
                        
                    // Toggle on key items with boolean effects.
                    int32_t effect_flag = 0;
                    switch (item) {
                        case ItemType::TOT_KEY_PEEKABOO:
                            effect_flag = GSWF_PeekabooEnabled;
                            break;
                        case ItemType::TOT_KEY_SUPER_PEEKABOO:
                            effect_flag = GSWF_SuperPeekabooEnabled;
                            break;
                        case ItemType::TOT_KEY_TIMING_TUTOR:
                            effect_flag = GSWF_TimingTutorEnabled;
                            break;
                        case ItemType::TOT_KEY_BGM_TOGGLE:
                            effect_flag = GSWF_BgmEnabled;
                    }
                    if (effect_flag) {
                        if (ToggleSWF(effect_flag)) {
                            ttyd::pmario_sound::psndSFXOn((char *)0x20038);
                        } else {
                            // Also disable Stat Master if Peekaboo disabled. 
                            if (effect_flag == GSWF_PeekabooEnabled) {
                                SetSWF(GSWF_SuperPeekabooEnabled, 0);
                            }
                            // Fade out current BGM if disabling music.
                            if (effect_flag == GSWF_BgmEnabled) {
                                ttyd::pmario_sound::psndStopAllFadeOut();
                            }
                            ttyd::pmario_sound::psndSFXOn((char *)0x20039);
                        }
                    }
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
                return -2;
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
                g_Mod->state_.ChangeOption(STAT_PERM_ITEMS_USED);
                
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
        case 400:
        case 401:
        case 402: {
            int32_t type = menu->cosmetic_options[menu->cosmetic_cursor_idx];

            if (menu->buttons_pressed & ButtonId::START) {
                menu->use_item_type = 0;
                ttyd::winmgr::winMgrClose(menu->winmgr_entry_1);
                ttyd::winmgr::winMgrClose(menu->winmgr_entry_2);
                return -2;
            } else if (menu->buttons_pressed & ButtonId::B) {
                ttyd::pmario_sound::psndSFXOn((char *)0x20013);
                menu->item_menu_state = 10;
                ttyd::winmgr::winMgrClose(menu->winmgr_entry_1);
                ttyd::winmgr::winMgrClose(menu->winmgr_entry_2);
            } else if (menu->buttons_pressed & ButtonId::A) {
                if (CosmeticsManager::ToggleEquipped(
                    menu->item_menu_state - 400, type)) {
                    ttyd::pmario_sound::psndSFXOn((char *)0x20038);
                } else {
                    ttyd::pmario_sound::psndSFXOn((char *)0x20039);
                }
                // For Mario clothes specifically, reload models.
                if (menu->item_menu_state == 401) {
                    if (menu->mario_anim_pose_id != -1) {
                        ttyd::animdrv::animPoseRelease(menu->mario_anim_pose_id);
                        ttyd::mario::marioSetCharMode(0);
                        menu->mario_anim_pose_id =
                            ttyd::animdrv::animPoseEntry("a_mario", 0);
                        ttyd::animdrv::animPoseSetAnim(
                            menu->mario_anim_pose_id, "M_S_1", 1);
                        ttyd::animdrv::animPoseSetMaterialFlagOn(
                            menu->mario_anim_pose_id, 0x1800);
                    } else {
                        ttyd::mario::marioSetCharMode(0);
                    }
                }
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_UP) {
                if (--menu->cosmetic_cursor_idx < 0) {
                    menu->cosmetic_cursor_idx = menu->cosmetic_num_options - 1;
                    if (menu->cosmetic_num_options > 8) {
                        menu->cosmetic_menu_offset =
                            menu->cosmetic_num_options - 8;
                    }
                } else {
                    if (menu->cosmetic_menu_offset > 0 &&
                        menu->cosmetic_menu_offset == menu->cosmetic_cursor_idx)
                        --menu->cosmetic_menu_offset;
                }
                ttyd::pmario_sound::psndSFXOn((char *)0x20005);
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_DOWN) {
                if (++menu->cosmetic_cursor_idx >= menu->cosmetic_num_options) {
                    menu->cosmetic_cursor_idx = 0;
                    menu->cosmetic_menu_offset = 0;
                } else {
                    if (menu->cosmetic_menu_offset + 8 < menu->cosmetic_num_options &&
                        menu->cosmetic_menu_offset + 7 == menu->cosmetic_cursor_idx)
                        ++menu->cosmetic_menu_offset;
                }
                ttyd::pmario_sound::psndSFXOn((char *)0x20005);
            }

            // Update help text.
            switch (menu->item_menu_state) {
                case 400: {
                    const auto* data = CosmeticsManager::GetAttackFxData(type);
                    ttyd::win_root::winMsgEntry(menu, 0, data->help_msg, 0);
                    break;
                }
                case 401: {
                    const auto* data = CosmeticsManager::GetMarioCostumeData(type);
                    ttyd::win_root::winMsgEntry(menu, 0, data->help_msg, 0);
                    break;
                }
                case 402: {
                    const auto* data = CosmeticsManager::GetYoshiCostumeData(type);
                    ttyd::win_root::winMsgEntry(menu, 0, data->help_msg, 0);
                    break;
                }
            }
            
            // Set cursor position.
            menu->main_cursor_target_x = -60.0f;
            menu->main_cursor_target_y = 123.0f - 24.0f * 
                (menu->cosmetic_cursor_idx - menu->cosmetic_menu_offset);
            // Set window scroll offset.
            menu->cosmetic_menu_scroll_target_y = menu->cosmetic_menu_offset * 24.0f;
            
            break;
        }
        case 500:
        case 501: {
            int32_t item = g_SelectorItems[menu->cosmetic_cursor_idx];

            if (menu->buttons_pressed & ButtonId::START) {
                menu->use_item_type = 0;
                ttyd::winmgr::winMgrClose(menu->winmgr_entry_1);
                ttyd::winmgr::winMgrClose(menu->winmgr_entry_2);
                return -2;
            } else if (menu->buttons_pressed & ButtonId::B) {
                ttyd::pmario_sound::psndSFXOn((char *)0x20013);
                menu->item_menu_state = 10;
                ttyd::winmgr::winMgrClose(menu->winmgr_entry_1);
                ttyd::winmgr::winMgrClose(menu->winmgr_entry_2);
            } else if (menu->buttons_pressed & ButtonId::A) {
                SetCurrentItemLoadoutEquipped(menu, true);
            } else if (menu->buttons_pressed & ButtonId::X) {
                SetCurrentItemLoadoutEquipped(menu, false);
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_UP) {
                if (menu->cosmetic_cursor_idx >= 13) {
                    menu->cosmetic_cursor_idx -= 13;
                }
                ttyd::pmario_sound::psndSFXOn((char *)0x20005);
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_DOWN) {
                menu->cosmetic_cursor_idx += 13;
                if (menu->cosmetic_cursor_idx >= menu->cosmetic_num_options) {
                    menu->cosmetic_cursor_idx = menu->cosmetic_num_options - 1;
                }
                ttyd::pmario_sound::psndSFXOn((char *)0x20005);
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_LEFT) {
                if (menu->cosmetic_cursor_idx % 13 == 0) {
                    menu->cosmetic_cursor_idx += 12;
                } else {
                    --menu->cosmetic_cursor_idx;
                }
                if (menu->cosmetic_cursor_idx >= menu->cosmetic_num_options) {
                    menu->cosmetic_cursor_idx = menu->cosmetic_num_options - 1;
                }
                ttyd::pmario_sound::psndSFXOn((char *)0x20005);
            } else if (menu->dirs_repeated & DirectionInputId::ANALOG_RIGHT) {
                if (++menu->cosmetic_cursor_idx % 13 == 0) {
                    menu->cosmetic_cursor_idx -= 13;
                }
                if (menu->cosmetic_cursor_idx >= menu->cosmetic_num_options) {
                    menu->cosmetic_cursor_idx = 
                        (menu->cosmetic_num_options - 1) / 13 * 13;
                }
                ttyd::pmario_sound::psndSFXOn((char *)0x20005);
            }

            // Update help text.
            ttyd::win_root::winMsgEntry(
                menu, item, ttyd::item_data::itemDataTable[item].description, 0);
            
            // Set cursor position.
            menu->main_cursor_target_x =
                -92.0f + (menu->cosmetic_cursor_idx % 13 - 6) * 30.0f;
            menu->main_cursor_target_y =
                124.0f - (menu->cosmetic_cursor_idx / 13) * 30.0f;
            
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

    menu->cosmetic_menu_scroll_y +=
        (menu->cosmetic_menu_scroll_target_y - menu->cosmetic_menu_scroll_y) / 8.0f;
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
  
    if (ttyd::winmgr::winMgrAction(menu->winmgr_entry_1) == 0) {
        switch (menu->item_menu_state) {
            case 300:
            case 301:
                DrawUseItemDialog(winmgr_entry);
                break;
            case 400:
            case 401:
            case 402:
                DrawCosmeticSelectionDialog(winmgr_entry);
                break;
            case 500:
            case 501:
                DrawLoadoutSelectionDialog(winmgr_entry);
                break;
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
        const char* msg = "";
        switch (menu->item_menu_state) {
            case 300:
            case 301:
                // "Use it on whom?"
                msg = msgSearch("msg_window_select_5");
                break;
            case 400:
            case 402:
                msg = msgSearch("tot_winsel_whichones");
                break;
            case 401:
                msg = msgSearch("tot_winsel_whichduds");
                break;
            case 500:
            case 501:
                // Second window not used for loadout selector.
                return;
        }

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