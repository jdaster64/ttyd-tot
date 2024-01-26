#pragma once

#include "battle_database_common.h"
#include "evtmgr.h"

#include <cstdint>

namespace ttyd::battle_unit {
struct BattleWorkUnit;
struct BattleWorkUnitPart;
}

namespace ttyd::sac_zubastar {

extern "C" {

// .text
// zubastar_disp2D
// zubastar_disp_bomb
// zubastar_capture
// zubastar_end
// zubastar_init
// zubastar_bomb
// zubastar_main
// zubastar_get_kouten
// zubastar_bunkatu
// zubastar_create_takaku
uint32_t weaponGetPower_ZubaStar(
    battle_unit::BattleWorkUnit* unit1,
    battle_database_common::BattleWeapon* weapon,
    battle_unit::BattleWorkUnit* unit2, battle_unit::BattleWorkUnitPart* part);
// GetZubaStarPtr
// zubaStarDispStar
// main_star
// wait_star_stone_up
// star_stone_appear

// .data
extern battle_database_common::BattleWeapon weapon_zubastar;
extern int32_t nice_event[1];
extern int32_t marioAttackEvent_ZubaStar[1];

}

}