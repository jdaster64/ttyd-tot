#pragma once

#include <cstdint>

namespace ttyd::evtmgr {
struct EvtEntry;
}
namespace ttyd::winmgr {
struct WinMgrSelectEntry;
}

namespace mod::tot::window_select {

// Extension to evtmgr::WinMgrSelectEntry_Type.
namespace MenuType {
    enum e {
        // Dummy.
        CUSTOM_START = 19,
        // Unlocking new moves.
        MOVE_UNLOCK,
        // Upgrading moves.
        MOVE_UPGRADE,
        // Charlieton.
        TOT_CHARLIETON_SHOP,
        // Run options menu (in lobby):
        RUN_OPTIONS,
        // Run results: general stats.
        RUN_RESULTS_STATS,
        // Run results: timer splits.
        RUN_RESULTS_SPLITS,
        
        MAX_MENU_TYPE,
    };
}

// Initializes a new table of selection window descriptions, including all of
// the ones used by the vanilla game.
void* InitNewSelectDescTable();

// Runs when winMgrSelectEntry is run to handle custom selection windows.
ttyd::winmgr::WinMgrSelectEntry* HandleSelectWindowEntry(
    int32_t type, int32_t new_item);

// Runs when winMgrSelectOther is run to handle custom selection windows.
int32_t HandleSelectWindowOther(
    ttyd::winmgr::WinMgrSelectEntry* sel_entry, ttyd::evtmgr::EvtEntry* evt);

}