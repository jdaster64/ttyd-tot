#include "tot_party_vivian.h"

#include "evt_cmd.h"
#include "tot_move_manager.h"

#include <ttyd/battle.h>
#include <ttyd/battle_camera.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_event_default.h>
#include <ttyd/battle_weapon_power.h>
#include <ttyd/evt_audience.h>
#include <ttyd/evt_eff.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evt_sub.h>
#include <ttyd/icondrv.h>
#include <ttyd/msgdrv.h>
#include <ttyd/unit_party_vivian.h>

namespace mod::tot::party_vivian {

namespace {
    
// Including entire namespaces for convenience.
using namespace ::ttyd::battle_camera;
using namespace ::ttyd::battle_database_common;
using namespace ::ttyd::battle_event_cmd;
using namespace ::ttyd::battle_event_default;
using namespace ::ttyd::battle_weapon_power;
using namespace ::ttyd::evt_audience;
using namespace ::ttyd::evt_eff;
using namespace ::ttyd::evt_snd;
using namespace ::ttyd::evt_sub;
using namespace ::ttyd::unit_party_vivian;

namespace IconType = ::ttyd::icondrv::IconType;

}  // namespace

// Declaration of weapon structs.
extern BattleWeapon customWeapon_VivianShadeFist;
extern BattleWeapon customWeapon_VivianVeil;
extern BattleWeapon customWeapon_VivianFieryJinx;
extern BattleWeapon customWeapon_VivianInfatuate;
extern BattleWeapon customWeapon_VivianMove5;
extern BattleWeapon customWeapon_VivianMove6;

BattleWeapon* g_WeaponTable[] = {
    &customWeapon_VivianShadeFist, &customWeapon_VivianVeil, 
    &customWeapon_VivianFieryJinx, &customWeapon_VivianInfatuate, 
    &customWeapon_VivianMove5, &customWeapon_VivianMove6
};

void MakeSelectWeaponTable(
    ttyd::battle::BattleWorkCommand* command_work, int32_t* num_options) {
    for (int32_t i = 0; i < 6; ++i) {
        auto& weapon_entry = command_work->weapon_table[*num_options];
        BattleWeapon* weapon = g_WeaponTable[i];
        
        weapon_entry.index = MoveType::VIVIAN_BASE + i;
        weapon_entry.item_id = 0;
        weapon_entry.weapon = weapon;
        weapon_entry.icon = weapon->icon;
        weapon_entry.unk_04 = 0;
        weapon_entry.unk_18 = 0;
        weapon_entry.name = ttyd::msgdrv::msgSearch(weapon->name);
        
        ++*num_options;
    }
}

EVT_BEGIN(partyVivianAttack_NormalAttack)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.0), FLOAT(0.70))
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    USER_FUNC(btlevtcmd_GetPartyTechLv, -2, LW(0))
    SWITCH(LW(0))
        CASE_EQUAL(2)
            USER_FUNC(_make_kagenuke_weapon, LW(12), 2)
        CASE_EQUAL(1)
            USER_FUNC(_make_kagenuke_weapon, LW(12), 2)
        CASE_ETC()
            USER_FUNC(_make_kagenuke_weapon, LW(12), 2)
    END_SWITCH()
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_CommandPayWeaponCost, -2)
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    USER_FUNC(evt_btl_camera_set_mode, 0, 11)
    USER_FUNC(evt_btl_camera_set_homing_unitparts, 0, -2, 1, LW(3), LW(4))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
    USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
    USER_FUNC(btlevtcmd_AcSetParamAll, 1, 1, 40, 1, 2, 0, 3, 0)
    USER_FUNC(btlevtcmd_AcSetFlag, 23)
    USER_FUNC(btlevtcmd_SetupAC, -2, 18, 1, 0)
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_VIVIAN_ATTACK1"), 0)
    USER_FUNC(battle_evt_majo_disp_off, -2, 1, 0, 0)
    USER_FUNC(btlevtcmd_StartAC, 1)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(unk_80182cc4, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetPos, -2, EVT_NULLPTR, LW(1), EVT_NULLPTR)
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 50, 0, 5, 0, -1)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 3, 1)
    USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_VIVIAN_ATTACK2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(battle_evt_majo_disp_on, -2, 1, LW(0), LW(1), LW(2), 0)
    USER_FUNC(btlevtcmd_SetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(6), LW(7), LW(8))
    USER_FUNC(unk_80182cc4, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(unk_80182cc4, LW(3), LW(6), LW(7), LW(8))
    USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(6), LW(7), LW(8))
    USER_FUNC(btlevtcmd_GetFaceDirection, LW(3), LW(15))
    MUL(LW(15), -1)
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(6), LW(7), LW(8))
    USER_FUNC(_get_move_frame, LW(0), LW(1), LW(2), LW(6), LW(7), LW(8), FLOAT(4.0), LW(6))
    IF_LARGE_EQUAL(LW(6), 1)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_VIVIAN_ATTACK3"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 12, 0, 5, 0, -1)
    END_IF()
    LBL(9)
    USER_FUNC(btlevtcmd_CommandPreCheckDamage, -2, LW(3), LW(4), 256, LW(5))
    IF_EQUAL(LW(5), 5)
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 256, LW(5))
    END_IF()
    USER_FUNC(btlevtcmd_ResultAC)
    SET(LW(10), 0)
    LBL(10)
    IF_LARGE_EQUAL(LW(10), 1)
        USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), LW(12), 1048832, LW(5))
    ELSE()
        USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), LW(12), 256, LW(5))
    END_IF()
    SWITCH(LW(5))
        CASE_OR(2)
        CASE_OR(3)
        CASE_OR(6)
        CASE_OR(4)
            CASE_END()
        CASE_EQUAL(1)
        CASE_ETC()
    END_SWITCH()
    IF_NOT_EQUAL(LW(5), 1)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PTR_A1_2"))
        WAIT_MSEC(166)
        IF_EQUAL(LW(5), 3)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 38)
        END_IF()
        IF_EQUAL(LW(5), 6)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 39)
        END_IF()
        IF_EQUAL(LW(5), 2)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 40)
        END_IF()
        WAIT_MSEC(500)
        GOTO(90)
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PTR_A1_2"))
    USER_FUNC(btlevtcmd_ftomsec, 4, LW(0))
    WAIT_MSEC(LW(0))
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_VIVIAN_ATTACK4"), 0)
    USER_FUNC(btlevtcmd_GetResultAC, LW(0))
    IF_FLAG(LW(0), 0x2)
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 131328, LW(5))
    ELSE()
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 536871168, LW(5))
    END_IF()
    IF_FLAG(LW(0), 0x2)
        USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), 0, LW(6))
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), LW(6))
    ELSE()
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), -1)
    END_IF()
    WAIT_MSEC(500)
    LBL(90)
    USER_FUNC(btlevtcmd_StopAC)
    LBL(95)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PTR_A1_1"))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 12, 0, 5, 0, -1)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 3, 0)
    USER_FUNC(battle_evt_majo_disp_off, -2, 1, 0, 0)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetPos, -2, EVT_NULLPTR, LW(1), EVT_NULLPTR)
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 40, 0, 5, 0, -1)
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_ACRStart, -2, 14, 29, 29, 0)
    END_BROTHER()
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(battle_evt_majo_disp_on, -2, 1, LW(0), LW(1), LW(2), 0)
    USER_FUNC(btlevtcmd_SetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
    IF_EQUAL(LW(6), 2)
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
        USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(0), 1, 0, 0, 0)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PTR_Y_1"))
        WAIT_FRM(52)
    END_IF()
    LBL(99)
    USER_FUNC(evt_audience_ap_recovery)
    USER_FUNC(btlevtcmd_InviteApInfoReport)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RUN_CHILD_EVT(PTR(&btldefaultevt_SuitoruBadgeEffect))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(partyVivianAttack_ShadowGuard)
    SET(LW(3), -3)
    USER_FUNC(btlevtcmd_GetBodyId, LW(3), LW(4))
    USER_FUNC(btlevtcmd_CommandPayWeaponCost, -2)
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, 1)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    USER_FUNC(evt_btl_camera_set_posoffset, 0, 20, 0, 0)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), -1, 16, LW(0))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(0))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PTR_A2_1"))
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, LW(3), LW(0), 15)
    ADD(LW(2), 5)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(2.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), -1, 16, LW(0))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(0))
    USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
    USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
    USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
    USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
    USER_FUNC(btlevtcmd_AcGetDifficulty, LW(0))
    SUB(LW(0), 3)
    SWITCH(LW(0))
        CASE_SMALL_EQUAL(-3)
            SET(LW(0), 420)
        CASE_EQUAL(-2)
            SET(LW(0), 330)
        CASE_EQUAL(-1)
            SET(LW(0), 270)
        CASE_EQUAL(0)
            SET(LW(0), 240)
        CASE_EQUAL(1)
            SET(LW(0), 200)
        CASE_EQUAL(2)
            SET(LW(0), 160)
        CASE_ETC()
            SET(LW(0), 120)
    END_SWITCH()
    USER_FUNC(btlevtcmd_AcSetParamAll, LW(0), 1, 5, -2, 5, EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AcSetFlag, 0)
    USER_FUNC(btlevtcmd_SetupAC, -2, 10, 1, 0)
    WAIT_FRM(22)
    USER_FUNC(btlevtcmd_StartAC, 1)
    USER_FUNC(btlevtcmd_ResultAC)
    USER_FUNC(btlevtcmd_GetResultAC, LW(0))
    IF_NOT_FLAG(LW(0), 0x2)
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), -1)
        USER_FUNC(btlevtcmd_StopAC)
        USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(2.0))
        USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
        GOTO(90)
    END_IF()
    USER_FUNC(btlevtcmd_GetResultPrizeLv, -1, 0, LW(6))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), LW(6))
    USER_FUNC(btlevtcmd_StopAC)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 350)
    USER_FUNC(evt_btl_camera_set_posoffset, 0, 0, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PTR_A2_2"))
    USER_FUNC(btlevtcmd_AnimeWaitPlayComplete, -2, 1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PTR_A2_3"))
    USER_FUNC(btlevtcmd_AnimeWaitPlayComplete, -2, 1)
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_AnimeChangePose, LW(3), 1, PTR("M_B_3"))
        USER_FUNC(battle_evt_majo_disp_off, LW(3), 1, 1, 0)
        USER_FUNC(btlevtcmd_OnAttribute, LW(3), 16)
        USER_FUNC(btlevtcmd_OnPartsAttribute, LW(3), 1, 50331648)
        WAIT_FRM(1)
        USER_FUNC(btlevtcmd_GetHomePos, LW(3), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_SetPos, LW(3), LW(0), LW(1), LW(2))
    END_BROTHER()
    USER_FUNC(battle_evt_majo_disp_off, -2, 1, 1, 1)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(5), LW(1), LW(2))
    ADD(LW(0), LW(5))
    DIV(LW(0), 2)
    USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
    ADD(LW(1), 1)
    USER_FUNC(evt_eff, 0, PTR("ripple"), 1, LW(0), LW(1), LW(2), 0, 0, 0, 0, 0, 0, 0, 0)
    WAIT_MSEC(333)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 1, 16842752)
    WAIT_FRM(1)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 4, 1)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetPos, -2, LW(0), LW(1), LW(2))
    WAIT_FRM(1)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, -1, -1, 4, LW(0))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(0))
    WAIT_MSEC(500)
    LBL(90)
    USER_FUNC(evt_audience_ap_recovery)
    USER_FUNC(btlevtcmd_InviteApInfoReport)
    LBL(95)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_posoffset, 0, 0, 0, 0)
    LBL(99)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(partyVivianAttack_MagicalPowder)
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_CommandPayWeaponCost, -2)
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    WAIT_FRM(20)
    USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
    USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
    USER_FUNC(btlevtcmd_AcGetDifficulty, LW(0))
    SUB(LW(0), 3)
    SWITCH(LW(0))
        CASE_SMALL_EQUAL(-3)
            SET(LW(0), 360)
        CASE_EQUAL(-2)
            SET(LW(0), 270)
        CASE_EQUAL(-1)
            SET(LW(0), 210)
        CASE_EQUAL(0)
            SET(LW(0), 180)
        CASE_EQUAL(1)
            SET(LW(0), 160)
        CASE_EQUAL(2)
            SET(LW(0), 140)
        CASE_ETC()
            SET(LW(0), 120)
    END_SWITCH()
    USER_FUNC(btlevtcmd_AcSetParamAll, LW(0), 1, 5, -3, 1, EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AcSetFlag, 7)
    USER_FUNC(btlevtcmd_SetupAC, -2, 10, 1, 0)
    WAIT_FRM(22)
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, 1)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 4)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 400)
    USER_FUNC(evt_btl_camera_set_posoffset, 0, 20, 0, 0)
    USER_FUNC(btlevtcmd_StartAC, 1)
    USER_FUNC(btlevtcmd_ResultAC)
    USER_FUNC(btlevtcmd_StopAC)
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetWidth, -2, LW(5))
        USER_FUNC(btlevtcmd_GetHeight, -2, LW(6))
        USER_FUNC(evt_eff, 0, PTR("stardust"), 4, LW(0), LW(1), LW(2), LW(5), LW(6), 30, 60, 0, 0, 0, 0)
    END_BROTHER()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_VIVIAN_MAGIC1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PTR_A3_1"))
    USER_FUNC(btlevtcmd_ftof, 20, LW(0))
    WAIT_FRM(20)
    USER_FUNC(btlevtcmd_ftof, 26, LW(0))
    USER_FUNC(btlevtcmd_ftof, 46, LW(1))
    USER_FUNC(btlevtcmd_ACRStart, -2, LW(0), LW(1), LW(1), 0)
    USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
    SWITCH(LW(6))
        CASE_LARGE_EQUAL(2)
            USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, 0, 0, 0)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PTR_Y_1"))
        CASE_ETC()
            USER_FUNC(evt_audience_acrobat_notry)
    END_SWITCH()
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_posoffset, 0, 0, 0, 0)
    WAIT_FRM(40)
    SET(LW(10), 0)
    LBL(10)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(evt_eff, 0, PTR("kemuri_test"), 6, LW(0), LW(1), LW(2), FLOAT(1.0), 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(btlevtcmd_CommandPreCheckDamage, -2, LW(3), LW(4), 256, LW(6))
    IF_NOT_EQUAL(LW(6), 1)
        IF_EQUAL(LW(6), 3)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 38)
        END_IF()
        IF_EQUAL(LW(6), 6)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 39)
        END_IF()
        IF_EQUAL(LW(6), 2)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 40)
        END_IF()
        GOTO(50)
    END_IF()
    USER_FUNC(btlevtcmd_AcGetOutputParam, 0, LW(0))
    IF_SMALL_EQUAL(LW(0), 3)
        USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 131328, LW(5))
    ELSE()
        USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 256, LW(5))
    END_IF()
    IF_EQUAL(LW(5), 18)
        USER_FUNC(btlevtcmd_AcGetOutputParam, 0, LW(0))
        SWITCH(LW(0))
            CASE_SMALL_EQUAL(0)
                SET(LW(0), -1)
            CASE_OR(1)
            CASE_OR(2)
                SET(LW(0), 0)
                CASE_END()
            CASE_OR(3)
            CASE_OR(4)
                SET(LW(0), 1)
                CASE_END()
            CASE_ETC()
                SET(LW(0), 2)
        END_SWITCH()
        IF_LARGE_EQUAL(LW(0), 0)
            USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), LW(0), LW(6))
            USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), LW(6))
        ELSE()
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), -1)
        END_IF()
    END_IF()
    LBL(50)
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    IF_NOT_EQUAL(LW(3), -1)
        ADD(LW(10), 1)
        GOTO(10)
    END_IF()
    LBL(80)
    USER_FUNC(evt_audience_ap_recovery)
    USER_FUNC(btlevtcmd_InviteApInfoReport)
    LBL(90)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_posoffset, 0, 0, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PTR_A1_1"))
    WAIT_MSEC(800)
    LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RUN_CHILD_EVT(PTR(&btldefaultevt_SuitoruBadgeEffect))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(partyVivianAttack_CharmKissAttack)
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_CommandPayWeaponCost, -2)
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    WAIT_FRM(20)
    SET(LW(10), 0)
    LBL(10)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PTR_A4_1"))
    USER_FUNC(evt_snd_sfxon, PTR("SFX_PARTY_MOJIMOJI1"), 0)
    USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
    USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
    USER_FUNC(btlevtcmd_AcGetDifficulty, LW(0))
    SUB(LW(0), 3)
    SWITCH(LW(0))
        CASE_SMALL_EQUAL(-3)
            SET(LW(5), 60)
        CASE_EQUAL(-2)
            SET(LW(5), 45)
        CASE_EQUAL(-1)
            SET(LW(5), 30)
        CASE_EQUAL(0)
            SET(LW(5), 25)
        CASE_EQUAL(1)
            SET(LW(5), 20)
        CASE_EQUAL(2)
            SET(LW(5), 12)
        CASE_ETC()
            SET(LW(5), 6)
    END_SWITCH()
    USER_FUNC(evt_sub_random, LW(1), 89)
    ADD(LW(1), 30)
    SET(LW(2), LW(1))
    SUB(LW(2), LW(5))
    USER_FUNC(btlevtcmd_AcSetParamAll, 5, LW(1), LW(2), LW(1), EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR)
    IF_LARGE(LW(10), 0)
        USER_FUNC(btlevtcmd_AcSetFlag, 9)
    ELSE()
        USER_FUNC(btlevtcmd_AcSetFlag, 8)
    END_IF()
    USER_FUNC(btlevtcmd_SetupAC, -2, 11, 1, 0)
    WAIT_FRM(22)
    IF_LARGE(LW(10), 0)
        USER_FUNC(btlevtcmd_StartAC, 0)
    ELSE()
        USER_FUNC(btlevtcmd_StartAC, 1)
    END_IF()
    BROTHER_EVT_ID(LW(15))
        SET(LW(0), 5)
        DO(LW(0))
            USER_FUNC(btlevtcmd_AcGetOutputParam, 2, LW(1))
            IF_NOT_EQUAL(LW(1), 0)
                USER_FUNC(_disp_heart_entry, 10, 20, 25, LW(3), LW(4))
                WAIT_FRM(1)
                DO_BREAK()
            END_IF()
            USER_FUNC(_disp_heart_entry_stop_check, LW(1))
            IF_NOT_EQUAL(LW(1), 0)
                DO_BREAK()
            END_IF()
            SET(LW(0), 5)
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_ResultAC)
    USER_FUNC(btlevtcmd_GetResultAC, LW(7))
    CHK_EVT(LW(15), LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(_disp_heart_entry_stop, LW(15))
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PTR_A4_2"))
    WAIT_MSEC(200)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_VIVIAN_MEROMERO1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 15)
    ADD(LW(1), 30)
    USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(6))
    MUL(LW(6), 45)
    USER_FUNC(evt_eff, 0, PTR("kiss"), 4, LW(0), LW(1), LW(2), LW(6), 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(btlevtcmd_CommandPreCheckDamage, -2, LW(3), LW(4), 256, LW(6))
    IF_NOT_EQUAL(LW(6), 1)
        IF_EQUAL(LW(6), 3)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 38)
        END_IF()
        IF_EQUAL(LW(6), 6)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 39)
        END_IF()
        IF_EQUAL(LW(6), 2)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 40)
        END_IF()
        GOTO(50)
    END_IF()
    IF_FLAG(LW(7), 0x2)
        USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 131328, LW(5))
    ELSE()
        USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 536871168, LW(5))
    END_IF()
    IF_EQUAL(LW(5), 18)
        IF_FLAG(LW(7), 0x2)
            USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
            ADD(LW(2), 10)
            USER_FUNC(evt_eff, 0, PTR("kiss"), 5, LW(0), LW(1), LW(2), 0, 0, 0, 0, 0, 0, 0, 0)
            USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), 0, LW(6))
            USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), LW(6))
        ELSE()
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), -1)
        END_IF()
    END_IF()
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_ACRStart, -2, 0, 30, 30, 0)
    END_BROTHER()
    USER_FUNC(btlevtcmd_AnimeWaitPlayComplete, -2, 1)
    USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
    IF_EQUAL(LW(6), 2)
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
        USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(0), 1, 0, 0, 0)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PTR_Y_1"))
        WAIT_FRM(32)
    END_IF()
    WAIT_FRM(5)
    LBL(50)
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    IF_NOT_EQUAL(LW(3), -1)
        ADD(LW(10), 1)
        GOTO(10)
    END_IF()
    LBL(80)
    USER_FUNC(btlevtcmd_StopAC)
    USER_FUNC(evt_audience_ap_recovery)
    USER_FUNC(btlevtcmd_InviteApInfoReport)
    LBL(90)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PTR_A1_1"))
    WAIT_FRM(48)
    LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RUN_CHILD_EVT(PTR(&btldefaultevt_SuitoruBadgeEffect))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

