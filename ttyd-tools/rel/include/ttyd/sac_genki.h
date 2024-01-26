#pragma once

#include "evtmgr.h"

#include <cstdint>

namespace ttyd::sac_genki {

extern "C" {

// recover_cmd_disable
// star_stone_appear
// star_appear
// status_on
EVT_DECLARE_USER_FUNC(get_score, 3)
// wait_game_end
// start_game
// object_get_num
// object_entry
// weapon_entry
// get_ptr
// disp_3D_alpha
// disp_3D
// disp_2D
EVT_DECLARE_USER_FUNC(end_genki, 0)
// main_star1
// main_star0
// main_star
// main_object
// main_weapon
// main_target
// main_mario
// main_base
EVT_DECLARE_USER_FUNC(main_genki, 2)

// .data
extern int32_t genki_evt_common[1];
extern int32_t status_recover_evt[1];

}

}