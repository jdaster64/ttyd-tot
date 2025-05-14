#pragma once

#include "battle_database_common.h"

#include <cstdint>

namespace ttyd::battle {
struct BattleWorkCommand;
}

namespace ttyd::unit_party_vivian {

extern "C" {

// .text
EVT_DECLARE_USER_FUNC(_disp_heart_entry_stop_check, 1)
EVT_DECLARE_USER_FUNC(_disp_heart_entry_stop, 1)
// _disp_heart
EVT_DECLARE_USER_FUNC(_disp_heart_entry, 5)
EVT_DECLARE_USER_FUNC(_make_kagenuke_weapon, 2)
EVT_DECLARE_USER_FUNC(unk_80182cc4, 4)
EVT_DECLARE_USER_FUNC(_get_move_frame, 8)
void __makeTechMenuFunc_vivian(
    battle::BattleWorkCommand* cmd, int32_t* num_options);
EVT_DECLARE_USER_FUNC(_vivian_make_extra_work_area, 0)
EVT_DECLARE_USER_FUNC(battle_evt_majo_disp_off, 4)
EVT_DECLARE_USER_FUNC(battle_evt_majo_disp_on, 6)

// .data
// defence
// defence_attr
// regist
extern battle_database_common::PoseTableEntry pose_table_vivian_stay[24];
extern battle_database_common::DataTableEntry data_table_Party_Vivian[20];
// unitpartsdata_Party_Vivian
extern battle_database_common::BattleUnitKind unitdata_Party_Vivian;
// battle_entry_event
// init_event
// damage_event
// vivian_hide_event
// attack_event
// wait_event
extern battle_database_common::BattleWeapon partyWeapon_VivianNormalAttack;
extern battle_database_common::BattleWeapon partyWeapon_VivianShadowGuard;
extern battle_database_common::BattleWeapon partyWeapon_VivianMagicalPowder;
extern battle_database_common::BattleWeapon partyWeapon_VivianCharmKissAttack;
// partyVivianAttack_NormalAttack
// partyVivianAttack_ShadowGuard
// partyVivianAttack_MagicalPowder
// partyVivianAttack_CharmKissAttack
extern int32_t vivian_shadow_tail_event[1];
// vivian_counter_damage_event
// vivian_appeal_event
// party_win_reaction
// attack_audience_event

}

}