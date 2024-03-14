#include "tot_generate_reward.h"

#include "evt_cmd.h"
#include "mod.h"
#include "mod_state.h"
#include "tot_move_manager.h"
#include "tot_state.h"
#include "tot_window_select.h"

#include <ttyd/dispdrv.h>
#include <ttyd/evt_eff.h>
#include <ttyd/evt_mario.h>
#include <ttyd/evt_msg.h>
#include <ttyd/evt_party.h>
#include <ttyd/evt_pouch.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evt_sub.h>
#include <ttyd/evt_window.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/icondrv.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/msgdrv.h>
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
using namespace ::ttyd::evt_sub;
using namespace ::ttyd::evt_window;

using ::ttyd::dispdrv::CameraId;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;
using ::ttyd::item_data::itemDataTable;

namespace IconType = ::ttyd::icondrv::IconType;
namespace ItemType = ::ttyd::item_data::ItemType;

enum RewardType {
    // Items to use as TOT reward placeholders.
    REWARD_HP_UP        = ItemType::HP_PLUS,
    REWARD_FP_UP        = ItemType::FP_PLUS,
    REWARD_BP_UP        = ItemType::LUCKY_START_P,
    REWARD_HP_UP_P      = ItemType::HP_PLUS_P,
    REWARD_INV_UP       = ItemType::INN_COUPON,
    
    REWARD_SHINE_SPRITE = ItemType::SHINE_SPRITE,
    REWARD_STAR_PIECE   = ItemType::STAR_PIECE,
    
    REWARD_GOOMBELLA    = -1,
    REWARD_KOOPS        = -2,
    REWARD_FLURRIE      = -3,
    REWARD_YOSHI        = -4,
    REWARD_VIVIAN       = -5,
    REWARD_BOBBERY      = -6,
    REWARD_MOWZ         = -7,
    REWARD_JUMP         = -8,
    REWARD_HAMMER       = -9,
    REWARD_SPECIAL_MOVE = -10,
    REWARD_COINS        = -11,
    
    // TODO: Make these placeholders as well.
    REWARD_BADGE_STACKABLE  = ItemType::SUPER_SHROOM,
    REWARD_BADGE_UNIQUE     = ItemType::DAMAGE_DODGE,
};

// Underlying data for chest positions + contents.
struct ChestData {
    gc::vec3    home_pos;
    int32_t     item;           // RewardType
    void*       pickup_script;
};
ChestData g_Chests[5];
int32_t g_ChestDrawAlpha = 0;

// Moves selected for unlocking / upgrading menus.
int32_t g_MoveSelections[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1 };
int32_t g_NumMovesSelected = 0;

