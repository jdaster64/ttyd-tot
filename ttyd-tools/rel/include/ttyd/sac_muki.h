#pragma once

#include "battle_database_common.h"
#include "evtmgr.h"

#include <cstdint>

namespace ttyd::sac_muki {

extern "C" {

// .text
// star_stone_appear
// wait_game_end
// start_game
// set_weapon
// get_ptr
// disp_3D_alpha
// disp_3D
// disp_2D
// end_muki
// main_star
// main_icon
// main_point
// main_cursor
// main_base
EVT_DECLARE_USER_FUNC(main_muki, 1);

// .data
extern battle_database_common::BattleWeapon weapon_muki;
extern int32_t marioAttackEvent_Muki[1];
extern float _sac_muki_power_per_arrow;

}

}