#include "patches_field_start.h"

#include "common_types.h"
#include "custom_chest_reward.h"
#include "custom_item.h"
#include "evt_cmd.h"
#include "mod.h"
#include "mod_state.h"
#include "patch.h"
#include "patches_options.h"
#include "patches_partner.h"

#include <ttyd/cardmgr.h>
#include <ttyd/evt_bero.h>
#include <ttyd/evt_mario.h>
#include <ttyd/evt_memcard.h>
#include <ttyd/evt_mobj.h>
#include <ttyd/evt_pouch.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/swdrv.h>

#include <cstdint>

namespace mod::infinite_pit {

namespace {

using ::ttyd::evt_bero::BeroEntry;
using ::ttyd::evtmgr::EvtEntry;
using ::ttyd::mario_pouch::PouchData;

namespace ItemType = ::ttyd::item_data::ItemType;

}

// Function hooks.
extern int32_t (*g_memcard_write_trampoline)(EvtEntry*, bool);
extern int32_t (*g_memcard_code_trampoline)(EvtEntry*, bool);
// Patch addresses.
extern const int32_t g_tik_06_PitBeroEntryOffset;
extern const int32_t g_tik_06_RightBeroEntryOffset;

namespace field_start {
    
namespace {
    
// Tracks whether the player is being prompted to save before entering the Pit.
bool g_ReadyForPitSaveWrite = false;
bool g_AttemptingPitSaveWrite = false;
bool g_SuccessfullySaved = false;
    
// Declarations for USER_FUNCs.
EVT_DECLARE_USER_FUNC(CheckHasSaved, 1)
EVT_DECLARE_USER_FUNC(InitOptionsOnPitEntry, 5)
EVT_DECLARE_USER_FUNC(IncrementYoshiColor, 0)

// Event that runs when taking the pipe to enter the Pit.
EVT_BEGIN(PitStartPipeEvt)
USER_FUNC(CheckHasSaved, LW(0))
IF_EQUAL(LW(0), 0)
    // If the player hasn't yet saved, force them to save to start the RTA timer
    // (and save their options if the player has to restart before floor 10).
    RUN_CHILD_EVT(ttyd::evt_mobj::mobj_save_blk_sysevt)
    SET(LW(0), 1)
ELSE()
    // Good to go; do any necessary setup for the selected options.
    // Parameters are dummy values, and get overwritten during execution.
    USER_FUNC(InitOptionsOnPitEntry, 0, 0, 0, 0, 0)
    SET(LW(0), 0)
END_IF()
RETURN()
EVT_END()

// Event that runs when taking the right loading zone in the pre-Pit room.
EVT_BEGIN(PrePitRoomLoopEvt)
USER_FUNC(IncrementYoshiColor)
RETURN()
EVT_END()

EVT_DEFINE_USER_FUNC(CheckHasSaved) {
    if (g_ReadyForPitSaveWrite &&
        !g_Mod->state_.GetOptionNumericValue(OPT_HAS_STARTED_RUN)) {
        // If save already declined, return true.
        ttyd::evtmgr_cmd::evtSetValue(evt, evt->evtArguments[0], true);
    } else {
        // Otherwise, return whether the player has saved successfully.
        ttyd::evtmgr_cmd::evtSetValue(
            evt, evt->evtArguments[0], g_SuccessfullySaved);
    }
    g_ReadyForPitSaveWrite = true;
    return 2;
}

// Initializes all selected options on initially entering the Pit.
EVT_DEFINE_USER_FUNC(InitOptionsOnPitEntry) {
    StateManager_v2& state = g_Mod->state_;
    PouchData& pouch = *ttyd::mario_pouch::pouchGetPtr();
    
    // Initialize number of upgrades per partner.
    const int32_t starting_rank = state.GetOptionNumericValue(OPT_PARTNER_RANK);
    for (int32_t i = 0; i < 7; ++i) {
        state.partner_upgrades_[i] = starting_rank;
    }
    switch (state.GetOptionValue(OPT_PARTNERS_OBTAINED)) {
        case OPTVAL_PARTNERS_ALL_START: {
            // Enable and initialize all partners.
            for (int32_t i = 1; i <= 7; ++i) {
                evt->evtArguments[0] = i;
                ttyd::evt_pouch::evt_pouch_party_join(evt, isFirstCall);
                partner::InitializePartyMember(evt, isFirstCall);
            }
            // Put Goombella on the field.
            evt->evtArguments[0] = 0;
            evt->evtArguments[1] = 1;
            evt->evtArguments[2] = 0;
            evt->evtArguments[3] = 0;
            evt->evtArguments[4] = 0;
            ttyd::evt_mario::evt_mario_set_party_pos(evt, isFirstCall);
            // Do NOT disable partners from the reward pool; they will be
            // replaced with extra Shine Sprites.
            break;
        }
        case OPTVAL_PARTNERS_ONE_START: {
            // Enable and initialize a random partner.
            int32_t starting_partner = -PickPartnerReward();
            evt->evtArguments[0] = starting_partner;
            ttyd::evt_pouch::evt_pouch_party_join(evt, isFirstCall);
            partner::InitializePartyMember(evt, isFirstCall);
            // Place partner on the field.
            evt->evtArguments[0] = 0;
            evt->evtArguments[1] = starting_partner;
            evt->evtArguments[2] = 0;
            evt->evtArguments[3] = 0;
            evt->evtArguments[4] = 0;
            ttyd::evt_mario::evt_mario_set_party_pos(evt, isFirstCall);
            break;
        }
        case OPTVAL_PARTNERS_NEVER: {
            // See if the player selected a single starting partner.
            int32_t first_partner = 0;
            switch (state.GetOptionValue(OPT_FIRST_PARTNER)) {
                case OPTVAL_GOOMBELLA_FIRST:    first_partner = 1; break;
                case OPTVAL_KOOPS_FIRST:        first_partner = 2; break;
                case OPTVAL_FLURRIE_FIRST:      first_partner = 5; break;
                case OPTVAL_YOSHI_FIRST:        first_partner = 4; break;
                case OPTVAL_VIVIAN_FIRST:       first_partner = 6; break;
                case OPTVAL_BOBBERY_FIRST:      first_partner = 3; break;
                case OPTVAL_MS_MOWZ_FIRST:      first_partner = 7; break;
            }
            if (first_partner) {
                evt->evtArguments[0] = first_partner;
                ttyd::evt_pouch::evt_pouch_party_join(evt, isFirstCall);
                partner::InitializePartyMember(evt, isFirstCall);
                // Place partner on the field.
                evt->evtArguments[0] = 0;
                evt->evtArguments[1] = first_partner;
                evt->evtArguments[2] = 0;
                evt->evtArguments[3] = 0;
                evt->evtArguments[4] = 0;
                ttyd::evt_mario::evt_mario_set_party_pos(evt, isFirstCall);
            } else {
                // No partners selected, so running with Mario alone.
                
                // Mark HP Plus P off in badge log, since HP Plus is marked
                // by default and HP Plus P can't be obtained.
                ttyd::swdrv::swSet(
                    0x80 + ItemType::HP_PLUS_P - ItemType::POWER_JUMP);
            }
            break;
        }
    }
    
    // Set up starting item inventory.
    switch (state.GetOptionValue(OPT_STARTER_ITEMS)) {
        case OPTVAL_STARTER_ITEMS_BASIC: {
            ttyd::mario_pouch::pouchGetItem(ItemType::THUNDER_BOLT);
            ttyd::mario_pouch::pouchGetItem(ItemType::FIRE_FLOWER);
            ttyd::mario_pouch::pouchGetItem(ItemType::HONEY_SYRUP);
            ttyd::mario_pouch::pouchGetItem(ItemType::MUSHROOM);
            break;
        }
        case OPTVAL_STARTER_ITEMS_STRONG: {
            ttyd::mario_pouch::pouchGetItem(ItemType::LIFE_SHROOM);
            ttyd::mario_pouch::pouchGetItem(ItemType::CAKE);
            ttyd::mario_pouch::pouchGetItem(ItemType::THUNDER_RAGE);
            ttyd::mario_pouch::pouchGetItem(ItemType::SHOOTING_STAR);
            ttyd::mario_pouch::pouchGetItem(ItemType::MAPLE_SYRUP);
            ttyd::mario_pouch::pouchGetItem(ItemType::SUPER_SHROOM);
            break;
        }
        case OPTVAL_STARTER_ITEMS_RANDOM: {
            // Set sequence forward so the items won't be the same as floor 1's.
            state.rng_sequences_[RNG_ITEM] = 100;
            // Give 4 to 6 random items, based on the seed.
            int32_t num_items = state.Rand(3, RNG_ITEM) + 4;
            for (int32_t i = 0; i < num_items; ++i) {
                int32_t item_type = PickRandomItem(RNG_ITEM, 10, 5, 0, 0);
                ttyd::mario_pouch::pouchGetItem(item_type);
            }
            break;
        }
    }
    
    // Start with 99 BP if using either variant of no-EXP mode.
    if (state.GetOptionNumericValue(OPT_NO_EXP_MODE)) {
        pouch.level = 99;
        pouch.unallocated_bp += 90;
        pouch.total_bp += 90;
    }
    
    // Start with Merlee curse, if enabled.
    if (state.GetOptionNumericValue(OPT_MERLEE_CURSE)) {
        pouch.merlee_curse_uses_remaining = 99;
        pouch.turns_until_merlee_activation = -1;
    } else {
        pouch.merlee_curse_uses_remaining = 0;
        pouch.turns_until_merlee_activation = 0;
    }
    
    // Start at max stage-rank, if enabled.
    if (state.CheckOptionValue(OPTVAL_STAGE_RANK_ALWAYSMAX)) {
        ttyd::mario_pouch::pouchGetPtr()->rank = 3;
    }
    
    // Always start with Sweet Treat Lv.1, Earth Tremor Lv.1, and 1.50 SP.
    ttyd::mario_pouch::pouchGetItem(ItemType::MAGICAL_MAP);
    ttyd::mario_pouch::pouchGetItem(ItemType::DIAMOND_STAR);
    pouch.max_sp = 150;
    pouch.current_sp = 150;
    pouch.star_powers_obtained |= 3;
    state.star_power_levels_ = 0b0101;
    
    options::ApplySettingBasedPatches();
    
    // Set the 'run started' flag to true, and start the run timer, if the
    // player didn't choose to save.
    if (!state.GetOptionNumericValue(OPT_HAS_STARTED_RUN)) {
        state.SetOption(OPT_HAS_STARTED_RUN, true);
        g_Mod->state_.SaveCurrentTime(/* pit_start = */ true);
    }
    
    // All other options are handled immediately on setting them,
    // or are checked explicitly every time they are relevant.
    return 2;
}

// Increments the randomly selected Yoshi color & marks it as manually changed.
EVT_DEFINE_USER_FUNC(IncrementYoshiColor) {
    g_Mod->state_.SetOption(OPT_YOSHI_COLOR_SELECT, true);
    int32_t color = ttyd::mario_pouch::pouchGetPartyColor(4);
    ttyd::mario_pouch::pouchSetPartyColor(4, (color + 1) % 7);
    return 2;
}

}

void ApplyFixedPatches() {
    g_memcard_write_trampoline = patch::hookFunction(
        ttyd::evt_memcard::memcard_write,
        [](EvtEntry* evt, bool isFirstCall) {
            if (g_ReadyForPitSaveWrite && !g_AttemptingPitSaveWrite) {
                // Save the timestamp you entered the Pit.
                g_AttemptingPitSaveWrite = true;
                g_Mod->state_.SaveCurrentTime(/* pit_start = */ true);
                // Mark run as "started", assuming the save will succeed.
                g_Mod->state_.SetOption(OPT_HAS_STARTED_RUN, true);
                // Copy state to save file location.
                g_Mod->state_.Save();
            }
            return g_memcard_write_trampoline(evt, isFirstCall);
        });
        
    g_memcard_code_trampoline = patch::hookFunction(
        ttyd::evt_memcard::memcard_code,
        [](EvtEntry* evt, bool isFirstCall) {
            if (!ttyd::cardmgr::cardIsExec()) {
                if (g_AttemptingPitSaveWrite && !ttyd::cardmgr::cardGetCode()) {
                    g_SuccessfullySaved = true;
                } else {
                    g_AttemptingPitSaveWrite = false;
                }
            }
            return g_memcard_code_trampoline(evt, isFirstCall);
        });
}

void ApplyModuleLevelPatches(void* module_ptr, ModuleId::e module_id) {
    // Clear the flags for tracking save status every time a module is loaded.
    g_ReadyForPitSaveWrite = false;
    g_AttemptingPitSaveWrite = false;
    g_SuccessfullySaved =
        g_Mod->state_.GetOptionNumericValue(OPT_HAS_STARTED_RUN);
    
    if (module_id != ModuleId::TIK || !module_ptr) return;
    const uint32_t module_start = reinterpret_cast<uint32_t>(module_ptr);
    
    // Run custom event code when entering the Pit pipe.
    BeroEntry* tik_06_pipe_bero = reinterpret_cast<BeroEntry*>(
        module_start + g_tik_06_PitBeroEntryOffset);
    tik_06_pipe_bero->out_evt_code = reinterpret_cast<void*>(
        const_cast<int32_t*>(PitStartPipeEvt));
    
    // Make tik_06 (pre-Pit room)'s right exit loop back to itself.
    BeroEntry* tik_06_e_bero = reinterpret_cast<BeroEntry*>(
        module_start + g_tik_06_RightBeroEntryOffset);
    tik_06_e_bero->target_map = "tik_06";
    tik_06_e_bero->target_bero = tik_06_e_bero->name;
    tik_06_e_bero->out_evt_code = reinterpret_cast<void*>(
        const_cast<int32_t*>(PrePitRoomLoopEvt));
}

}  // namespace field_start
}  // namespace mod::infinite_pit