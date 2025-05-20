#include "tot_state.h"

#include "common_functions.h"
#include "common_types.h"
#include "evt_cmd.h"
#include "mod.h"
#include "tot_gon_tower_npcs.h"
#include "tot_gsw.h"
#include "tot_manager_achievements.h"
#include "tot_manager_debug.h"
#include "tot_manager_options.h"
#include "tot_manager_progress.h"

#include <gc/OSLink.h>
#include <gc/OSTime.h>
#include <third_party/fasthash.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/item_data.h>
#include <ttyd/mario.h>
#include <ttyd/mario_party.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/party.h>
#include <ttyd/system.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::tot {

namespace {

using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

namespace ItemType = ::ttyd::item_data::ItemType;

const int32_t kEarliestSupportedVersion = 10;
const int32_t kCurrentVersion = 11;

// Holds backup save data (updated on floor 0 and after every boss floor).
TotSaveSlot g_BackupSave;
bool g_HasBackupSave = false;

void ComputeChecksum(TotSaveSlot& save) {
    constexpr const char kVersion[] = "010";
    strcpy(save.version, kVersion);

    // Larger than necessary, to future-proof.
    save.size = 0x3800;
    
    uint32_t checksum = 0U;
    uint8_t* ptr = (uint8_t*)&save.data;
    for (int32_t i = 0; i < (int32_t)sizeof(save.data); i++) {
        checksum += ptr[i];
    }
    save.checksum1 = checksum;
    save.checksum2 = ~save.checksum1;
}

}  // namespace

void StateManager::Init() {
    memset(this, 0, sizeof(StateManager));
    version_ = kCurrentVersion;

    // Initialize best run times to maximum time.
    // Default options
    SetOption(STAT_PERM_HALF_BEST_TIME, 100 * 60 * 60 * 100 - 1);
    SetOption(STAT_PERM_FULL_BEST_TIME, 100 * 60 * 60 * 100 - 1);
    SetOption(STAT_PERM_EX_BEST_TIME,   100 * 60 * 60 * 100 - 1);
    // RTA Race / speedrun options
    SetOption(STAT_PERM_HALF_BEST_RTA,  100 * 60 * 60 * 100 - 1);
    SetOption(STAT_PERM_FULL_BEST_RTA,  100 * 60 * 60 * 100 - 1);
    SetOption(STAT_PERM_EX_BEST_RTA,    100 * 60 * 60 * 100 - 1);

    ResetOptions();

    // Enable special file modes, if specific file names are entered.
    if (!strcmp(ttyd::mariost::g_MarioSt->saveFileName, "RaceMode")) {
        SetOption(OPTVAL_RACE_MODE_ENABLED);
        DebugManager::SpecialFileSetup();
    }
    if (!strcmp(ttyd::mariost::g_MarioSt->saveFileName, "YCNOTGBY")) {
        SetOption(OPTVAL_100_MODE_ENABLED);
        DebugManager::SpecialFileSetup();
    }
    if (!strcmp(ttyd::mariost::g_MarioSt->saveFileName, "xyzzy")) {
        SetOption(OPTVAL_DEBUG_MODE_ENABLED);
    }
}

