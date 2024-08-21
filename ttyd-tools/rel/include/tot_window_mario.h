#pragma once

#include <ttyd/dispdrv.h>

#include <cstdint>

// Forward declarations.
namespace ttyd::win_root { struct WinPauseMenu; }
namespace ttyd::winmgr { struct WinMgrEntry; }

namespace mod::tot::win {

// Replacement functions for the Mario tab in the pause menu.

void MarioMenuInit(ttyd::win_root::WinPauseMenu* menu);

void MarioMenuInit2(ttyd::win_root::WinPauseMenu* menu);

int32_t MarioMenuMain(ttyd::win_root::WinPauseMenu* menu);

void MarioMenuDisp(
    ttyd::dispdrv::CameraId camera_id, ttyd::win_root::WinPauseMenu* menu,
    int32_t tab_number);

}