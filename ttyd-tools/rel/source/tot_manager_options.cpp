#include "tot_manager_options.h"

#include "common_functions.h"
#include "mod.h"
#include "tot_generate_item.h"
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
    // Un-obfuscate items if previously enabled.
    if (g_Mod->state_.GetOption(tot::OPT_OBFUSCATE_ITEMS)) {
        ObfuscateItems(false);
    }

    g_Mod->state_.InitDefaultOptions();
    
    auto& pouch = *ttyd::mario_pouch::pouchGetPtr();
    
    for (int32_t i = 0; i < 20; ++i) pouch.items[i] = 0;
    for (int32_t i = 0; i < 121; ++i) pouch.key_items[i] = 0;
    for (int32_t i = 0; i < 200; ++i) pouch.badges[i] = 0;
    for (int32_t i = 0; i < 200; ++i) pouch.equipped_badges[i] = 0;
    pouch.coins = 0;
    pouch.star_pieces = 0;
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
    
    // Update any stats / equipment / flags as necessary.
    ttyd::mario_pouch::pouchGetItem(ItemType::BOOTS);
    ttyd::mario_pouch::pouchGetItem(ItemType::HAMMER);
    ttyd::mario_pouch::pouchGetItem(ItemType::W_EMBLEM);
    ttyd::mario_pouch::pouchGetItem(ItemType::L_EMBLEM);
    
    // Assign Yoshi a totally random color.
    ttyd::mario_pouch::pouchSetPartyColor(4, g_Mod->state_.Rand(7));
    
    // Assign Peekaboo and Timing Tutor (for testing; might make optional).
    ttyd::mario_pouch::pouchGetItem(ItemType::TIMING_TUTOR);
    ttyd::mario_pouch::pouchEquipBadgeID(ItemType::TIMING_TUTOR);
    ttyd::mario_pouch::pouchGetItem(ItemType::PEEKABOO);
    ttyd::mario_pouch::pouchEquipBadgeID(ItemType::PEEKABOO);
    
    // Set starting HP, FP, BP.
    SetBaseStats();
    
    // Initialize default moves.
    MoveManager::Init();
    
    // Set run to not having started.
    g_Mod->state_.SetOption(OPT_RUN_STARTED, 0);
    g_Mod->state_.SetOption(OPT_DEBUG_MODE_USED, 0);
}

void OptionsManager::InitFromSelectedOptions() {
    auto& state = g_Mod->state_;
    auto& pouch = *ttyd::mario_pouch::pouchGetPtr();
    if (state.seed_ == 0) state.PickRandomSeed();

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
                int32_t item_type = PickRandomItem(RNG_STARTER_ITEM, 10, 5, 0, 0);
                ttyd::mario_pouch::pouchGetItem(item_type);
            }
            break;
        }
    }
    
    // Start with Merlee curse, if enabled.
    // TODO: If enabling Merlee, make ToT Merlee unable to buff EXP gain.
    if (state.GetOption(tot::OPT_MERLEE_CURSE)) {
        pouch.merlee_curse_uses_remaining = 99;
        pouch.turns_until_merlee_activation = -1;
    } else {
        pouch.merlee_curse_uses_remaining = 0;
        pouch.turns_until_merlee_activation = 0;
    }

    ApplyOptionsOnLoad();
    
    // Start timers and mark run as started.
    state.SetOption(OPT_RUN_STARTED, 1);
    state.TimerStart();
}

void OptionsManager::ApplyOptionsOnLoad() {
    // Force item obfuscation, if enabled.
    if (g_Mod->state_.GetOption(tot::OPT_OBFUSCATE_ITEMS)) {
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