// Loading / saving functions.
bool StateManager::Load(TotSaveSlot* save) {
    int32_t previous_version = save->data.tot_state.version_;

    // Check version to make sure ToT data is valid before loading.
    if (previous_version < kEarliestSupportedVersion)
        return false;

    memcpy(this, &save->data.tot_state, sizeof(StateManager));

    version_ = kCurrentVersion;
    
    auto* mariost = ttyd::mariost::g_MarioSt;
    auto* player = ttyd::mario::marioGetPtr();
    
    // Back up certain fields.
    gc::OSLink::OSModuleInfo* relAlloc = mariost->pMapAlloc;
    void* fbatData = mariost->fbatData;
    uint32_t gp_flag8 = (mariost->flags & 8);
    uint32_t language = mariost->language;
    int32_t unk1 = *(int32_t*)((uintptr_t)mariost + 0x1274);
    int32_t unk2 = *(int32_t*)((uintptr_t)mariost + 0x1294);  // rumble
    int32_t unk3 = *(int32_t*)((uintptr_t)mariost + 0x11b8);
    
    memcpy(
        mariost, 
        &save->data.global_data, 
        sizeof(ttyd::mariost::MarioSt_Globals));
    mariost->fbatData = fbatData;
    
    memcpy(
        ttyd::mario_pouch::pouchGetPtr(),
        &save->data.pouch_data, 
        sizeof(ttyd::mario_pouch::PouchData));
    memcpy(
        ttyd::npcdrv::fbatGetPointer()->deadInfos, 
        save->data.npc_dead_info,
        sizeof(ttyd::npcdrv::FbatDatabaseNpcDeadInfo) * 64);
    memcpy(
        ttyd::evt_badgeshop::g_BadgeShopWork,
        &save->data.badge_shop_work,
        sizeof(ttyd::evt_badgeshop::BadgeShopWork));
        
    *(int32_t*)((uintptr_t)mariost + 0x1278) = 0;
    *(int32_t*)((uintptr_t)mariost + 0x127c) = 0;
    *(int32_t*)((uintptr_t)mariost + 0x1280) = 0;
    mariost->bDvdHasError = 0;
    mariost->lastFrameRetraceTime = gc::OSTime::OSGetTime();
    
    if (save != &g_BackupSave) {
        // Required for curtain transition to end if loading from a hard save.
        strcpy(mariost->currentAreaName, "123");
        mariost->pRelFileBase = nullptr;

        // If in the middle of a run, treat like a continue from a Game Over.
        if (GetOption(OPT_RUN_STARTED)) {
            SetSWByte(GSW_Tower_ContinuingFromGameOver, 1);
        }
    }
    
    *(int32_t*)((uintptr_t)mariost + 0x1274) = unk1;
    *(int32_t*)((uintptr_t)mariost + 0x1294) = unk2;
    *(int32_t*)((uintptr_t)mariost + 0x11b8) = unk3;
    mariost->pMapAlloc = relAlloc;
    
    mariost->unk_0008 = 0;
    mariost->bDebugMode = 0;
    if (gp_flag8 != 0) {
        mariost->flags |= 8;
    }
    else {
        mariost->flags &= ~8;
    }
    mariost->language = language;
    
    // Restore the previous active partner (or none).
    player->prevFollowerId[0] = mariost->saveParty0Id;
    player->prevFollowerId[1] = mariost->saveParty1Id;
    
    // Add 1 to number of times the current run was continued.
    if (GetOption(OPT_RUN_STARTED)) {
        ChangeOption(STAT_RUN_CONTINUES);
        ChangeOption(STAT_PERM_CONTINUES);
        // Decrement so exiting floor after a continue doesn't double-count.
        if (floor_ > 0) ChangeOption(STAT_PERM_FLOORS, -1);
    }
    
    // Make sure partner HP is kept consistent.
    OptionsManager::OnRunResumeFromAutoSave();

    // Version compatibility: fill in missing best RTA times from pre-v1.10.
    if (GetOption(STAT_PERM_HALF_BEST_RTA) == 0) {
        SetOption(STAT_PERM_HALF_BEST_RTA,  100 * 60 * 60 * 100 - 1);
        SetOption(STAT_PERM_FULL_BEST_RTA,  100 * 60 * 60 * 100 - 1);
        SetOption(STAT_PERM_EX_BEST_RTA,    100 * 60 * 60 * 100 - 1);
    }

    // Older version conversion logic.
    if (previous_version == 10) {
        // Revert achievements that are no longer true.
        const int32_t kRevertAch[] = {
            AchievementId::META_ALL_KEY_ITEMS,
            AchievementId::META_ALL_OPTIONS,
            AchievementId::META_ALL_ACHIEVEMENTS,
        };
        for (const auto& ach : kRevertAch) {
            achievement_flags_[ach / 0x20] &= ~(1 << (ach % 0x20));

            // Also unequip respective costumes for Mario / Yoshi.
            const auto* data = AchievementsManager::GetData(ach);
            switch (data->reward_type) {
                case AchievementRewardType::MARIO_COSTUME: {
                    int32_t costume_id = data->reward_id - 1;
                    if (GetSWByte(GSW_MarioCostume) == costume_id) {
                        SetSWByte(GSW_MarioCostume, 0);
                    }
                    break;
                }
                case AchievementRewardType::YOSHI_COSTUME: {
                    int32_t costume_id = data->reward_id - 1;
                    if (GetSWF(GSWF_YoshiColors + costume_id)) {
                        SetSWF(GSWF_YoshiColors + costume_id, 0);
                    }
                    break;
                }
            }
        }

        // Check if automatically eligible for new aggregate achievements.
        AchievementsManager::CheckCompleted(AchievementId::V2_META_USE_ALL_MOVES);
        AchievementsManager::CheckCompleted(AchievementId::V2_AGG_ENEMY_TIMES_100);
        AchievementsManager::CheckCompleted(AchievementId::V2_AGG_RUN_AWAY_30);
        // As well as achievements that've been retroactively made easier.
        AchievementsManager::CheckCompleted(AchievementId::META_TATTLE_LOG_50);

        // Re-run special file setup to collect new achievements, options, etc.
        DebugManager::SpecialFileSetup();
    }
    
    return true;
}

