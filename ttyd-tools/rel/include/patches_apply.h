#pragma once

#include "common_types.h"
#include "patches_battle.h"
#include "patches_battle_seq.h"
#include "patches_core.h"
#include "patches_enemy.h"
#include "patches_enemy_fix.h"
#include "patches_field.h"
#include "patches_field_start.h"
#include "patches_item.h"
#include "patches_mario_move.h"
#include "patches_misc_fix.h"
#include "patches_options.h"
#include "patches_partner.h"
#include "patches_stats.h"
#include "patches_ui.h"

namespace mod::infinite_pit {

// Applies all patches that only need to be applied once at initialization.
inline void ApplyAllFixedPatches() {
    battle::ApplyFixedPatches();
    battle_seq::ApplyFixedPatches();
    core::ApplyFixedPatches();
    enemy::ApplyFixedPatches();
    enemy_fix::ApplyFixedPatches();
    field::ApplyFixedPatches();
    field_start::ApplyFixedPatches();
    item::ApplyFixedPatches();
    mario_move::ApplyFixedPatches();
    misc_fix::ApplyFixedPatches();
    options::ApplyFixedPatches();
    partner::ApplyFixedPatches();
    stats::ApplyFixedPatches();
    ui::ApplyFixedPatches();
}

// Applies all patches that need to be applied once per module load.
inline void ApplyAllModuleLevelPatches(void* module_ptr, ModuleId::e module_id) {
    enemy_fix::ApplyModuleLevelPatches(module_ptr, module_id);
    field::ApplyModuleLevelPatches(module_ptr, module_id);
    field_start::ApplyModuleLevelPatches(module_ptr, module_id);
}

// Links / unlinks all custom events that rely on code in TTYD's modules.
// Unlinks if `link` = false.
inline void LinkAllCustomEvts(void* module_ptr, ModuleId::e module_id, bool link) {
    enemy_fix::LinkCustomEvts(module_ptr, module_id, link);
    field::LinkCustomEvts(module_ptr, module_id, link);
}

}