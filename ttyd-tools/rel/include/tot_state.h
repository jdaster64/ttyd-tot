#pragma once

#include "evt_cmd.h"
#include "tot_manager_move.h"

#include <ttyd/evt_badgeshop.h>
#include <ttyd/evtmgr.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/mariost.h>
#include <ttyd/npcdrv.h>

#include <cstdint>

namespace mod::tot {

struct TotSaveSlot;
    
class StateManager {
public:
    // State revision; will eventually be used for versioning.
    uint8_t     version_ = 1;
    
    // Whether in-game run timer is currently active.
    uint8_t     igt_active_;
    
    // Used by RewardManager to track character stats in a run.
    int16_t     hp_level_;
    int16_t     fp_level_;
    int16_t     bp_level_;
    int16_t     hp_p_level_;
    int16_t     max_inventory_;
    
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
    uint16_t    rng_states_[56];
    
    // In-game and real-time timers.
    uint64_t    run_start_time_rta_;
    uint64_t    last_floor_rta_;
    uint64_t    last_floor_total_igt_;
    uint64_t    last_floor_total_battle_igt_;
    uint64_t    current_total_igt_;
    // Split timers for floors (# of centiseconds).
    uint32_t    splits_rta_[129];
    uint32_t    splits_igt_[129];
    uint32_t    splits_battle_igt_[129];
    uint32_t    reserved_pad_;
    
    // Permanent tracking data.
    uint32_t    achievement_flags_[4];
    uint32_t    move_encountered_flags_[4];
    uint32_t    item_encountered_flags_[8];
    uint32_t    item_purchased_flags_[8];
    uint32_t    option_unlocked_flags_[4];
    
    // Saves various stats for current runs and all-time.
    uint8_t     play_stats_[1024];
    
    // Loading / saving functions; copies all game data, not just ToT state.
    bool Load(TotSaveSlot* save);
    void Save(TotSaveSlot* save);
    bool HasBackupSave() const;
    TotSaveSlot* GetBackupSave() const;

    // Picks a random seed.
    void PickRandomSeed(); 
    // Initialize all settings to default.
    void InitDefaultOptions();
    // Resets settings based on the currently selected preset.
    void ApplyPresetOptions();
    
    // Sets / adjusts options, play stats, achievements, etc.
    // If OPTVAL is provided for 'SetOption', value parameter is ignored.
    // `index` is used for STAT_x options that represent arrays.
    // All flag / numeric options saturate at both ends.
    // Returns false if the value is invalid or was not changed.
    bool SetOption(uint32_t option, int32_t value = 0, int32_t index = 0);
    bool ChangeOption(uint32_t option, int32_t change = 1, int32_t index = 0);

    // Changes the option by 1 in either direction, wrapping around once it 
    // hits its min or max.  Should only be used for menus, and only supports 
    // OPT and OPTNUM type options.
    void NextOption(uint32_t option, int32_t direction);
    
    // Gets the numeric value of options, play stats, achievements, etc.
    // `index` is only used as a parameter for option types that require it
    // (flag arrays or STAT_x arrays).
    int32_t GetOption(uint32_t option, int32_t index = 0) const;
    
    // Returns values as / checks values against OPTVAL.
    uint32_t GetOptionValue(uint32_t option) const;
    bool CheckOptionValue(uint32_t option_value) const;
    
    // Returns a string representing the current options encoded.
    const char* GetEncodedOptions() const;
    
    // Sets/increments the current tower floor, and makes any necessary changes.
    void IncrementFloor(int32_t change = 1);
    // Returns whether the floor (or current, if -1) contains the final boss.
    bool IsFinalBossFloor(int32_t floor = -1) const;
    // Returns how many floors the tower has.
    int32_t GetNumFloors() const;
    
    // Functions for time-tracking...
    void TimerStart();
    void TimerTick();
    void TimerFloorUpdate();
    void ToggleIGT(bool toggle);
    
    // Clear play stats, timers, etc. from current run.
    void ClearRunStats();
    