void StateManager::Save(TotSaveSlot* save) {
    auto* mariost = ttyd::mariost::g_MarioSt;
    auto* player = ttyd::mario::marioGetPtr();
    
    memset(save, 0, sizeof(TotSaveSlot));

    // Update the completion percentage before saving.
    ProgressManager::RefreshCache();
    ProgressManager::GetOverallProgression();
    
    mariost->saveLastPlayerPosition.x = player->playerPosition[0];
    mariost->saveLastPlayerPosition.y = player->playerPosition[1];
    mariost->saveLastPlayerPosition.z = player->playerPosition[2];
    
    auto* party = ttyd::party::partyGetPtr(ttyd::mario_party::marioGetPartyId());
    if (party) {
        mariost->saveParty0Id = party->current_member_id;
    } else {
        mariost->saveParty0Id = player->prevFollowerId[0];
    }
    party = ttyd::party::partyGetPtr(ttyd::mario_party::marioGetExtraPartyId());
    if (party) {
        mariost->saveParty1Id = party->current_member_id;
    } else {
        mariost->saveParty1Id = player->prevFollowerId[1];
    }
    
    mariost->lastSaveTime = gc::OSTime::OSGetTime();
    ++mariost->saveCounter;
    mariost->flags &= ~3;
    
    *(int32_t*)((uintptr_t)mariost + 0x1278) = 0;
    *(int32_t*)((uintptr_t)mariost + 0x127c) = 0;
    *(int32_t*)((uintptr_t)mariost + 0x1280) = 0;
    mariost->bDvdHasError = 0;
    
    memcpy(&save->data.tot_state, this, sizeof(StateManager));
    memcpy(
        &save->data.global_data, 
        mariost,
        sizeof(ttyd::mariost::MarioSt_Globals));
    memcpy(
        &save->data.pouch_data, 
        ttyd::mario_pouch::pouchGetPtr(),
        sizeof(ttyd::mario_pouch::PouchData));
    memcpy( 
        save->data.npc_dead_info,
        ttyd::npcdrv::fbatGetPointer()->deadInfos,
        sizeof(ttyd::npcdrv::FbatDatabaseNpcDeadInfo) * 64);
    memcpy(
        &save->data.badge_shop_work,
        ttyd::evt_badgeshop::g_BadgeShopWork,
        sizeof(ttyd::evt_badgeshop::BadgeShopWork));
    
    save->data.flags &= ~3;
    
    ComputeChecksum(*save);
    
    if (save == &g_BackupSave) g_HasBackupSave = true;
}

