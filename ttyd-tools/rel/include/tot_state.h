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
    // StateManager data format revision; used for versioning.
    // Offset: g_Mod->state_ + 0x0000
    uint8_t     version_;
    
    // Whether the in-game run timer is currently active.
    // Offset: g_Mod->state_ + 0x0001
    uint8_t     igt_active_;
    
    // Used to track character stats in a run.
    // Offset: g_Mod->state_ + 0x0002
    int16_t     hp_level_;
    int16_t     fp_level_;
    int16_t     bp_level_;
    int16_t     hp_p_level_;
    int16_t     max_inventory_;
    
    // Used to track tower progression.
    // Offset: g_Mod->state_ + 0x000c
    int32_t     floor_;

    // Saves the current completion progress as an easily accessible integer.
    // Offset: g_Mod->state_ + 0x0010
    int32_t     completion_score_;
    
    // Timer-related information.
    // Offset: g_Mod->state_ + 0x0014
    // Split timers for floors (# of centiseconds).
    uint32_t    splits_rta_[129];
    uint32_t    splits_igt_[129];
    uint32_t    splits_battle_igt_[129];
    // In-game and real-time timers.
    uint64_t    run_start_time_rta_;
    uint64_t    last_floor_rta_;
    uint64_t    last_floor_total_igt_;
    uint64_t    last_floor_total_battle_igt_;
    uint64_t    current_total_igt_;
    uint64_t    unused_timer_;    // Reserved for potential future use.
    
    // RNG information.
    // Offset: g_Mod->state_ + 0x0650
    uint32_t    seed_;
    // Bub-ulb's seed name. Resets between runs.
    char        seed_name_[12];
    uint16_t    rng_states_[56];
    
    // Options data.
    // Offset: g_Mod->state_ + 0x06d0
    uint32_t    option_flags_[8];   // Last 8 bytes reserved for cosmetics.
    uint8_t     option_bytes_[64];
    
    // Permanent tracking bitfields.
    // Offset: g_Mod->state_ + 0x0730
    uint32_t    item_encountered_flags_[8];
    uint32_t    item_purchased_flags_[8];
    uint32_t    achievement_flags_[4];
    uint32_t    option_unlocked_flags_[4];
    uint32_t    cosmetic_purchased_flags_[4];
    uint32_t    midboss_defeated_flags_[4];
    // Reserved in case similar flags are needed in the future.
    uint32_t    reserved_flags_[20];
    
    // Various play stats, pertaining to current run / all time.
    // Offset: g_Mod->state_ + 0x0800
    // The first 0x100 * 4 bytes are used for stats for the current run,
    // including the currently available + selected move levels.
    uint8_t     play_stats_[0x400 * 4];

    // Initializes basic fields the first time a file is created.
    void Init();
    
    // Loading / saving functions; copies all game data, not just ToT state.
    bool Load(TotSaveSlot* save);
    void Save(TotSaveSlot* save);
    bool HasBackupSave() const;
    TotSaveSlot* GetBackupSave() const;

    // Initialize run-specific stats, options, play stats, etc. to defaults.
    void ResetOptions();

    // Sets the current seed to a random value.
    void SelectRandomSeed();
    // Converts the current seed name into an integer value.
    void HashSeedName();
    // Gets the current seed number / name as a string.
    const char* GetSeedAsString() const;
    
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
    
    // Fetches a random value from the desired sequence (using the RngSequence
    // enum), returning a value in the range [0, range). If `sequence` is not
    // a valid enum value, returns a random value using ttyd::system::irand().
    uint32_t Rand(uint32_t range, int32_t sequence = -1);

    // Static; gets parts of an option value's internal representation.
    // Should mostly only be used internally, but exposed for OptionsManager.
    static void GetOptionParts(
        uint32_t v, int32_t* t, int32_t* x, int32_t* y, int32_t* a, int32_t* b);
};
static_assert(sizeof(StateManager) == 0x1800);

// Standard evt wrappers to call certain functions / access fields.

// Calls StateManager.IncrementFloor(arg0).
EVT_DECLARE_USER_FUNC(evtTot_IncrementFloor, 1)

