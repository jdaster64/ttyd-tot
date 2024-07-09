#include "tot_generate_reward.h"

#include "common_functions.h"
#include "evt_cmd.h"
#include "mod.h"
#include "mod_state.h"
#include "patches_item.h"
#include "tot_generate_enemy.h"
#include "tot_generate_item.h"
#include "tot_manager_move.h"
#include "tot_manager_options.h"
#include "tot_manager_timer.h"
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

#include <cinttypes>
#include <cstdio>
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
    REWARD_HP_UP            = ItemType::HP_PLUS,
    REWARD_FP_UP            = ItemType::FP_PLUS,
    REWARD_BP_UP            = ItemType::LUCKY_START_P,
    REWARD_HP_UP_P          = ItemType::HP_PLUS_P,
    REWARD_INV_UP           = ItemType::INN_COUPON,
    
    REWARD_SHINE_SPRITE     = ItemType::SHINE_SPRITE,
    REWARD_STAR_PIECE       = ItemType::STAR_PIECE,
    
    REWARD_GOOMBELLA        = -1,
    REWARD_KOOPS            = -2,
    REWARD_FLURRIE          = -3,
    REWARD_YOSHI            = -4,
    REWARD_VIVIAN           = -5,
    REWARD_BOBBERY          = -6,
    REWARD_MOWZ             = -7,
    REWARD_JUMP             = -8,
    REWARD_HAMMER           = -9,
    REWARD_SPECIAL_MOVE     = -10,
    REWARD_COINS            = -11,
    REWARD_FULL_HEAL        = -12,
    
    // Used to determine how many chests should be filled.
    REWARD_PLACEHOLDER      = -999,
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

// Returns the option flag to check for whether the badge was already obtained.
int32_t GetUniqueBadgeObtainedIndex(int32_t item_type) {
    switch (item_type) {
        case ItemType::CHILL_OUT:       return 0;
        case ItemType::DOUBLE_DIP:      return 1;
        case ItemType::DOUBLE_DIP_P:    return 2;
        case ItemType::FEELING_FINE:    return 3;
        case ItemType::FEELING_FINE_P:  return 4;
        case ItemType::LUCKY_START:     return 5;
        case ItemType::QUICK_CHANGE:    return 6;
        case ItemType::RETURN_POSTAGE:  return 7;
        case ItemType::ZAP_TAP:         return 8;
        case ItemType::SPIKE_SHIELD:    return 9;
    }
    return -1;
}

