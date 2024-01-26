#pragma once

#include <cstdint>

namespace mod::infinite_pit {

// Picks an item from the standardized pool of items / stackable badges used
// for various purposes (enemy items, Charlieton, Kiss Thief, etc.),
// using the specified RngSequence type.
// Returns 0 if the "no item" case was picked.
// If force_no_partner is set or you have no partners, will not pick "P" badges.
int32_t PickRandomItem(
    int32_t sequence, int32_t normal_item_weight, int32_t recipe_item_weight,
    int32_t badge_weight, int32_t no_item_weight = 0);
    
// Obfuscates or un-obfuscates the appearance and description of items.
void ObfuscateItems(bool enable);
    
// Returns whether an item is a stackable Mario badge that has a "P" variant.
bool IsStackableMarioBadge(int32_t item_type);
// Returns whether an item is a stackable badge that is a "P" variant.
bool IsStackablePartnerBadge(int32_t item_type);

}