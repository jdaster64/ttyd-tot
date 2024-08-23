#include "tot_window_item.h"

#include "common_functions.h"
#include "common_types.h"
#include "mod.h"
#include "tot_state.h"

#include <gc/mtx.h>
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
#include <ttyd/mariost.h>
#include <ttyd/msgdrv.h>
#include <ttyd/pmario_sound.h>
#include <ttyd/win_main.h>
#include <ttyd/win_mario.h>
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
using ::ttyd::mariost::g_MarioSt;
using ::ttyd::msgdrv::msgSearch;
using ::ttyd::win_mario::linkDt;
using ::ttyd::win_root::WinPauseMenu;
using ::ttyd::winmgr::WinMgrEntry;

namespace IconType = ::ttyd::icondrv::IconType;
namespace ItemType = ::ttyd::item_data::ItemType;

}  // namespace

void MarioMenuInit(WinPauseMenu* menu) {
    if ((menu->flags & 0x2000) == 0) {
        menu->mario_anim_pose_id = ttyd::animdrv::animPoseEntry("a_mario", 0);
        ttyd::animdrv::animPoseSetAnim(menu->mario_anim_pose_id, "M_S_1", 1);
    } else {
        menu->mario_anim_pose_id = ttyd::animdrv::animPoseEntry("d_mario", 0);
        ttyd::animdrv::animPoseSetAnim(menu->mario_anim_pose_id, "S_1", 1);
    }
    ttyd::animdrv::animPoseSetMaterialFlagOn(menu->mario_anim_pose_id, 0x1800);
    if (ttyd::mario::marioGetPtr()->flags1 & 0x4000'0000) {
        ttyd::animdrv::animPoseSetMaterialFlagOn(menu->mario_anim_pose_id, 0xa000000);
    }
    menu->mario_anim_timer = 0;
    
    menu->mario_move_menu_opened = 0;
    menu->mario_move_cursor_idx = 0;
}

void MarioMenuInit2(WinPauseMenu* menu) {
    menu->mario_menu_state = 0;
    menu->main_cursor_target_x = linkDt[0].x_offset;
    menu->main_cursor_target_y = linkDt[0].y_offset;
    ttyd::win_root::winMsgEntry(menu, 0, linkDt[0].help_msg, 0);
  
    menu->mario_anim_timer = 0;
    menu->mario_move_menu_opened = 0;
    menu->mario_move_cursor_idx = 0;

    // Update help messages for reordered menu options.
    linkDt[13].help_msg = "msg_menu_mario_coin";
    linkDt[14].help_msg = "msg_menu_mario_kakera";
    linkDt[15].help_msg = "msg_menu_mario_scoin";
    linkDt[16].help_msg = "msg_menu_mario_star_p";

    // Update links between menu options to reflect inaccessibility.
    linkDt[0].neighbors[0] = 6;
    linkDt[0].neighbors[1] = 2;
    linkDt[0].neighbors[2] = 11;
    linkDt[0].neighbors[3] = 11;

    linkDt[2].neighbors[0] = 0;
    linkDt[2].neighbors[1] = 3;
    linkDt[2].neighbors[2] = 12;
    linkDt[2].neighbors[3] = 12;

    linkDt[3].neighbors[0] = 2;
    linkDt[3].neighbors[1] = 4;
    linkDt[3].neighbors[2] = 13;
    linkDt[3].neighbors[3] = 13;

    linkDt[4].neighbors[0] = 3;
    linkDt[4].neighbors[1] = 5;
    linkDt[4].neighbors[2] = 15;
    linkDt[4].neighbors[3] = 15;

    linkDt[5].neighbors[0] = 4;
    linkDt[5].neighbors[1] = 6;
    linkDt[5].neighbors[2] = 16;
    linkDt[5].neighbors[3] = 16;

    linkDt[6].neighbors[0] = 5;
    linkDt[6].neighbors[1] = 0;
    linkDt[6].neighbors[2] = 17;
    linkDt[6].neighbors[3] = 17;

    linkDt[11].neighbors[0] = 17;
    linkDt[11].neighbors[1] = 12;
    linkDt[11].neighbors[2] = 0;
    linkDt[11].neighbors[3] = 0;

    linkDt[12].neighbors[0] = 11;
    linkDt[12].neighbors[1] = 13;
    linkDt[12].neighbors[2] = 2;
    linkDt[12].neighbors[3] = 2;

    linkDt[13].neighbors[0] = 12;
    linkDt[13].neighbors[1] = 14;
    linkDt[13].neighbors[2] = 3;
    linkDt[13].neighbors[3] = 3;

    linkDt[14].neighbors[0] = 13;
    linkDt[14].neighbors[1] = 15;
    linkDt[14].neighbors[2] = 4;
    linkDt[14].neighbors[3] = 4;

    linkDt[15].neighbors[0] = 14;
    linkDt[15].neighbors[1] = 16;
    linkDt[15].neighbors[2] = 4;
    linkDt[15].neighbors[3] = 4;

    linkDt[16].neighbors[0] = 15;
    linkDt[16].neighbors[1] = 17;
    linkDt[16].neighbors[2] = 5;
    linkDt[16].neighbors[3] = 5;

    linkDt[17].neighbors[0] = 16;
    linkDt[17].neighbors[1] = 11;
    linkDt[17].neighbors[2] = 6;
    linkDt[17].neighbors[3] = 6;
}

