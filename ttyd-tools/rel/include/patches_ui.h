#pragma once

#include <gc/types.h>
#include <ttyd/win_party.h>

#include <cstdint>

namespace ttyd::win_root { struct WinPauseMenu; }

namespace mod::infinite_pit::ui {

// Apply patches to the HUD, pause menu, etc.
void ApplyFixedPatches();

// Correctly displays multi-digit Charge / ATK / DEF-change number icons.
void DisplayUpDownNumberIcons(
    int32_t number, void* tex_obj, gc::mtx34* icon_mtx, gc::mtx34* view_mtx,
    uint32_t unk0);

// Checks whether to open the moves submenu in the "Mario" tab.
bool CheckOpenMarioMoveMenu(ttyd::win_root::WinPauseMenu* menu);
// Prints the maximum level of each available move in the Mario tab.
void MarioMoveMenuDisp(ttyd::win_root::WinPauseMenu* menu);
// Prints the description for the currently hovered move in the Mario tab.
void MarioMoveMenuMsgEntry(ttyd::win_root::WinPauseMenu* menu);

// Prints the partner's description and sets their move count in the Party menu.
void PartyMenuSetupPartnerDescAndMoveCount(ttyd::win_root::WinPauseMenu* menu);
// Sets the move description and updates the move cursor XY pos for Party menu.
void PartyMenuSetMoveDescAndCursorPos(ttyd::win_root::WinPauseMenu* menu);
// Prints all of the information on the right side of the Party tab.
void PartyMenuDispStats(ttyd::win_root::WinPauseMenu* menu);

// Initializes the Tattle log Journal page with only enemies used in ToT.
void InitializeTattleLog(ttyd::win_root::WinPauseMenu* menu);

}