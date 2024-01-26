#pragma once

#include <cstdint>

namespace mod::infinite_pit {

class StateManager_v2 {
public:
    // Save file revision; makes it possible to add fields while maintaining
    // backwards compatibility, and detect when a vanilla file is loaded.
    // Current version = 8, compatible versions = 7-8 (2.30 onward).
    uint8_t     version_;
    
    // Game state / progression.
    uint8_t     partner_upgrades_[7];   // Includes Super- and Ultra- rank.
    int32_t     floor_;
    uint32_t    reward_flags_;          // Everything but Star Powers.
    uint16_t    star_power_levels_;
    uint8_t     padding_[2];
    // RNG state.
    uint32_t    filename_seed_;
    uint16_t    rng_sequences_[28];
    // Game timers.
    uint64_t    pit_start_time_;
    uint64_t    last_save_time_;
    // Option values & gameplay stats; all accessed by new Options enum.
    uint32_t    option_flags_[4];       // Last 4 bytes reserved for cosmetics.
    uint8_t     option_bytes_[32];
    uint8_t     play_stats_  [64];

    // Initializes the mod's state based on the current save file.
    // Returns whether the mod was successfully initialized.
    bool Load(bool new_save);
    // Copies the mod's state to g_MarioSt so it can be saved.
    void Save();
    
    // Gets the maximum level of a particular Star Power.
    int32_t GetStarPowerLevel(int32_t star_power_type) const;
    
    // Fetches a random value from the desired sequence (using the RngSequence
    // enum), returning a value in the range [0, range). If `sequence` is not
    // a valid enum value, returns a random value using ttyd::system::irand().
    uint32_t Rand(uint32_t range, int32_t sequence = -1);
    
    // Set all options / play stats to 0.
    void ClearAllOptions();
    // Set all non-cosmetic options to their default values.
    void SetDefaultOptions();
    // Changes the selected option/stat by the given amount.
    // Flag options will wrap by default, and increment if `change` is 0;
    // numeric options will saturate and will not increment if `change` is 0.
    void ChangeOption(int32_t option, int32_t change = 1);
    // Sets the selected option/stat to a particular value.
    // If option is set to an OPTVAL_xxx, 'value' is ignored.
    void SetOption(int32_t option, int32_t value = 0);
    // Returns the appropriate flag / numeric value for the specified option.
    // Returns -1 if the option doesn't exist or if the wrong type is requested.
    int32_t GetOptionValue(int32_t option) const;
    int32_t GetOptionFlagValue(int32_t option) const;
    int32_t GetOptionNumericValue(int32_t option) const;
    // Checks whether a flag option has a particular value.
    bool CheckOptionValue(int32_t option_value) const;
    // Returns strings for the given menu option and its current value,
    // and whether the value is default / affects seeding in any way.
    void GetOptionStrings(
        int32_t option, char* name_buf, char* value_buf,
        bool* is_default, bool* affects_seeding) const;

    // Returns a string representing all of this file's user-selectable options.
    const char* GetEncodedOptions() const;
    // Gets a string containing all the saved gameplay stats on this save file.
    // Returns false if no play stats are present on this save file.
    bool GetPlayStatsString(char* out_buf);
    
    // Saves the current clock time (used for calculating RTA time since start).
    void SaveCurrentTime(bool pit_start = false);
    // Get the current RTA play time as a string.
    // Will return the empty string if the start time was unset or incompatible.
    const char* GetCurrentTimeString();
    