bool StateManager::HasBackupSave() const {
    return g_HasBackupSave;
}

TotSaveSlot* StateManager::GetBackupSave() const {
    return &g_BackupSave;
}

void StateManager::ResetOptions() {
    
    // Set floor to 0 (starting floor that only gives a partner).
    floor_ = 0;
    // Set stat upgrades to base # of levels.
    hp_level_ = 2;
    hp_p_level_ = 2;
    fp_level_ = 1;
    bp_level_ = 1;
    max_inventory_ = 6;
    
    // Clear all options, except internals.
    memset(option_flags_, 0, sizeof(uint32_t) * 6);
    memset(option_bytes_, 0, sizeof(option_bytes_));
    // Reset per-run play stats.
    memset(play_stats_, 0, 0x100 * 4);

    // Set all options to their default values.
    SetOption(OPTVAL_PRESET_DEFAULT);
    SetOption(OPTVAL_DIFFICULTY_HALF);
    OptionsManager::ApplyCurrentPresetOptions(true);

    // Clear seed information, and reset all RNG states.
    seed_ = 0;
    seed_name_[0] = 0;
    SetOption(OPT_UNSEEDED_RUN, 0);
    SetOption(OPT_USE_SEED_NAME, 0);
    for (int32_t i = 0; i < RNG_SEQUENCE_MAX; ++i) rng_states_[i] = 0;
    
    g_HasBackupSave = false;
}

void StateManager::SelectRandomSeed() {
    uint64_t time = gc::OSTime::OSGetTime();
    seed_ = third_party::fasthash64(&time, sizeof(time), 417) % 999'999'999 + 1;
}

void StateManager::HashSeedName() {
    int32_t number = 0;
    for (const char* ptr = seed_name_; *ptr; ++ptr) {
        // Check to see if seed name is fully numeric and non-zero.
        if (*ptr >= '0' && *ptr <= '9') {
            number = number * 10 + (*ptr - '0');
        } else {
            number = 0;
            break;
        }
    }
    if (number) {
        seed_ = number;
    } else {
        // Hash the seed name into a numeric seed.
        seed_ = third_party::fasthash64(
            seed_name_, sizeof(seed_name_), 417) % 999'999'999 + 1;
    }
}

const char* StateManager::GetSeedAsString() const {
    static char buf[16];
    if (GetOption(OPT_USE_SEED_NAME)) {
        sprintf(buf, "\"%s\"", seed_name_);
    } else if (seed_ == 0) {
        sprintf(buf, "Random on start");
    } else {
        sprintf(buf, "%09" PRId32, seed_);
    }
    return buf;
}

