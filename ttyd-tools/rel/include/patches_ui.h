#pragma once

#include <gc/types.h>

#include <cstdint>

namespace ttyd::seq_load { struct SeqLoadWinDataEntry; }
namespace ttyd::win_root { struct WinPauseMenu; }

namespace mod::infinite_pit::ui {

// Apply patches to the HUD, pause menu, etc.
void ApplyFixedPatches();

// Displays completion percentage + run progress on file select screen.
void DisplayFileSelectProgress(
    ttyd::seq_load::SeqLoadWinDataEntry* win, int32_t file, int32_t file_hovered);

// Correctly displays multi-digit Charge / ATK / DEF-change number icons.
void DisplayUpDownNumberIcons(
    int32_t number, void* tex_obj, gc::mtx34* icon_mtx, gc::mtx34* view_mtx,
    uint32_t unk0);

// Prints the partner's description and sets their move count in the Party menu.
void PartyMenuSetupPartnerDescAndMoveCount(ttyd::win_root::WinPauseMenu* menu);
// Sets the move description and updates the move cursor XY pos for Party menu.
void PartyMenuSetMoveDescAndCursorPos(ttyd::win_root::WinPauseMenu* menu);
// Prints all of the information on the right side of the Party tab.
void PartyMenuDispStats(ttyd::win_root::WinPauseMenu* menu);

// Returns the icon for the Nth item in the badge/items collected log.
int32_t GetIconForBadgeOrItemLogEntry(
    ttyd::win_root::WinPauseMenu* menu, bool item_log, int32_t index);

}