    // Enables standard settings for community races, including showing ATK/DEF,
    // and disables the menu so changes cannot be made.
    void EnableRaceOptions();
} __attribute__((__packed__));

static_assert(sizeof(StateManager_v2) <= 0x120);

// An enumeration of all flag options, flag option values, numeric options,
// and gameplay stats used in StateManager_v2.
//
// Types of options / encoding:
//  - OPT_BLAH:     Flag option type    (0x1 XX Y W ZZZ);
//      Represents the range of bits [XX, XX+Y) and having ZZZ possible values.
//      Bits used for a single option should not cross a word boundary.
//  - OPTVAL_BLAH:  Flag option value   (0x2 XX Y W ZZZ);
//      Represents the range of bits [XX, XX+Y) being set to the value ZZZ.
//  - OPTNUM_BLAH:  Bytes option value  (0x3 XX Y W ZZZ);
//      Represents the range of option_bytes_ from offset [XX, XX+Y).
//  - STAT_BLAH:    Play stats value    (0x4 XX Y W ZZZ);
//      Represents the range of play_stats_ from offset [XX, XX+Y).
//  - Negative values: Not valid for flags/options/stats, may be used for
//    other purposes.
//
//  Use of "W" bits:
//      If (W & 7) == 0 or 1: 
//          Numeric options / play stats are clamped to the range [0 or 1, ZZZ].
//      If (W & 7) == 2:
//          Numeric options / play stats are clamped to Z digits (from 1-9).
//      If (W & 7) == 3:
//          Numeric options / play stats are clamped to +/- Z digits (from 1-9).
//      If W & 8 != 0, the option affects seeding.
// 
// Intentionally placed in global namespace for convenience.
enum Options_v2 {
    // Non-cosmetic (menu-visible) flag-based options.
    // How many rewards are obtained per chest (1 to 7, or 0 = random).
    OPT_CHEST_REWARDS           = 0x1'00'4'8'008,
    OPTVAL_CHEST_REWARDS_RANDOM = 0x2'00'4'8'000,
    // If enabled, Mario starts at level 99 with base HP/FP and 99/Infinite BP.
    OPT_NO_EXP_MODE             = 0x1'04'2'0'003,
    OPTVAL_NO_EXP_MODE_OFF      = 0x2'04'2'0'000,
    OPTVAL_NO_EXP_MODE_ON       = 0x2'04'2'0'001,
    OPTVAL_NO_EXP_MODE_INFINITE = 0x2'04'2'0'002,
    // How enemies hold / drop items, and whether there are challenges.
    OPT_BATTLE_REWARD_MODE      = 0x1'06'2'8'004,
    OPTVAL_DROP_STANDARD        = 0x2'06'2'8'000,   // 1 drop + bonus chance
    OPTVAL_DROP_HELD_FROM_BONUS = 0x2'06'2'8'001,   // held drop from bonus
    OPTVAL_DROP_NO_HELD_W_BONUS = 0x2'06'2'8'002,   // no held, only bonus
    OPTVAL_DROP_ALL_HELD        = 0x2'06'2'8'003,   // all drop + bonus chance
    // How partners are obtained.
    OPT_PARTNERS_OBTAINED       = 0x1'08'2'8'004,
    OPTVAL_PARTNERS_ALL_REWARDS = 0x2'08'2'8'000,   // all from chest rewards
    OPTVAL_PARTNERS_ONE_START   = 0x2'08'2'8'001,   // one at start, rest later
    OPTVAL_PARTNERS_ALL_START   = 0x2'08'2'8'002,   // all at start
    OPTVAL_PARTNERS_NEVER       = 0x2'08'2'8'003,   // no partner rewards
    // Which rank partners start at.
    OPT_PARTNER_RANK            = 0x1'0a'2'0'003,
    OPTVAL_PARTNER_RANK_NORMAL  = 0x2'0a'2'0'000,
    OPTVAL_PARTNER_RANK_SUPER   = 0x2'0a'2'0'001,
    OPTVAL_PARTNER_RANK_ULTRA   = 0x2'0a'2'0'002,
    // How high a level of each badge-based move can be used.
    OPT_BADGE_MOVE_LEVEL        = 0x1'0c'2'0'004,
    OPTVAL_BADGE_MOVE_1X        = 0x2'0c'2'0'000,   // up to 1x badges equipped
    OPTVAL_BADGE_MOVE_2X        = 0x2'0c'2'0'001,   // up to 2x badges equipped
    OPTVAL_BADGE_MOVE_RANK      = 0x2'0c'2'0'002,   // up to Mario's rank + 1
    OPTVAL_BADGE_MOVE_INFINITE  = 0x2'0c'2'0'003,   // up to lv. 99 always
    // Which set of items the player starts the pit with.
    OPT_STARTER_ITEMS           = 0x1'0e'2'0'004,
    OPTVAL_STARTER_ITEMS_OFF    = 0x2'0e'2'0'000,
    OPTVAL_STARTER_ITEMS_BASIC  = 0x2'0e'2'0'001,
    OPTVAL_STARTER_ITEMS_STRONG = 0x2'0e'2'0'002,
    OPTVAL_STARTER_ITEMS_RANDOM = 0x2'0e'2'0'003,
    // Whether to use faster enemy HP/ATK scaling after floor 100.
    OPT_FLOOR_100_SCALING       = 0x1'10'1'0'002,
    // Whether to increase bosses' HP/ATK stats by 25% / 50% / 100%.
    OPT_BOSS_SCALING            = 0x1'11'2'0'004,
    OPTVAL_BOSS_SCALING_NORMAL  = 0x2'11'2'0'000,
    OPTVAL_BOSS_SCALING_1_25X   = 0x2'11'2'0'001,
    OPTVAL_BOSS_SCALING_1_50X   = 0x2'11'2'0'002,
    OPTVAL_BOSS_SCALING_2_00X   = 0x2'11'2'0'003,
    // How the stage's rank increases.
    OPT_STAGE_RANK              = 0x1'13'1'0'002,
    OPTVAL_STAGE_RANK_30_FLOORS = 0x2'13'1'0'000,
    OPTVAL_STAGE_RANK_ALWAYSMAX = 0x2'13'1'0'001,
    // Alternate balance options.
    OPT_PERCENT_BASED_DANGER    = 0x1'14'1'0'002,
    OPT_WEAKER_RUSH_BADGES      = 0x1'15'1'0'002,
    OPT_EVASION_BADGES_CAP      = 0x1'16'1'0'002,
    OPT_64_STYLE_HP_FP_DRAIN    = 0x1'17'1'0'002,
    // Changes to stage hazard rates.
    OPT_STAGE_HAZARDS           = 0x1'18'3'0'005,
    OPTVAL_STAGE_HAZARDS_NORMAL = 0x2'18'3'0'000,
    OPTVAL_STAGE_HAZARDS_HIGH   = 0x2'18'3'0'001,
    OPTVAL_STAGE_HAZARDS_LOW    = 0x2'18'3'0'002,
    OPTVAL_STAGE_HAZARDS_NO_FOG = 0x2'18'3'0'003,
    OPTVAL_STAGE_HAZARDS_OFF    = 0x2'18'3'0'004,
    // Whether to enable variance on all sources of variable damage.
    OPT_RANDOM_DAMAGE           = 0x1'1b'2'0'003,
    OPTVAL_RANDOM_DAMAGE_NONE   = 0x2'1b'2'0'000,
    OPTVAL_RANDOM_DAMAGE_25     = 0x2'1b'2'0'001,
    OPTVAL_RANDOM_DAMAGE_50     = 0x2'1b'2'0'002,
    // Random items from thrown audience items.
    OPT_AUDIENCE_RANDOM_THROWS  = 0x1'1d'1'0'002,
    // Whether Chet Rippo should appear early.
    OPT_CHET_RIPPO_APPEARANCE   = 0x1'1e'1'0'002,
    OPTVAL_CHET_RIPPO_RANDOM    = 0x2'1e'1'0'000,
    OPTVAL_CHET_RIPPO_GUARANTEE = 0x2'1e'1'0'001,
    // Whether Merlee's curse should be enabled.
    OPT_MERLEE_CURSE            = 0x1'1f'1'0'002,
    // Whether to disable full-healing the party after collecting a chest.
    OPT_DISABLE_CHEST_HEAL      = 0x1'20'1'0'002,
    // Whether to allow Movers to spawn or not.
    OPT_MOVERS_ENABLED          = 0x1'21'1'8'002,
    // Whether to override which partner is rewarded first.
    OPT_FIRST_PARTNER           = 0x1'22'3'0'008,
    OPTVAL_NONE_FIRST           = 0x2'22'3'0'000,
    OPTVAL_GOOMBELLA_FIRST      = 0x2'22'3'0'001,
    OPTVAL_KOOPS_FIRST          = 0x2'22'3'0'002,
    OPTVAL_FLURRIE_FIRST        = 0x2'22'3'0'003,
    OPTVAL_YOSHI_FIRST          = 0x2'22'3'0'004,
    OPTVAL_VIVIAN_FIRST         = 0x2'22'3'0'005,
    OPTVAL_BOBBERY_FIRST        = 0x2'22'3'0'006,
    OPTVAL_MS_MOWZ_FIRST        = 0x2'22'3'0'007,
    // Hidden option; shuffles the appearance and description of items.
    OPT_OBFUSCATE_ITEMS         = 0x1'25'1'0'002,
    // Turns on options for community races and disables option changes.
    OPT_RACE_MODE               = 0x1'26'1'0'002,
    // Next: 0x1/2'27...
    
