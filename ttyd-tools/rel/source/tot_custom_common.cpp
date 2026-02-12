#include "evt_cmd.h"

#include <gc/types.h>
#include <ttyd/battle.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_sub.h>
#include <ttyd/battle_unit.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/system.h>

#include <cstdint>

namespace mod::tot::custom {

namespace {

// For convenience.
using namespace ::mod::tot::custom;
using namespace ::ttyd::battle_database_common;
using namespace ::ttyd::battle_unit;

using ::ttyd::battle::g_BattleWork;
using ::ttyd::battle::BattleWork;
using ::ttyd::evtmgr::EvtEntry;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

void SelectMultihitTargets_common(EvtEntry* evt, int32_t num_to_assign) {
    int32_t units[32];
    int32_t parts[32];
    int32_t num_targets = 0;
  
    auto& tw = g_BattleWork->weapon_targets_work;
    
    tw.current_target = 0;
    auto& target_info = tw.targets[tw.target_indices[tw.current_target]];
    units[num_targets] = target_info.unit_idx;
    parts[num_targets] = target_info.part_idx;
    
    while (units[num_targets] != -1) {
        ++num_targets;
        if (++tw.current_target < tw.num_targets) {
            auto& target_info = tw.targets[tw.target_indices[tw.current_target]];
            units[num_targets] = target_info.unit_idx;
            parts[num_targets] = target_info.part_idx;
        } else {
            units[num_targets] = -1;
            parts[num_targets] = 0;
        }
    }
    
    for (int32_t i = 0; i < num_to_assign * 2; i += 2) {
        int32_t target = ttyd::system::irand(num_targets);
        evtSetValue(evt, evt->evtArguments[i + 0], units[target]);
        evtSetValue(evt, evt->evtArguments[i + 1], parts[target]);
    }
}

}

EVT_DEFINE_USER_FUNC(evtTot_CheckNumEnemiesRemaining) {
    int32_t num_enemies = 0;
    bool is_midboss = false;
    for (int32_t i = 0; i < 64; ++i) {
        BattleWorkUnit* unit = g_BattleWork->battle_units[i];
        // Count enemies of either alliance that are still alive.
        if (unit && unit->current_kind <= BattleUnitType::BONETAIL &&
            unit->alliance <= 1 && !BtlUnit_CheckStatus(unit, 27)) {
            ++num_enemies;
            if (unit->status_flags & BattleUnitStatus_Flags::MIDBOSS) {
                is_midboss = true;
            }
        }
    }
    // If there is a midboss, force enemies to use their AI for 2+ enemies.
    if (is_midboss && num_enemies < 2) num_enemies = 2;
    evtSetValue(evt, evt->evtArguments[0], num_enemies);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_SelectMultihitTargetsX2) {
    SelectMultihitTargets_common(evt, 2);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_SelectMultihitTargetsX3) {
    SelectMultihitTargets_common(evt, 3);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_SelectMultihitTargetsX4) {
    SelectMultihitTargets_common(evt, 4);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_SelectMultihitTargetsX5) {
    SelectMultihitTargets_common(evt, 5);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_SelectMultihitTargetsX6) {
    SelectMultihitTargets_common(evt, 6);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_GetPercentOfMaxHP) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    int32_t id = evtGetValue(evt, evt->evtArguments[0]);
    id = ttyd::battle_sub::BattleTransID(evt, id);
    auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, id);
    evtSetValue(
        evt, evt->evtArguments[1], unit->current_hp * 100 / unit->max_hp);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_CheckSpeciesIsEnemy) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    int32_t id = evtGetValue(evt, evt->evtArguments[0]);
    id = ttyd::battle_sub::BattleTransID(evt, id);
    auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, id);
    evtSetValue(evt, evt->evtArguments[1], unit->true_kind <= BattleUnitType::BONETAIL);
    return 2;
}

}