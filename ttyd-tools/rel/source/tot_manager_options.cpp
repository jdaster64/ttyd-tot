#include "tot_manager_options.h"

#include "common_functions.h"
#include "mod.h"
#include "tot_generate_item.h"
#include "tot_gon_tower_npcs.h"
#include "tot_gsw.h"
#include "tot_manager_cosmetics.h"
#include "tot_manager_reward.h"
#include "tot_manager_title.h"
#include "tot_state.h"

#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace mod::tot {
    
namespace {

namespace ItemType = ::ttyd::item_data::ItemType;

struct RunOptionMetadata {
    uint32_t option;
    uint32_t default_optval;
    int32_t  default_value;             // Used instead if default_optval = 0.
    int8_t   check_for_default;
    int8_t   check_for_default_stat;
};
RunOptionMetadata g_OptionMetadata[] = {
    { OPT_PRESET, 0, -1, false, false },
    { OPT_DIFFICULTY, 0, -1, false, false },
    { OPT_TIMER_DISPLAY, 0, -1, false, false },
    { OPT_COUNTDOWN_TIMER, OPTVAL_COUNTDOWN_OFF, -1, true, false },
    { OPT_NUM_CHESTS, OPTVAL_CHESTS_DEFAULT, -1, true, false },
    { OPT_BATTLE_DROPS, OPTVAL_DROP_STANDARD, -1, true, false },
    { OPT_STARTER_ITEMS, OPTVAL_STARTER_ITEMS_BASIC, -1, true, false },
    { OPT_MOVE_AVAILABILITY, OPTVAL_MOVES_DEFAULT, -1, true, false },
    { OPT_MOVE_LIMIT, OPTVAL_MOVE_LIMIT_DEFAULT, -1, true, false },
    { OPT_MAX_PARTNERS, 0, -1, true, false },
    { OPT_PARTNER, OPTVAL_PARTNER_RANDOM, -1, true, false },
    { OPT_REVIVE_PARTNERS, OPTVAL_REVIVE_PARTNERS_ON, -1, true, false },
    { OPT_MARIO_HP, 0, 5, true, true },
    { OPT_MARIO_FP, 0, 5, true, true },
    { OPT_MARIO_BP, 0, 5, true, true },
    { OPT_PARTNER_HP, 0, 5, true, true },
    { OPT_INVENTORY_SACK_SIZE, 0, 2, true, false },
    { OPTNUM_ENEMY_HP, 0, 100, true, false },
    { OPTNUM_ENEMY_ATK, 0, 100, true, false },
    { OPTNUM_SUPERGUARD_SP_COST, 0, 0, true, false },
    { OPT_AC_DIFFICULTY, OPTVAL_AC_DEFAULT, -1, true, false },
    { OPT_RUN_AWAY, OPTVAL_RUN_AWAY_DEFAULT, -1, true, false },
    { OPT_STAGE_HAZARDS, OPTVAL_STAGE_HAZARDS_NORMAL, -1, true, false },
    { OPT_RANDOM_DAMAGE, OPTVAL_RANDOM_DAMAGE_NONE, -1, true, false },
    { OPT_AUDIENCE_RANDOM_THROWS, OPTVAL_AUDIENCE_THROWS_OFF, -1, true, false },
    { OPT_OBFUSCATE_ITEMS, OPTVAL_OBFUSCATE_ITEMS_OFF, -1, true, false },
    { OPT_BANDIT_ESCAPE, OPTVAL_BANDIT_NO_REFIGHT, -1, true, false },
    { OPT_CHARLIETON_STOCK, OPTVAL_CHARLIETON_NORMAL, -1, true, false },
    { OPT_NPC_CHOICE_1, 0, -1, true, false },
    { OPT_NPC_CHOICE_2, 0, -1, true, false },
    { OPT_NPC_CHOICE_3, 0, -1, true, false },
    { OPT_NPC_CHOICE_4, 0, -1, true, false },
    { OPT_SECRET_BOSS, OPTVAL_SECRET_BOSS_RANDOM, -1, true, false },
};

void EncodeOption(
    int8_t* encoding_bytes, int32_t& encoded_bit_count, uint32_t option) {
    int32_t t, x, y, a, b;

    StateManager::GetOptionParts(option, &t, &x, &y, &a, &b);
    int32_t bits_left = 6 - (encoded_bit_count % 6);
    int32_t num_bits  = t == TYPE_OPTNUM ? 8 : y;
    int32_t divisor   = t == TYPE_OPTNUM ? a : 1;
    int32_t value = g_Mod->state_.GetOption(option) / divisor;
    int32_t default_value = OptionsManager::GetDefaultValue(option) / divisor;
    // XOR with default values to make options string look sparser.
    value ^= default_value;

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

// Mario, then partners' HP scaling, in internal order.
const int32_t kHpMultipliers[] = { 100, 100, 80, 120, 80, 120, 100, 80 };

// Returns the current expected base stat given its option value and
// corresponding level (e.g. OPT_MARIO_HP + hp_level_).
int32_t GetBaseStat(uint32_t option, int32_t party = 0) {
    auto& state = g_Mod->state_;
    int32_t value = state.GetOption(option);
    switch (option) {
        case OPT_MARIO_HP: {
            value *= state.hp_level_;
            break;
        }
        case OPT_MARIO_FP: {
            value *= state.fp_level_;
            break;
        }
        case OPT_MARIO_BP: {
            value *= state.bp_level_;
            break;
        }
        case OPT_PARTNER_HP: {
            value *= state.hp_p_level_;
            // Scale the HP multiplier based on each party member's stats.
            value = (value * kHpMultipliers[party] + 50) / 100;
            break;
        }
        default:
            return -1;
    }
    // Stat values can never go below 1 or above 99.
    return Clamp(value, 1, 99);
}

void SetBaseStats() {
    auto& pouch = *ttyd::mario_pouch::pouchGetPtr();
    
    // Set starting HP, FP, BP.
    const int32_t hp = GetBaseStat(OPT_MARIO_HP);
    const int32_t fp = GetBaseStat(OPT_MARIO_FP);
    const int32_t bp = GetBaseStat(OPT_MARIO_BP);
    
    pouch.current_hp = hp;
    pouch.max_hp = hp;
    pouch.base_max_hp = hp;
    pouch.current_fp = fp;
    pouch.max_fp = fp;
    pouch.base_max_fp = fp;
    pouch.total_bp = bp;
    pouch.unallocated_bp = bp;
    
    // Disable partners, and set starting partner HP.
    for (int32_t i = 1; i <= 7; ++i) {
        pouch.party_data[i].flags &= ~1;
        
        const int32_t php = GetBaseStat(OPT_PARTNER_HP, i);
        pouch.party_data[i].current_hp = php;
        pouch.party_data[i].max_hp = php;
        pouch.party_data[i].base_max_hp = php;
            
        // Update ttyd::mario_pouch::_party_max_hp_table.
        ttyd::mario_pouch::_party_max_hp_table[i * 4] = pouch.party_data[i].max_hp;
    }
}
    
}

void OptionsManager::ApplyCurrentPresetOptions(bool first_time) {
    switch (g_Mod->state_.GetOptionValue(OPT_PRESET)) {
        case OPTVAL_PRESET_DEFAULT:
        case OPTVAL_PRESET_RTA_RACE: {
            for (const auto& data : g_OptionMetadata) {
                if (!data.check_for_default) continue;
                if (data.option == OPT_SECRET_BOSS) continue;
                g_Mod->state_.SetOption(data.option, GetDefaultValue(data.option));
            }
            if (first_time) {
                g_Mod->state_.SetOption(
                    OPT_TIMER_DISPLAY, GetDefaultValue(OPT_TIMER_DISPLAY));
                g_Mod->state_.SetOption(
                    OPT_SECRET_BOSS, GetDefaultValue(OPT_SECRET_BOSS));
            }
            break;
        }
        case OPTVAL_PRESET_CUSTOM: {
            if (first_time) {
                for (const auto& data : g_OptionMetadata) {
                    if (!data.check_for_default) continue;
                    g_Mod->state_.SetOption(data.option, GetDefaultValue(data.option));
                }
                g_Mod->state_.SetOption(
                    OPT_TIMER_DISPLAY, GetDefaultValue(OPT_TIMER_DISPLAY));
            }
            break;
        }
    }
}

int32_t OptionsManager::GetDefaultValue(uint32_t option) {
    int32_t value = -1;

    // Race settings / Race mode has different defaults.
    if (g_Mod->state_.CheckOptionValue(OPTVAL_RACE_MODE_ENABLED) ||
        g_Mod->state_.CheckOptionValue(OPTVAL_PRESET_RTA_RACE)) {
        if (option == OPT_TIMER_DISPLAY)
            return OPTVAL_TIMER_RTA & 0xff;
        if (option == OPT_SECRET_BOSS)
            return OPTVAL_SECRET_BOSS_OFF & 0xff;
        if (option == OPT_STARTER_ITEMS)
            return OPTVAL_STARTER_ITEMS_RANDOM & 0xff;
    } else {
        if (option == OPT_TIMER_DISPLAY) {
            return OPTVAL_TIMER_NONE & 0xff;
        }
    }

    switch (option) {
        case OPT_MAX_PARTNERS:
            if (g_Mod->state_.CheckOptionValue(OPTVAL_DIFFICULTY_HALF))
                return 3;
            return 4;
        case OPT_NPC_CHOICE_1:
        case OPT_NPC_CHOICE_2:
        case OPT_NPC_CHOICE_3:
            return gon::GetNumSecondaryNpcTypes();
        case OPT_NPC_CHOICE_4:
            if (g_Mod->state_.CheckOptionValue(OPTVAL_DIFFICULTY_HALF))
                return gon::GetNumSecondaryNpcTypes() + 1;
            return gon::GetNumSecondaryNpcTypes();
        default: {
            for (const auto& data : g_OptionMetadata) {
                if (data.option == option) {
                    if (data.default_optval)
                        return data.default_optval & 0xff;
                    return data.default_value;
                }
            }
            break;
        }
    }
    return value;
}

bool OptionsManager::IsDefault(uint32_t option) {
    for (const auto& data : g_OptionMetadata) {
        if (data.option == option) {
            if (!data.check_for_default) return true;
            return g_Mod->state_.GetOption(option) == GetDefaultValue(option);
        }
    }
    // Should not be reached for any valid option.
    return true;
}

bool OptionsManager::AllDefault() {
    for (const auto& data : g_OptionMetadata) {
        if (!data.check_for_default)
            continue;
        // Always allow changing secret boss option.
        if (data.option == OPT_SECRET_BOSS)
            continue;
        if (g_Mod->state_.GetOption(data.option) != GetDefaultValue(data.option))
            return false;
    }
    return true;
}

bool OptionsManager::AllDefaultExceptZeroStatLevels() {
    for (const auto& data : g_OptionMetadata) {
        if (!data.check_for_default)
            continue;
        // Always allow changing secret boss option.
        if (data.option == OPT_SECRET_BOSS)
            continue;
        // Allow stats to be set to zero.
        if (data.check_for_default_stat && !g_Mod->state_.GetOption(data.option))
            continue;
        if (g_Mod->state_.GetOption(data.option) != GetDefaultValue(data.option))
            return false;
    }
    return true;
}

int32_t OptionsManager::GetIntensity(uint32_t option) {
    const auto& state = g_Mod->state_;

    switch (option) {
        case OPT_DIFFICULTY:
            return state.CheckOptionValue(OPTVAL_DIFFICULTY_FULL_EX) ? 30 : 0;
        case OPT_NUM_CHESTS:
            switch (state.GetOption(option)) {
                case 1:
                    return 15;
                case 2:
                    return -10;
                case 3:
                    return -20;
                case 4:
                    return -30;
            }
            break;
        case OPT_STARTER_ITEMS:
            switch (state.GetOptionValue(option)) {
                case OPTVAL_STARTER_ITEMS_OFF:
                    return 5;
                case OPTVAL_STARTER_ITEMS_STRONG:
                    return -10;
                case OPTVAL_STARTER_ITEMS_CUSTOM:
                    return -30;
                default:
                    return 0;
            }
            break;
        case OPT_BATTLE_DROPS:
            switch (state.GetOptionValue(option)) {
                case OPTVAL_DROP_HELD_FROM_BONUS:
                case OPTVAL_DROP_NO_HELD_W_BONUS:
                    return 10;
                case OPTVAL_DROP_ALL_HELD:
                    return -30;
            }
            break;
        case OPT_REVIVE_PARTNERS:
            // If partners are disabled, partner revive setting is irrelevant.
            if (state.CheckOptionValue(OPTVAL_NO_PARTNERS)) return 0;
            return state.GetOption(option) ? 0 : 10;
        case OPT_MARIO_HP:
        case OPT_MARIO_FP:
        case OPT_MARIO_BP:
            return (5 - state.GetOption(option)) * 5;
        case OPT_PARTNER_HP:
            // If partners are disabled, their max HP setting is irrelevant.
            if (state.CheckOptionValue(OPTVAL_NO_PARTNERS)) return 0;
            return (5 - state.GetOption(option)) * 5;
        case OPT_INVENTORY_SACK_SIZE:
            return (2 - state.GetOption(option)) * 10;
        case OPT_AC_DIFFICULTY:
            return (state.GetOption(option) - 3) * 5;
        case OPTNUM_ENEMY_HP:
        case OPTNUM_ENEMY_ATK: {
            int32_t value = Clamp(state.GetOption(option), 0, 200);
            if (value > 100) {
                return (value - 100) * 2 / 5;
            } else {
                return (value - 100);
            }
        }
        case OPTNUM_SUPERGUARD_SP_COST: {
            int32_t sp_cost = state.GetOption(option);
            // 0 if disabled, or 5 plus 1 more for every 0.05 SP, rounded down.
            return sp_cost > 0 ? 5 + sp_cost / 5 : 0;
        }
        case OPT_AUDIENCE_RANDOM_THROWS:
            return state.GetOption(option) ? -30 : 0;
        case OPT_OBFUSCATE_ITEMS:
            return state.GetOption(option) ? 30 : 0;
        case OPT_RUN_AWAY:
            return state.GetOption(option) ? -10 : 0;
        case OPT_CHARLIETON_STOCK:
            switch (state.GetOptionValue(option)) {
                case OPTVAL_CHARLIETON_SMALLER:
                    return 10;
                case OPTVAL_CHARLIETON_LIMITED:
                case OPTVAL_CHARLIETON_TINY:
                    return 20;
            }
            break;
        case OPT_MOVE_AVAILABILITY:
            switch (state.GetOptionValue(option)) {
                case OPTVAL_MOVES_PARTNER_BONUS:
                case OPTVAL_MOVES_RANDOM:
                    return -15;
                case OPTVAL_MOVES_CUSTOM:
                    return -30;
            }
            break;
        case OPT_MOVE_LIMIT:
            // Adds 5% per additional level of restriction.
            return state.GetOption(option) * 5;
        case OPT_COUNTDOWN_TIMER: {
            int32_t level = state.GetOption(option);
            // Adds 10%, plus 5% per additional level of restriction.
            if (level) return (level + 1) * 5;
            break;
        }
    }

    // All other options / option values have no impact on intensity.
    return 0;
}

int32_t OptionsManager::GetTotalIntensity() {
    int32_t total = 100;
    for (const auto& data : g_OptionMetadata) {
        total += GetIntensity(data.option);
    }
    if (total < 5) total = 5;
    return total;
}

const char* OptionsManager::GetEncodedOptions() {
    static char encoding_str[28] = { 0 };
    int8_t encoding_bytes[28] = { 0 };
    // Start with version encoding, then period as separator from main encoding.
    encoding_bytes[0] = g_Mod->state_.version_ - 10;
    encoding_bytes[1] = 99;
    int32_t encoded_bit_count = 12;

    // Add additional indication for preset runs with non-default final boss.
    const char* boss_option = "";
    if (!OptionsManager::IsDefault(OPT_SECRET_BOSS)) {
        switch (g_Mod->state_.GetOptionValue(OPT_SECRET_BOSS)) {
            case OPTVAL_SECRET_BOSS_OFF:
                boss_option = "SB:N, ";    break;
            case OPTVAL_SECRET_BOSS_ON:
                boss_option = "SB:Y, ";    break;
            case OPTVAL_SECRET_BOSS_RANDOM:
                boss_option = "SB:?, ";    break;
        }
    }

    // If a preset is selected, use its name + the current version instead.
    switch (g_Mod->state_.GetOptionValue(OPT_PRESET)) {
        case OPTVAL_PRESET_DEFAULT:
            if (g_Mod->state_.CheckOptionValue(OPTVAL_RACE_MODE_ENABLED)) {
                sprintf(
                    encoding_str, "RTA Race (%s%s)",
                    boss_option, TitleScreenManager::GetVersionString());
            } else {
                sprintf(
                    encoding_str, "Default (%s%s)",
                    boss_option, TitleScreenManager::GetVersionString());
            }
            return encoding_str;
        case OPTVAL_PRESET_RTA_RACE:
            sprintf(
                encoding_str, "RTA Race (%s%s)",
                boss_option, TitleScreenManager::GetVersionString());
            return encoding_str;
    }

    // Encode all options that can be changed / have an effect on gameplay.
    for (const auto& data : g_OptionMetadata) {
        if (!data.check_for_default) continue;
        EncodeOption(encoding_bytes, encoded_bit_count, data.option);
    }

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

void OptionsManager::ResetAfterRun() {
    auto& state = g_Mod->state_;

    // Settings already cleared; don't need to be reset.
    if (GetSWF(GSWF_RunSettingsCleared)) return;

    // Un-obfuscate items if enabled during the previous run.
    if (state.GetOption(OPT_RUN_STARTED) && state.GetOption(OPT_OBFUSCATE_ITEMS)) {
        ObfuscateItems(false);
    }

    state.ResetOptions();
    // Set intensity level to default.
    state.SetOption(STAT_RUN_INTENSITY, 100);
    
    auto& pouch = *ttyd::mario_pouch::pouchGetPtr();
    
    for (int32_t i = 0; i < 20; ++i) pouch.items[i] = 0;
    for (int32_t i = 0; i < 200; ++i) pouch.badges[i] = 0;
    for (int32_t i = 0; i < 200; ++i) pouch.equipped_badges[i] = 0;
    pouch.coins = state.GetOption(STAT_PERM_CURRENT_COINS);
    pouch.star_pieces = state.GetOption(STAT_PERM_CURRENT_SP);
    pouch.shine_sprites = 0;
    pouch.star_points = 0;
    pouch.star_powers_obtained = 0;
    pouch.max_sp = 0;
    pouch.current_sp = 0;
    pouch.rank = 0;
    pouch.jump_level = 1;
    pouch.hammer_level = 1;
    // Only relevant to calculating "long fight" turn counts.
    pouch.level = 20;
    
    // Give a small amount of audience by default.
    pouch.audience_level = 10.0f;
    
    // Remove tier 2+ equipment if it was obtained in a previous run.
    ttyd::mario_pouch::pouchRemoveItem(ItemType::SUPER_BOOTS);
    ttyd::mario_pouch::pouchRemoveItem(ItemType::ULTRA_BOOTS);
    ttyd::mario_pouch::pouchRemoveItem(ItemType::SUPER_HAMMER);
    ttyd::mario_pouch::pouchRemoveItem(ItemType::ULTRA_HAMMER);
    
    // Assign Yoshi his default color.
    ttyd::mario_pouch::pouchSetPartyColor(4, 0);
    
    // Set starting HP, FP, BP.
    SetBaseStats();
    
    // Initialize default moves.
    MoveManager::Init();
    
    // Set run to not having started.
    state.SetOption(OPT_RUN_STARTED, 0);
    // Set flag indicating settings have already been cleared.
    SetSWF(GSWF_RunSettingsCleared, 1);
}

void OptionsManager::OnRunStart() {
    auto& state = g_Mod->state_;
    auto& pouch = *ttyd::mario_pouch::pouchGetPtr();

    // Select random seed or convert string seed to hash if necessary.
    if (state.GetOption(OPT_USE_SEED_NAME)) {
        state.HashSeedName();
    } else if (state.seed_ == 0) {
        state.SelectRandomSeed();
        state.SetOption(OPT_UNSEEDED_RUN, 1);
    }

    // Force special options for tutorial runs.
    switch (GetSWByte(GSW_Tower_TutorialClears)) {
        case 0:
            state.SetOption(OPTVAL_PRESET_CUSTOM);
            // Hooktail's tower.
            state.SetOption(OPTVAL_DIFFICULTY_HALF);
            state.SetOption(OPT_MAX_PARTNERS, 3);
            // Always start with Goombella.
            state.SetOption(OPTVAL_PARTNER_GOOMBELLA);
            // Disable NPCs.
            state.SetOption(OPT_NPC_CHOICE_1, gon::GetNumSecondaryNpcTypes() + 1);
            state.SetOption(OPT_NPC_CHOICE_2, gon::GetNumSecondaryNpcTypes() + 1);
            state.SetOption(OPT_NPC_CHOICE_3, gon::GetNumSecondaryNpcTypes() + 1);
            state.SetOption(OPT_NPC_CHOICE_4, gon::GetNumSecondaryNpcTypes() + 1);
            // Disable secret boss.
            state.SetOption(OPTVAL_SECRET_BOSS_OFF);
            break;
        case 1:
            state.SetOption(OPTVAL_PRESET_CUSTOM);
            // Gloomtail's tower.
            state.SetOption(OPTVAL_DIFFICULTY_FULL);
            state.SetOption(OPT_MAX_PARTNERS, 4);
            // Force the first four NPC types to ease the player into mechanics.
            state.SetOption(OPT_NPC_CHOICE_1, 0);
            state.SetOption(OPT_NPC_CHOICE_2, 1);
            state.SetOption(OPT_NPC_CHOICE_3, 2);
            state.SetOption(OPT_NPC_CHOICE_4, 3);
            // Disable secret boss.
            state.SetOption(OPTVAL_SECRET_BOSS_OFF);
            break;
        default:
            break;
    }

    // Make levels for disabled / infinite stats 0 / 99 respectively.
    if (state.GetOption(OPT_MARIO_HP) == 0) state.hp_level_ = 0;
    if (state.GetOption(OPT_MARIO_FP) == 0) state.fp_level_ = 0;
    if (state.GetOption(OPT_MARIO_BP) == 0) state.bp_level_ = 0;
    if (state.GetOption(OPT_PARTNER_HP) == 0) state.hp_p_level_ = 0;
    if (state.CheckOptionValue(OPTVAL_INFINITE_BP)) state.bp_level_ = 99;

    // Set starting HP, FP, BP.
    SetBaseStats();

    pouch.star_powers_obtained = 0b11;
    pouch.max_sp = 300;
    pouch.coins = 0;
    pouch.star_pieces = 0;
    
    // Apply starting items.
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
            // Give 4 to 6 random items, based on the seed.
            int32_t num_items = state.Rand(3, RNG_STARTER_ITEM) + 4;
            for (int32_t i = 0; i < num_items; ++i) {
                int32_t item_type = PickRandomItem(RNG_STARTER_ITEM, 15, 5, 0, 0);
                ttyd::mario_pouch::pouchGetItem(item_type);
            }
            break;
        }
        case OPTVAL_STARTER_ITEMS_CUSTOM: {
            // Give the items and badges in the player's custom loadout.
            int32_t num_items = state.GetOption(STAT_PERM_ITEM_LOAD_SIZE);
            int32_t num_badges = state.GetOption(STAT_PERM_BADGE_LOAD_SIZE);
            for (int32_t i = num_items - 1; i >= 0; --i) {
                int32_t item_id = state.GetOption(STAT_PERM_ITEM_LOADOUT, i)
                    + ItemType::THUNDER_BOLT;
                ttyd::mario_pouch::pouchGetItem(item_id);
            }
            for (int32_t i = num_badges - 1; i >= 0; --i) {
                int32_t item_id = state.GetOption(STAT_PERM_BADGE_LOADOUT, i)
                    + ItemType::THUNDER_BOLT;
                ttyd::mario_pouch::pouchGetItem(item_id);
            }
            break;
        }
    }

