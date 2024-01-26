#include "mod_state.h"

#include "common_functions.h"
#include "common_types.h"
#include "mod.h"
#include "mod_achievements.h"
#include "patch.h"

#include <gc/OSTime.h>
#include <third_party/fasthash.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/mariost.h>
#include <ttyd/system.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::infinite_pit {
    
namespace {
    
using ::ttyd::mario_pouch::PouchData;
namespace ItemType = ::ttyd::item_data::ItemType;

// Global for floor 100 time, and whether it has been stopped on this file load.
bool g_RtaTimerStopped = false;
uint64_t g_RtaTimer100 = 0;

const char* GetSavefileName() {
    return ttyd::mariost::g_MarioSt->saveFileName;
}
    
void* GetSavedStateLocation() {
    // Store state_v2 in otherwise unused space at the end of the GSWF array.
    // This affords 0x120 bytes of space, and should be aligned to 8 bytes.
    return &ttyd::mariost::g_MarioSt->gswf[0x2e0];
}

// Updates partners' Ultra Rank max HP based how many times each partner
// has had a Shine Sprite used on them (each after the second adds +5 max HP).
void InitPartyMaxHpTable(uint8_t* partner_upgrades) {
    static constexpr const int16_t kDefaultUltraRankMaxHp[] = {
        30, 25, 40, 30, 35, 30, 25
    };
    int16_t* hp_table = ttyd::mario_pouch::_party_max_hp_table + 4;
    for (int32_t i = 0; i < 7; ++i) {
        int32_t hp = kDefaultUltraRankMaxHp[i] +
            (partner_upgrades[i] > 2 ? (partner_upgrades[i] - 2) * 5 : 0);
        if (hp > 200) hp = 200;
        hp_table[i * 4 + 2] = hp;
    }
}

bool LoadFromPreviousVersion(StateManager_v2* state) {
    void* saved_state = GetSavedStateLocation();
    uint8_t version = *reinterpret_cast<uint8_t*>(saved_state);
    if (version < 7) {
        // Version is incompatible with the current version; fail to load.
        return false;
    }
    
    // Since all compatible versions have the same memory layout, copy to start.
    patch::writePatch(state, saved_state, sizeof(StateManager_v2));
    // Update fields that wouldn't have default value when updating versions...

    // Force version to current.
    state->version_ = 8;
    InitPartyMaxHpTable(state->partner_upgrades_);
    
    // Reset item obfuscation RNG position in case it needs to be performed.
    g_Mod->state_.rng_sequences_[RNG_ITEM_OBFUSCATION] = 0;
    
    // Set "has started" flag if loading into an already-in-progress file.
    if (state->floor_ > 0) {
        state->SetOption(OPT_HAS_STARTED_RUN, true);
    }
    
    // If playing Mario-alone, make sure all "P" variants of collected Mario
    // badges are marked off in the badge log.
    AchievementsManager::UpdatePartnerVariantBadgesCollected();
    
    return true;
}

// Gets the individual parts from an Options_v2 enum value.
void GetOptionParts(uint32_t v, int32_t* x, int32_t* y, int32_t* w, int32_t* z) {
    *x = GetShiftedBitMask(v, 20, 27);
    *y = GetShiftedBitMask(v, 16, 19);
    *w = GetShiftedBitMask(v, 12, 15);
    *z = v & 0xfff;
}

// Gets the base64-encoded character for the encoded options string.
char GetBase64EncodingChar(int32_t sextet) {
    if (sextet < 26) {
        return 'A' + sextet;
    } else if (sextet < 52) {
        return 'a' + sextet - 26;
    } else if (sextet < 62) {
        return '0' + sextet - 52;
    } else if (sextet < 63) {
        return '!';
    } else {
        return '?';
    }
}

// Appends a completion percentage in the form "X/Y (ZZ.Z%)" to a string.
// Returns the number of characters printed.
int32_t PrintCompletionPercentage(char* buf, int32_t completed, int32_t total) {
    char* cur = buf;
    cur += IntegerToFmtString(completed, cur);
    cur += sprintf(cur, "/");
    cur += IntegerToFmtString(total, cur);
    // Prevents "nan" from showing up if 0/0.
    if (total == 0) total = 1;
    cur += sprintf(cur, " (%.1f%%)", 100.f * completed / total);
    return cur - buf;
}

}

