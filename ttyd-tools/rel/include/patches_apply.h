#pragma once

#include "common_types.h"
#include "patches_battle.h"
#include "patches_battle_seq.h"
#include "patches_core.h"
#include "patches_costume.h"
#include "patches_enemy.h"
#include "patches_enemy_fix.h"
#include "patches_field.h"
#include "patches_item.h"
#include "patches_mario_move.h"
#include "patches_misc_fix.h"
#include "patches_options.h"
#include "patches_partner.h"
#include "patches_stats.h"
#include "patches_ui.h"

namespace mod::tot::patch {

// Applies all patches that only need to be applied once at initialization.
inline void ApplyAllFixedPatches() {
    battle::ApplyFixedPatches();
    battle_seq::ApplyFixedPatches();
    core::ApplyFixedPatches();
    costume::ApplyFixedPatches();
    enemy::ApplyFixedPatches();
    enemy_fix::ApplyFixedPatches();
    field::ApplyFixedPatches();
    item::ApplyFixedPatches();
    mario_move::ApplyFixedPatches();
    misc_fix::ApplyFixedPatches();
    options::ApplyFixedPatches();
    partner::ApplyFixedPatches();
    stats::ApplyFixedPatches();
    ui::ApplyFixedPatches();
}

}