    // Fetches a random value from the desired sequence (using the RngSequence
    // enum), returning a value in the range [0, range). If `sequence` is not
    // a valid enum value, returns a random value using ttyd::system::irand().
    uint32_t Rand(uint32_t range, int32_t sequence = -1);
};
static_assert(sizeof(StateManager) == 0xc00);

// Standard evt wrappers to call certain functions / access fields.

// Calls StateManager.IncrementFloor(arg0).
EVT_DECLARE_USER_FUNC(evtTot_IncrementFloor, 1)

// Returns the current floor number.
EVT_DECLARE_USER_FUNC(evtTot_GetFloor, 1)

// Returns the current seed.
EVT_DECLARE_USER_FUNC(evtTot_GetSeed, 1)

// Returns a string encoding the currently selected options.
EVT_DECLARE_USER_FUNC(evtTot_GetEncodedOptions, 1)

// Returns the current difficulty setting.
EVT_DECLARE_USER_FUNC(evtTot_GetDifficulty, 1)

// Format of save data used by Tower of Trials mod.
struct TotSaveData {
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
};
static_assert(sizeof(TotSaveData) == 0x2bd8);

struct TotSaveSlot {
    TotSaveData data;                   // 0x0000
    uint8_t     pad2[0x3ff0-0x2bd8];    // 0x2bd8
    
    char        version[4];             // 0x3ff0
    int32_t     size;                   // 0x3ff4
    uint32_t    checksum1;              // 0x3ff8
    uint32_t    checksum2;              // 0x3ffc
};
static_assert(sizeof(TotSaveSlot) == 0x4000);

enum RngSequence {
    // "Totally random"; not reproducible run-to-run.
    RNG_VANILLA                 = -1,   // Calls ttyd::system::irand().
    
    // Not currently used.
    RNG_RESERVED                = 0,
    
    // Battle generation; mangled with floor number + reset every floor.
    RNG_ENEMY                   = 1,    // Enemy loadout + which one drops item.
    RNG_ENEMY_ITEM              = 2,    // Enemy held item types.
    RNG_ENEMY_CONDITION         = 3,    // Bonus challenge condition.
    RNG_ENEMY_CONDITION_ITEM    = 4,    // Bonus challenge reward.
    
    // NPC generation; mangled with floor number + reset every floor.
    RNG_NPC_TYPE                = 5,    // Type of NPC(s) to spawn.
    RNG_NPC_OPTIONS             = 6,    // NPC parameters, e.g. shop items.
    RNG_NPC_RESERVED            = 7,    //
    
    // Choosing reward metatypes; mangled with floor number + reset every floor.
    RNG_REWARD                  = 8,    // Types of rewards.
    // Choosing reward subtypes; not mangled with floor number.
    RNG_REWARD_MOVE             = 9,    // Jump, Hammer, Special, or partner.
    RNG_REWARD_PARTNER          = 10,   // Partner, from all seven options.
    RNG_REWARD_PARTNER_UNUSED_1 = 11,   // Partner, alternate RNG (placeholder).
    RNG_REWARD_PARTNER_UNUSED_2 = 12,   // Partner, alternate RNG (placeholder).
    RNG_REWARD_STAT_UP          = 13,   // HP, FP, BP, HP-P, or item inv.
    RNG_REWARD_OTHER            = 14,   // Coins, SP, SS, unique/stackable badge.
    RNG_REWARD_BADGE_SPECIAL    = 15,   // Which unique badge.
    RNG_REWARD_BADGE_NORMAL     = 16,   // Which stackable badge.
    
    // Order to offer moves for unlocking; not mangled with floor number.
    RNG_MOVE_GOOMBELLA          = 17,
    RNG_MOVE_KOOPS              = 18,
    RNG_MOVE_FLURRIE            = 19,
    RNG_MOVE_YOSHI              = 20,
    RNG_MOVE_VIVIAN             = 21,
    RNG_MOVE_BOBBERY            = 22,
    RNG_MOVE_MOWZ               = 23,
    RNG_MOVE_JUMP               = 24,
    RNG_MOVE_HAMMER             = 25,
    RNG_MOVE_SPECIAL            = 26,
    // Order to offer moves for upgrading; not mangled with floor number.
    // Chooses from all moves together, rolling until valid option found.
    RNG_MOVE_UPGRADE            = 27,
    
