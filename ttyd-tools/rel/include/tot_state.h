#pragma once

#include "tot_move_manager.h"

#include <ttyd/evt_badgeshop.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/mariost.h>
#include <ttyd/npcdrv.h>

#include <cstdint>

namespace mod::tot {

struct TotSaveSlot;
    
class StateManager {
public:
    // State revision; will eventually be used for versioning.
    uint8_t     version_;
    
    // Whether in-game run timer is currently active.
    uint8_t     igt_active_;
    
    // Used by RewardManager to track character stats in a run.
    int16_t     hp_level_;
    int16_t     fp_level_;
    int16_t     bp_level_;
    int16_t     hp_p_level_;
    int16_t     num_sack_upgrades_;
    
    // Used to track tower progression.
    int32_t     floor_;
    
    // Used by MoveManager to track unlocked / selected levels of moves in a run.
    int8_t      level_unlocked_[MoveType::MOVE_TYPE_MAX];
    int8_t      level_selected_[MoveType::MOVE_TYPE_MAX];
    
    // Options / saved data.
    uint32_t    option_flags_[4];   // Last 4 bytes reserved for cosmetics?
    uint8_t     option_bytes_[64];
    
    // RNG information.
    uint32_t    seed_;
    uint16_t    rng_states_half_[32];
    uint8_t     rng_states_byte_[48];
    
    // In-game and real-time timers.
    uint64_t    run_start_time_rta_;
    uint64_t    last_save_time_rta_;
    uint64_t    current_total_igt_;
    uint64_t    current_battle_igt_;
    // Split timers for floors (# of centiseconds).
    uint32_t    start_time_igt_[130];
    uint32_t    end_time_igt_[130];
    uint32_t    battle_duration_igt_[130];
    
    // Permanent tracking data.
    uint32_t    achievement_flags_[4];
    uint32_t    move_encountered_flags_[4];
    uint32_t    item_encountered_flags_[8];
    uint32_t    item_purchased_flags_[8];
    uint32_t    option_unlocked_flags_[4];
    
    // Saves various stats for current runs and all-time.
    uint8_t     play_stats_[1024];
    
    // Loading / saving functions; copies all game data, not just ToT state.
    // bool Load(TotSaveSlotData* save);
    // void Save(TotSaveSlotData* save);
    // TotSaveSlotData* GetBackupSave();
    
    // Initialize all settings to default.
    // void InitDefaultOptions();
    
    // Sets / adjusts options, play stats, achievements, etc.
    // If OPTVAL is provided for 'SetOption', value parameter is ignored.
    // All flag / numeric options saturate at both ends.
    // void SetOption(int32_t option, int32_t value = 0);
    // void ChangeOption(int32_t option, int32_t change = 1);
    
    // Gets the numeric value of options, play stats, achievements, etc.
    // 'value' is only used as a parameter for option types that require it.
    // int32_t GetOption(int32_t option, int32_t value = 0) const;
    
    // Returns values as / checks values against OPTVAL.
    // int32_t GetOptionValue(int32_t option) const;
    // bool CheckOptionValue(int32_t option_value) const;
    
    // Gets menu information (raw strings, not msg keys) for a given option.
    // void GetOptionStrings(
    //      int32_t option, char* name_buf, char* value_buf, int32_t* cost,
    //      bool* unlocked, bool* default, bool* affects_seeding) const;
    
    // Returns a string representing the current options encoded.
    // const char* GetEncodedOptions() const;
    
    // Updates necessary fields for a new floor of the tower.
    // By default, increments the floor by 1.
    // void SetFloor(int32_t floor = -1);
    
    // Functions for time-tracking...
    // void StartTimer();
    // void UpdateTimer();
    void ToggleIGT(bool toggle);
    
    // Clear play stats, timers, etc. from current run.
    // void ClearRunStats();
    
    // Fetches a random value from the desired sequence (using the RngSequence
    // enum), returning a value in the range [0, range). If `sequence` is not
    // a valid enum value, returns a random value using ttyd::system::irand().
    // uint32_t Rand(uint32_t range, int32_t sequence = -1);
};
static_assert(sizeof(StateManager) == 0xc00);

// Format of save data used by Tower of Trials mod.
struct TotSaveSlot {
    uint16_t    flags;                  // 0x0
    uint8_t     pad1[6];                // 0x2
    
    // Vanilla save data.
    ttyd::mariost::MarioSt_Globals          global_data;        // 0x8
    ttyd::mario_pouch::PouchData            pouch_data;         // 0x13e0
    ttyd::npcdrv::FbatDatabaseNpcDeadInfo   npc_dead_info[64];  // 0x19b4
    ttyd::evt_badgeshop::BadgeShopWork      badge_shop_work;    // 0x1eb4
    // PiantaParlorWork, JohoyaWork excluded as they're unused.
    
    // Tower of Trials save data.
    mod::tot::StateManager tot_state;   // 0x1fd8
    
    uint8_t     pad2[0x3ff0-0x2bd8];    // 0x2bd8
    
    char        version[4];             // 0x3ff0
    int32_t     size;                   // 0x3ff4
    uint32_t    checksum1;              // 0x3ff8
    uint32_t    checksum2;              // 0x3ffc
};
static_assert(sizeof(TotSaveSlot) == 0x4000);

// TODO: New RngSequence enum.

// TODO: New Options enum.

}