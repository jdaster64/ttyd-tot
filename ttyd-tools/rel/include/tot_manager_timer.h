#pragma once

#include <ttyd/evtmgr.h>

#include <cstdint>

namespace mod::tot {
    
class TimerManager {
public:
    // Function that updates ToT timer tracking functions every frame.
    static void Update();
    // Function that draws the current time to the screen.
    static void Draw();
};

// Toggles the in-game timer on or off.
EVT_DECLARE_USER_FUNC(evtTot_ToggleIGT, 1)

}