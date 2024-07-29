#include "patches_ui.h"

#include "common_functions.h"
#include "common_types.h"
#include "mod.h"
#include "patch.h"
#include "patches_item.h"
#include "tot_generate_enemy.h"
#include "tot_manager_move.h"
#include "tot_state.h"
#include "tot_window_item.h"
#include "tot_window_log.h"
#include "tot_window_select.h"

#include <gc/mtx.h>
#include <gc/types.h>
#include <ttyd/gx/GXTev.h>
#include <ttyd/gx/GXAttr.h>
#include <ttyd/gx/GXTexture.h>
#include <ttyd/gx/GXTransform.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_mario.h>
#include <ttyd/battle_monosiri.h>
#include <ttyd/battle_seq_end.h>
#include <ttyd/dispdrv.h>
#include <ttyd/eff_updown.h>
#include <ttyd/evtmgr.h>
#include <ttyd/filemgr.h>
#include <ttyd/fontmgr.h>
#include <ttyd/icondrv.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_party.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/msgdrv.h>
#include <ttyd/sound.h>
#include <ttyd/statuswindow.h>
#include <ttyd/win_item.h>
#include <ttyd/win_log.h>
#include <ttyd/win_main.h>
#include <ttyd/win_mario.h>
#include <ttyd/win_party.h>
#include <ttyd/win_root.h>
#include <ttyd/winmgr.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

// Assembly patch functions.
extern "C" {
    // eff_updown_disp_patches.s
    void StartDispUpdownNumberIcons();
    void BranchBackDispUpdownNumberIcons();
    // menu_patches.s
    void StartHakoGxInitializeFields();
    void BranchBackHakoGxInitializeFields();
    void StartHakoGxCheckDrawNoItemBox();
    void ReturnHakoGxCheckDrawNoItemBoxNoItemCase();
    void ReturnHakoGxCheckDrawNoItemBoxItemCase();
    void StartHakoGxCheckDrawItemIcon();
    void ReturnHakoGxCheckDrawItemIconNoItemCase();
    void ReturnHakoGxCheckDrawItemIconItemCase();

    void StartCheckOpenMarioMoveMenu();
    void BranchBackCheckOpenMarioMoveMenu();
    void StartMarioMoveMenuDisp();
    void BranchBackMarioMoveMenuDisp();
    void StartMarioMoveMenuMsgEntry();
    void BranchBackMarioMoveMenuMsgEntry();
    
    void StartPartySetPartnerDescAndMoveCount();
    void BranchBackPartySetPartnerDescAndMoveCount();
    void StartPartyOverrideMoveTextAndCursorPos();
    void BranchBackPartyOverrideMoveTextAndCursorPos();
    void StartPartyDispHook1();
    void BranchBackPartyDispHook1();
    void StartPartyDispHook2();
    void BranchBackPartyDispHook2();
    // status_window_patches.s
    void StartPreventDpadShortcutsOutsidePit();
    void ConditionalBranchPreventDpadShortcutsOutsidePit();
    void BranchBackPreventDpadShortcutsOutsidePit();
    void StartHideTopBarInSomeWindows();
    void BranchBackHideTopBarInSomeWindows();
  
    void dispUpdownNumberIcons(
        int32_t number, void* tex_obj, gc::mtx34* icon_mtx, gc::mtx34* view_mtx,
        uint32_t unk0) {
        mod::infinite_pit::ui::DisplayUpDownNumberIcons(
            number, tex_obj, icon_mtx, view_mtx, unk0);
    }
    bool checkOutsidePit() {
        return strcmp("jon", mod::GetCurrentArea()) != 0;
    }
    bool checkOpenMarioMoveMenu(ttyd::win_root::WinPauseMenu* menu) {
        return mod::infinite_pit::ui::CheckOpenMarioMoveMenu(menu);
    }
    void marioMoveMenuDisp(ttyd::win_root::WinPauseMenu* menu) {
        mod::infinite_pit::ui::MarioMoveMenuDisp(menu);
    }
    void marioMoveMenuMsgEntry(ttyd::win_root::WinPauseMenu* menu) {
        mod::infinite_pit::ui::MarioMoveMenuMsgEntry(menu);
    }
    void partyMenuSetupPartnerDescAndMoveCount(ttyd::win_root::WinPauseMenu* menu) {
        mod::infinite_pit::ui::PartyMenuSetupPartnerDescAndMoveCount(menu);
    }
    void partyMenuSetMoveDescAndCursorPos(ttyd::win_root::WinPauseMenu* menu) {
        mod::infinite_pit::ui::PartyMenuSetMoveDescAndCursorPos(menu);
    }
    void partyMenuDispStats(ttyd::win_root::WinPauseMenu* menu) {
        mod::infinite_pit::ui::PartyMenuDispStats(menu);
    }
    int32_t getIconForBadgeOrItemLogEntry(
        ttyd::win_root::WinPauseMenu* menu, bool item_log, int32_t index) {
        return mod::infinite_pit::ui::GetIconForBadgeOrItemLogEntry(
            menu, item_log, index);
    }

    bool checkHideTopBarInWindow(ttyd::winmgr::WinMgrSelectEntry* sel_entry) {
        switch (sel_entry->type) {
            // Suppress forcing the top bar open for certain windows only.
            case mod::tot::window_select::MenuType::RUN_OPTIONS:
            case mod::tot::window_select::MenuType::RUN_RESULTS_SPLITS:
            case mod::tot::window_select::MenuType::RUN_RESULTS_STATS:
                return true;
        }
        return false;
    }
}

