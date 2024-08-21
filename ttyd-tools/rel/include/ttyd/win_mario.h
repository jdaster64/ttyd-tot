#pragma once

#include <gc/types.h>

#include <cstdint>

namespace ttyd::win_root { struct WinPauseMenu; }

namespace ttyd::win_mario {

struct WinMarioLinkData {
    int16_t x_offset;
    int16_t y_offset;
    const char* pose_1;
    const char* pose_2;
    const char* pose_3;
    int16_t item_id;        // key item id that enables this menu option
    int16_t pad_0x12;
    const char* help_msg;
    int8_t neighbors[4];    // index to jump to when moving up/down/left/right.
};
static_assert(sizeof(WinMarioLinkData) == 0x1c);

extern "C" {

void winMarioDisp(
    ttyd::dispdrv::CameraId camera, ttyd::win_root::WinPauseMenu* menu, 
    int32_t tab_number);
void fukidashi(double x, double y, win_root::WinPauseMenu* menu, int type);
// unk_801703e8
const char* winZenkakuStr(int32_t value);
// winMarioMain2
int32_t winMarioMain(win_root::WinPauseMenu* menu);
// winMarioExit
void winMarioInit2(win_root::WinPauseMenu* menu);
void winMarioInit(win_root::WinPauseMenu* menu);

extern WinMarioLinkData linkDt[18];
extern const char* hammer_help[4];
extern const char* boots_help[4];
extern const char* hammer_pose[4];

}

}