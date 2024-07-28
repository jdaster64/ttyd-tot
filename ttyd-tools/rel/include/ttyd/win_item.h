#pragma once

#include <ttyd/dispdrv.h>

#include <cstdint>

namespace ttyd::win_root { struct WinPauseMenu; }
namespace ttyd::winmgr { struct WinMgrEntry; }

namespace ttyd::win_item {

extern "C" {

void itemUseDisp2(ttyd::winmgr::WinMgrEntry* winmgr_entry);
void itemUseDisp(ttyd::winmgr::WinMgrEntry* winmgr_entry);
// item_disp
void winItemDisp(
    ttyd::dispdrv::CameraId camera, ttyd::win_root::WinPauseMenu* menu, 
    int32_t tab_number);
void winItemMain2(ttyd::win_root::WinPauseMenu* menu);
int32_t winItemMain(ttyd::win_root::WinPauseMenu* menu);
// winItemExit
// winItemInit2
// winItemInit
// winMakeSkipList

// List of items that are not displayed in Key Items menu.
extern int16_t menu_skip_list[18];

}

}