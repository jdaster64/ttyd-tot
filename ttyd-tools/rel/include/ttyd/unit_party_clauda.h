#pragma once

#include "battle_database_common.h"

#include <cstdint>

namespace ttyd::battle {
struct BattleWorkCommand;
}

namespace ttyd::unit_party_clauda {

extern "C" {

// .text
EVT_DECLARE_USER_FUNC(_make_kumoguard_weapon, 2)
EVT_DECLARE_USER_FUNC(_get_clauda_kiss_hit_position, 6)
EVT_DECLARE_USER_FUNC(_make_kiss_weapon, 2)
EVT_DECLARE_USER_FUNC(_make_breath_weapon, 2)
EVT_DECLARE_USER_FUNC(_clauda_breath_effect_fire, 1)
EVT_DECLARE_USER_FUNC(_clauda_breath_effect_ready, 0)
EVT_DECLARE_USER_FUNC(_check_blow_rate, 2)
void __makeTechMenuFunc_clauda(
    battle::BattleWorkCommand* cmd, int32_t* num_options);
EVT_DECLARE_USER_FUNC(_clauda_make_extra_work_area, 0)

// .data
extern battle_database_common::BattleWeapon partyWeapon_ClaudaNormalAttack;
extern battle_database_common::BattleWeapon partyWeapon_ClaudaBreathAttack;
extern battle_database_common::BattleWeapon partyWeapon_ClaudaLipLockAttack;
extern battle_database_common::BattleWeapon partyWeapon_ClaudaKumogakureAttack;
// defence
// defence_attr
// regist
extern battle_database_common::PoseTableEntry pose_table_clauda_stay[25];
// data_table
// unitpartsdata_Party_Clauda
// unitdata_Party_Clauda
// battle_entry_event
// init_event
// damage_event
// attack_event
// wait_event
// partyClaudaAttack_NormalAttack
// partyClaudaAttack_BreathAttack
// partyClaudaAttack_LipLockAttack
// partyClaudaAttack_KumoGuard
// clauda_appeal_event
// party_win_reaction
// attack_audience_event
// attack_bad_audience_event
// attack_good_audience_event

}

}