bool StateManager_v2::Load(bool new_save) {
    g_RtaTimerStopped = false;
    
    if (!new_save) return LoadFromPreviousVersion(this);
    
    // Carry cosmetic / cheats state over between files.
    uint32_t cosmetic_options = option_flags_[3];
    
    memset(this, 0, sizeof(StateManager_v2));
    version_ = 8;
    SetDefaultOptions();
    option_flags_[3] = cosmetic_options;
    InitPartyMaxHpTable(partner_upgrades_);
    
    // Turn off "has started" bit, so it won't carry over from previous files.
    SetOption(OPT_HAS_STARTED_RUN, 0);
    // Turn off Debug Mode flag, so it won't carry over from previous files.
    SetOption(OPT_DEBUG_MODE_USED, 0);
    
    // Start new files with Lv. 1 Sweet Treat and Earth Tremor.
    star_power_levels_ = 0b00'00'00'00'00'00'01'01;
    
    // Seed the RNG based on the filename.
    // If the filename is a few variants of "random" or a single 'star',
    // pick a random replacement filename.
    const char* filename = GetSavefileName();
    for (const char* ch = filename; *ch; ++ch) {
        // If the filename contains a 'heart' character, start w/FX badges.
        if (*ch == '\xd0') SetOption(OPT_START_WITH_FX, 1);
    }
    if (!strcmp(filename, "random") || !strcmp(filename, "Random") ||
        !strcmp(filename, "RANDOM") || !strcmp(filename, "\xde") ||
        !strcmp(filename, "random\xd0") || !strcmp(filename, "Random\xd0") ||
        !strcmp(filename, "RANDOM\xd0") || !strcmp(filename, "\xde\xd0") ||
        !strcmp(filename, "\xd0\xde")) { 
        char filenameChars[9];
        // Temporarily set the filename seed based on the current time.
        filename_seed_ = static_cast<uint32_t>(gc::OSTime::OSGetTime());
        for (int32_t i = 0; i < 8; ++i) {
            // Pick uppercase / lowercase characters randomly (excluding I / l).
            int32_t ch = Rand(50, RNG_FILENAME);
            if (ch < 25) {
                filenameChars[i] = ch + 'a';
                if (filenameChars[i] == 'l') filenameChars[i] = 'z';
            } else {
                filenameChars[i] = (ch - 25) + 'A';
                if (filenameChars[i] == 'I') filenameChars[i] = 'Z';
            }
        }
        // If a heart was in the initial filename, put one at the end of the
        // random one as well.
        if (GetOptionNumericValue(OPT_START_WITH_FX)) {
            filenameChars[7] = '\xd0';
        }
        filenameChars[8] = 0;
        // Copy generated filename to MarioSt.
        strcpy(const_cast<char*>(filename), filenameChars);
    }
    
    // Save the filename's hash as a seed for part of the RNG function.
    filename_seed_ = third_party::fasthash64(filename, 8, /* seed */ 417);
    return true;
}

void StateManager_v2::Save() {
    void* saved_state = GetSavedStateLocation();
    patch::writePatch(saved_state, this, sizeof(StateManager_v2));
}

int32_t StateManager_v2::GetStarPowerLevel(int32_t star_power_type) const {
    if (star_power_type < 0 || star_power_type > 7) return 0;
    return GetShiftedBitMask(
        star_power_levels_, star_power_type*2, star_power_type*2 + 1);
}

uint32_t StateManager_v2::Rand(uint32_t range, int32_t sequence) {
    if (sequence > RNG_VANILLA && sequence < RNG_SEQUENCE_MAX) {
        uint32_t data[2] = { 0, 0 };
        uint16_t* seq_val = rng_sequences_ + sequence;
        // Include the sequence id and current position, so the beginnings of
        // different sequences can't end up identical.
        // (e.g. chest random badge rewards + first floor's enemy items)
        data[0] = (*seq_val)++ | (sequence << 16);
        switch (sequence) {
            case RNG_CHEST: {
                // Mix in the number of rewards, so the types of rewards are
                // totally differently ordered for each setting.
                data[0] |= GetOptionNumericValue(OPT_CHEST_REWARDS) << 24;
                data[1] = floor_;
                break;
            }
            case RNG_ENEMY:
            case RNG_ITEM:
            case RNG_CONDITION:
            case RNG_CONDITION_ITEM:
            case RNG_CHET_RIPPO: {
                data[1] = floor_;
                break;
            }
            case RNG_MOVER: {
                // Special-case; for Movers, have the data just be the floor #,
                // with higher `seq_val` simulating values from earlier floors.
                // Calling code will use this to limit how close together
                // subsequent Movers can appear.
                data[0] = floor_ - *seq_val;
                break;
            }
            case RNG_INVENTORY_UPGRADE:
            case RNG_CHEST_BADGE_FIXED:
            case RNG_PARTNER:
            case RNG_STAR_POWER:
            case RNG_KISS_THIEF:
            case RNG_CHEST_BADGE_RANDOM:
            case RNG_AUDIENCE_ITEM:
            case RNG_FILENAME:
            case RNG_ITEM_OBFUSCATION:
                break;
        }
        return third_party::fasthash64(
            data, sizeof(data), filename_seed_) % range;
    }
    return ttyd::system::irand(range);
}