// Returns the current floor number.
EVT_DECLARE_USER_FUNC(evtTot_GetFloor, 1)

// Returns whether the current floor is a special type of room.
// Final boss floor.
EVT_DECLARE_USER_FUNC(evtTot_IsFinalFloor, 1)
// Midboss (not Atomic Boo) floor.
EVT_DECLARE_USER_FUNC(evtTot_IsMidbossFloor, 1)
// Rest floor (no enemies, shop + NPCs only).
EVT_DECLARE_USER_FUNC(evtTot_IsRestFloor, 1)

// Returns the current difficulty setting.
EVT_DECLARE_USER_FUNC(evtTot_GetDifficulty, 1)

// Updates the player's permanent currency in lockstep with current coins.
// arg0 = 0 (coins) or 1 (Star Pieces), arg1 = amount (greater than 0).
EVT_DECLARE_USER_FUNC(evtTot_SpendPermanentCurrency, 2)

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
static_assert(sizeof(TotSaveData) == 0x37d8);

struct TotSaveSlot {
    TotSaveData data;                       // 0x0000
    uint8_t     reserved[0x3800-0x37d8];    // 0x37d8
    uint8_t     padding[0x3ff0-0x3800];     // 0x3800
    
    char        version[4];                 // 0x3ff0
    int32_t     size;                       // 0x3ff4
    uint32_t    checksum1;                  // 0x3ff8
    uint32_t    checksum2;                  // 0x3ffc
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
    RNG_ALTERNATE_BOSS          = 35,
    
    RNG_SEQUENCE_MAX            = 36,
};

// Different types of option values (see below for descriptions of each).
enum OptionsType {
    TYPE_OPT                        = 4,
    TYPE_OPTVAL                     = 5,
    TYPE_OPTNUM                     = 6,
    TYPE_FLAGS_ACHIEVEMENT          = 7,
    TYPE_FLAGS_OPT_UNLOCKED         = 8,
    TYPE_FLAGS_ITEM_ENCOUNTERED     = 9,
    TYPE_FLAGS_ITEM_PURCHASED       = 10,
    TYPE_FLAGS_MIDBOSS_DEFEATED     = 11,
    TYPE_FLAGS_COSMETIC_PURCHASED   = 12,
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
//          W = 8: option_unlocked_flags_
//          W = 9: item_encountered_flags_
//          W = A: item_purchased_flags_
//          W = B: midboss_defeated_flags_
//          W = C: cosmetic_purchased_flags_
//  - STAT_x:   Play stats value:   (0x XXX Y AA BB) with XXX < 0x400;
//      Represents a range of bytes in play_stats_, from index 0xXXX * 4 onward.
//          If Y > 0, Cap value(s) to Y digits in length.
//          If AA & 1, 2, 3, value(s) are 1-3 bytes long instead of default 4.
//          If AA & 0x10, value should be interpreted as signed.
//          If BB > 1, Value is actually an array of BB values.
//  - Other options (0xD0000000+) : reserved for future / other uses.
//
// Intentionally placed in global namespace for convenience.
enum Options : uint32_t {
    // Flag-based options.
    // Select a set of settings all at once, or choose a custom set.
    OPT_PRESET                  = 0x400'3'00'02,
    OPTVAL_PRESET_CUSTOM        = 0x500'3'00'00,
    OPTVAL_PRESET_DEFAULT       = 0x500'3'00'01,
    OPTVAL_PRESET_RTA_RACE      = 0x500'3'00'02,
    // Reserved: Space for extra presets.

    // Tower difficulty.
    OPT_DIFFICULTY              = 0x403'2'01'03,
    OPTVAL_DIFFICULTY_TUTORIAL  = 0x503'2'00'00,    // Not used.
    OPTVAL_DIFFICULTY_HALF      = 0x503'2'00'01,
    OPTVAL_DIFFICULTY_FULL      = 0x503'2'00'02,
    OPTVAL_DIFFICULTY_FULL_EX   = 0x503'2'00'03,

