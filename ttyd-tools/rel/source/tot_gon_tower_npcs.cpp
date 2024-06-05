#include "tot_gon_tower_npcs.h"

#include "evt_cmd.h"
#include "mod.h"
#include "tot_generate_item.h"
#include "tot_generate_reward.h"
#include "tot_manager_options.h"
#include "tot_manager_timer.h"
#include "tot_state.h"
#include "tot_window_select.h"

#include <ttyd/evt_badgeshop.h>
#include <ttyd/evt_item.h>
#include <ttyd/evt_mario.h>
#include <ttyd/evt_msg.h>
#include <ttyd/evt_npc.h>
#include <ttyd/evt_pouch.h>
#include <ttyd/evt_shop.h>
#include <ttyd/evt_sub.h>
#include <ttyd/evt_win.h>
#include <ttyd/evt_window.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/item_data.h>
#include <ttyd/npc_data.h>
#include <ttyd/npcdrv.h>
#include <ttyd/system.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::tot::gon {

namespace {

// Including entire namespaces for convenience.
using namespace ::ttyd::evt_item;
using namespace ::ttyd::evt_mario;
using namespace ::ttyd::evt_msg;
using namespace ::ttyd::evt_npc;
using namespace ::ttyd::evt_pouch;
using namespace ::ttyd::evt_shop;
using namespace ::ttyd::evt_sub;
using namespace ::ttyd::evt_win;
using namespace ::ttyd::evt_window;

using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;
using ::ttyd::npc_data::npcTribe;
using ::ttyd::npcdrv::NpcSetupInfo;
using ::ttyd::system::qqsort;

namespace ItemType = ::ttyd::item_data::ItemType;
namespace NpcTribeType = ::ttyd::npc_data::NpcTribeType;

}  // namespace

namespace SecondaryNpcType {
    enum e {
        NONE = -1,
        LUMPY = 0,
        DOOPLISS,
        GRUBBA,
        CHET_RIPPO,
        WONKY,
        DAZZLE,
        
        NUM_NPC_TYPES
    };
}

// Declarations for USER_FUNCs.
EVT_DECLARE_USER_FUNC(evtTot_SelectCharlietonItems, 0)
EVT_DECLARE_USER_FUNC(evtTot_TrackNpcAction, 2)
EVT_DECLARE_USER_FUNC(evtTot_CheckAnyStatsDowngradeable, 1)
EVT_DECLARE_USER_FUNC(evtTot_DowngradeStat, 1)
EVT_DECLARE_USER_FUNC(evtTot_GetChetCost, 1)
EVT_DECLARE_USER_FUNC(evtTot_GetDazzleCost, 1)
EVT_DECLARE_USER_FUNC(evtTot_CheckNpcEffectEnabled, 2)
EVT_DECLARE_USER_FUNC(evtTot_EnableNpcEffect, 1)

// Declarations for NPCs.
extern int32_t g_SecondaryNpcTribeIndices[SecondaryNpcType::NUM_NPC_TYPES];
extern NpcSetupInfo g_SecondaryNpcTemplates[SecondaryNpcType::NUM_NPC_TYPES];
extern NpcSetupInfo g_NpcSetup[3];

// Generic move script that can be shared by all mobile npcs.
EVT_BEGIN(TowerNpc_GenericMove)
    USER_FUNC(evt_npc_flag_onoff, 1, PTR("me"), 0x20410)
    USER_FUNC(evt_npc_status_onoff, 1, PTR("me"), 2)
    // Pick random start direction.
    USER_FUNC(evt_sub_random, 360, LW(3))
LBL(0)
    // Flip around twice.
    USER_FUNC(evt_npc_reverse_ry, PTR("me"))
    WAIT_MSEC(500)
    USER_FUNC(evt_npc_reverse_ry, PTR("me"))
    WAIT_MSEC(1000)
    // Move in a new direction 120-240 degrees different from the last.
    USER_FUNC(evt_npc_get_position, PTR("me"), LW(0), LW(1), LW(2))
    USER_FUNC(evt_npc_get_loiter_dir, LW(3), FLOAT(120.0), FLOAT(240.0))
    USER_FUNC(evt_npc_add_dirdist, LW(0), LW(2), LW(3), FLOAT(100.0))
    USER_FUNC(evt_npc_move_position, PTR("me"), LW(0), LW(2), 2000, FLOAT(40.0), 4)
    GOTO(0)
    RETURN()
