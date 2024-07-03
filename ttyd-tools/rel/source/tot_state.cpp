#include "tot_state.h"

#include "common_functions.h"
#include "common_types.h"
#include "evt_cmd.h"
#include "mod.h"

#include <gc/OSLink.h>
#include <gc/OSTime.h>
#include <third_party/fasthash.h>
#include <ttyd/evtmgr_cmd.h>
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

void GetOptionParts(
    uint32_t v, int32_t* t, int32_t* x, int32_t* y, int32_t* a, int32_t* b) {
    *t = GetShiftedBitMask(v, 28, 31);
    // Get the full upper 12 bits if STAT_, otherwise just the lower eight.
    *x = GetShiftedBitMask(v, 20, *t < 4 ? 31 : 27);
    *y = GetShiftedBitMask(v, 16, 19);
    *a = GetShiftedBitMask(v, 8, 15);
    *b = GetShiftedBitMask(v, 0, 7);
}

void EncodeOption(
    int8_t* encoding_bytes, int32_t& encoded_bit_count, uint32_t option) {
    int32_t t, x, y, a, b;
    GetOptionParts(option, &t, &x, &y, &a, &b);
    int32_t bits_left = 6 - (encoded_bit_count % 6);
    int32_t num_bits  = t == TYPE_OPTNUM ? 8 : y;
    int32_t divisor   = t == TYPE_OPTNUM ? a : 1;
    int32_t value = g_Mod->state_.GetOption(option) / divisor;

    while (num_bits > 0) {
        encoding_bytes[encoded_bit_count / 6]
            |= (value << (encoded_bit_count % 6)) & 63;
        if (num_bits > bits_left) {
            encoded_bit_count += bits_left;
            value >>= bits_left;
            bits_left = 6;
        } else {
            encoded_bit_count += num_bits;
        }
        num_bits -= bits_left;
    }
}

// Holds backup save data (updated on floor 0 and after every boss floor).
TotSaveSlot g_BackupSave;
bool g_HasBackupSave = false;

void ComputeChecksum(TotSaveSlot& save) {
    constexpr const char kVersion[] = "009";
    strcpy(save.version, kVersion);
    save.size = sizeof(save.data);
    
    uint32_t checksum = 0U;
    uint8_t* ptr = (uint8_t*)&save.data;
    for (int32_t i = 0; i < (int32_t)sizeof(save.data); i++) {
        checksum += ptr[i];
    }
    save.checksum1 = checksum;
    save.checksum2 = ~save.checksum1;
}

}  // namespace

// Loading / saving functions.
bool StateManager::Load(TotSaveSlot* save) {
    // TODO: Check version to make sure ToT data is valid before loading.
    memcpy(this, &save->data.tot_state, sizeof(StateManager));
    
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
    
    // TODO: Is this required if loading from a hard save?
    // strcpy(mariost->currentAreaName, "123");
    // mariost->pRelFileBase = nullptr;
    
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
    if (GetOption(OPT_RUN_STARTED)) ChangeOption(STAT_RUN_CONTINUES);
    
    return true;
}

void StateManager::Save(TotSaveSlot* save) {
    auto* mariost = ttyd::mariost::g_MarioSt;
    auto* player = ttyd::mario::marioGetPtr();
    
    memset(save, 0, sizeof(TotSaveSlot));
    
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
    
    g_HasBackupSave = true;
}

bool StateManager::HasBackupSave() const {
    return g_HasBackupSave;
}

TotSaveSlot* StateManager::GetBackupSave() const {
    return &g_BackupSave;
}

void StateManager::PickRandomSeed() {
    uint64_t time = gc::OSTime::OSGetTime();
    seed_ = third_party::fasthash64(&time, sizeof(time), 417) % 999'999'999 + 1;
}

