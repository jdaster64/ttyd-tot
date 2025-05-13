#pragma once

#include "battle_database_common.h"

#include <cstdint>

namespace ttyd::battle {
struct BattleWorkCommand;
}

namespace ttyd::unit_party_nokotarou {

extern "C" {

// .text
EVT_DECLARE_USER_FUNC(_check_guard_koura, 2)
EVT_DECLARE_USER_FUNC(_tsuranuki_effect_control, 1)
EVT_DECLARE_USER_FUNC(_color_lv_set, 3)
EVT_DECLARE_USER_FUNC(_check_mario_move_count, 1)
void __makeTechMenuFunc_nokotarou(
    battle::BattleWorkCommand* cmd, int32_t* num_options);

// .data
// defence
// defence_attr
// defence_turn
// regist
extern battle_database_common::PoseTableEntry pose_table_nokotarou_stay[26];
extern battle_database_common::PoseTableEntry pose_table_nokotarou_turn[12];
// data_table
// unitpartsdata_Party_Nokotarou
// unitdata_Party_Nokotarou
// battle_entry_event
// init_event
// damage_event
// change_party_spawn_init_event
// dmg_turn_event
// wakeup_event
// attack_event
// wait_event
extern battle_database_common::BattleWeapon partyWeapon_NokotarouFirstAttack;
extern battle_database_common::BattleWeapon partyWeapon_NokotarouKouraAttack;
extern battle_database_common::BattleWeapon partyWeapon_NokotarouSyubibinKoura;
extern battle_database_common::BattleWeapon partyWeapon_NokotarouKouraGuard;
extern battle_database_common::BattleWeapon partyWeapon_NokotarouTsuranukiKoura;
extern int32_t _change_koura_pose[1];
extern int32_t _change_koura_pose_fast[1];
extern int32_t _restore_koura_pose[1];
extern int32_t _koura_rotate_start[1];
extern int32_t _koura_rotate_stop[1];
// partyNokotarouAttack_FirstAttack
// partyNokotarouAttack_NormalAttack
// partyNokotarouAttack_SyubibinKoura
extern ttyd::battle_database_common::BattleUnitSetup entry_koura;
// partyNokotarouAttack_KouraGuard
// partyNokotarouAttack_TsuranukiKoura
// nokotarou_appeal_event
// party_win_reaction
// attack_audience_event
// UNKNOWN_MS_z0x80393a14

}

}