int32_t GetIcon(ChestData* chest) {
    switch (chest->item) {
        case -1:    return IconType::GOOMBELLA;
        case -2:    return IconType::KOOPS;
        case -3:    return IconType::FLURRIE;
        case -4:    return IconType::YOSHI_GREEN;
        case -5:    return IconType::VIVIAN;
        case -6:    return IconType::BOBBERY;
        case -7:    return IconType::MS_MOWZ;
        case -8:    return IconType::BOOTS;
        case -9:    return IconType::HAMMER;
        case -10:   return IconType::STAR_ICON;
        case -11:   return IconType::COIN;
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
EVT_DECLARE_USER_FUNC(evtTot_ShouldUnlockPartner, 2)
EVT_DECLARE_USER_FUNC(evtTot_GetPartnerName, 2)
EVT_DECLARE_USER_FUNC(evtTot_InitializePartyMember, 2)
EVT_DECLARE_USER_FUNC(evtTot_PartyJumpOutOfChest, 7)
EVT_DECLARE_USER_FUNC(evtTot_PartyVictoryPose, 1)
EVT_DECLARE_USER_FUNC(evtTot_SelectMoves, 5)

// Pickup script for Star Pieces (base).
EVT_BEGIN(Reward_StarPieceBaseEvt)
    USER_FUNC(evtTot_SelectMoves, 1, 0, LW(0), LW(1), LW(2))
    IF_LARGE(LW(0), 0)
        USER_FUNC(evtTot_UpgradeMove, LW(1))
        USER_FUNC(
            evt_msg_print_insert, 0, PTR("tot_reward_upgrademove"), 0, 0, LW(2))
    END_IF()
    RETURN()
EVT_END()

// Pickup script for Star Pieces from chest.
EVT_BEGIN(Reward_StarPieceChestEvt)
    RUN_CHILD_EVT(Reward_StarPieceBaseEvt)
    USER_FUNC(evt_mario_key_onoff, 1)
    SET(GSW(1000), 1)
    RETURN()
EVT_END()

// Pickup script for Star Pieces as field item.
EVT_BEGIN(Reward_StarPieceItemDropEvt)
    USER_FUNC(evt_mario_key_onoff, 0)
    RUN_CHILD_EVT(Reward_StarPieceBaseEvt)
    USER_FUNC(evt_mario_key_onoff, 1)
    RETURN()
EVT_END()

// Pickup script for Shine Sprites.
EVT_BEGIN(Reward_ShineSpriteChestEvt)
    USER_FUNC(evtTot_SelectMoves, 1, 0, LW(0), LW(1), LW(2))
    IF_LARGE(LW(0), 0)
        // Open menu to select a move.
        // Note that LW(1) and LW(2) are overwritten by result.
        USER_FUNC(evt_win_other_select,
            (uint32_t)window_select::MenuType::MOVE_UPGRADE)
        USER_FUNC(evtTot_UpgradeMove, LW(1))
        USER_FUNC(
            evt_msg_print_insert, 0, PTR("tot_reward_upgrademove"), 0, 0, LW(2))
    END_IF()
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

// Script to reward coins.
// TODO: Parameterize based on floor count or something.
EVT_BEGIN(Reward_GetCoinsEvt)
    USER_FUNC(evt_sub_get_coin, 64)
    RETURN()
EVT_END()

// Script to reward a partner or jump/hammer/SP/partner move.
// Called from tot_gon_tower; LW(10-12) = position, LW(13) = reward id (-1 ~ -7).
EVT_BEGIN(Reward_PartnerOrMove)
    USER_FUNC(evtTot_ShouldUnlockPartner, LW(13), LW(0))
    IF_EQUAL(LW(0), 0)
        // Partner is already unlocked; check for unlockable moves.
        USER_FUNC(evtTot_SelectMoves, 0, LW(13), LW(0), LW(1), LW(2))
        IF_LARGE(LW(0), 0)
            // Open menu to select a move.
            // Note that LW(1) and LW(2) are overwritten by result.
            USER_FUNC(evt_win_other_select,
                (uint32_t)window_select::MenuType::MOVE_UNLOCK)
            USER_FUNC(evtTot_UpgradeMove, LW(1))
            USER_FUNC(
                evt_msg_print_insert, 0, PTR("tot_reward_learnmove"), 0, 0, LW(2))
        END_IF()
    ELSE()
        USER_FUNC(evt_mario_normalize)
        USER_FUNC(evt_mario_goodbye_party, 0)
        WAIT_MSEC(500)
        USER_FUNC(evtTot_InitializePartyMember, LW(13), LW(6))
        USER_FUNC(evt_mario_set_party_pos, 0, LW(6), LW(10), LW(11), LW(12))
        // Have partner jump out of chest; wrapper to evt_party_jump_pos.
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
        USER_FUNC(evtTot_GetPartnerName, LW(13), LW(1))
        USER_FUNC(evt_msg_print_insert, 0, PTR("tot_reward_getparty"), 0, 0, LW(1))
        
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
    END_IF()
    RETURN()
EVT_END()


void RewardManager::PatchRewardItemData() {
    // HP upgrade.
    itemDataTable[REWARD_HP_UP].name = "tot_reward_hpplus";
    itemDataTable[REWARD_HP_UP].description = "tot_rewarddesc_hpplus";
    // FP upgrade.
    itemDataTable[REWARD_FP_UP].name = "tot_reward_fpplus";
    itemDataTable[REWARD_FP_UP].description = "tot_rewarddesc_fpplus";
    // BP upgrade.
    itemDataTable[REWARD_BP_UP].name = "tot_reward_bpplus";
    itemDataTable[REWARD_BP_UP].description = "tot_rewarddesc_bpplus";
    itemDataTable[REWARD_BP_UP].icon_id = IconType::BP_ICON;
    // Partner HP upgrade.
    itemDataTable[REWARD_HP_UP_P].name = "tot_reward_hpplusp";
    itemDataTable[REWARD_HP_UP_P].description = "tot_rewarddesc_hpplusp";
    // Strange Sack upgrade.
    itemDataTable[REWARD_INV_UP].name = "tot_reward_sack";
    itemDataTable[REWARD_INV_UP].description = "tot_rewarddesc_sack";
    itemDataTable[REWARD_INV_UP].icon_id = IconType::STRANGE_SACK;
}

bool RewardManager::HandleRewardItemPickup(int32_t item_type) {
    auto& pouch = *ttyd::mario_pouch::pouchGetPtr();
    
    switch (item_type) {
        case ItemType::PIANTA:
            // "Big" coins, worth 5 apiece.
            ttyd::mario_pouch::pouchAddCoin(5);
            return true;
        case REWARD_HP_UP:
            pouch.current_hp += 5;
            pouch.max_hp += 5;
            pouch.base_max_hp += 5;
            return true;
        case REWARD_FP_UP:
            pouch.current_fp += 5;
            pouch.max_fp += 5;
            pouch.base_max_fp += 5;
            return true;
        case REWARD_BP_UP:
            pouch.total_bp += 5;
            pouch.unallocated_bp += 5;
            return true;
        case REWARD_INV_UP:
            ++infinite_pit::g_Mod->state_.num_sack_upgrades_;
            return true;
        case REWARD_HP_UP_P: {
            for (int32_t i = 1; i <= 7; ++i) {
                // TODO: Different amounts of HP per party member?
                pouch.party_data[i].current_hp += 5;
                pouch.party_data[i].max_hp += 5;
                pouch.party_data[i].base_max_hp += 5;
            }
            return true;
        }
        default:
            return false;
    }
}

int32_t* RewardManager::GetSelectedMoves(int32_t* num_moves) {
    if (num_moves) *num_moves = g_NumMovesSelected;
    return g_MoveSelections;
}

// Selects the contents of the chests.
// TODO: Spawn chests based on Mario's current position.
EVT_DEFINE_USER_FUNC(evtTot_GenerateChestContents) {
    const int32_t kRewardTypes[] = {
        REWARD_GOOMBELLA, REWARD_KOOPS, REWARD_FLURRIE, 
        REWARD_YOSHI, REWARD_VIVIAN, REWARD_BOBBERY,
        REWARD_MOWZ, REWARD_JUMP, REWARD_HAMMER,
        REWARD_SPECIAL_MOVE,
        
        REWARD_COINS, REWARD_SHINE_SPRITE, REWARD_STAR_PIECE,
        
        REWARD_HP_UP, REWARD_FP_UP, REWARD_BP_UP, REWARD_HP_UP_P,
        REWARD_INV_UP, REWARD_BADGE_STACKABLE, REWARD_BADGE_UNIQUE,
    };
    const void* kRewardScripts[] = {
        Reward_PartnerOrMove, Reward_PartnerOrMove, Reward_PartnerOrMove,
        Reward_PartnerOrMove, Reward_PartnerOrMove, Reward_PartnerOrMove,
        Reward_PartnerOrMove, Reward_PartnerOrMove, Reward_PartnerOrMove,
        Reward_PartnerOrMove,
        
        Reward_GetCoinsEvt, Reward_ShineSpriteChestEvt, Reward_StarPieceChestEvt,
        
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    };
    const gc::vec3 positions[] = {
        { 0.0, 0.0, -100.0 },
        { -80.0, 0.0, -100.0 },
        { 80.0, 0.0, -100.0 },
    };
    
    memset(g_Chests, 0, sizeof(g_Chests));
    
    // TODO: Replace with real generation code.
    for (int32_t i = 0; i < 3; ++i) {
        g_Chests[i].home_pos = positions[i];
        
        int32_t rand_val =
            infinite_pit::g_Mod->inf_state_.Rand(sizeof(kRewardTypes) / sizeof(int32_t));
        g_Chests[i].item = kRewardTypes[rand_val];
        g_Chests[i].pickup_script = (void*)kRewardScripts[rand_val];
    }
    
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

// Returns a pointer to the pickup script for a Star Piece.
EVT_DEFINE_USER_FUNC(evtTot_GetStarPiecePickupEvt) {
    evtSetValue(evt, evt->evtArguments[0], PTR(Reward_StarPieceItemDropEvt));
    return 2;
}

// Returns whether a partner needs unlocking given the reward id.
// arg0 = reward id, arg1 = (out) needs unlock
EVT_DEFINE_USER_FUNC(evtTot_ShouldUnlockPartner) {
    int32_t idx = 0;
    switch((int32_t)evtGetValue(evt, evt->evtArguments[0])) {
        case -1:    idx = 1;    break;
        case -2:    idx = 2;    break;
        case -3:    idx = 5;    break;
        case -4:    idx = 4;    break;
        case -5:    idx = 6;    break;
        case -6:    idx = 3;    break;
        case -7:    idx = 7;    break;
        default:                break;
    }
    int32_t should_unlock =
        idx && !(ttyd::mario_pouch::pouchGetPtr()->party_data[idx].flags & 1);
    evtSetValue(evt, evt->evtArguments[1], should_unlock);
    return 2;
}

// Returns the party member's name.
// arg0 = reward id, arg1 = (out) name
EVT_DEFINE_USER_FUNC(evtTot_GetPartnerName) {
    const char* str = "A new member";
    switch((int32_t)evtGetValue(evt, evt->evtArguments[0])) {
        case -1:    str = "Goombella";  break;
        case -2:    str = "Koops";      break;
        case -3:    str = "Flurrie";    break;
        case -4:    str = "Yoshi";      break;
        case -5:    str = "Vivian";     break;
        case -6:    str = "Bobbery";    break;
        case -7:    str = "Ms. Mowz";   break;
        default:                        break;
    }
    evtSetValue(evt, evt->evtArguments[1], PTR(str));
    return 2;
}

// Initializes party member's stats.
// arg0 = reward id (-1 to -7 in actual game order).
// arg1 = (out) idx in internal party order.
EVT_DEFINE_USER_FUNC(evtTot_InitializePartyMember) {
    int32_t reward_type = evtGetValue(evt, evt->evtArguments[0]);
    int32_t idx = 1;
    switch(reward_type) {
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
    
    // TODO: Different amounts of HP per party member?
    party_data.hp_level = 0;
    party_data.attack_level = 0;
    party_data.tech_level = 0;
    
    evtSetValue(evt, evt->evtArguments[1], idx);
    return 2;
}

// Handle the partner jumping out of the chest.
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

// Selects moves for unlocking selection menu.
// arg0 = mode; unlock (0) or upgrade (1)
// arg1 = move type (partner/jump/hammer/SP), or all (0)
// arg2 = (out) # options
// arg3 = (out) first option, arg4 = (out) first option name
EVT_DEFINE_USER_FUNC(evtTot_SelectMoves) {
    bool is_upgrade_mode = evtGetValue(evt, evt->evtArguments[0]);
    int32_t option_start = 0;
    int32_t max_options = 0;
    switch((int32_t)evtGetValue(evt, evt->evtArguments[1])) {
        case 0:  {
            option_start = 0;
            max_options = MoveType::MOVE_TYPE_MAX;
            break;
        }
        case -1: {
            option_start = MoveType::GOOMBELLA_BASE;
            max_options = 6;
            break;
        }
        case -2: {
            option_start = MoveType::KOOPS_BASE;
            max_options = 6;
            break;
        }
        case -3: {
            option_start = MoveType::FLURRIE_BASE;
            max_options = 6;
            break;
        }
        case -4: {
            option_start = MoveType::YOSHI_BASE;
            max_options = 6;
            break;
        }
        case -5: {
            option_start = MoveType::VIVIAN_BASE;
            max_options = 6;
            break;
        }
        case -6: {
            option_start = MoveType::BOBBERY_BASE;
            max_options = 6;
            break;
        }
        case -7: {
            option_start = MoveType::MOWZ_BASE;
            max_options = 6;
            break;
        }
        case -8: {
            option_start = MoveType::JUMP_BASE;
            max_options = 8;
            break;
        }
        case -9: {
            option_start = MoveType::HAMMER_BASE;
            max_options = 8;
            break;
        }
        case -10: {
            option_start = MoveType::SP_SWEET_TREAT;
            max_options = 8;
            break;
        }
    }
    
    // TODO: Write generation code that selects a few randomly from all options.
    
    int32_t num_options = 0;
    for (int32_t i = 0; i < max_options; ++i) {
        // Can only support 8 options at once.
        if (num_options >= 8) break;
        
        int32_t move = option_start + i;
        if ((is_upgrade_mode && MoveManager::IsUpgradable(move)) ||
            (!is_upgrade_mode && MoveManager::IsUnlockable(move))) {
            g_MoveSelections[num_options] = move;
            ++num_options;
        }
    }
    g_MoveSelections[num_options] = -1;
    g_NumMovesSelected = num_options;
    
    evtSetValue(evt, evt->evtArguments[2], num_options);
    if (num_options > 0) {
        evtSetValue(evt, evt->evtArguments[3], g_MoveSelections[0]);
        evtSetValue(
            evt, evt->evtArguments[4],
            PTR(ttyd::msgdrv::msgSearch(
                MoveManager::GetMoveData(g_MoveSelections[0])->name_msg)));
    }
    
    return 2;
}

}  // namespace mod::tot