#pragma once

#include <cstdint>

namespace ttyd::battle_database_common {
struct BattleWeapon;
}

namespace ttyd::battle_unit {
struct BattleWorkUnit;
struct BattleWorkUnitPart;
}

namespace ttyd::battle_weapon_power {

extern "C" {

uint32_t weaponGetPowerTatsumaki(
    battle_unit::BattleWorkUnit* unit1,
    battle_database_common::BattleWeapon* weapon,
    battle_unit::BattleWorkUnit* unit2, battle_unit::BattleWorkUnitPart* part);
uint32_t weaponGetFPPowerFPHalf(
    battle_unit::BattleWorkUnit* unit1,
    battle_database_common::BattleWeapon* weapon,
    battle_unit::BattleWorkUnit* unit2, battle_unit::BattleWorkUnitPart* part);
uint32_t weaponGetPowerHPHalf2(
    battle_unit::BattleWorkUnit* unit1,
    battle_database_common::BattleWeapon* weapon,
    battle_unit::BattleWorkUnit* unit2, battle_unit::BattleWorkUnitPart* part);
uint32_t weaponGetPowerHPHalf(
    battle_unit::BattleWorkUnit* unit1,
    battle_database_common::BattleWeapon* weapon,
    battle_unit::BattleWorkUnit* unit2, battle_unit::BattleWorkUnitPart* part);
uint32_t weaponGetACOutputParam(
    battle_unit::BattleWorkUnit* unit1,
    battle_database_common::BattleWeapon* weapon,
    battle_unit::BattleWorkUnit* unit2, battle_unit::BattleWorkUnitPart* part);
uint32_t weaponGetPowerFromGulliblePartyAttackLv(
    battle_unit::BattleWorkUnit* unit1,
    battle_database_common::BattleWeapon* weapon,
    battle_unit::BattleWorkUnit* unit2, battle_unit::BattleWorkUnitPart* part);
uint32_t weaponGetPowerFromPartyAttackLv(
    battle_unit::BattleWorkUnit* unit1,
    battle_database_common::BattleWeapon* weapon,
    battle_unit::BattleWorkUnit* unit2, battle_unit::BattleWorkUnitPart* part);
uint32_t weaponGetPowerOverlapHammer1(
    battle_unit::BattleWorkUnit* unit1,
    battle_database_common::BattleWeapon* weapon,
    battle_unit::BattleWorkUnit* unit2, battle_unit::BattleWorkUnitPart* part);
uint32_t weaponGetPowerOverlapJump1(
    battle_unit::BattleWorkUnit* unit1,
    battle_database_common::BattleWeapon* weapon,
    battle_unit::BattleWorkUnit* unit2, battle_unit::BattleWorkUnitPart* part);
uint32_t weaponGetPowerFromMarioHammerLv(
    battle_unit::BattleWorkUnit* unit1,
    battle_database_common::BattleWeapon* weapon,
    battle_unit::BattleWorkUnit* unit2, battle_unit::BattleWorkUnitPart* part);
uint32_t weaponGetPowerFromMarioJumpLv(
    battle_unit::BattleWorkUnit* unit1,
    battle_database_common::BattleWeapon* weapon,
    battle_unit::BattleWorkUnit* unit2, battle_unit::BattleWorkUnitPart* part);
uint32_t weaponGetFPPowerDefault(
    battle_unit::BattleWorkUnit* unit1,
    battle_database_common::BattleWeapon* weapon,
    battle_unit::BattleWorkUnit* unit2, battle_unit::BattleWorkUnitPart* part);
uint32_t weaponGetPowerDefault(
    battle_unit::BattleWorkUnit* unit1,
    battle_database_common::BattleWeapon* weapon,
    battle_unit::BattleWorkUnit* unit2, battle_unit::BattleWorkUnitPart* part);

}

}