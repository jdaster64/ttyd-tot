#pragma once

#include <cstdint>

namespace ttyd::battle {
struct SpBonusInfo;
}
namespace ttyd::battle_database_common {
struct BattleWeapon;
}
namespace ttyd::battle_unit {
struct BattleWorkUnit;
}

namespace ttyd::battle_audience {

extern "C" {

// BattleAudience_GetPresentItemType
void BattleAudience_SetPresentItemType(int32_t harmful_item);
// BattleAudience_GetPresentTargetUnitId
// BattleAudience_SetPresentTargetUnitId
// BattleAudience_GetPresentItemNo
// BattleAudience_SetPresentItemNo
// BattleAudience_Case_FirstAttack_Bad
// BattleAudience_Case_FirstAttack_Good
// BattleAudience_Case_FallObject_Aud
// BattleAudience_Case_FallObject_Stage
// BattleAudience_Case_FastVictory
// BattleAudience_Case_GreatVictory
// BattleAudience_Case_HaikeiSet
// BattleAudience_Case_Countered
// BattleAudience_Case_EnemyNoDamage
// BattleAudience_Case_EnemyDamage
// BattleAudience_Case_Escape_Bad
// BattleAudience_Case_Escape_Good
// BattleAudience_Case_Escape
void BattleAudience_Case_Appeal(ttyd::battle_unit::BattleWorkUnit* unit);
// BattleAudience_Case_TurnEnd
// BattleAudience_Case_FinalAttack
// BattleAudience_Case_JumpNewRecord
// BattleAudience_Case_PartyDown
// BattleAudience_Case_MarioDanger
// BattleAudience_Case_MarioPinch
// BattleAudience_Case_MarioBigDamage
// BattleAudience_Case_AcrobatNoTry
// BattleAudience_Case_AcrobatBad
// BattleAudience_Case_AcrobatGood
// BattleAudience_Case_GuardBad
// BattleAudience_Case_GuardGood
void BattleAudience_Case_ActionCommandBad(
    ttyd::battle_database_common::BattleWeapon* weapon);
// BattleAudience_Case_ActionCommandGood
// BattleAudience_Case_KillEnemy
// BattleAudienceNoiseMain
// BattleAudienceSoundMain
// BattleAudienceSoundGetInfo2
// BattleAudienceSound2
// BattleAudienceSoundGetInfo1
// BattleAudienceSound1
// BattleAudienceSound0
// BattleAudienceSoundStop
// BattleAudienceSoundCheck
// BattleAudienceSoundSetVolAll
// BattleAudienceSoundSetVol
// BattleAudienceSoundPakkunEat
// BattleAudienceSoundBombFire
// BattleAudienceSoundBombIgnite
// BattleAudienceSoundShell
// BattleAudienceSoundSing
// BattleAudienceSoundZZZ
// BattleAudienceSoundSleep
// BattleAudienceSoundRun
// BattleAudienceSoundItemThrow
// BattleAudienceSoundHandBeat
// BattleAudienceSoundCallKind
// BattleAudienceSoundWhistleKind
// BattleAudienceSoundWhistle
// BattleAudienceSoundCheerKind
void BattleAudienceSoundCheer(int32_t length, int32_t fade_length);
// BattleAudienceSoundNoiseAlways
void BattleAudienceSoundClap(int32_t length, int32_t fade_length);
// BattleAudienceSoundBooingKind
// BattleAudienceSoundBooing
// BattleAudienceDetectPakkunEatTarget
// BattleAudienceDetectPakkunEatTargetSub
// BattleAudienceDetectPakkunEatTargetSub2
// BattleAudienceJoy_Sub
// BattleAudienceJoyEnding
// BattleAudienceJoySACLecture
void BattleAudienceJoy(int32_t cheer_type);
// BattleAudienceCheer
void BattleAudienceAddPhaseEvtList(int32_t phase_evt_type);
// BattleAudienceAddPuni
// BattleAudiencePuniAllEscape
// BattleAudienceAddAudienceNum
// BattleAudienceAddTargetNumSub
void BattleAudienceAddTargetNum(double added, double bonus);
// BattleAudienceNumToTargetSub
// BattleAudienceNumToTarget
// BattleAudience_WinSetActive
void BattleAudience_ApRecoveryBuild(battle::SpBonusInfo* bonus_info);
// BattleAudience_GetPPAudienceNum_Sub
// BattleAudience_GetPPAudienceNum_RL_Sub
// BattleAudience_GetPPAudienceNum_L
// BattleAudience_GetPPAudienceNum_R
int32_t BattleAudience_GetPPAudienceNumKind(int32_t audience_kind);
int32_t BattleAudience_GetPPAudienceNum();
// BattleAudience_GetAudienceNum
// BattleAudience_NoUsedFCHaitiRand
// unk_801a21e0
// unk_801a23e0
// BattleAudience_NoUsedHaitiRand
// BattleAudience_HaitiRandForFallObject
// BattleAudience_GetAudienceNoFromOffset
// BattleAudience_GetEscapeChangeOK
// BattleAudience_GetWaiting
// BattleAudience_GetSysCtrl
// BattleAudience_GetExist
// BattleAudience_ChangeStatus
// BattleAudience_GetFront
// BattleAudience_Attack
// BattleAudience_SetTarget
// BattleAudience_GetItemOn2
// BattleAudience_GetItemOn
// BattleAudience_SetRotateOffset
// BattleAudience_SetRotate
// BattleAudience_SetPosition
// BattleAudience_GetRotate
// BattleAudience_GetHomePosition
// BattleAudience_GetPosition
// BattleAudience_GetAnimEnd
// BattleAudience_SetAnim
// BattleAudience_Delete
// BattleAudience_Entry_Sub
// BattleAudience_Entry
// BattleAudienceDispWin
// BattleAudienceDispApSrc
// BattleAudienceDispItem
// BattleAudienceDispAudience
// BattleAudienceAnimProcess
// BattleAudienceGXInit
// BattleAudienceWinCtrlProcess
// BattleAudienceApSrcEntry
// BattleAudienceApSrcCtrlProcess
// BattleAudienceItemCtrlProcess
// BattleAudienceDetectTargetPlayer
// BattleAudienceDetectTargetAll
void BattleAudienceSetThrowItemMax();
// BattleAudienceItemOn
// BattleAudienceCtrlProcess
// BattleAudienceGuestTPLRead
// BattleAudienceSettingAudience
// BattleAudience_End
// BattleAudience_Disp
// BattleAudience_CheckReactionPerPhase
// BattleAudience_PerPhase
// check_exe_phase_evt_status
// BattleAudience_CheckReaction
// BattleAudience_PerAct
// BattleAudience_ActInit
// BattleAudience_Main
// BattleAudience_Init
// BattleAudienceSoundGetPtr
// BattleAudienceWinGetPtr
// BattleAudienceItemGetPtr
// BattleAudienceGetPtr
// BattleAudienceBaseGetPtr
// tplRead

}

}