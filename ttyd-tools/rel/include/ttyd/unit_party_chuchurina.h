#pragma once

#include "battle_database_common.h"
#include "evtmgr.h"

#include <cstdint>

namespace ttyd::battle {
struct BattleWorkCommand;
}

namespace ttyd::unit_party_chuchurina {

extern "C" {

// .text
EVT_DECLARE_USER_FUNC(mono_off, 0)
EVT_DECLARE_USER_FUNC(mono_on, 0)
EVT_DECLARE_USER_FUNC(mono_capture_event, 0)
EVT_DECLARE_USER_FUNC(mono_main, 0)
// mono_disp
EVT_DECLARE_USER_FUNC(mono_capture, 0)
EVT_DECLARE_USER_FUNC(_chuchu_item_steal, 3)
EVT_DECLARE_USER_FUNC(_get_itemsteal_param, 3)
EVT_DECLARE_USER_FUNC(_make_madowase_weapon, 2)
EVT_DECLARE_USER_FUNC(_get_binta_hit_position, 6)
void __makeTechMenuFunc_chuchurina(
    battle::BattleWorkCommand* cmd, int32_t* num_options);
EVT_DECLARE_USER_FUNC(_chuchu_make_extra_work_area, 0)

// .data
// defence
// defence_attr
// regist
// pose_table_chuchurina_stay
// data_table
// unitpartsdata_Party_Chuchurina
// unitdata_Party_Chuchurina
// battle_entry_event
// init_event
// damage_event
// attack_event
// wait_event
extern battle_database_common::BattleWeapon partyWeapon_ChuchurinaNormalAttackLeft;
extern battle_database_common::BattleWeapon partyWeapon_ChuchurinaNormalAttackRight;
extern battle_database_common::BattleWeapon partyWeapon_ChuchurinaNormalAttackLeftLast;
extern battle_database_common::BattleWeapon partyWeapon_ChuchurinaNormalAttackRightLast;
extern battle_database_common::BattleWeapon partyWeapon_ChuchurinaMadowaseAttack;
extern battle_database_common::BattleWeapon partyWeapon_ChuchurinaItemSteal;
extern battle_database_common::BattleWeapon partyWeapon_ChuchurinaKiss;
// partyChuchurinaAttack_NormalAttack
extern int32_t main_evt[1];
// partyChuchurinaAttack_MadowaseAttack
// partyChuchurinaAttack_ItemSteal
// partyChuchurinaAttack_Kiss
// chuchurina_appeal_event
// party_win_reaction
// attack_audience_event

}

}