namespace mod::infinite_pit {

namespace {

// For convenience.
using namespace ::ttyd::gx::GXTev;
    
using ::ttyd::battle_database_common::BattleWeapon;
using ::ttyd::dispdrv::CameraId;
using ::ttyd::evtmgr::EvtEntry;
using ::ttyd::mario_pouch::PouchData;
using ::ttyd::msgdrv::msgSearch;
using ::ttyd::win_party::WinPartyData;
using ::ttyd::win_root::WinPauseMenu;
using ::ttyd::winmgr::WinMgrEntry;
using ::ttyd::winmgr::WinMgrSelectEntry;

namespace BattleUnitType = ::ttyd::battle_database_common::BattleUnitType;
namespace IconType = ::ttyd::icondrv::IconType;
namespace ItemType = ::ttyd::item_data::ItemType;

}

// Function hooks.
extern void (*g_statusWinDisp_trampoline)(void);
extern void (*g_gaugeDisp_trampoline)(double, double, int32_t);
extern void (*g_itemUseDisp2_trampoline)(WinMgrEntry*);
extern void (*g_itemUseDisp_trampoline)(WinMgrEntry*);
extern void (*g_winItemDisp_trampoline)(CameraId, WinPauseMenu*, int32_t);
extern void (*g_winItemMain2_trampoline)(WinPauseMenu*);
extern int32_t (*g_winItemMain_trampoline)(WinPauseMenu*);
extern void (*g_winLogDisp_trampoline)(CameraId, WinPauseMenu*, int32_t);
extern void (*g_winLogMain2_trampoline)(WinPauseMenu*);
extern int32_t (*g_winLogMain_trampoline)(WinPauseMenu*);
extern void (*g_winLogExit_trampoline)(WinPauseMenu*);
extern void (*g_winLogInit2_trampoline)(WinPauseMenu*);
extern void (*g_winLogInit_trampoline)(WinPauseMenu*);
extern const char* (*g_BattleGetRankNameLabel_trampoline)(int32_t);
extern int32_t (*g_winMgrSelectOther_trampoline)(WinMgrSelectEntry*, EvtEntry*);
extern WinMgrSelectEntry* (*g_winMgrSelectEntry_trampoline)(int32_t, int32_t, int32_t);
// Patch addresses.
extern const int32_t g_effUpdownDisp_TwoDigitSupport_BH;
extern const int32_t g_effUpdownDisp_TwoDigitSupport_EH;
extern const int32_t g_statusWinDisp_HideDpadMenuOutsidePit_BH;
extern const int32_t g_statusWinDisp_HideDpadMenuOutsidePit_EH;
extern const int32_t g_statusWinDisp_HideDpadMenuOutsidePit_CH1;
extern const int32_t g_winHakoGX_SetInitialFields_BH;
extern const int32_t g_winHakoGX_SetInitialFields_EH;
extern const int32_t g_winHakoGX_Patch_SkipSingleBox;
extern const int32_t g_winHakoGX_CheckDrawNoItemBox_BH;
extern const int32_t g_winHakoGX_CheckDrawNoItemBox_EH;
extern const int32_t g_winHakoGX_CheckDrawNoItemBox_CH1;
extern const int32_t g_winHakoGX_CheckDrawItemIcon_BH;
extern const int32_t g_winHakoGX_CheckDrawItemIcon_EH;
extern const int32_t g_winHakoGX_CheckDrawItemIcon_CH1;
extern const int32_t g_winPartyDisp_StatsHook1_BH;
extern const int32_t g_winPartyDisp_StatsHook1_EH;
extern const int32_t g_winPartyDisp_StatsHook2_BH;
extern const int32_t g_winPartyDisp_StatsHook2_EH;
extern const int32_t g_winPartyMain_RotatePartnersHook_BH;
extern const int32_t g_winPartyMain_RotatePartnersHook_EH;
extern const int32_t g_winPartyMain_OverrideMoveTextCursor_BH;
extern const int32_t g_winPartyMain_OverrideMoveTextCursor_EH;
extern const int32_t g_itemUseDisp_FixPartyOrder_BH;
extern const int32_t g_itemUseDisp_FixPartyOrder_EH;
extern const int32_t g_winItemDisp_DispInventorySize_BH;
extern const int32_t g_winItemMain_FixPartyOrder_BH;
extern const int32_t g_winItemMain_FixPartyOrder_EH;
extern const int32_t g_winMarioDisp_MoveMenuDisp_BH;
extern const int32_t g_winMarioDisp_MoveMenuDisp_EH;
extern const int32_t g_winMarioMain_MoveDescription_BH;
extern const int32_t g_winMarioMain_MoveDescription_EH;
extern const int32_t g_winMarioMain_CheckOpenMoveMenu_BH;
extern const int32_t g_select_main_CheckHideTopBar_BH;
extern const int32_t g_winMgrSelectEntry_Patch_SelectDescTblHi16;
extern const int32_t g_winMgrSelectEntry_Patch_SelectDescTblLo16;
extern const int32_t g__btlcmd_MakeSelectWeaponTable_Patch_GetNameFromItem;

namespace ui {
    
namespace {

// Displays the Star Power in 0.01 units numerically below the status window.
void DisplayStarPowerNumber() {
    // Don't display SP if no Star Powers have been unlocked yet.
    if (ttyd::mario_pouch::pouchGetMaxAP() <= 0) return;
    
    // Don't try to display SP if the status bar is not on-screen.
    float menu_height = *reinterpret_cast<float*>(
        reinterpret_cast<uintptr_t>(ttyd::statuswindow::g_StatusWindowWork)
        + 0x24);
    if (menu_height < 100.f || menu_height > 330.f) return;
    
    gc::mtx34 matrix;
    uint32_t color = ~0U;
    int32_t current_AP = ttyd::mario_pouch::pouchGetAP();
    gc::mtx::PSMTXTrans(&matrix, 192.f, menu_height - 100.f, 0.f);
    ttyd::icondrv::iconNumberDispGx(
        &matrix, current_AP, /* is_small = */ 1, &color);
}

// Display the orbs representing the Star Power (replaces the vanilla logic
// since it wasn't built around receiving Star Powers out of order).
void DisplayStarPowerOrbs(double x, double y, int32_t star_power) {
    int32_t max_star_power = ttyd::mario_pouch::pouchGetMaxAP();
    if (max_star_power > 800) max_star_power = 800;
    if (star_power > max_star_power) star_power = max_star_power;
    if (star_power < 0) star_power = 0;
    
    int32_t full_orbs = star_power / 100;
    int32_t remainder = star_power % 100;
    int32_t part_frame = remainder * 15 / 99;
    if (remainder > 0 && star_power > 0 && part_frame == 0) part_frame = 1;
    
    if (part_frame != 0) {
        gc::vec3 pos = { 
            static_cast<float>(x + 32.f * full_orbs),
            static_cast<float>(y),
            0.f };
        ttyd::icondrv::iconDispGx(
            1.f, &pos, 0x10, ttyd::statuswindow::gauge_wakka[part_frame]);
    }
    // Draw grey orbs up to the max amount of SP / 100 (rounded up, max of 8).
    for (int32_t i = 0; i < (max_star_power + 99) / 100; ++i) {
        gc::vec3 pos = {
            static_cast<float>(x + 32.f * i), 
            static_cast<float>(y + 12.f),
            0.f };
        uint16_t icon = i < full_orbs ?
            ttyd::statuswindow::gauge_back[i] : IconType::SP_ORB_EMPTY;
        ttyd::icondrv::iconDispGx(1.f, &pos, 0x10, icon);
    }
}

}
    
void ApplyFixedPatches() {
    g_statusWinDisp_trampoline = patch::hookFunction(
        ttyd::statuswindow::statusWinDisp, []() {
            g_statusWinDisp_trampoline();
            DisplayStarPowerNumber();
        });
        
    g_gaugeDisp_trampoline = patch::hookFunction(
        ttyd::statuswindow::gaugeDisp, [](double x, double y, int32_t sp) {
            // Replaces the original logic completely.
            DisplayStarPowerOrbs(x, y, sp);
        });
        
    // Fix rank string shown in the menu.
    g_BattleGetRankNameLabel_trampoline = patch::hookFunction(
        ttyd::battle_seq_end::BattleGetRankNameLabel,
        [](int32_t level) {
            int32_t rank = ttyd::mario_pouch::pouchGetPtr()->rank;
            if (rank < 0 || rank > 3) rank = 0;
            return ttyd::battle_seq_end::_rank_up_data[rank].mario_menu_msg;
        });
        
    // Apply patch to effUpdownDisp code to display the correct number
    // when Charging / +ATK/DEF-ing by more than 9 points.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_effUpdownDisp_TwoDigitSupport_BH),
        reinterpret_cast<void*>(g_effUpdownDisp_TwoDigitSupport_EH),
        reinterpret_cast<void*>(StartDispUpdownNumberIcons),
        reinterpret_cast<void*>(BranchBackDispUpdownNumberIcons));
        
