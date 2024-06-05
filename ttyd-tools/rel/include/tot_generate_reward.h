#pragma once

#include "evt_cmd.h"

#include <ttyd/evtmgr.h>

#include <cstdint>

namespace mod::tot {

class RewardManager {
public:
    // Change item data for items used as placeholders for rewards.
    static void PatchRewardItemData();
    // Apply special effects for reward items.
    static bool HandleRewardItemPickup(int32_t item_type);
    // Returns a pointer to an array of moves chosen for unlock/upgrade.
    // Guaranteed to end with a -1.
    static int32_t* GetSelectedMoves(int32_t* num_moves = nullptr);
    // Returns a pointer to the event that runs when picking up a Star Piece
    // on the field (as an item drop / condition reward).
    static void* GetStarPieceItemDropEvt();
    static void* GetShineSpriteItemDropEvt();
    
    // Picks a random unique badge to add to Charlieton's shop.
    // Returns 0 if the attempted badge is already in a chest on the same floor.
    static int32_t GetUniqueBadgeForShop();
    // Marks unique items as being collected.
    static void MarkUniqueItemCollected(int32_t item_id);
};

// Generates chest contents.
EVT_DECLARE_USER_FUNC(evtTot_GenerateChestContents, 0)

// Returns information about a chest.
// arg0     = (in) index
// arg1~3   = position
// arg4     = item
// arg5     = item pickup callback script
EVT_DECLARE_USER_FUNC(evtTot_GetChestData, 6)

// Raises the max level of a random move (in-battle).
// arg0 = (out) announcement string.
EVT_DECLARE_USER_FUNC(evtTot_RankUpRandomMoveInBattle, 1)

// Draws icons above chests.
EVT_DECLARE_USER_FUNC(evtTot_DisplayChestIcons, 0)

// Run after buying an item from an NPC.
EVT_DECLARE_USER_FUNC(evtTot_AfterItemBought, 1)

}