    // Apply starting moves.
    if (state.CheckOptionValue(OPTVAL_MOVES_CUSTOM)) {
        int32_t num_moves = state.GetOption(STAT_PERM_MOVE_LOAD_SIZE);
        for (int32_t i = 0; i < num_moves; ++i) {
            int32_t move = state.GetOption(STAT_PERM_MOVE_LOADOUT, i);
            MoveManager::UpgradeMove(move);
        }
    }
    
    // Start with Merlee curse, if enabled (not supported yet).
    // TODO: If Merlee is ever supported, make her unable to buff EXP gain.
    if (state.GetOption(OPT_MERLEE_CURSE)) {
        pouch.merlee_curse_uses_remaining = 99;
        pouch.turns_until_merlee_activation = -1;
    } else {
        pouch.merlee_curse_uses_remaining = 0;
        pouch.turns_until_merlee_activation = 0;
    }
    
    // Assign Yoshi a random color.
    CosmeticsManager::PickYoshiColor();

    // Run item obfuscation, if enabled.
    if (g_Mod->state_.GetOption(OPT_OBFUSCATE_ITEMS)) {
        g_Mod->state_.rng_states_[RNG_ITEM_OBFUSCATION] = 0;
        ObfuscateItems(true);
    }

    OnRunResumeFromFileSelect();

