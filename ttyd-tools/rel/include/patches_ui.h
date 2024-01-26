#pragma once

#include <gc/types.h>
#include <ttyd/win_party.h>

#include <cstdint>

namespace mod::infinite_pit::ui {

// Apply patches to the HUD, pause menu, etc.
void ApplyFixedPatches();

// Correctly displays multi-digit Charge / ATK / DEF-change number icons.
void DisplayUpDownNumberIcons(
    int32_t number, void* tex_obj, gc::mtx34* icon_mtx, gc::mtx34* view_mtx,
    uint32_t unk0);
// Prints the maximum level of each available Star Power to the "Mario" menu.
void StarPowerMenuDisp();
// Gets the correct description for the selected Star Power in the menu.
const char* GetStarPowerMenuDescriptionMsg(int32_t cursor_pos);
// Stores pointers to WinPartyData entries in the correct order based
// on the currently active partner and partners currently obtained.
void GetPartyMemberMenuOrder(ttyd::win_party::WinPartyData** out_party_data);
// If the player attempts to use the currently selected item in the pause
// menu on an invalid target, prevents it and returns true.
bool CheckForUnusableItemInMenu();
// Ranks up and fully heals the selected party member when using a Shine Sprite,
// or restores random HP/FP if using a Strawberry Cake.
void UseSpecialItemsInMenu(ttyd::win_party::WinPartyData** party_data);
// Initializes the Tattle log Journal page with only Infinite Pit's enemies.
void InitializeTattleLog(void* win_log_ptr);

}