int32_t MarioMenuMain(WinPauseMenu* menu) {
    const auto* pouch = ttyd::mario_pouch::pouchGetPtr();
    
    if (menu->mario_move_menu_opened) {
        if (menu->buttons_pressed & ButtonId::START) {
            return -2;
        } else if (menu->buttons_pressed & ButtonId::B) {
            menu->mario_move_menu_opened = 0;
            ttyd::pmario_sound::psndSFXOn((char *)0x20013);
        } else if (menu->dirs_repeated & DirectionInputId::ANALOG_UP) {
            if (--menu->mario_move_cursor_idx < 0) {
                menu->mario_move_cursor_idx = menu->mario_move_count - 1;
            }
            ttyd::pmario_sound::psndSFXOn((char *)0x20005);
        } else if (menu->dirs_repeated & DirectionInputId::ANALOG_DOWN) {
            if (++menu->mario_move_cursor_idx >= menu->mario_move_count) {
                menu->mario_move_cursor_idx = 0;
            }
            ttyd::pmario_sound::psndSFXOn((char *)0x20005);
        }
        
        menu->main_cursor_target_x = -280.0f;
        menu->main_cursor_target_y = 118.0f - 26.0f * menu->mario_move_cursor_idx;
        
        // Select appropriate description message for selected move.
        int32_t starting_move;
        switch (menu->mario_menu_state) {
            case 3:     starting_move = MoveType::JUMP_BASE;       break;
            case 2:     starting_move = MoveType::HAMMER_BASE;     break;
            default:    starting_move = MoveType::SP_SWEET_TREAT;  break;
        }
        int32_t current_pos = -1;
        for (int32_t i = 0; i < 8; ++i) {
            int32_t move = starting_move + i;
            if (MoveManager::GetUnlockedLevel(move) > 0) ++current_pos;
            if (current_pos == menu->mario_move_cursor_idx) {
                winMsgEntry(
                    menu, 0, MoveManager::GetMoveData(move)->desc_msg, 0);
                break;
            }
        }
            
        return 0;
    }
    
    if (menu->buttons_pressed & ButtonId::START) {
        return -2;
    } else if (menu->buttons_pressed & ButtonId::B) {
        ttyd::pmario_sound::psndSFXOn((char *)0x20013);
        const char* pose = "M_S_1";
        if (menu->flags & 0x2000) {
            pose = "S_1";
        }
        ttyd::animdrv::animPoseSetAnim(menu->mario_anim_pose_id, pose, 0);
        return -1;
    } else if (menu->buttons_pressed & ButtonId::A) {
        int32_t starting_move = -1;
        // Only allow opening move menus during a run.
        if (g_Mod->state_.GetOption(OPT_RUN_STARTED)) {
            switch (menu->mario_menu_state) {
                case 3:     starting_move = MoveType::JUMP_BASE;        break;
                case 2:     starting_move = MoveType::HAMMER_BASE;      break;
                case 12:    starting_move = MoveType::SP_SWEET_TREAT;   break;
            }
        }
        
        if (starting_move >= 0) {
            // Get max number of selectable moves.
            int32_t num_selections = 0;
            for (int32_t i = 0; i < 8; ++i) {
                if (tot::MoveManager::GetUnlockedLevel(starting_move + i) > 0) {
                    ++num_selections;
                }
            }
            menu->mario_move_menu_opened = 1;
            menu->mario_move_cursor_idx = 0;
            menu->mario_move_count = num_selections;
            ttyd::pmario_sound::psndSFXOn((char *)0x20012);
        }
    } else {
        int32_t dir = -1;
        if (menu->dirs_repeated & DirectionInputId::ANALOG_UP) {
            dir = 0;
        } else if (menu->dirs_repeated & DirectionInputId::ANALOG_DOWN) {
            dir = 1;
        } else if (menu->dirs_repeated & DirectionInputId::ANALOG_LEFT) {
            dir = 2;
        } else if (menu->dirs_repeated & DirectionInputId::ANALOG_RIGHT) {
            dir = 3;
        }
        if (dir >= 0) {
            int32_t next_state = menu->mario_menu_state;
            while (true) {
                next_state = linkDt[next_state].neighbors[dir];
                
                if (next_state == menu->mario_menu_state) {
                    // Reached initial state; exit to prevent infinite loop.
                    break;
                } else if (next_state == 1) {
                    // Rank information removed; never selectable.
                    continue;
                } else if (next_state == 11 || next_state == 12) {
                    // Star Power options selectable only if obtained.
                    if (pouch->star_powers_obtained)
                        break;
                } else {
                    // Boots, hammers and paper curses selectable if obtained.
                    int32_t item_id = linkDt[next_state].item_id;
                    if (!item_id || ttyd::mario_pouch::pouchCheckItem(item_id))
                        break;
                }
            }
            ttyd::pmario_sound::psndSFXOn((char *)0x20005);
            menu->mario_menu_state = next_state;
        }
    }
    
    menu->main_cursor_target_x = linkDt[menu->mario_menu_state].x_offset;
    menu->main_cursor_target_y = linkDt[menu->mario_menu_state].y_offset;
    
    if (menu->mario_menu_state == 2) {
        const char* pose = ttyd::win_mario::hammer_pose[pouch->hammer_level];
        if (menu->flags & 0x2000) {
            pose = "R_1";
        }
        ttyd::animdrv::animPoseSetAnim(menu->mario_anim_pose_id, pose, 0);
        if (menu->mario_anim_timer == 0) {
            if (ttyd::animdrv::animPoseGetLoopTimes(menu->mario_anim_pose_id) >= 0.89f) {
                menu->mario_anim_timer = 30;
            }
        } else {
            --menu->mario_anim_timer;
        }
        if (menu->flags & 0x2000) {
            menu->mario_anim_timer = 0;
        }
    } else {
        menu->mario_anim_timer = 0;
        const char* pose = linkDt[menu->mario_menu_state].pose_1;
        if (menu->flags & 0x2000) {
            if (!strcmp(linkDt[menu->mario_menu_state].pose_3, "S_1")) {
                pose = "S_1";
            } else {
                pose = "R_1";
            }
        }
        ttyd::animdrv::animPoseSetAnim(menu->mario_anim_pose_id, pose, 0);
    }
    
    switch (menu->mario_menu_state) {
        case 3:
            winMsgEntry(
                menu, 0, ttyd::win_mario::boots_help[pouch->jump_level], 0);
            break;
        case 2:
            winMsgEntry(
                menu, 0, ttyd::win_mario::hammer_help[pouch->hammer_level], 0);
            break;
        default:
            winMsgEntry(menu, 0, linkDt[menu->mario_menu_state].help_msg, 0);
            break;
    }
    return 0;
}

