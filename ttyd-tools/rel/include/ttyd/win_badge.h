#pragma once

#include <cstdint>

namespace ttyd::win_root { struct WinPauseMenu; }

namespace ttyd::win_badge {

extern "C" {

// badge_disp
// winBadgeDisp
// winBadgeMain2
// winBadgeMain
void mario_change(win_root::WinPauseMenu* menu);
// winBadgeExit
// winBadgeInit2
// winBadgeInit
// winMakeEquipList

}

}