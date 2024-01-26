#pragma once

#include "battle_database_common.h"

#include <cstdint>

namespace ttyd::battle_unit {
struct BattleWorkUnit;
struct BattleWorkUnitPart;
}

namespace ttyd::sac_deka {

extern "C" {
    
// get_result
// star_stone_attack
// get_ride_pos
// star_stone_appear
// wait_game_end
// start_game
// wait_star_stone_take_on
uint32_t weaponGetPower_Deka(
    battle_unit::BattleWorkUnit* unit1,
    battle_database_common::BattleWeapon* weapon,
    battle_unit::BattleWorkUnit* unit2, battle_unit::BattleWorkUnitPart* part);
// create_timing
// get_ptr
// yuka_disp
// yuka_capture
// disp_3D_alpha
// disp_3D
// disp_2D
// end_deka
// main_enemy_sub
// main_enemy
// main_star
// main_timing
// main_gauge
// main_base
// yuka_end
// yuka_main
// yuka_init
// main_deka

}

}