    // Cosmetic / internal-only flag-based options.
    OPT_RTA_TIMER               = 0x1'60'1'0'002,
    OPT_YOSHI_COLOR_SELECT      = 0x1'61'1'0'002,
    OPT_START_WITH_FX           = 0x1'62'1'0'002,
    OPT_BGM_DISABLED            = 0x1'63'1'0'002,
    OPT_ENABLE_P_BADGES         = 0x1'64'1'0'002,
    OPT_ENABLE_PARTNER_REWARD   = 0x1'65'1'0'002,
    OPT_DEBUG_MODE_USED         = 0x1'66'1'0'002,
    OPT_ENABLE_UPGRADE_REWARD   = 0x1'67'1'0'002,   // Boots, Hammer, S. Sack
    // Used to ensure similar partner order if using non-zero OPT_FIRST_PARTNER.
    OPT_LAST_PARTNER_INTENDED   = 0x1'68'3'0'008,
    // v2.22, determines whether the player has saved at the start of a run.
    // (disables changing options any further, including after a reload)
    OPT_HAS_STARTED_RUN         = 0x1'6b'1'0'002,
    // v2.23, shows a one-time tutorial when picking up a Lv. 2 Special Move.
    OPT_SEEN_MOVE_LEVEL_TUT     = 0x1'6c'1'0'002,
    OPT_SHOW_ATK_DEF            = 0x1'6d'1'0'002,
    // Next: 0x1'6e...
    