    // Miscellaneous uses; not mangled with floor number.
    RNG_STARTER_ITEM            = 28,
    RNG_STOLEN_ITEM             = 29,
    RNG_AUDIENCE_ITEM           = 30,
    RNG_ITEM_OBFUSCATION        = 31,
    RNG_SECONDARY_NPC           = 32,
    RNG_STAR_PIECE_CHEST        = 33,
    RNG_MYSTERY_COOK_RESULT     = 34,
    
    RNG_SEQUENCE_MAX            = 35,
};

// Different types of option values (see below for descriptions of each).
enum OptionsType {
    TYPE_OPT                    = 4,
    TYPE_OPTVAL                 = 5,
    TYPE_OPTNUM                 = 6,
    TYPE_FLAGS_ACHIEVEMENT      = 7,
    TYPE_FLAGS_MOVE_ENCOUNTERED = 8,
    TYPE_FLAGS_ITEM_ENCOUNTERED = 9,
    TYPE_FLAGS_ITEM_PURCHASED   = 10,
    TYPE_FLAGS_OPT_UNLOCKED     = 11,
};

// An enumeration of all options (flags, option values, numeric values), 
// tracking flags, and play stats used in StateManager.
//
// Types of options / encoding:
//  - OPT_x:    Flag option type    (0x4 XX Y AA BB);
//      Represents option_flags_ bits [XX, XX+Y), which can take values from
//      [AA, BB]. Bits used for an option should not cross a word boundary.
//  - OPTVAL_x: Flag option value   (0x5 XX Y 00 VV);
//      Represents option_flags_ bits [XX, XX+Y), set to value VV.
//      Can be used to more efficiently check the exact value of an 'enum' flag.
//  - OPTNUM_x: Bytes option value  (0x6 XX Z MM VV);
//      Represents option_bytes_ byte XX, which can take values Z ~ VV.
//      (Getting / setting values uses a scale of MM times those values.)
//  - FLAGS_x:  Tracking flags:     (0xW 00 0 00 VV);
//      Represents VV'th bit in one of the tracking bitfields, based on W:
//          W = 7: achievement_flags_
//          W = 8: move_encountered_flags_
//          W = 9: item_encountered_flags_
//          W = A: item_purchased_flags_
//          W = B: option_unlocked_flags_
//  - STAT_x:   Play stats value:   (0x XXX Y ZZ VV);
//      Represents play_stats_ bytes [XXX, XXX+Y) (in the range 0x000 ~ 0x400).
//          If ZZ = 0: Value is uncapped; if VV > 1, array of VVx Y-byte values.
//          If ZZ = 1: Value is capped to +/- value with VV (1-9) digits.
//  - Other options (0xC0000000+) : reserved for future / other uses.
//
// Intentionally placed in global namespace for convenience.
enum Options : uint32_t {
    // Flag-based options.
    // Select a set of settings all at once.
    OPT_PRESET                  = 0x400'2'00'01,
    OPTVAL_PRESET_CUSTOM        = 0x500'2'00'00,
    OPTVAL_PRESET_DEFAULT       = 0x500'2'00'01,
    // Reserved: OPTVAL_PRESET_RACE, possibly another?
    // Tower difficulty.
    OPT_DIFFICULTY              = 0x402'2'01'03,
    OPTVAL_DIFFICULTY_TUTORIAL  = 0x502'2'00'00,
    OPTVAL_DIFFICULTY_HALF      = 0x502'2'00'01,
    OPTVAL_DIFFICULTY_FULL      = 0x502'2'00'02,
    OPTVAL_DIFFICULTY_FULL_EX   = 0x502'2'00'03,
    // Starting partner choice, or no partners.
    OPT_PARTNER                 = 0x404'4'00'07,
    OPTVAL_PARTNER_RANDOM       = 0x504'4'00'00,
    OPTVAL_PARTNER_GOOMBELLA    = 0x504'4'00'01,
    OPTVAL_PARTNER_KOOPS        = 0x504'4'00'02,
    OPTVAL_PARTNER_FLURRIE      = 0x504'4'00'03,
    OPTVAL_PARTNER_YOSHI        = 0x504'4'00'04,
    OPTVAL_PARTNER_VIVIAN       = 0x504'4'00'05,
    OPTVAL_PARTNER_BOBBERY      = 0x504'4'00'06,
    OPTVAL_PARTNER_MOWZ         = 0x504'4'00'07,
    OPTVAL_PARTNER_NONE         = 0x504'4'00'08,    // Not used yet.
    // Starting item set.
    OPT_STARTER_ITEMS           = 0x408'3'00'03,
    OPTVAL_STARTER_ITEMS_OFF    = 0x508'3'00'00,
    OPTVAL_STARTER_ITEMS_BASIC  = 0x508'3'00'01,
    OPTVAL_STARTER_ITEMS_STRONG = 0x508'3'00'02,
    OPTVAL_STARTER_ITEMS_RANDOM = 0x508'3'00'03,
    // Reserved: OPTVAL_STARTER_ITEMS_CUSTOM
    // How enemies hold / drop items, and whether there are battle conditions.
    OPT_BATTLE_DROPS            = 0x40b'2'00'03,
    OPTVAL_DROP_STANDARD        = 0x50b'2'00'00,   // one drop + bonus chance
    OPTVAL_DROP_HELD_FROM_BONUS = 0x50b'2'00'01,   // held drop from bonus
    OPTVAL_DROP_NO_HELD_W_BONUS = 0x50b'2'00'02,   // no held, only bonus
    OPTVAL_DROP_ALL_HELD        = 0x50b'2'00'03,   // all drop + bonus chance
    // Enable timer display.
    OPT_TIMER_DISPLAY           = 0x40d'3'00'02,
    OPTVAL_TIMER_NONE           = 0x50d'3'00'00,
    OPTVAL_TIMER_IGT            = 0x50d'3'00'01,
    OPTVAL_TIMER_RTA            = 0x50d'3'00'02,
    // Reserved: several options for countdown timers?
    // Whether to enable Merlee curses.
    OPT_MERLEE_CURSE            = 0x410'1'00'01,
    // Whether to have the audience throw random items.
    OPT_AUDIENCE_RANDOM_THROWS  = 0x411'1'00'01,
    // Whether to enable variance on all sources of variable damage.
    OPT_RANDOM_DAMAGE           = 0x412'2'00'02,
    OPTVAL_RANDOM_DAMAGE_NONE   = 0x512'2'00'00,
    OPTVAL_RANDOM_DAMAGE_25     = 0x512'2'00'01,
    OPTVAL_RANDOM_DAMAGE_50     = 0x512'2'00'02,
    // Changes to stage hazard rates.
    OPT_STAGE_HAZARDS           = 0x414'3'00'04,
    OPTVAL_STAGE_HAZARDS_NORMAL = 0x514'3'00'00,
    OPTVAL_STAGE_HAZARDS_HIGH   = 0x514'3'00'01,
    OPTVAL_STAGE_HAZARDS_LOW    = 0x514'3'00'02,
    OPTVAL_STAGE_HAZARDS_NO_FOG = 0x514'3'00'03,
    OPTVAL_STAGE_HAZARDS_OFF    = 0x514'3'00'04,
    // Whether to shuffle the appearance and description of items.
    OPT_OBFUSCATE_ITEMS         = 0x417'1'00'01,
    // Whether to auto-revive partners after finishing a battle.
    OPT_REVIVE_PARTNERS         = 0x418'1'00'01,
    OPTVAL_REVIVE_PARTNERS_OFF  = 0x518'1'00'00,
    OPTVAL_REVIVE_PARTNERS_ON   = 0x518'1'00'01,
    // Whether Charlieton should have smaller or limited stock.
    OPT_CHARLIETON_STOCK        = 0x419'2'00'02,
    OPTVAL_CHARLIETON_NORMAL    = 0x519'2'00'00,
    OPTVAL_CHARLIETON_SMALLER   = 0x519'2'00'01,
    OPTVAL_CHARLIETON_LIMITED   = 0x519'2'00'02,
    // Whether to force the player to refight Bandits with stolen items.
    OPT_BANDIT_ESCAPE           = 0x41b'1'00'01,
    OPTVAL_BANDIT_NO_REFIGHT    = 0x51b'1'00'00,
    OPTVAL_BANDIT_FORCE_REFIGHT = 0x51b'1'00'01,
    // Stat increase per upgrade (0-10); at 0, you only ever have 1 point.
    // BP increase of "11" treats BP as infinite.
    OPT_MARIO_HP                = 0x41c'4'00'0a,
    OPT_PARTNER_HP              = 0x420'4'00'0a,
    OPT_MARIO_FP                = 0x424'4'00'0a,
    OPT_MARIO_BP                = 0x428'4'00'0b,
    OPTVAL_INFINITE_BP          = 0x528'4'00'0b,
    // Increase per Strange Sack (0-3); base inventory is always 6.
    OPT_INVENTORY_SACK_SIZE     = 0x42c'2'00'03,
    // Booleans for enabling individual NPCs.
    OPT_ENABLE_NPC_LUMPY        = 0x42e'1'00'01,
    OPTVAL_NPC_LUMPY_OFF        = 0x52e'1'00'00,
    OPTVAL_NPC_LUMPY_ON         = 0x52e'1'00'01,
    OPT_ENABLE_NPC_DOOPLISS     = 0x42f'1'00'01,
    OPTVAL_NPC_DOOPLISS_OFF     = 0x52f'1'00'00,
    OPTVAL_NPC_DOOPLISS_ON      = 0x52f'1'00'01,
    OPT_ENABLE_NPC_GRUBBA       = 0x430'1'00'01,
    OPTVAL_NPC_GRUBBA_OFF       = 0x530'1'00'00,
    OPTVAL_NPC_GRUBBA_ON        = 0x530'1'00'01,
    OPT_ENABLE_NPC_CHET_RIPPO   = 0x431'1'00'01,
    OPTVAL_NPC_CHET_RIPPO_OFF   = 0x531'1'00'00,
    OPTVAL_NPC_CHET_RIPPO_ON    = 0x531'1'00'01,
    OPT_ENABLE_NPC_WONKY        = 0x432'1'00'01,
    OPTVAL_NPC_WONKY_OFF        = 0x532'1'00'00,
    OPTVAL_NPC_WONKY_ON         = 0x532'1'00'01,
    OPT_ENABLE_NPC_DAZZLE       = 0x433'1'00'01,
    OPTVAL_NPC_DAZZLE_OFF       = 0x533'1'00'00,
    OPTVAL_NPC_DAZZLE_ON        = 0x533'1'00'01,
    OPT_ENABLE_NPC_MOVER        = 0x434'1'00'01,
    OPTVAL_NPC_MOVER_OFF        = 0x534'1'00'00,
    OPTVAL_NPC_MOVER_ON         = 0x534'1'00'01,
    OPT_ENABLE_NPC_ZESS_T       = 0x435'1'00'01,
    OPTVAL_NPC_ZESS_T_OFF       = 0x535'1'00'00,
    OPTVAL_NPC_ZESS_T_ON        = 0x535'1'00'01,
    // Determines the maximum number of partners Mario can have in a run,
    // from 1-7 (default of 4).
    OPT_MAX_PARTNERS            = 0x436'3'00'07,
    OPTVAL_NO_PARTNERS          = 0x536'3'00'00,
    // Determines the difficulty of Action Commands.
    OPT_AC_DIFFICULTY           = 0x439'3'00'06,
    OPTVAL_AC_3_SIMP            = 0x539'3'00'00,
    OPTVAL_AC_2_SIMP            = 0x539'3'00'01,
    OPTVAL_AC_1_SIMP            = 0x539'3'00'02,
    OPTVAL_AC_DEFAULT           = 0x539'3'00'03,
    OPTVAL_AC_1_UNSIMP          = 0x539'3'00'04,
    OPTVAL_AC_2_UNSIMP          = 0x539'3'00'05,
    OPTVAL_AC_3_UNSIMP          = 0x539'3'00'06,
    // Next: 0x43c
    
