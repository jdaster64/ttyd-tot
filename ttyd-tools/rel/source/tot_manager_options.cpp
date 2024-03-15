#include "tot_manager_options.h"

#include "mod.h"
#include "tot_state.h"

#include <ttyd/mario_pouch.h>

namespace mod::tot {
    
namespace {

// Mario, then partners in internal order.
const int32_t kHpMultipliers[] = { 100, 100, 80, 120, 80, 120, 100, 80 };

// Returns the current expected base stat given its option value and
// corresponding level (e.g. OPTNUM_MARIO_HP + hp_level_).
int32_t GetBaseStat(uint32_t option, int32_t party = 0) {
    auto& state = infinite_pit::g_Mod->state_;
    int32_t value = state.GetOption(option);
    switch (option) {
        case OPTNUM_MARIO_HP: {
            value *= state.hp_level_;
            break;
        }
        case OPTNUM_MARIO_FP: {
            value *= state.fp_level_;
            break;
        }
        case OPTNUM_MARIO_BP: {
            value *= state.bp_level_;
            break;
        }
        case OPTNUM_PARTNER_HP: {
            value *= state.hp_p_level_;
            // Scale the HP multiplier based on each party member's stats.
            value = (value * kHpMultipliers[party] + 50) / 100;
            break;
        }
        default:
            return -1;
    }
    // Stat values can never go below 1.
    return value > 0 ? value : 1;
}
    
}

void OptionsManager::InitFromSelectedOptions() {
    auto& pouch = *ttyd::mario_pouch::pouchGetPtr();
    
    // Set starting HP, FP, BP.
    const int32_t hp = GetBaseStat(OPTNUM_MARIO_HP);
    const int32_t fp = GetBaseStat(OPTNUM_MARIO_FP);
    const int32_t bp = GetBaseStat(OPTNUM_MARIO_BP);
    
    pouch.current_hp = hp;
    pouch.max_hp = hp;
    pouch.base_max_hp = hp;
    pouch.current_fp = fp;
    pouch.max_fp = fp;
    pouch.base_max_fp = fp;
    pouch.total_bp = bp;
    pouch.unallocated_bp = bp;
    
    // Set starting partner HP.
    for (int32_t i = 1; i <= 7; ++i) {
        const int32_t php = GetBaseStat(OPTNUM_PARTNER_HP, i);
        
        pouch.party_data[i].current_hp = php;
        pouch.party_data[i].max_hp = php;
        pouch.party_data[i].base_max_hp = php;
            
        // Update ttyd::mario_pouch::_party_max_hp_table.
        ttyd::mario_pouch::_party_max_hp_table[i * 4] = pouch.party_data[i].max_hp;
    }
}

void OptionsManager::UpdateLevelupStats() {
    auto& pouch = *ttyd::mario_pouch::pouchGetPtr();
    
    // Get change HP, FP, BP, party HP (using Goombella by default).
    const int32_t delta_hp = GetBaseStat(OPTNUM_MARIO_HP) - pouch.max_hp;
    const int32_t delta_fp = GetBaseStat(OPTNUM_MARIO_FP) - pouch.max_fp;
    const int32_t delta_bp = GetBaseStat(OPTNUM_MARIO_BP) - pouch.total_bp;
    const int32_t delta_php =
        GetBaseStat(OPTNUM_PARTNER_HP, 1) - pouch.party_data[1].max_hp;
    
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
            int32_t delta_php = GetBaseStat(OPTNUM_PARTNER_HP, i)
                              - pouch.party_data[i].max_hp;
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