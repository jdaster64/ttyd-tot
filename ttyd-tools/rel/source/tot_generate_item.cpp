#include "tot_generate_item.h"

#include "common_functions.h"
#include "evt_cmd.h"
#include "mod.h"
#include "tot_state.h"

#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/system.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::tot {

namespace {

using ::ttyd::evtmgr_cmd::evtSetValue;

namespace ItemType = ::ttyd::item_data::ItemType;

// Bitfields of whether each item is included in its respective pool or not;
// item X is enabled if kItemPool[X / 16 - offset] & (1 << (X % 16)) != 0.
static constexpr const uint16_t kCommonItems[] = {
    0xffdf, 0xffff, 0x214f, 0x0006, 0x0000, 0x0000, 0x0000
};
static constexpr const uint16_t kRareItems[] = {
    0x0020, 0x0000, 0x0000, 0x88b8, 0x5e60, 0x0333, 0x02fd
};
static constexpr const uint16_t kSignatureItems[] = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x001e, 0x0400, 0x0000
};
static constexpr const uint16_t kStackableBadges[] = {
    0x0000, 0xffff, 0x0e3f, 0xfff7, 0x018f, 0xf000, 0x0007
};
static constexpr const uint16_t kStackableBadgesNoP[] = {
    0x0000, 0x5555, 0x0a15, 0xaad7, 0x0186, 0xd000, 0x0002
};
static constexpr const uint16_t kChestRewardBadges[] = {
    0xc000, 0x0000, 0xf000, 0x0008, 0x0010, 0x0041, 0x0000
};

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

