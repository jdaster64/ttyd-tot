#pragma once

#include <cstdint>

#include "evtmgr.h"

namespace ttyd::evt_shop {

extern "C" {

// evtShopIsActive
// evt_shop_main_func
// unkeep_pouchcheck_func
// keep_pouchcheck_func
// item_data_db_restore
// item_data_db_arrange
EVT_DECLARE_USER_FUNC(sell_pouchcheck_func, 0)
EVT_DECLARE_USER_FUNC(name_price, 3)
// get_fook_evt
// get_ptr
// set_buy_item_id
// get_buy_evt
// get_msg
// get_value
// disp_off
// shopper_name
// shop_point_item
// exchange_shop_point
// chk_shop_point
// reset_shop_point
// add_shop_point
// get_shop_point
// shop_flag_onoff
// point_wait
// point_disp_onoff
// evt_shop_setup
// disp_list
// list_disp
// title_disp
// point_disp
// help_disp
// help_main
// name_disp

}

}