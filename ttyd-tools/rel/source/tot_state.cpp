#include "tot_state.h"

#include "common_functions.h"
#include "common_types.h"

#include <gc/OSTime.h>
#include <third_party/fasthash.h>
#include <ttyd/system.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::tot {

namespace {

}  // namespace

// Loading / saving functions.
// bool StateManager::Load(TotSaveSlotData* save);
// void StateManager::Save(TotSaveSlotData* save);
// TotSaveSlotData* StateManager::GetBackupSave();

// Initialize all settings to default.
void StateManager::InitDefaultOptions() {
    // TODO: Move this stuff elsewhere, as relevant.
    floor_ = 0;
    hp_level_ = 2;
    fp_level_ = 1;
    bp_level_ = 1;
    hp_p_level_ = 2;
    num_sack_upgrades_ = 0;
}

// Sets / adjusts options, play stats, achievements, etc.
// If OPTVAL is provided for 'SetOption', value parameter is ignored.
// All flag / numeric options saturate at both ends.
// void StateManager::SetOption(int32_t option, int32_t value = 0);
// void StateManager::ChangeOption(int32_t option, int32_t change = 1);

// Gets the numeric value of options, play stats, achievements, etc.
// 'value' is only used as a parameter for option types that require it.
// int32_t StateManager::GetOption(int32_t option, int32_t value = 0) const;

// Returns values as / checks values against OPTVAL.
// int32_t StateManager::GetOptionValue(int32_t option) const;
// bool StateManager::CheckOptionValue(int32_t option_value) const;

// Gets menu information (raw strings, not msg keys) for a given option.
// void StateManager::GetOptionStrings(
//      int32_t option, char* name_buf, char* value_buf, int32_t* cost,
//      bool* unlocked, bool* default, bool* affects_seeding) const;

// Returns a string representing the current options encoded.
// const char* StateManager::GetEncodedOptions() const;

// Updates necessary fields for a new floor of the tower.
// By default, increments the floor by 1.
// void StateManager::SetFloor(int32_t floor = -1);

// Functions for time-tracking...
// void StateManager::StartTimer();
// void StateManager::UpdateTimer();

void StateManager::ToggleIGT(bool toggle) {
    igt_active_ = toggle;
}

// Clear play stats, timers, etc. from current run.
// void StateManager::ClearRunStats();

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
            case RNG_NPC_RESERVED: {
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

}  // namespace mod::tot