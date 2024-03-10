#include "tot_generate_reward.h"

#include "evt_cmd.h"

#include <ttyd/dispdrv.h>
#include <ttyd/evt_eff.h>
#include <ttyd/evt_mario.h>
#include <ttyd/evt_msg.h>
#include <ttyd/evt_party.h>
#include <ttyd/evt_pouch.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evt_window.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/icondrv.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/party.h>
#include <ttyd/swdrv.h>

#include <cstring>

namespace mod::tot {

namespace {

// For convenience.
using namespace ::ttyd::evt_eff;
using namespace ::ttyd::evt_mario;
using namespace ::ttyd::evt_msg;
using namespace ::ttyd::evt_party;
using namespace ::ttyd::evt_pouch;
using namespace ::ttyd::evt_snd;
using namespace ::ttyd::evt_window;

using ::ttyd::dispdrv::CameraId;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;
using ::ttyd::item_data::itemDataTable;

namespace IconType = ::ttyd::icondrv::IconType;
namespace ItemType = ::ttyd::item_data::ItemType;

struct ChestData {
    gc::vec3    home_pos;
    int32_t     item;
    void*       pickup_script;
};
ChestData g_Chests[5];
int32_t g_ChestDrawAlpha = 0;

int32_t GetIcon(ChestData* chest) {
    switch (chest->item) {
        case -1:    return IconType::GOOMBELLA;
        case -2:    return IconType::KOOPS;
        case -3:    return IconType::FLURRIE;
        case -4:    return IconType::YOSHI_GREEN;
        case -5:    return IconType::VIVIAN;
        case -6:    return IconType::BOBBERY;
        case -7:    return IconType::MS_MOWZ;
        default:    return itemDataTable[chest->item].icon_id;
    }
}

void DisplayIcons(CameraId camera, void* user_data) {
    // Ideally this would fade, but alpha doesn't seem supported for this cam.
    int32_t value = ttyd::swdrv::swByteGet(1001);
    if (g_ChestDrawAlpha < 0xff && value == 1) g_ChestDrawAlpha = 0xff;
    if (g_ChestDrawAlpha > 0 && value != 1) g_ChestDrawAlpha = 0;
    
    auto* chest = (ChestData*)user_data;
    for (; chest->item; ++chest) {
        gc::vec3 pos = chest->home_pos;
        pos.y += 75.f;
        ttyd::icondrv::iconDispGxAlpha(
            1.0f, &pos, 0, GetIcon(chest), g_ChestDrawAlpha);
    }
}

}  // namespace

// Evt declarations.
EVT_DECLARE_USER_FUNC(evtTot_InitializePartyMember, 2)
EVT_DECLARE_USER_FUNC(evtTot_PartyJumpOutOfChest, 7)
EVT_DECLARE_USER_FUNC(evtTot_PartyVictoryPose, 1)

// Dummy script to run for picking up an Ultra Shroom specifically.
EVT_BEGIN(Reward_Dummy)
    USER_FUNC(evt_win_other_select, 19)
    USER_FUNC(
        evt_msg_print_insert, 0, PTR("zz_test_win_select"), 0, 0,
        LW(1), LW(2), LW(3), LW(4))
    USER_FUNC(evt_mario_key_onoff, 1)
    SET(GSW(1000), 1)
    RETURN()
EVT_END()

// Event that plays "get partner" fanfare.
EVT_BEGIN(Reward_PartnerFanfareEvt)
    USER_FUNC(evt_snd_bgmoff, 0x400)
    USER_FUNC(evt_snd_bgmon, 1, PTR("BGM_FF_GET_PARTY1"))
    WAIT_MSEC(2000)
    RETURN()
EVT_END()

EVT_BEGIN(Reward_PartnerRestoreBgmEvt)
    WAIT_MSEC(100)
    USER_FUNC(evt_snd_bgmon_f, 0x300, PTR("BGM_STG0_100DN1"), 1500)
    RETURN()
EVT_END()

// Called from tot_gon_tower; LW(10-12) = position, LW(13) = item id (-1 to -7).
// TODO: Handle move selection menu if you already have the partner.
EVT_BEGIN(Reward_Partner)
    USER_FUNC(evt_mario_normalize)
    USER_FUNC(evt_mario_goodbye_party, 0)
    WAIT_MSEC(500)
    USER_FUNC(evtTot_InitializePartyMember, LW(13), LW(6))
    // TODO: Reposition partner by box? At origin is fine...
    USER_FUNC(evt_mario_set_party_pos, 0, LW(6), LW(10), LW(11), LW(12))
    // Wrapper to evt_party_jump_pos.
    USER_FUNC(evtTot_PartyJumpOutOfChest, 0, 0, 0, 0, 500, 0, -1000)
    // Start victory animation after a bit.
    BROTHER_EVT()
        WAIT_MSEC(50)
        USER_FUNC(evtTot_PartyVictoryPose, LW(13))
        WAIT_MSEC(1500)
        USER_FUNC(evt_party_set_breed_pose, 0, 1)
    END_BROTHER()
    RUN_EVT_ID(Reward_PartnerFanfareEvt, LW(7))
    
