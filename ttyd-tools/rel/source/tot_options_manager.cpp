#include "tot_options_manager.h"

#include "mod.h"
#include "tot_state.h"

#include <ttyd/mario_pouch.h>

namespace mod::tot {
    
namespace {

// Mario, then partners in internal order.
const int32_t kBaseHP[] = { 10, 10, 8, 12, 8, 12, 10, 8 };
const int32_t kBaseFP = 10;
const int32_t kBaseBP = 10;
    
}

void OptionsManager::InitFromSelectedOptions() {
    auto& pouch = *ttyd::mario_pouch::pouchGetPtr();
    auto& state = infinite_pit::g_Mod->state_;
    
    // Set starting HP, FP, BP.
    const int32_t hp = kBaseHP[0] * state.hp_level_ / 2;
    const int32_t fp = kBaseFP * state.fp_level_ / 2;
    const int32_t bp = kBaseBP * state.bp_level_ / 2;
    
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
        const int32_t php = kBaseHP[i] * state.hp_p_level_ / 2;
        
        pouch.party_data[i].current_hp = php;
        pouch.party_data[i].max_hp = php;
        pouch.party_data[i].base_max_hp = php;
            
        // Update ttyd::mario_pouch::_party_max_hp_table.
        ttyd::mario_pouch::_party_max_hp_table[i * 4] = pouch.party_data[i].max_hp;
    }
}

void OptionsManager::UpdateLevelupStats() {
    auto& pouch = *ttyd::mario_pouch::pouchGetPtr();
    auto& state = infinite_pit::g_Mod->state_;
    
    // Get change HP, FP, BP, party HP.
    const int32_t delta_hp = kBaseHP[0] * state.hp_level_ / 2 - pouch.max_hp;
    const int32_t delta_fp = kBaseFP * state.fp_level_ / 2 - pouch.max_fp;
    const int32_t delta_bp = kBaseBP * state.bp_level_ / 2 - pouch.total_bp;
    const int32_t delta_php = 
        kBaseHP[1] * state.hp_p_level_ / 2 - pouch.party_data[1].max_hp;
    
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
            int32_t delta_php = kBaseHP[i] * state.hp_p_level_ / 2 
                              - pouch.party_data[i].max_hp;
            pouch.party_data[i].max_hp += delta_php;
            pouch.party_data[i].base_max_hp += delta_php;
            if (delta_php > 0) {
                pouch.party_data[i].current_hp += delta_php;
            } else if (pouch.party_data[i].current_hp < pouch.party_data[i].max_hp) {
                pouch.party_data[i].current_hp = pouch.party_data[i].max_hp;
            }
            
            // Update ttyd::mario_pouch::_party_max_hp_table.
            ttyd::mario_pouch::_party_max_hp_table[i * 4] = pouch.party_data[i].max_hp;
        }
    }
}

}  // namespace mod::tot