    // Apply patches to statusWinDisp to prevent D-Pad shortcuts from appearing
    // if the player is outside the Pit (so it doesn't interfere with the
    // Infinite Pit options menu).
    mod::patch::writeBranch(
        reinterpret_cast<void*>(g_statusWinDisp_HideDpadMenuOutsidePit_BH),
        reinterpret_cast<void*>(StartPreventDpadShortcutsOutsidePit));
    mod::patch::writeBranch(
        reinterpret_cast<void*>(BranchBackPreventDpadShortcutsOutsidePit),
        reinterpret_cast<void*>(g_statusWinDisp_HideDpadMenuOutsidePit_EH));
    mod::patch::writeBranch(
        reinterpret_cast<void*>(ConditionalBranchPreventDpadShortcutsOutsidePit),
        reinterpret_cast<void*>(g_statusWinDisp_HideDpadMenuOutsidePit_CH1));
        
    // Make item name in battle menu based on item data rather than weapon data.
    mod::patch::writePatch(
        reinterpret_cast<void*>(
            g__btlcmd_MakeSelectWeaponTable_Patch_GetNameFromItem),
        0x807b0004U /* lwz r3, 0x4 (r27) */);
        
    // Apply patch to Mario menu code to open the moves menu on the jump/hammer
    // selections as well as the Special Moves one.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_winMarioMain_CheckOpenMoveMenu_BH),
        reinterpret_cast<void*>(StartCheckOpenMarioMoveMenu),
        reinterpret_cast<void*>(BranchBackCheckOpenMarioMoveMenu));
        
