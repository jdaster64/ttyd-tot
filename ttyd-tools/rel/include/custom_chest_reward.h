#pragma once

#include <cstdint>

namespace mod::infinite_pit {
    
// Picks a reward for a chest, updating the mod's state accordingly.
// Reward is either an item/badge (if the result > 0) or a partner (-1 to -7).
int16_t PickChestReward();

// Picks a partner for a reward. Returns -1 to -7 for partners 1-7.
int16_t PickPartnerReward();

}