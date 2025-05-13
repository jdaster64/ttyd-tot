#pragma once

#include "battle_database_common.h"
#include "evtmgr.h"

#include <cstdint>

namespace ttyd::unit_mario {

extern "C" {

// .text
EVT_DECLARE_USER_FUNC(unit_mario__get_mario_hammer_lv, 1)
EVT_DECLARE_USER_FUNC(_mario_super_emblem_anim_set, 1)
EVT_DECLARE_USER_FUNC(_mario_makkuro_set, 0)
void faker_mario_makkuro_set();
// unit_mario_callback
// mario_pinch_pose_sound_callback
EVT_DECLARE_USER_FUNC(_get_local_frame, 1)

// .data
// unit_mario_defence
// unit_mario_defence_attr
// unit_mario_regist
extern ttyd::battle_database_common::PoseTableEntry pose_table_mario_stay[34];
extern ttyd::battle_database_common::PoseTableEntry pose_table_object[3];
extern ttyd::battle_database_common::DataTableEntry data_table_mario[10];
// pose_sound_timing_table
extern ttyd::battle_database_common::BattleUnitKind unitdata_Mario;
extern ttyd::battle_database_common::BattleUnitKindPart unitpartsdata_Mario[3];
extern int32_t unit_mario_init_event[1];
extern int32_t unit_mario_damage_event[1];
extern int32_t unit_mario_attack_event[1];
extern int32_t unit_mario_wait_event[1];
extern int32_t unit_mario_battle_entry_event[1];
extern int32_t mario_ride_btl_entry_event[1];
extern int32_t mario_dead_event[1];
extern int32_t mario_emergency_revival_event[1];
extern int32_t mario_appeal_event[1];
extern int32_t unit_mario_attack_audience_event[1];
extern int32_t btlataudevtMarioJump[1];
extern int32_t btlataudevtMarioHammer[1];

}

}