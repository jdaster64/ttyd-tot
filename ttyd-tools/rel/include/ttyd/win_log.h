#pragma once

#include <ttyd/dispdrv.h>

#include <cstdint>

namespace ttyd::win_root {
    struct WinLogTattleMenuWork;
    struct WinPauseMenu; 
}
namespace ttyd::winmgr { struct WinMgrEntry; }

namespace ttyd::win_log {

extern "C" {

// monosiri_disp
// capture
// monosiriExit
// monosiriCloseWait
// monosiriClose
void monosiriMain(ttyd::win_root::WinLogTattleMenuWork* tattle_work);
// monosiriInit
void monoshiriGX(double x, double y, ttyd::win_root::WinPauseMenu* menu);
// mapGX
// winGetMapTplName
void winLogDisp(
    ttyd::dispdrv::CameraId camera, ttyd::win_root::WinPauseMenu* menu, 
    int32_t tab_number);
void winLogMain2(ttyd::win_root::WinPauseMenu* menu);
int32_t winLogMain(ttyd::win_root::WinPauseMenu* menu);
void winLogExit(ttyd::win_root::WinPauseMenu* menu);
void winLogInit2(ttyd::win_root::WinPauseMenu* menu);
void winLogInit(ttyd::win_root::WinPauseMenu* menu);

}

}