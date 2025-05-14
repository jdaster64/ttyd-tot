#pragma once

#include "battle_database_common.h"
#include "evtmgr.h"

#include <cstdint>

namespace ttyd::battle {
struct BattleWorkCommand;
}

namespace ttyd::unit_party_christine {

extern "C" {

// .text
EVT_DECLARE_USER_FUNC(_set_hustle, 1)
EVT_DECLARE_USER_FUNC(_dictionary, 2)
EVT_DECLARE_USER_FUNC(_monosiri_flag_on, 1)
EVT_DECLARE_USER_FUNC(btlevtcmd_get_monosiri_msg_no, 3)
void __makeTechMenuFunc_christine(
        battle::BattleWorkCommand* cmd, int32_t* num_options);
EVT_DECLARE_USER_FUNC(krb_get_dir, 5)

// .data
// defence
// defence_attr
// regist
extern battle_database_common::PoseTableEntry pose_table_christine_stay[25];
// data_table
// unitpartsdata_Party_Christine
extern battle_database_common::BattleUnitKind unitdata_Party_Christine;
// battle_entry_event
// init_event
// damage_event
// attack_event
// wait_event
extern battle_database_common::BattleWeapon partyWeapon_ChristineNormalAttack;
extern battle_database_common::BattleWeapon partyWeapon_ChristineMonosiri;
extern battle_database_common::BattleWeapon partyWeapon_ChristineRenzokuAttack;
extern battle_database_common::BattleWeapon partyWeapon_ChristineKiss;
// partyChristineAttack_NormalAttack
extern int32_t christine_dictionary_event[1];
// partyChristineAttack_Monosiri
// partyChristineAttack_RenzokuAttack
// partyChristineAttack_Kiss
// christine_appeal_event
// party_win_reaction
// attack_audience_event

}

}