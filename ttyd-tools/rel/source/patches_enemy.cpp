#include "patches_enemy.h"

#include "custom_enemy.h"
#include "mod.h"
#include "mod_state.h"
#include "patch.h"

#include <ttyd/battle.h>
#include <ttyd/battle_damage.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_enemy_item.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_sub.h>
#include <ttyd/battle_unit.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/system.h>

#include <cstdint>

namespace mod::infinite_pit {

namespace {
    
using ::ttyd::battle_database_common::BattleUnitKind;
using ::ttyd::battle_database_common::BattleUnitSetup;
using ::ttyd::battle_database_common::BattleWeapon;
using ::ttyd::battle_unit::BattleWorkUnit;
using ::ttyd::battle_unit::BattleWorkUnitPart;
using ::ttyd::evtmgr::EvtEntry;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

namespace BattleUnitType = ::ttyd::battle_database_common::BattleUnitType;
namespace ItemType = ::ttyd::item_data::ItemType;

}

// Function hooks.
extern BattleWorkUnit* (*g_BtlUnit_Entry_trampoline)(BattleUnitSetup*);
extern int32_t (*g_BattleCalculateDamage_trampoline)(
    BattleWorkUnit*, BattleWorkUnit*, BattleWorkUnitPart*, BattleWeapon*,
    uint32_t*, uint32_t);
extern int32_t (*g_BattleCalculateFpDamage_trampoline)(
    BattleWorkUnit*, BattleWorkUnit*, BattleWorkUnitPart*, BattleWeapon*,
    uint32_t*, uint32_t);
extern int32_t (*g_btlevtcmd_ConsumeItem_trampoline)(EvtEntry*, bool);
extern int32_t (*g_btlevtcmd_GetConsumeItem_trampoline)(EvtEntry*, bool);
extern void* (*g_BattleEnemyUseItemCheck_trampoline)(BattleWorkUnit*);
// Patch addresses.
extern const int32_t g_BtlUnit_EnemyItemCanUseCheck_Patch_SkipCheck;

namespace enemy {
    
namespace {

// Global variable for the last type of item consumed;
// this is necessary to allow enemies to use cooked items.
int32_t g_EnemyItem = 0;

void AlterUnitKindParams(BattleUnitKind* unit) {
    // If not an enemy, nothing to change.
    if (unit->unit_type > BattleUnitType::BONETAIL) return;
    
    int32_t hp, level, coinlvl;
    if (!GetEnemyStats(
        unit->unit_type, &hp, nullptr, nullptr, &level, &coinlvl)) return;
    unit->max_hp = hp;
    
    if (ttyd::mario_pouch::pouchGetPtr()->level >= 99) {
        // Assign enemies a high level so you can't Gale Force them to oblivion.
        unit->level = 99;
        unit->bonus_exp = 0;
    } else if (level >= 0) {
        unit->level = level;
        unit->bonus_exp = 0;
    } else {
        // If negative, give it as bonus EXP instead (to avoid level overflow).
        unit->level = ttyd::mario_pouch::pouchGetPtr()->level + 1;
        unit->bonus_exp = -level;
    }
    
    unit->base_coin = coinlvl / 2;
    // Give an additional coin half the time if coinlvl is odd.
    unit->bonus_coin_rate = 50;
    unit->bonus_coin = coinlvl & 1;
    
    // Additional global changes for enemies in this mod.
    unit->itemsteal_param = 20;
}

int32_t AlterDamageCalculation(
    BattleWorkUnit* attacker, BattleWorkUnit* target,
    BattleWorkUnitPart* target_part, BattleWeapon* weapon,
    uint32_t* unk0, uint32_t unk1) {
    int32_t base_atk = weapon->damage_function_params[0];
    int8_t* def_ptr  = target_part->defense;
    int32_t base_def = def_ptr[weapon->element];
    
    int32_t altered_atk = base_atk, altered_def = base_def;
    // Alter ATK power for enemy attacks.
    if (attacker->current_kind <= BattleUnitType::BONETAIL
        && !(weapon->target_property_flags & 0x100000)  // not a recoil attack
        && !weapon->item_id && base_atk > 0) {
        GetEnemyStats(
            attacker->current_kind, nullptr, &altered_atk, nullptr, 
            nullptr, nullptr, base_atk);
        if (altered_atk < 1) altered_atk = 1;
        if (altered_atk > 99) altered_atk = 99;
        weapon->damage_function_params[0] = altered_atk;
    }
    // Alter DEF power for enemies on defense.
    if (target->current_kind <= BattleUnitType::BONETAIL
        && base_def >= 0 && base_def < 99) {
        if (base_def > 0) {
            GetEnemyStats(
                target->current_kind, nullptr, nullptr, &altered_def, 
                nullptr, nullptr);
        }
        if (altered_def > 99) altered_def = 99;
        def_ptr[weapon->element] = altered_def;
    }
    
    // Run vanilla damage calculation.
    int32_t damage = g_BattleCalculateDamage_trampoline(
        attacker, target, target_part, weapon, unk0, unk1);
        
    // Set Shell Shield max damage to 1 (essentially making its HP hit-based).
    if (damage > 0 && target->current_kind == BattleUnitType::SHELL_SHIELD) {
        damage = 1;
    }
    
    // Increment HP/FP Drain counter if this was intended to be a damaging move.
    if (weapon->damage_function) ++attacker->total_damage_dealt_this_attack;
    
    // Randomize damage dealt, if option enabled.
    const int32_t damage_scale =
        g_Mod->state_.GetOptionNumericValue(OPT_RANDOM_DAMAGE);
    if (damage_scale != 0) {
        // Generate a number from -25 to 25 in increments of 5.
        int32_t scale = (ttyd::system::irand(11) - 5) * 5;
        // Scale by 1x or 2x based on the setting.
        scale *= damage_scale;
        // Round damage modifier away from 0, based on the sign of the scale.
        damage += (damage * scale + (scale > 0 ? 50 : -50)) / 100;
    }
        
    // Change ATK and DEF back, and return calculated damage.
    weapon->damage_function_params[0] = base_atk;
    def_ptr[weapon->element] = base_def;
    return damage;
}

int32_t AlterFpDamageCalculation(
    BattleWorkUnit* attacker, BattleWorkUnit* target,
    BattleWorkUnitPart* target_part, BattleWeapon* weapon,
    uint32_t* unk0, uint32_t unk1) {
    int32_t base_atk = weapon->fp_damage_function_params[0];
    
    int32_t altered_atk = base_atk;
    // Alter FP damage for enemy attacks.
    if (attacker->current_kind <= BattleUnitType::BONETAIL
        && !weapon->item_id && base_atk > 0) {
        GetEnemyStats(
            attacker->current_kind, nullptr, &altered_atk, nullptr,
            nullptr, nullptr, base_atk);
        if (altered_atk < 1) altered_atk = 1;
        if (altered_atk > 99) altered_atk = 99;
        weapon->fp_damage_function_params[0] = altered_atk;
    }
    
    // Run vanilla damage calculation.
    int32_t damage = g_BattleCalculateFpDamage_trampoline(
        attacker, target, target_part, weapon, unk0, unk1);
    
    // Randomize damage dealt, if option enabled.
    const int32_t damage_scale =
        g_Mod->state_.GetOptionNumericValue(OPT_RANDOM_DAMAGE);
    if (damage_scale != 0) {
        // Generate a number from -25 to 25 in increments of 5.
        int32_t scale = (ttyd::system::irand(11) - 5) * 5;
        // Scale by 1x or 2x based on the setting.
        scale *= damage_scale;
        // Round damage modifier away from 0, based on the sign of the scale.
        damage += (damage * scale + (scale > 0 ? 50 : -50)) / 100;
    }
        
    // Change FP damage value back, and return calculated FP loss.
    weapon->fp_damage_function_params[0] = base_atk;
    return damage;
}

// Runs extra code on consuming an item and getting the item to be consumed,
// allowing for enemies to use generic cooking items.
void EnemyConsumeItem(ttyd::evtmgr::EvtEntry* evt) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    int32_t id = evtGetValue(evt, evt->evtArguments[0]);
    id = ttyd::battle_sub::BattleTransID(evt, id);
    auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, id);
    if (unit->current_kind <= BattleUnitType::BONETAIL) {
        g_EnemyItem = unit->held_item;
    }
}
// Returns true if the evt was run by an enemy.
bool GetEnemyConsumeItem(ttyd::evtmgr::EvtEntry* evt) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    BattleWorkUnit* unit = nullptr;
    if (evt->wActorThisPtr) {
        unit = ttyd::battle::BattleGetUnitPtr(
            battleWork,
            reinterpret_cast<uint32_t>(evt->wActorThisPtr));
        if (unit->current_kind <= BattleUnitType::BONETAIL) {
            evtSetValue(evt, evt->evtArguments[0], g_EnemyItem);
            return true;
        }
    }
    return false;
}