    // Apply patch to Mario menu code to display the max levels of all
    // currently unlocked jump / hammer / Special Moves.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_winMarioDisp_MoveMenuDisp_BH),
        reinterpret_cast<void*>(g_winMarioDisp_MoveMenuDisp_EH),
        reinterpret_cast<void*>(StartMarioMoveMenuDisp),
        reinterpret_cast<void*>(BranchBackMarioMoveMenuDisp));
        
    // Apply patch to Mario menu code to show the right description
    // of the currently selected jump / hammer / Special Move.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_winMarioMain_MoveDescription_BH),
        reinterpret_cast<void*>(g_winMarioMain_MoveDescription_EH),
        reinterpret_cast<void*>(StartMarioMoveMenuMsgEntry),
        reinterpret_cast<void*>(BranchBackMarioMoveMenuMsgEntry));
        
    // Apply patch to Party menu code to print the party member's description
    // and update the number of options in their moves dialog.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_winPartyMain_RotatePartnersHook_BH),
        reinterpret_cast<void*>(g_winPartyMain_RotatePartnersHook_EH),
        reinterpret_cast<void*>(StartPartySetPartnerDescAndMoveCount),
        reinterpret_cast<void*>(BranchBackPartySetPartnerDescAndMoveCount));
        
    // Apply patch to Party menu code to override the description and
    // XY cursor position for the moves window, given the current index.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_winPartyMain_OverrideMoveTextCursor_BH),
        reinterpret_cast<void*>(g_winPartyMain_OverrideMoveTextCursor_EH),
        reinterpret_cast<void*>(StartPartyOverrideMoveTextAndCursorPos),
        reinterpret_cast<void*>(BranchBackPartyOverrideMoveTextAndCursorPos));
        
    // Completely replace code to draw Party menu stats (HP and moves window).
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_winPartyDisp_StatsHook1_BH),
        reinterpret_cast<void*>(g_winPartyDisp_StatsHook1_EH),
        reinterpret_cast<void*>(StartPartyDispHook1),
        reinterpret_cast<void*>(BranchBackPartyDispHook1));
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_winPartyDisp_StatsHook2_BH),
        reinterpret_cast<void*>(g_winPartyDisp_StatsHook2_EH),
        reinterpret_cast<void*>(StartPartyDispHook2),
        reinterpret_cast<void*>(BranchBackPartyDispHook2));

    // Replace most win_item functions with custom logic.
    g_winItemMain_trampoline = patch::hookFunction(
        ttyd::win_item::winItemMain, [](WinPauseMenu* menu) {
            return tot::win::ItemMenuMain(menu);
        });
    g_winItemMain2_trampoline = patch::hookFunction(
        ttyd::win_item::winItemMain2, [](WinPauseMenu* menu) {
            tot::win::ItemMenuMain2(menu);
        });
    g_winItemDisp_trampoline = patch::hookFunction(
        ttyd::win_item::winItemDisp, [](
            CameraId camera, WinPauseMenu* menu, int32_t tab_number) {
            tot::win::ItemMenuDisp(camera, menu, tab_number);
        });
    g_itemUseDisp_trampoline = patch::hookFunction(
        ttyd::win_item::itemUseDisp, [](WinMgrEntry* menu) {
            tot::win::ItemSubdialogMain1(menu);
        });
    g_itemUseDisp2_trampoline = patch::hookFunction(
        ttyd::win_item::itemUseDisp2, [](WinMgrEntry* menu) {
            tot::win::ItemSubdialogMain2(menu);
        });

    // Add Mailbox SP to Key Items menu skip list (in place of Boat curse).
    ttyd::win_item::menu_skip_list[6] = ItemType::MAILBOX_SP;

    // Replace all basic win_log functions with custom logic.
    g_winLogInit_trampoline = patch::hookFunction(
        ttyd::win_log::winLogInit, [](WinPauseMenu* menu) {
            tot::win::LogMenuInit(menu);
        });
    g_winLogInit2_trampoline = patch::hookFunction(
        ttyd::win_log::winLogInit2, [](WinPauseMenu* menu) {
            tot::win::LogMenuInit2(menu);
        });
    g_winLogExit_trampoline = patch::hookFunction(
        ttyd::win_log::winLogExit, [](WinPauseMenu* menu) {
            tot::win::LogMenuExit(menu);
        });
    g_winLogMain_trampoline = patch::hookFunction(
        ttyd::win_log::winLogMain, [](WinPauseMenu* menu) {
            return tot::win::LogMenuMain(menu);
        });
    g_winLogMain2_trampoline = patch::hookFunction(
        ttyd::win_log::winLogMain2, [](WinPauseMenu* menu) {
            tot::win::LogMenuMain2(menu);
        });
    g_winLogDisp_trampoline = patch::hookFunction(
        ttyd::win_log::winLogDisp, [](
            CameraId camera, WinPauseMenu* menu, int32_t tab_number) {
            tot::win::LogMenuDisp(camera, menu, tab_number);
        });

    // Update sorting functions used by certain win_log menus.
    tot::win::ReplaceLogSortMethods();
        
    // Override the default "Type" order used for the Tattle log.
    int32_t kNumEnemyTypes = BattleUnitType::BONETAIL + 1;
    uint8_t custom_tattle_order[kNumEnemyTypes];
    for (int32_t i = 0; i <= kNumEnemyTypes; ++i) {
        custom_tattle_order[i] = 
            static_cast<uint8_t>(tot::GetCustomTattleIndex(i));
    }
    mod::patch::writePatch(
        ttyd::win_root::enemy_monoshiri_sort_table,
        custom_tattle_order, sizeof(custom_tattle_order));

    // winHakoGX (Item / Badge log box drawing function) patches:
    // References new WinPauseMenu field locations in initialization. 
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_winHakoGX_SetInitialFields_BH),
        reinterpret_cast<void*>(g_winHakoGX_SetInitialFields_EH),
        reinterpret_cast<void*>(StartHakoGxInitializeFields),
        reinterpret_cast<void*>(BranchBackHakoGxInitializeFields));
    // Handles checking whether to draw a "not obtained" box.
    mod::patch::writeBranch(
        reinterpret_cast<void*>(g_winHakoGX_CheckDrawNoItemBox_BH),
        reinterpret_cast<void*>(StartHakoGxCheckDrawNoItemBox));
    mod::patch::writeBranch(
        reinterpret_cast<void*>(ReturnHakoGxCheckDrawNoItemBoxNoItemCase),
        reinterpret_cast<void*>(g_winHakoGX_CheckDrawNoItemBox_EH));
    mod::patch::writeBranch(
        reinterpret_cast<void*>(ReturnHakoGxCheckDrawNoItemBoxItemCase),
        reinterpret_cast<void*>(g_winHakoGX_CheckDrawNoItemBox_CH1));
    // Handles checking whether to draw an item icon (opposite of above).
    mod::patch::writeBranch(
        reinterpret_cast<void*>(g_winHakoGX_CheckDrawItemIcon_BH),
        reinterpret_cast<void*>(StartHakoGxCheckDrawItemIcon));
    mod::patch::writeBranch(
        reinterpret_cast<void*>(ReturnHakoGxCheckDrawItemIconNoItemCase),
        reinterpret_cast<void*>(g_winHakoGX_CheckDrawItemIcon_EH));
    mod::patch::writeBranch(
        reinterpret_cast<void*>(ReturnHakoGxCheckDrawItemIconItemCase),
        reinterpret_cast<void*>(g_winHakoGX_CheckDrawItemIcon_CH1));
    // Patch out the logic to draw a single background box on the last page.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_winHakoGX_Patch_SkipSingleBox),
        0x48000a1cU  /* unconditional branch to 0x80159144 */);
        
    // Update winmgr::select_desc_tbl with a larger one to handle custom menus.
    uintptr_t new_select_desc_tbl = reinterpret_cast<uintptr_t>(
        tot::window_select::InitNewSelectDescTable());
    uint32_t new_select_desc_tbl_hi16 = 
        (0x3c60'0000U | (new_select_desc_tbl >> 16));
    uint32_t new_select_desc_tbl_lo16 =
        (0x6065'0000U | (new_select_desc_tbl & 0xffff));
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_winMgrSelectEntry_Patch_SelectDescTblHi16),
        new_select_desc_tbl_hi16  /* lis r3, ptr_hi16 */);
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_winMgrSelectEntry_Patch_SelectDescTblLo16),
        new_select_desc_tbl_lo16  /* ori r5, r3, ptr_lo16 */);
        
    // Run custom code after vanilla winMgrSelectEntry / Other for custom menus.
    g_winMgrSelectEntry_trampoline = patch::hookFunction(
        ttyd::winmgr::winMgrSelectEntry, [](
            int32_t type, int32_t new_item, int32_t cancellable) {
            if (type >= tot::window_select::MenuType::CUSTOM_START) {
                return tot::window_select::HandleSelectWindowEntry(type, new_item);
            }
            return g_winMgrSelectEntry_trampoline(type, new_item, cancellable);
        });
    g_winMgrSelectOther_trampoline = patch::hookFunction(
        ttyd::winmgr::winMgrSelectOther, [](
            WinMgrSelectEntry* sel_entry, EvtEntry* evt) {
            if (sel_entry->type >= tot::window_select::MenuType::CUSTOM_START) {
                return tot::window_select::HandleSelectWindowOther(
                    sel_entry, evt);
            }
            return g_winMgrSelectOther_trampoline(sel_entry, evt);
        });
        
    // Force the top bar closed instead of open for certain selection windows,
    // e.g. run options.
    mod::patch::writeBranchPair(
        reinterpret_cast<void*>(g_select_main_CheckHideTopBar_BH),
        reinterpret_cast<void*>(StartHideTopBarInSomeWindows),
        reinterpret_cast<void*>(BranchBackHideTopBarInSomeWindows));
}