    // Enable timer display.
    OPT_TIMER_DISPLAY           = 0x405'3'00'02,
    OPTVAL_TIMER_NONE           = 0x505'3'00'00,
    OPTVAL_TIMER_IGT            = 0x505'3'00'01,
    OPTVAL_TIMER_RTA            = 0x505'3'00'02,
    // Reserved: Space for extra timer options (countdown timer...?)

    // Determines how many chests appear per floor.
    OPT_NUM_CHESTS              = 0x408'3'00'03,
    OPTVAL_CHESTS_DEFAULT       = 0x508'3'00'00,
    OPTVAL_CHESTS_1             = 0x508'3'00'01,
    OPTVAL_CHESTS_2             = 0x508'3'00'02,
    OPTVAL_CHESTS_3             = 0x508'3'00'03,
    OPTVAL_CHESTS_4             = 0x508'3'00'04,    // Not currently used.
    
    // How enemies hold / drop items, and whether there are battle conditions.
    OPT_BATTLE_DROPS            = 0x40b'2'00'03,
    OPTVAL_DROP_STANDARD        = 0x50b'2'00'00,   // one drop + bonus chance
    OPTVAL_DROP_HELD_FROM_BONUS = 0x50b'2'00'01,   // held drop from bonus
    OPTVAL_DROP_NO_HELD_W_BONUS = 0x50b'2'00'02,   // no held, only bonus
    OPTVAL_DROP_ALL_HELD        = 0x50b'2'00'03,   // all drop + bonus chance

    // Starting item set.
    OPT_STARTER_ITEMS           = 0x40d'3'00'04,
    OPTVAL_STARTER_ITEMS_OFF    = 0x50d'3'00'00,
    OPTVAL_STARTER_ITEMS_BASIC  = 0x50d'3'00'01,
    OPTVAL_STARTER_ITEMS_STRONG = 0x50d'3'00'02,
    OPTVAL_STARTER_ITEMS_RANDOM = 0x50d'3'00'03,
    OPTVAL_STARTER_ITEMS_CUSTOM = 0x50d'3'00'04,

    // Starting partner choice, or no partners.
    OPT_PARTNER                 = 0x410'4'00'07,
    OPTVAL_PARTNER_RANDOM       = 0x510'4'00'00,
    OPTVAL_PARTNER_GOOMBELLA    = 0x510'4'00'01,
    OPTVAL_PARTNER_KOOPS        = 0x510'4'00'02,
    OPTVAL_PARTNER_FLURRIE      = 0x510'4'00'03,
    OPTVAL_PARTNER_YOSHI        = 0x510'4'00'04,
    OPTVAL_PARTNER_VIVIAN       = 0x510'4'00'05,
    OPTVAL_PARTNER_BOBBERY      = 0x510'4'00'06,
    OPTVAL_PARTNER_MOWZ         = 0x510'4'00'07,
    OPTVAL_PARTNER_NONE         = 0x510'4'00'08,    // Not currently used.

    // Maximum number of partners Mario can have at once (0 to 7).
    // If set to 0, "floor 0" will have a jump, hammer or Special Move.
    OPT_MAX_PARTNERS            = 0x414'3'00'07,
    OPTVAL_NO_PARTNERS          = 0x514'3'00'00,

    // Whether to auto-revive partners after finishing a battle.
    OPT_REVIVE_PARTNERS         = 0x417'1'00'01,
    OPTVAL_REVIVE_PARTNERS_OFF  = 0x517'1'00'00,
    OPTVAL_REVIVE_PARTNERS_ON   = 0x517'1'00'01,

    // Stat increase per upgrade (0-10); at 0, you only ever have 1 point.
    // Setting BP increase to "11" treats BP as infinite.
    OPT_MARIO_HP                = 0x418'4'00'0a,
    OPT_MARIO_FP                = 0x41c'4'00'0a,
    OPT_MARIO_BP                = 0x420'4'00'0b,
    OPTVAL_INFINITE_BP          = 0x520'4'00'0b,
    OPT_PARTNER_HP              = 0x424'4'00'0a,
    // Increase per Strange Sack (0-3); base inventory is always 6.
    OPT_INVENTORY_SACK_SIZE     = 0x428'2'00'03,