bool StateManager::SetOption(uint32_t option, int32_t value, int32_t index) {
    int32_t t, x, y, a, b;
    GetOptionParts(option, &t, &x, &y, &a, &b);
    
    switch (t) {
        case TYPE_OPT:
        case TYPE_OPTVAL: {
            if (t == TYPE_OPTVAL) {
                // We know that the value must be in range.
                value = b;
            } else if (value < a || value > b) {
                // Value out of range for this flag, leave unchanged.
                return false;
            }
            uint32_t* ptr = option_flags_ + (x >> 5);
            const uint32_t start_bit = x & 31;
            const uint32_t mask = GetBitMask(start_bit, start_bit + y - 1);
            *ptr = (*ptr & ~mask) | (value << start_bit);
            return true;
        }
        case TYPE_OPTNUM: {
            value /= a;
            if (value < y || value > b) return false;
            option_bytes_[x] = value;
            return true;
        }
        case TYPE_FLAGS_ACHIEVEMENT:
        case TYPE_FLAGS_ITEM_ENCOUNTERED:
        case TYPE_FLAGS_ITEM_PURCHASED:
        case TYPE_FLAGS_OPT_UNLOCKED:
        case TYPE_FLAGS_MIDBOSS_DEFEATED:
        case TYPE_FLAGS_COSMETIC_PURCHASED: {
            // Can use either value or manually bit-or'd "b" for setting.
            if (value == 0) value = b;
            // Check for values outside valid range.
            if (value < 0 || value >= 256) return false;
            if (t != TYPE_FLAGS_ITEM_ENCOUNTERED && 
                t != TYPE_FLAGS_ITEM_PURCHASED && value >= 128) return false;
            
            uint32_t* ptr;
            switch (t) {
                case TYPE_FLAGS_ACHIEVEMENT: {
                    ptr = achievement_flags_ + (value >> 5);
                    break;
                }
                case TYPE_FLAGS_ITEM_ENCOUNTERED: {
                    ptr = item_encountered_flags_ + (value >> 5);
                    break;
                }
                case TYPE_FLAGS_ITEM_PURCHASED: {
                    ptr = item_purchased_flags_ + (value >> 5);
                    break;
                }
                case TYPE_FLAGS_OPT_UNLOCKED: {
                    ptr = option_unlocked_flags_ + (value >> 5);
                    break;
                }
                case TYPE_FLAGS_MIDBOSS_DEFEATED: {
                    ptr = midboss_defeated_flags_ + (value >> 5);
                    break;
                }
                case TYPE_FLAGS_COSMETIC_PURCHASED: {
                    ptr = cosmetic_purchased_flags_ + (value >> 5);
                    break;
                }
            }
            *ptr |= (1 << (value & 31));
            return true;
        }
        // Play stats.
        default: {
            uint8_t* ptr = play_stats_ + (x * 4);
            // Value is 4 bytes long, unless A flags 0x3 are set to 1, 2, or 3.
            int32_t val_bytes = (a & 3) ? (a & 3) : 4;
            // Cap value to Y digits long, if applicable.
            if (y > 0) {
                if (y > 9) y = 9;
                static const constexpr int32_t powers_of_10[] = {
                    1, 10, 100, 1000, 10'000, 100'000, 1'000'000, 10'000'000,
                    100'000'000, 1'000'000'000
                };
                if (value >= powers_of_10[y]) value = powers_of_10[y] - 1;
                if (a & 0x10) {
                    // Signed, clamp min at -max.
                    if (value <= -powers_of_10[y]) value = -(powers_of_10[y] - 1);
                } else {
                    // Unsigned, clamp min at 0.
                    if (value < 0) value = 0;
                }
            }
            // Array, index into values.
            if (b > 1) {
                if (index < 0 || index >= b) return false;
                ptr += (index * val_bytes);
            }
            uint32_t uint_val = static_cast<uint32_t>(value);
            for (int32_t i = val_bytes - 1; i >= 0; --i) {
                ptr[i] = uint_val & 0xff;
                uint_val >>= 8;
            }
            return true;
        }
    }
}

bool StateManager::ChangeOption(uint32_t option, int32_t change, int32_t index) {
    int32_t t, x, y, a, b;
    GetOptionParts(option, &t, &x, &y, &a, &b);
    if (t == TYPE_OPTNUM) change *= a;
    int32_t value = GetOption(option, index);
    return SetOption(option, value + change, index);
}

void StateManager::NextOption(uint32_t option, int32_t direction) {
    int32_t t, x, y, a, b;
    GetOptionParts(option, &t, &x, &y, &a, &b);
    if (t == TYPE_OPT) {
        int32_t value = GetOption(option) + direction;
        if (value > b) value = a;
        if (value < a) value = b;
        SetOption(option, value);
    } else if (t == TYPE_OPTNUM) {
        int32_t value = GetOption(option) + direction * a;
        if (value > b * a) value = y * a;
        if (value < y * a) value = b * a;
        SetOption(option, value);
    }
}