void StateManager_v2::ClearAllOptions() {
    memset(option_flags_, 0, sizeof(option_flags_));
    memset(option_bytes_, 0, sizeof(option_bytes_));
    memset(play_stats_,   0, sizeof(play_stats_));
}

void StateManager_v2::SetDefaultOptions() {
    memset(option_flags_, 0, 12);
    memset(option_bytes_, 0, 6);
    // Set non-zero default values to their proper values.
    SetOption(OPT_CHEST_REWARDS, 3);
    SetOption(OPTVAL_STARTER_ITEMS_BASIC);
    SetOption(OPTNUM_ENEMY_HP, 100);
    SetOption(OPTNUM_ENEMY_ATK, 100);
    SetOption(OPTNUM_SP_REGEN_MODIFIER, 20);  // 1.00x
}

void StateManager_v2::ChangeOption(int32_t option, int32_t change) {
    const int32_t type = option >> 28;
    const int32_t max = option & 0xfff;
    int32_t value = GetOptionNumericValue(option);

    if (type == 1) {
        // For flag values, wrap around, and advance if change is 0.
        if (change == 0) change = 1;
        value = (value + change) % max;
        if (value < 0) value += max;
        SetOption(option, value);
    } else if (type == 3 || type == 4) {
        // Saturate range, don't change on 0; range checking is done in Set.
        SetOption(option, value + change);
    }
}

void StateManager_v2::SetOption(int32_t option, int32_t value) {
    int32_t x, y, w, z;
    GetOptionParts(option, &x, &y, &w, &z);
    w &= 7;  // Mask out "changes seeding" bit.
    int32_t type = option >> 28;
    
    if (type == 1 || type == 2) {
        if (type == 2) {
            // OPTVAL, we know the value and that it must be valid.
            value = z;
        } else if (value < 0 || value >= z) {
            // Value out of range for this flag, can't do anything meaningful.
            return;
        }
        // Set the value.
        uint32_t* ptr = option_flags_ + (x >> 5);
        const uint32_t start_bit = x & 31;
        const uint32_t mask = GetBitMask(start_bit, start_bit + y - 1);
        *ptr = (*ptr & ~mask) | (value << start_bit);
    } else if (type == 3 || type == 4) {
        // Clamp value to the appropriate min and max range.
        if (w < 2) {
            // Clamp to [W, Z].
            if (value < w) value = w;
            if (value > z) value = z;
        } else if ((w == 2 || w == 3) && z > 0 && z < 10) {
            // Clamp to Z digits, either [0, 999...] or [-999..., 999...].
            static const constexpr int32_t powers_of_10[] = {
                1, 10, 100, 1000, 10'000, 100'000, 1'000'000, 10'000'000,
                100'000'000, 1'000'000'000
            };
            if (value >= powers_of_10[z]) value = powers_of_10[z] - 1;
            if (w == 2) {
                if (value < 0) value = 0;
            } else {
                if (value <= -powers_of_10[z]) value = -powers_of_10[z] + 1;
            }
        }
        // Set the value.
        uint8_t* ptr = (type == 3 ? option_bytes_ : play_stats_) + x;
        uint32_t uint_val = static_cast<uint32_t>(value);
        for (int32_t i = y - 1; i >= 0; --i) {
            ptr[i] = uint_val & 0xff;
            uint_val >>= 8;
        }
    }
}

int32_t StateManager_v2::GetOptionValue(int32_t option) const {
    int32_t type = option >> 28;
    if (type == 1) return GetOptionFlagValue(option);
    return GetOptionNumericValue(option);
}

int32_t StateManager_v2::GetOptionFlagValue(int32_t option) const {
    int32_t x, y, w, z;
    GetOptionParts(option, &x, &y, &w, &z);
    int32_t type = option >> 28;
    
    if (type == 1) {
        uint32_t word = option_flags_[x >> 5];
        const uint32_t start_bit = x & 31;
        int32_t value = GetShiftedBitMask(word, start_bit, start_bit + y - 1);
        // Return as OPTVAL_... type.
        return 0x20000000 | (option & 0x0ffff000) | value;
    }
    return -1;
}