    // Numeric options.
    // Global HP and ATK scaling (in percentage).
    OPTNUM_ENEMY_HP             = 0x3'00'2'1'3e8,
    OPTNUM_ENEMY_ATK            = 0x3'02'2'1'3e8,
    // SP cost for Superguarding (in increments of 0.01 SP).
    OPTNUM_SUPERGUARD_SP_COST   = 0x3'04'1'0'096,
    // FP cost for switching partners.
    OPTNUM_SWITCH_PARTY_FP_COST = 0x3'05'1'0'00a,
    // Multiplier for attacks' SP regen (in increments of 0.05 SP, up to 3.00).
    OPTNUM_SP_REGEN_MODIFIER    = 0x3'06'1'0'03c,
    // Next: 0x3'07...
    
    // Gameplay stats.
    STAT_TURNS_SPENT            = 0x4'00'3'2'007,
    STAT_MOST_TURNS_RECORD      = 0x4'03'2'2'004,
    STAT_MOST_TURNS_CURRENT     = 0x4'05'2'2'004,
    STAT_MOST_TURNS_FLOOR       = 0x4'07'4'7'fff,
    STAT_TIMES_RAN_AWAY         = 0x4'0b'2'2'004,
    STAT_ENEMY_DAMAGE           = 0x4'0d'3'2'007,
    STAT_PLAYER_DAMAGE          = 0x4'10'3'2'007,
    STAT_ITEMS_USED             = 0x4'13'3'2'007,
    STAT_COINS_EARNED           = 0x4'16'3'2'007,
    STAT_COINS_SPENT            = 0x4'19'3'2'007,
    STAT_FP_SPENT               = 0x4'1c'3'2'007,
    STAT_SP_SPENT               = 0x4'1f'3'2'007,
    STAT_SUPERGUARDS            = 0x4'22'3'2'007,
    STAT_ITEMS_SOLD             = 0x4'25'3'2'007,
    STAT_BADGES_SOLD            = 0x4'28'3'2'007,
    STAT_LEVELS_SOLD            = 0x4'2b'2'2'004,
    STAT_SHINE_SPRITES          = 0x4'2d'2'2'003,
    STAT_CONDITIONS_MET         = 0x4'2f'3'2'007,
    STAT_CONDITIONS_TOTAL       = 0x4'32'3'2'007,
    STAT_MOVERS_USED            = 0x4'35'2'2'004,
    STAT_BATTLES_SKIPPED        = 0x4'37'3'2'007,
    // Next: 0x4'3a...
    
