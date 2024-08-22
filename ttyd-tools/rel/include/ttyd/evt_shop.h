#pragma once

#include <cstdint>

#include "evtmgr.h"

namespace ttyd::evt_shop {

struct ShopItem {
    int32_t item_id;
    int16_t trade_price;
    int16_t buy_price;
};
static_assert(sizeof(ShopItem) == 0x8);

struct ShopkeeperData {
    const char*     npc_name;
    void*           smorg_talk_evt;
    void*           pianta_code_check_evt;
    const char*     shop_msg_keys[35];
};
static_assert(sizeof(ShopkeeperData) == 0x98);

struct ShopWork {
    uint16_t        flags;
    int8_t          unk_0x02[2];
    char**          obj_list;
    ShopItem*       buy_list;
    ShopkeeperData* shopkeeper_data;
    ShopItem*       trade_list;
    int16_t         item_flags[6];
    int32_t         item_count;
    evtmgr::EvtEntry* main_evt;
    evtmgr::EvtEntry* buy_evt;
    int32_t         buy_item_idx;
    int8_t          unk_0x30[0x50 - 0x30];
    int32_t         winmgr_entry_1;
    int32_t         winmgr_entry_2;
    int32_t         winmgr_entry_3;
    int8_t          unk_0x5c[0xd0 - 0x5c];
    int32_t         unk_0xd0;
    int32_t         purchased_item_id;
};
static_assert(sizeof(ShopWork) == 0xd8);

extern "C" {

// .text
// evtShopIsActive
EVT_DECLARE_USER_FUNC(evt_shop_main_func, 0)
EVT_DECLARE_USER_FUNC(unkeep_pouchcheck_func, 0)
EVT_DECLARE_USER_FUNC(keep_pouchcheck_func, 0)
EVT_DECLARE_USER_FUNC(item_data_db_restore, 0)
EVT_DECLARE_USER_FUNC(item_data_db_arrange, 0)
EVT_DECLARE_USER_FUNC(sell_pouchcheck_func, 0)
EVT_DECLARE_USER_FUNC(name_price, 3)
EVT_DECLARE_USER_FUNC(get_fook_evt, 1)
EVT_DECLARE_USER_FUNC(_evt_shop_get_ptr, 1)
EVT_DECLARE_USER_FUNC(set_buy_item_id, 1)
EVT_DECLARE_USER_FUNC(get_buy_evt, 1)
EVT_DECLARE_USER_FUNC(_evt_shop_get_msg, 3)
EVT_DECLARE_USER_FUNC(_evt_shop_get_value, 4)
EVT_DECLARE_USER_FUNC(disp_off, 1)
EVT_DECLARE_USER_FUNC(shopper_name, 1)
EVT_DECLARE_USER_FUNC(shop_point_item, 3)
EVT_DECLARE_USER_FUNC(exchange_shop_point, 0)
EVT_DECLARE_USER_FUNC(chk_shop_point, 3)
EVT_DECLARE_USER_FUNC(reset_shop_point, 0)
EVT_DECLARE_USER_FUNC(add_shop_point, 4)
EVT_DECLARE_USER_FUNC(get_shop_point, 1)
EVT_DECLARE_USER_FUNC(shop_flag_onoff, 2)
EVT_DECLARE_USER_FUNC(point_wait, 0)
EVT_DECLARE_USER_FUNC(point_disp_onoff, 1)
EVT_DECLARE_USER_FUNC(evt_shop_setup, 4)
EVT_DECLARE_USER_FUNC(disp_list, 0)
// list_disp
// title_disp
// point_disp
// help_disp
// help_main
// name_disp

// .data
extern int32_t buy_evt[1];
extern int32_t evt_shoplist[1];
extern ShopWork* g_ShopWork;

}

}