void StateManager::InitDefaultOptions() {
    // Pick a random seed, and reset all RNG states to the start.
    PickRandomSeed();
    for (int32_t i = 0; i < RNG_SEQUENCE_MAX; ++i) rng_states_[i] = 0;
    
    // Set floor to 0 (starting floor that only gives a partner).
    floor_ = 0;
    // Set stat upgrades to base # of levels.
    hp_level_ = 2;
    hp_p_level_ = 2;
    fp_level_ = 1;
    bp_level_ = 1;
    max_inventory_ = 6;
    
    memset(option_flags_, 0, sizeof(option_flags_));
    memset(option_bytes_, 0, sizeof(option_bytes_));
    // TODO: Only reset per-run play stats.
    memset(play_stats_, 0, sizeof(play_stats_));

    // Set non-zero default values to their default values.
    SetOption(OPTVAL_PRESET_DEFAULT);
    SetOption(OPTVAL_DIFFICULTY_FULL);
    SetOption(OPTVAL_STARTER_ITEMS_BASIC);
    SetOption(OPTVAL_REVIVE_PARTNERS_ON);
    SetOption(OPTVAL_AC_DEFAULT);
    SetOption(OPTVAL_NPC_LUMPY_ON);
    SetOption(OPTVAL_NPC_DOOPLISS_ON);
    SetOption(OPTVAL_NPC_GRUBBA_ON);
    SetOption(OPTVAL_NPC_CHET_RIPPO_ON);
    SetOption(OPTVAL_NPC_WONKY_ON);
    SetOption(OPTVAL_NPC_DAZZLE_ON);
    SetOption(OPT_MARIO_HP, 5);
    SetOption(OPT_MARIO_FP, 5);
    SetOption(OPT_MARIO_BP, 5);
    SetOption(OPT_PARTNER_HP, 5);
    SetOption(OPT_INVENTORY_SACK_SIZE, 2);
    SetOption(OPTNUM_ENEMY_HP, 100);
    SetOption(OPTNUM_ENEMY_ATK, 100);
    SetOption(OPT_MAX_PARTNERS, 4);
    
    g_HasBackupSave = false;
}