    switch (state.GetOptionValue(OPT_DIFFICULTY)) {
        case OPTVAL_DIFFICULTY_HALF:
            state.ChangeOption(STAT_PERM_HALF_ATTEMPTS, 1);
            break;
        case OPTVAL_DIFFICULTY_FULL:
            state.ChangeOption(STAT_PERM_FULL_ATTEMPTS, 1);
            break;
        case OPTVAL_DIFFICULTY_FULL_EX:
            state.ChangeOption(STAT_PERM_EX_ATTEMPTS, 1);
            break;
        default:
            break;
    }

    // Update global variable to track whether the first / second tutorial runs
    // were started, or a run was started after the second tutorial run.
    int32_t tut_clears = GetSWByte(GSW_Tower_TutorialClears);
    if (GetSWByte(GSW_Tower_TutorialClearAttempts) <= tut_clears) {
        SetSWByte(GSW_Tower_TutorialClearAttempts, tut_clears + 1);
    }

    // Set flag to indicate settings should be cleared on next lobby/hub load.
    SetSWF(GSWF_RunSettingsCleared, 0);
    // Clear state of Koopa's "run results" conversation.
    SetSWByte(GSW_NpcA_SpecialConversation, 0);
    
    // Start timers and mark run as started.
    state.SetOption(OPT_RUN_STARTED, 1);
    state.TimerStart();
}