int32_t StateManager_v2::GetOptionNumericValue(int32_t option) const {
    int32_t x, y, w, z;
    GetOptionParts(option, &x, &y, &w, &z);
    int32_t type = option >> 28;
    
    if (type == 1) {
        uint32_t word = option_flags_[x >> 5];
        const uint32_t start_bit = x & 31;
        return GetShiftedBitMask(word, start_bit, start_bit + y - 1);
    } else if (type == 3 || type == 4) {
        const uint8_t* ptr = (type == 3 ? option_bytes_ : play_stats_) + x;
        uint32_t uint_val = 0;
        if ((w & 7) == 3 && *ptr & 0x80) {
            // If option supports negative values and currently is negative,
            // fill the buffer with all 1-bits to properly represent it.
            uint_val = ~0;
        }
        for (int32_t i = 0; i < y; ++i) {
            uint_val = (uint_val << 8) + *ptr++;
        }
        return static_cast<int32_t>(uint_val);
    }
    return -1;
}

bool StateManager_v2::CheckOptionValue(int32_t option_value) const {
    if (option_value >> 28 != 2) return false;
    
    int32_t x, y, w, z;
    GetOptionParts(option_value, &x, &y, &w, &z);
    
    uint32_t word = option_flags_[x >> 5];
    const uint32_t start_bit = x & 31;
    const int32_t value = GetShiftedBitMask(word, start_bit, start_bit + y - 1);
    return value == z;
}