EVT_END()

// Generic talk script - TODO: Move all NPCs to their own scripts.
EVT_BEGIN(TowerNpc_GenericTalk)
    USER_FUNC(evt_msg_print, 0, PTR("tot_npc_generic"), 0, PTR("me"))
    RETURN()
EVT_END()

// Init script for Charlieton; generates item table.
EVT_BEGIN(TowerNpc_CharlietonInit)
    USER_FUNC(evtTot_SelectCharlietonItems)
    RETURN()
EVT_END()

// Talk script for Charlieton.
EVT_BEGIN(TowerNpc_CharlietonTalk)
    USER_FUNC(evt_msg_print, 0, PTR("tot_charlieton_intro"), 0, PTR("me"))
    USER_FUNC(evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_charlieton_decline"))
        RETURN()
    END_IF()
    USER_FUNC(evt_msg_continue)
LBL(0)
    USER_FUNC(evt_win_coin_on, 0, LW(12))
    USER_FUNC(evt_win_other_select, 
        (uint32_t)window_select::MenuType::TOT_CHARLIETON_SHOP)
    IF_EQUAL(LW(0), 0)
        USER_FUNC(evt_win_coin_off, LW(12))
        USER_FUNC(evt_msg_print, 0, PTR("tot_charlieton_decline"), 0, PTR("me"))
        RETURN()
    END_IF()
    USER_FUNC(evt_pouch_get_coin, LW(0))
    IF_SMALL(LW(0), LW(3))
        USER_FUNC(evt_win_coin_off, LW(12))
        USER_FUNC(evt_msg_print, 0, PTR("tot_charlieton_nocoins"), 0, PTR("me"))
        GOTO(0)
    END_IF()
    USER_FUNC(evt_msg_fill_num, 0, LW(14), PTR("tot_charlieton_itemdesc"), LW(3))
    USER_FUNC(evt_msg_fill_item, 1, LW(14), LW(14), LW(2))
    USER_FUNC(evt_msg_print, 1, LW(14), 0, PTR("me"))
    USER_FUNC(evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_win_coin_off, LW(12))
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_charlieton_decline"))
        RETURN()
    END_IF()
    IF_EQUAL(LW(1), (int32_t)ItemType::STAR_PIECE)
        // Give item directly.
        GOTO(10)
    END_IF()
    USER_FUNC(evt_pouch_add_item, LW(1), LW(0))
    IF_EQUAL(LW(0), -1)
        // Inventory full; prompt if the player wants to buy the item anyway.
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_charlieton_full_inv"))
        USER_FUNC(evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
        IF_EQUAL(LW(0), 1)  // Declined.
            USER_FUNC(evt_win_coin_off, LW(12))
            USER_FUNC(evt_msg_print_add, 0, PTR("tot_charlieton_decline"))
            RETURN()
        END_IF()
LBL(10)
        MUL(LW(3), -1)
        USER_FUNC(evt_pouch_add_coin, LW(3))
        USER_FUNC(evt_win_coin_wait, LW(12))
        WAIT_MSEC(200)
        USER_FUNC(evt_win_coin_off, LW(12))
        USER_FUNC(evtTot_AfterItemBought, LW(1))
        // Close text dialog and give item directly.
        USER_FUNC(evt_msg_continue)
        USER_FUNC(evtTot_GetUniqueItemName, LW(0))
        USER_FUNC(evt_item_entry, LW(0), LW(1), FLOAT(0.0), FLOAT(-999.0), FLOAT(0.0), 17, -1, 0)
        USER_FUNC(evt_item_get_item, LW(0))
        RETURN()
    END_IF()
    MUL(LW(3), -1)
    USER_FUNC(evt_pouch_add_coin, LW(3))
    USER_FUNC(evt_win_coin_wait, LW(12))
    WAIT_MSEC(200)
    USER_FUNC(evt_win_coin_off, LW(12))
    USER_FUNC(evtTot_AfterItemBought, LW(1))
    USER_FUNC(evt_pouch_get_coin, LW(0))
    IF_EQUAL(LW(0), 0)
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_charlieton_success"))
        RETURN()
    END_IF()
    USER_FUNC(evt_msg_print_add, 0, PTR("tot_charlieton_buyanother"))
    USER_FUNC(evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_charlieton_decline"))
        RETURN()
    END_IF()
    USER_FUNC(evt_msg_continue)
    GOTO(0)
    RETURN()
EVT_END()

// Selling items event.
EVT_BEGIN(TowerNpc_SellItems)
    USER_FUNC(ttyd::evt_shop::sell_pouchcheck_func)
    IF_EQUAL(LW(0), 0)
        USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("tot_wonky_noitems"))
        RETURN()
    END_IF()
    USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("tot_wonky_whichitem"))
    USER_FUNC(ttyd::evt_window::evt_win_coin_on, 0, LW(8))
LBL(0)
    USER_FUNC(ttyd::evt_window::evt_win_item_select, 1, 3, LW(1), LW(4))
    IF_SMALL_EQUAL(LW(1), 0)
        USER_FUNC(ttyd::evt_window::evt_win_coin_off, LW(8))
        USER_FUNC(ttyd::evt_msg::evt_msg_print,
            0, PTR("tot_wonky_exit"), 0, PTR("npc_wonky"))
        RETURN()
    END_IF()
    USER_FUNC(ttyd::evt_shop::name_price, LW(1), LW(2), LW(3))
    USER_FUNC(ttyd::evt_msg::evt_msg_fill_num, 0, LW(14), PTR("tot_wonky_itemok"), LW(3))
    USER_FUNC(ttyd::evt_msg::evt_msg_fill_item, 1, LW(14), LW(14), LW(2))
    USER_FUNC(ttyd::evt_msg::evt_msg_print, 1, LW(14), 0, PTR("npc_wonky"))
    USER_FUNC(ttyd::evt_msg::evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("tot_wonky_itemdifferent"))
        GOTO(0)
    END_IF()
    USER_FUNC(ttyd::evt_pouch::N_evt_pouch_remove_item_index, LW(1), LW(4), LW(0))
    USER_FUNC(ttyd::evt_pouch::evt_pouch_add_coin, LW(3))
    USER_FUNC(evtTot_TrackNpcAction, (int32_t)STAT_RUN_NPC_ITEMS_SOLD, 1)
    USER_FUNC(ttyd::evt_window::evt_win_coin_wait, LW(8))
    WAIT_MSEC(200)
    USER_FUNC(ttyd::evt_window::evt_win_coin_off, LW(8))
    USER_FUNC(ttyd::evt_shop::sell_pouchcheck_func)
    IF_EQUAL(LW(0), 0)
        USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("tot_wonky_thankslast"))
        RETURN()
    END_IF()
    USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("tot_wonky_thanksnext"))
    USER_FUNC(ttyd::evt_msg::evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("tot_wonky_exit"))
        RETURN()
    END_IF()
    USER_FUNC(ttyd::evt_msg::evt_msg_continue)
    USER_FUNC(ttyd::evt_window::evt_win_coin_on, 0, LW(8))
    GOTO(0)
    RETURN()
