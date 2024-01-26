#include "patches_stats.h"

#include "custom_item.h"
#include "mod.h"
#include "mod_state.h"
#include "patch.h"

#include <ttyd/battle.h>
#include <ttyd/battle_actrecord.h>
#include <ttyd/battle_damage.h>
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
extern void (*g_BattleDamageDirect_trampoline)(
    int32_t, BattleWorkUnit*, BattleWorkUnitPart*, int32_t, int32_t,
    uint32_t, uint32_t, uint32_t);
extern void (*g_BtlUnit_PayWeaponCost_trampoline)(
    BattleWorkUnit*, BattleWeapon*);
extern uint32_t (*g_pouchGetItem_trampoline)(int32_t);
extern int32_t (*g_pouchAddCoin_trampoline)(int16_t);
extern void (*g_BtlActRec_AddCount_trampoline)(uint8_t*);

namespace stats {
    
void ApplyFixedPatches() {    
    g_BattleDamageDirect_trampoline = mod::patch::hookFunction(
        ttyd::battle_damage::BattleDamageDirect, [](
            int32_t unit_idx, BattleWorkUnit* target, BattleWorkUnitPart* part,
            int32_t damage, int32_t fp_damage, uint32_t unk0, 
            uint32_t damage_pattern, uint32_t unk1) {
            // Save original damage so elemental healing still works.
            const int32_t original_damage = damage;
            // Track damage taken, if target is player/enemy and damage > 0.
            if (target->current_kind == BattleUnitType::MARIO ||
                target->current_kind >= BattleUnitType::GOOMBELLA) {
                if (damage < 0) damage = 0;
                if (damage > 99) damage = 99;
                g_Mod->state_.ChangeOption(STAT_PLAYER_DAMAGE, damage);
            } else if (target->current_kind <= BattleUnitType::BONETAIL) {
                if (damage < 0) damage = 0;
                if (damage > 99) damage = 99;
                g_Mod->state_.ChangeOption(STAT_ENEMY_DAMAGE, damage);
            }
            // Run normal damage logic.
            g_BattleDamageDirect_trampoline(
                unit_idx, target, part, original_damage, fp_damage, 
                unk0, damage_pattern, unk1);
        });
        
    g_BtlUnit_PayWeaponCost_trampoline = mod::patch::hookFunction(
        ttyd::battle_unit::BtlUnit_PayWeaponCost, [](
            BattleWorkUnit* unit, BattleWeapon* weapon) {
            // Track FP / SP spent.
            const int32_t fp_cost = BtlUnit_GetWeaponCost(unit, weapon);
            g_Mod->state_.ChangeOption(STAT_FP_SPENT, fp_cost);
            g_Mod->state_.ChangeOption(STAT_SP_SPENT, weapon->base_sp_cost);
            // Run normal pay-weapon-cost logic.
            g_BtlUnit_PayWeaponCost_trampoline(unit, weapon);
        });

    g_pouchGetItem_trampoline = mod::patch::hookFunction(
        ttyd::mario_pouch::pouchGetItem, [](int32_t item_type) {
            // Track coins gained.
            if (item_type == ItemType::COIN) {
                g_Mod->state_.ChangeOption(STAT_COINS_EARNED);
            }
            
            // If badge is a "P" badge and playing Mario-alone, also mark
            // off the relevant "P" badge in the badge log.
            if (g_Mod->state_.CheckOptionValue(OPTVAL_PARTNERS_NEVER)
                && !g_Mod->state_.GetOptionNumericValue(OPT_FIRST_PARTNER)
                && IsStackableMarioBadge(item_type)) {
                ttyd::swdrv::swSet(0x81 + item_type - ItemType::POWER_JUMP);
            }
            
            // Run coin increment logic.
            return g_pouchGetItem_trampoline(item_type);
        });

    g_pouchAddCoin_trampoline = mod::patch::hookFunction(
        ttyd::mario_pouch::pouchAddCoin, [](int16_t coins) {
            // Track coins gained / lost; if a reward floor, assume lost
            // coins were spent on badges / items from Charlieton.
            if (coins < 0 && g_Mod->state_.floor_ % 10 == 9) {
                g_Mod->state_.ChangeOption(STAT_COINS_SPENT, -coins);
            } else {
                g_Mod->state_.ChangeOption(STAT_COINS_EARNED, coins);
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
                g_Mod->state_.ChangeOption(STAT_ITEMS_USED);
            }
            // Run act record counting logic.
            g_BtlActRec_AddCount_trampoline(counter); 
        });
}

}  // namespace stats
}  // namespace mod::infinite_pit