void* EnemyUseAdditionalItemsCheck(BattleWorkUnit* unit) {
    switch (unit->held_item) {
        // Items that aren't normally usable but work with no problems:
        case ItemType::COURAGE_MEAL:
        case ItemType::EGG_BOMB:
        case ItemType::COCONUT_BOMB:
        case ItemType::ZESS_DYNAMITE:
        case ItemType::HOT_SAUCE:
        case ItemType::SPITE_POUCH:
        case ItemType::KOOPA_CURSE:
        case ItemType::SHROOM_BROTH:
        case ItemType::LOVE_PUDDING:
        case ItemType::PEACH_TART:
        case ItemType::ELECTRO_POP:
        // Additional items (would not have the desired effect without patches):
        case ItemType::POISON_SHROOM:
        case ItemType::POINT_SWAP:
        case ItemType::TRIAL_STEW:
        case ItemType::TRADE_OFF:
            return ttyd::battle_enemy_item::_check_attack_item(unit);
        case ItemType::FRESH_JUICE:
        case ItemType::HEALTHY_SALAD:
            return ttyd::battle_enemy_item::_check_status_recover_item(unit);
        // Explicitly not allowed:
        case ItemType::FRIGHT_MASK:
        case ItemType::MYSTERY:
        default:
            return nullptr;
    }
}

}
    
