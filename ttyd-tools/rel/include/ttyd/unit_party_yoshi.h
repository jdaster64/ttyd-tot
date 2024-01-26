#pragma once

#include "battle_database_common.h"

#include <cstdint>

namespace ttyd::battle {
struct BattleWorkCommand;
}

namespace ttyd::unit_party_yoshi {

extern "C" {

// .text
// _btlYoshiDisp
EVT_DECLARE_USER_FUNC(_gundan_yoshi_run_effect, 1)
EVT_DECLARE_USER_FUNC(_wait_yoshig_complete, 1)
EVT_DECLARE_USER_FUNC(_wait_yoshig_run, 1)
EVT_DECLARE_USER_FUNC(_check_swallow_attribute, 3)
EVT_DECLARE_USER_FUNC(_get_swallow_param, 2)
EVT_DECLARE_USER_FUNC(_get_nomikomi_hit_position, 6)
// _yoshi_slide_move_sound
EVT_DECLARE_USER_FUNC(btl_yoshi_yoroyoro_jump_move, 6)
EVT_DECLARE_USER_FUNC(btl_yoshi_yoroyoro_jump_calc_param, 6)
void __makeTechMenuFunc_yoshi(
    battle::BattleWorkCommand* cmd, int32_t* num_options);
EVT_DECLARE_USER_FUNC(yoshi_original_color_anim_set, 1)

// .data
// defence
// defence_attr
// regist
// pose_table_yoshi_stay
extern uint32_t pose_table_egg_g[1];
extern uint32_t pose_table_egg_y[1];
extern uint32_t pose_table_egg_p[1];
// data_table
// unitpartsdata_Party_Yoshi
// unitdata_Party_Yoshi
// battle_entry_event
// init_event
// damage_event
// attack_event
// wait_event
// _yoshig_tpl_no_tbl
extern battle_database_common::BattleWeapon partyWeapon_YoshiNormalAttack;
extern battle_database_common::BattleWeapon partyWeapon_YoshiNomikomi_Shot;
extern battle_database_common::BattleWeapon partyWeapon_YoshiNomikomi_Spew;
extern battle_database_common::BattleWeapon partyWeapon_YoshiNomikomi_Dmg0;
extern battle_database_common::BattleWeapon partyWeapon_YoshiNomikomi_Fire;
extern battle_database_common::BattleWeapon partyWeapon_YoshiNomikomi_Involved;
extern battle_database_common::BattleWeapon partyWeapon_YoshiEggAttack_Minimini;
extern battle_database_common::BattleWeapon partyWeapon_YoshiCallGuard;
extern uint32_t posesound_normal_attack[1];
// partyYoshiAttack_NormalAttack
// partyYoshiAttack_Nomikomi
// partyYoshiAttack_EggAttack
// _egg_attack_event
// partyYoshiAttack_CallGuard
// yoshi_appeal_event
// party_win_reaction
// attack_audience_event

}

}