    // Determines the difficulty of Action Commands.
    OPT_AC_DIFFICULTY           = 0x42a'3'00'06,
    OPTVAL_AC_3_SIMP            = 0x52a'3'00'00,
    OPTVAL_AC_2_SIMP            = 0x52a'3'00'01,
    OPTVAL_AC_1_SIMP            = 0x52a'3'00'02,
    OPTVAL_AC_DEFAULT           = 0x52a'3'00'03,
    OPTVAL_AC_1_UNSIMP          = 0x52a'3'00'04,
    OPTVAL_AC_2_UNSIMP          = 0x52a'3'00'05,
    OPTVAL_AC_3_UNSIMP          = 0x52a'3'00'06,

    // Whether to enable variance on all sources of variable damage.
    OPT_RANDOM_DAMAGE           = 0x42d'2'00'02,
    OPTVAL_RANDOM_DAMAGE_NONE   = 0x52d'2'00'00,
    OPTVAL_RANDOM_DAMAGE_25     = 0x52d'2'00'01,
    OPTVAL_RANDOM_DAMAGE_50     = 0x52d'2'00'02,

    // Whether to have the audience throw random items.
    OPT_AUDIENCE_RANDOM_THROWS  = 0x42f'1'00'01,
    OPTVAL_AUDIENCE_THROWS_OFF  = 0x52f'1'00'00,
    OPTVAL_AUDIENCE_THROWS_ON   = 0x52f'1'00'01,

    // Whether to shuffle the appearance and description of items.
    OPT_OBFUSCATE_ITEMS         = 0x430'1'00'01,
    OPTVAL_OBFUSCATE_ITEMS_OFF  = 0x530'1'00'00,
    OPTVAL_OBFUSCATE_ITEMS_ON   = 0x530'1'00'01,

    // Whether to force the player to refight Bandits with stolen items.
    OPT_BANDIT_ESCAPE           = 0x431'1'00'01,
    OPTVAL_BANDIT_NO_REFIGHT    = 0x531'1'00'00,
    OPTVAL_BANDIT_FORCE_REFIGHT = 0x531'1'00'01,

    // Whether Charlieton should have smaller or limited stock.
    OPT_CHARLIETON_STOCK        = 0x432'2'00'03,
    OPTVAL_CHARLIETON_NORMAL    = 0x532'2'00'00,
    OPTVAL_CHARLIETON_SMALLER   = 0x532'2'00'01,
    OPTVAL_CHARLIETON_TINY      = 0x532'2'00'02,
    OPTVAL_CHARLIETON_LIMITED   = 0x532'2'00'03,

    // Select which NPCs appear in a run, either specific ones, random or none.
    OPT_NPC_CHOICE_1            = 0x434'4'00'09,
    OPT_NPC_CHOICE_2            = 0x438'4'00'09,
    OPT_NPC_CHOICE_3            = 0x43c'4'00'09,
    OPT_NPC_CHOICE_4            = 0x440'4'00'09,

    // Determines whether the secret boss will appear.
    OPT_SECRET_BOSS             = 0x444'2'00'02,
    OPTVAL_SECRET_BOSS_RANDOM   = 0x544'2'00'00,
    OPTVAL_SECRET_BOSS_OFF      = 0x544'2'00'01,
    OPTVAL_SECRET_BOSS_ON       = 0x544'2'00'02,

    // Whether to enable Merlee curses (not supported).
    OPT_MERLEE_CURSE            = 0x446'1'00'01,

    // Changes to stage hazard rates (not supported yet).
    OPT_STAGE_HAZARDS           = 0x447'3'00'04,
    OPTVAL_STAGE_HAZARDS_NORMAL = 0x547'3'00'00,
    OPTVAL_STAGE_HAZARDS_HIGH   = 0x547'3'00'01,
    OPTVAL_STAGE_HAZARDS_LOW    = 0x547'3'00'02,
    OPTVAL_STAGE_HAZARDS_NO_FOG = 0x547'3'00'03,
    OPTVAL_STAGE_HAZARDS_OFF    = 0x547'3'00'04,

    // Whether running away from battle is guaranteed (not supported yet).
    OPT_RUN_AWAY                = 0x44a'2'00'01,
    OPTVAL_RUN_AWAY_DEFAULT     = 0x54a'2'00'00,
    OPTVAL_RUN_AWAY_GUARANTEED  = 0x54a'2'00'01,

