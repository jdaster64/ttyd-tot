#include "tot_manager_options.h"

#include "common_functions.h"
#include "mod.h"
#include "tot_generate_item.h"
#include "tot_generate_reward.h"
#include "tot_gsw.h"
#include "tot_manager_cosmetics.h"
#include "tot_state.h"

#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>

namespace mod::tot {
    
namespace {

namespace ItemType = ::ttyd::item_data::ItemType;

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

void OptionsManager::InitLobby() {
    auto& state = g_Mod->state_;

    // Un-obfuscate items if enabled during the previous run.
    if (state.GetOption(OPT_RUN_STARTED) && state.GetOption(OPT_OBFUSCATE_ITEMS)) {
        ObfuscateItems(false);
    }

    state.InitDefaultOptions();
    
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
}

void OptionsManager::InitFromSelectedOptions() {
    auto& state = g_Mod->state_;
    auto& pouch = *ttyd::mario_pouch::pouchGetPtr();
    if (state.seed_ == 0) state.SelectRandomSeed();

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
                int32_t item_id =
                    static_cast<uint8_t>(state.GetOption(STAT_PERM_ITEM_LOADOUT, i))
                    + ItemType::THUNDER_BOLT;
                ttyd::mario_pouch::pouchGetItem(item_id);
            }
            for (int32_t i = num_badges - 1; i >= 0; --i) {
                int32_t item_id =
                    static_cast<uint8_t>(state.GetOption(STAT_PERM_BADGE_LOADOUT, i))
                    + ItemType::THUNDER_BOLT;
                ttyd::mario_pouch::pouchGetItem(item_id);
            }
            break;
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

    ApplyOptionsOnLoad();

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
    
    // Start timers and mark run as started.
    state.SetOption(OPT_RUN_STARTED, 1);
    state.TimerStart();
}

void OptionsManager::ApplyOptionsOnLoad() {
    // Redo item obfuscation, if enabled.
    if (g_Mod->state_.GetOption(OPT_OBFUSCATE_ITEMS)) {
        g_Mod->state_.rng_states_[RNG_ITEM_OBFUSCATION] = 0;
        ObfuscateItems(true);
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

}  // namespace mod::tot