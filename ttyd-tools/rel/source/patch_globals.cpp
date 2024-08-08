#include "common_types.h"

#include <gc/OSLink.h>
#include <ttyd/battle.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_unit.h>
#include <ttyd/dispdrv.h>
#include <ttyd/evtmgr.h>
#include <ttyd/npcdrv.h>
#include <ttyd/seqdrv.h>
#include <ttyd/win_root.h>
#include <ttyd/winmgr.h>

#include <cstdint>

namespace mod::infinite_pit {
    
namespace {

using ::gc::OSLink::OSModuleInfo;
using ::ttyd::battle_database_common::BattleUnitSetup;
using ::ttyd::battle_database_common::BattleWeapon;
using ::ttyd::battle::BattleWork;
using ::ttyd::battle::BattleWorkCommand;
using ::ttyd::battle::SpBonusInfo;
using ::ttyd::battle_unit::BattleWorkUnit;
using ::ttyd::battle_unit::BattleWorkUnitPart;
using ::ttyd::dispdrv::CameraId;
using ::ttyd::evtmgr::EvtEntry;
using ::ttyd::npcdrv::FbatBattleInformation;
using ::ttyd::seqdrv::SeqIndex;
using ::ttyd::win_root::WinPauseMenu;
using ::ttyd::winmgr::WinMgrEntry;
using ::ttyd::winmgr::WinMgrSelectEntry;

}

// Function hooks.

// seqdrv.o  8002e218
void (*g_seqSetSeq_trampoline)(SeqIndex, const char*, const char*) = nullptr;
// npcdrv.o  80046a78
void (*g_fbatBattleMode_trampoline)(void) = nullptr;
// seq_battle.o  80073f20
void (*g_seq_battleInit_trampoline)(void) = nullptr;
// msgdrv.o  80081b8c
const char* (*g_msgSearch_trampoline)(const char*) = nullptr;
// itemdrv.o  800ad7f0
void* (*g_itemEntry_trampoline)(
    const char*, int32_t, float, float, float, uint32_t, int32_t, void*) = nullptr;
// cardmgr.o  800b2388
void (*g_cardCopy2Main_trampoline)(int32_t) = nullptr;
// mario_pouch.o  800d35a8
void (*g_pouchReviseMarioParam_trampoline)() = nullptr;
// mario_pouch.o 800d3e18
int32_t (*g_pouchAddCoin_trampoline)(int16_t) = nullptr;
// mario_pouch.o 800d50cc
uint32_t (*g_pouchGetItem_trampoline)(int32_t) = nullptr;
// pmario_sound.o  800daf24
uint32_t (*g_psndBGMOn_f_d_trampoline)(
    uint32_t, const char*, uint32_t, uint16_t) = nullptr;
// event.o  800ee688
void (*g_stg0_00_init_trampoline)(void) = nullptr;
// battle.o  800f7bfc
void (*g_BattleStoreExp_trampoline)(BattleWork*, int32_t) = nullptr;
// battle.o  800f7ca4
void (*g__EquipItem_trampoline)(BattleWorkUnit*, uint32_t, int32_t) = nullptr;
// battle.o  800f8aac
void (*g_Btl_UnitSetup_trampoline)(BattleWork*) = nullptr;
// battle_ac.o  800fa12c
void (*g_BattleActionCommandSetDifficulty_trampoline)(
    BattleWork*, BattleWorkUnit*, int32_t) = nullptr;
// battle_ac.o  800fa1b8
int32_t (*g_BattleActionCommandCheckDefence_trampoline)(
    BattleWorkUnit*, BattleWeapon*) = nullptr;
// battle_damage.o  800fbadc
int32_t (*g_BattlePreCheckDamage_trampoline)(
    BattleWorkUnit*, BattleWorkUnit*, BattleWorkUnitPart*,
    BattleWeapon*, uint32_t) = nullptr;
// battle_damage.o  800fbd8c
uint32_t (*g_BattleSetStatusDamageFromWeapon_trampoline)(
    BattleWorkUnit*, BattleWorkUnit*, BattleWorkUnitPart*,
    BattleWeapon*, uint32_t) = nullptr;
// battle_damage.o  800fd680
int32_t (*g_BattleCalculateFpDamage_trampoline)(
    BattleWorkUnit*, BattleWorkUnit*, BattleWorkUnitPart*, BattleWeapon*,
    uint32_t*, uint32_t) = nullptr;
// battle_damage.o  800fd790
int32_t (*g_BattleCalculateDamage_trampoline)(
    BattleWorkUnit*, BattleWorkUnit*, BattleWorkUnitPart*, BattleWeapon*,
    uint32_t*, uint32_t) = nullptr;
// battle_damage.o  800fde2c
void (*g_BattleCheckPikkyoro_trampoline)(BattleWeapon*, uint32_t*) = nullptr;
// battle_damage.o  800fdfd4
void (*g_BattleDamageDirect_trampoline)(
    int32_t, BattleWorkUnit*, BattleWorkUnitPart*, int32_t, int32_t,
    uint32_t, uint32_t, uint32_t) = nullptr;
// battle_event_cmd.o  801054e8
int32_t (*g_btlevtcmd_WeaponAftereffect_trampoline)(EvtEntry*, bool) = nullptr;
// battle_event_cmd.o  801056f4
int32_t (*g_btlevtcmd_GetItemRecoverParam_trampoline)(EvtEntry*, bool) = nullptr;
// battle_event_cmd.o  8010a388
int32_t (*g_btlevtcmd_SetEventAttack_trampoline)(EvtEntry*, bool) = nullptr;
// battle_event_cmd.o  8010af58
int32_t (*g_btlevtcmd_ConsumeItem_trampoline)(EvtEntry*, bool) = nullptr;
// battle_event_cmd.o  8010b3d4
int32_t (*g_btlevtcmd_GetConsumeItem_trampoline)(EvtEntry*, bool) = nullptr;
// battle_event_cmd.o  8010b540
int32_t (*g_btlevtcmd_GetSelectEnemy_trampoline)(EvtEntry*, bool) = nullptr;
// battle_event_cmd.o  8010ef44
int32_t (*g_btlevtcmd_CheckSpace_trampoline)(EvtEntry*, bool) = nullptr;
// battle_event_cmd.o  80113140
int32_t (*g_btlevtcmd_ChangeParty_trampoline)(EvtEntry*, bool) = nullptr;
// battle_event_default.o  801138d8
int32_t (*g__get_flower_suitoru_point_trampoline)(EvtEntry*, bool) = nullptr;
// battle_event_default.o  80113950
int32_t (*g__get_heart_suitoru_point_trampoline)(EvtEntry*, bool) = nullptr;
// battle_information.o  801141bc
void (*g_BattleInformationSetDropMaterial_trampoline)(
    FbatBattleInformation*) = nullptr;
// battle_menu_disp.o 80115b30
void (*g_DrawOperationWin_trampoline)() = nullptr;
// battle_menu_disp.o 80115dac
void (*g_DrawWeaponWin_trampoline)() = nullptr;
// battle_seq.o  8011aa34
uint32_t (*g_BattleCheckConcluded_trampoline)(BattleWork*) = nullptr;
// battle_seq.o  8011c350
void (*g__rule_disp_trampoline)(void) = nullptr;
// battle_seq.o  8011edb8
BattleWeapon* (*g__GetFirstAttackWeapon_trampoline)(int32_t) = nullptr;
// battle_seq_command.o  8011f488
int32_t (*g_BattleSeqCmd_get_msg_trampoline)(EvtEntry*, bool) = nullptr;
// battle_seq_command.o  8011f62c
void* (*g_BattleSetConfuseAct_trampoline)(BattleWork*, BattleWorkUnit*) = nullptr;
// battle_seq_command.o  80120268
void (*g__btlcmd_SetAttackEvent_trampoline)(BattleWorkUnit*, BattleWorkCommand*) = nullptr;
// battle_seq_command.o  80123db0
void (*g__btlcmd_UpdateSelectWeaponTable_trampoline)(BattleWork*, int32_t) = nullptr;
// battle_seq_command.o  80123ec0
int32_t (*g__btlcmd_MakeSelectWeaponTable_trampoline)(BattleWork*, int32_t) = nullptr;
// battle_seq_command.o  80125380
void (*g_BattleCommandInit_trampoline)(BattleWork*) = nullptr;
// battle_unit.o  80126840
void (*g_BtlUnit_PayWeaponCost_trampoline)(BattleWorkUnit*, BattleWeapon*) = nullptr;
// battle_unit.o  80126968
int32_t (*g_BtlUnit_GetWeaponCost_trampoline)(BattleWorkUnit*, BattleWeapon*) = nullptr;
// battle_unit.o  80126ca8
int32_t (*g_BtlUnit_GetCoin_trampoline)(BattleWorkUnit*) = nullptr;
// battle_unit.o  80127890
uint32_t (*g_BtlUnit_CheckRecoveryStatus_trampoline)(BattleWorkUnit*, int8_t) = nullptr;
// battle_unit.o  80128fe0
BattleWorkUnit* (*g_BtlUnit_Entry_trampoline)(BattleUnitSetup*) = nullptr;
// battle_unit_event.o  80129994
bool (*g_BattleCheckEndUnitInitEvent_trampoline)(BattleWork*) = nullptr;
// battle_item_data.o  8012eaf4
int32_t (*g_BattleItemData_rank_up_trampoline)(EvtEntry*, bool) = nullptr;
// statuswindow.o  8013cb24
void (*g_statusWinDisp_trampoline)(void) = nullptr;
// statuswindow.o  8013d440
void (*g_gaugeDisp_trampoline)(double, double, int32_t) = nullptr;
// win_item.o  80169cf0
void (*g_itemUseDisp2_trampoline)(WinMgrEntry*) = nullptr;
// win_item.o  80169ec0
void (*g_itemUseDisp_trampoline)(WinMgrEntry*) = nullptr;
// win_item.o  8016bbec
void (*g_winItemDisp_trampoline)(CameraId, WinPauseMenu*, int32_t) = nullptr;
// win_item.o  8016bf10
void (*g_winItemMain2_trampoline)(WinPauseMenu*) = nullptr;
// win_item.o  8016c030
int32_t (*g_winItemMain_trampoline)(WinPauseMenu*) = nullptr;
// win_log.o  80173c70
void (*g_winLogDisp_trampoline)(CameraId, WinPauseMenu*, int32_t) = nullptr;
// win_log.o  80174758
void (*g_winLogMain2_trampoline)(WinPauseMenu*) = nullptr;
// win_log.o  80174df4
int32_t (*g_winLogMain_trampoline)(WinPauseMenu*) = nullptr;
// win_log.o  80176b7c
void (*g_winLogExit_trampoline)(WinPauseMenu*) = nullptr;
// win_log.o  80176bf8
void (*g_winLogInit2_trampoline)(WinPauseMenu*) = nullptr;
// win_log.o  80176d30
void (*g_winLogInit_trampoline)(WinPauseMenu*) = nullptr;
// unit_party_chuchurina.o  80181bdc
int32_t (*g__make_madowase_weapon_trampoline)(EvtEntry*, bool) = nullptr;
// unit_party_christine.o  801895b0
int32_t (*g_btlevtcmd_get_monosiri_msg_no_trampoline)(EvtEntry*, bool) = nullptr;
// battle_actrecord.o  8018ef8c
void (*g_BtlActRec_JudgeRuleKeep_trampoline)(void) = nullptr;
// battle_actrecord.o  8018f990
void (*g_BtlActRec_AddCount_trampoline)(uint8_t*) = nullptr;
// battle_audience.o  801a1d0c
void (*g_BattleAudience_ApRecoveryBuild_trampoline)(SpBonusInfo*) = nullptr;
// battle_audience.o  801a5a0c
void (*g_BattleAudienceSetThrowItemMax_trampoline)() = nullptr;
// battle_message.o  801abdd0
int32_t (*g_btlevtcmd_AnnounceMessage_trampoline)(EvtEntry*, bool) = nullptr;
// battle_enemy_item.o  801f9658
void* (*g_BattleEnemyUseItemCheck_trampoline)(BattleWorkUnit*) = nullptr;
// ac_button_down.o  80202164
uint32_t (*g_battleAcMain_ButtonDown_trampoline)(BattleWork*) = nullptr;
// battle_seq_end.o  802166a4
const char* (*g_BattleGetRankNameLabel_trampoline)(int32_t) = nullptr;
// sac_bakugame.o  8022f990
int32_t (*g_bakuGameDecideWeapon_trampoline)(EvtEntry*, bool) = nullptr;
// sac_zubastar.o  80235c7c
uint32_t (*g_weaponGetPower_ZubaStar_trampoline)(
    BattleWorkUnit*, BattleWeapon*, BattleWorkUnit*, BattleWorkUnitPart*) = nullptr;
// winmgr.o  8023cf50
int32_t (*g_winMgrSelectOther_trampoline)(WinMgrSelectEntry*, EvtEntry*) = nullptr;
// winmgr.o  8023d624
WinMgrSelectEntry* (*g_winMgrSelectEntry_trampoline)(int32_t, int32_t, int32_t) = nullptr;
// sac_genki.o  80245a50
int32_t (*g_sac_genki_get_score_trampoline)(EvtEntry*, bool) = nullptr;
// sac_deka.o  80249048
int32_t (*g_weaponGetPower_Deka_trampoline)(
    BattleWorkUnit*, BattleWeapon*, BattleWorkUnit*, BattleWorkUnitPart*) = nullptr;
// sac_muki.o  8024ee50
int32_t (*g_main_muki_trampoline)(EvtEntry*, bool) = nullptr;
// sac_suki.o  8024fa74
int32_t (*g_sac_suki_set_weapon_trampoline)(EvtEntry*, bool) = nullptr;
// evt_memcard.o  8025c0b8
int32_t (*g_memcard_write_trampoline)(EvtEntry*, bool) = nullptr;
// evt_memcard.o  8025c454
int32_t (*g_memcard_code_trampoline)(EvtEntry*, bool) = nullptr;
// os.a OSLink.c  8029a8e4
bool (*g_OSLink_trampoline)(OSModuleInfo*, void*) = nullptr;

// Branch / patch addresses (BH / EH / CH# = begin / end / conditional hooks).

extern const int32_t g_seq_mapChangeMain_OnMapUnload_BH = 0x80007e18;
extern const int32_t g_seq_mapChangeMain_OnMapUnload_EH = 0x80007e68;
extern const int32_t g_seq_mapChangeMain_MapLoad_BH = 0x80007ef0;
extern const int32_t g_seq_mapChangeMain_MapLoad_EH = 0x80008148;
extern const int32_t g_titleMain_Patch_NeverPlayDemo = 0x800096c0;
extern const int32_t g_titleInit_Patch_EnableCrashHandler = 0x80009b2c;
extern const int32_t g_fbatBattleMode_Patch_BumpAttackLevel = 0x80046b94;
extern const int32_t g_fbatBattleMode_SkipStolenCheck_BH = 0x80046e6c;
extern const int32_t g_fbatBattleMode_SkipStolenCheck_EH = 0x80046e70;
extern const int32_t g_fbatBattleMode_SkipStolenCheck_CH1 = 0x80046f04;
extern const int32_t g_fbatBattleMode_CalculateCoinDrops_BH = 0x80046f20;
extern const int32_t g_fbatBattleMode_CalculateCoinDrops_EH = 0x80046fac;
extern const int32_t g_fbatBattleMode_GivePlayerInvuln_BH = 0x8004706c;
extern const int32_t g_fbatBattleMode_GivePlayerInvuln_EH = 0x800470c8;
extern const int32_t g_marioMain_Patch_SkipRunawayCoinDrop = 0x8005aa94;
extern const int32_t g_msgWindow_Entry_Patch_FixAllocSize = 0x800816f4;
extern const int32_t g_seq_logoMain_Patch_AlwaysSkipIntro = 0x800872c4;
extern const int32_t g_mot_hammer_PickHammerFieldSfx_BH = 0x8009897c;
extern const int32_t g_mot_hammer_PickHammerFieldSfx_EH = 0x80098a18;
extern const int32_t g_itemseq_Bound_Patch_BounceRange = 0x800aac68;
extern const int32_t g_itemseq_GetItem_Patch_SkipTutorials = 0x800abcd8;
extern const int32_t g_itemEntry_CheckDeleteFieldItem_BH = 0x800adae4;
extern const int32_t g_itemMain_CheckItemFreeze_BH = 0x800ae0bc;
extern const int32_t g_pouchRemoveItemIndex_CheckMaxInv_BH = 0x800d49d8;
extern const int32_t g_pouchRemoveItem_CheckMaxInv_BH = 0x800d4c94;
extern const int32_t g_pouchGetItem_CheckMaxInv_BH = 0x800d533c;
extern const int32_t g_pouchGetEmptyHaveItemCnt_CheckMaxInv_BH = 0x800d5638;
extern const int32_t g_pouchInit_FixAllocLeak_BH = 0x800d59dc;
extern const int32_t g_continueGame_Patch_SkipZeroingGswfs = 0x800f3ecc;
extern const int32_t g_loadMain_Patch_SkipZeroingGswfs = 0x800f6358;
extern const int32_t g_BattleActionCommandCheckDefence_GetDifficulty_BH = 0x800fa224;
extern const int32_t g_BattleActionCommandCheckDefence_GetDifficulty_EH = 0x800fa238;
extern const int32_t g_BattleCheckDamage_AlwaysFreezeBreak_BH = 0x800fb5bc;
extern const int32_t g_BattleCheckDamage_CalculateCounterDamage_BH = 0x800fb7cc;
extern const int32_t g_BattleCheckDamage_CalculateCounterDamage_EH = 0x800fb838;
extern const int32_t g_BattleSetStatusDamage_Patch_FeelingFineYesCase = 0x800fc038;
extern const int32_t g_BattleSetStatusDamage_Patch_FeelingFineNoCase = 0x800fc040;
extern const int32_t g_BattleSetStatusDamage_Patch_GaleLevelFactor = 0x800fc0a8;
extern const int32_t g_BattleSetStatusDamage_Patch_SkipHugeTinyArrows = 0x800fcb2c;
extern const int32_t g_BattleDamageDirect_Patch_AddTotalDamage = 0x800fe058;
extern const int32_t g_BattleDamageDirect_Patch_PityFlowerChance = 0x800fe500;
extern const int32_t g_BattleDamageDirect_CheckPlayAttackFX_BH = 0x800fe5a4;
extern const int32_t g_BattleDamageDirect_CheckPlayAttackFX_EH = 0x800fe61c;
extern const int32_t g_BattleDamageDirect_CheckPlayAttackFX_CH1 = 0x800fe71c;
extern const int32_t g_BattleChoiceSamplingEnemy_SumRandWeights_BH = 0x800ff528;
extern const int32_t g_BattleChoiceSamplingEnemy_SumRandWeights_EH = 0x800ff544;
extern const int32_t g_btlDispMain_DrawNormalHeldItem_BH = 0x80102ff4;
extern const int32_t g_btlevtcmd_ConsumeItemReserve_Patch_RefundPer = 0x8010ae84;
extern const int32_t g_btlevtcmd_ConsumeItemReserve_Patch_RefundBase = 0x8010aea0;
extern const int32_t g_btlevtcmd_ConsumeItem_Patch_RefundPer = 0x8010affc;
extern const int32_t g_btlevtcmd_ConsumeItem_Patch_RefundBase = 0x8010b018;
extern const int32_t g_btlevtcmd_CheckSpace_Patch_CheckEnemyTypes = 0x8010efdc;
extern const int32_t g_DrawMenuPartyPinchMark_CheckThreshold_BH = 0x80117e90;
extern const int32_t g_DrawMenuPartyPinchMark_Patch_CheckResult1 = 0x80117e98;
extern const int32_t g_DrawMenuPartyPinchMark_Patch_CheckResult2 = 0x80117e9c;
extern const int32_t g_DrawMenuMarioPinchMark_CheckThreshold_BH = 0x80118074;
extern const int32_t g_DrawMenuMarioPinchMark_Patch_CheckResult1 = 0x8011807c;
extern const int32_t g_DrawMenuMarioPinchMark_Patch_CheckResult2 = 0x80118080;
extern const int32_t g_btlSeqAct_SetConfuseProcRate_BH = 0x8011b75c;
extern const int32_t g_btlSeqMove_FixMarioSingleMoveCheck_BH = 0x8011c05c;
extern const int32_t g_btlSeqMove_FixMarioSingleMoveCheck_EH = 0x8011c11c;
extern const int32_t g_btlseqTurn_Patch_RuleDispShowLonger = 0x8011c5ec;
extern const int32_t g_btlseqTurn_Patch_RuleDispDismissOnlyWithB = 0x8011c62c;
extern const int32_t g_btlseqTurn_HappyHeartProc_BH = 0x8011ded4;
extern const int32_t g_btlseqTurn_HappyHeartProc_EH = 0x8011defc;
extern const int32_t g_btlseqTurn_HappyFlowerProc_BH = 0x8011e08c;
extern const int32_t g_btlseqTurn_HappyFlowerProc_EH = 0x8011e0b4;
extern const int32_t g_btlseqTurn_SpGradualRecoveryProc_BH = 0x8011e36c;
extern const int32_t g_BattleDrawEnemyHP_DrawEnemyHPText_BH = 0x8011ffcc;
extern const int32_t g__btlcmd_SetAttackEvent_SwitchPartnerCost_BH = 0x801204c8;
extern const int32_t g_BattleCommandDisplay_HandleSelectSide_BH = 0x80120cf4;
extern const int32_t g_BattleCommandDisplay_HandleSelectSide_EH = 0x80120cf8;
extern const int32_t g_BattleCommandDisplay_HandleSelectSide_CH1 = 0x80120fa4;
extern const int32_t g_BattleCommandInput_HandleSelectSide_BH = 0x80122e84;
extern const int32_t g__btlcmd_MakeOperationTable_AppealAlways_BH = 0x801239e4;
extern const int32_t g__btlcmd_MakeOperationTable_Patch_NoSuperCharge = 0x80123b00;
extern const int32_t g__btlcmd_MakeSelectWeaponTable_Patch_GetNameFromItem = 0x80124924;
extern const int32_t g_BtlUnit_EnemyItemCanUseCheck_Patch_SkipCheck = 0x80125d54;
extern const int32_t g_BtlUnit_CheckPinchStatus_PerilThreshold_BH = 0x80126e20;
extern const int32_t g_BtlUnit_CheckPinchStatus_DangerThreshold_BH = 0x80126e84;
extern const int32_t g_statusWinDisp_HideDpadMenuOutsidePit_BH = 0x8013d140;
extern const int32_t g_statusWinDisp_HideDpadMenuOutsidePit_EH = 0x8013d144;
extern const int32_t g_statusWinDisp_HideDpadMenuOutsidePit_CH1 = 0x8013d404;
extern const int32_t g_winHakoGX_SetInitialFields_BH = 0x801584ac;
extern const int32_t g_winHakoGX_SetInitialFields_EH = 0x801584d4;
extern const int32_t g_winHakoGX_Patch_SkipSingleBox = 0x80158728;
extern const int32_t g_winHakoGX_CheckDrawNoItemBox_BH = 0x80159cf8;
extern const int32_t g_winHakoGX_CheckDrawNoItemBox_EH = 0x80159d34;
extern const int32_t g_winHakoGX_CheckDrawNoItemBox_CH1 = 0x80159dcc;
extern const int32_t g_winHakoGX_CheckDrawItemIcon_BH = 0x80159e8c;
extern const int32_t g_winHakoGX_CheckDrawItemIcon_EH = 0x80159f8c;
extern const int32_t g_winHakoGX_CheckDrawItemIcon_CH1 = 0x80159ef8;
extern const int32_t g_winRootDisp_Patch_SkipMailGx = 0x801647b0;
extern const int32_t g_winPartyDisp_StatsHook1_BH = 0x80165c4c;
extern const int32_t g_winPartyDisp_StatsHook1_EH = 0x80166290;
extern const int32_t g_winPartyDisp_StatsHook2_BH = 0x80166cb8;
extern const int32_t g_winPartyDisp_StatsHook2_EH = 0x801671a8;
extern const int32_t g_winPartyMain_RotatePartnersHook_BH = 0x801675ac;
extern const int32_t g_winPartyMain_RotatePartnersHook_EH = 0x801675ec;
extern const int32_t g_winPartyMain_OverrideMoveTextCursor_BH = 0x801676b8;
extern const int32_t g_winPartyMain_OverrideMoveTextCursor_EH = 0x80167718;
extern const int32_t g_winBadge_mario_change_Patch_SkipMapAnim1 = 0x801697a0;
extern const int32_t g_winBadge_mario_change_Patch_SkipMapAnim2 = 0x80169868;
extern const int32_t g_winMarioDisp_MoveMenuDisp_BH = 0x8016f584;
extern const int32_t g_winMarioDisp_MoveMenuDisp_EH = 0x8016f7f8;
extern const int32_t g_winMarioMain_MoveDescription_BH = 0x80170c38;
extern const int32_t g_winMarioMain_MoveDescription_EH = 0x80170c48;
extern const int32_t g_winMarioMain_CheckOpenMoveMenu_BH = 0x801704d4;
extern const int32_t g_acShot_dispAfterimage_Patch_numBombs = 0x80197ffb;
extern const int32_t g_acShot_dispAfterimage_Patch_targetVar2_1 = 0x80197f6f;
extern const int32_t g_acShot_dispAfterimage_Patch_targetVar2_2 = 0x80197fcf;
extern const int32_t g_acShot_dispAfterimage_Patch_targetVar3_1 = 0x80197ee7;
extern const int32_t g_acShot_dispAfterimage_Patch_targetVar3_2 = 0x80197efb;
extern const int32_t g_acShot_dispAfterimage_Patch_targetVar3_3 = 0x80197f43;
extern const int32_t g_acShot_dispAfterimage_Patch_targetVar3_4 = 0x80197fd7;
extern const int32_t g_acShot_main_Patch_numBombs = 0x801987af;
extern const int32_t g_acShot_main_Patch_targetVar2_1 = 0x801987f3;
extern const int32_t g_acShot_main_Patch_targetVar2_2 = 0x80198987;
extern const int32_t g_acShot_main_Patch_targetVar2_3 = 0x80198acb;
extern const int32_t g_acShot_main_Patch_targetVar2_4 = 0x80198c83;
extern const int32_t g_acShot_main_Patch_targetVar2_5 = 0x80198d83;
extern const int32_t g_acShot_main_Patch_targetVar2_6 = 0x80198e97;
extern const int32_t g_acShot_main_Patch_targetVar3_1 = 0x80198997;
extern const int32_t g_acShot_main_Patch_targetVar3_2 = 0x80198adb;
extern const int32_t g_acShot_main_Patch_targetVar3_3 = 0x80198c93;
extern const int32_t g_acShot_main_Patch_targetVar3_4 = 0x80198d93;
extern const int32_t g_acShot_main_Patch_targetVar3_5 = 0x80198ea7;
extern const int32_t g_acShot_main_Patch_sfxId_1 = 0x80198803;
extern const int32_t g_acShot_main_Patch_sfxId_2 = 0x80198827;
extern const int32_t g_acShot_main_Patch_sfxId_3 = 0x80198f13;
extern const int32_t g_BattleAudience_Case_Appeal_Patch_AppealSp = 0x8019f238;
extern const int32_t g_BattleAudienceAddPuni_EnableAlways_BH = 0x801a15c8;
extern const int32_t g_BattleAudienceAddAudienceNum_EnableAlways_BH = 0x801a1734;
extern const int32_t g_BattleAudienceItemCtrlProcess_Patch_CheckItemValidRange = 0x801a5418;
extern const int32_t g_BattleAudienceItemCtrlProcess_CheckSpace_BH = 0x801a5424;
extern const int32_t g_BattleAudienceDetectTargetPlayer_CheckPlayer_BH = 0x801a58b0;
extern const int32_t g_BattleAudienceDetectTargetPlayer_CheckPlayer_EH = 0x801a58b8;
extern const int32_t g_BattleAudienceItemOn_RandomItem_BH = 0x801a5c70;
extern const int32_t g_BattleAudienceSettingAudience_EnableAlways_BH = 0x801a61ac;
extern const int32_t g__object_fall_attack_AudienceEnableAlways_BH = 0x801469e4;
extern const int32_t g_effUpdownDisp_TwoDigitSupport_BH = 0x80193aec;
extern const int32_t g_effUpdownDisp_TwoDigitSupport_EH = 0x80193cd4;
extern const int32_t g_BattleAudience_SetTargetAmount_BH = 0x801a61ec;
extern const int32_t g_BattleAudience_End_SaveAmountAlways_BH = 0x801a6b68;
extern const int32_t g_BattleAudience_Disp_EnableAlways_BH = 0x801a6cb0;
extern const int32_t g_battleAcMain_ButtonDown_ChooseButtons_BH = 0x80202414;
extern const int32_t g_battleAcMain_ButtonDown_ChooseButtons_EH = 0x80202418;
extern const int32_t g_battleAcMain_ButtonDown_WrongButton_BH = 0x80202574;
extern const int32_t g_battleAcMain_ButtonDown_WrongButton_EH = 0x80202578;
extern const int32_t g_battleAcMain_ButtonDown_WrongButton_CH1 = 0x802025c8;
extern const int32_t g_battleAcMain_ButtonDown_CheckComplete_BH = 0x80202614;
extern const int32_t g_battleAcMain_ButtonDown_CheckComplete_EH = 0x80202618;
extern const int32_t g_battleAcMain_ButtonDown_CheckComplete_CH1 = 0x8020263c;
extern const int32_t g_BattleBreakSlot_PointInc_EnableAlways_BH = 0x802034b4;
extern const int32_t g_effStarPointDisp_Patch_SetIconId = 0x8020af38;
extern const int32_t g_btlseqEnd_JudgeRuleEarly_BH = 0x80215348;
extern const int32_t g_btlseqEnd_Patch_CheckDisableExpLevel = 0x802157e0;
extern const int32_t g_btlseqEnd_Patch_RemoveJudgeRule = 0x80216678;
extern const int32_t g_scissor_damage_sub_ArtAttackDamage_BH = 0x80231cc0;
extern const int32_t g_scissor_damage_sub_ArtAttackDamage_EH = 0x80231d38;
extern const int32_t g_scissor_damage_Patch_ArtAttackCheckImmunity = 0x80231e50;
extern const int32_t g_select_disp_Patch_PitListPriceHook = 0x8023c120;
extern const int32_t g_select_main_CheckHideTopBar_BH = 0x8023cb50;
extern const int32_t g_select_disp_Patch_PitItemPriceHook = 0x8023d2e0;
extern const int32_t g_winMgrSelectEntry_Patch_SelectDescTblHi16 = 0x8023d6b4;
extern const int32_t g_winMgrSelectEntry_Patch_SelectDescTblLo16 = 0x8023d6bc;
extern const int32_t g_sac_genki_main_base_BlinkNumbers_BH = 0x80248220;
extern const int32_t g_sac_genki_main_base_BlinkNumbers_EH = 0x802483b4;
extern const int32_t g_sac_genki_main_base_SetupTargets_BH = 0x80248430;
extern const int32_t g_sac_genki_main_base_SetupTargets_EH = 0x8024864c;
extern const int32_t g_sac_deka_main_base_GetNumberOfBars_BH = 0x8024b0b8;
extern const int32_t g_battle_status_icon_SkipIconForPermanentStatus_BH = 0x80253888;
extern const int32_t g_battle_status_icon_SkipIconForPermanentStatus_EH = 0x8025388c;
extern const int32_t g_battle_status_icon_SkipIconForPermanentStatus_CH1 = 0x80253924;
extern const int32_t g_crashHandler_Patch_LoopForever1 = 0x8025e4a4;
extern const int32_t g_crashHandler_Patch_LoopForever2 = 0x8025e4a8;
extern const int32_t g_ac_monosiri_target_WhiteReticleScale = 0x80301250;
extern const int32_t g_ac_monosiri_target_GreyReticleScale = 0x80301280;
extern const int32_t g_ac_monosiri_target_ReticleZoomSpeed = 0x80428610;
extern const int32_t g_enemy_common_dead_event_SpawnCoinsHook = 0x8033f094;
extern const int32_t g_chorobon_move_event_Patch_SetScale = 0x803427e8;
extern const int32_t g_chorobon_find_event_Patch_SetScale = 0x803429c8;
extern const int32_t g_chorobon_lost_event_Patch_SetScale = 0x80342a00;
extern const int32_t g_chorobon_return_event_Patch_SetScale = 0x80342a38;
extern const int32_t g_BattleSetStatusDamage_FeelingFine_SwitchTable = 0x8034c8b4;
extern const int32_t g_subsetevt_blow_dead_Patch_GetRewards = 0x80351ea4;
extern const int32_t g_subsetevt_shot_damage_Patch_SuperInvolvedWeapon = 0x80351f40;
extern const int32_t g_subsetevt_shot_damage_Patch_UltraInvolvedWeapon = 0x80351f58;
extern const int32_t g_subsetevt_swallow_shot_damage_Patch_InvolvedWeapon = 0x80352c78;
extern const int32_t g_ItemEvent_Support_NoEffect_TradeOffJumpPoint = 0x803652b8;
extern const int32_t g_ItemEvent_Teki_Kyouka_ApplyStatusHook = 0x80369b34;
extern const int32_t g_ItemEvent_Poison_Kinoko_PoisonChance = 0x8036c914;
extern const int32_t g_ItemEvent_LastDinner_Weapon = 0x8036caf4;
extern const int32_t g_partyClauda_makeTechMenuFuncPtr = 0x8037917c;
extern const int32_t g_partyYoshi_makeTechMenuFuncPtr = 0x8037c55c;
extern const int32_t g_partyChuchurina_makeTechMenuFuncPtr = 0x80381764;
extern const int32_t g_partySanders_makeTechMenuFuncPtr = 0x8038778c;
extern const int32_t g_partyVivian_makeTechMenuFuncPtr = 0x8038b7cc;
extern const int32_t g_partyNokotarou_makeTechMenuFuncPtr = 0x8038eeac;
extern const int32_t g_partyChristine_makeTechMenuFuncPtr = 0x80393b74;
extern const int32_t g_partyNokotarou_Patch_InitWaitPhase = 0x8038f0e4;
extern const int32_t g_koura_damage_core_Patch_HeavyDmg = 0x8039c190;
extern const int32_t g_koura_damage_core_Patch_LightDmg = 0x8039c1ec;
extern const int32_t g_koura_pose_tbl_reset_Patch_HeavyDmg = 0x8039c2ec;
extern const int32_t g_koura_pose_tbl_reset_Patch_LightDmg = 0x8039c338;
extern const int32_t g_genki_evt_common_Patch_SweetTreatFeastResult = 0x803b6bac;
extern const int32_t g_genki_evt_common_SweetTreatResultJumpPoint = 0x803b6be8;
extern const int32_t g_ac_power_gauge_lv2_LipLockPower = 0x8041dbf0;
extern const int32_t g_ac_air_gauge_FlurrieGaleForceResistance = 0x80426fd0;
extern const int32_t g_crashHandler_Patch_FontScale = 0x80428bc0;

}