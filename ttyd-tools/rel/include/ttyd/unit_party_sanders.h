#pragma once

#include "battle_database_common.h"

#include <cstdint>

namespace ttyd::battle {
struct BattleWorkCommand;
}

namespace ttyd::unit_party_sanders {

extern "C" {

// .text
EVT_DECLARE_USER_FUNC(_judge_on_stage, 2)
EVT_DECLARE_USER_FUNC(_shot_move, 7)
EVT_DECLARE_USER_FUNC(_make_counterset_weapon, 2)
EVT_DECLARE_USER_FUNC(_get_bomb_hit_position, 6)
void __makeTechMenuFunc_sanders(
    battle::BattleWorkCommand* cmd, int32_t* num_options);
EVT_DECLARE_USER_FUNC(_sanders_make_extra_work_area, 0)

// .data
// defence
// defence_attr
// regist
// pose_table_sanders_stay
// pose_table_counter_sanders
// data_table
// unitpartsdata_Party_Sanders
// unitdata_Party_Sanders
// battle_entry_event
// init_event
// damage_event
// attack_event
// wait_event
extern battle_database_common::BattleWeapon partyWeapon_SandersFirstAttack;
extern battle_database_common::BattleWeapon partyWeapon_SandersNormalAttack;
extern battle_database_common::BattleWeapon partyWeapon_SandersTimeBombSet;
extern battle_database_common::BattleWeapon partyWeapon_SandersCounterSet;
extern battle_database_common::BattleWeapon partyWeapon_SandersSuperBombAttack;
// partySandersAttack_FirstAttack
// partySandersAttack_NormalAttack
// partySandersAttack_SuperBombAttack
// partySandersAttack_CounterSet
battle_database_common::BattleUnitSetup entry_bomzou;
// partySandersAttack_TimeBombSet
// _shot_bomb_event
// sanders_appeal_event
// party_win_reaction
// attack_audience_event
// UNKNOWN_MS_z0x8038b678

}

}