// Selects which unique badge to give as a reward.
int32_t SelectUniqueBadge() {
    auto& state = g_Mod->state_;
    static constexpr const int32_t kBadges[] = {
        ItemType::CHILL_OUT, ItemType::DOUBLE_DIP,
        ItemType::DOUBLE_DIP_P, ItemType::FEELING_FINE,
        ItemType::FEELING_FINE_P, ItemType::LUCKY_START,
        ItemType::QUICK_CHANGE, ItemType::RETURN_POSTAGE,
        ItemType::ZAP_TAP, ItemType::SPIKE_SHIELD,
    };
    uint16_t weights[] = { 10, 10, 10, 10, 10, 10, 15, 10, 10, 20 };
    uint8_t eligible[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
    int32_t total_weight = 115;
    
    if (GetNumActivePartners() == 0) {
        eligible[2] = 0;
        eligible[4] = 0;
        eligible[6] = 0;
    }
    
    // Disable badges that have already been picked up.
    for (int32_t i = 0; i < 10; ++i) {
        if (state.GetOption(
                STAT_RUN_UNIQUE_BADGE_FLAGS,
                GetUniqueBadgeObtainedIndex(kBadges[i]))) {
            eligible[i] = 0;
        }
    }
    
    int32_t num_eligible = 0;
    for (int32_t i = 0; i < 10; ++i) num_eligible += eligible[i];
    
    if (num_eligible > 0) {
        // Find the next badge in the sequence that hasn't yet been claimed,
        // trying multiple times if necessary.
        for (int32_t retries = 100; retries > 0; --retries) {
            int32_t weight = state.Rand(total_weight, RNG_REWARD_BADGE_SPECIAL);
            total_weight = 0;
            for (int32_t i = 0; i < 10; ++i) {
                total_weight += weights[i];
                if (weight < total_weight) {
                    if (eligible[i]) return kBadges[i];
                    break;
                }
            }
        }
    }
    
    // No unique badges left to unlock.
    return 0;
}

// Returns the starting move type and number of moves for the category.
void GetMoveRange(int32_t reward_type, int32_t& move_start, int32_t& num_moves) {
    switch(reward_type) {
        case 0:  {
            // All moves.
            move_start = 0;
            num_moves = MoveType::MOVE_TYPE_MAX;
            break;
        }
        case REWARD_GOOMBELLA: {
            move_start = MoveType::GOOMBELLA_BASE;
            num_moves = 6;
            break;
        }
        case REWARD_KOOPS: {
            move_start = MoveType::KOOPS_BASE;
            num_moves = 6;
            break;
        }
        case REWARD_FLURRIE: {
            move_start = MoveType::FLURRIE_BASE;
            num_moves = 6;
            break;
        }
        case REWARD_YOSHI: {
            move_start = MoveType::YOSHI_BASE;
            num_moves = 6;
            break;
        }
        case REWARD_VIVIAN: {
            move_start = MoveType::VIVIAN_BASE;
            num_moves = 6;
            break;
        }
        case REWARD_BOBBERY: {
            move_start = MoveType::BOBBERY_BASE;
            num_moves = 6;
            break;
        }
        case REWARD_MOWZ: {
            move_start = MoveType::MOWZ_BASE;
            num_moves = 6;
            break;
        }
        case REWARD_JUMP: {
            move_start = MoveType::JUMP_BASE;
            num_moves = 8;
            break;
        }
        case REWARD_HAMMER: {
            move_start = MoveType::HAMMER_BASE;
            num_moves = 8;
            break;
        }
        case REWARD_SPECIAL_MOVE: {
            move_start = MoveType::SP_SWEET_TREAT;
            num_moves = 8;
            break;
        }
    }
}

// Returns whether all moves for the given reward type are already unlocked.
bool HasAllMovesUnlocked(int32_t reward_type) {
    int32_t move_start = 0;
    int32_t num_moves = 0;
    GetMoveRange(reward_type, move_start, num_moves);
    
    for (int32_t i = 0; i < num_moves; ++i) {
        int32_t move = move_start + i;
        if (MoveManager::IsUnlockable(move)) return false;
    }
    
    return true;
}

int32_t PartnerRewardTypeToPouchIndex(int32_t reward_type) {
    switch (reward_type) {
        case REWARD_GOOMBELLA:  return 1;
        case REWARD_KOOPS:      return 2;
        case REWARD_FLURRIE:    return 5;
        case REWARD_YOSHI:      return 4;
        case REWARD_VIVIAN:     return 6;
        case REWARD_BOBBERY:    return 3;
        case REWARD_MOWZ:       return 7;
    }
    return 0;
}

// Selects which partner to offer as a reward.
int32_t SelectPartner() {
    auto& state = g_Mod->state_;

    // Handle specific partner requests on floor 0.
    if (state.floor_ == 0) {
        switch (state.GetOptionValue(OPT_PARTNER)) {
            case OPTVAL_PARTNER_GOOMBELLA:  return REWARD_GOOMBELLA;
            case OPTVAL_PARTNER_KOOPS:      return REWARD_KOOPS;
            case OPTVAL_PARTNER_FLURRIE:    return REWARD_FLURRIE;
            case OPTVAL_PARTNER_YOSHI:      return REWARD_YOSHI;
            case OPTVAL_PARTNER_VIVIAN:     return REWARD_VIVIAN;
            case OPTVAL_PARTNER_BOBBERY:    return REWARD_BOBBERY;
            case OPTVAL_PARTNER_MOWZ:       return REWARD_MOWZ;
            // default: run normal logic.
        }
    }

    for (int32_t i = 0; i < 50; ++i) {
        int32_t value = state.Rand(7, RNG_REWARD_PARTNER);
        int32_t reward_type = -(value + 1);
        
        // Once the player has filled their partner pool...
        if (GetNumActivePartners() >= state.GetOption(OPT_MAX_PARTNERS)) {
            // If the chosen partner is not in the pool; try rolling again.
            int32_t partner_idx = PartnerRewardTypeToPouchIndex(reward_type);
            if (!(ttyd::mario_pouch::pouchGetPtr()->
                    party_data[partner_idx].flags & 1)) {
                continue;
            }
        }

        // Skip next roll if it yields the same value, to make it less likely
        // you see consecutive upgrades for the same partner.
        if (state.Rand(7, RNG_REWARD_PARTNER) != (uint32_t)value) {
            --state.rng_states_[RNG_REWARD_PARTNER];
        }

        return reward_type;
    }
    
    // If no partners could be found, return 0 as a backup.
    return 0;
}

// Selects which Special Move to offer as a reward.
int32_t SelectSpecialMove() {
    auto& state = g_Mod->state_;
    auto& pouch = *ttyd::mario_pouch::pouchGetPtr();
    const int32_t max_sp = pouch.max_sp / 100;
    uint16_t weights[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    int32_t total_weight = 0;
    
    for (int32_t i = 2; i < 8; ++i) {
        auto* data = MoveManager::GetMoveData(MoveType::SP_SWEET_TREAT + i);
        // Set weight for move only if it can be afforded and isn't unlocked.
        if (data->move_cost[2] <= (max_sp + 1) && 
            !(pouch.star_powers_obtained & (1 << i))) {
            weights[i] = 1;
            ++total_weight;
        }
    }
    
    if (total_weight > 0) {
        // Make sure the same special move is chosen every time until taken.
        state.rng_states_[RNG_MOVE_SPECIAL] = max_sp;
        int32_t weight = state.Rand(total_weight, RNG_MOVE_SPECIAL);
        total_weight = 0;
        for (int32_t i = 2; i < 8; ++i) {
            total_weight += weights[i];
            if (weight < total_weight) return ItemType::EMERALD_STAR + (i - 2);
        }
    }
    
    // No Special Moves left to unlock.
    return 0;
}

// Selects set of moves to offer for unlocking or upgrading moves.
void SelectMoves(int32_t reward_type, bool is_upgrade_mode) {
    auto& state = g_Mod->state_;
    
    // Determine how many moves to offer at maximum.
    bool is_partner = reward_type <= REWARD_GOOMBELLA && reward_type >= REWARD_MOWZ;
    int32_t max_options = 3;
    if (!is_upgrade_mode && is_partner) {
        max_options = 2;
    }
    
    // Fill in array of available moves for the type.
    int32_t move_start = 0;
    int32_t num_moves = 0;
    GetMoveRange(reward_type, move_start, num_moves);
    
    bool moves[MoveType::MOVE_TYPE_MAX] = { 0 };
    int32_t num_options = 0;
    for (int32_t i = 0; i < num_moves; ++i) {        
        int32_t move = move_start + i;
        if ((is_upgrade_mode && MoveManager::IsUpgradable(move)) ||
            (!is_upgrade_mode && MoveManager::IsUnlockable(move))) {
            moves[move] = 1;
            ++num_options;
        }
    }
    if (num_options < max_options) max_options = num_options;
    
    g_NumMovesSelected = 0;
    
    int32_t i = 0;
    if (max_options > 0) {
        // Pick which RNG state to use.
        int32_t rng_type = RNG_MOVE_UPGRADE;
        if (!is_upgrade_mode) {
            rng_type = RNG_MOVE_GOOMBELLA - (reward_type + 1);
        }
        
        // Select moves at random until picking the requisite number,
        // or running out of options, whichever happens first.
        for (int32_t retries = 300; retries > 0; --retries) {
            int32_t move = move_start + state.Rand(num_moves, rng_type);
            if (moves[move] == 0) continue;
            
            // For unlocking partner moves, limit the tiers that can appear
            // to 1 on the first pick, 1-2 on the second, and 1-3 otherwise.
            if (!is_upgrade_mode && is_partner) {
                int32_t tier = MoveManager::GetMoveData(move)->move_tier;
                // Determine which # pick this is by the # of available moves.
                int32_t max_rank = 6 - num_options;
                if (reward_type == REWARD_GOOMBELLA) --max_rank;
                // Force a level-2 or 3 to appear in the second and third pick.
                if (i == 0 && (max_rank == 2 || max_rank == 3)) {
                    if (tier != max_rank) continue;
                } else {
                    if (tier > max_rank) continue;
                }
            }
            
            // Move is valid; add to possible selections.
            g_MoveSelections[i] = move;
            ++g_NumMovesSelected;
            if (++i >= max_options) break;
            moves[move] = 0;
        }
    }
    
    // Put a sentinel at the end of the array of move selections.
    g_MoveSelections[i] = -1;
}

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
        case -12:   return IconType::HP_ICON;
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
EVT_DECLARE_USER_FUNC(evtTot_FullRecover, 0)
EVT_DECLARE_USER_FUNC(evtTot_ShouldUnlockPartner, 2)
EVT_DECLARE_USER_FUNC(evtTot_GetPartnerName, 2)
EVT_DECLARE_USER_FUNC(evtTot_InitializePartyMember, 2)
EVT_DECLARE_USER_FUNC(evtTot_PartyJumpOutOfChest, 7)
EVT_DECLARE_USER_FUNC(evtTot_PartyVictoryPose, 1)
EVT_DECLARE_USER_FUNC(evtTot_SelectMoves, 5)
EVT_DECLARE_USER_FUNC(evtTot_GetStarPieceRolls, 1)

// Script for health.
EVT_BEGIN(Reward_FullHealEvt)
    USER_FUNC(evt_mario_get_pos, 0, LW(0), LW(1), LW(2))
    USER_FUNC(evt_eff, 0, PTR("stardust"), 2, LW(0), LW(1), LW(2), 50, 50, 50, 100, 0, 0, 0, 0)
    USER_FUNC(evt_snd_sfxon_3d, PTR("SFX_HP_RECOVER_SHINE2"), LW(0), LW(1), LW(2), 0)
    USER_FUNC(evtTot_FullRecover)
    WAIT_MSEC(500)
    USER_FUNC(evt_msg_print, 0, PTR("tot_reward_fullrecovery"), 0, 0)
    RETURN()
EVT_END()

// Pickup script for Star Pieces (base).
EVT_BEGIN(Reward_StarPieceBaseEvt)
    USER_FUNC(infinite_pit::item::evtTot_FreezeFieldItemTimers, 1)
    USER_FUNC(evtTot_SelectMoves, 1, 0, LW(0), LW(1), LW(2))
    IF_LARGE(LW(0), 0)
        USER_FUNC(evtTot_UpgradeMove, LW(1))
        USER_FUNC(
            evt_msg_print_insert, 0, PTR("tot_reward_upgrademove"), 0, 0, LW(2))
    END_IF()
    USER_FUNC(infinite_pit::item::evtTot_FreezeFieldItemTimers, 0)
    RETURN()
EVT_END()

// Pickup script for Shine Sprites (base).
EVT_BEGIN(Reward_ShineSpriteBaseEvt)
    USER_FUNC(infinite_pit::item::evtTot_FreezeFieldItemTimers, 1)
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
    USER_FUNC(infinite_pit::item::evtTot_FreezeFieldItemTimers, 0)
    RETURN()
EVT_END()

// Pickup script for Star Pieces from chest (does 1-3 rolls, at random).
EVT_BEGIN(Reward_StarPieceChestEvt)
    USER_FUNC(evtTot_GetStarPieceRolls, LW(3))
    DO(LW(3))
        RUN_CHILD_EVT(Reward_StarPieceBaseEvt)
    WHILE()
    USER_FUNC(evtTot_ToggleIGT, 1)
    USER_FUNC(evt_mario_key_onoff, 1)
    SET((int32_t)GSW_Tower_ChestClaimed, 1)
    RETURN()
EVT_END()

// Pickup script for Star Pieces as field item.
EVT_BEGIN(Reward_StarPieceItemDropEvt)
    USER_FUNC(evtTot_ToggleIGT, 0)
    USER_FUNC(evt_mario_key_onoff, 0)
    RUN_CHILD_EVT(Reward_StarPieceBaseEvt)
    USER_FUNC(evtTot_ToggleIGT, 1)
    USER_FUNC(evt_mario_key_onoff, 1)
    RETURN()
EVT_END()

// Pickup script for Shine Sprites from chest.
EVT_BEGIN(Reward_ShineSpriteChestEvt)
    RUN_CHILD_EVT(Reward_ShineSpriteBaseEvt)
    USER_FUNC(evtTot_ToggleIGT, 1)
    USER_FUNC(evt_mario_key_onoff, 1)
    SET((int32_t)GSW_Tower_ChestClaimed, 1)
    RETURN()
EVT_END()

// Pickup script for Shine Sprites as field item.
EVT_BEGIN(Reward_ShineSpriteItemDropEvt)
    USER_FUNC(evtTot_ToggleIGT, 0)
    USER_FUNC(evt_mario_key_onoff, 0)
    RUN_CHILD_EVT(Reward_ShineSpriteBaseEvt)
    USER_FUNC(evtTot_ToggleIGT, 1)
    USER_FUNC(evt_mario_key_onoff, 1)
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
    USER_FUNC(evt_snd_bgmon_f, 0x300, PTR("BGM_STG1_GON1"), 1500)
    RETURN()
EVT_END()

// Script to reward coins.
EVT_BEGIN(Reward_GetCoinsEvt)
    USER_FUNC(evtTot_GetFloor, LW(0))
    // Give 25 coins + 5 per every 8 floors afterward.
    DIV(LW(0), 8)
    MUL(LW(0), 5)
    ADD(LW(0), 25)
    USER_FUNC(evt_sub_get_coin, LW(0))
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
        
        // Could write custom evt_sub_get_coin-like event in the future,
        // but this looks good enough as is.
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
    // Crystal Star icons.
    itemDataTable[ItemType::DIAMOND_STAR].icon_id = IconType::DIAMOND_STAR;
    itemDataTable[ItemType::EMERALD_STAR].icon_id = IconType::EMERALD_STAR;
    itemDataTable[ItemType::GOLD_STAR].icon_id = IconType::GOLD_STAR;
    itemDataTable[ItemType::RUBY_STAR].icon_id = IconType::RUBY_STAR;
    itemDataTable[ItemType::SAPPHIRE_STAR].icon_id = IconType::SAPPHIRE_STAR;
    itemDataTable[ItemType::GARNET_STAR].icon_id = IconType::GARNET_STAR;
    itemDataTable[ItemType::CRYSTAL_STAR].icon_id = IconType::CRYSTAL_STAR;
}

bool RewardManager::HandleRewardItemPickup(int32_t item_type) {
    switch (item_type) {
        case ItemType::PIANTA:
            // "Big" coins, worth 5 apiece.
            ttyd::mario_pouch::pouchAddCoin(5);
            return true;
        case REWARD_HP_UP:
            ++g_Mod->state_.hp_level_;
            OptionsManager::UpdateLevelupStats();
            return true;
        case REWARD_FP_UP:
            ++g_Mod->state_.fp_level_;
            OptionsManager::UpdateLevelupStats();
            return true;
        case REWARD_BP_UP:
            ++g_Mod->state_.bp_level_;
            OptionsManager::UpdateLevelupStats();
            return true;
        case REWARD_HP_UP_P:
            ++g_Mod->state_.hp_p_level_;
            OptionsManager::UpdateLevelupStats();
            return true;
        case REWARD_INV_UP:
            g_Mod->state_.max_inventory_ 
                += g_Mod->state_.GetOption(OPT_INVENTORY_SACK_SIZE);
            return true;
        case ItemType::DIAMOND_STAR:
        case ItemType::EMERALD_STAR:
        case ItemType::GOLD_STAR:
        case ItemType::RUBY_STAR:
        case ItemType::SAPPHIRE_STAR:
        case ItemType::GARNET_STAR:
        case ItemType::CRYSTAL_STAR: {
            int32_t move = MoveType::SP_EARTH_TREMOR + 
                (item_type - ItemType::DIAMOND_STAR);
            MoveManager::UpgradeMove(move);
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

void* RewardManager::GetStarPieceItemDropEvt() {
    return (void*)Reward_StarPieceItemDropEvt;
}

void* RewardManager::GetShineSpriteItemDropEvt() {
    return (void*)Reward_ShineSpriteItemDropEvt;
}

int32_t RewardManager::GetUniqueBadgeForShop() {
    // Use same function / RNG state as chests.
    int32_t unique_badge = SelectUniqueBadge();
    for (ChestData* chest = g_Chests; chest->item; ++chest) {
        if (chest->item == unique_badge) {
            unique_badge = 0;
            break;
        }
    }
    return unique_badge;
}

void RewardManager::MarkUniqueItemCollected(int32_t item_id) {
    int32_t index = GetUniqueBadgeObtainedIndex(item_id);
    if (index >= 0) {
        g_Mod->state_.SetOption(STAT_RUN_UNIQUE_BADGE_FLAGS, 1, index);
    }
}

// Assigns reward types and corresponding pickup scripts to all chests.
void SelectChestContents() {
    auto& state = g_Mod->state_;
    
    // Weights for different types of moves (Jump, Hammer, Special, partner).
    int32_t kMoveWeights[] = { 17, 17, 12, 52 };
    // Weights for different types of stat upgrades (HP, FP, BP, HP P, inv.).
    int32_t kStatWeights[] = { 20, 20, 20, 15, 10 };
    // Weights for different types of other rewards
    // (coins, Star Piece, Shine Sprite, unique badge, stackable badge).
    int32_t kOtherWeights[] = { 20, 20, 20, 20, 10 };
    
    // Top-level weight for choosing a move, stat-up, or other reward.
    // The former two categories cannot be chosen more than once per floor.
    int32_t top_level_weights[] = { 15, 15, 15 };
    // Tracks which kind of 'other' categories have been chosen already;
    // if one of them is rolled twice in one floor, picks a random stackable
    // badge in its place.
    bool others_picked[] = { false, false, false, false, false };

    // Change weights for moves based on how many partners are in the pool.
    const int32_t partner_pool_size = state.GetOption(OPT_MAX_PARTNERS);
    if (partner_pool_size > 4) {
        // Slightly increase partner weight.
        kMoveWeights[3] += (partner_pool_size - 4) * 8;
    } else if (partner_pool_size < 4) {
        // Decrease partner weight, and increase jump/hammer accordingly.
        const int32_t diff = kMoveWeights[3] * (4 - partner_pool_size) / 4;
        kMoveWeights[3] -= diff;
        kMoveWeights[0] += diff / 2;
        kMoveWeights[1] += diff / 2;
    }

    // Filter out stat-ups that are forced to 0 / have reached their maximum.
    if (state.GetOption(OPT_MARIO_HP) == 0) {
        top_level_weights[1] -= 3;
        kStatWeights[0] = 0;
    }
    if (state.GetOption(OPT_MARIO_FP) == 0) {
        top_level_weights[1] -= 3;
        kStatWeights[1] = 0;
    }
    if (state.GetOption(OPT_MARIO_BP) == 0 ||
        state.CheckOptionValue(OPTVAL_INFINITE_BP)) {
        top_level_weights[1] -= 3;
        kStatWeights[2] = 0;
    }
    if (state.GetOption(OPT_PARTNER_HP) == 0 ||
        state.CheckOptionValue(OPTVAL_NO_PARTNERS)) {
        top_level_weights[1] -= 3;
        kStatWeights[3] = 0;
    }
    if (state.GetOption(OPT_INVENTORY_SACK_SIZE) == 0 || 
        state.max_inventory_ >= 20) {
        top_level_weights[1] -= 3;
        kStatWeights[4] = 0;
    }
    
    for (ChestData* chest = g_Chests; chest->item; ++chest) {       
        int32_t sum_weights, weight, type;
        int32_t reward = 0;
        const void* pickup_script = nullptr;
        
        if (state.floor_ == 0) {
            // Floor 0: Force a move.
            type = 0;
        } else {
            // Otherwise, pick top-level reward category.
            sum_weights = 0;
            for (const auto& weight : top_level_weights) {
                sum_weights += weight;
            }
            weight = state.Rand(sum_weights, RNG_REWARD);
            sum_weights = 0;
            for (type = 0; type < 3; ++type) {
                sum_weights += top_level_weights[type];
                if (weight < sum_weights) break;
            }
        }
        
        // If 'move' or 'stat-up' category, disable for rest of chests.
        if (type < 2) top_level_weights[type] = 0;
        
        if (type == 0) {
            // Pick type of move reward.
            sum_weights = 0;
            for (const auto& weight : kMoveWeights) {
                sum_weights += weight;
            }
            weight = state.Rand(sum_weights, RNG_REWARD_MOVE);
            sum_weights = 0;
            for (type = 0; type < 4; ++type) {
                sum_weights += kMoveWeights[type];
                if (weight < sum_weights) break;
            }
            
            // Floor 0: Force a partner, unless partners are disabled.
            if (state.floor_ == 0) {
                if (!state.CheckOptionValue(OPTVAL_NO_PARTNERS)) {
                    type = 3;
                }
            }
            
            // Assign reward.
            switch (type) {
                case 0:
                    reward = REWARD_JUMP;
                    pickup_script = Reward_PartnerOrMove;
                    break;
                case 1:
                    reward = REWARD_HAMMER;
                    pickup_script = Reward_PartnerOrMove;
                    break;
                case 2:
                    reward = SelectSpecialMove();
                    break;
                case 3:
                    reward = SelectPartner();
                    pickup_script = Reward_PartnerOrMove;
                    break;
            }
            
            // If all moves of type already unlocked, pick random badge instead.
            if (reward == 0 || (reward < 0 && HasAllMovesUnlocked(reward))) {
                reward = 0;
                pickup_script = nullptr;
                top_level_weights[0] = 10;
            }
        } else if (type == 1) {
            // Pick type of stat-up reward.
            sum_weights = 0;
            for (const auto& weight : kStatWeights) {
                sum_weights += weight;
            }
            weight = state.Rand(sum_weights, RNG_REWARD_STAT_UP);
            sum_weights = 0;
            for (type = 0; type < 5; ++type) {
                sum_weights += kStatWeights[type];
                if (weight < sum_weights) break;
            }
            // Assign reward.
            switch (type) {
                case 0:
                    reward = REWARD_HP_UP;
                    break;
                case 1:
                    reward = REWARD_FP_UP;
                    break;
                case 2:
                    reward = REWARD_BP_UP;
                    break;
                case 3:
                    reward = REWARD_HP_UP_P;
                    break;
                case 4:
                    reward = REWARD_INV_UP;
                    break;
            }
        } else {
            // Pick type of other reward.
            sum_weights = 0;
            for (const auto& weight : kOtherWeights) {
                sum_weights += weight;
            }
            weight = state.Rand(sum_weights, RNG_REWARD_OTHER);
            sum_weights = 0;
            for (type = 0; type < 5; ++type) {
                sum_weights += kOtherWeights[type];
                if (weight < sum_weights) break;
            }
            // If this reward type was not already picked, assign it.
            if (!others_picked[type]) {
                switch (type) {
                    case 0:
                        reward = REWARD_COINS;
                        pickup_script = Reward_GetCoinsEvt;
                        break;
                    case 1:
                        reward = REWARD_STAR_PIECE;
                        pickup_script = Reward_StarPieceChestEvt;
                        break;
                    case 2:
                        reward = REWARD_SHINE_SPRITE;
                        pickup_script = Reward_ShineSpriteChestEvt;
                        break;
                    case 3:
                        reward = SelectUniqueBadge();
                        break;
                }
                others_picked[type] = true;
            }
        }
        
        // If not selected yet, pick a random stackable badge.
        if (reward == 0) {
            reward = PickRandomItem(RNG_REWARD_BADGE_NORMAL, 0, 0, 1, 0);
        }
        
        chest->item = reward;
        chest->pickup_script = (void*)pickup_script;
    }
}

// Selects the contents of the chests.
EVT_DEFINE_USER_FUNC(evtTot_GenerateChestContents) {
    const gc::vec3 positions[] = {
        { -50.0, 0.0, -200.0 },
        { 50.0, 0.0, -200.0 },
        { -150.0, 0.0, -200.0 },
        { 150.0, 0.0, -200.0 },
    };
    memset(g_Chests, 0, sizeof(g_Chests));
    
    int32_t num_chests = GetBattleRewardTier();

    for (int32_t i = 0; i < num_chests; ++i) {
        g_Chests[i].home_pos = positions[i];
        g_Chests[i].item = REWARD_PLACEHOLDER;
    }
    SelectChestContents();
    
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

EVT_DEFINE_USER_FUNC(evtTot_RankUpRandomMoveInBattle) {
    static char buf[128] = { 0 };

    SelectMoves(/* reward_type */ 0, /* is_upgrade_mode*/ 1);

    if (g_NumMovesSelected > 0) {
        // Upgrade first move selected.
        MoveManager::UpgradeMove(g_MoveSelections[0]);

        // Get message to display in-battle.
        const char* move_name = ttyd::msgdrv::msgSearch(
            MoveManager::GetMoveData(g_MoveSelections[0])->name_msg);
        const char* msg = 
            ttyd::msgdrv::msgSearch("tot_reward_upgrademove_inbattle");
        sprintf(buf, msg, move_name);

        evtSetValue(evt, evt->evtArguments[0], PTR(buf));
    } else {
        evtSetValue(evt, evt->evtArguments[0], 0);
    }
    return 2;
}

// Displays item icons above the chests.
EVT_DEFINE_USER_FUNC(evtTot_DisplayChestIcons) {
    ttyd::dispdrv::dispEntry(
        CameraId::k3d, 1, /* order = */ 900.f, DisplayIcons, g_Chests);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_AfterItemBought) {
    int32_t item_type = evtGetValue(evt, evt->evtArguments[0]);

    int32_t index = GetUniqueBadgeObtainedIndex(item_type);
    if (index >= 0) {
        // Mark unique badges as collected even if the player throws it away.
        RewardManager::MarkUniqueItemCollected(item_type);
    }
    if (index >= 0 || item_type == ItemType::STAR_PIECE ||
        g_Mod->state_.CheckOptionValue(OPTVAL_CHARLIETON_LIMITED)) {
        // Search Charlieton's array for the item id and remove it.
        int16_t* charlieton_data = GetCharlietonInventoryPtr();
        int16_t* last = charlieton_data;
        while (*charlieton_data >= 0) {
            if (*charlieton_data != item_type) {
                *last++ = *charlieton_data;
            }
            ++charlieton_data;
        }
        *last = -1;
    }
    return 2;
}

// Fully recovers the party's HP and FP.
EVT_DEFINE_USER_FUNC(evtTot_FullRecover) {
    auto& pouch = *ttyd::mario_pouch::pouchGetPtr();
    pouch.current_hp = pouch.max_hp;
    pouch.current_fp = pouch.max_fp;
    for (int32_t i = 0; i < 8; ++i) {
        pouch.party_data[i].current_hp = pouch.party_data[i].max_hp;
    }
    return 2;
}

// Returns whether a partner needs unlocking given the reward id.
// arg0 = reward id, arg1 = (out) needs unlock
EVT_DEFINE_USER_FUNC(evtTot_ShouldUnlockPartner) {
    int32_t reward_type = evtGetValue(evt, evt->evtArguments[0]);
    int32_t idx = PartnerRewardTypeToPouchIndex(reward_type);
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
        case REWARD_GOOMBELLA:  str = "Goombella";  break;
        case REWARD_KOOPS:      str = "Koops";      break;
        case REWARD_FLURRIE:    str = "Flurrie";    break;
        case REWARD_YOSHI:      str = "Yoshi";      break;
        case REWARD_VIVIAN:     str = "Vivian";     break;
        case REWARD_BOBBERY:    str = "Bobbery";    break;
        case REWARD_MOWZ:       str = "Ms. Mowz";   break;
    }
    evtSetValue(evt, evt->evtArguments[1], PTR(str));
    return 2;
}

// Initializes party member's stats.
// arg0 = reward id (-1 to -7 in actual game order).
// arg1 = (out) idx in internal party order.
EVT_DEFINE_USER_FUNC(evtTot_InitializePartyMember) {
    int32_t reward_type = evtGetValue(evt, evt->evtArguments[0]);
    int32_t idx = PartnerRewardTypeToPouchIndex(reward_type);
    auto& party_data = ttyd::mario_pouch::pouchGetPtr()->party_data[idx];
    
    party_data.flags |= 1;
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
            case REWARD_GOOMBELLA:  pose_name = "PKR_Y_1";  break;
            case REWARD_KOOPS:      pose_name = "PNK_Y_1";  break;
            case REWARD_FLURRIE:    pose_name = "PWD_Y_1";  break;
            case REWARD_YOSHI:      pose_name = "PYS_Y_1";  break;
            case REWARD_VIVIAN:     pose_name = "PTR_Y_1";  break;
            case REWARD_BOBBERY:    pose_name = "Y_1";      break;
            case REWARD_MOWZ:       pose_name = "PCH_Y_1";  break;
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
    int32_t move_type = evtGetValue(evt, evt->evtArguments[1]);
    
    SelectMoves(move_type, is_upgrade_mode);
    
    evtSetValue(evt, evt->evtArguments[2], g_NumMovesSelected);
    if (g_NumMovesSelected > 0) {
        evtSetValue(evt, evt->evtArguments[3], g_MoveSelections[0]);
        evtSetValue(
            evt, evt->evtArguments[4],
            PTR(ttyd::msgdrv::msgSearch(
                MoveManager::GetMoveData(g_MoveSelections[0])->name_msg)));
    }
    
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_GetStarPieceRolls) {
    int32_t rolls = g_Mod->state_.Rand(3, RNG_STAR_PIECE_CHEST) + 1;
    evtSetValue(evt, evt->evtArguments[0], rolls);
    return 2;
}

}  // namespace mod::tot