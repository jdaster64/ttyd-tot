#pragma once

#include "evt_cmd.h"

#include <ttyd/evtmgr.h>

#include <cstdint>

namespace mod::tot {
    
class OptionsManager {
public:
    // Sets up starting stats, etc. from cold boot / returning to lobby.
    static void InitLobby();

    // Sets up starting stats, etc. from tot_state options.
    static void InitFromSelectedOptions();

    // Sets up options that need to be initialized on loading a file.
    static void ApplyOptionsOnLoad();

    // Sets HP, FP, and BP according to current fields in tot_state.
    static void UpdateLevelupStats();
};

}