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

extern "C" {

// BattleCheckCounter
// BattleInitCounterPreCheckWork
// BattleAttackDeclareAll
// __declare
// BattleAttackDeclare
// BattleCheckDamage
// _checkDamageCode_EmergencyRevival
// BattlePreCheckDamage
// BattleSetStatusDamageFromWeapon
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
// BattleCheckPikkyoro
void BattleDamageDirect(
    int32_t unit_idx, battle_unit::BattleWorkUnit* target,
    battle_unit::BattleWorkUnitPart* part, int32_t damage,
    int32_t fp_damage, uint32_t unk0, uint32_t damage_pattern, uint32_t unk1);

}

}