void StateManager::ApplyPresetOptions() {
    switch (GetOptionValue(OPT_PRESET)) {
        case OPTVAL_PRESET_DEFAULT: {
            // Preserve preset, timer and difficulty settings, overwrite others.
            uint32_t difficulty_option = GetOptionValue(OPT_DIFFICULTY);
            uint32_t timer_option = GetOptionValue(OPT_TIMER_DISPLAY);

            memset(option_flags_, 0, sizeof(option_flags_));
            memset(option_bytes_, 0, sizeof(option_bytes_));

            SetOption(OPTVAL_PRESET_DEFAULT);
            SetOption(difficulty_option);
            SetOption(timer_option);

            // Set non-zero default values to their default values.
            SetOption(OPTVAL_STARTER_ITEMS_BASIC);
            SetOption(OPTVAL_REVIVE_PARTNERS_ON);
            SetOption(OPTVAL_AC_DEFAULT);
            SetOption(OPTVAL_NPC_LUMPY_ON);
            SetOption(OPTVAL_NPC_DOOPLISS_ON);
            SetOption(OPTVAL_NPC_GRUBBA_ON);
            SetOption(OPTVAL_NPC_CHET_RIPPO_ON);
            SetOption(OPTVAL_NPC_WONKY_ON);
            SetOption(OPTVAL_NPC_DAZZLE_ON);
            SetOption(OPT_MARIO_HP, 5);
            SetOption(OPT_MARIO_FP, 5);
            SetOption(OPT_MARIO_BP, 5);
            SetOption(OPT_PARTNER_HP, 5);
            SetOption(OPT_INVENTORY_SACK_SIZE, 2);
            SetOption(OPTNUM_ENEMY_HP, 100);
            SetOption(OPTNUM_ENEMY_ATK, 100);
            SetOption(OPT_MAX_PARTNERS, 4);

            break;
        }
    }
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
        case TYPE_FLAGS_MOVE_ENCOUNTERED:
        case TYPE_FLAGS_ITEM_ENCOUNTERED:
        case TYPE_FLAGS_ITEM_PURCHASED:
        case TYPE_FLAGS_OPT_UNLOCKED: {
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
                case TYPE_FLAGS_MOVE_ENCOUNTERED: {
                    ptr = move_encountered_flags_ + (value >> 5);
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
            }
            *ptr |= (1 << (value & 31));
            return true;
        }
        // Play stats.
        default: {
            uint8_t* ptr = play_stats_ + x;
            if (a == 0 && b > 1) {
                // Index into array of values.
                if (index < 0 || index >= b) return false;
                ptr += (index * y);
            } else if (a == 1) {
                // Clamp number to b digits long.
                if (b < 1 || b > 9) return false;
                static const constexpr int32_t powers_of_10[] = {
                    1, 10, 100, 1000, 10'000, 100'000, 1'000'000, 10'000'000,
                    100'000'000, 1'000'000'000
                };
                if (value >= powers_of_10[b]) value = powers_of_10[b] - 1;
                if (value <= -powers_of_10[b]) value = -(powers_of_10[b] - 1);
            }
            uint32_t uint_val = static_cast<uint32_t>(value);
            for (int32_t i = y - 1; i >= 0; --i) {
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
        case TYPE_FLAGS_MOVE_ENCOUNTERED: {
            flag_ptr = move_encountered_flags_ + (x >> 5);
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
        case TYPE_OPTNUM: {
            byte_ptr = option_bytes_ + x;
            break;
        }
        default: {
            byte_ptr = play_stats_ + x;
            break;
        }
    }
    
    if (flag_ptr) {
        const uint32_t start_bit = x & 31;
        return GetShiftedBitMask(*flag_ptr, start_bit, start_bit + y - 1);
    } else if (t == TYPE_OPTNUM) {
        return *byte_ptr * a;
    } else {
        // Index into array, if STAT_x option is an array.
        if (a == 0 && b > 1) {
            // Value is out of range for array.
            if (index < 0 || index >= b) return -1;
            byte_ptr += (index * y);
        }
        // Start with all 0 or 1 bits based on sign of value.
        uint32_t uint_val = (*byte_ptr & 0x80) ? ~0 : 0;
        for (int32_t i = 0; i < y; ++i) {
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

// Returns a string representing the current options encoded.
const char* StateManager::GetEncodedOptions() const {
    static char encoding_str[24] = { 0 };
    int8_t encoding_bytes[24] = { (int8_t)version_, 99 };
    int32_t encoded_bit_count = 12;

    // If a preset is selected, use its name instead.
    switch (GetOptionValue(OPT_PRESET)) {
        case OPTVAL_PRESET_DEFAULT:
            return "Default";
    }

    EncodeOption(encoding_bytes, encoded_bit_count, OPT_BATTLE_DROPS);
    EncodeOption(encoding_bytes, encoded_bit_count, OPT_STARTER_ITEMS);
    EncodeOption(encoding_bytes, encoded_bit_count, OPT_MAX_PARTNERS);
    EncodeOption(encoding_bytes, encoded_bit_count, OPT_PARTNER);
    EncodeOption(encoding_bytes, encoded_bit_count, OPT_REVIVE_PARTNERS);
    EncodeOption(encoding_bytes, encoded_bit_count, OPT_MARIO_HP);
    EncodeOption(encoding_bytes, encoded_bit_count, OPT_MARIO_FP);
    EncodeOption(encoding_bytes, encoded_bit_count, OPT_MARIO_BP);
    EncodeOption(encoding_bytes, encoded_bit_count, OPT_PARTNER_HP);
    EncodeOption(encoding_bytes, encoded_bit_count, OPT_INVENTORY_SACK_SIZE);
    EncodeOption(encoding_bytes, encoded_bit_count, OPTNUM_ENEMY_HP);
    EncodeOption(encoding_bytes, encoded_bit_count, OPTNUM_ENEMY_ATK);
    EncodeOption(encoding_bytes, encoded_bit_count, OPTNUM_SUPERGUARD_SP_COST);
    EncodeOption(encoding_bytes, encoded_bit_count, OPT_AC_DIFFICULTY);
    EncodeOption(encoding_bytes, encoded_bit_count, OPT_BANDIT_ESCAPE);
    EncodeOption(encoding_bytes, encoded_bit_count, OPT_CHARLIETON_STOCK);
    EncodeOption(encoding_bytes, encoded_bit_count, OPT_ENABLE_NPC_WONKY);
    EncodeOption(encoding_bytes, encoded_bit_count, OPT_ENABLE_NPC_DAZZLE);
    EncodeOption(encoding_bytes, encoded_bit_count, OPT_ENABLE_NPC_CHET_RIPPO);
    EncodeOption(encoding_bytes, encoded_bit_count, OPT_ENABLE_NPC_LUMPY);
    EncodeOption(encoding_bytes, encoded_bit_count, OPT_ENABLE_NPC_DOOPLISS);
    EncodeOption(encoding_bytes, encoded_bit_count, OPT_ENABLE_NPC_GRUBBA);

    const int32_t kEncodedByteCount = (encoded_bit_count + 5) / 6;
    for (int32_t i = 0; i < kEncodedByteCount; ++i) {
        if (encoding_bytes[i] < 26) {
            encoding_str[i] = 'A' + encoding_bytes[i];
        } else if (encoding_bytes[i] < 52) {
            encoding_str[i] = 'a' + encoding_bytes[i] - 26;
        } else if (encoding_bytes[i] < 62) {
            encoding_str[i] = '0' + encoding_bytes[i] - 52;
        } else if (encoding_bytes[i] == 62) {
            encoding_str[i] = '!';
        } else if (encoding_bytes[i] == 63) {
            encoding_str[i] = '?';
        } else {
            encoding_str[i] = '.';
        }
    }
    encoding_str[kEncodedByteCount] = 0;

    return encoding_str;
}

void StateManager::IncrementFloor(int32_t change) {
    // Update timer values for the current floor.
    TimerFloorUpdate();
    
    // Make a backup save if advancing, and if the floor is divisible by 8.
    if (floor_ % 8 == 0 && change > 0) {
        Save(&g_BackupSave);
    }
    
    int32_t max_floor = 64;
    switch (GetOptionValue(tot::OPT_DIFFICULTY)) {
        case tot::OPTVAL_DIFFICULTY_TUTORIAL:
            max_floor = 8;
            break;
        case tot::OPTVAL_DIFFICULTY_HALF:
            max_floor = 32;
            break;
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

// Clear play stats, timers, etc. from current run.
void StateManager::ClearRunStats() {
    igt_active_ = false;
    
    run_start_time_rta_ = 0;
    last_floor_rta_ = 0;
    last_floor_total_igt_ = 0;
    last_floor_total_battle_igt_ = 0;
    current_total_igt_ = 0;
    
    for (int32_t i = 0; i < 129; ++i) {
        splits_rta_[i] = 0;
        splits_igt_[i] = 0;
        splits_battle_igt_[i] = 0;
    }
    
    for (int32_t i = 0; i < 0x100; ++i) {
        play_stats_[i] = 0;
    }
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
            case RNG_NPC_RESERVED:
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

EVT_DEFINE_USER_FUNC(evtTot_GetSeed) {
    uint32_t seed = g_Mod->state_.seed_;
    evtSetValue(evt, evt->evtArguments[0], seed);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_GetEncodedOptions) {
    const char* encoded_str = g_Mod->state_.GetEncodedOptions();
    evtSetValue(evt, evt->evtArguments[0], PTR(encoded_str));
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_GetDifficulty) {
    evtSetValue(
        evt, evt->evtArguments[0], g_Mod->state_.GetOptionValue(OPT_DIFFICULTY));
    return 2;
}

}  // namespace mod::tot