    // Move availability (not supported yet).
    OPT_MOVE_AVAILABILITY       = 0x44c'2'00'02,
    OPTVAL_MOVES_DEFAULT        = 0x54c'2'00'00,
    OPTVAL_MOVES_PARTNER_BONUS  = 0x54c'2'00'01,
    OPTVAL_MOVES_RANDOM         = 0x54c'2'00'02,
    OPTVAL_MOVES_CUSTOM         = 0x54c'2'00'03,

    // Move limits (not supported yet).
    OPT_MOVE_LIMIT              = 0x44e'3'00'05,
    OPTVAL_MOVE_LIMIT_DEFAULT   = 0x54e'3'00'00,
    OPTVAL_MOVE_LIMIT_4         = 0x54e'3'00'01,
    OPTVAL_MOVE_LIMIT_3         = 0x54e'3'00'02,
    OPTVAL_MOVE_LIMIT_2         = 0x54e'3'00'03,
    OPTVAL_MOVE_LIMIT_1         = 0x54e'3'00'04,
    OPTVAL_MOVE_LIMIT_0         = 0x54e'3'00'05,

    // Countdown timer (not supported yet).
    OPT_COUNTDOWN_TIMER         = 0x451'4'00'0b,
    OPTVAL_COUNTDOWN_OFF        = 0x451'4'00'00,
    // Next: 0x455
    
    // Internal options; are not automatically reset between runs.
    OPT_RUN_STARTED             = 0x4c0'1'00'01,
    // Special modes for a save file based on its name.
    OPT_SPECIAL_FILE_MODE       = 0x4c1'3'00'03,
    OPTVAL_DEBUG_MODE_ENABLED   = 0x5c1'3'00'01,
    OPTVAL_RACE_MODE_ENABLED    = 0x5c1'3'00'02,
    OPTVAL_100_MODE_ENABLED     = 0x5c1'3'00'03,
    // Other options.
    OPT_UNSEEDED_RUN            = 0x4c4'1'00'01,
    OPT_USE_SEED_NAME           = 0x4c5'1'00'01,
    OPT_SHOP_ITEMS_CHOSEN       = 0x4c6'1'00'01,
    // Next: 0x4c7
    
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
    FLAGS_OPTION_UNLOCKED       = 0x800'0'00'00U,
    FLAGS_ITEM_ENCOUNTERED      = 0x900'0'00'00U,
    FLAGS_ITEM_PURCHASED        = 0xa00'0'00'00U,
    FLAGS_MIDBOSS_DEFEATED      = 0xb00'0'00'00U,
    FLAGS_COSMETIC_PURCHASED    = 0xc00'0'00'00U,
    
    // Play stats.

