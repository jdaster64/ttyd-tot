#include "tot_window_select.h"

#include "common_functions.h"
#include "mod.h"
#include "tot_generate_item.h"
#include "tot_generate_reward.h"
#include "tot_manager_move.h"
#include "tot_state.h"

#include <gc/types.h>
#include <gc/mtx.h>
#include <ttyd/dispdrv.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/fontmgr.h>
#include <ttyd/icondrv.h>
#include <ttyd/item_data.h>
#include <ttyd/mariost.h>
#include <ttyd/memory.h>
#include <ttyd/msgdrv.h>
#include <ttyd/statuswindow.h>
#include <ttyd/win_main.h>
#include <ttyd/winmgr.h>
#include <ttyd/gx/GXPixel.h>
#include <ttyd/gx/GXTransform.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::tot::window_select {

namespace {

// For convenience.
using namespace ::ttyd::winmgr;

using ::ttyd::dispdrv::CameraId;
using ::ttyd::evtmgr::EvtEntry;
using ::ttyd::evtmgr_cmd::evtSetValue;
using ::ttyd::item_data::itemDataTable;
using ::ttyd::mariost::g_MarioSt;
using ::ttyd::msgdrv::msgSearch;

namespace IconType = ::ttyd::icondrv::IconType;

struct OptionMenuData {
    uint32_t    option;
    const char* name_msg;
    uint16_t    lookup_key;
    bool        in_run_options;
    bool        in_run_stats;
};

OptionMenuData g_OptionMenuData[] = {
    { STAT_RUN_TURNS_SPENT, "tot_optr_turnsspent", 1, false, true },
    { STAT_RUN_MOST_TURNS_RECORD, "tot_optr_turnsmost", 2, false, true },
    { STAT_RUN_MOST_TURNS_FLOOR, "tot_optr_turnsmostfloor", 3, false, true },
    { STAT_RUN_TIMES_RAN_AWAY, "tot_optr_timesran", 4, false, true },
    { STAT_RUN_ENEMY_DAMAGE, "tot_optr_enemydamage", 5, false, true },
    { STAT_RUN_PLAYER_DAMAGE, "tot_optr_playerdamage", 6, false, true },
    { STAT_RUN_ITEMS_USED, "tot_optr_itemsused", 7, false, true },
    { STAT_RUN_SHINE_SPRITES, "tot_optr_shinesprites", 8, false, true },
    { STAT_RUN_STAR_PIECES, "tot_optr_starpieces", 9, false, true },
    { STAT_RUN_COINS_EARNED, "tot_optr_coinsearned", 10, false, true },
    { STAT_RUN_COINS_SPENT, "tot_optr_coinsspent", 11, false, true },
    { STAT_RUN_FP_SPENT, "tot_optr_fpspent", 12, false, true },
    { STAT_RUN_SP_SPENT, "tot_optr_spspent", 13, false, true },
    { STAT_RUN_SUPERGUARDS, "tot_optr_superguards", 14, false, true },
    { STAT_RUN_CONDITIONS_MET, "tot_optr_conditionsmet", 15, false, true },
    { STAT_RUN_CONDITIONS_TOTAL, "tot_optr_conditionstotal", 16, false, true },
};

const char* OptionName(uint16_t lookup_key) {
    for (const auto& data : g_OptionMenuData) {
        if (data.lookup_key == lookup_key) {
            return msgSearch(data.name_msg);
        }
    }
    // Option not found, return placeholder text.
    return "Option text missing!";
}

const char* OptionValue(uint16_t lookup_key) {
    static char buf[16];
    // Set to empty string by default.
    buf[0] = 0;
    for (const auto& data : g_OptionMenuData) {
        if (data.lookup_key == lookup_key) {
            int32_t val = g_Mod->state_.GetOption(data.option);
            IntegerToFmtString(val, buf);
            break;
        }
    }
    // TODO: Implement exceptions (named OPTVALs, etc.)
    return buf;
}

void SelectMainOptionsWrapper(WinMgrEntry* entry) {
    // Run vanilla selection main function.
    select_main(entry);
    // Force status window to be closed.
    ttyd::statuswindow::statusWinForceOff();
}

void DispMainWindow(WinMgrEntry* entry) {
    auto* sel_entry = (WinMgrSelectEntry*)entry->param;
    if ((winmgr_work->entries[sel_entry->entry_indices[0]].flags & 
        WinMgrEntry_Flags::IN_FADE) != 0) return;
    
    uint32_t kWhite = 0xFFFF'FFFFU;
    uint32_t kOffWhite = 0xF0F0'F0FFU;
    uint32_t kMedGrey = 0xA0A0'A0FFU;
    uint32_t kBlack = 0x0000'00FFU;
    
    ttyd::gx::GXPixel::GXSetFog(/* GX_FOG_NONE */ 0, 0, 0, 0, 0, &kWhite);
    // Save previous scissor bounds.
    int32_t scissor_0, scissor_1, scissor_2, scissor_3;
    ttyd::gx::GXTransform::GXGetScissor(
        &scissor_0, &scissor_1, &scissor_2, &scissor_3);
    ttyd::gx::GXTransform::GXSetScissor(
        entry->x + 304, -entry->y + 34 + 240, entry->width, entry->height - 50);
    
    int32_t offset = 0;
    for (int32_t i = 0; i < sel_entry->num_rows; ++i) {
        auto& row = sel_entry->row_data[i];
        float y_trans = sel_entry->list_y_offset + entry->y - 44 - offset;
        
        // Only draw info for rows that are visible.
        if (y_trans - 32 <= entry->y && 
            entry->y - entry->height <= y_trans + 32) {
            uint32_t* text_color;
            if (row.flags & WinMgrSelectEntryRow_Flags::GREYED_OUT) {
                ttyd::win_main::winIconGrayInit();
                text_color = &kMedGrey;
            } else {
                ttyd::win_main::winIconInit();
                text_color = &kBlack;
            }
            
            int32_t entry_icon = -1;
            const char* entry_text = "";
            float max_width = 185.0f;

            switch (sel_entry->type) {
                case MenuType::CUSTOM_START:
                case MenuType::TOT_CHARLIETON_SHOP: {
                    entry_icon = itemDataTable[row.value].icon_id;
                    entry_text = msgSearch(itemDataTable[row.value].name);
                    break;
                }
                case MenuType::MOVE_UNLOCK:
                case MenuType::MOVE_UPGRADE: {
                    auto* move_data = MoveManager::GetMoveData(row.value);
                    entry_icon = move_data->icon_id;
                    entry_text = msgSearch(move_data->name_msg);
                    if (sel_entry->type == MenuType::MOVE_UPGRADE) 
                        max_width -= 13.f;
                    break;
                }
                case MenuType::RUN_RESULTS_STATS: {
                    entry_text = OptionName(row.value);
                    max_width = 300.f;
                    break;
                }
            }
            
            float x_pos = entry->x + 30.0f;
            if (entry_icon >= 0) {
                x_pos += 5.0f;
                gc::vec3 pos = { x_pos, y_trans, 0.0f };
                gc::vec3 scale = { 0.5f, 0.5f, 0.5f };
                ttyd::win_main::winIconSet(entry_icon, &pos, &scale, &kWhite);
                x_pos += 25.0f;
            }
            
            ttyd::win_main::winFontInit();
            gc::vec3 text_pos = { x_pos, y_trans + 12.0f, 0.0f };
            gc::vec3 text_scale = { 1.0f, 1.0f, 1.0f };
            ttyd::win_main::winFontSetWidth(
                &text_pos, &text_scale, text_color, max_width, entry_text);
        }
        
        switch (sel_entry->type) {
            case MenuType::CUSTOM_START:
            case MenuType::TOT_CHARLIETON_SHOP: {
                // Draw "buy prices".
                int32_t value = itemDataTable[row.value].buy_price;
                // For Charlieton, scale based on tower progress.
                if (sel_entry->type == MenuType::TOT_CHARLIETON_SHOP) {
                    value = value * GetBuyPriceScale() / 100;
                }
                
                if (value > 0) {
                    char buf[8] = { 0 };
                    sprintf(buf, "%" PRId32 "", value);
                    int32_t length = ttyd::fontmgr::FontGetMessageWidth(buf);
                    if (length > 30) length = 30;
                    ttyd::win_main::winFontInit();
                    
                    uint32_t* text_color;
                    if (row.flags & WinMgrSelectEntryRow_Flags::GREYED_OUT) {
                        text_color = &kMedGrey;
                    } else {
                        text_color = &kBlack;
                    }
                    gc::vec3 text_pos = {
                        entry->x + entry->width - 10.0f - length,
                        y_trans + 12.0f,
                        0.0f
                    };
                    gc::vec3 text_scale = { 1.0f, 1.0f, 1.0f };
                    ttyd::win_main::winFontSetWidth(
                        &text_pos, &text_scale, text_color, 30.0, buf);
                }
                break;
            }
            case MenuType::MOVE_UPGRADE: {
                // Draw the level of the upgraded move.
                const char* lvl_string = 
                    MoveManager::GetUnlockedLevel(row.value) == 1 ? "2" : "3";
                
                uint32_t* text_color;
                if (row.flags & WinMgrSelectEntryRow_Flags::GREYED_OUT) {
                    text_color = &kMedGrey;
                } else {
                    text_color = &kBlack;
                }
                gc::vec3 text_pos = {
                    entry->x + entry->width - 25.0f,
                    y_trans + 12.0f,
                    0.0f
                };
                gc::vec3 text_scale = { 1.0f, 1.0f, 1.0f };
                ttyd::win_main::winFontInit();
                ttyd::win_main::winFontSetWidth(
                    &text_pos, &text_scale, text_color, 15.0, lvl_string);
                
                text_pos.x -= 27.0f;
                text_pos.y -= 10.0f;
                gc::vec3 lvl_text_scale = { 0.5f, 0.5f, 0.5f };
                ttyd::win_main::winFontInit();
                ttyd::win_main::winFontSetWidth(
                    &text_pos, &lvl_text_scale, text_color, 27.0, "Lvl.");
                
                break;
            }
            case MenuType::RUN_RESULTS_STATS: {
                // Draw the value / string representation of the stat.
                const char* stat_text = OptionValue(row.value);

                if (stat_text) {
                    ttyd::win_main::winFontInit();

                    int32_t length = 
                        ttyd::fontmgr::FontGetMessageWidth(stat_text);
                    float x_scale = 1.0f;
                    if (length > 140) {
                        x_scale = 140.0f / length;
                        length = 140;
                    }
                    
                    uint32_t* text_color;
                    if (row.flags & WinMgrSelectEntryRow_Flags::GREYED_OUT) {
                        text_color = &kMedGrey;
                    } else {
                        text_color = &kBlack;
                    }
                    gc::vec3 text_pos = {
                        entry->x + entry->width - 20.0f - length,
                        y_trans + 12.0f,
                        0.0f
                    };
                    gc::vec3 text_scale = { x_scale, 1.0f, 1.0f };
                    ttyd::win_main::winFontSet(
                        &text_pos, &text_scale, text_color, stat_text);
                }
                
                break;
            }
        }
        
        offset += 24;
    }
    
    // Restore previous scissor boundaries.
    ttyd::gx::GXTransform::GXSetScissor(
        scissor_0, scissor_1, scissor_2, scissor_3);
    
    const char* title = nullptr;
    switch (sel_entry->type) {
        case MenuType::CUSTOM_START:
            title = msgSearch("in_konran_hammer");
            break;
        case MenuType::TOT_CHARLIETON_SHOP:
            title = msgSearch("msg_window_title_1");  // "Items"
            break;
        case MenuType::MOVE_UNLOCK:
        case MenuType::MOVE_UPGRADE:
            // MOVE_UNLOCK, MOVE_UPGRADE
            title = msgSearch("tot_winsel_titlemove");
            break;
    }
    if (title) {
        int32_t length = ttyd::fontmgr::FontGetMessageWidth(title);
        if (length > 120) length = 120;
        ttyd::win_main::winFontInit();
        
        gc::vec3 text_pos = {
            entry->x + (entry->width - length) * 0.5f,
            entry->y + 14.0f,
            0.0f
        };
        gc::vec3 text_scale = { 1.0f, 1.0f, 1.0f };
        ttyd::win_main::winFontSetEdgeWidth(
            &text_pos, &text_scale, &kWhite, 120.0, title);
    }

    // Draw white circle + currency icon in upper-right corner.
    switch (sel_entry->type) {
        case MenuType::CUSTOM_START:
        case MenuType::TOT_CHARLIETON_SHOP:
            gc::mtx34 mtx, mtx2;
            gc::mtx::PSMTXScale(&mtx2, 0.6f, 0.6f, 0.6f);
            gc::mtx::PSMTXTrans(
                &mtx, entry->x + 268.0f, entry->y - 18.0f, 0.0f);
            gc::mtx::PSMTXConcat(&mtx, &mtx2, &mtx);
            ttyd::icondrv::iconDispGxCol(
                &mtx, 0x10, IconType::BLACK_WITH_WHITE_CIRCLE, &kOffWhite);

            gc::vec3 icon_pos = { entry->x + 268.0f, entry->y - 12.0f, 0.0f };
            ttyd::icondrv::iconDispGx(0.6f, &icon_pos, 0x10, IconType::COIN);
            break;
    }
    
    // Draw cursor and scrolling arrows, if necessary.
    gc::vec3 cursor_pos = { sel_entry->cursor_x, sel_entry->cursor_y, 1.0f };
    ttyd::icondrv::iconDispGx(1.0f, &cursor_pos, 0x14, IconType::GLOVE_POINTER_H);
    
    if (sel_entry->num_rows > 8 &&
        (g_MarioSt->currentRetraceCount & 0x1f) < 20) {
        if (sel_entry->list_row_offset != 0) {
            gc::vec3 pos = {
                entry->width * 0.5f +  entry->x,
                entry->y - 36.0f,
                1.0f
            };
            ttyd::icondrv::iconDispGx(
                0.6f, &pos, 0x10, IconType::MENU_UP_POINTER);
        }
        if (sel_entry->list_row_offset != sel_entry->num_rows - 8) {
            gc::vec3 pos = {
                entry->width * 0.5f +  entry->x,
                entry->y - entry->height - 12.0f,
                1.0f
            };
            ttyd::icondrv::iconDispGx(
                0.6f, &pos, 0x10, IconType::MENU_DOWN_POINTER);
        }
    }
}

void DispWindow2(WinMgrEntry* entry) {
    auto* sel_entry = (WinMgrSelectEntry*)entry->param;
    if ((winmgr_work->entries[sel_entry->entry_indices[1]].flags & 
        WinMgrEntry_Flags::IN_FADE) != 0) return;
    
    const char* msg = "";
    if (sel_entry->type == MenuType::CUSTOM_START) {
        msg = msgSearch("in_konran_hammer");
    } else if (sel_entry->type == MenuType::TOT_CHARLIETON_SHOP) {
        msg = msgSearch("msg_window_select_6");     // "Buy which one?"
    } else if (sel_entry->type == MenuType::MOVE_UNLOCK) {
        msg = msgSearch("tot_winsel_whichunlock");
    } else if (sel_entry->type == MenuType::MOVE_UPGRADE) {
        msg = msgSearch("tot_winsel_whichunlock");
    }
    
    uint16_t lines;
    int32_t length = ttyd::fontmgr::FontGetMessageWidthLine(msg, &lines);
    
    gc::mtx34 mtx, mtx2;
    if (length <= entry->width - 30) {
        gc::mtx::PSMTXScale(&mtx2, 1.0, 1.0, 1.0);
    } else {
        gc::mtx::PSMTXScale(&mtx2, (entry->width - 30.0) / length, 1.0, 1.0);
        length = entry->width - 30;
    }
    gc::mtx::PSMTXTrans(
        &mtx,
        entry->x + (entry->width - length) * 0.5,
        entry->y - (entry->height - (lines + 1) * 24) / 2,
        0.0);
    gc::mtx::PSMTXConcat(&mtx, &mtx2, &mtx);
    ttyd::fontmgr::FontDrawStart();
    ttyd::fontmgr::FontDrawMessageMtx(&mtx, msg);
    
    auto& lookup_entry = winmgr_work->entries[sel_entry->entry_indices[1]];
    if (lookup_entry.flags & WinMgrEntry_Flags::IN_USE) {
        lookup_entry.y = entry->desc->y + lines * 11;
        lookup_entry.height = entry->desc->height + lines * 11;
    }
}

void DispSelectionHelp(WinMgrEntry* entry) {
    auto* sel_entry = (WinMgrSelectEntry*)entry->param;
    if ((winmgr_work->entries[sel_entry->entry_indices[2]].flags &
        WinMgrEntry_Flags::IN_FADE) == 0) {
            
        int32_t value = sel_entry->row_data[sel_entry->cursor_index].value;
        
        const char* help_msg = "";
        if (sel_entry->type == MenuType::CUSTOM_START) {
            help_msg = msgSearch("in_konran_hammer");
        } else if (sel_entry->type == MenuType::TOT_CHARLIETON_SHOP) { 
            help_msg = msgSearch(itemDataTable[value].description);
        } else if (sel_entry->type == MenuType::MOVE_UNLOCK) {
            help_msg = msgSearch(
                MoveManager::GetMoveData(value)->desc_msg);
        } else if (sel_entry->type == MenuType::MOVE_UPGRADE) {
            help_msg = msgSearch(
                MoveManager::GetMoveData(value)->upgrade_msg);
        }
        entry->help_msg = help_msg;
        winMgrHelpDraw(entry);
    }
}

void DispOptionsWindowTopBar(WinMgrEntry* entry) {
    // TODO: Update to include seed, final timer, etc.

    auto* sel_entry = (WinMgrSelectEntry*)entry->param;
    if ((winmgr_work->entries[sel_entry->entry_indices[1]].flags & 
        WinMgrEntry_Flags::IN_FADE) != 0) return;
    
    const char* msg = msgSearch("tot_winsel_runstats_topbar");
    
    uint16_t lines;
    int32_t length = ttyd::fontmgr::FontGetMessageWidthLine(msg, &lines);
    
    gc::mtx34 mtx, mtx2;
    if (length <= entry->width - 30) {
        gc::mtx::PSMTXScale(&mtx2, 1.0, 1.0, 1.0);
    } else {
        gc::mtx::PSMTXScale(&mtx2, (entry->width - 30.0) / length, 1.0, 1.0);
        length = entry->width - 30;
    }
    gc::mtx::PSMTXTrans(
        &mtx,
        entry->x + (entry->width - length) * 0.5,
        entry->y - (entry->height - (lines + 1) * 24) / 2,
        0.0);
    gc::mtx::PSMTXConcat(&mtx, &mtx2, &mtx);
    ttyd::fontmgr::FontDrawStart();
    ttyd::fontmgr::FontDrawMessageMtx(&mtx, msg);
}

WinMgrSelectDescList g_SelectDescList[MenuType::MAX_MENU_TYPE];

WinMgrDesc g_CustomDescs[] = {
    // Descs 0-2: "shop"-like windows
    {
        .fade_mode = WinMgrDesc_FadeMode::SCALE_AND_ROTATE,
        .heading_type = WinMgrDesc_HeadingType::SINGLE_CENTERED,
        .camera_id = (int32_t)CameraId::k2d,
        .x = -24,
        .y = 118,
        .width = 300,
        .height = 240,
        .color = 0xFFFFFFFFU,
        .main_func = (void*)select_main,
        .disp_func = (void*)DispMainWindow,
    },
    {
        .fade_mode = WinMgrDesc_FadeMode::INSTANT,
        .heading_type = WinMgrDesc_HeadingType::NONE,
        .camera_id = (int32_t)CameraId::k2d,
        .x = -280,
        .y = 120,
        .width = 240,
        .height = 45,
        .color = 0xFFFFFFFFU,
        .main_func = nullptr,
        .disp_func = (void*)DispWindow2,
    },
    {
        .fade_mode = WinMgrDesc_FadeMode::SCALE_AND_ROTATE,
        .heading_type = WinMgrDesc_HeadingType::NONE,
        .camera_id = (int32_t)CameraId::k2d,
        .x = -240,
        .y = -130,
        .width = 500,
        .height = 80,
        .color = 0xFFFFFFFFU,
        .main_func = (void*)select_main3,
        .disp_func = (void*)DispSelectionHelp,
    },
    // Descs 3-4: "stats" / "options"-like windows. (WIP)
    {
        .fade_mode = WinMgrDesc_FadeMode::SCALE_AND_ROTATE,
        .heading_type = WinMgrDesc_HeadingType::NONE,
        .camera_id = (int32_t)CameraId::k2d,
        .x = -250,
        .y = 120,
        .width = 500,
        .height = 240,
        .color = 0xFFFFFFFFU,
        .main_func = (void*)SelectMainOptionsWrapper,
        .disp_func = (void*)DispMainWindow,
    },
    {
        .fade_mode = WinMgrDesc_FadeMode::INSTANT,
        .heading_type = WinMgrDesc_HeadingType::NONE,
        .camera_id = (int32_t)CameraId::k2d,
        .x = -200,
        .y = 180,
        .width = 400,
        .height = 45,
        .color = 0xC4ECF2FFU,
        .main_func = nullptr,
        .disp_func = (void*)DispOptionsWindowTopBar,
    },
};

}  // namespace