EVT_END()

// Selling badges event.
EVT_BEGIN(TowerNpc_SellBadges)
    USER_FUNC(ttyd::evt_pouch::evt_pouch_get_havebadgecnt, LW(0))
    IF_EQUAL(LW(0), 0)
        USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("tot_wonky_nobadges"))
        RETURN()
    END_IF()
    USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("tot_wonky_whichitem"))
    USER_FUNC(ttyd::evt_window::evt_win_coin_on, 0, LW(8))
LBL(0)
    USER_FUNC(ttyd::evt_window::evt_win_other_select, 12)
    IF_EQUAL(LW(0), 0)
        USER_FUNC(ttyd::evt_window::evt_win_coin_off, LW(8))
        USER_FUNC(ttyd::evt_msg::evt_msg_print,
            0, PTR("tot_wonky_exit"), 0, PTR("npc_wonky"))
        RETURN()
    END_IF()
    USER_FUNC(ttyd::evt_msg::evt_msg_fill_num, 0, LW(14), PTR("tot_wonky_itemok"), LW(3))
    USER_FUNC(ttyd::evt_msg::evt_msg_fill_item, 1, LW(14), LW(14), LW(2))
    USER_FUNC(ttyd::evt_msg::evt_msg_print, 1, LW(14), 0, PTR("npc_wonky"))
    USER_FUNC(ttyd::evt_msg::evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("tot_wonky_itemdifferent"))
        GOTO(0)
    END_IF()
    USER_FUNC(ttyd::evt_pouch::N_evt_pouch_remove_item_index, LW(1), LW(4), LW(0))
    USER_FUNC(ttyd::evt_pouch::evt_pouch_add_coin, LW(3))
    USER_FUNC(evtTot_TrackNpcAction, (int32_t)STAT_RUN_NPC_BADGES_SOLD, 1)
    USER_FUNC(ttyd::evt_window::evt_win_coin_wait, LW(8))
    WAIT_MSEC(200)
    USER_FUNC(ttyd::evt_window::evt_win_coin_off, LW(8))
    USER_FUNC(ttyd::evt_pouch::evt_pouch_get_havebadgecnt, LW(0))
    IF_EQUAL(LW(0), 0)
        USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("tot_wonky_thankslast"))
        RETURN()
    END_IF()
    USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("tot_wonky_thanksnext"))
    USER_FUNC(ttyd::evt_msg::evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("tot_wonky_exit"))
        RETURN()
    END_IF()
    USER_FUNC(ttyd::evt_msg::evt_msg_continue)
    USER_FUNC(ttyd::evt_window::evt_win_coin_on, 0, LW(8))
    GOTO(0)
    RETURN()