template <class T> inline void KnuthShuffle(T* arr, int32_t size, int32_t rng) {
    auto& state = g_Mod->state_;
    for (int32_t i = size-1; i > 0; --i) {
        int32_t j = state.Rand(i+1, rng);
        T temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

template <class T> inline void KnuthUnshuffle(T* arr, int32_t size, int32_t rng) {
    auto& state = g_Mod->state_;
    for (int32_t i = 1; i < size; ++i) {
        --state.rng_states_[rng];
        int32_t j = state.Rand(i+1, rng);
        --state.rng_states_[rng];
        T temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

}

int32_t PickRandomItem(
    int32_t sequence, int32_t normal_item_weight, int32_t recipe_item_weight,
    int32_t badge_weight, int32_t no_item_weight) {
    auto& state = g_Mod->state_;
    
    int32_t total_weight =
        normal_item_weight + recipe_item_weight + badge_weight + no_item_weight;
    int32_t result; 
    result = state.Rand(total_weight, sequence);
    const uint16_t* bitfield;
    int32_t len_bitfield;
    int32_t offset;
    
    int32_t current_weight = normal_item_weight;
    if (result < current_weight) {
        bitfield = kCommonItems;
        len_bitfield = sizeof(kCommonItems) / sizeof(uint16_t);
        offset = 0x80;
    } else {
        current_weight += recipe_item_weight;
        if (result < current_weight) {
            bitfield = kRareItems;
            len_bitfield = sizeof(kRareItems) / sizeof(uint16_t);
            offset = 0x80;
        } else {
            current_weight += badge_weight;
            if (result < current_weight) {
                int32_t num_partners = GetNumActivePartners();
                bitfield = num_partners ? kStackableBadges : kStackableBadgesNoP;
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

int32_t BuyPriceComparator(int16_t* lhs, int16_t* rhs) {
    auto* itemData = ttyd::item_data::itemDataTable;
    return itemData[*lhs].buy_price - itemData[*rhs].buy_price;
}

int32_t TypeSortOrderComparator(int16_t* lhs, int16_t* rhs) {
    auto* itemData = ttyd::item_data::itemDataTable;
    const int32_t left_sort =
        itemData[*lhs].type_sort_order + (*lhs < ItemType::POWER_JUMP ? 0 : 200);
    const int32_t right_sort =
        itemData[*rhs].type_sort_order + (*rhs < ItemType::POWER_JUMP ? 0 : 200);
    return left_sort - right_sort;
}

int16_t g_CharlietonInventory[20 + 1] = { -1 };

int16_t* GetCharlietonInventoryPtr() {
    return g_CharlietonInventory;
}

int32_t GetBuyPriceScale() {
    // 20% for first shop, 30% for second, etc.
    int32_t shop_index = (g_Mod->state_.floor_ + 1) / 8;
    return (shop_index + 1) * 10;
}

void ObfuscateItems(bool enable) {
    // Obfuscation has already been performed; no need to redo.
    if (enable && g_Mod->state_.rng_states_[RNG_ITEM_OBFUSCATION])
        return;
    
    auto* itemData = ttyd::item_data::itemDataTable;
    uint16_t ids[180];
    
    uint16_t* ptr = ids;
    ptr = PopulateFromBitfield(
        ptr, kCommonItems, sizeof(kCommonItems) / sizeof(uint16_t), 0x80);
    ptr = PopulateFromBitfield(
        ptr, kRareItems, sizeof(kRareItems) / sizeof(uint16_t), 0x80);
    ptr = PopulateFromBitfield(
        ptr, kSignatureItems, sizeof(kSignatureItems) / sizeof(uint16_t), 0x80);
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
        KnuthShuffle(shuffled_ids, num_badges_and_items, RNG_ITEM_OBFUSCATION);
        for (int32_t i = 0; i < num_badges_and_items; ++i) {
            data[i] = reinterpret_cast<uint32_t>(
                itemData[shuffled_ids[i]].name);
        }
        for (int32_t i = 0; i < num_badges_and_items; ++i) {
            itemData[ids[i]].name = reinterpret_cast<const char*>(data[i]);
        }
        
        // Shuffle description & menu description (same order for both).
        memcpy(shuffled_ids, ids, sizeof(ids));
        KnuthShuffle(shuffled_ids, num_badges_and_items, RNG_ITEM_OBFUSCATION);
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
        KnuthShuffle(shuffled_ids, num_badges_and_items, RNG_ITEM_OBFUSCATION);
        for (int32_t i = 0; i < num_badges_and_items; ++i) {
            data[i] = itemData[shuffled_ids[i]].icon_id;
        }
        for (int32_t i = 0; i < num_badges_and_items; ++i) {
            itemData[ids[i]].icon_id = data[i];
        }
        
        // Shuffle sort order, separately for items and badges.
        memcpy(shuffled_ids, ids, sizeof(ids));
        KnuthShuffle(shuffled_ids, num_items, RNG_ITEM_OBFUSCATION);
        KnuthShuffle(shuffled_ids + num_items, num_badges, RNG_ITEM_OBFUSCATION);
        for (int32_t i = 0; i < num_badges_and_items; ++i) {
            data[i] = itemData[shuffled_ids[i]].type_sort_order;
        }
        for (int32_t i = 0; i < num_badges_and_items; ++i) {
            itemData[ids[i]].type_sort_order = data[i];
        }
    } else {
        memcpy(shuffled_ids, ids, sizeof(ids));
        
        // Unshuffle sort order, separately for items and badges.
        KnuthUnshuffle(shuffled_ids + num_items, num_badges, RNG_ITEM_OBFUSCATION);
        KnuthUnshuffle(shuffled_ids, num_items, RNG_ITEM_OBFUSCATION);
        for (int32_t i = 0; i < num_badges_and_items; ++i) {
            data[i] = itemData[shuffled_ids[i]].type_sort_order;
        }
        for (int32_t i = 0; i < num_badges_and_items; ++i) {
            itemData[ids[i]].type_sort_order = data[i];
        }
        
        // Unshuffle icons.
        memcpy(shuffled_ids, ids, sizeof(ids));
        KnuthUnshuffle(shuffled_ids, num_badges_and_items, RNG_ITEM_OBFUSCATION);
        for (int32_t i = 0; i < num_badges_and_items; ++i) {
            data[i] = itemData[shuffled_ids[i]].icon_id;
        }
        for (int32_t i = 0; i < num_badges_and_items; ++i) {
            itemData[ids[i]].icon_id = data[i];
        }
        
        // Unshuffle description & menu description (same order for both).
        memcpy(shuffled_ids, ids, sizeof(ids));
        KnuthUnshuffle(shuffled_ids, num_badges_and_items, RNG_ITEM_OBFUSCATION);
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
        KnuthUnshuffle(shuffled_ids, num_badges_and_items, RNG_ITEM_OBFUSCATION);
        for (int32_t i = 0; i < num_badges_and_items; ++i) {
            data[i] = reinterpret_cast<uint32_t>(
                itemData[shuffled_ids[i]].name);
        }
        for (int32_t i = 0; i < num_badges_and_items; ++i) {
            itemData[ids[i]].name = reinterpret_cast<const char*>(data[i]);
        }
    }
}

void GenerateHubShopItems() {
    auto& state = g_Mod->state_;

    const int32_t kMaxItem = ItemType::MAX_ITEM_TYPE - ItemType::THUNDER_BOLT;

    // Do unbiased shuffle of array of all possible item types.
    int16_t items[ItemType::MAX_ITEM_TYPE - ItemType::THUNDER_BOLT];
    for (int32_t i = 0; i < kMaxItem; ++i) items[i] = i;
    KnuthShuffle(items, sizeof(items) / sizeof(int16_t), RNG_VANILLA);

    // Pick the first five items that are valid & encountered but not purchased.
    int32_t num_chosen = 0;
    for (int32_t i = 0; i < kMaxItem; ++i) {
        int32_t id = items[i];
        if (ttyd::item_data::itemDataTable[id + ItemType::THUNDER_BOLT]
                .type_sort_order >= 0 &&
            state.GetOption(FLAGS_ITEM_ENCOUNTERED, id) &&
            !state.GetOption(FLAGS_ITEM_PURCHASED, id)) {
            state.SetOption(STAT_PERM_SHOP_ITEMS, id, num_chosen);
            if (++num_chosen == 5) break;
        }
    }
    // Fill the rest with a sentinel.
    for (; num_chosen < 5; ++num_chosen) {
        state.SetOption(STAT_PERM_SHOP_ITEMS, 255, num_chosen);
    }

    state.SetOption(OPT_SHOP_ITEMS_CHOSEN, 1);
}

EVT_DEFINE_USER_FUNC(evtTot_GetUniqueItemName) {
    static int32_t id = 0;
    static char name[16];
    
    id = (id + 1) % 1000;
    sprintf(name, "item_t%03" PRId32, id);
    evtSetValue(evt, evt->evtArguments[0], PTR(name));
    return 2;
}

}  // namespace mod::tot