    // Stats used for a single run.
    // Currently unlocked + selected move levels.
    STAT_RUN_MOVE_LV_UNLOCKED   = 0x000'0'01'60,
    STAT_RUN_MOVE_LV_SELECTED   = 0x018'0'01'60,
    // Tracking for unique badges collected.
    STAT_RUN_UNIQUE_BADGE_FLAGS = 0x030'0'01'0a,
    // Tracking for midbosses and npcs spawned / used.
    STAT_RUN_MIDBOSSES_USED     = 0x034'0'01'10,
    STAT_RUN_NPCS_SELECTED      = 0x038'0'11'10,
    STAT_RUN_NPCS_DEALT_WITH    = 0x03c'0'01'10,
    // Current run's intensity rating.
    STAT_RUN_INTENSITY          = 0x040'3'00'00,
    // Other per-run stats.
    STAT_RUN_TURNS_SPENT        = 0x041'6'00'00,
    STAT_RUN_MOST_TURNS_RECORD  = 0x042'4'00'00,
    STAT_RUN_MOST_TURNS_CURRENT = 0x043'4'00'00,
    STAT_RUN_MOST_TURNS_FLOOR   = 0x044'0'00'00,
    STAT_RUN_TIMES_RAN_AWAY     = 0x045'4'00'00,
    STAT_RUN_CONTINUES          = 0x046'4'00'00,
    STAT_RUN_CONDITIONS_MET     = 0x047'0'00'00,
    STAT_RUN_CONDITIONS_TOTAL   = 0x048'0'00'00,
    STAT_RUN_ENEMY_DAMAGE       = 0x049'7'00'00,
    STAT_RUN_PLAYER_DAMAGE      = 0x04a'7'00'00,
    STAT_RUN_STAR_PIECES        = 0x04b'3'00'00,
    STAT_RUN_SHINE_SPRITES      = 0x04c'3'00'00,
    STAT_RUN_COINS_EARNED       = 0x04d'6'00'00,
    STAT_RUN_COINS_SPENT        = 0x04e'6'00'00,
    STAT_RUN_ITEMS_BOUGHT       = 0x04f'3'00'00,
    STAT_RUN_ITEMS_USED         = 0x050'3'00'00,
    STAT_RUN_FP_SPENT           = 0x051'6'00'00,
    STAT_RUN_SP_SPENT           = 0x052'6'00'00,
    STAT_RUN_SUPERGUARDS        = 0x053'6'00'00,
    STAT_RUN_NPC_DAZZLE_FLOOR   = 0x054'3'00'00,
    STAT_RUN_NPC_GRUBBA_FLOOR   = 0x055'3'00'00,
    STAT_RUN_NPC_DOOPLISS_FLOOR = 0x056'3'00'00,
    STAT_RUN_NPC_LUMPY_FLOOR    = 0x057'3'00'00,
    STAT_RUN_NPC_MOVER_FLOOR    = 0x058'3'00'00,
    STAT_RUN_NPC_LUMPY_COINS    = 0x059'3'00'00,
    STAT_RUN_NPC_SP_PURCHASED   = 0x05a'3'00'00,
    STAT_RUN_NPC_ITEMS_SOLD     = 0x05b'4'00'00,
    STAT_RUN_NPC_BADGES_SOLD    = 0x05c'4'00'00,
    STAT_RUN_NPC_LEVELS_SOLD    = 0x05d'4'00'00,
    // Only used for achievement tracking.
    STAT_RUN_JUMPS_HAMMERS_USED = 0x05e'4'00'00,
    STAT_RUN_BADGES_EQUIPPED    = 0x05f'4'00'00,
    STAT_RUN_INFATUATE_DAMAGE   = 0x060'6'00'00,
    STAT_RUN_TRADE_OFF_ON_BOSS  = 0x061'1'00'00,
    // Used as temporary storage for run rewards.
    STAT_RUN_META_COINS_EARNED  = 0x062'7'00'00,
    STAT_RUN_META_SP_EARNED     = 0x063'7'00'00,
    // Next: 0x064