EVT_END()

// Talk script for Wonky.
EVT_BEGIN(TowerNpc_WonkyTalk)
    USER_FUNC(ttyd::evt_mario::evt_mario_key_onoff, 0)
    USER_FUNC(ttyd::evt_win::unitwin_get_work_ptr, LW(10))
    USER_FUNC(ttyd::evt_msg::evt_msg_print,
        0, PTR("tot_wonky_intro"), 0, PTR("npc_wonky"))
    USER_FUNC(ttyd::evt_msg::evt_msg_select, 0, PTR("tot_wonky_topmenu"))
    SWITCH(LW(0))
        CASE_EQUAL(0)
            RUN_CHILD_EVT(TowerNpc_SellItems)
        CASE_EQUAL(1)
            RUN_CHILD_EVT(TowerNpc_SellBadges)
        CASE_EQUAL(2)
            USER_FUNC(ttyd::evt_msg::evt_msg_print_add, 0, PTR("tot_wonky_exit"))
    END_SWITCH()
    USER_FUNC(ttyd::evt_mario::evt_mario_key_onoff, 1)
    RETURN()
EVT_END()

// Talk script for Chet Rippo.
EVT_BEGIN(TowerNpc_ChetRippoTalk)
    USER_FUNC(evt_mario_key_onoff, 0)
    USER_FUNC(evt_msg_print, 0, PTR("tot_chet_intro"), 0, PTR("me"))
    USER_FUNC(evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_chet_decline"))
        GOTO(99)
    END_IF()

    USER_FUNC(evtTot_CheckAnyStatsDowngradeable, LW(0))
    IF_EQUAL(LW(0), 0)
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_chet_nostats"))
        GOTO(99)
    END_IF()

    USER_FUNC(evt_msg_print_add, 0, PTR("tot_chet_whichstat"))
LBL(0)
    USER_FUNC(evt_win_other_select, window_select::MenuType::TOT_CHET_RIPPO_TRADE)
    IF_EQUAL(LW(1), 0)
        USER_FUNC(evt_msg_print, 0, PTR("tot_chet_different"), 0, PTR("me"))
        GOTO(0)
    END_IF()

    USER_FUNC(evt_win_coin_on, 0, LW(8))
    USER_FUNC(evtTot_GetChetCost, LW(5))
    USER_FUNC(
        evt_msg_print_insert, 0, PTR("tot_chet_confirm"), 0, PTR("me"), 
        LW(2), LW(3), LW(4), LW(5))

    USER_FUNC(evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_chet_decline"))
        USER_FUNC(evt_win_coin_off, LW(8))
        GOTO(99)
    END_IF()
    USER_FUNC(evt_pouch_get_coin, LW(0))
    IF_SMALL(LW(0), LW(5))
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_chet_nocoins"))
        USER_FUNC(evt_win_coin_off, LW(8))
        GOTO(99)
    END_IF()

    MUL(LW(5), -1)
    // TODO: Play a sound effect or visual effect?
    USER_FUNC(evt_pouch_add_coin, LW(5))
    USER_FUNC(evtTot_TrackNpcAction, (int32_t)STAT_RUN_NPC_LEVELS_SOLD, 1)
    USER_FUNC(evt_win_coin_wait, LW(8))
    WAIT_MSEC(200)
    USER_FUNC(evt_win_coin_off, LW(8))

    USER_FUNC(evtTot_DowngradeStat, LW(1))
    USER_FUNC(evt_msg_continue)
    USER_FUNC(evtTot_GetUniqueItemName, LW(0))
    USER_FUNC(evt_item_entry, LW(0), ItemType::SHINE_SPRITE, FLOAT(0.0), FLOAT(-999.0), FLOAT(0.0), 17, -1, 0)
    USER_FUNC(evt_item_get_item, LW(0))

LBL(99)
    USER_FUNC(evt_mario_key_onoff, 1)
    RETURN()
EVT_END()

// Talk script for Dazzle.
EVT_BEGIN(TowerNpc_DazzleTalk)
    USER_FUNC(evt_mario_key_onoff, 0)

    USER_FUNC(evtTot_GetDazzleCost, LW(5))
    IF_EQUAL(LW(5), 0)
        USER_FUNC(evt_msg_print, 0, PTR("tot_dazzle_intro"), 0, PTR("me"))
        USER_FUNC(evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
        IF_EQUAL(LW(0), 1)
            USER_FUNC(evt_msg_print_add, 0, PTR("tot_dazzle_decline"))
            GOTO(99)
        END_IF()
        GOTO(10)
    END_IF()

    USER_FUNC(evt_win_coin_on, 0, LW(8))
    USER_FUNC(evt_msg_print_insert, 0, PTR("tot_dazzle_offer"), 0, PTR("me"), LW(5))
    USER_FUNC(evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_dazzle_decline"))
        USER_FUNC(evt_win_coin_off, LW(8))
        GOTO(99)
    END_IF()
    USER_FUNC(evt_pouch_get_coin, LW(0))
    IF_SMALL(LW(0), LW(5))
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_dazzle_nocoins"))
        USER_FUNC(evt_win_coin_off, LW(8))
        GOTO(99)
    END_IF()
    
    MUL(LW(5), -1)
    USER_FUNC(evt_pouch_add_coin, LW(5))
    USER_FUNC(evt_win_coin_wait, LW(8))
    WAIT_MSEC(200)
    USER_FUNC(evt_win_coin_off, LW(8))

LBL(10)
    USER_FUNC(evt_msg_continue)
    USER_FUNC(evtTot_TrackNpcAction, (int32_t)STAT_RUN_NPC_SP_PURCHASED, 1)
    USER_FUNC(evtTot_GetUniqueItemName, LW(0))
    USER_FUNC(evt_item_entry, LW(0), ItemType::STAR_PIECE, FLOAT(0.0), FLOAT(-999.0), FLOAT(0.0), 17, -1, 0)
    USER_FUNC(evt_item_get_item, LW(0))

LBL(99)
    USER_FUNC(evt_mario_key_onoff, 1)
    RETURN()
EVT_END()

// Talk script for Doopliss.
EVT_BEGIN(TowerNpc_DooplissTalk)
    USER_FUNC(evt_mario_key_onoff, 0)
    USER_FUNC(evtTot_CheckNpcEffectEnabled, (int32_t)SecondaryNpcType::DOOPLISS, LW(0))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_msg_print, 0, PTR("tot_doopliss_active"), 0, PTR("me"))
        GOTO(99)
    END_IF()

    USER_FUNC(evt_msg_print, 0, PTR("tot_doopliss_intro"), 0, PTR("me"))
    USER_FUNC(evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_doopliss_decline"))
        GOTO(99)
    END_IF()

    USER_FUNC(evtTot_EnableNpcEffect, (int32_t)SecondaryNpcType::DOOPLISS)
    USER_FUNC(evt_msg_print_add, 0, PTR("tot_doopliss_accept"))

LBL(99)
    USER_FUNC(evt_mario_key_onoff, 1)
    RETURN()
EVT_END()

// Talk script for Grubba.
EVT_BEGIN(TowerNpc_GrubbaTalk)
    USER_FUNC(evt_mario_key_onoff, 0)
    USER_FUNC(evtTot_CheckNpcEffectEnabled, (int32_t)SecondaryNpcType::GRUBBA, LW(0))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_msg_print, 0, PTR("tot_grubba_active"), 0, PTR("me"))
        GOTO(99)
    END_IF()

    USER_FUNC(evt_msg_print, 0, PTR("tot_grubba_intro"), 0, PTR("me"))
    USER_FUNC(evt_msg_select, 0, PTR("tot_npc_yesnoopt"))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_msg_print_add, 0, PTR("tot_grubba_decline"))
        GOTO(99)
    END_IF()

    USER_FUNC(evtTot_EnableNpcEffect, (int32_t)SecondaryNpcType::GRUBBA)
    USER_FUNC(evt_msg_print_add, 0, PTR("tot_grubba_accept"))

LBL(99)
    USER_FUNC(evt_mario_key_onoff, 1)
    RETURN()
EVT_END()

int32_t g_SecondaryNpcTribeIndices[SecondaryNpcType::NUM_NPC_TYPES] = {
    NpcTribeType::LUMPY,
    NpcTribeType::DOOPLISS,
    NpcTribeType::GRUBBA,
    NpcTribeType::CHET_RIPPO,
    NpcTribeType::WONKY,
    NpcTribeType::DAZZLE,
};

NpcSetupInfo g_SecondaryNpcTemplates[SecondaryNpcType::NUM_NPC_TYPES] = {
    {
        .name = "npc_lumpy",
        .flags = 0x1000'0600,
        .initEvtCode = nullptr,
        .regularEvtCode = nullptr,
        .talkEvtCode = (void*)TowerNpc_GenericTalk,
        .battleInfoId = -1,
    },
    {
        .name = "npc_doopliss",
        .flags = 0x1000'0600,
        .initEvtCode = nullptr,
        .regularEvtCode = (void*)TowerNpc_GenericMove,
        .talkEvtCode = (void*)TowerNpc_DooplissTalk,
        .battleInfoId = -1,
    },
    {
        .name = "npc_grubba",
        .flags = 0x1000'0600,
        .initEvtCode = nullptr,
        .regularEvtCode = (void*)TowerNpc_GenericMove,
        .talkEvtCode = (void*)TowerNpc_GrubbaTalk,
        .battleInfoId = -1,
    },
    {
        .name = "npc_chet",
        .flags = 0x1000'0600,
        .initEvtCode = nullptr,
        .regularEvtCode = nullptr,
        .talkEvtCode = (void*)TowerNpc_ChetRippoTalk,
        .battleInfoId = -1,
    },
    {
        .name = "npc_wonky",
        .flags = 0x1000'0600,
        .initEvtCode = nullptr,
        .regularEvtCode = (void*)TowerNpc_GenericMove,
        .talkEvtCode = (void*)TowerNpc_WonkyTalk,
        .battleInfoId = -1,
    },
    {
        .name = "npc_dazzle",
        .flags = 0x1000'0600,
        .initEvtCode = nullptr,
        .regularEvtCode = (void*)TowerNpc_GenericMove,
        .talkEvtCode = (void*)TowerNpc_DazzleTalk,
        .battleInfoId = -1,
    },
};

NpcSetupInfo g_NpcSetup[3] = {
    {
        .name = "npc_shop",
        .flags = 0x1000'0600,
        .initEvtCode = (void*)TowerNpc_CharlietonInit,
        .regularEvtCode = (void*)TowerNpc_GenericMove,
        .talkEvtCode = (void*)TowerNpc_CharlietonTalk,
        .battleInfoId = -1,
    },
};

EVT_DEFINE_USER_FUNC(evtTot_SelectCharlietonItems) {
    int16_t* inventory = GetCharlietonInventoryPtr();
    
    // Pick 5 normal items, 5 special items, and 5 badges.
    // TODO: Add option to lower this to 3?
    const int32_t kNumItemsPerType = 5;
    for (int32_t i = 0; i < kNumItemsPerType * 3; ++i) {
        bool found = true;
        while (found) {
            found = false;
            int32_t item = PickRandomItem(
                RNG_NPC_OPTIONS,
                i / kNumItemsPerType == 0,
                i / kNumItemsPerType == 1, 
                i / kNumItemsPerType == 2, 
                0);
            // Make sure no duplicate items exist.
            for (int32_t j = 0; j < i; ++j) {
                if (inventory[j] == item) {
                    found = true;
                    break;
                }
            }
            inventory[i] = item;
        }
    }
    
    // Add a unique badge, and a (one-time purchase) Star Piece to the shop.
    int32_t num_badges = kNumItemsPerType;
    int32_t special_badge = RewardManager::GetUniqueBadgeForShop();
    if (special_badge) {
        inventory[kNumItemsPerType * 3 + 0] = special_badge;
        // inventory[kNumItemsPerType * 3 + 1] = ItemType::STAR_PIECE;
        inventory[kNumItemsPerType * 3 + 1] = -1;
        ++num_badges;
    } else {
        // inventory[kNumItemsPerType * 3 + 0] = ItemType::STAR_PIECE;
        inventory[kNumItemsPerType * 3 + 0] = -1;
    }
    
    // Sort each category by ascending price.
    qqsort(
        &inventory[kNumItemsPerType * 0], kNumItemsPerType, sizeof(int16_t),
        (void*)BuyPriceComparator);
    qqsort(
        &inventory[kNumItemsPerType * 1], kNumItemsPerType, sizeof(int16_t),
        (void*)BuyPriceComparator);
    qqsort(
        &inventory[kNumItemsPerType * 2], num_badges, sizeof(int16_t),
        (void*)BuyPriceComparator);
    
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_TrackNpcAction) {
    uint32_t type = evtGetValue(evt, evt->evtArguments[0]);
    uint32_t amount = evtGetValue(evt, evt->evtArguments[1]);
    g_Mod->state_.ChangeOption(type, amount);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_CheckAnyStatsDowngradeable) {
    int32_t stats_downgradeable = 0;
    if (g_Mod->state_.hp_level_ > 0) ++stats_downgradeable;
    if (g_Mod->state_.fp_level_ > 0) ++stats_downgradeable;
    if (g_Mod->state_.bp_level_ > 0) ++stats_downgradeable;
    // TODO: Don't check this if partners are disabled.
    if (g_Mod->state_.hp_p_level_ > 0) ++stats_downgradeable;

    evtSetValue(evt, evt->evtArguments[0], stats_downgradeable > 0);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_DowngradeStat) {
    switch (evtGetValue(evt, evt->evtArguments[0])) {
        case 1:
            --g_Mod->state_.hp_level_;
            break;
        case 2:
            --g_Mod->state_.hp_p_level_;
            break;
        case 3:
            --g_Mod->state_.fp_level_;
            break;
        case 4:
            --g_Mod->state_.bp_level_;
            break;
    }
    OptionsManager::UpdateLevelupStats();
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_GetChetCost) {
    evtSetValue(evt, evt->evtArguments[0], 50 * GetBuyPriceScale() / 100);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_GetDazzleCost) {
    int32_t num_sp_bought = g_Mod->state_.GetOption(STAT_RUN_NPC_SP_PURCHASED);
    evtSetValue(evt, evt->evtArguments[0], num_sp_bought * 10);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_CheckNpcEffectEnabled) {
    const int32_t floor = g_Mod->state_.floor_;
    bool effect = false;
    switch (evtGetValue(evt, evt->evtArguments[0])) {
        case SecondaryNpcType::GRUBBA:
            effect = g_Mod->state_.GetOption(STAT_RUN_NPC_GRUBBA_FLOOR) == floor;
            break;
        case SecondaryNpcType::DOOPLISS:
            effect = g_Mod->state_.GetOption(STAT_RUN_NPC_DOOPLISS_FLOOR) == floor;
            break;
    }
    evtSetValue(evt, evt->evtArguments[1], effect);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_EnableNpcEffect) {
    const int32_t floor = g_Mod->state_.floor_;
    switch (evtGetValue(evt, evt->evtArguments[0])) {
        case SecondaryNpcType::GRUBBA:
            g_Mod->state_.SetOption(STAT_RUN_NPC_GRUBBA_FLOOR, floor);
            break;
        case SecondaryNpcType::DOOPLISS:
            g_Mod->state_.SetOption(STAT_RUN_NPC_DOOPLISS_FLOOR, floor);
            break;
    }
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_GetTowerNpcParams) {
    // Charlieton parameters.
    evtSetValue(evt, evt->evtArguments[0], PTR(g_NpcSetup[0].name));
    evtSetValue(evt, evt->evtArguments[1], PTR(
        npcTribe[NpcTribeType::CHARLIETON].nameJp));
    evtSetValue(evt, evt->evtArguments[2], PTR(
        npcTribe[NpcTribeType::CHARLIETON].modelName));

    // Secondary NPC parameters (if one exists).
    int32_t floor = g_Mod->state_.floor_ / 8;
    int32_t npc_type = g_Mod->state_.GetOption(STAT_RUN_NPCS_SELECTED, floor);
    if (npc_type == SecondaryNpcType::NONE) {
        memset(&g_NpcSetup[1], 0, sizeof(NpcSetupInfo));
        evtSetValue(evt, evt->evtArguments[3], 0);
        evtSetValue(evt, evt->evtArguments[4], 0);
        evtSetValue(evt, evt->evtArguments[5], 0);
    } else {
        memcpy(
            &g_NpcSetup[1], &g_SecondaryNpcTemplates[npc_type],
            sizeof(NpcSetupInfo));
        evtSetValue(evt, evt->evtArguments[3], PTR(g_NpcSetup[1].name));
        evtSetValue(evt, evt->evtArguments[4], PTR(
            npcTribe[g_SecondaryNpcTribeIndices[npc_type]].nameJp));
        evtSetValue(evt, evt->evtArguments[5], PTR(
            npcTribe[g_SecondaryNpcTribeIndices[npc_type]].modelName));
    }

    evtSetValue(evt, evt->evtArguments[6], PTR(g_NpcSetup));
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_SelectSecondaryNpcs) {
    int32_t active_npc_types = 0;
    int32_t base_weights[SecondaryNpcType::NUM_NPC_TYPES];
    for (int32_t i = 0; i < SecondaryNpcType::NUM_NPC_TYPES; ++i) {
        // TODO: Make new options for whether to allow every individudal NPC?
        switch (i) {
            case SecondaryNpcType::LUMPY:
                // TODO: Add support for Lumpy.
                base_weights[i] = 0;
                break;
            default:
                base_weights[i] = 10;
                ++active_npc_types;
                break;
        }
    }
    bool selected_lumpy = false;

    // Select at most four types of NPC to spawn.
    while (active_npc_types > 4) {
        int32_t active_type_to_disable = 
            g_Mod->state_.Rand(active_npc_types, RNG_SECONDARY_NPC);
        int32_t active_type_idx = 0;
        for (int32_t i = 0; i < SecondaryNpcType::NUM_NPC_TYPES; ++i) {
            if (base_weights[i] > 0) {
                if (active_type_to_disable == active_type_idx) {
                    base_weights[i] = 0;
                    --active_npc_types;
                    break;
                }
                ++active_type_idx;
            }
        }
    }

    // Select NPCs for every floor.
    const int32_t num_rest_floors = g_Mod->state_.GetNumFloors() / 8;
    for (int32_t floor = 0; floor < num_rest_floors; ++floor) {
        // Make Lumpy slightly likelier to show up at the beginning.
        if (base_weights[SecondaryNpcType::LUMPY]) {
            if (floor < num_rest_floors / 4) {
                base_weights[SecondaryNpcType::LUMPY] = 15;
            } else {
                base_weights[SecondaryNpcType::LUMPY] = 10;
            }
        }

        // Disable Doopliss and Grubba on last floor, as well as Lumpy if he's
        // never showed up before (as it's impossible for him to show up later).
        if (floor == num_rest_floors - 1) {
            if (base_weights[SecondaryNpcType::GRUBBA]) {
                base_weights[SecondaryNpcType::GRUBBA] = 0;
                --active_npc_types;
            }
            if (base_weights[SecondaryNpcType::DOOPLISS]) {
                base_weights[SecondaryNpcType::DOOPLISS] = 0;
                --active_npc_types;
            }
            if (base_weights[SecondaryNpcType::LUMPY] && !selected_lumpy) {
                base_weights[SecondaryNpcType::LUMPY] = 0;
                --active_npc_types;
            }
        }

        // Pick an NPC at random (if there are fewer than 4 options, add some
        // chance that no NPC shows up so you aren't guaranteed one).
        int32_t weight = 0;
        for (int32_t i = 0; i < SecondaryNpcType::NUM_NPC_TYPES; ++i) {
            weight += base_weights[i];
        }
        if (active_npc_types < 4) weight += (4 - active_npc_types) * 10;
        weight = g_Mod->state_.Rand(weight, RNG_SECONDARY_NPC);

        g_Mod->state_.SetOption(
            STAT_RUN_NPCS_SELECTED, SecondaryNpcType::NONE, floor);
        for (int32_t i = 0; i < SecondaryNpcType::NUM_NPC_TYPES; ++i) {
            weight -= base_weights[i];
            if (weight < 0) {
                g_Mod->state_.SetOption(STAT_RUN_NPCS_SELECTED, i, floor);
                if (i == SecondaryNpcType::LUMPY) selected_lumpy = true;
                break;
            }
        }
    }

    return 2;
}

}  // namespace mod::tot::gon