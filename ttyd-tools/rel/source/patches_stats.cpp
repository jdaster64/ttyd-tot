#include "patches_stats.h"

#include "mod.h"
#include "tot_state.h"
#include "patch.h"

#include <ttyd/battle.h>
#include <ttyd/battle_actrecord.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_unit.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/swdrv.h>

#include <cstdint>

namespace mod::infinite_pit {

namespace {

using ::ttyd::battle_database_common::BattleWeapon;
using ::ttyd::battle_unit::BattleWorkUnit;
using ::ttyd::battle_unit::BattleWorkUnitPart;

namespace BattleUnitType = ::ttyd::battle_database_common::BattleUnitType;
namespace ItemType = ::ttyd::item_data::ItemType;

}

// Function hooks.
extern void (*g_BtlUnit_PayWeaponCost_trampoline)(
    BattleWorkUnit*, BattleWeapon*);
extern int32_t (*g_pouchAddCoin_trampoline)(int16_t);
extern void (*g_BtlActRec_AddCount_trampoline)(uint8_t*);

namespace stats {
    
void ApplyFixedPatches() {        
    g_BtlUnit_PayWeaponCost_trampoline = mod::patch::hookFunction(
        ttyd::battle_unit::BtlUnit_PayWeaponCost, [](
            BattleWorkUnit* unit, BattleWeapon* weapon) {
            // Track FP / SP spent.
            const int32_t fp_cost = BtlUnit_GetWeaponCost(unit, weapon);
            g_Mod->state_.ChangeOption(tot::STAT_RUN_FP_SPENT, fp_cost);
            g_Mod->state_.ChangeOption(tot::STAT_PERM_FP_SPENT, fp_cost);
            g_Mod->state_.ChangeOption(tot::STAT_RUN_SP_SPENT, weapon->base_sp_cost);
            g_Mod->state_.ChangeOption(tot::STAT_PERM_SP_SPENT, weapon->base_sp_cost);
            // Run normal pay-weapon-cost logic.
            g_BtlUnit_PayWeaponCost_trampoline(unit, weapon);
        });

    g_pouchAddCoin_trampoline = mod::patch::hookFunction(
        ttyd::mario_pouch::pouchAddCoin, [](int16_t coins) {
            // Track coins gained / spent (stolen coins subtract from gained).
            if (coins < 0 && !ttyd::mariost::g_MarioSt->bInBattle) {
                g_Mod->state_.ChangeOption(tot::STAT_RUN_COINS_SPENT, -coins);
                g_Mod->state_.ChangeOption(tot::STAT_PERM_COINS_SPENT, -coins);
            } else {
                g_Mod->state_.ChangeOption(tot::STAT_RUN_COINS_EARNED, coins);
                g_Mod->state_.ChangeOption(tot::STAT_PERM_COINS_EARNED, coins);
            }
            // Run coin increment logic.
            return g_pouchAddCoin_trampoline(coins);
        });

    g_BtlActRec_AddCount_trampoline = mod::patch::hookFunction(
        ttyd::battle_actrecord::BtlActRec_AddCount, [](uint8_t* counter) {
            auto& actRecordWork = ttyd::battle::g_BattleWork->act_record_work;
            // Track every time an item is used by the player in-battle.
            if (counter == &actRecordWork.mario_num_times_attack_items_used ||
                counter == &actRecordWork.mario_num_times_non_attack_items_used ||
                counter == &actRecordWork.partner_num_times_attack_items_used ||
                counter == &actRecordWork.partner_num_times_non_attack_items_used) {
                g_Mod->state_.ChangeOption(tot::STAT_RUN_ITEMS_USED);
                g_Mod->state_.ChangeOption(tot::STAT_PERM_ITEMS_USED);
            }
            // Run act record counting logic.
            g_BtlActRec_AddCount_trampoline(counter); 
        });
}

}  // namespace stats
}  // namespace mod::infinite_pit