    // TODO: Move from evt_eff and system text to evt_sub_get_coin-like event?
    USER_FUNC(
        evt_eff, PTR("sub_bg"), PTR("itemget"), 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(evt_msg_toge, 1, 0, 0, 0)
    USER_FUNC(evt_msg_print, 0, PTR("pit_reward_party_join"), 0, 0)
    
    CHK_EVT(LW(7), LW(6))
    IF_EQUAL(LW(6), 1)
        DELETE_EVT(LW(7))
        USER_FUNC(evt_snd_bgmoff, 0x201)
    END_IF()
    USER_FUNC(evt_eff_softdelete, PTR("sub_bg"))
    USER_FUNC(evt_snd_bgmon, 0x120, 0)
    RUN_EVT(Reward_PartnerRestoreBgmEvt)
    WAIT_MSEC(500)
    USER_FUNC(evt_party_run, 0)
    USER_FUNC(evt_party_run, 1)
    RETURN()
EVT_END()


void RewardManager::PatchRewardItemData() {
    // HP upgrade.
    itemDataTable[ItemType::REWARD_HP_UP].name = "tot_reward_hpplus";
    itemDataTable[ItemType::REWARD_HP_UP].description = "tot_rewarddesc_hpplus";
    // FP upgrade.
    itemDataTable[ItemType::REWARD_FP_UP].name = "tot_reward_fpplus";
    itemDataTable[ItemType::REWARD_FP_UP].description = "tot_rewarddesc_fpplus";
    // BP upgrade.
    itemDataTable[ItemType::REWARD_BP_UP].name = "tot_reward_bpplus";
    itemDataTable[ItemType::REWARD_BP_UP].description = "tot_rewarddesc_bpplus";
    itemDataTable[ItemType::REWARD_BP_UP].icon_id = IconType::BP_ICON;
}

bool RewardManager::HandleRewardItemPickup(int32_t item_type) {
    auto& pouch = *ttyd::mario_pouch::pouchGetPtr();
    
    switch (item_type) {
        case ItemType::PIANTA:
            // "Big" coins, worth 5 apiece.
            ttyd::mario_pouch::pouchAddCoin(5);
            return true;
        case ItemType::REWARD_HP_UP:
            pouch.current_hp += 5;
            pouch.max_hp += 5;
            pouch.base_max_hp += 5;
            return true;
        case ItemType::REWARD_FP_UP:
            pouch.current_fp += 5;
            pouch.max_fp += 5;
            pouch.base_max_fp += 5;
            return true;
        case ItemType::REWARD_BP_UP:
            pouch.total_bp += 5;
            pouch.unallocated_bp += 5;
            return true;
        default:
            return false;
    }
}

// Selects the contents of the chests.
// TODO: Spawn chests based on Mario's current position.
EVT_DEFINE_USER_FUNC(evtTot_GenerateChestContents) {
    memset(g_Chests, 0, sizeof(g_Chests));
    
    g_Chests[0].home_pos = { 0.0, 0.0, -100.0 };
    g_Chests[0].item = -1;
    g_Chests[0].pickup_script = (void*)Reward_Partner;
    g_Chests[1].home_pos = { -80.0, 0.0, -100.0 };
    g_Chests[1].item = ItemType::COIN;
    g_Chests[1].pickup_script = nullptr;
    g_Chests[2].home_pos = { 80.0, 0.0, -100.0 };
    g_Chests[2].item = ItemType::REWARD_BP_UP;
    g_Chests[2].pickup_script = (void*)Reward_Dummy;
    
    return 2;
}

// Gets chest's XYZ position and contents.
EVT_DEFINE_USER_FUNC(evtTot_GetChestData) {
    int32_t idx = evtGetValue(evt, evt->evtArguments[0]);
    evtSetValue(evt, evt->evtArguments[1], g_Chests[idx].home_pos.x);
    evtSetValue(evt, evt->evtArguments[2], g_Chests[idx].home_pos.y);
    evtSetValue(evt, evt->evtArguments[3], g_Chests[idx].home_pos.z);
    evtSetValue(evt, evt->evtArguments[4], g_Chests[idx].item);
    evtSetValue(evt, evt->evtArguments[5], PTR(g_Chests[idx].pickup_script));
    return 2;
}

// Displays item icons above the chests.
EVT_DEFINE_USER_FUNC(evtTot_DisplayChestIcons) {
    ttyd::dispdrv::dispEntry(
        CameraId::k3d, 1, /* order = */ 900.f, DisplayIcons, g_Chests);
    return 2;
}

// Initializes party member's stats.
// arg0 = idx in actual game order, negative.
// arg1 = (out) idx in internal party order.
EVT_DEFINE_USER_FUNC(evtTot_InitializePartyMember) {
    int32_t idx = 1;
    switch((int32_t)evtGetValue(evt, evt->evtArguments[0])) {
        case -1:    idx = 1;    break;
        case -2:    idx = 2;    break;
        case -3:    idx = 5;    break;
        case -4:    idx = 4;    break;
        case -5:    idx = 6;    break;
        case -6:    idx = 3;    break;
        case -7:    idx = 7;    break;
    }
    auto& party_data = ttyd::mario_pouch::pouchGetPtr()->party_data[idx];
    
    party_data.flags |= 1;
    // TODO: Different amounts per party member.
    party_data.base_max_hp = 10;
    party_data.max_hp = 10;
    party_data.current_hp = 10;
    party_data.hp_level = 0;
    party_data.attack_level = 0;
    party_data.tech_level = 0;
    // TODO: Start with only base moves.
    
    evtSetValue(evt, evt->evtArguments[1], idx);
    return 2;
}

// Handle the partner jumping out of the chest.
// TODO: Jump to more central position / beside Mario?
EVT_DEFINE_USER_FUNC(evtTot_PartyJumpOutOfChest) {
    int32_t returnVal = ttyd::evt_party::evt_party_jump_pos(evt, isFirstCall);
    if (isFirstCall) {
        // Multiply initial jump velocity by 1.5 to not clip into the chest.
        auto* party_ptr = ttyd::party::partyGetPtr(ttyd::party::partyCtrlNo);
        if (party_ptr) party_ptr->velocity_y *= 1.5;
    }
    
    return returnVal;
}

// Play a victory animation for the partner.
EVT_DEFINE_USER_FUNC(evtTot_PartyVictoryPose) {
    auto* party_ptr = ttyd::party::partyGetPtr(ttyd::party::partyCtrlNo);
    if (party_ptr) {
        const char* pose_name = nullptr;
        switch((int32_t)evtGetValue(evt, evt->evtArguments[0])) {
            case -1:    pose_name = "PKR_Y_1";  break;
            case -2:    pose_name = "PNK_Y_1";  break;
            case -3:    pose_name = "PWD_Y_1";  break;
            case -4:    pose_name = "PYS_Y_1";  break;
            case -5:    pose_name = "PTR_Y_1";  break;
            case -6:    pose_name = "Y_1";      break;
            case -7:    pose_name = "PCH_Y_1";  break;
        }
        if (pose_name) ttyd::party::partyChgPose(party_ptr, pose_name);
    }
    return 2;
}

}  // namespace mod::tot