#include "custom_item.h"

#include "mod.h"
#include "mod_state.h"

#include <ttyd/item_data.h>
#include <ttyd/system.h>

#include <cstdint>
#include <cstring>

namespace mod::infinite_pit {

namespace {

namespace ItemType = ::ttyd::item_data::ItemType;

// Bitfields of whether each item is included in its respective pool or not;
// item X is enabled if kItemPool[X / 16 - offset] & (1 << (X % 16)) != 0.
static constexpr const uint16_t kNormalItems[] = {
    0xffff, 0xffff, 0x000f, 0x0006
};
static constexpr const uint16_t kRecipeItems[] = {
    0x2020, 0xffb8, 0x3edf, 0x97ef, 0x0bff
};
static constexpr const uint16_t kStackableBadges[] = {
    0x3fff, 0xffff, 0x0fff, 0xfff7, 0x018f, 0x0030, 0x0006
};
static constexpr const uint16_t kStackableBadgesNoP[] = {
    0x3fff, 0x5555, 0x0b55, 0xaad7, 0x0186, 0x0030, 0x0002
};
static constexpr const uint16_t kChestRewardBadges[] = {
    0xc000, 0x0000, 0xf000, 0x0008, 0x0010, 0x0041, 0x0000
};

bool CheckBitfieldBit(const uint16_t* bitfield, int32_t idx) {
    return bitfield[idx >> 4] & (1 << (idx & 0xf));
}

uint16_t* PopulateFromBitfield(
    uint16_t* arr, const uint16_t* bitfield, 
    int32_t len_bitfield, int32_t start_item_offset) {
    for (int32_t i = 0; i < len_bitfield; ++i) {
        for (int32_t bit = 0; bit < 16; ++bit) {
            if (bitfield[i] & (1U << bit)) {
                *arr++ = start_item_offset + i * 16 + bit;
            }
        }
    }
    return arr;
}

template <class T> inline void KnuthShuffle(T* arr, int32_t size) {
    StateManager_v2& state = g_Mod->state_;
    for (int32_t i = size-1; i > 0; --i) {
        int32_t j = state.Rand(i+1, RNG_ITEM_OBFUSCATION);
        T temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

template <class T> inline void KnuthUnshuffle(T* arr, int32_t size) {
    StateManager_v2& state = g_Mod->state_;
    for (int32_t i = 1; i < size; ++i) {
        --state.rng_sequences_[RNG_ITEM_OBFUSCATION];
        int32_t j = state.Rand(i+1, RNG_ITEM_OBFUSCATION);
        --state.rng_sequences_[RNG_ITEM_OBFUSCATION];
        T temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

}

int32_t PickRandomItem(
    int32_t sequence, int32_t normal_item_weight, int32_t recipe_item_weight,
    int32_t badge_weight, int32_t no_item_weight) {
    StateManager_v2& state = g_Mod->state_;
    
    int32_t total_weight =
        normal_item_weight + recipe_item_weight + badge_weight + no_item_weight;
    int32_t result; 
    result = state.Rand(total_weight, sequence);
    const uint16_t* bitfield;
    int32_t len_bitfield;
    int32_t offset;
    
    int32_t current_weight = normal_item_weight;
    if (result < current_weight) {
        bitfield = kNormalItems;
        len_bitfield = sizeof(kNormalItems) / sizeof(uint16_t);
        offset = 0x80;
    } else {
        current_weight += recipe_item_weight;
        if (result < current_weight) {
            bitfield = kRecipeItems;
            len_bitfield = sizeof(kRecipeItems) / sizeof(uint16_t);
            offset = 0xa0;
        } else {
            current_weight += badge_weight;
            if (result < current_weight) {
                bool partners_enabled =
                    state.GetOptionNumericValue(OPT_ENABLE_P_BADGES);
                bitfield =
                    partners_enabled ? kStackableBadges : kStackableBadgesNoP;
                len_bitfield = sizeof(kStackableBadges) / sizeof(uint16_t);
                offset = 0xf0;
            } else {
                // "No item" chosen; make sure # of rand calls is consistent.
                state.Rand(1, sequence);
                return 0;
            }
        }
    }
    
    int32_t num_items_seen = 0;
    for (int32_t i = 0; i < len_bitfield; ++i) {
        for (int32_t bit = 0; bit < 16; ++bit) {
            if (bitfield[i] & (1U << bit)) ++num_items_seen;
        }
    }
    result = state.Rand(num_items_seen, sequence);
    num_items_seen = 0;
    for (int32_t i = 0; i < len_bitfield; ++i) {
        for (int32_t bit = 0; bit < 16; ++bit) {
            if (bitfield[i] & (1U << bit)) {
                if (result == num_items_seen) return offset + i * 16 + bit;
                ++num_items_seen;
            }
        }
    }
    // Should not be reached, as that would mean the random function returned
    // a larger index than there are bits in the bitfield.
    return -1;
}

void ObfuscateItems(bool enable) {
    // Obfuscation has already been performed; no need to redo.
    if (enable && g_Mod->state_.rng_sequences_[RNG_ITEM_OBFUSCATION]) return;
    
    auto* itemData = ttyd::item_data::itemDataTable;
    uint16_t ids[180];
    
    uint16_t* ptr = ids;
    ptr = PopulateFromBitfield(
        ptr, kNormalItems, sizeof(kNormalItems) / sizeof(uint16_t), 0x80);
    ptr = PopulateFromBitfield(
        ptr, kRecipeItems, sizeof(kRecipeItems) / sizeof(uint16_t), 0xa0);
    const int32_t num_items = (ptr - ids);
        
    ptr = PopulateFromBitfield(
        ptr, kStackableBadges, 
        sizeof(kStackableBadges) / sizeof(uint16_t), 0xf0);
    ptr = PopulateFromBitfield(
        ptr, kChestRewardBadges, 
        sizeof(kChestRewardBadges) / sizeof(uint16_t), 0xf0);
    const int32_t num_badges_and_items = (ptr - ids);
    const int32_t num_badges = num_badges_and_items - num_items;
    
    // For each type of field we want to shuffle the ids, then stage all the
    // changed data values all at once, since this needs to be reversible!
    uint16_t shuffled_ids[180];
    uint32_t data[180];
    
    if (enable) {        
        // Shuffle names.
        memcpy(shuffled_ids, ids, sizeof(ids));
        KnuthShuffle(shuffled_ids, num_badges_and_items);
        for (int32_t i = 0; i < num_badges_and_items; ++i) {
            data[i] = reinterpret_cast<uint32_t>(
                itemData[shuffled_ids[i]].name);
        }
        for (int32_t i = 0; i < num_badges_and_items; ++i) {
            itemData[ids[i]].name = reinterpret_cast<const char*>(data[i]);
        }
        
        // Shuffle description & menu description (same order for both).
        memcpy(shuffled_ids, ids, sizeof(ids));
        KnuthShuffle(shuffled_ids, num_badges_and_items);
        for (int32_t i = 0; i < num_badges_and_items; ++i) {
            data[i] = reinterpret_cast<uint32_t>(
                itemData[shuffled_ids[i]].description);
        }
        for (int32_t i = 0; i < num_badges_and_items; ++i) {
            itemData[ids[i]].description = 
                reinterpret_cast<const char*>(data[i]);
        }
        for (int32_t i = 0; i < num_badges_and_items; ++i) {
            data[i] = reinterpret_cast<uint32_t>(
                itemData[shuffled_ids[i]].menu_description);
        }
        for (int32_t i = 0; i < num_badges_and_items; ++i) {
            itemData[ids[i]].menu_description = 
                reinterpret_cast<const char*>(data[i]);
        }
        
        // Shuffle icons.
        memcpy(shuffled_ids, ids, sizeof(ids));
        KnuthShuffle(shuffled_ids, num_badges_and_items);
        for (int32_t i = 0; i < num_badges_and_items; ++i) {
            data[i] = itemData[shuffled_ids[i]].icon_id;
        }
        for (int32_t i = 0; i < num_badges_and_items; ++i) {
            itemData[ids[i]].icon_id = data[i];
        }
        
        // Shuffle sort order, separately for items and badges.
        memcpy(shuffled_ids, ids, sizeof(ids));
        KnuthShuffle(shuffled_ids, num_items);
        KnuthShuffle(shuffled_ids + num_items, num_badges);
        for (int32_t i = 0; i < num_badges_and_items; ++i) {
            data[i] = itemData[shuffled_ids[i]].type_sort_order;
        }
        for (int32_t i = 0; i < num_badges_and_items; ++i) {
            itemData[ids[i]].type_sort_order = data[i];
        }
    } else {
        memcpy(shuffled_ids, ids, sizeof(ids));
        
        // Unshuffle sort order, separately for items and badges.
        KnuthUnshuffle(shuffled_ids + num_items, num_badges);
        KnuthUnshuffle(shuffled_ids, num_items);
        for (int32_t i = 0; i < num_badges_and_items; ++i) {
            data[i] = itemData[shuffled_ids[i]].type_sort_order;
        }
        for (int32_t i = 0; i < num_badges_and_items; ++i) {
            itemData[ids[i]].type_sort_order = data[i];
        }
        
        // Unshuffle icons.
        memcpy(shuffled_ids, ids, sizeof(ids));
        KnuthUnshuffle(shuffled_ids, num_badges_and_items);
        for (int32_t i = 0; i < num_badges_and_items; ++i) {
            data[i] = itemData[shuffled_ids[i]].icon_id;
        }
        for (int32_t i = 0; i < num_badges_and_items; ++i) {
            itemData[ids[i]].icon_id = data[i];
        }
        
        // Unshuffle description & menu description (same order for both).
        memcpy(shuffled_ids, ids, sizeof(ids));
        KnuthUnshuffle(shuffled_ids, num_badges_and_items);
        for (int32_t i = 0; i < num_badges_and_items; ++i) {
            data[i] = reinterpret_cast<uint32_t>(
                itemData[shuffled_ids[i]].description);
        }
        for (int32_t i = 0; i < num_badges_and_items; ++i) {
            itemData[ids[i]].description = 
                reinterpret_cast<const char*>(data[i]);
        }
        for (int32_t i = 0; i < num_badges_and_items; ++i) {
            data[i] = reinterpret_cast<uint32_t>(
                itemData[shuffled_ids[i]].menu_description);
        }
        for (int32_t i = 0; i < num_badges_and_items; ++i) {
            itemData[ids[i]].menu_description = 
                reinterpret_cast<const char*>(data[i]);
        }
        
        // Unshuffle names.
        memcpy(shuffled_ids, ids, sizeof(ids));
        KnuthUnshuffle(shuffled_ids, num_badges_and_items);
        for (int32_t i = 0; i < num_badges_and_items; ++i) {
            data[i] = reinterpret_cast<uint32_t>(
                itemData[shuffled_ids[i]].name);
        }
        for (int32_t i = 0; i < num_badges_and_items; ++i) {
            itemData[ids[i]].name = reinterpret_cast<const char*>(data[i]);
        }
    }
}

bool IsStackableMarioBadge(int32_t item_type) {
    if (item_type < ItemType::POWER_JUMP || item_type >= ItemType::MAX_ITEM_TYPE)
        return false;
    // Partner badges are always one index ahead of their Mario equivalent.
    return IsStackablePartnerBadge(item_type + 1);
}

bool IsStackablePartnerBadge(int32_t item_type) {
    if (item_type < ItemType::POWER_JUMP || item_type >= ItemType::MAX_ITEM_TYPE)
        return false;
    return CheckBitfieldBit(kStackableBadges, item_type - ItemType::POWER_JUMP)
        && !CheckBitfieldBit(kStackableBadgesNoP, item_type - ItemType::POWER_JUMP);
}

}