void DisplayUpDownNumberIcons(
    int32_t number, void* tex_obj, gc::mtx34* icon_mtx, gc::mtx34* view_mtx, 
    uint32_t unk0) {
    gc::mtx34 pos_mtx;
    gc::mtx34 temp_mtx;
    
    ttyd::gx::GXAttr::GXSetNumTexGens(1);
    ttyd::gx::GXAttr::GXSetTexCoordGen2(0, 1, 4, 60, 0, 125);
    
    int32_t abs_number = number < 0 ? -number : number;
    if (abs_number > 99) abs_number = 99;
    double x_pos = abs_number > 9 ? 10.0 : 5.0;
    
    do {
        // Print digits, right-to-left.
        ttyd::icondrv::iconGetTexObj(
            &tex_obj, ttyd::eff_updown::icon_id[abs_number % 10]);
        ttyd::gx::GXTexture::GXLoadTexObj(&tex_obj, 0);
        if (number < 0) {
            gc::mtx::PSMTXTrans(&pos_mtx, x_pos, 7.0, 1.0);
        } else {
            gc::mtx::PSMTXTrans(&pos_mtx, x_pos, 0.0, 1.0);
        }
        gc::mtx::PSMTXConcat(icon_mtx, &pos_mtx, &temp_mtx);
        gc::mtx::PSMTXConcat(view_mtx, &temp_mtx, &temp_mtx);
        ttyd::gx::GXTransform::GXLoadPosMtxImm(&temp_mtx, 0);
        ttyd::eff_updown::polygon(
            -8.0, 16.0, 16.0, 16.0, 1.0, 1.0, 0, unk0);
        x_pos -= 10.0;
    } while (abs_number /= 10);
        
    // Print plus / minus sign.
    if (number < 0) {
        ttyd::icondrv::iconGetTexObj(&tex_obj, IconType::NUMBER_MINUS);
        ttyd::gx::GXTexture::GXLoadTexObj(&tex_obj, 0);
        gc::mtx::PSMTXTrans(&pos_mtx, x_pos, 7.0, 1.0);
    } else {
        ttyd::icondrv::iconGetTexObj(&tex_obj, IconType::NUMBER_PLUS);
        ttyd::gx::GXTexture::GXLoadTexObj(&tex_obj, 0);
        gc::mtx::PSMTXTrans(&pos_mtx, x_pos, 0.0, 1.0);
    }
    gc::mtx::PSMTXConcat(icon_mtx, &pos_mtx, &temp_mtx);
    gc::mtx::PSMTXConcat(view_mtx, &temp_mtx, &temp_mtx);
    ttyd::gx::GXTransform::GXLoadPosMtxImm(&temp_mtx, 0);
    ttyd::eff_updown::polygon(
        -8.0, 16.0, 16.0, 16.0, 1.0, 1.0, 0, unk0);
}

