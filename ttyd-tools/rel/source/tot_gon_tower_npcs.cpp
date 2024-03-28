#include "tot_gon_tower_npcs.h"

#include "evt_cmd.h"
#include "mod.h"
#include "tot_generate_item.h"
#include "tot_generate_reward.h"
#include "tot_state.h"
#include "tot_window_select.h"

#include <ttyd/evt_badgeshop.h>
#include <ttyd/evt_item.h>
#include <ttyd/evt_msg.h>
#include <ttyd/evt_npc.h>
#include <ttyd/evt_pouch.h>
#include <ttyd/evt_sub.h>
#include <ttyd/evt_window.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/item_data.h>
#include <ttyd/npcdrv.h>
#include <ttyd/system.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::tot::gon {

namespace {

// Including entire namespaces for convenience.
using namespace ::ttyd::evt_item;
using namespace ::ttyd::evt_msg;
using namespace ::ttyd::evt_npc;
using namespace ::ttyd::evt_pouch;
using namespace ::ttyd::evt_sub;
using namespace ::ttyd::evt_window;

using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;
using ::ttyd::npcdrv::NpcSetupInfo;
using ::ttyd::system::qqsort;

namespace ItemType = ::ttyd::item_data::ItemType;

const char kCharlietonName[] = 
    "\x8d\x73\x8f\xa4\x90\x6c";  // "gyoushounin" / "peddler"
const char kCharlietonTribe[] =
    "\x83\x7b\x83\x62\x83\x5e\x83\x4e\x81\x5b\x83\x8b";
const char kChetRippoTribe[] =
    "\x83\x70\x83\x8f\x81\x5b\x83\x5f\x83\x45\x83\x93\x89\xae";
const char kMoverTribeName[] = 
    "\x83\x76\x83\x6a\x8f\xee\x95\xf1\x89\xae";

}  // namespace

// Declarations for USER_FUNCs.
EVT_DECLARE_USER_FUNC(evtTot_SelectCharlietonItems, 0)

// Declarations for NPCs.
extern NpcSetupInfo g_CharlietonNpcSetup[2];
extern NpcSetupInfo g_ChetRippoNpcSetup[2];

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

// Init script for Charlieton; generates item table.
EVT_BEGIN(TowerNpc_CharlietonInit)
    USER_FUNC(evtTot_SelectCharlietonItems)
    RETURN()
EVT_END()

// Talk script for Charlieton.
// TODO: Add necessary dialogue to update_text_strings.
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
    // TODO: Custom window to account for different prices based on floor.
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

NpcSetupInfo g_CharlietonNpcSetup[2] = {
    {
        .nameJp = kCharlietonName,
        .flags = 0x1000'0600,
        .initEvtCode = (void*)TowerNpc_CharlietonInit,
        .regularEvtCode = (void*)TowerNpc_GenericMove,
        .talkEvtCode = (void*)TowerNpc_CharlietonTalk,
        .battleInfoId = -1,
    },
};

NpcSetupInfo g_ChetRippoNpcSetup[2] = {
    {
        .nameJp = kChetRippoTribe,
        .flags = 0x1000'0600,
        .regularEvtCode = nullptr,
        .talkEvtCode = nullptr,  // (void*)TowerNpc_ChetRippoTalk,
        .battleInfoId = -1,
    },
};

EVT_DEFINE_USER_FUNC(evtTot_SelectCharlietonItems) {
    int16_t* inventory = GetCharlietonInventoryPtr();
    
    // Pick 5 normal items, 5 special items, and 5 badges.
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
        inventory[kNumItemsPerType * 3] = special_badge;
        inventory[kNumItemsPerType * 3 + 1] = ItemType::STAR_PIECE;
        inventory[kNumItemsPerType * 3 + 2] = -1;
        ++num_badges;
    } else {
        inventory[kNumItemsPerType * 3] = ItemType::STAR_PIECE;
        inventory[kNumItemsPerType * 3 + 1] = -1;
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

EVT_DEFINE_USER_FUNC(evtTot_GetCharlietonNpcParams) {
    evtSetValue(evt, evt->evtArguments[0], PTR(kCharlietonName));
    evtSetValue(evt, evt->evtArguments[1], PTR(kCharlietonTribe));
    evtSetValue(evt, evt->evtArguments[2], PTR(g_CharlietonNpcSetup));
    return 2;
}

}  // namespace mod::tot::gon