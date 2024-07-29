#pragma once

#include <ttyd/dispdrv.h>

#include <cstdint>

// Forward declarations.
namespace ttyd::win_root { struct WinPauseMenu; }
namespace ttyd::winmgr { struct WinMgrEntry; }

namespace mod::tot::win {

// Replacement functions for the Journal ("Log") tab in the pause menu.

void LogMenuInit(ttyd::win_root::WinPauseMenu* menu);

void LogMenuInit2(ttyd::win_root::WinPauseMenu* menu);

void LogMenuExit(ttyd::win_root::WinPauseMenu* menu);

int32_t LogMenuMain(ttyd::win_root::WinPauseMenu* menu);

void LogMenuMain2(ttyd::win_root::WinPauseMenu* menu);

void LogMenuDisp(
    ttyd::dispdrv::CameraId camera_id, ttyd::win_root::WinPauseMenu* menu,
    int32_t tab_number);

}