    // Internal / cosmetic flag options.
    OPT_RUN_STARTED             = 0x460'1'00'01,
    OPT_DEBUG_MODE_USED         = 0x461'1'00'01,
    OPT_BGM_DISABLED            = 0x462'1'00'01,
    
    // Numeric options.
    // Global enemy HP and ATK scaling (0.05x ~ 10.00x in increments of 0.05).
    OPTNUM_ENEMY_HP             = 0x600'1'05'c8,
    OPTNUM_ENEMY_ATK            = 0x601'1'05'c8,
    // Superguard SP cost (0.00 ~ 1.00 in increments of 0.01).
    OPTNUM_SUPERGUARD_SP_COST   = 0x602'0'01'64,
    // Next: 0x603
    
    // Different types of boolean tracking flags.
    // Can only be set to true; bitwise OR the value or just use 'value' param.
    FLAGS_ACHIEVEMENT           = 0x700'0'00'00,
    FLAGS_MOVE_ENCOUNTERED      = 0x800'0'00'00U,
    FLAGS_ITEM_ENCOUNTERED      = 0x900'0'00'00U,
    FLAGS_ITEM_PURCHASED        = 0xa00'0'00'00U,
    FLAGS_OPTION_UNLOCKED       = 0xb00'0'00'00U,
    