void OptionsManager::OnRunResumeFromAutoSave() {
    // Correct party max HP stats.
    auto& pouch = *ttyd::mario_pouch::pouchGetPtr();
    for (int32_t i = 1; i <= 7; ++i) {
        ttyd::mario_pouch::_party_max_hp_table[i * 4] 
            = pouch.party_data[i].max_hp;
    }
}

void OptionsManager::OnRunResumeFromFileSelect() {
    // Redo item obfuscation, if enabled and reloading during a run.
    if (g_Mod->state_.GetOption(OPT_RUN_STARTED) &&
        g_Mod->state_.GetOption(OPT_OBFUSCATE_ITEMS)) {
        g_Mod->state_.rng_states_[RNG_ITEM_OBFUSCATION] = 0;
        ObfuscateItems(true);
    }

    // Correct party max HP stats.
    auto& pouch = *ttyd::mario_pouch::pouchGetPtr();
    for (int32_t i = 1; i <= 7; ++i) {
        ttyd::mario_pouch::_party_max_hp_table[i * 4] 
            = pouch.party_data[i].max_hp;
    }
}

void OptionsManager::UpdateLevelupStats() {
    auto& pouch = *ttyd::mario_pouch::pouchGetPtr();
    
    // Get change HP, FP, BP, party HP (using Goombella by default).
    const int32_t delta_hp = GetBaseStat(OPT_MARIO_HP) - pouch.max_hp;
    const int32_t delta_fp = GetBaseStat(OPT_MARIO_FP) - pouch.max_fp;
    const int32_t delta_bp = GetBaseStat(OPT_MARIO_BP) - pouch.total_bp;
    const int32_t delta_php =
        GetBaseStat(OPT_PARTNER_HP, 1) - pouch.party_data[1].max_hp;
    
    // Update stats that have changed.
    if (delta_hp > 0) {
        pouch.current_hp += delta_hp;
        pouch.max_hp += delta_hp;
        pouch.base_max_hp += delta_hp;
    } else if (delta_hp < 0) {
        pouch.max_hp += delta_hp;
        pouch.base_max_hp += delta_hp;
        if (pouch.current_hp > pouch.max_hp) pouch.current_hp = pouch.max_hp;
    }
    
    if (delta_fp > 0) {
        pouch.current_fp += delta_fp;
        pouch.max_fp += delta_fp;
        pouch.base_max_fp += delta_fp;
    } else if (delta_fp < 0) {
        pouch.max_fp += delta_fp;
        pouch.base_max_fp += delta_fp;
        if (pouch.current_fp > pouch.max_fp) pouch.current_fp = pouch.max_fp;
    }
    
    if (delta_bp != 0) {
        pouch.total_bp += delta_bp;
        pouch.unallocated_bp += delta_bp;
        // If not enough unallocated bp, forcibly unequip all badges.
        if (pouch.unallocated_bp < 0) {
            for (int32_t i = 0; i < 200; ++i) pouch.equipped_badges[i] = 0;
            pouch.unallocated_bp = pouch.total_bp;
        }
    }
    
    if (delta_php != 0) {
        for (int32_t i = 1; i <= 7; ++i) {
            int32_t delta_php = 
                GetBaseStat(OPT_PARTNER_HP, i) - pouch.party_data[i].max_hp;
            pouch.party_data[i].max_hp += delta_php;
            pouch.party_data[i].base_max_hp += delta_php;
            if (delta_php > 0) {
                pouch.party_data[i].current_hp += delta_php;
            } else if (pouch.party_data[i].current_hp > pouch.party_data[i].max_hp) {
                pouch.party_data[i].current_hp = pouch.party_data[i].max_hp;
            }
            
            // Update ttyd::mario_pouch::_party_max_hp_table.
            ttyd::mario_pouch::_party_max_hp_table[i * 4] = pouch.party_data[i].max_hp;
        }
    }
}

EVT_DEFINE_USER_FUNC(evtTot_ResetSettingsAfterRun) {
    OptionsManager::ResetAfterRun();
    return 2;
}

}  // namespace mod::tot