void MarioMenuDisp(CameraId camera_id, WinPauseMenu* menu, int32_t tab_number) {
    
    auto* pouch = ttyd::mario_pouch::pouchGetPtr();
    float win_x = menu->tab_body_info[tab_number].x;
    float win_y = menu->tab_body_info[tab_number].y;
    uint32_t kWhite = 0xFFFFFFFFU;
    uint32_t kBlack = 0x000000FFU;
    
    winBgGX(win_x, win_y, menu, 0);
    winNameGX(win_x - 265.0f, win_y + 154.0f, 240.0f, 24.0f, menu, 2);
    
    ttyd::win_main::winFontInit();
    {
        gc::vec3 position = { win_x - 255.0f, win_y + 152.0f, 0.0f };
        gc::vec3 scale = { 0.8f, 0.8f, 0.8f };
        ttyd::win_main::winFontSetEdge(
            &position, &scale, &kWhite, msgSearch("name_mario"));
    }
    {
        // Print filename centered, on right of Mario head icon.
        float mario_width =
            ttyd::fontmgr::FontGetMessageWidth(msgSearch("name_mario")) * 0.8f;
        float max_width = 180.f - mario_width;

        const char* text = g_MarioSt->saveFileName;
        float fn_width = ttyd::fontmgr::FontGetMessageWidth(text) * 0.8f;
        if (fn_width > max_width) fn_width = max_width;

        gc::vec3 position = {
            win_x - 255.0f + 40.0f + mario_width + (max_width - fn_width) * 0.5f,
            win_y + 152.0f,
            0.0f
        };
        gc::vec3 scale = { 0.8f, 0.8f, 0.8f };
        ttyd::win_main::winFontSetEdgeWidth(
            &position, &scale, &kWhite, max_width, text);
    }
    ttyd::win_main::winIconInit();
    {
        float mario_width =
            ttyd::fontmgr::FontGetMessageWidth(msgSearch("name_mario")) * 0.8f;
        gc::vec3 position = { 
            win_x - 255.0f + 20.0f + mario_width,
            win_y + 142.0f,
            0.0f
        };
        gc::vec3 scale = { 0.6f, 0.6f, 0.6f };
        ttyd::win_main::winIconSet(IconType::MARIO_HEAD, &position, &scale, &kWhite);
    }
    
    // Skip printing Mario's level + rank information.
    
    // Draw bubbles for Jump, Hammer, paper curses.
    const int32_t priority[] = { 3, 2, 8, 10, 7, 9 };
    for (int32_t i = 0; i < 6; ++i) {
        if (menu->mario_menu_state != priority[i]) {
            ttyd::win_mario::fukidashi(win_x, win_y, menu, priority[i]);
        }
    }
    ttyd::win_mario::fukidashi(win_x, win_y, menu, menu->mario_menu_state);
    
    winKirinukiGX(win_x - 270.0f, win_y - 10.0f, 264.0f, 110.0f, menu, 0);
    
    // HP, FP, BP descriptive text.
    for (int32_t i = 0; i < 3; ++i) {
        const char* stat_strings[] = { "HP", "FP", "BP" };
        const char* ruby_strings[] = {
            "msg_menu_mario_ruby_hp",
            "msg_menu_mario_ruby_fp",
            "msg_menu_mario_ruby_bp"
        };
        uint32_t colors[] = {
            0xA73C3CFFU, 0x37691EFFU, 0x0F217AFFU,  // "ruby" descriptors
            0xCB8CA2FFU, 0xCBBE89FFU, 0x94B9C7FFU,  // background spots
        };
        
        {
            ttyd::win_main::winTexInit(*menu->win_tpl->mpFileData);
            ttyd::gx::GXTev::GXSetTevColorIn(
                GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
            ttyd::gx::GXTev::GXSetTevAlphaIn(
                GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_TEXA, GX_CA_ZERO);
            
            gc::vec3 position = {
                win_x - 220.0f - 2.0f,
                win_y - 38.0f - (i * 34.0f) - 4.0f,
                0.0f
            };
            gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
            ttyd::win_main::winTexSet(0xaf, &position, &scale, &colors[i+3]);
        }
        {
            ttyd::win_main::winFontInit();
            int16_t width = ttyd::fontmgr::FontGetMessageWidth(stat_strings[i]);
            gc::vec3 position = {
                win_x - 220.0f - width * 0.5f,
                win_y - 38.0f - (i * 34.0f) + 12.0f,
                0.0f
            };
            gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
            ttyd::win_main::winFontSet(&position, &scale, &kBlack, stat_strings[i]);
        }
        {
            ttyd::win_main::winFontInit();
            const char* text = msgSearch(ruby_strings[i]);
            int16_t width = ttyd::fontmgr::FontGetMessageWidth(text);
            gc::vec3 position = {
                win_x - 220.0f - width * 0.5f * 0.5f,
                win_y - 38.0f - (i * 34.0f) + 21.0f,
                0.0f
            };
            gc::vec3 scale = { 0.5f, 0.5f, 0.5f };
            ttyd::win_main::winFontSet(&position, &scale, &colors[i], text);
        }
    }
    
    // HP, FP, BP icons.
    ttyd::win_main::winIconInit();
    {
        gc::vec3 position = { win_x - 270.0f + 110.0f, win_y - 38.0f + 4.0f, 0.0f };
        gc::vec3 scale = { 0.82f, 0.82f, 0.82f };
        ttyd::win_main::winIconSet(IconType::HP_ICON, &position, &scale, &kWhite);
    }
    {
        gc::vec3 position = { win_x - 270.0f + 110.0f, win_y - 72.0f + 4.0f, 0.0f };
        gc::vec3 scale = { 0.82f, 0.82f, 0.82f };
        ttyd::win_main::winIconSet(IconType::FP_ICON, &position, &scale, &kWhite);
    }
    {
        gc::vec3 position = { win_x - 270.0f + 110.0f, win_y - 106.0f + 4.0f, 0.0f };
        gc::vec3 scale = { 0.82f, 0.82f, 0.82f };
        ttyd::win_main::winIconSet(IconType::BP_ICON, &position, &scale, &kWhite);
    }
    
    // HP, FP, BP stats.
    ttyd::win_main::winFontInit();
    {
        gc::vec3 position = { win_x - 270.0f + 130.0f, win_y - 38.0f + 16.0f, 0.0f };
        gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
        ttyd::win_main::winFontSetR(&position, &scale, &kBlack, "%d", pouch->current_hp);
    }
    {
        gc::vec3 position = { win_x - 270.0f + 130.0f, win_y - 72.0f + 16.0f, 0.0f };
        gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
        ttyd::win_main::winFontSetR(&position, &scale, &kBlack, "%d", pouch->current_fp);
    }
    {
        gc::vec3 position = { win_x - 270.0f + 130.0f, win_y - 106.0f + 16.0f, 0.0f };
        gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
        ttyd::win_main::winFontSetR(&position, &scale, &kBlack, "%d", pouch->total_bp);
    }
    {
        gc::vec3 position = { win_x - 270.0f + 200.0f, win_y - 38.0f + 16.0f, 0.0f };
        gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
        ttyd::win_main::winFontSet(&position, &scale, &kBlack, "%d", pouch->max_hp);
    }
    {
        gc::vec3 position = { win_x - 270.0f + 200.0f, win_y - 72.0f + 16.0f, 0.0f };
        gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
        ttyd::win_main::winFontSet(&position, &scale, &kBlack, "%d", pouch->max_fp);
    }
    // HP, FP slashes.
    ttyd::win_main::winTexInit(*menu->win_tpl->mpFileData);
    {
        gc::vec3 position = { win_x - 270.0f + 190.0f, win_y - 38.0f + 4.0f, 0.0f };
        gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
        ttyd::win_main::winTexSet(0x10, &position, &scale, &kWhite);
    }
    {
        gc::vec3 position = { win_x - 270.0f + 190.0f, win_y - 72.0f + 4.0f, 0.0f };
        gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
        ttyd::win_main::winTexSet(0x10, &position, &scale, &kWhite);
    }
    
    // Star Power / Special moves window; shown only during runs.
    if (pouch->star_powers_obtained) {
        winKirinukiGX(win_x + 10.0f, win_y + 130.0f, 260.0f, 80.0f, menu, 0);
        
        ttyd::win_main::winIconInit();
        {
            gc::vec3 position = { win_x + 50.0f, win_y + 106.0f, 0.0f };
            gc::vec3 scale = { 0.8f, 0.8f, 0.8f };
            ttyd::win_main::winIconSet(
                IconType::STAR, &position, &scale, &kWhite);
        }
        {
            gc::vec3 position = { win_x + 50.0f, win_y + 70.0f, 0.0f };
            gc::vec3 scale = { 0.8f, 0.8f, 0.8f };
            ttyd::win_main::winIconSet(
                IconType::DIAMOND_STAR, &position, &scale, &kWhite);
        }
        for (int32_t i = 0; i < pouch->max_sp / 100; ++i) {
            // Show maximum SP rather than current, including ninth dot.
            gc::vec3 position = {
                win_x + 82.0f + i * 20.0f,
                win_y + 106.0f,
                0.0f
            };
            gc::vec3 scale = { 0.8f, 0.8f, 0.8f };
            ttyd::win_main::winIconSet(
                IconType::SP_ORB_YELLOW + i % 8, &position, &scale, &kWhite);
        }
        
        ttyd::win_main::winFontInit();
        const char* text = msgSearch("msg_menu_mario_special");
        gc::vec3 position = { win_x + 80.0f, win_y + 80.0f, 0.0f };
        gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
        ttyd::win_main::winFontSetWidth(&position, &scale, &kBlack, 175.0f, text);
    }
    
    // Bottom-right stats section.
    // TODO: Replace Star Points with completion percentage.
    // TODO: Remove Shine Sprites when not in the middle of a run?
    
    winKirinukiGX(win_x + 10.0f, win_y + 30.0f, 260.0f, 150.0f, menu, 0);
    
    ttyd::win_main::winIconInit();
    for (int32_t i = 0; i < 5; ++i) {
        int32_t icons[] = {
            IconType::COIN, IconType::STAR_PIECE, IconType::SHINE_SPRITE, 
            IconType::STAR_POINT, IconType::PLAY_TIME_ICON
        };
        gc::vec3 position = { win_x + 30.0f, win_y + 10.0f - 28.0f * i, 0.0f };
        gc::vec3 scale = { 0.62f, 0.62f, 0.62f };
        ttyd::win_main::winIconSet(icons[i], &position, &scale, &kWhite);
    }
    
    ttyd::win_main::winFontInit();
    for (int32_t i = 0; i < 5; ++i) {
        const char* text[] = {
            "msg_menu_mario_num_coin",
            "msg_menu_mario_num_hoshi",
            "msg_menu_mario_num_super_coin",
            "msg_menu_mario_num_sp",
            "msg_menu_mario_num_playtime"
        };
        gc::vec3 position = { win_x + 45.0f, win_y + 22.0f - 28.0f * i, 0.0f };
        gc::vec3 scale = { 0.9f, 0.9f, 0.9f };
        ttyd::win_main::winFontSetWidth(
            &position, &scale, &kBlack, 140.0f, msgSearch(text[i]));
    }
    
    const int32_t stat_amounts[4] = {
        pouch->coins,
        pouch->star_pieces,
        pouch->shine_sprites,
        pouch->star_points
    };
    for (int32_t i = 0; i < 4; ++i) {
        gc::vec3 position = { win_x + 215.0f, win_y + 22.0f - 28.0f * i, 0.0f };
        gc::vec3 scale = { 0.9f, 0.9f, 0.9f };
        ttyd::win_main::winFontSetR(&position, &scale, &kBlack, "%d", stat_amounts[i]);
    }
    
    int32_t h, m, s, cs;
    if (DurationTicksToCentiseconds(g_MarioSt->lastFrameRetraceLocalTime) < 
        100 * 60 * 60 * 100) {
        DurationTicksToParts(g_MarioSt->lastFrameRetraceLocalTime, &h, &m, &s, &cs);
    } else {
        h = 99;
        m = 59;
    }
    {
        gc::vec3 position = { win_x + 168.0f, win_y + 22.0f - 112.0f, 0.0f };
        gc::vec3 scale = { 0.9f, 0.9f, 0.9f };
        ttyd::win_main::winFontSetR(&position, &scale, &kBlack, "%02d", h);
    }
    {
        gc::vec3 position = { win_x + 215.0f, win_y + 22.0f - 112.0f, 0.0f };
        gc::vec3 scale = { 0.9f, 0.9f, 0.9f };
        ttyd::win_main::winFontSetR(&position, &scale, &kBlack, "%02d", m);
    }
    
    ttyd::win_main::winTexInit(*menu->win_tpl->mpFileData);
    for (int32_t i = 0; i < 4; ++i) {
        gc::vec3 position = { win_x + 180.0f, win_y + 10.0f - 28.0f * i, 0.0f };
        gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
        ttyd::win_main::winTexSet(0x11, &position, &scale, &kWhite);
    }
    {
        gc::vec3 position = { win_x + 220.0f, win_y + 10.0f - 112.0f, 0.0f };
        gc::vec3 scale = { 1.0f, 1.0f, 1.0f };
        ttyd::win_main::winTexSet(0x12, &position, &scale, &kWhite);
    }
    
    // Mario animation.
    if (menu->mario_anim_timer == 0) {
        void* data_ptr = ttyd::animdrv::animPoseGetAnimDataPtr(menu->mario_anim_pose_id);
        int32_t frames = *(float*)(*(uintptr_t*)((uintptr_t)data_ptr + 0x24) + 8);
        int32_t frame_no = g_MarioSt->currentRetraceCount % frames;
        ttyd::animdrv::animPoseSetLocalTime(frame_no, menu->mario_anim_pose_id);
    }
    ttyd::animdrv::animPoseMain(menu->mario_anim_pose_id);
    uint32_t shadow_color = 0x00000080U;
    winKageGX(win_x - 150.0f, win_y, 0.0f, 1.0f, menu, &shadow_color);
    ttyd::win_root::winZClear();
    gc::mtx34 mtx;
    gc::mtx::PSMTXTrans(&mtx, win_x - 150.0f, win_y, 0.0f);
    ttyd::animdrv::animPoseDrawMtx(menu->mario_anim_pose_id, &mtx, 2, 0.0f, 5.0f);
    
    // Moves submenu.
    if (menu->mario_move_menu_opened) {
        winWazaGX(win_x - 270.0f, 160.0f, 270.0f, 250.0f, menu, 0);
        
        ttyd::win_main::winFontInit();
        {
            const char* text = msgSearch("msg_menu_mario_sac_waza");
            int16_t width = ttyd::fontmgr::FontGetMessageWidth(text);
            gc::vec3 position = { 
                win_x - 270.0f + (240.0f - width) / 2,
                155.0f,
                0.0f
            };
            gc::vec3 scale = { 0.8f, 0.8f, 0.8f };
            ttyd::win_main::winFontSetEdge(&position, &scale, &kWhite, text);
        }
        {
            gc::vec3 position = { win_x - 50.0f, 155.0f, 0.0f };
            gc::vec3 scale = { 0.8f, 0.8f, 0.8f };
            ttyd::win_main::winFontSetEdge(&position, &scale, &kWhite, "Lvl.");
        }
            
        int32_t starting_move;
        switch (menu->mario_menu_state) {
            case 3:     starting_move = MoveType::JUMP_BASE;        break;
            case 2:     starting_move = MoveType::HAMMER_BASE;      break;
            default:    starting_move = MoveType::SP_SWEET_TREAT;   break;
        }
        
        gc::vec3 move_name_position     = { -250.0f, 130.0f, 0.0f };
        gc::vec3 move_max_lvl_position  = { -22.0f, 130.0f, 0.0f };
        gc::vec3 move_text_scale        = { 1.0f, 1.0f, 1.0f };
        gc::vec3 move_lvl_icon_position = { -55.0f, 120.0f, 0.0f };
        gc::vec3 move_lvl_icon_scale    = { 0.5f, 0.5f, 0.5f };
        char level_str_buf[3] = { 0 };
        
        for (int32_t i = 0; i < 8; ++i) {
            int32_t move = starting_move + i;
            // Get max level of attack; skip if not unlocked.
            const int32_t max_level = MoveManager::GetUnlockedLevel(move);
            if (max_level < 1) continue;
            
            // Print attack name.
            const char* name = msgSearch(MoveManager::GetMoveData(move)->name_msg);
            ttyd::win_main::winFontSetWidth(
                &move_name_position, &move_text_scale, &kBlack, 180.0f, name);
                
            // Print "Lvl." string + max level.
            ttyd::win_main::winFontSet(
                &move_lvl_icon_position, &move_lvl_icon_scale, &kBlack, "Lvl.");
            sprintf(level_str_buf, "%" PRId32, max_level);
            int16_t width = ttyd::fontmgr::FontGetMessageWidth(level_str_buf);
            move_max_lvl_position.x = -21.0f - width / 2.0f;
            ttyd::win_main::winFontSet(
                &move_max_lvl_position, &move_text_scale, &kBlack, level_str_buf);
            
            // Move to next row of table.
            move_name_position.y -= 26.0f;
            move_max_lvl_position.y -= 26.0f;
            move_lvl_icon_position.y -= 26.0f;
        }
    }
}

}  // namespace mod::tot::win