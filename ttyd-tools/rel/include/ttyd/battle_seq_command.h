#pragma once

#include <ttyd/battle_database_common.h>

#include <cstdint>

namespace ttyd::battle {
struct BattleWork;
struct BattleWorkCommand;
struct BattleWorkCommandCursor;
}
namespace ttyd::battle_unit {
struct BattleWorkUnit;
}

namespace ttyd::battle_seq_command {

extern "C" {

// _check_weapon_type_attack
EVT_DECLARE_USER_FUNC(BattleSeqCmd_get_msg, 1)
// BattleCommandAttackAudienceCheck
// _commandRestoreRec
void* BattleSetConfuseAct(
    battle::BattleWork* battleWork, battle_unit::BattleWorkUnit* unit);
// BattleDrawEnemyHPBar
// BattleDrawEnemyHP
// _check_present_item
// BattleGetSelectWeapon
// _btlcmd_SetAttackEvent
// BattleCommandDisplay_AllEnd
// BattleCommandDisplay_ProtectPartnerSelectMenuEnd
// BattleCommandDisplay_ProtectPartnerSelectMenuDisp
// BattleCommandDisplay_ProtectPartnerSelectMenuSetup
// BattleCommandDisplay_TargetSelectMenuEnd
// BattleCommandDisplay_TargetSelectMenuDisp
// BattleCommandDisplay_TargetSelectMenuSetup
// BattleCommandDisplay_ChangePartySelectMenuEnd
// BattleCommandDisplay_ChangePartySelectMenuDisp
// BattleCommandDisplay_ChangePartySelectMenuMain
// BattleCommandDisplay_ChangePartySelectMenuSetup
// BattleCommandDisplay_MultiItemMenuEnd
// BattleCommandDisplay_MultiItemMenuDisp
// BattleCommandDisplay_MultiItemMenuMain
// BattleCommandDisplay_MultiItemMenuSetup
// BattleCommandDisplay_OperationMenuEnd
// BattleCommandDisplay_OperationMenuDisp
// BattleCommandDisplay_OperationMenuMain
// BattleCommandDisplay_OperationMenuSetup
// BattleCommandDisplay_WeaponMultiItemCancelCheckDisp
// BattleCommandDisplay_WeaponMultiItemCancelCheckMain
// BattleCommandDisplay_WeaponMultiItemCancelCheckSetup
// BattleCommandDisplay_WeaponSelectMenuEnd
// BattleCommandDisplay_WeaponSelectMenuDisp
// BattleCommandDisplay_WeaponSelectMenuMain
// BattleCommandDisplay_WeaponSelectMenuSetup
// BattleCommandDisplay_ActMenuEnd
// BattleCommandDisplay_ActMenuDisp
// BattleCommandCheckChangePositionEnable
// BattleCommandDisplay_ActMenuMain
// BattleCommandDisplay_ActMenuSetup
// _cursor_init
// BattleCommandDisplay
// BattleCommandInput
// _btlcmd_SelectWeaponDecide
// _btlcmd_MakePartyTable
// _battleGetPartyIcon
// _btlcmd_MakeMultiItemTable
// _btlcmd_MakeOperationTable
// _btlcmd_CheckWeaponTargetNum
void _btlcmd_UpdateSelectWeaponTable(
    battle::BattleWork* battleWork, int32_t table_type);
int32_t _btlcmd_MakeSelectWeaponTable(
    battle::BattleWork* battleWork, int32_t table_type);
// _getHammerIconId
// _getJumpIconId
void _btlcmd_GetCursorPtr(
    battle::BattleWorkCommand* command_work, int cursor_type,
    battle::BattleWorkCommandCursor** out_cursor);
// _btlcmd_MakeActClassTable
void BattleCommandInit(battle::BattleWork* battleWork);

// .data
extern ttyd::battle_database_common::BattleWeapon defaultWeapon_Dummy_NoItem;

}

}