int32_t StateManager::GetOption(uint32_t option, int32_t index) const {
    int32_t t, x, y, a, b;
    GetOptionParts(option, &t, &x, &y, &a, &b);
    // OPTVALs are constants, so this doesn't make sense to request.
    if (t == TYPE_OPTVAL) return -1;
    // FLAGS can use value or | with the base flag type.
    if (t >= TYPE_FLAGS_ACHIEVEMENT) {
        x = index == 0 ? b : index;
        y = 1;  // Always requesting a single bit.
    }
    
    const uint32_t* flag_ptr = nullptr;
    const uint8_t* byte_ptr = nullptr;
    switch (t) {
        case TYPE_OPT: {
            flag_ptr = option_flags_ + (x >> 5);
            break;
        }
        case TYPE_FLAGS_ACHIEVEMENT: {
            flag_ptr = achievement_flags_ + (x >> 5);
            break;
        }
        case TYPE_FLAGS_ITEM_ENCOUNTERED: {
            flag_ptr = item_encountered_flags_ + (x >> 5);
            break;
        }
        case TYPE_FLAGS_ITEM_PURCHASED: {
            flag_ptr = item_purchased_flags_ + (x >> 5);
            break;
        }
        case TYPE_FLAGS_OPT_UNLOCKED: {
            flag_ptr = option_unlocked_flags_ + (x >> 5);
            break;
        }
        case TYPE_FLAGS_MIDBOSS_DEFEATED: {
            flag_ptr = midboss_defeated_flags_ + (x >> 5);
            break;
        }
        case TYPE_FLAGS_COSMETIC_PURCHASED: {
            flag_ptr = cosmetic_purchased_flags_ + (x >> 5);
            break;
        }
        case TYPE_OPTNUM: {
            byte_ptr = option_bytes_ + x;
            break;
        }
        default: {
            byte_ptr = play_stats_ + (x * 4);
            break;
        }
    }
    
    if (flag_ptr) {
        const uint32_t start_bit = x & 31;
        return GetShiftedBitMask(*flag_ptr, start_bit, start_bit + y - 1);
    } else if (t == TYPE_OPTNUM) {
        return *byte_ptr * a;
    } else {
        // Play stats.
        // Values are 4 bytes long, unless A flags 0x3 are set to 1, 2, or 3.
        int32_t val_bytes = (a & 3) ? (a & 3) : 4;
        // Index into array, if STAT_x option is an array.
        if (b > 1) {
            // Value is out of range for array.
            if (index < 0 || index >= b) return -1;
            byte_ptr += (index * val_bytes);
        }
        // Treat values as unsigned, unless A flag 0x10 is set.
        uint32_t uint_val = 0;
        if (a & 0x10) {
            uint_val = (*byte_ptr & 0x80) ? ~0 : 0;
        }
        for (int32_t i = 0; i < val_bytes; ++i) {
            uint_val = (uint_val << 8) + *byte_ptr++;
        }
        return static_cast<int32_t>(uint_val);
    }
    // Should never be reachable.
    return -1;
}