BattleWeapon customWeapon_VivianShadeFist = {
    .name = "btl_wn_ptr_normal",
    .icon = IconType::PARTNER_MOVE_0,
    .item_id = 0,
    .description = "msg_ptr_kagenuke",
    .base_accuracy = 100,
    .base_fp_cost = 0,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 1,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)weaponGetPowerFromPartyAttackLv,
    .damage_function_params = { 3, 3, 4, 4, 5, 5, 0, 0 },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags =
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_kagenuke",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::USABLE_IF_CONFUSED |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::TOP_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    .burn_chance = 100,
    .burn_time = 2,
    
    .attack_evt_code = (void*)partyVivianAttack_NormalAttack,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 4,
    .nozzle_fire_chance = 2,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

BattleWeapon customWeapon_VivianVeil = {
    .name = "btl_wn_ptr_lv1",
    .icon = IconType::PARTNER_MOVE_1,
    .item_id = 0,
    .description = "msg_ptr_kagegakure",
    .base_accuracy = 100,
    .base_fp_cost = 1,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = nullptr,
    .damage_function_params = { 0, 2, 2, 3, 3, 4, 0, 0 },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags =
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_OPPOSING_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_SAME_ALLIANCE_DIR |
        AttackTargetProperty_Flags::UNKNOWN_0x2,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_kagegakure",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::CANNOT_MISS,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)partyVivianAttack_ShadowGuard,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 0,
    .nozzle_fire_chance = 0,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

BattleWeapon customWeapon_VivianFieryJinx = {
    .name = "btl_wn_ptr_lv2",
    .icon = IconType::PARTNER_MOVE_2,
    .item_id = 0,
    .description = "msg_ptr_mahou_no_kona",
    .base_accuracy = 100,
    .base_fp_cost = 6,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)weaponGetACOutputParam,
    .damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags =
        AttackTargetClass_Flags::MULTIPLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::FIRE,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_mahou_no_kona",
    .special_property_flags = 
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000,
        
    // status chances
    .burn_chance = 100,
    .burn_time = 3,
    
    .attack_evt_code = (void*)partyVivianAttack_MagicalPowder,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 10,
    .nozzle_fire_chance = 5,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

BattleWeapon customWeapon_VivianInfatuate = {
    .name = "btl_wn_ptr_lv3",
    .icon = IconType::PARTNER_MOVE_3,
    .item_id = 0,
    .description = "msg_ptr_meromero_kiss",
    .base_accuracy = 100,
    .base_fp_cost = 4,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = nullptr,
    .damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags =
        AttackTargetClass_Flags::MULTIPLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_meromero_kiss",
    .special_property_flags = AttackSpecialProperty_Flags::UNGUARDABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    .confuse_chance = 100,
    .confuse_time = 3,
    
    .attack_evt_code = (void*)partyVivianAttack_CharmKissAttack,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 10,
    .nozzle_fire_chance = 5,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};
BattleWeapon customWeapon_VivianMove5 = {
    .name = "btl_wn_ptr_normal",
    .icon = IconType::PARTNER_MOVE_0,
    .item_id = 0,
    .description = "msg_ptr_kagenuke",
    .base_accuracy = 100,
    .base_fp_cost = 0,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 1,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)weaponGetPowerFromPartyAttackLv,
    .damage_function_params = { 3, 3, 4, 4, 5, 5, 0, 0 },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags =
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_kagenuke",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::USABLE_IF_CONFUSED |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::TOP_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    .burn_chance = 100,
    .burn_time = 2,
    
    .attack_evt_code = (void*)partyVivianAttack_NormalAttack,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 4,
    .nozzle_fire_chance = 2,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

BattleWeapon customWeapon_VivianMove6 = {
    .name = "btl_wn_ptr_normal",
    .icon = IconType::PARTNER_MOVE_0,
    .item_id = 0,
    .description = "msg_ptr_kagenuke",
    .base_accuracy = 100,
    .base_fp_cost = 0,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 1,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)weaponGetPowerFromPartyAttackLv,
    .damage_function_params = { 3, 3, 4, 4, 5, 5, 0, 0 },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags =
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_kagenuke",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::USABLE_IF_CONFUSED |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::TOP_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    .burn_chance = 100,
    .burn_time = 2,
    
    .attack_evt_code = (void*)partyVivianAttack_NormalAttack,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 4,
    .nozzle_fire_chance = 2,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

}  // namespace mod::tot::party_vivian