#pragma once

#include "tot_move_manager.h"

#include <cstdint>

namespace mod::tot {
    
struct State {
    // Used by MoveManager to track unlocked / selected levels of moves.
    int8_t level_unlocked_[MoveType::MOVE_TYPE_MAX];
    int8_t level_selected_[MoveType::MOVE_TYPE_MAX];
    // Determines maximum item inventory size.
    int8_t num_sack_upgrades;
};

}