void* InitNewSelectDescTable() {
    // Copy descriptions from vanilla selection menus.
    memcpy(
        g_SelectDescList, ttyd::winmgr::select_desc_tbl,
        sizeof(ttyd::winmgr::select_desc_tbl));
    // Initialize custom menu parameters.
    g_SelectDescList[MenuType::CUSTOM_START] = WinMgrSelectDescList{ 
        .num_descs = 3, .descs = &g_CustomDescs[0] 
    };
    g_SelectDescList[MenuType::MOVE_UNLOCK] = WinMgrSelectDescList{ 
        .num_descs = 3, .descs = &g_CustomDescs[0] 
    };
    g_SelectDescList[MenuType::MOVE_UPGRADE] = WinMgrSelectDescList{ 
        .num_descs = 3, .descs = &g_CustomDescs[0] 
    };
    g_SelectDescList[MenuType::TOT_CHARLIETON_SHOP] = WinMgrSelectDescList{ 
        .num_descs = 3, .descs = &g_CustomDescs[0] 
    };
    g_SelectDescList[MenuType::RUN_RESULTS_STATS] = WinMgrSelectDescList{
        .num_descs = 2, .descs = &g_CustomDescs[3]
    };
    return g_SelectDescList;
}

WinMgrSelectEntry* HandleSelectWindowEntry(int32_t type, int32_t new_item) {
    auto* sel_entry = 
        (WinMgrSelectEntry*)ttyd::memory::__memAlloc(0, sizeof(WinMgrSelectEntry));
    memset(sel_entry, 0, sizeof(WinMgrSelectEntry));
        
    sel_entry->type = type;
    sel_entry->cursor_index = 0;
    sel_entry->list_row_offset = 0;
    sel_entry->cursor_x = 0;
    sel_entry->cursor_y = 0;
    sel_entry->list_y_offset = 0;
    sel_entry->new_item = new_item;
    
    // Determine whether window should be cancellable based on type.
    switch (type) {
        case MenuType::MOVE_UNLOCK:
        case MenuType::MOVE_UPGRADE:
            // Not cancellable.
            break;
        case MenuType::CUSTOM_START:
        case MenuType::TOT_CHARLIETON_SHOP:
        case MenuType::RUN_RESULTS_STATS:
        default:
            sel_entry->flags |= WinMgrSelectEntry_Flags::CANCELLABLE;
            break;
    }
    
    sel_entry->num_entries = g_SelectDescList[type].num_descs;
    
    for (int32_t i = 0; i < sel_entry->num_entries; ++i) {
        int32_t entry_idx = 0;
        for (; entry_idx < winmgr_work->num_entries; ++entry_idx) {
            if (!(winmgr_work->entries[entry_idx].flags & 
                WinMgrEntry_Flags::IN_USE))
                break;
        }
        auto& entry = winmgr_work->entries[entry_idx];
        entry.flags = WinMgrEntry_Flags::IN_USE;
        entry.fade_state = WinMgrEntry_FadeState::IDLE;
        entry.fade_frame_counter = 0;
        entry.desc = g_SelectDescList[type].descs + i;
        entry.x = entry.desc->x;
        entry.y = entry.desc->y;
        entry.width = entry.desc->width;
        entry.height = entry.desc->height;
        entry.priority = 0;
        entry.help_line_cursor_index = 0;
        entry.help_line_count = 0;
        
        sel_entry->entry_indices[i] = entry_idx;
        entry.param = sel_entry;
    }
    
    auto& main_desc = g_SelectDescList[type].descs[0];
    sel_entry->cursor_x = main_desc.x;
    sel_entry->cursor_y = main_desc.y - 54;

    switch (type) {
        case MenuType::CUSTOM_START: {
            // Dummy selection dialog for testing.
            sel_entry->num_rows = 10;
            sel_entry->row_data = 
                (WinMgrSelectEntryRow*)ttyd::memory::__memAlloc(
                    0, sel_entry->num_rows * sizeof(WinMgrSelectEntryRow));
            memset(
                sel_entry->row_data, 0, 
                sel_entry->num_rows * sizeof(WinMgrSelectEntryRow));
            sel_entry->row_data[0].value = 0xf0;
            sel_entry->row_data[1].value = 0xfb;
            sel_entry->row_data[2].value = 0xf0;
            sel_entry->row_data[3].value = 0xfb;
            sel_entry->row_data[4].value = 0xf0;
            sel_entry->row_data[5].value = 0xfb;
            sel_entry->row_data[6].value = 0xfb;
            sel_entry->row_data[7].value = 0xfb;
            sel_entry->row_data[8].flags = 3;
            sel_entry->row_data[8].value = 0xf7;
            sel_entry->row_data[9].value = 0xfb;
            break;
        }
        case MenuType::TOT_CHARLIETON_SHOP: {
            // Read inventory from tot_generate_item.
            int16_t* inventory = GetCharlietonInventoryPtr();
            int32_t num_items = 0;
            for (int16_t* ptr = inventory; *ptr != -1; ++ptr) ++num_items;
            sel_entry->num_rows = num_items;
            sel_entry->row_data = 
                (WinMgrSelectEntryRow*)ttyd::memory::__memAlloc(
                    0, sel_entry->num_rows * sizeof(WinMgrSelectEntryRow));
            memset(
                sel_entry->row_data, 0,
                sel_entry->num_rows * sizeof(WinMgrSelectEntryRow));
            for (int32_t i = 0; i < num_items; ++i) {
                sel_entry->row_data[i].value = inventory[i];
            }
            break;
        }
        case MenuType::MOVE_UNLOCK:
        case MenuType::MOVE_UPGRADE: {
            // Read moves from tot_generate_reward.
            int32_t num_moves;
            int32_t* moves = RewardManager::GetSelectedMoves(&num_moves);
            sel_entry->num_rows = num_moves;
            sel_entry->row_data =
                (WinMgrSelectEntryRow*)ttyd::memory::__memAlloc(
                    0, sel_entry->num_rows * sizeof(WinMgrSelectEntryRow));
            memset(
                sel_entry->row_data, 0,
                sel_entry->num_rows * sizeof(WinMgrSelectEntryRow));
            for (int32_t i = 0; i < num_moves; ++i) {
                sel_entry->row_data[i].value = moves[i];
            }
            break;
        }
        case MenuType::RUN_RESULTS_STATS: {
            // Assign options from g_OptionMenuData.
            int32_t num_options = 0;
            for (const auto& data : g_OptionMenuData) {
                if (data.in_run_stats) ++num_options;
            }
            sel_entry->num_rows = num_options;
            sel_entry->row_data =
                (WinMgrSelectEntryRow*)ttyd::memory::__memAlloc(
                    0, sel_entry->num_rows * sizeof(WinMgrSelectEntryRow));
            memset(
                sel_entry->row_data, 0,
                sel_entry->num_rows * sizeof(WinMgrSelectEntryRow));
            int32_t i = 0;
            for (const auto& data : g_OptionMenuData) {
                if (!data.in_run_stats) continue;
                sel_entry->row_data[i].value = data.lookup_key;
                ++i;
            }
            break;
        }
    }
    
    // Shrink the window if it's fewer than eight entries.
    if (sel_entry->num_rows < 8) {
        int32_t height_reduction = (8 - sel_entry->num_rows) * 24;
        auto& entry = winmgr_work->entries[sel_entry->entry_indices[0]];
        entry.height -= height_reduction;
    }
    
    return sel_entry;
}

int32_t HandleSelectWindowOther(WinMgrSelectEntry* sel_entry, EvtEntry* evt) {
    if (!(sel_entry->flags & WinMgrSelectEntry_Flags::FINISHED_SELECTION))
        return 0;
    if (sel_entry->flags & WinMgrSelectEntry_Flags::CANCELLED)
        return -1;
    
    int32_t value = sel_entry->row_data[sel_entry->cursor_index].value;

    switch(sel_entry->type) {
        case MenuType::CUSTOM_START:
            evt->lwData[1] = value;
            break;
        case MenuType::TOT_CHARLIETON_SHOP:
            evt->lwData[1] = value;
            evt->lwData[2] = PTR(msgSearch(itemDataTable[value].name));
            evt->lwData[3] = 
                itemDataTable[value].buy_price * GetBuyPriceScale() / 100;
            evt->lwData[4] = itemDataTable[value].bp_cost;
            break;
        case MenuType::MOVE_UNLOCK:
        case MenuType::MOVE_UPGRADE:
            evt->lwData[1] = value;
            evt->lwData[2] = 
                PTR(msgSearch(MoveManager::GetMoveData(value)->name_msg));
            break;
    }
    
    return 1;
}

}  // namespace mod::tot::window_select