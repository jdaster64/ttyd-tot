#pragma once

#include <cstdint>

namespace ttyd::battle_seq_end {

extern "C" {

struct RankupData {
    int16_t     level;
    int16_t     rank;
    const char* mario_menu_msg;
    const char* rankup_msg;
} __attribute__((__packed__));

static_assert(sizeof(RankupData) == 0xc);
    
// _get_rank_up_msg
// _check_guard_koura
// _BattleMarioKouraDelete
// _audience_Whistle_control
// _check_audience_battle_end_exec
// _lvup_select_object_disp
// _keta_get
// _lvup_select_object_init
// _lvup_object_decide_bound
// _lvup_object_select_move
// _lvup_object_noselect_delete
// _lvup_spot_off
// _lvup_spot_move
// _lvup_spot_on
// _lvup_disp_flag_onoff
// _lvup_obj_set_color
// _lvup_select_obj_lvup_param_disp
// _lvup_select_obj_disp_on
// _set_confetti_control_evtid
// _disable_enemy_unit_disp
// _lvup_ap_recover_full
// _nozzle_set_offset
// N__display_select_one_to_upgrade_text
// N__draw_select_one_to_upgrade_text
// N__set_initial_upgrade_cursor_position
// _GetExpIcon_End
// _GetExpIcon_Disp
// _GetExpIcon_Main
// _GetExpIcon_Init
// L__LvupParamHelpMsgDisp
// _LvupParamConfirmDisp
// _MarioExpDisp
// btlseqEnd_DispMain
// btlseqEnd_DispInit
// _ExecAllUnitBattleEndEvent
// btlseqEnd
const char* BattleGetRankNameLabel(int32_t level);
// _get_rank_data

// Rankup data for ranks 0-3 + NULL struct at end.
extern RankupData _rank_up_data[5];

}

}