void StateManager_v2::GetOptionStrings(
    int32_t option, char* name_buf, char* value_buf,
    bool* is_default, bool* affects_seeding) const {
    const int32_t value = GetOptionValue(option);
    const int32_t num_value = GetOptionNumericValue(option);
    if (option >> 28 == 7) {
        // Menu placeholder option; return empty strings.
        *is_default = false;
        *affects_seeding = false;
        strcpy(name_buf, "");
        strcpy(value_buf, "");
        if (option == MENU_SET_DEFAULT) {
            strcpy(name_buf, "\xde Reset all settings to default \xde");
        }
        return;
    }
    
    // Get option parts.
    int32_t x, y, w, z;
    GetOptionParts(option, &x, &y, &w, &z);
    
    // Set name.
    switch (option) {
        case OPT_CHEST_REWARDS: {
            strcpy(name_buf, "Rewards per chest:");         break;
        }
        case OPT_NO_EXP_MODE: {
            strcpy(name_buf, "No-EXP mode:");               break;
        }
        case OPT_BATTLE_REWARD_MODE: {
            strcpy(name_buf, "Battle drops:");              break;
        }
        case OPT_PARTNERS_OBTAINED: {
            strcpy(name_buf, "Partners obtained:");         break;
        }
        case OPT_PARTNER_RANK: {
            strcpy(name_buf, "Partner starting rank:");     break;
        }
        case OPT_BADGE_MOVE_LEVEL: {
            strcpy(name_buf, "Max badge move level:");      break;
        }
        case OPT_STARTER_ITEMS: {
            strcpy(name_buf, "Starter item set:");          break;
        }
        case OPT_FLOOR_100_SCALING: {
            strcpy(name_buf, "Floor 100+ stat scaling:");   break;
        }
        case OPT_BOSS_SCALING: {
            strcpy(name_buf, "Boss HP/ATK stat scaling:");  break;
        }
        case OPT_MERLEE_CURSE: {
            strcpy(name_buf, "Infinite Merlee curses:");    break;
        }
        case OPT_STAGE_RANK: {
            strcpy(name_buf, "Stage rank-up:");             break;
        }
        case OPT_PERCENT_BASED_DANGER: {
            strcpy(name_buf, "Danger/Peril thresholds:");   break;
        }
        case OPT_WEAKER_RUSH_BADGES: {
            strcpy(name_buf, "Power/Mega Rush power:");     break;
        }
        case OPT_EVASION_BADGES_CAP: {
            strcpy(name_buf, "Cap badge evasion at 80%:");  break;
        }
        case OPT_64_STYLE_HP_FP_DRAIN: {
            strcpy(name_buf, "PM64-style Drain badges:");   break;
        }
        case OPT_STAGE_HAZARDS: {
            strcpy(name_buf, "Stage hazard frequency:");    break;
        }
        case OPT_RANDOM_DAMAGE: {
            strcpy(name_buf, "Damage randomization:");      break;
        }
        case OPT_AUDIENCE_RANDOM_THROWS: {
            strcpy(name_buf, "Random audience items:");     break;
        }
        case OPT_CHET_RIPPO_APPEARANCE: {
            strcpy(name_buf, "Chet Rippo appear %:");       break;
        }
        case OPT_DISABLE_CHEST_HEAL: {
            strcpy(name_buf, "Full heal from chests:");     break;
        }
        case OPT_MOVERS_ENABLED: {
            strcpy(name_buf, "Mover appearance:");          break;
        }
        case OPT_FIRST_PARTNER: {
            strcpy(name_buf, "Pick first partner:");        break;
        }
        case OPTNUM_ENEMY_HP: {
            strcpy(name_buf, "Enemy HP multiplier:");       break;
        }
        case OPTNUM_ENEMY_ATK: {
            strcpy(name_buf, "Enemy ATK multiplier:");      break;
        }
        case OPTNUM_SUPERGUARD_SP_COST: {
            strcpy(name_buf, "Superguard cost:");           break;
        }
        case OPTNUM_SWITCH_PARTY_FP_COST: {
            strcpy(name_buf, "Partner switch cost:");       break;
        }
        case OPTNUM_SP_REGEN_MODIFIER: {
            strcpy(name_buf, "SP regen from attacks:");     break;
        }
    }
    
    // Check if option is default.
    if (option == OPTNUM_ENEMY_ATK || option == OPTNUM_ENEMY_HP) {
        *is_default = value == 100;
    } else if (option == OPTNUM_SP_REGEN_MODIFIER) {
        *is_default = value == 20;
    } else if (option == OPT_CHEST_REWARDS) {
        *is_default = num_value == 3;
    } else if (option == OPT_STARTER_ITEMS) {
        *is_default = value == OPTVAL_STARTER_ITEMS_BASIC;
    } else {
        *is_default = num_value == 0;
    }
    
    // Check top bit of 'W' nybble from option to see if it affects seeding.
    *affects_seeding = (w & 8) != 0;
    
    // Set value.
    
    // Options with special text.
    switch (value) {
        case OPTVAL_CHEST_REWARDS_RANDOM: {
            strcpy(value_buf, "Varies");                return;
        }
        case OPTVAL_NO_EXP_MODE_ON: {
            strcpy(value_buf, "On (99 BP)");            return;
        }
        case OPTVAL_NO_EXP_MODE_INFINITE: {
            strcpy(value_buf, "On (Infinite BP)");      return;
        }
        case OPTVAL_DROP_STANDARD: {
            strcpy(value_buf, "One held + bonus");      return;
        }
        case OPTVAL_DROP_HELD_FROM_BONUS: {
            strcpy(value_buf, "Held gated by bonus");   return;
        }
        case OPTVAL_DROP_NO_HELD_W_BONUS: {
            strcpy(value_buf, "No held, bonus only");   return;
        }
        case OPTVAL_DROP_ALL_HELD: {
            strcpy(value_buf, "All held + bonus");      return;
        }
        case OPTVAL_PARTNERS_ALL_REWARDS: {
            strcpy(value_buf, "All as rewards");        return;
        }
        case OPTVAL_PARTNERS_ALL_START: {
            strcpy(value_buf, "Start with all");        return;
        }
        case OPTVAL_PARTNERS_ONE_START: {
            strcpy(value_buf, "Start with one");        return;
        }
        case OPTVAL_PARTNERS_NEVER: {
            // If a 'first partner' is selected, rather than disabling them
            // entirely, start with that one and never receive any others.
            if (GetOptionNumericValue(OPT_FIRST_PARTNER)) {
                strcpy(value_buf, "Starter only");
            } else {
                strcpy(value_buf, "Never");
            }
            return;
        }
        case OPTVAL_PARTNER_RANK_NORMAL: {
            strcpy(value_buf, "Normal");                return;
        }
        case OPTVAL_PARTNER_RANK_SUPER: {
            strcpy(value_buf, "Super");                 return;
        }
        case OPTVAL_PARTNER_RANK_ULTRA: {
            strcpy(value_buf, "Ultra");                 return;
        }
        case OPTVAL_BADGE_MOVE_1X: {
            strcpy(value_buf, "1 per copy");            return;
        }
        case OPTVAL_BADGE_MOVE_2X: {
            strcpy(value_buf, "2 per copy");            return;
        }
        case OPTVAL_BADGE_MOVE_RANK: {
            strcpy(value_buf, "Rank-based");            return;
        }
        case OPTVAL_BADGE_MOVE_INFINITE: {
            strcpy(value_buf, "Always 99");             return;
        }
        case OPTVAL_STARTER_ITEMS_BASIC: {
            strcpy(value_buf, "Basic");                 return;
        }
        case OPTVAL_STARTER_ITEMS_STRONG: {
            strcpy(value_buf, "Strong");                return;
        }
        case OPTVAL_STARTER_ITEMS_RANDOM: {
            strcpy(value_buf, "Random");                return;
        }
        case OPTVAL_BOSS_SCALING_NORMAL: {
            strcpy(value_buf, "Default");               return;
        }
        case OPTVAL_BOSS_SCALING_1_25X: {
            strcpy(value_buf, "x1.25");                 return;
        }
        case OPTVAL_BOSS_SCALING_1_50X: {
            strcpy(value_buf, "x1.5");                  return;
        }
        case OPTVAL_BOSS_SCALING_2_00X: {
            strcpy(value_buf, "x2.0");                  return;
        }
        case OPTVAL_STAGE_RANK_30_FLOORS: {
            strcpy(value_buf, "Floor 30/60/90");        return;
        }
        case OPTVAL_STAGE_RANK_ALWAYSMAX: {
            strcpy(value_buf, "Always Superstar");      return;
        }
        case OPTVAL_STAGE_HAZARDS_NORMAL: {
            strcpy(value_buf, "Default");               return;
        }
        case OPTVAL_STAGE_HAZARDS_HIGH: {
            strcpy(value_buf, "High");                  return;
        }
        case OPTVAL_STAGE_HAZARDS_LOW: {
            strcpy(value_buf, "Low");                   return;
        }
        case OPTVAL_STAGE_HAZARDS_NO_FOG: {
            strcpy(value_buf, "No Fog");                return;
        }
        case OPTVAL_STAGE_HAZARDS_OFF: {
            strcpy(value_buf, "None");                  return;
        }
        case OPTVAL_RANDOM_DAMAGE_25: {
            strcpy(value_buf, "+/-25%");                return;
        }
        case OPTVAL_RANDOM_DAMAGE_50: {
            strcpy(value_buf, "+/-50%");                return;
        }
        case OPTVAL_CHET_RIPPO_RANDOM: {
            strcpy(value_buf, "Floor-based");           return;
        }
        case OPTVAL_CHET_RIPPO_GUARANTEE: {
            strcpy(value_buf, "Guaranteed");            return;
        }
        case OPTVAL_NONE_FIRST: {
            strcpy(value_buf, "Default");               return;
        }
        case OPTVAL_GOOMBELLA_FIRST: {
            strcpy(value_buf, "Goombella");             return;
        }
        case OPTVAL_KOOPS_FIRST: {
            strcpy(value_buf, "Koops");                 return;
        }
        case OPTVAL_FLURRIE_FIRST: {
            strcpy(value_buf, "Flurrie");               return;
        }
        case OPTVAL_YOSHI_FIRST: {
            strcpy(value_buf, "Yoshi");                 return;
        }
        case OPTVAL_VIVIAN_FIRST: {
            strcpy(value_buf, "Vivian");                return;
        }
        case OPTVAL_BOBBERY_FIRST: {
            strcpy(value_buf, "Bobbery");               return;
        }
        case OPTVAL_MS_MOWZ_FIRST: {
            strcpy(value_buf, "Ms. Mowz");              return;
        }
    }
    // Options with special formatting.
    switch (option) {
        case OPT_CHEST_REWARDS: {
            sprintf(value_buf, "%" PRId32, num_value);
            return;
        }
        case OPT_FLOOR_100_SCALING: {
            strcpy(value_buf, num_value ? "+10% / set" : "+5% / set");
            return;
        }
        case OPT_PERCENT_BASED_DANGER: {
            strcpy(value_buf, num_value ? "%-based" : "Fixed");
            return;
        }
        case OPT_WEAKER_RUSH_BADGES: {
            strcpy(value_buf, num_value ? "+1/+2" : "+2/+5");
            return;
        }
        case OPT_DISABLE_CHEST_HEAL: {
            // Reverse conditions, since healing is defaulted to be on.
            strcpy(value_buf, num_value ? "Off" : "On");
            return;
        }
        case OPTNUM_ENEMY_HP:
        case OPTNUM_ENEMY_ATK: {
            sprintf(value_buf, "%" PRId32 "%s", num_value, "%");
            return;
        }
        case OPTNUM_SUPERGUARD_SP_COST: {
            sprintf(value_buf, "%.2f SP", num_value / 100.0f);
            return;
        }
        case OPTNUM_SWITCH_PARTY_FP_COST: {
            sprintf(value_buf, "%" PRId32 " FP", num_value);
            return;
        }
        case OPTNUM_SP_REGEN_MODIFIER: {
            sprintf(value_buf, "%.2fx", num_value / 20.0f);
            return;
        }
    }
    // Default to using "On"/"Off" for nonzero/zero values.
    strcpy(value_buf, num_value ? "On" : "Off");
}