    // Play stats.
    STAT_RUN_TURNS_SPENT        = 0x000'3'01'06,
    STAT_RUN_MOST_TURNS_RECORD  = 0x003'2'01'04,
    STAT_RUN_MOST_TURNS_CURRENT = 0x005'2'01'04,
    STAT_RUN_MOST_TURNS_FLOOR   = 0x007'1'00'00,
    STAT_RUN_TIMES_RAN_AWAY     = 0x008'2'01'04,
    STAT_RUN_ENEMY_DAMAGE       = 0x00a'3'01'06,
    STAT_RUN_PLAYER_DAMAGE      = 0x00d'3'01'06,
    STAT_RUN_ITEMS_USED         = 0x010'2'01'04,
    STAT_RUN_STAR_PIECES        = 0x012'2'01'03,
    STAT_RUN_SHINE_SPRITES      = 0x014'2'01'03,
    STAT_RUN_COINS_EARNED       = 0x016'3'01'06,
    STAT_RUN_COINS_SPENT        = 0x019'3'01'06,
    STAT_RUN_FP_SPENT           = 0x01c'3'01'06,
    STAT_RUN_SP_SPENT           = 0x01f'3'01'06,
    STAT_RUN_SUPERGUARDS        = 0x022'3'01'06,
    STAT_RUN_NPCS_SELECTED      = 0x025'1'00'08,
    STAT_RUN_NPC_SP_PURCHASED   = 0x02d'1'01'03,
    STAT_RUN_NPC_DAZZLE_FLOOR   = 0x02e'1'01'03,
    STAT_RUN_NPC_GRUBBA_FLOOR   = 0x02f'1'01'03,
    STAT_RUN_NPC_DOOPLISS_FLOOR = 0x030'1'01'03,
    STAT_RUN_NPC_LUMPY_FLOOR    = 0x031'1'01'03,
    STAT_RUN_NPC_MOVER_FLOOR    = 0x032'1'01'03,
    STAT_RUN_NPC_LUMPY_COINS    = 0x033'2'01'03,
    STAT_RUN_NPC_ITEMS_SOLD     = 0x035'2'01'04,
    STAT_RUN_NPC_BADGES_SOLD    = 0x037'2'01'04,
    STAT_RUN_NPC_LEVELS_SOLD    = 0x039'2'01'04,
    STAT_RUN_CONDITIONS_MET     = 0x03b'1'00'00,
    STAT_RUN_CONDITIONS_TOTAL   = 0x03c'1'00'00,
    STAT_RUN_UNIQUE_BADGE_FLAGS = 0x03d'1'00'0a,
    STAT_RUN_MIDBOSSES_USED     = 0x047'1'00'07,
    STAT_RUN_CONTINUES          = 0x04e'2'01'03,

    // Next: 0x050
    // TODO: Add versions of most stats that persist across runs.
};

}  // namespace mod::tot