bool CheckOpenMarioMoveMenu(WinPauseMenu* menu) {
    int32_t starting_move = -1;
    switch (menu->mario_menu_state) {
        case 3:     starting_move = tot::MoveType::JUMP_BASE;       break;
        case 2:     starting_move = tot::MoveType::HAMMER_BASE;     break;
        case 12:    starting_move = tot::MoveType::SP_SWEET_TREAT;  break;
    }
    if (starting_move < 0) return false;
    
    // Update the maximum number of selections.
    int32_t num_selections = 0;
    for (int32_t i = 0; i < 8; ++i) {
        if (tot::MoveManager::GetUnlockedLevel(starting_move + i) > 0) {
            ++num_selections;
        }
    }
    menu->special_move_count = num_selections;
    
    return true;
}

void MarioMoveMenuDisp(WinPauseMenu* menu) {
    // Constants for menu code.
    gc::vec3    header_position         = { -50.0, 155.0, 0.0 };
    gc::vec3    header_scale            = { 0.8, 0.8, 0.8 };
    gc::vec3    sac_name_position       = { -250.0f, 130.0f, 0.0f };
    gc::vec3    sac_name_scale          = { 1.0f, 1.0f, 1.0f };
    gc::vec3    sac_max_lvl_position    = { -22.0f, 130.0f, 0.0f };
    gc::vec3    sac_max_lvl_scale       = { 1.0f, 1.0f, 1.0f };
    gc::vec3    sac_lvl_icon_position   = { -55.0f, 120.0f, 0.0f };
    gc::vec3    sac_lvl_icon_scale      = { 0.5f, 0.5f, 0.5f };
    char        level_str_buf[3]        = { 0, 0, 0 };
    uint32_t    header_color            = 0xffffffffU;
    uint32_t    sac_color               = 0xffU;

    // Print "Lvl." string on header in place of "SP".
    ttyd::win_main::winFontSetEdge(
        &header_position, &header_scale, &header_color, "Lvl.");
        
    int32_t starting_move;
    switch (menu->mario_menu_state) {
        case 3:     starting_move = tot::MoveType::JUMP_BASE;       break;
        case 2:     starting_move = tot::MoveType::HAMMER_BASE;     break;
        default:    starting_move = tot::MoveType::SP_SWEET_TREAT;  break;
    }
    
    for (int32_t i = 0; i < 8; ++i) {
        int32_t move = starting_move + i;
        // Get maximum level of attack; if not unlocked, skip.
        const int32_t max_level = tot::MoveManager::GetUnlockedLevel(move);
        if (max_level < 1) continue;
        
        // Print attack name.
        const char* name = ttyd::msgdrv::msgSearch(
            tot::MoveManager::GetMoveData(move)->name_msg);
        ttyd::win_main::winFontSetWidth(
            &sac_name_position, &sac_name_scale, &sac_color, 180.0, name);
        
        // Print mini "Lvl." string.
        ttyd::win_main::winFontSet(
            &sac_lvl_icon_position, &sac_lvl_icon_scale, &sac_color, "Lvl.");
        // Print max level (in a centered position).
        sprintf(level_str_buf, "%" PRId32, max_level);
        const uint32_t max_level_width =
            ttyd::fontmgr::FontGetMessageWidth(level_str_buf) / 2;
        sac_max_lvl_position.x = -21.0f - max_level_width / 2.0f;
        ttyd::win_main::winFontSet(
            &sac_max_lvl_position, &sac_max_lvl_scale, &sac_color, level_str_buf);
            
        // Move to next row of table.
        sac_name_position.y     -= 26.0;
        sac_max_lvl_position.y  -= 26.0;
        sac_lvl_icon_position.y -= 26.0;
    }
}

void MarioMoveMenuMsgEntry(WinPauseMenu* menu) {
    int32_t starting_move;
    switch (menu->mario_menu_state) {
        case 3:     starting_move = tot::MoveType::JUMP_BASE;       break;
        case 2:     starting_move = tot::MoveType::HAMMER_BASE;     break;
        default:    starting_move = tot::MoveType::SP_SWEET_TREAT;  break;
    }
    int32_t current_pos = -1;
    for (int32_t i = 0; i < 8; ++i) {
        int32_t move = starting_move + i;
        if (tot::MoveManager::GetUnlockedLevel(move) > 0) ++current_pos;
        if (current_pos == menu->special_move_cursor_idx) {
            ttyd::win_root::winMsgEntry(
                menu, 0, tot::MoveManager::GetMoveData(move)->desc_msg, 0);
            return;
        }
    }
}

void PartyMenuSetupPartnerDescAndMoveCount(WinPauseMenu* menu) {
    int32_t selected_partner_idx = menu->active_party_winPartyDt_idx;
    auto* winpartydt = ttyd::win_party::g_winPartyDt;
    int32_t party_id = winpartydt[selected_partner_idx].partner_id;
        
    // Calculate number of unlocked moves.
    int32_t starting_move;
    switch (party_id) {
        case 1:     starting_move = tot::MoveType::GOOMBELLA_BASE;  break;
        case 2:     starting_move = tot::MoveType::KOOPS_BASE;      break;
        case 3:     starting_move = tot::MoveType::BOBBERY_BASE;    break;
        case 4:     starting_move = tot::MoveType::YOSHI_BASE;      break;
        case 5:     starting_move = tot::MoveType::FLURRIE_BASE;    break;
        case 6:     starting_move = tot::MoveType::VIVIAN_BASE;     break;
        default:    starting_move = tot::MoveType::MOWZ_BASE;       break;
    }
    int32_t num_moves = 0;
    for (int32_t i = 0; i < 6; ++i) {
        int32_t move = starting_move + i;
        if (tot::MoveManager::GetUnlockedLevel(move) > 0) ++num_moves;
    }
    menu->party_moves_count = num_moves;
    
    ttyd::win_root::winMsgEntry(
        menu, 0, winpartydt[selected_partner_idx].msg_menu, 0);
}