const char* StateManager_v2::GetEncodedOptions() const {    
    static char enc_options[24];
    
    if (GetOptionNumericValue(OPT_RACE_MODE)) {
        // Return special string for race mode.
        sprintf(enc_options, "Community Race (v.%" PRId32 ")", version_);
        return enc_options;
    }

    uint64_t numeric_options = GetOptionValue(OPTNUM_ENEMY_HP);
    numeric_options <<= 12;
    numeric_options += GetOptionValue(OPTNUM_ENEMY_ATK);
    numeric_options <<= 8;
    numeric_options += GetOptionValue(OPTNUM_SUPERGUARD_SP_COST);
    numeric_options <<= 4;
    numeric_options += GetOptionValue(OPTNUM_SWITCH_PARTY_FP_COST);
    numeric_options <<= 6;
    numeric_options += GetOptionValue(OPTNUM_SP_REGEN_MODIFIER);
    // Flip "starting items" flag off to make default options look simpler.
    uint64_t flag_options = option_flags_[1];
    flag_options <<= 32;
    flag_options |= (option_flags_[0] ^ 0x4000);

    // Convert to a base-64 scheme using A-Z, a-z, 0-9, !, ?.
    // Format: 7+.7+.1 (FLAGS.NUMERIC.VERSION)
    for (int32_t i = 6; i >= 0; --i) {
        const int32_t sextet = flag_options & 63;
        enc_options[i] = GetBase64EncodingChar(sextet);
        flag_options >>= 6;
    }
    for (int32_t i = 14; i >= 8; --i) {
        const int32_t sextet = numeric_options & 63;
        enc_options[i] = GetBase64EncodingChar(sextet);
        numeric_options >>= 6;
    }
    enc_options[7]  = '.';
    enc_options[15] = '.';
    enc_options[16] = GetBase64EncodingChar(version_);
    enc_options[17] = '\0';
    return enc_options;
}

