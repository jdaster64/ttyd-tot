#include "tot_window_select.h"

#include "tot_generate_reward.h"
#include "tot_move_manager.h"

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

// TODO: All of this is testing, pretty much.
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
        entry->x + 304, 274 - entry->y, entry->width, entry->height - 50);
    
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
            
            int32_t entry_icon = 0;
            const char* entry_text = "";
            
            if (sel_entry->type == MenuType::CUSTOM_START) {
                entry_icon = itemDataTable[row.value].icon_id;
                entry_text = msgSearch(itemDataTable[row.value].name);
            } else {
                // MOVE_UNLOCK
                auto* move_data = MoveManager::GetMoveData(row.value);
                entry_icon = move_data->icon_id;
                entry_text = msgSearch(move_data->name_msg);
            }
            
            gc::vec3 pos = { entry->x + 35.0f, y_trans, 0.0f };
            gc::vec3 scale = { 0.5f, 0.5f, 0.5f };
            ttyd::win_main::winIconSet(entry_icon, &pos, &scale, &kWhite);
            
            ttyd::win_main::winFontInit();
            gc::vec3 text_pos = { entry->x + 60.0f, y_trans + 12.0f, 0.0f };
            gc::vec3 text_scale = { 1.0f, 1.0f, 1.0f };
            ttyd::win_main::winFontSetWidth(
                &text_pos, &text_scale, text_color, 185.0, entry_text);
        }
        
        if (sel_entry->type == MenuType::CUSTOM_START) {
            int32_t value = itemDataTable[row.value].buy_price;
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
        }
        
        offset += 24;
    }
    
    // Restore previous scissor boundaries.
    ttyd::gx::GXTransform::GXSetScissor(
        scissor_0, scissor_1, scissor_2, scissor_3);
    
    const char* title = "";
    if (sel_entry->type == MenuType::CUSTOM_START) {
        title = msgSearch("in_konran_hammer");
    } else if (sel_entry->type == MenuType::MOVE_UNLOCK) {
        title = msgSearch("tot_winsel_titlemove");
    }
    
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
        
    // Draw white circle + currency icon in upper-right corner.
    if (sel_entry->type == MenuType::CUSTOM_START) {
        gc::mtx34 mtx, mtx2;
        gc::mtx::PSMTXScale(&mtx2, 0.6f, 0.6f, 0.6f);
        gc::mtx::PSMTXTrans(&mtx, entry->x + 268.0f, entry->y - 18.0f, 0.0f);
        gc::mtx::PSMTXConcat(&mtx, &mtx2, &mtx);
        ttyd::icondrv::iconDispGxCol(
            &mtx, 0x10, IconType::BLACK_WITH_WHITE_CIRCLE, &kOffWhite);

        gc::vec3 icon_pos = { entry->x + 268.0f, entry->y - 12.0f, 0.0f };
        ttyd::icondrv::iconDispGx(0.6f, &icon_pos, 0x10, IconType::COIN);
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
    } else if (sel_entry->type == MenuType::MOVE_UNLOCK) {
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
        
        const char* help_msg = "";
        if (sel_entry->type == MenuType::CUSTOM_START) {
            help_msg = msgSearch("in_konran_hammer");
        } else if (sel_entry->type == MenuType::MOVE_UNLOCK) {
            help_msg = msgSearch(
                MoveManager::GetMoveData(
                    sel_entry->row_data[sel_entry->cursor_index].value
                )->desc_msg);
        }
        entry->help_msg = help_msg;
        winMgrHelpDraw(entry);
    }
}

WinMgrSelectDescList g_SelectDescList[MenuType::MAX_MENU_TYPE];

WinMgrDesc g_CustomDescs[] = {
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
            break;
        case MenuType::CUSTOM_START:
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
    
    if (type == MenuType::CUSTOM_START) {
        // CUSTOM_START (Dummy selection dialog for testing).
        sel_entry->num_rows = 10;
        sel_entry->row_data = (WinMgrSelectEntryRow*)ttyd::memory::__memAlloc(
            0, sel_entry->num_rows * sizeof(WinMgrSelectEntryRow));
        memset(sel_entry->row_data, 0, sel_entry->num_rows * sizeof(WinMgrSelectEntryRow));
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
    } else {
        // MOVE_UNLOCK
        int32_t num_moves;
        int32_t* moves = RewardManager::GetSelectedMoves(&num_moves);
        sel_entry->num_rows = num_moves;
        sel_entry->row_data = (WinMgrSelectEntryRow*)ttyd::memory::__memAlloc(
            0, sel_entry->num_rows * sizeof(WinMgrSelectEntryRow));
        memset(sel_entry->row_data, 0, sel_entry->num_rows * sizeof(WinMgrSelectEntryRow));
        for (int32_t i = 0; i < num_moves; ++i) {
            sel_entry->row_data[i].value = moves[i];
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
    if (sel_entry->type == MenuType::CUSTOM_START) {
        evt->lwData[1] = value;
    } else {
        // MOVE_UNLOCK - Assign move id + move name.
        evt->lwData[1] = value;
        evt->lwData[2] = PTR(msgSearch(MoveManager::GetMoveData(value)->name_msg));
    }
    
    return 1;
}

}  // namespace mod::tot::window_select