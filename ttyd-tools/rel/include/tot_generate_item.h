#pragma once

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
    
// Obfuscates or un-obfuscates the appearance and description of items.
void ObfuscateItems(bool enable);

}