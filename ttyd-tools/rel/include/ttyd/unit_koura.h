#pragma once

#include "battle_database_common.h"

#include <cstdint>

namespace ttyd::unit_koura {

extern "C" {

// .data
extern battle_database_common::BattleUnitKind unit_koura;
extern int8_t defence[5];
extern int8_t defence_attr[5];
extern battle_database_common::StatusVulnerability regist;
// pose_table
// pose_table_crack_lv1
// pose_table_crack_lv2
// data_table
extern battle_database_common::BattleUnitKindPart parts[2];
extern int32_t init_event[1];
extern int32_t damage_event[1];
extern int32_t phase_event[1];
extern int32_t koura_open[1];
extern int32_t wait_event[1];
extern int32_t attack_event[1];
extern int32_t damage_core[1];
extern int32_t pose_tbl_reset[1];
extern int32_t dead_event[1];
extern int32_t destroy_event[1];

}

}