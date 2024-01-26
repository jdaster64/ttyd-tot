#pragma once

#include "battle_database_common.h"

#include <gc/types.h>

#include <cstdint>

namespace ttyd::battle_unit {
struct BattleWorkUnit;
}

namespace ttyd::battle_monosiri {
    
struct MonosiriMsgEntry {
    const char*     unit_name;
    const char*     battle_tattle;
    const char*     menu_tattle;
    const char*     model_name;
    const char*     pose_name;
    const char*     location_name;
} __attribute__((__packed__));

static_assert(sizeof(MonosiriMsgEntry) == 0x18);

extern "C" {

// .text
void battleSetUnitMonosiriFlag(int32_t unit_type);
bool battleCheckUnitMonosiriFlag(ttyd::battle_unit::BattleWorkUnit* unit);
MonosiriMsgEntry* battleGetUnitMonosiriPtr(int32_t unit_type);

}

}