    // Stats that persist across runs.
    // Bitfields / arrays for permanent progression. 
    STAT_PERM_ENEMY_KILLS       = 0x100'4'02'80,
    STAT_PERM_MOVE_LOG          = 0x140'0'01'60,
    STAT_PERM_REWARDS_OFFERED   = 0x158'7'03'20,
    STAT_PERM_REWARDS_TAKEN     = 0x170'7'03'20,
    STAT_PERM_PARTNERS_OBTAINED = 0x188'0'00'00,
    // Last enemy that attacked the player; only used for NPC chatter.
    STAT_PERM_LAST_ATTACKER     = 0x189'0'00'00,
    // Permanent currencies (coins, Star Pieces, achievement hammers).
    STAT_PERM_CURRENT_COINS     = 0x18a'4'00'00,
    STAT_PERM_CURRENT_SP        = 0x18b'4'00'00,
    STAT_PERM_ACH_HAMMERS       = 0x18c'1'00'00,
    // Custom item/badge loadouts.
    STAT_PERM_ITEM_LOADOUT      = 0x18d'0'01'06,
    STAT_PERM_ITEM_LOAD_SIZE    = 0x18f'1'00'00,
    STAT_PERM_BADGE_LOADOUT     = 0x190'0'01'06,
    STAT_PERM_BADGE_LOAD_SIZE   = 0x192'1'00'00,
    // Hub shop items.
    STAT_PERM_SHOP_ITEMS        = 0x193'0'01'05,
    // Meta-progression stats.
    STAT_PERM_META_COINS_EARNED = 0x195'9'00'00,
    STAT_PERM_META_SP_EARNED    = 0x196'9'00'00,
    // Run stats.
    STAT_PERM_HALF_ATTEMPTS     = 0x197'5'00'00,
    STAT_PERM_HALF_FINISHES     = 0x198'5'00'00,
    STAT_PERM_FULL_ATTEMPTS     = 0x199'5'00'00,
    STAT_PERM_FULL_FINISHES     = 0x19a'5'00'00,
    STAT_PERM_EX_ATTEMPTS       = 0x19b'5'00'00,
    STAT_PERM_EX_FINISHES       = 0x19c'5'00'00,
    STAT_PERM_HALF_BEST_TIME    = 0x19d'9'00'00,
    STAT_PERM_FULL_BEST_TIME    = 0x19e'9'00'00,
    STAT_PERM_EX_BEST_TIME      = 0x19f'9'00'00,
    STAT_PERM_CONTINUES         = 0x1a0'5'00'00,
    STAT_PERM_MAX_INTENSITY     = 0x1a1'3'00'00,
    STAT_PERM_CONDITIONS_MET    = 0x1a2'7'00'00,
    STAT_PERM_CONDITIONS_TOTAL  = 0x1a3'7'00'00,
    STAT_PERM_FLOORS            = 0x1a4'9'00'00,
    STAT_PERM_TURNS_SPENT       = 0x1a5'9'00'00,
    STAT_PERM_TIMES_RAN_AWAY    = 0x1a6'5'00'00,
    STAT_PERM_ENEMIES_DEFEATED  = 0x1a7'9'00'00,
    STAT_PERM_ENEMY_DAMAGE      = 0x1a8'9'00'00,
    STAT_PERM_PLAYER_DAMAGE     = 0x1a9'9'00'00,
    STAT_PERM_COINS_EARNED      = 0x1aa'9'00'00,
    STAT_PERM_COINS_SPENT       = 0x1ab'9'00'00,
    STAT_PERM_ITEMS_BOUGHT      = 0x1ac'9'00'00,
    STAT_PERM_ITEMS_USED        = 0x1ad'9'00'00,
    STAT_PERM_FP_SPENT          = 0x1ae'9'00'00,
    STAT_PERM_SP_SPENT          = 0x1af'9'00'00,
    STAT_PERM_SUPERGUARDS       = 0x1b0'9'00'00,
    STAT_PERM_STAR_PIECES       = 0x1b1'9'00'00,
    STAT_PERM_SHINE_SPRITES     = 0x1b2'9'00'00,
    STAT_PERM_NPC_WONKY_TRADES  = 0x1b3'7'00'00,
    STAT_PERM_NPC_DAZZLE_TRADES = 0x1b4'7'00'00,
    STAT_PERM_NPC_RIPPO_TRADES  = 0x1b5'7'00'00,
    STAT_PERM_NPC_LUMPY_TRADES  = 0x1b6'7'00'00,
    STAT_PERM_NPC_GRUBBA_DEAL   = 0x1b7'7'00'00,
    STAT_PERM_NPC_DOOPLISS_DEAL = 0x1b8'7'00'00,
    STAT_PERM_NPC_MOVER_TRADES  = 0x1b9'7'00'00,
    STAT_PERM_NPC_ZESS_COOKS    = 0x1ba'7'00'00,
    STAT_PERM_NPC_DEALS_TOTAL   = 0x1bb'9'00'00,
    // Added in v1.10; Race preset best times.
    STAT_PERM_HALF_BEST_RTA     = 0x1bc'9'00'00,
    STAT_PERM_FULL_BEST_RTA     = 0x1bd'9'00'00,
    STAT_PERM_EX_BEST_RTA       = 0x1be'9'00'00,
    // Custom move loadout.
    STAT_PERM_MOVE_LOADOUT      = 0x1bf'0'01'08,
    STAT_PERM_MOVE_LOAD_SIZE    = 0x1c1'1'00'00,
    // Next: 0x1c2
};

}  // namespace mod::tot