    // Non-option values (used for menu, etc.)
    MENU_EMPTY_OPTION           = 0x7000'0001,      // Used for blank spaces.
    MENU_CHANGE_PAGE            = 0x7000'0002,      // Changes menu pages.
    MENU_SET_DEFAULT            = 0x7000'0003,      // Resets all settings.
};

// An enumeration of all of the RNG sequences used by StateManager_v2.
// All RNG calls (except RNG_VANILLA) mangle the results with the filename seed,
// so results should differ between seeds, but be consistent on the same seed.
enum RngSequence {
    // "Totally random"; not reproducible run-to-run.
    RNG_VANILLA             = -1,    // Calls ttyd::system::irand().
    
    // Mangled w/floor number and OPT_CHEST_REWARDS; will change dramatically
    // if using options that change seeding or if skipping floors with Movers,
    // but will be consistent otherwise.
    RNG_CHEST               = 0,    // Chest top-level weighting.
    
    // Mangled w/floor number; may stay reasonably consistent even if using
    // options that change seeding for unrelated features.
    RNG_ENEMY               = 1,    // Enemy loadout generation (+ item drop).
    RNG_ITEM                = 2,    // Items (for enemy loadouts or shops).
    RNG_CONDITION           = 3,    // Bonus challenge condition.
    RNG_CONDITION_ITEM      = 4,    // Bonus challenge reward.
    
    // Mangled w/floor number; completely consistent per filename.
    RNG_CHET_RIPPO          = 13,   // Chet Rippo appearance chance.
    
    // Not mangled w/ floor number; order should stay completely consistent,
    // independent of all seeding options (unless partners are disabled).
    RNG_INVENTORY_UPGRADE   = 5,    // Boots and Hammer upgrades.
    RNG_CHEST_BADGE_FIXED   = 6,    // Chest one-time badges' order.
    RNG_PARTNER             = 7,    // Partners' obtained order.
    RNG_STAR_POWER          = 8,    // Star powers' obtained order.
    RNG_KISS_THIEF          = 9,    // Kiss Thief reward order.
    
    // Not mangled w/ floor number; will stay more or less consistent depending
    // on when partners are first unlocked, but independent of other options.
    RNG_CHEST_BADGE_RANDOM  = 10,   // Chest random badge order.
    RNG_AUDIENCE_ITEM       = 11,   // Random audience items (if using option).
    
    // Used for picking random filenames; should NOT be used for anything else.
    RNG_FILENAME            = 12,
    
    // Used for Movers; unlike other RNG calls, this is used as an offset to the
    // floor number, to make sure Movers don't appear too close in succession.
    // Completely consistent per filename.
    RNG_MOVER               = 14,
    
    // Used to seed the shuffles used for item obfuscation option.
    RNG_ITEM_OBFUSCATION    = 15,

    RNG_SEQUENCE_MAX        = 16,
};

}