uint32_t StateManager::GetOptionValue(uint32_t option) const {
    int32_t t, x, y, a, b;
    GetOptionParts(option, &t, &x, &y, &a, &b);
    // Only TYPE_OPT has OPTVAL values.
    if (t != TYPE_OPT) return -1;
    
    const uint32_t word = option_flags_[x >> 5];
    const uint32_t start_bit = x & 31;
    const int32_t value = GetShiftedBitMask(word, start_bit, start_bit + y - 1);
    return (TYPE_OPTVAL << 28) | (option & 0x0fff'0000) | value;
}

bool StateManager::CheckOptionValue(uint32_t option_value) const {
    int32_t t, x, y, a, b;
    GetOptionParts(option_value, &t, &x, &y, &a, &b);
    if (t != TYPE_OPTVAL) return false;
    
    const uint32_t word = option_flags_[x >> 5];
    const uint32_t start_bit = x & 31;
    const int32_t value = GetShiftedBitMask(word, start_bit, start_bit + y - 1);
    return value == b;
}

void StateManager::GetOptionParts(
    uint32_t v, int32_t* t, int32_t* x, int32_t* y, int32_t* a, int32_t* b) {
    *t = GetShiftedBitMask(v, 28, 31);
    // Get the full upper 12 bits if STAT_, otherwise just the lower eight.
    *x = GetShiftedBitMask(v, 20, *t < 4 ? 31 : 27);
    *y = GetShiftedBitMask(v, 16, 19);
    *a = GetShiftedBitMask(v, 8, 15);
    *b = GetShiftedBitMask(v, 0, 7);
}

void StateManager::IncrementFloor(int32_t change) {
    // Update timer values for the current floor.
    TimerFloorUpdate();
    
    int32_t max_floor = 64;
    switch (GetOptionValue(OPT_DIFFICULTY)) {
        case OPTVAL_DIFFICULTY_TUTORIAL:
            max_floor = 8;
            break;
        case OPTVAL_DIFFICULTY_HALF:
            max_floor = 32;
            break;
    }
    if (floor_ != 0) {
        // Update number of total floors cleared across all runs.
        ChangeOption(STAT_PERM_FLOORS, change);
    }
    
    // Make a backup save if advancing, and if the floor is divisible by 8.
    if (floor_ % 8 == 0 && change > 0) {
        Save(&g_BackupSave);
    }

    floor_ = Clamp(floor_ + change, 0, max_floor);
    
    // Clear RNG state values that should reset every floor.
    for (int32_t i = RNG_ENEMY; i <= RNG_REWARD; ++i) rng_states_[i] = 0;

    // Set current floor's turn count to 0.
    SetOption(STAT_RUN_MOST_TURNS_CURRENT, 0);
    
    auto& pouch = *ttyd::mario_pouch::pouchGetPtr();
    // Set stage rank based on floor.
    pouch.rank = (floor_ - 1) / 16;
    // Display the current floor in place of Star Points, since they're unused.
    pouch.star_points = floor_;
    
    // Revive any fallen partners.
    if (CheckOptionValue(OPTVAL_REVIVE_PARTNERS_ON)) {
        for (int32_t i = 1; i <= 7; ++i) {
            auto& party_data = pouch.party_data[i];
            if ((party_data.flags & 1) && party_data.current_hp == 0) {
                party_data.current_hp = 1;
            }
        }
    }
}

bool StateManager::IsFinalBossFloor(int32_t floor) const {
    if (floor < 0) floor = floor_;
    bool is_full = GetOptionValue(OPT_DIFFICULTY) >= OPTVAL_DIFFICULTY_FULL;
    return floor >= (is_full ? 64 : 32);
}

int32_t StateManager::GetNumFloors() const {
    bool is_full = GetOptionValue(OPT_DIFFICULTY) >= OPTVAL_DIFFICULTY_FULL;
    return is_full ? 64 : 32;
}

// Functions for time-tracking...
void StateManager::TimerStart() {
    auto* mariost = ttyd::mariost::g_MarioSt;
    
    uint64_t current_time = mariost->lastFrameRetraceTime;
    run_start_time_rta_ = current_time;
    last_floor_rta_ = current_time;
    last_floor_total_battle_igt_ =
        mariost->animationTimeIncludingBattle - mariost->animationTimeNoBattle;
    last_floor_total_igt_ = 0;
    current_total_igt_ = 0;
    
    for (int32_t i = 0; i < 129; ++i) {
        splits_rta_[i] = 0;
        splits_igt_[i] = 0;
        splits_battle_igt_[i] = 0;
    }
}

void StateManager::TimerTick() {
    if (igt_active_) {
        current_total_igt_ += 
            ttyd::mariost::g_MarioSt->lastFrameRetraceDeltaTime;
    }
}

void StateManager::TimerFloorUpdate() {
    auto* mariost = ttyd::mariost::g_MarioSt;
    uint64_t current_time = mariost->lastFrameRetraceTime;
    uint64_t current_battle_igt =
        mariost->animationTimeIncludingBattle - mariost->animationTimeNoBattle;
    
    // Update splits for the current floor, if not already set.
    if (splits_rta_[floor_] == 0) {
        splits_rta_[floor_] = DurationTicksToCentiseconds(
            current_time - last_floor_rta_);
        splits_igt_[floor_] = DurationTicksToCentiseconds(
            current_total_igt_ - last_floor_total_igt_);
        splits_battle_igt_[floor_] = DurationTicksToCentiseconds(
            current_battle_igt - last_floor_total_battle_igt_);
    }
    
    last_floor_rta_ = current_time;
    last_floor_total_igt_ = current_total_igt_;
    last_floor_total_battle_igt_ = current_battle_igt;
}

void StateManager::ToggleIGT(bool toggle) {
    igt_active_ = toggle;
}

// Fetches a random value from the desired sequence (using the RngSequence
// enum), returning a value in the range [0, range). If `sequence` is not
// a valid enum value, returns a random value using ttyd::system::irand().
uint32_t StateManager::Rand(uint32_t range, int32_t sequence) {
    if (sequence > RNG_VANILLA && sequence < RNG_SEQUENCE_MAX) {
        uint32_t data[2] = { 0, 0 };
        uint16_t* seq_val = rng_states_ + sequence;
        // Include the sequence id and current position, so the beginnings of
        // different sequences can't end up identical.
        // (e.g. chest random badge rewards + first floor's enemy items)
        data[0] = (*seq_val)++ | (sequence << 16);
        switch (sequence) {
            case RNG_ENEMY:
            case RNG_ENEMY_ITEM:
            case RNG_ENEMY_CONDITION:
            case RNG_ENEMY_CONDITION_ITEM:
            case RNG_NPC_TYPE:
            case RNG_NPC_OPTIONS:
            case RNG_MIDBOSS_MOB:
            case RNG_REWARD: {
                data[1] = floor_;
                break;
            }
            default:
                break;
        }
        return third_party::fasthash64(data, sizeof(data), seed_) % range;
    }
    return ttyd::system::irand(range);
}

EVT_DEFINE_USER_FUNC(evtTot_IncrementFloor) {
    int32_t change = evtGetValue(evt, evt->evtArguments[0]);
    g_Mod->state_.IncrementFloor(change);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_GetFloor) {
    int32_t floor = g_Mod->state_.floor_;
    evtSetValue(evt, evt->evtArguments[0], floor);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_IsFinalFloor) {
    bool is_final_floor = g_Mod->state_.IsFinalBossFloor();
    evtSetValue(evt, evt->evtArguments[0], is_final_floor);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_IsMidbossFloor) {
    bool is_midboss_floor = 
        (g_Mod->state_.floor_ % 8 == 0) && 
        (g_Mod->state_.floor_ % 32 != 0);
    evtSetValue(evt, evt->evtArguments[0], is_midboss_floor);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_IsRestFloor) {
    auto& state = g_Mod->state_;
    int32_t floor = state.floor_;
    bool is_rest_floor = floor == 0 || (floor % 8 == 7);
    evtSetValue(evt, evt->evtArguments[0], is_rest_floor);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_GetDifficulty) {
    evtSetValue(
        evt, evt->evtArguments[0], g_Mod->state_.GetOptionValue(OPT_DIFFICULTY));
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_SpendPermanentCurrency) {
    int32_t currency_type = evtGetValue(evt, evt->evtArguments[0]);
    int32_t amount = evtGetValue(evt, evt->evtArguments[1]);
    if (currency_type == 0) {
        g_Mod->state_.ChangeOption(STAT_PERM_CURRENT_COINS, -amount);
    } else if (currency_type == 1) {
        g_Mod->state_.ChangeOption(STAT_PERM_CURRENT_SP, -amount);
    }
    return 2;
}

}  // namespace mod::tot