void ApplyFixedPatches() {
    g_BtlUnit_Entry_trampoline = patch::hookFunction(
        ttyd::battle_unit::BtlUnit_Entry, [](BattleUnitSetup* unit_setup) {
            AlterUnitKindParams(unit_setup->unit_kind_params);
            return g_BtlUnit_Entry_trampoline(unit_setup);
        });
        
    g_BattleCalculateDamage_trampoline = patch::hookFunction(
        ttyd::battle_damage::BattleCalculateDamage, [](
            BattleWorkUnit* attacker, BattleWorkUnit* target,
            BattleWorkUnitPart* target_part, BattleWeapon* weapon,
            uint32_t* unk0, uint32_t unk1) {
            return AlterDamageCalculation(
                attacker, target, target_part, weapon, unk0, unk1);
        });
        
    g_BattleCalculateFpDamage_trampoline = patch::hookFunction(
        ttyd::battle_damage::BattleCalculateFpDamage, [](
            BattleWorkUnit* attacker, BattleWorkUnit* target,
            BattleWorkUnitPart* target_part, BattleWeapon* weapon,
            uint32_t* unk0, uint32_t unk1) {
            return AlterFpDamageCalculation(
                attacker, target, target_part, weapon, unk0, unk1);
        });
        
    g_btlevtcmd_ConsumeItem_trampoline = patch::hookFunction(
        ttyd::battle_event_cmd::btlevtcmd_ConsumeItem,
        [](EvtEntry* evt, bool isFirstCall) {
            EnemyConsumeItem(evt);
            return g_btlevtcmd_ConsumeItem_trampoline(evt, isFirstCall);
        });
        
    g_btlevtcmd_GetConsumeItem_trampoline = patch::hookFunction(
        ttyd::battle_event_cmd::btlevtcmd_GetConsumeItem,
        [](EvtEntry* evt, bool isFirstCall) {
            if (GetEnemyConsumeItem(evt)) return 2;
            return g_btlevtcmd_GetConsumeItem_trampoline(evt, isFirstCall);
        });
        
    g_BattleEnemyUseItemCheck_trampoline = patch::hookFunction(
        ttyd::battle_enemy_item::BattleEnemyUseItemCheck,
        [](BattleWorkUnit* unit) {
            void* evt_code = g_BattleEnemyUseItemCheck_trampoline(unit);
            if (!evt_code) {
                evt_code = EnemyUseAdditionalItemsCheck(unit);
            }
            return evt_code;
        });
        
    // Disable the check for enemies only holding certain types of items.
    mod::patch::writePatch(
        reinterpret_cast<void*>(g_BtlUnit_EnemyItemCanUseCheck_Patch_SkipCheck),
        0x60000000U /* nop */);
}

}  // namespace enemy
}  // namespace mod::infinite_pit