bool StateManager_v2::GetPlayStatsString(char* out_buf) {
    // Check to make sure the file's timer is valid.
    GetCurrentTimeString();
    
    const auto* mariost = ttyd::mariost::g_MarioSt;
    const uint64_t current_time = gc::OSTime::OSGetTime();
    const int64_t last_save_diff = current_time - last_save_time_;
    // If the start time was never initialized or the current time is before
    // the last save's time (indicating clock tampering), don't show any stats.
    if (last_save_diff < 0 || !pit_start_time_) {
        return false;
    }
    
    // Lock timer value when the player reads the sign on floor 100 w/race mode.
    if (GetOptionNumericValue(OPT_RACE_MODE) && floor_ + 1 == 100) {
        if (!g_RtaTimerStopped) {
            g_RtaTimerStopped = true;
            g_RtaTimer100 = current_time;
        }
    } else {
        g_RtaTimerStopped = false;
    }
    
    const int64_t start_diff =
        (g_RtaTimerStopped ? g_RtaTimer100 : current_time) - pit_start_time_;
    const int32_t turn_total    = GetOptionValue(STAT_TURNS_SPENT);
    const int64_t battle_time   = 
        mariost->animationTimeIncludingBattle - mariost->animationTimeNoBattle;
    int32_t battles_won   = (floor_ + 1) * 91 / 100;
    battles_won -= GetOptionValue(STAT_BATTLES_SKIPPED);
    
    // Page 1: Seed, floor, options & total play time.
    out_buf += sprintf(
        out_buf,
        "<kanban>\n"
        "Seed: <col 0000ffff>%s</col>, Floor: <col ff0000ff>%" PRId32 "\n</col>"
        "Options: <col 0000ffff>%s\n</col>"
        "RTA Time: <col ff0000ff>",
        GetSavefileName(), floor_ + 1, GetEncodedOptions());
    out_buf += DurationTicksToFmtString(start_diff, out_buf);
    
    // Page 2: Battle time, Average time / battle, Average turns / battle.
    out_buf += sprintf(out_buf, "\n</col><k><p>In-battle time: ");
    out_buf += DurationTicksToFmtString(battle_time, out_buf);
    out_buf += sprintf(out_buf, "\nAvg. battle time: ");
    out_buf += DurationTicksToFmtString(battle_time / battles_won, out_buf);
    out_buf += sprintf(
        out_buf, "\nAvg. battle turns: %.2f", turn_total * 1.0f / battles_won);
    
    // Page 3: Total turn count, max turns, run away count.
    out_buf += sprintf(out_buf, "\n<k><p>Total turns: ");
    out_buf += IntegerToFmtString(turn_total, out_buf);
    out_buf += sprintf(
        out_buf, "\nMax turns: %" PRId32 " (Floor %" PRId32 ")", 
        GetOptionValue(STAT_MOST_TURNS_RECORD),
        GetOptionValue(STAT_MOST_TURNS_FLOOR));
    out_buf += sprintf(out_buf, "\nTimes ran away: ");
    out_buf += IntegerToFmtString(GetOptionValue(STAT_TIMES_RAN_AWAY), out_buf);
    
    // Page 4: Damage dealt / taken, Superguards hit.
    out_buf += sprintf(out_buf, "\n<k><p>Enemy damage taken: ");
    out_buf += IntegerToFmtString(GetOptionValue(STAT_ENEMY_DAMAGE), out_buf);
    out_buf += sprintf(out_buf, "\nPlayer damage taken: ");
    out_buf += IntegerToFmtString(GetOptionValue(STAT_PLAYER_DAMAGE), out_buf);
    out_buf += sprintf(out_buf, "\nSuperguards hit: ");
    out_buf += IntegerToFmtString(GetOptionValue(STAT_SUPERGUARDS), out_buf);
    
    // Page 5: FP spent, SP spent, Items (& shine sprites) used.
    out_buf += sprintf(out_buf, "\n<k><p>FP spent: ");
    out_buf += IntegerToFmtString(GetOptionValue(STAT_FP_SPENT), out_buf);
    out_buf += sprintf(out_buf, "\nSP spent: ");
    out_buf += IntegerToFmtString(GetOptionValue(STAT_SP_SPENT), out_buf);
    out_buf += sprintf(out_buf, "\nItems used: ");
    out_buf += IntegerToFmtString(GetOptionValue(STAT_ITEMS_USED), out_buf);
    out_buf += sprintf(out_buf, " (");
    out_buf += IntegerToFmtString(GetOptionValue(STAT_SHINE_SPRITES), out_buf);
    out_buf += sprintf(out_buf, " Shines)");
    
    // Page 6: Coins earned / spent, bonus battle conditions met.
    out_buf += sprintf(out_buf, "\n<k><p>Coins earned: ");
    out_buf += IntegerToFmtString(GetOptionValue(STAT_COINS_EARNED), out_buf);
    out_buf += sprintf(out_buf, "\nCoins spent: ");
    out_buf += IntegerToFmtString(GetOptionValue(STAT_COINS_SPENT), out_buf);
    out_buf += sprintf(out_buf, "\nConditions met: ");
    out_buf += PrintCompletionPercentage(
        out_buf,
        GetOptionValue(STAT_CONDITIONS_MET), 
        GetOptionValue(STAT_CONDITIONS_TOTAL));
    
    // Page 7: Achievement progress.
    out_buf += sprintf(out_buf, "\n<k><p>Chest rewards: ");
    out_buf += PrintCompletionPercentage(
        out_buf,
        AchievementsManager::GetCurrentCompletionPoints(
            AchievementsManager::kChestRewardItem),
        AchievementsManager::GetMaxCompletionPoints(
            AchievementsManager::kChestRewardItem));
    out_buf += sprintf(out_buf, "\nBadge log: ");
    out_buf += PrintCompletionPercentage(
        out_buf,
        AchievementsManager::GetCurrentCompletionPoints(
            AchievementsManager::kBadgeLogItem),
        AchievementsManager::GetMaxCompletionPoints(
            AchievementsManager::kBadgeLogItem));
    out_buf += sprintf(out_buf, "\nTattle log: ");
    out_buf += PrintCompletionPercentage(
        out_buf,
        AchievementsManager::GetCurrentCompletionPoints(
            AchievementsManager::kTattleLogItem),
        AchievementsManager::GetMaxCompletionPoints(
            AchievementsManager::kTattleLogItem));
    
    // TODO: Add page for Items, badges, level-ups sold or Mover use in future?
    out_buf += sprintf(out_buf, "\n<k>");
    
    return true;
}