void PartyMenuSetMoveDescAndCursorPos(WinPauseMenu* menu) {
    int32_t cursor_pos = menu->party_moves_cursor_idx;
    
    // Override cursor XY position.
    menu->main_cursor_target_x = -13.0f;
    menu->main_cursor_target_y = 38.0f - 23.4f * cursor_pos;
    
    int32_t selected_partner_idx = menu->active_party_winPartyDt_idx;
    int32_t party_id =
        ttyd::win_party::g_winPartyDt[selected_partner_idx].partner_id;
    
    // Set move description.
    int32_t starting_move;
    switch (party_id) {
        case 1:     starting_move = tot::MoveType::GOOMBELLA_BASE;  break;
        case 2:     starting_move = tot::MoveType::KOOPS_BASE;      break;
        case 3:     starting_move = tot::MoveType::BOBBERY_BASE;    break;
        case 4:     starting_move = tot::MoveType::YOSHI_BASE;      break;
        case 5:     starting_move = tot::MoveType::FLURRIE_BASE;    break;
        case 6:     starting_move = tot::MoveType::VIVIAN_BASE;     break;
        default:    starting_move = tot::MoveType::MOWZ_BASE;       break;
    }
    int32_t current_pos = -1;
    for (int32_t i = 0; i < 6; ++i) {
        int32_t move = starting_move + i;
        if (tot::MoveManager::GetUnlockedLevel(move) > 0) ++current_pos;
        if (current_pos == cursor_pos) {
            ttyd::win_root::winMsgEntry(
                menu, 0, tot::MoveManager::GetMoveData(move)->desc_msg, 0);
            return;
        }
    }
}

