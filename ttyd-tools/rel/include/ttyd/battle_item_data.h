#pragma once

#include "battle_database_common.h"

#include <cstdint>

namespace ttyd::battle_item_data {

extern "C" {

// Event code for various items / item types.
extern int32_t _return_home_event[1];
extern int32_t ItemEvent_Automatic_Target[1];
extern int32_t ItemEvent_Recovery_Core_Effect[1];
extern int32_t ItemEvent_Recovery_Core[1];
extern int32_t ItemEvent_Recovery[1];
extern int32_t ItemEvent_Support_Sub_UseDeclere[1];
extern int32_t ItemEvent_GetTarget[1];
extern int32_t ItemEvent_Support_Sub_Effect[1];
extern int32_t ItemEvent_Support_NoEffect[1];
extern int32_t ItemEvent_Support_RefreshEffect[1];
extern int32_t ItemEvent_Pow_Block[1];
extern int32_t ItemEvent_Fire_Flower_Core[1];
extern int32_t ItemEvent_Fire_Flower[1];
extern int32_t ItemEvent_Kaminari_Core[1];
extern int32_t ItemEvent_Kaminari[1];
extern int32_t ItemEvent_Kaminari_Dokkan[1];
extern int32_t ItemEvent_Kaminari_Gorogoro[1];
extern int32_t ItemEvent_Kirakira_Otoshi_Core[1];
extern int32_t ItemEvent_Kirakira_Otoshi[1];
extern int32_t ItemEvent_Koorino_Ibuki[1];
extern int32_t ItemEvent_Yurayura_Jishin[1];
extern int32_t ItemEvent_Teresano_Fuku[1];
extern int32_t ItemEvent_Biribiri_Kinoko_Core[1];
extern int32_t ItemEvent_Biribiri_Kinoko[1];
extern int32_t ItemEvent_Teki_Yokeeru_Core[1];
extern int32_t ItemEvent_Teki_Yokeeru[1];
extern int32_t ItemEvent_Madowaseno_Kona[1];
extern int32_t ItemEvent_Nemure_Yoikoyo[1];
extern int32_t ItemEvent_Stop_Watch_Core[1];
extern int32_t ItemEvent_Stop_Watch[1];
extern int32_t ItemEvent_Guruguru_Memawashi[1];
extern int32_t ItemEvent_Dekadeka_Drink[1];
extern int32_t ItemEvent_Katikati_Koura[1];
extern int32_t ItemEvent_Suitooru[1];
extern int32_t ItemEvent_Teki_Kyouka[1];
extern int32_t ItemEvent_Minimini_Kun[1];
extern int32_t ItemEvent_Funyafunya_Kun[1];
extern int32_t ItemEvent_Sukkiri_Drink[1];
extern int32_t ItemEvent_Jiwajiwa_Kinoko_Core[1];
extern int32_t ItemEvent_Jiwajiwa_Kinoko[1];
extern int32_t ItemEvent_Jiwajiwa_Syrup_Core[1];
extern int32_t ItemEvent_Jiwajiwa_Syrup[1];
extern int32_t ItemEvent_Irekaeeru[1];
extern int32_t ItemEvent_Attiike_Shisshi[1];
extern int32_t ItemEvent_Naniga_Okorukana[1];
extern int32_t ItemEvent_Shikaeshino_Kona[1];
extern int32_t ItemEvent_Kameno_Noroi[1];
extern int32_t ItemEvent_RecoveryAndStatus[1];
extern int32_t ItemEvent_ThrowAttack[1];
extern int32_t ItemEvent_Poison_Kinoko[1];
extern int32_t ItemEvent_LastDinner[1];

// Possible items for a Mystery.
extern int32_t mystery_item_table[14];

// Weapon parameters for various items or item types.
extern battle_database_common::BattleWeapon ItemWeaponData_CookingItem;
extern battle_database_common::BattleWeapon ItemWeaponData_Kinoko;
extern battle_database_common::BattleWeapon ItemWeaponData_Kinkyuu_Kinoko;
extern battle_database_common::BattleWeapon ItemWeaponData_Super_Kinoko;
extern battle_database_common::BattleWeapon ItemWeaponData_Ultra_Kinoko;
extern battle_database_common::BattleWeapon ItemWeaponData_Sinabita_Kinoko;
extern battle_database_common::BattleWeapon ItemWeaponData_Honey_Syrup;
extern battle_database_common::BattleWeapon ItemWeaponData_Maple_Syrup;
extern battle_database_common::BattleWeapon ItemWeaponData_Royal_Jelly;
extern battle_database_common::BattleWeapon ItemWeaponData_Tankobu;
extern battle_database_common::BattleWeapon ItemWeaponData_Pow_Block;
extern battle_database_common::BattleWeapon ItemWeaponData_Fire_Flower;
extern battle_database_common::BattleWeapon ItemWeaponData_Kaminari_Dokkan;
extern battle_database_common::BattleWeapon ItemWeaponData_Kaminari_Gorogoro;
extern battle_database_common::BattleWeapon ItemWeaponData_Kirakira_Otoshi;
extern battle_database_common::BattleWeapon ItemWeaponData_Koorino_Ibuki;
extern battle_database_common::BattleWeapon ItemWeaponData_Yurayura_Jishin;
extern battle_database_common::BattleWeapon ItemWeaponData_Teresano_Fuku;
extern battle_database_common::BattleWeapon ItemWeaponData_Biribiri_Kinoko;
extern battle_database_common::BattleWeapon ItemWeaponData_Teki_Yokeeru;
extern battle_database_common::BattleWeapon ItemWeaponData_Madowaseno_Kona;
extern battle_database_common::BattleWeapon ItemWeaponData_Nemure_Yoikoyo;
extern battle_database_common::BattleWeapon ItemWeaponData_Stop_Watch;
extern battle_database_common::BattleWeapon ItemWeaponData_Guruguru_Memawashi;
extern battle_database_common::BattleWeapon ItemWeaponData_Dekadeka_Drink;
extern battle_database_common::BattleWeapon ItemWeaponData_Katikati_Koura;
extern battle_database_common::BattleWeapon ItemWeaponData_Suitooru;
extern battle_database_common::BattleWeapon ItemWeaponData_Teki_Kyouka;
extern battle_database_common::BattleWeapon ItemWeaponData_Minimini_Kun;
extern battle_database_common::BattleWeapon ItemWeaponData_Funyafunya_Kun;
extern battle_database_common::BattleWeapon ItemWeaponData_Sukkiri_Drink;
extern battle_database_common::BattleWeapon ItemWeaponData_Jiwajiwa_Kinoko;
extern battle_database_common::BattleWeapon ItemWeaponData_Jiwajiwa_Syrup;
extern battle_database_common::BattleWeapon ItemWeaponData_Irekaeeru;
extern battle_database_common::BattleWeapon ItemWeaponData_Attiike_Shisshi;
extern battle_database_common::BattleWeapon ItemWeaponData_Naniga_Okorukana;
extern battle_database_common::BattleWeapon ItemWeaponData_Shikaeshino_Kona;
extern battle_database_common::BattleWeapon ItemWeaponData_Kameno_Noroi;
extern battle_database_common::BattleWeapon ItemWeaponData_SpaceFood;
extern battle_database_common::BattleWeapon ItemWeaponData_IceCandy;
extern battle_database_common::BattleWeapon ItemWeaponData_NancyFrappe;
extern battle_database_common::BattleWeapon ItemWeaponData_SnowRabbit;
extern battle_database_common::BattleWeapon ItemWeaponData_TeaKinoko;
extern battle_database_common::BattleWeapon ItemWeaponData_FirstLovePuddingBiribiri;
extern battle_database_common::BattleWeapon ItemWeaponData_FirstLovePuddingTrans;
extern battle_database_common::BattleWeapon ItemWeaponData_FirstLovePuddingSleep;
extern battle_database_common::BattleWeapon ItemWeaponData_StarryDinner;
extern battle_database_common::BattleWeapon ItemWeaponData_KararinaPasta;
extern battle_database_common::BattleWeapon ItemWeaponData_MeromeroCake;
extern battle_database_common::BattleWeapon ItemWeaponData_PeachTaltBiribiri;
extern battle_database_common::BattleWeapon ItemWeaponData_PeachTaltDodge;
extern battle_database_common::BattleWeapon ItemWeaponData_PeachTaltSleep;
extern battle_database_common::BattleWeapon ItemWeaponData_BiribiriCandy;
extern battle_database_common::BattleWeapon ItemWeaponData_HealthySalad;
extern battle_database_common::BattleWeapon ItemWeaponData_FreshJuice;
extern battle_database_common::BattleWeapon ItemWeaponData_RedKararing;
extern battle_database_common::BattleWeapon ItemWeaponData_FutarideForever;
extern battle_database_common::BattleWeapon ItemWeaponData_NancyDynamite;
extern battle_database_common::BattleWeapon ItemWeaponData_CoconutsBomb;
extern battle_database_common::BattleWeapon ItemWeaponData_KachikachiDish;
extern battle_database_common::BattleWeapon ItemWeaponData_BomberEgg;
extern battle_database_common::BattleWeapon ItemWeaponData_PoisonKinoko;
extern battle_database_common::BattleWeapon ItemWeaponData_LastDinner;

// Unused; don't use unless you provide a definition.
// extern battle_database_common::BattleWeapon ItemWeaponData_FireCandy;

}

}