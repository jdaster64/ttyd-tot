#pragma once

#include "evt_cmd.h"

#include <ttyd/evtmgr.h>

#include <cstdint>

namespace mod::tot {

// Picks an item from the standardized pool of items / stackable badges used
// for various purposes (enemy items, Charlieton, Kiss Thief, etc.),
// using the specified RngSequence type.
// Returns 0 if the "no item" case was picked.
// If no partners are currently unlocked, will not pick "P" badges.
int32_t PickRandomItem(
    int32_t sequence, int32_t normal_item_weight, int32_t recipe_item_weight,
    int32_t badge_weight, int32_t no_item_weight = 0);

// Returns a comparison value that sorts items by ascending buy price.
int32_t BuyPriceComparator(int16_t* lhs, int16_t* rhs);
// Returns a comparison value that sorts items/badges by type sort order.
int32_t TypeSortOrderComparator(int16_t* lhs, int16_t* rhs);
// Returns a pointer to Charlieton's item inventory.
// Holds up to 20 items + a '-1' terminator.
int16_t* GetCharlietonInventoryPtr();
// Returns the price for an item or NPC service, scaled based on tower progress
// and the overall coin price scaling option.
int32_t GetScaledPrice(int32_t base_price);
    
// Obfuscates or un-obfuscates the appearance and description of items.
void ObfuscateItems(bool enable);

// Generates the next set of items for the hub item shop.
void GenerateHubShopItems();

// Generates a globally unique name to use to identify item pickups.
// This is necessary to avoid softlocks with full inventory.
EVT_DECLARE_USER_FUNC(evtTot_GetUniqueItemName, 1)

}