void PartyMenuDispStats(WinPauseMenu* menu) {
    int32_t tab = 0;
    for (int32_t i = 0; i < 5; ++i) {
        // Check for which tab is the Party tab.
        if (menu->tab_body_info[i].id == 1) {
            tab = i;
            break;
        }
    }
    float win_x = menu->tab_body_info[tab].x;
    float win_y = menu->tab_body_info[tab].y;
    
    int32_t selected_partner_idx = menu->active_party_winPartyDt_idx;
    int32_t party_id =
        ttyd::win_party::g_winPartyDt[selected_partner_idx].partner_id;

    auto* pouch = ttyd::mario_pouch::pouchGetPtr();
    
    // "Constant" colors.
    static uint32_t kBlack = 0x000000FFU;
    static uint32_t kWhite = 0xFFFFFFFFU;
    static uint32_t kPinkBlobColor = 0xCB8CA2FEU;
    static uint32_t kHeartPointsTextColor = 0xA73C3CFFU;
    
    // Temporary variables, used across draw calls.
    gc::vec3 pos = { 0.0f, 0.0f, 0.0f };
    gc::vec3 scale;
    int32_t width;
    
    // Top-left corner of the two windows, for reference.
    gc::vec3 hp_win = { win_x + 5.0f, win_y + 136.0f, 0.0f };
    gc::vec3 tbl_win = { win_x - 5.0f, win_y + 80.0f, 0.0f };

    // Draw recessed window for HP.
    ttyd::win_root::winKirinukiGX(hp_win.x, hp_win.y, 240.0f, 46.0f, menu, 0);
    // Draw move table background.
    ttyd::win_root::winWazaGX(tbl_win.x, tbl_win.y, 260.0f, 176.0f, menu, 0);

    // Draw pink blob behind "HP" text.
    ttyd::win_main::winTexInit(*menu->win_tpl->mpFileData);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_TEXA, GX_CA_ZERO);
    pos.x = hp_win.x + 45.0f - 2.0f;
    pos.y = hp_win.y - 26.0f - 4.0f;
    scale.x = 1.0f;
    scale.y = 1.0f;
    scale.z = 1.0f;
    ttyd::win_main::winTexSet(0xaf, &pos, &scale, &kPinkBlobColor);
    
    // Draw slash between current and maximum HP.
    ttyd::win_main::winTexInit(*menu->win_tpl->mpFileData);
    pos.x = hp_win.x + 8.0f + 170.f;
    pos.y = hp_win.y - 26.0f + 4.0f;
    ttyd::win_main::winTexSet(0x10, &pos, &scale, &kWhite);
    
    // Draw heart icon for HP.
    ttyd::win_main::winIconInit();
    pos.x = hp_win.x + 45.0f + 60.f;
    pos.y = hp_win.y - 26.0f + 4.0f;
    scale.x = 0.82f;
    scale.y = 0.82f;
    scale.z = 0.82f;
    ttyd::win_main::winIconSet(IconType::HP_ICON, &pos, &scale, &kWhite);
        
    ttyd::win_main::winFontInit();
    
    // Draw "HP" text.
    const char* hp_string = msgSearch("msg_menu_party_hp");
    width = ttyd::fontmgr::FontGetMessageWidth(hp_string);
    pos.x = hp_win.x + 45.0f - width * 0.5f;
    pos.y = hp_win.y - 26.0f + 12.0f;
    scale.x = 1.0f;
    scale.y = 1.0f;
    scale.z = 1.0f;
    ttyd::win_main::winFontSet(&pos, &scale, &kBlack, hp_string);

    // Draw "Heart Pts." text.
    const char* ruby_string = ttyd::msgdrv::msgSearch("msg_menu_mario_ruby_hp");
    width = ttyd::fontmgr::FontGetMessageWidth(ruby_string);
    pos.x = hp_win.x + 45.0f - width * 0.25f;
    pos.y = hp_win.y - 26.0f + 21.0f;
    scale.x = 0.5f;
    scale.y = 0.5f;
    scale.z = 0.5f;
    ttyd::win_main::winFontSet(&pos, &scale, &kHeartPointsTextColor, ruby_string);

    // Draw HP current and max numbers.
    const char* temp_current_hp = ttyd::win_mario::winZenkakuStr(
        pouch->party_data[party_id].current_hp);
    pos.x = hp_win.x + 10.0f + 110.0f;
    pos.y = hp_win.y - 26.0f + 4.0f + 12.0f;
    scale.x = 0.9f;
    scale.y = 0.9f;
    scale.z = 0.9f;
    ttyd::win_main::winFontSetR(&pos, &scale, &kBlack, "%s", temp_current_hp);

    const char* temp_max_hp = ttyd::win_mario::winZenkakuStr(
        pouch->party_data[party_id].max_hp);
    pos.x = hp_win.x + 8.0f + 180.0f;
    pos.y = hp_win.y - 26.0f + 4.0f + 12.0f;
    ttyd::win_main::winFontSet(&pos, &scale, &kBlack, "%s", temp_max_hp);

    // Draw "Check moves" text and X button icon.
    const char* check_moves = msgSearch("msg_menu_party_waza_kakunin");
    width = ttyd::fontmgr::FontGetMessageWidth(check_moves);
    pos.x = win_x + 250.0f - 0.7f * width;
    pos.y = win_y - 100.f;
    scale.x = 0.7f;
    scale.y = 0.7f;
    scale.z = 0.7f;
    ttyd::win_main::winFontSet(&pos, &scale, &kBlack, check_moves);

    ttyd::win_main::winIconInit();
    pos.x = win_x + 250.0f - 0.7f * width - 20.f;
    pos.y = win_y - 110.f;
    ttyd::win_main::winIconSet(IconType::FLAT_X_BUTTON, &pos, &scale, &kWhite);

    ttyd::win_main::winFontInit();
    
    // Draw table headings.
    const char* move = msgSearch("msg_menu_party_waza");
    width = ttyd::fontmgr::FontGetMessageWidth(move);
    pos.x = tbl_win.x + 5.0f + (200.0f - 0.8f * width) * 0.5f;
    pos.y = tbl_win.y - 5.0f;
    scale.x = 0.8f;
    scale.y = 0.8f;
    scale.z = 0.8f;
    ttyd::win_main::winFontSetEdge(&pos, &scale, &kWhite, move);

    pos.x = tbl_win.x + 213.0f;
    pos.y = tbl_win.y - 5.0f;
    ttyd::win_main::winFontSetEdge(&pos, &scale, &kWhite, "Lvl.");

    // Draw rows of table.
    int32_t starting_move;
    switch (party_id) {
        case 1:     starting_move = tot::MoveType::GOOMBELLA_BASE;  break;
        case 2:     starting_move = tot::MoveType::KOOPS_BASE;      break;
        case 3:     starting_move = tot::MoveType::BOBBERY_BASE;    break;
        case 4:     starting_move = tot::MoveType::YOSHI_BASE;      break;
        case 5:     starting_move = tot::MoveType::FLURRIE_BASE;    break;
        case 6:     starting_move = tot::MoveType::VIVIAN_BASE;     break;
        default:    starting_move = tot::MoveType::MOWZ_BASE;       break;
    }
    float y_offset = 0.0f;
    for (int32_t i = 0; i < 6; ++i) {
        int32_t move = starting_move + i;
        
        // Skip moves that aren't unlocked.
        int32_t level = tot::MoveManager::GetUnlockedLevel(move);
        if (level < 1) continue;
        
        auto* move_data = tot::MoveManager::GetMoveData(move);
        const char* move_name = msgSearch(move_data->name_msg);
        int32_t max_level = move_data->max_level;
        
        pos.x = tbl_win.x + 15.0f;
        pos.y = tbl_win.y - 31.0f + y_offset;
        scale.x = 0.9f;
        scale.y = 0.9f;
        scale.z = 0.9f;
        ttyd::win_main::winFontSetWidth(&pos, &scale, &kBlack, 175.0f, move_name);
        
        if (max_level > 1) {
            // Print current level if move's max level is > 1.
            const char* level_temp = ttyd::win_mario::winZenkakuStr(level);
            width = ttyd::fontmgr::FontGetMessageWidth(level_temp);
            pos.x = tbl_win.x + 243.0f - 0.5f * width;
            ttyd::win_main::winFontSet(&pos, &scale, &kBlack, "%s", level_temp);
            
            pos.x = tbl_win.x + 209.0f;
            pos.y  -= 8.0f;
            scale.x = 0.5f;
            scale.y = 0.5f;
            scale.z = 0.5f;
            ttyd::win_main::winFontSet(&pos, &scale, &kBlack, "Lvl.");
        } else {
            // Print "-" instead.
            pos.x = tbl_win.x + 225.0f;
            ttyd::win_main::winFontSet(&pos, &scale, &kBlack, "-");
        }
        
        y_offset -= 23.4f;
    }
}

int32_t GetIconForBadgeOrItemLogEntry(
    WinPauseMenu* menu, bool item_log, int32_t index) {
    const int32_t item_type = item_log
        ? menu->recipe_log_ids[index] : menu->badge_log_ids[index];
    return g_Mod->state_.GetOption(tot::FLAGS_ITEM_ENCOUNTERED, item_type)
        ? ttyd::item_data::itemDataTable[item_type + 0x80].icon_id : -1;
}

}  // namespace ui
}  // namespace mod::infinite_pit