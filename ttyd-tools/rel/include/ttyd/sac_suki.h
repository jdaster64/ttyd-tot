#pragma once

#include "evtmgr.h"

#include <cstdint>

namespace ttyd::sac_suki {

extern "C" {

// set_tev
// get_img_name
// eff_crystal
// get_screen_point
// exist_map
// flash_update
// aud_set_draw
EVT_DECLARE_USER_FUNC(sac_suki_set_weapon, 2)
// wait_star_stone_attack_end
// star_stone_attack
// star_stone_get_attack_point
// wait_star_stone_up_end
// star_stone_appear
// wait_game_end
// start_game
// get_ptr
// disp_3D_alpha
// disp_3D
// N__disp_2D
// disp_2D
// end_suki
// main_star
// main_button
// main_base
// main_suki

}

}