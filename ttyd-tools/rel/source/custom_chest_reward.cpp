#include "custom_chest_reward.h"

#include "common_functions.h"
#include "custom_item.h"
#include "mod.h"
#include "mod_state.h"
#include "patches_mario_move.h"

#include <ttyd/item_data.h>

#include <cstdint>

namespace mod::infinite_pit {

namespace {

namespace ItemType = ::ttyd::item_data::ItemType;

// Rewards corresponding to state.reward_flags_.
static constexpr const int16_t kRewards[] = {
    // Mario / inventory upgrades (items 0 - 4).
    ItemType::STRANGE_SACK, ItemType::SUPER_BOOTS, ItemType::ULTRA_BOOTS,
    ItemType::SUPER_HAMMER, ItemType::ULTRA_HAMMER,
    // Unique badges (items 5 - 16).
    ItemType::CHILL_OUT, ItemType::DOUBLE_DIP, ItemType::DOUBLE_DIP,
    ItemType::DOUBLE_DIP_P, ItemType::DOUBLE_DIP_P, ItemType::FEELING_FINE,
    ItemType::FEELING_FINE_P, ItemType::LUCKY_START, ItemType::QUICK_CHANGE,
    ItemType::RETURN_POSTAGE, ItemType::ZAP_TAP, ItemType::SPIKE_SHIELD,
    // Partners (represented by dummy values); (items 17 - 23).
    // If partners all unlocked at start, these give Shine Sprites instead.
    -1, -2, -3, -4, -5, -6, -7,
    // Star Powers (items 24 - 31, not in reward_flags_).
    ItemType::MAGICAL_MAP, ItemType::DIAMOND_STAR, ItemType::EMERALD_STAR,
    ItemType::GOLD_STAR, ItemType::RUBY_STAR, ItemType::SAPPHIRE_STAR,
    ItemType::GARNET_STAR, ItemType::CRYSTAL_STAR,
};
static_assert(sizeof(kRewards) == 32 * sizeof(int16_t));

// Enum of types of chest rewards, for top-level reward selection.
enum ChestRewardType {
    REWARD_INVENTORY_UPGRADE = 0,
    REWARD_UNIQUE_BADGE,
    REWARD_PARTNER,
    REWARD_STAR_POWER,
    REWARD_RANDOM_BADGE,
    REWARD_SHINE_SPRITE,
    REWARD_TYPE_MAX
};

int16_t PickInventoryUpgrade(StateManager_v2& state) {
    uint16_t weights[5] = { 10, 30, 0, 30, 0 };
    // Make Strange Sack way more likely starting on floor 50.
    if (state.floor_ >= 49) weights[0] = 50;
    for (int32_t i = 0; i < 5; ++i) {
        if (state.reward_flags_ & (1 << i)) {
            weights[i] = 0;
            // Make Ultra Boots/Hammer possible if Super variant is unlocked.
            if (i == 1 || i == 3) weights[i + 1] = 20;
        }
    }
    
    int32_t sum_weights = 0;
    for (const auto& weight : weights) sum_weights += weight;
    
    // Check for Strange Sack using the CHEST rng sequence; since its weight
    // changes based on progression it can't use the INVENTORY_UPGRADE sequence.
    int32_t weight = state.Rand(sum_weights, RNG_CHEST);
    if (weight < weights[0]) {
        state.reward_flags_ |= (1 << 0);
        // Disable getting upgrade rewards until the next reward floor.
        state.SetOption(OPT_ENABLE_UPGRADE_REWARD, 0);
        return kRewards[0];
    }
    
    // Not chosen; pick boots/hammer using the INVENTORY_UPGRADE sequence.
    weights[0] = 0;
    sum_weights = 0;
    for (const auto& weight : weights) sum_weights += weight;
    
    weight = state.Rand(sum_weights, RNG_INVENTORY_UPGRADE);
    int32_t reward_idx = 0;
    for (; (weight -= weights[reward_idx]) >= 0; ++reward_idx);
    
    state.reward_flags_ |= (1 << reward_idx);
    // Disable getting upgrade rewards until the next reward floor.
    state.SetOption(OPT_ENABLE_UPGRADE_REWARD, 0);
    return kRewards[reward_idx];
}

int16_t PickUniqueBadge(StateManager_v2& state) {
    // Second copies of Double Dip slightly less likely, QC + Spike Shield more.
    uint16_t weights[12] = { 10, 10, 5, 10, 5, 10, 10, 10, 15, 10, 10, 20 };
    for (int32_t i = 0; i < 12; ++i) {
        if (state.reward_flags_ & (1 << (i + 5))) weights[i] = 0;
    }
    
    int32_t sum_weights = 0;
    for (const auto& weight : weights) sum_weights += weight;
    
    int32_t weight = state.Rand(sum_weights, RNG_CHEST_BADGE_FIXED);
    int32_t reward_idx = 0;
    for (; (weight -= weights[reward_idx]) >= 0; ++reward_idx);
    reward_idx += 5;
    
    state.reward_flags_ |= (1 << reward_idx);
    return kRewards[reward_idx];
}

int16_t PickStarPower(StateManager_v2& state) {
    uint16_t weights[8];
    for (int32_t i = 0; i < 8; ++i) {
        weights[i] = mario_move::CanUnlockNextLevel(i) ? 10 : 0;
    }
    
    int32_t sum_weights = 0;
    for (const auto& weight : weights) sum_weights += weight;
    
    int32_t weight = state.Rand(sum_weights, RNG_STAR_POWER);
    int32_t reward_idx = 0;
    for (; (weight -= weights[reward_idx]) >= 0; ++reward_idx);
    
    // Don't increase Star Power level here; wait until the item is claimed.
    return kRewards[reward_idx + 24];
}

}

int16_t PickChestReward() {
    StateManager_v2& state = g_Mod->state_;
    const bool partners_enabled  =
        !state.CheckOptionValue(OPTVAL_PARTNERS_NEVER) &&
        state.GetOptionNumericValue(OPT_ENABLE_PARTNER_REWARD);
    const bool upgrades_enabled =
        state.GetOptionNumericValue(OPT_ENABLE_UPGRADE_REWARD);

    uint16_t weights[REWARD_TYPE_MAX];
    for (auto& weight : weights) weight = 0;
    
    if (partners_enabled) {
        if (int32_t num_partners_remaining =
            7 - CountSetBits(GetBitMask(17, 23) & state.reward_flags_);
            num_partners_remaining) {
            if (num_partners_remaining == 7 && state.floor_ == 29 &&
                !state.CheckOptionValue(OPTVAL_PARTNERS_ALL_START)) {
                // Force a partner on floor 30 you don't have any.
                return PickPartnerReward();
            }
            int32_t partner_weight = 10;
            int32_t num_partners = 7 - num_partners_remaining;
            if (state.floor_ > 30) {
                // Decrease average weight, but increase total considerably
                // if behind expected count (2 by floor 50, 3 by 70, etc.)
                partner_weight = 5;
                int32_t expected_partners = (state.floor_ - 9) / 20;
                if (num_partners < expected_partners) {
                    partner_weight +=
                        30 * (expected_partners - num_partners)
                            / num_partners_remaining;
                }
                if (partner_weight > 100) partner_weight = 100;
            }
            weights[REWARD_PARTNER] = partner_weight * num_partners_remaining;
        }
    }
    if (upgrades_enabled) {
        if (int32_t num_upgrades_remaining =
            5 - CountSetBits(GetBitMask(0, 4) & state.reward_flags_);
            num_upgrades_remaining) {
            weights[REWARD_INVENTORY_UPGRADE] = 25 + 15 * num_upgrades_remaining;
        }
    }
    if (int32_t num_unique_badges_remaining =
        12 - CountSetBits(GetBitMask(5, 16) & state.reward_flags_);
        num_unique_badges_remaining) {
        weights[REWARD_UNIQUE_BADGE] = 15 + 5 * num_unique_badges_remaining;
    }
    
    int32_t star_power_levels_left = 0;
    for (int32_t i = 0; i < 8; ++i) {
        star_power_levels_left += 3 - state.GetStarPowerLevel(i);
    }
    if (star_power_levels_left) {
        weights[REWARD_STAR_POWER] = 12 + 4 * star_power_levels_left;
    }
    
    weights[REWARD_RANDOM_BADGE] = state.floor_ < 99 ? 15 : 25;
    weights[REWARD_SHINE_SPRITE] =
        state.floor_ > 30 ? (state.floor_ < 99 ? 25 : 15) : 0;
        
    // Make Shine Sprites and Jump/Hammer/Strange Sack upgrades less frequent
    // if playing with 4-5 or 6-9 chest rewards.
    if (state.GetOptionNumericValue(OPT_CHEST_REWARDS) > 5) {
        weights[REWARD_INVENTORY_UPGRADE] /= 2;
        weights[REWARD_SHINE_SPRITE] /= 2;
    } else if (state.GetOptionNumericValue(OPT_CHEST_REWARDS) > 3) {
        weights[REWARD_INVENTORY_UPGRADE] =
            weights[REWARD_INVENTORY_UPGRADE] * 3/4;
        weights[REWARD_SHINE_SPRITE] = weights[REWARD_SHINE_SPRITE] * 3/4;
    }
    // If playing without partner rewards, reduce the weight for Shine Sprites.
    if (state.CheckOptionValue(OPTVAL_PARTNERS_NEVER)) {
        weights[REWARD_SHINE_SPRITE] /= 2;
    }
    
    int32_t sum_weights = 0;
    for (const auto& weight : weights) sum_weights += weight;
    
    int32_t weight = state.Rand(sum_weights, RNG_CHEST);
    int32_t reward_idx = 0;
    for (; (weight -= weights[reward_idx]) >= 0; ++reward_idx);
    
    switch (reward_idx) {
        case REWARD_INVENTORY_UPGRADE:  return PickInventoryUpgrade(state);
        case REWARD_UNIQUE_BADGE:       return PickUniqueBadge(state);
        case REWARD_STAR_POWER:         return PickStarPower(state);
        case REWARD_PARTNER: {
            const int32_t partner_reward = PickPartnerReward();
            // Replace rewards with Shine Sprites if starting with all partners.
            return state.CheckOptionValue(OPTVAL_PARTNERS_ALL_START)
                ? ItemType::GOLD_BAR_X3 : partner_reward;
        }
        case REWARD_RANDOM_BADGE:
            return PickRandomItem(RNG_CHEST_BADGE_RANDOM, 0, 0, 1, 0);
    }
    // Not one of the other categories, must be a Shine Sprite.
    return ItemType::GOLD_BAR_X3;
}

int16_t PickPartnerReward() {
    StateManager_v2& state = g_Mod->state_;

    uint16_t weights[7] = { 10, 10, 10, 10, 10, 10, 10 };
    for (int32_t i = 0; i < 7; ++i) {
        if (state.reward_flags_ & (1 << (i + 17))) weights[i] = 0;
    }
    
    int32_t sum_weights = 0;
    for (const auto& weight : weights) sum_weights += weight;
    
    int32_t weight = state.Rand(sum_weights, RNG_PARTNER);
    int32_t reward_idx = 0;
    for (; (weight -= weights[reward_idx]) >= 0; ++reward_idx);
    reward_idx += 17;
    
    state.reward_flags_ |= (1 << reward_idx);
    // Disable getting partners until the next reward floor.
    state.SetOption(OPT_ENABLE_PARTNER_REWARD, 0);
    
    int32_t partner = kRewards[reward_idx];
    
    // If a fixed first partner was chosen, reorder so they come first, and all
    // others are shifted back one slot in the order as necessary.
    if (!state.CheckOptionValue(OPTVAL_PARTNERS_ALL_START) &&
        !state.CheckOptionValue(OPTVAL_NONE_FIRST)) {
            
        // Determine which partner id is associated with the one chosen.
        int32_t first_partner = 0;
        switch (state.GetOptionValue(OPT_FIRST_PARTNER)) {
            case OPTVAL_GOOMBELLA_FIRST:    first_partner = 1; break;
            case OPTVAL_KOOPS_FIRST:        first_partner = 2; break;
            case OPTVAL_FLURRIE_FIRST:      first_partner = 5; break;
            case OPTVAL_YOSHI_FIRST:        first_partner = 4; break;
            case OPTVAL_VIVIAN_FIRST:       first_partner = 6; break;
            case OPTVAL_BOBBERY_FIRST:      first_partner = 3; break;
            case OPTVAL_MS_MOWZ_FIRST:      first_partner = 7; break;
        }
        
        // Check the last_partner that was intended to be obtained.
        int32_t last_partner =
            state.GetOptionNumericValue(OPT_LAST_PARTNER_INTENDED);
            
        // If we've passed the point where the selected first partner was
        // meant to be obtained, no more shifting is necessary.
        if (last_partner == first_partner) {
            return partner;
        }
        
        // Otherwise, save the partner intended to be obtained for next time.
        state.SetOption(OPT_LAST_PARTNER_INTENDED, -partner);
        // Return the previous intended partner if one exists, or the fixed
        // first partner if one does not (meaning that this is the first).
        return last_partner ? -last_partner : -first_partner;
    }
    
    return partner;
}

}