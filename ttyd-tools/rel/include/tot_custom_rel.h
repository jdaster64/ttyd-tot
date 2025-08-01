#pragma once

#include "evt_cmd.h"

#include <gc/types.h>
#include <ttyd/evtmgr.h>

#include <cstdint>

namespace ttyd::battle_database_common {

struct BattleUnitKind;
struct BattleUnitKindPart;
struct BattleWeapon;

}

namespace mod::tot::custom {

extern "C" {

// BattleUnitKind structures for all supported enemies.
// Defined in custom.rel...
extern ttyd::battle_database_common::BattleUnitKind unit_Goomba;
extern ttyd::battle_database_common::BattleUnitKind unit_Paragoomba;
extern ttyd::battle_database_common::BattleUnitKind unit_SpikyGoomba;
extern ttyd::battle_database_common::BattleUnitKind unit_HyperGoomba;
extern ttyd::battle_database_common::BattleUnitKind unit_HyperParagoomba;
extern ttyd::battle_database_common::BattleUnitKind unit_HyperSpikyGoomba;
extern ttyd::battle_database_common::BattleUnitKind unit_Gloomba;
extern ttyd::battle_database_common::BattleUnitKind unit_Paragloomba;
extern ttyd::battle_database_common::BattleUnitKind unit_SpikyGloomba;
extern ttyd::battle_database_common::BattleUnitKind unit_KoopaTroopa;
extern ttyd::battle_database_common::BattleUnitKind unit_Paratroopa;
extern ttyd::battle_database_common::BattleUnitKind unit_KpKoopa;
extern ttyd::battle_database_common::BattleUnitKind unit_KpParatroopa;
extern ttyd::battle_database_common::BattleUnitKind unit_ShadyKoopa;
extern ttyd::battle_database_common::BattleUnitKind unit_ShadyParatroopa;
extern ttyd::battle_database_common::BattleUnitKind unit_DarkKoopa;
extern ttyd::battle_database_common::BattleUnitKind unit_DarkParatroopa;
extern ttyd::battle_database_common::BattleUnitKind unit_Koopatrol;
extern ttyd::battle_database_common::BattleUnitKind unit_DarkKoopatrol;
extern ttyd::battle_database_common::BattleUnitKind unit_DullBones;
extern ttyd::battle_database_common::BattleUnitKind unit_RedBones;
extern ttyd::battle_database_common::BattleUnitKind unit_DryBones;
extern ttyd::battle_database_common::BattleUnitKind unit_DarkBones;
extern ttyd::battle_database_common::BattleUnitKind unit_HammerBro;
extern ttyd::battle_database_common::BattleUnitKind unit_BoomerangBro;
extern ttyd::battle_database_common::BattleUnitKind unit_FireBro;
extern ttyd::battle_database_common::BattleUnitKind unit_Lakitu;
extern ttyd::battle_database_common::BattleUnitKind unit_DarkLakitu;
extern ttyd::battle_database_common::BattleUnitKind unit_Spiny;
extern ttyd::battle_database_common::BattleUnitKind unit_SkyBlueSpiny;
extern ttyd::battle_database_common::BattleUnitKind unit_BuzzyBeetle;
extern ttyd::battle_database_common::BattleUnitKind unit_SpikeTop;
extern ttyd::battle_database_common::BattleUnitKind unit_Parabuzzy;
extern ttyd::battle_database_common::BattleUnitKind unit_SpikyParabuzzy;
extern ttyd::battle_database_common::BattleUnitKind unit_Magikoopa;
extern ttyd::battle_database_common::BattleUnitKind unit_RedMagikoopa;
extern ttyd::battle_database_common::BattleUnitKind unit_WhiteMagikoopa;
extern ttyd::battle_database_common::BattleUnitKind unit_GreenMagikoopa;
extern ttyd::battle_database_common::BattleUnitKind unit_Bandit;
extern ttyd::battle_database_common::BattleUnitKind unit_BigBandit;
extern ttyd::battle_database_common::BattleUnitKind unit_BadgeBandit;
extern ttyd::battle_database_common::BattleUnitKind unit_Spinia;
extern ttyd::battle_database_common::BattleUnitKind unit_Spania;
extern ttyd::battle_database_common::BattleUnitKind unit_Spunia;
extern ttyd::battle_database_common::BattleUnitKind unit_Fuzzy;
extern ttyd::battle_database_common::BattleUnitKind unit_GreenFuzzy;
extern ttyd::battle_database_common::BattleUnitKind unit_FlowerFuzzy;
extern ttyd::battle_database_common::BattleUnitKind unit_Pokey;
extern ttyd::battle_database_common::BattleUnitKind unit_PoisonPokey;
extern ttyd::battle_database_common::BattleUnitKind unit_CrazeeDayzee;
extern ttyd::battle_database_common::BattleUnitKind unit_Pider;
extern ttyd::battle_database_common::BattleUnitKind unit_Arantula;
extern ttyd::battle_database_common::BattleUnitKind unit_Swooper;
extern ttyd::battle_database_common::BattleUnitKind unit_Swoopula;
extern ttyd::battle_database_common::BattleUnitKind unit_Swampire;
extern ttyd::battle_database_common::BattleUnitKind unit_DarkPuff;
extern ttyd::battle_database_common::BattleUnitKind unit_RuffPuff;
extern ttyd::battle_database_common::BattleUnitKind unit_IcePuff;
extern ttyd::battle_database_common::BattleUnitKind unit_PoisonPuff;
extern ttyd::battle_database_common::BattleUnitKind unit_Boo;
extern ttyd::battle_database_common::BattleUnitKind unit_DarkBoo;
extern ttyd::battle_database_common::BattleUnitKind unit_LavaBubble;
extern ttyd::battle_database_common::BattleUnitKind unit_Ember;
extern ttyd::battle_database_common::BattleUnitKind unit_PhantomEmber;
extern ttyd::battle_database_common::BattleUnitKind unit_Bristle;
extern ttyd::battle_database_common::BattleUnitKind unit_DarkBristle;
extern ttyd::battle_database_common::BattleUnitKind unit_BulkyBobOmb;
extern ttyd::battle_database_common::BattleUnitKind unit_BobUlk;
extern ttyd::battle_database_common::BattleUnitKind unit_ChainChomp;
extern ttyd::battle_database_common::BattleUnitKind unit_RedChomp;
extern ttyd::battle_database_common::BattleUnitKind unit_Wizzerd;
extern ttyd::battle_database_common::BattleUnitKind unit_DarkWizzerd;
extern ttyd::battle_database_common::BattleUnitKind unit_EliteWizzerd;
extern ttyd::battle_database_common::BattleUnitKind unit_XNaut;
extern ttyd::battle_database_common::BattleUnitKind unit_XNautPhD;
extern ttyd::battle_database_common::BattleUnitKind unit_EliteXNaut;
extern ttyd::battle_database_common::BattleUnitKind unit_Yux;
extern ttyd::battle_database_common::BattleUnitKind unit_MiniYux;
extern ttyd::battle_database_common::BattleUnitKind unit_ZYux;
extern ttyd::battle_database_common::BattleUnitKind unit_MiniZYux;
extern ttyd::battle_database_common::BattleUnitKind unit_XYux;
extern ttyd::battle_database_common::BattleUnitKind unit_MiniXYux;
extern ttyd::battle_database_common::BattleUnitKind unit_AmazyDayzee;
// Defined in codebase...
extern ttyd::battle_database_common::BattleUnitKind unit_BobOmb;
extern ttyd::battle_database_common::BattleUnitKind unit_HyperBobOmb;
extern ttyd::battle_database_common::BattleUnitKind unit_Craw;
extern ttyd::battle_database_common::BattleUnitKind unit_DarkCraw;
extern ttyd::battle_database_common::BattleUnitKind unit_Cleft;
extern ttyd::battle_database_common::BattleUnitKind unit_HyperCleft;
extern ttyd::battle_database_common::BattleUnitKind unit_MoonCleft;
extern ttyd::battle_database_common::BattleUnitKind unit_IronCleft;
extern ttyd::battle_database_common::BattleUnitKind unit_PalePiranha;
extern ttyd::battle_database_common::BattleUnitKind unit_PutridPiranha;
extern ttyd::battle_database_common::BattleUnitKind unit_FrostPiranha;
extern ttyd::battle_database_common::BattleUnitKind unit_PiranhaPlant;
extern ttyd::battle_database_common::BattleUnitKind unit_AtomicBoo;
extern ttyd::battle_database_common::BattleUnitKind unit_CosmicBoo;
extern ttyd::battle_database_common::BattleUnitKind unit_Hooktail;
extern ttyd::battle_database_common::BattleUnitKind unit_Gloomtail;
extern ttyd::battle_database_common::BattleUnitKind unit_Bonetail;
extern ttyd::battle_database_common::BattleUnitKind unit_GoldFuzzy;
extern ttyd::battle_database_common::BattleUnitKind unit_FuzzyHorde;
extern ttyd::battle_database_common::BattleUnitKind unit_Bowser;
extern ttyd::battle_database_common::BattleUnitKind unit_Kammy;
extern ttyd::battle_database_common::BattleUnitKind unit_Doopliss;

// Other functions and structures referenced by patches, etc.
extern ttyd::battle_database_common::BattleWeapon unitBobOmb_weaponBomb;
extern ttyd::battle_database_common::BattleWeapon unitDragon_weaponBite;
extern ttyd::battle_database_common::BattleUnitKindPart part_Yux_Main;
extern ttyd::battle_database_common::BattleUnitKindPart part_Yux_Main;
extern ttyd::battle_database_common::BattleUnitKindPart part_ZYux_Main;
extern ttyd::battle_database_common::BattleUnitKindPart part_XYux_Main;
extern int8_t defense_GreenMagikoopa[5];

// Exposed for ease of integration into party scripts.
extern ttyd::battle_database_common::BattleWeapon* unitDoopliss_weaponSelected;

// Hooked / patched for Infatuate reasons.
EVT_DECLARE_USER_FUNC(unitBoo_teresa_check_trans, 1);
EVT_DECLARE_USER_FUNC(unitDarkBoo_teresa_check_trans, 1);

extern ttyd::battle_database_common::BattleWeapon unitHyperGoomba_weaponCharge;
extern ttyd::battle_database_common::BattleWeapon unitHyperParagoomba_weaponChargePara;
extern ttyd::battle_database_common::BattleWeapon unitHyperParagoomba_weaponCharge;
extern ttyd::battle_database_common::BattleWeapon unitHyperSpikyGoomba_weaponCharge;
extern ttyd::battle_database_common::BattleWeapon unitKoopatrol_weaponCharge;
extern ttyd::battle_database_common::BattleWeapon unitDarkKoopatrol_weaponCharge;
extern ttyd::battle_database_common::BattleWeapon unitDarkLakitu_weaponCharge;
extern ttyd::battle_database_common::BattleWeapon unitSkyBlueSpiny_weaponCharge;
extern ttyd::battle_database_common::BattleWeapon unitBoo_weaponInvis;
extern ttyd::battle_database_common::BattleWeapon unitDarkBoo_weaponInvis;
extern ttyd::battle_database_common::BattleWeapon unitWizzerd_weaponHuge;
extern ttyd::battle_database_common::BattleWeapon unitWizzerd_weaponDef;
extern ttyd::battle_database_common::BattleWeapon unitDarkWizzerd_weaponInvis;
extern ttyd::battle_database_common::BattleWeapon unitDarkWizzerd_weaponDodgy;
extern ttyd::battle_database_common::BattleWeapon unitDarkWizzerd_weaponHuge;
extern ttyd::battle_database_common::BattleWeapon unitDarkWizzerd_weaponDef;
extern ttyd::battle_database_common::BattleWeapon unitEliteWizzerd_weaponInvis;
extern ttyd::battle_database_common::BattleWeapon unitEliteWizzerd_weaponDodgy;
extern ttyd::battle_database_common::BattleWeapon unitEliteWizzerd_weaponHuge;
extern ttyd::battle_database_common::BattleWeapon unitEliteWizzerd_weaponDef;
extern ttyd::battle_database_common::BattleWeapon unitMagikoopa_weaponHuge;
extern ttyd::battle_database_common::BattleWeapon unitMagikoopa_weaponElectric;
extern ttyd::battle_database_common::BattleWeapon unitMagikoopa_weaponInvis;
extern ttyd::battle_database_common::BattleWeapon unitMagikoopa_weaponDef;
extern ttyd::battle_database_common::BattleWeapon unitGreenMagikoopa_weaponHuge;
extern ttyd::battle_database_common::BattleWeapon unitGreenMagikoopa_weaponElectric;
extern ttyd::battle_database_common::BattleWeapon unitGreenMagikoopa_weaponInvis;
extern ttyd::battle_database_common::BattleWeapon unitGreenMagikoopa_weaponDef;
extern ttyd::battle_database_common::BattleWeapon unitRedMagikoopa_weaponHuge;
extern ttyd::battle_database_common::BattleWeapon unitRedMagikoopa_weaponElectric;
extern ttyd::battle_database_common::BattleWeapon unitRedMagikoopa_weaponInvis;
extern ttyd::battle_database_common::BattleWeapon unitRedMagikoopa_weaponDef;
extern ttyd::battle_database_common::BattleWeapon unitWhiteMagikoopa_weaponHuge;
extern ttyd::battle_database_common::BattleWeapon unitWhiteMagikoopa_weaponElectric;
extern ttyd::battle_database_common::BattleWeapon unitWhiteMagikoopa_weaponInvis;
extern ttyd::battle_database_common::BattleWeapon unitWhiteMagikoopa_weaponDef;
extern ttyd::battle_database_common::BattleWeapon unitXNaut_weaponPotion;
extern ttyd::battle_database_common::BattleWeapon unitEliteXNaut_weaponPotion;
extern ttyd::battle_database_common::BattleWeapon unitXNautPhD_weaponPotion;
extern ttyd::battle_database_common::BattleWeapon unitBoomerangBros_weaponNormal;
extern ttyd::battle_database_common::BattleWeapon unitBoomerangBros_weaponRenzoku;

// Individual evt instructions referenced by patches, etc.
extern int32_t evt_Koopatrol_NormalAttackReturnLbl_PatchLoc[1];
extern int32_t evt_DarkKoopatrol_NormalAttackReturnLbl_PatchLoc[1];
extern int32_t evt_HammerBros_CheckHp_PatchLoc[1];
extern int32_t evt_BoomerangBros_CheckHp_PatchLoc[1];
extern int32_t evt_BoomerangBros_CheckEnemyN_PatchLoc[1];
extern int32_t evt_BoomerangBros_CheckEnemyR_PatchLoc[1];
extern int32_t evt_FireBros_CheckHp_PatchLoc[1];
extern int32_t evt_Magikoopa_CheckNumEnemies_PatchLoc[1];
extern int32_t evt_Magikoopa_GaleForceDeath_PatchLoc[1];
extern int32_t evt_RedMagikoopa_CheckNumEnemies_PatchLoc[1];
extern int32_t evt_RedMagikoopa_GaleForceDeath_PatchLoc[1];
extern int32_t evt_WhiteMagikoopa_CheckNumEnemies_PatchLoc[1];
extern int32_t evt_WhiteMagikoopa_GaleForceDeath_PatchLoc[1];
extern int32_t evt_GreenMagikoopa_CheckNumEnemies_PatchLoc[1];
extern int32_t evt_GreenMagikoopa_GaleForceDeath_PatchLoc[1];
extern int32_t evt_Pider_GaleForceDeath_PatchLoc[1];
extern int32_t evt_Arantula_GaleForceDeath_PatchLoc[1];
extern int32_t evt_Bandit_CheckConfusion_PatchLoc[1];
extern int32_t evt_BigBandit_CheckConfusion_PatchLoc[1];
extern int32_t evt_BadgeBandit_CheckConfusion_PatchLoc[1];
extern int32_t evt_DarkWizzerd_CheckNumEnemies_PatchLoc[1];
extern int32_t evt_DarkWizzerd_GaleForceDeath_PatchLoc[1];
extern int32_t evt_EliteWizzerd_CheckNumEnemies_PatchLoc[1];
extern int32_t evt_EliteWizzerd_GaleForceDeath_PatchLoc[1];
extern int32_t evt_XNaut_NormalAttackReturnLbl_PatchLoc[1];
extern int32_t evt_XNaut_JumpAttackReturnLbl_PatchLoc[1];
extern int32_t evt_EliteXNaut_NormalAttackReturnLbl_PatchLoc[1];
extern int32_t evt_EliteXNaut_JumpAttackReturnLbl_PatchLoc[1];

// Evt support for opening cutscene (from original hei_00 scripts).
EVT_DECLARE_USER_FUNC(evt_hei_00_cam_test, 0)
extern int32_t evt_hei_00_gonbaba_shadow_evt[1];
// Data to make trees interactable.
extern int8_t hei_00_ki_data[96];

}

}