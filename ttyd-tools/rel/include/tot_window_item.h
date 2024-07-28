#pragma once

#include <ttyd/dispdrv.h>

#include <cstdint>

// Forward declarations.
namespace ttyd::win_root { struct WinPauseMenu; }
namespace ttyd::winmgr { struct WinMgrEntry; }

namespace mod::tot::win {

// Replacement functions for the Items tab in the pause menu.

int32_t ItemMenuMain(ttyd::win_root::WinPauseMenu* menu);

void ItemMenuMain2(ttyd::win_root::WinPauseMenu* menu);

void ItemMenuDisp(
    ttyd::dispdrv::CameraId camera_id, ttyd::win_root::WinPauseMenu* menu,
    int32_t tab_number);

// For the dialog that appears when using items.
void ItemSubdialogMain1(ttyd::winmgr::WinMgrEntry* winmgr_entry);
void ItemSubdialogMain2(ttyd::winmgr::WinMgrEntry* winmgr_entry);

}