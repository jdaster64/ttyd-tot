#pragma once

#include <cstdint>

namespace ttyd::battle_database_common {
struct BattleWeapon;
}
namespace ttyd::battle_unit {
struct BattleWorkUnit;
struct BattleWorkUnitPart;
}

namespace ttyd::battle_damage {

struct CounterattackWork {
    int32_t allow_attacker_damage;  // Whether attack will still damage target.
    int32_t payback_or_poison_countered;
    int32_t fiery_status_countered;
    int32_t poison_status_countered;
    int32_t electric_status_countered;
    int32_t icy_status_countered;
    int32_t hold_fast_countered;
    int32_t return_postage_countered;
    int32_t counter_type_1;
    int32_t counter_type_2;         // Same as counter_type_1 + 0x100 flag?
    int32_t total_damage;
    int32_t target_hit_event;       // Run on target after successful counter?
}  ;

static_assert(sizeof(CounterattackWork) == 0x30);

extern "C" {

// BattleCheckCounter
// BattleInitCounterPreCheckWork
// BattleAttackDeclareAll
// __declare
// BattleAttackDeclare
// BattleCheckDamage
// _checkDamageCode_EmergencyRevival
int32_t BattlePreCheckDamage(
    battle_unit::BattleWorkUnit* attacker, battle_unit::BattleWorkUnit* target,
    battle_unit::BattleWorkUnitPart* part,
    battle_database_common::BattleWeapon* weapon, uint32_t extra_params);
uint32_t BattleSetStatusDamageFromWeapon(
    battle_unit::BattleWorkUnit* attacker, battle_unit::BattleWorkUnit* target, 
    battle_unit::BattleWorkUnitPart* part,
    battle_database_common::BattleWeapon* weapon, uint32_t extra_params);
int32_t BattleSetStatusDamage(
    uint32_t* out_unk0, battle_unit::BattleWorkUnit* unit,
    battle_unit::BattleWorkUnitPart* part, uint32_t special_property_flags,
    int32_t status_type, int32_t rate, int32_t gale_factor, int32_t turns,
    int32_t strength);
// _getRegistStatus
void _getSickStatusParam(
    battle_unit::BattleWorkUnit* unit,
    battle_database_common::BattleWeapon* weapon,
    int32_t status_type, int8_t* out_turns, int8_t* out_strength);
// _getSickStatusRate
int32_t BattleCalculateFpDamage(
    battle_unit::BattleWorkUnit* attacker, battle_unit::BattleWorkUnit* target,
    battle_unit::BattleWorkUnitPart* target_part,
    battle_database_common::BattleWeapon* weapon, uint32_t* unk0, uint32_t unk1);
int32_t BattleCalculateDamage(
    battle_unit::BattleWorkUnit* attacker, battle_unit::BattleWorkUnit* target,
    battle_unit::BattleWorkUnitPart* target_part,
    battle_database_common::BattleWeapon* weapon, uint32_t* unk0, uint32_t unk1);
void BattleCheckPikkyoro(
    battle_database_common::BattleWeapon* weapon, uint32_t* unk_flags);
void BattleDamageDirect(
    int32_t unit_idx, battle_unit::BattleWorkUnit* target,
    battle_unit::BattleWorkUnitPart* part, int32_t damage,
    int32_t fp_damage, uint32_t unk0, uint32_t damage_pattern, uint32_t unk1);

}

}