void StateManager_v2::SaveCurrentTime(bool pit_start) {
    uint64_t current_time = gc::OSTime::OSGetTime();
    if (pit_start) pit_start_time_ = current_time;
    last_save_time_ = current_time;
}

const char* StateManager_v2::GetCurrentTimeString() {    
    static char buf[16];
    const uint64_t current_time = gc::OSTime::OSGetTime();
    const int64_t start_diff =
        (g_RtaTimerStopped ? g_RtaTimer100 : current_time) - pit_start_time_;
    const int64_t last_save_diff = current_time - last_save_time_;
    // If debug mode was used on this file, the time since the last save
    // is negative (implying the time was rolled back), or time was never set,
    // return the empty string and clear the timebases previously set.
    if (last_save_diff < 0 || !pit_start_time_ ||
        g_Mod->state_.GetOptionNumericValue(OPT_DEBUG_MODE_USED)) {
        pit_start_time_ = 0;
        last_save_time_ = 0;
        return "";
    }
    // Otherwise, return the time since start as a string.
    DurationTicksToFmtString(start_diff, buf);
    return buf;
}

void StateManager_v2::EnableRaceOptions() {
    SetDefaultOptions();
    SetOption(OPT_RACE_MODE, 1);
    SetOption(OPT_CHEST_REWARDS, 4);
    SetOption(OPT_DISABLE_CHEST_HEAL, 1);
    SetOption(OPTVAL_STARTER_ITEMS_RANDOM);
}

}