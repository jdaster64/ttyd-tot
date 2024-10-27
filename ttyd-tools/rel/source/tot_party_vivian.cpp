#include "tot_party_vivian.h"

#include "evt_cmd.h"
#include "patches_battle.h"
#include "tot_manager_move.h"

#include <ttyd/battle.h>
#include <ttyd/battle_camera.h>
#include <ttyd/battle_damage.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_event_default.h>
#include <ttyd/battle_message.h>
#include <ttyd/battle_sub.h>
#include <ttyd/battle_unit.h>
#include <ttyd/battle_weapon_power.h>
#include <ttyd/evt_audience.h>
#include <ttyd/evt_eff.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evt_sub.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/icondrv.h>
#include <ttyd/msgdrv.h>
#include <ttyd/unit_party_christine.h>
#include <ttyd/unit_party_vivian.h>

#include <cstring>

namespace mod::tot::party_vivian {

namespace {
    
// Including entire namespaces for convenience.
using namespace ::ttyd::battle_camera;
using namespace ::ttyd::battle_database_common;
using namespace ::ttyd::battle_event_cmd;
using namespace ::ttyd::battle_event_default;
using namespace ::ttyd::battle_message;
using namespace ::ttyd::battle_unit;
using namespace ::ttyd::battle_weapon_power;
using namespace ::ttyd::evt_audience;
using namespace ::ttyd::evt_eff;
using namespace ::ttyd::evt_snd;
using namespace ::ttyd::evt_sub;
using namespace ::ttyd::unit_party_vivian;

using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

namespace BattleUnitType = ::ttyd::battle_database_common::BattleUnitType;
namespace IconType = ::ttyd::icondrv::IconType;
namespace StatusEffectType = ::ttyd::battle_database_common::StatusEffectType;

}  // namespace

// Declaration of weapon structs.
extern BattleWeapon customWeapon_VivianShadeFist;
extern BattleWeapon customWeapon_VivianVeil;
extern BattleWeapon customWeapon_VivianFieryJinx;
extern BattleWeapon customWeapon_VivianInfatuate;
extern BattleWeapon customWeapon_VivianCurse;
extern BattleWeapon customWeapon_VivianNeutralize;

BattleWeapon* g_WeaponTable[] = {
    &customWeapon_VivianShadeFist, &customWeapon_VivianCurse,
    &customWeapon_VivianNeutralize, &customWeapon_VivianVeil,
    &customWeapon_VivianFieryJinx, &customWeapon_VivianInfatuate
};

void MakeSelectWeaponTable(
    ttyd::battle::BattleWorkCommand* command_work, int32_t* num_options) {
    for (int32_t i = 0; i < 6; ++i) {
        auto& weapon_entry = command_work->weapon_table[*num_options];
        BattleWeapon* weapon = g_WeaponTable[i];
        
        if (MoveManager::GetUnlockedLevel(MoveType::VIVIAN_BASE + i)) {
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
}

// Sets turn count of Neutralize based on number of successful presses.
EVT_DECLARE_USER_FUNC(evtTot_SetNeutralizeTurnCount, 2)
EVT_DEFINE_USER_FUNC(evtTot_SetNeutralizeTurnCount) {
    int32_t buttons_pressed = evtGetValue(evt, evt->evtArguments[0]);
    int32_t level = evtGetValue(evt, evt->evtArguments[1]);
    customWeapon_VivianNeutralize.allergic_time =
        buttons_pressed == 1 ? 1 : buttons_pressed / level;
    return 2;
}

// If Infatuate lands successfully, changes the target's alliance permanently.
EVT_DECLARE_USER_FUNC(evtTot_InfatuateChangeAlliance, 2)
EVT_DEFINE_USER_FUNC(evtTot_InfatuateChangeAlliance) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[0]));
    int32_t part_idx = evtGetValue(evt, evt->evtArguments[1]);
    auto* battleWork = ttyd::battle::g_BattleWork;
    auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, unit_idx);
    auto* part = ttyd::battle::BattleGetUnitPartsPtr(unit_idx, part_idx);
    
    // If not a boss enemy or Yux, undo Confusion status and change alliance.
    switch (unit->current_kind) {
        case BattleUnitType::HOOKTAIL:
        case BattleUnitType::GLOOMTAIL:
        case BattleUnitType::BONETAIL:
        case BattleUnitType::ATOMIC_BOO:
        case BattleUnitType::TOT_COSMIC_BOO:
        case BattleUnitType::YUX:
        case BattleUnitType::Z_YUX:
        case BattleUnitType::X_YUX:
        case BattleUnitType::MINI_YUX:
        case BattleUnitType::MINI_Z_YUX:
        case BattleUnitType::MINI_X_YUX:
            break;
            
        default: {
            // Infatuate should not work on midbosses.
            if (unit->status_flags & BattleUnitStatus_Flags::MIDBOSS) break;
            
            uint32_t dummy = 0;
            ttyd::battle_damage::BattleSetStatusDamage(
                &dummy, unit, part, 0x100 /* ignore status vulnerability */,
                StatusEffectType::CONFUSE, 100, 0, 0, 0);
            unit->alliance = 0;
            
            // Unqueue the status message for inflicting confusion.
            static constexpr const uint32_t kNoStatusMsg[] = {
                0xff000000U, 0, 0
            };
            memcpy(
                reinterpret_cast<void*>(
                    reinterpret_cast<uintptr_t>(battleWork) + 0x18ddc),
                kNoStatusMsg, sizeof(kNoStatusMsg));
            memcpy(
                reinterpret_cast<void*>(
                    reinterpret_cast<uintptr_t>(unit) + 0xae8),
                kNoStatusMsg, sizeof(kNoStatusMsg));
                
            // Queue a custom message instead.
            patch::battle::QueueCustomStatusMessage(
                unit, "tot_ptr5_infatuate_effect_msg");
        }
    }
    
    return 2;
}

EVT_BEGIN(partyVivianAttack_NormalAttack)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.0), FLOAT(0.70))
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    USER_FUNC(_make_kagenuke_weapon, LW(12), 2)
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
        USER_FUNC(evtTot_LogActiveMoveStylish, 0)
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

    // Also set Unit Work var 5 to contain the level of the move.
    USER_FUNC(evtTot_GetMoveSelectedLevel, MoveType::VIVIAN_VEIL, LW(0))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 5, LW(0))

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
    // Number of frames per button.
    SWITCH(LW(0))
        CASE_SMALL_EQUAL(-3)
            SET(LW(0), 72)
        CASE_EQUAL(-2)
            SET(LW(0), 54)
        CASE_EQUAL(-1)
            SET(LW(0), 42)
        CASE_EQUAL(0)
            SET(LW(0), 36)
        CASE_EQUAL(1)
            SET(LW(0), 32)
        CASE_EQUAL(2)
            SET(LW(0), 28)
        CASE_ETC()
            SET(LW(0), 24)
    END_SWITCH()

    IF_EQUAL(LW(12), PTR(&customWeapon_VivianFieryJinx))
        USER_FUNC(evtTot_GetMoveSelectedLevel, MoveType::VIVIAN_FIERY_JINX, LW(1))
        // Number of buttons (3, 5, 7 at level 1, 2, 3).
        MUL(LW(1), 2)
        ADD(LW(1), 1)
        MUL(LW(0), LW(1))
        // Random ABXY command.
        USER_FUNC(btlevtcmd_AcSetParamAll, LW(0), 1, LW(1), -3, 1, EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR)
    ELSE()
        USER_FUNC(evtTot_GetMoveSelectedLevel, MoveType::VIVIAN_NEUTRALIZE, LW(1))
        // Number of buttons (3, 6 at level 1, 2).
        MUL(LW(1), 3)
        MUL(LW(0), LW(1))
        // Shuffle ABXYLRZ command.
        USER_FUNC(btlevtcmd_AcSetParamAll, LW(0), 1, LW(1), -5, 1, EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR)
    END_IF()

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
            USER_FUNC(evtTot_LogActiveMoveStylish, 0)
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
    IF_EQUAL(LW(12), PTR(&customWeapon_VivianFieryJinx))
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
        USER_FUNC(evt_eff, 0, PTR("kemuri_test"), 6, LW(0), LW(1), LW(2), FLOAT(1.0), 0, 0, 0, 0, 0, 0, 0)
    END_IF()
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

    IF_EQUAL(LW(12), PTR(&customWeapon_VivianNeutralize))
        USER_FUNC(evtTot_GetMoveSelectedLevel, MoveType::VIVIAN_NEUTRALIZE, LW(1))
        USER_FUNC(evtTot_SetNeutralizeTurnCount, LW(0), LW(1))
    END_IF()

    // Delay self-targeting by a couple frames to make sure it happens after
    // hitting all other targets, SP regeneration, etc.
    USER_FUNC(btlevtcmd_GetUnitId, -4, LW(14))
    USER_FUNC(btlevtcmd_GetUnitId, LW(3), LW(15))
    IF_EQUAL(LW(14), LW(15))
        // Targeting self.
        SET(LW(5), 18)
        BROTHER_EVT()
            WAIT_FRM(2)
            IF_SMALL_EQUAL(LW(0), 3)
                USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 131328, LW(5))
            ELSE()
                USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 256, LW(5))
            END_IF()
        END_BROTHER()
    ELSE()
        IF_SMALL_EQUAL(LW(0), 3)
            USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 131328, LW(5))
        ELSE()
            USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 256, LW(5))
        END_IF()
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

    // If level 1 Neutralize, skip all other targets.
    IF_EQUAL(LW(12), PTR(&customWeapon_VivianNeutralize))
        USER_FUNC(evtTot_GetMoveSelectedLevel, MoveType::VIVIAN_NEUTRALIZE, LW(0))
        IF_EQUAL(LW(0), 1)
            GOTO(80)
        END_IF()
    END_IF()

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
            USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), 0, LW(6))
            USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
            IF_EQUAL(LW(12), PTR(&customWeapon_VivianInfatuate))
                // Only show heart effects for Infatuate.
                USER_FUNC(evt_eff, 0, PTR("kiss"), 5, LW(0), LW(1), LW(2), 0, 0, 0, 0, 0, 0, 0, 0)
                // Apply Infatuate effect.
                USER_FUNC(evtTot_InfatuateChangeAlliance, LW(3), LW(4))
            END_IF()
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
        USER_FUNC(evtTot_LogActiveMoveStylish, 0)
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
        USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(0), 1, 0, 0, 0)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PTR_Y_1"))
        WAIT_FRM(32)
    END_IF()
    WAIT_FRM(5)
    LBL(50)
    
    SWITCH(LW(12))
        CASE_EQUAL(PTR(&customWeapon_VivianInfatuate))
            SET(LW(0), 1)
        CASE_EQUAL(PTR(&customWeapon_VivianCurse))
            USER_FUNC(evtTot_GetMoveSelectedLevel, MoveType::VIVIAN_CURSE, LW(0))
    END_SWITCH()
    // If Infatuate, or level 1 version of Curse, don't check next.
    IF_LARGE(LW(0), 1)
        USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
        IF_NOT_EQUAL(LW(3), -1)
           ADD(LW(10), 1)
           GOTO(10)
        END_IF()
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

EVT_BEGIN(vivian_hide_event)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, 4, LW(0))
    IF_LARGE_EQUAL(LW(0), 1)
        SUB(LW(0), 1)
    END_IF()
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 4, LW(0))
    IF_SMALL_EQUAL(LW(0), 0)
        SET(LW(3), -3)
        USER_FUNC(btlevtcmd_PhaseEventStartDeclare, LW(3))
        USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_FaceDirectionSub, LW(3), LW(0), 15)
        USER_FUNC(btlevtcmd_SetPos, -2, LW(0), LW(1), LW(2))
        INLINE_EVT()
            USER_FUNC(btlevtcmd_OffAttribute, LW(3), 16)
            USER_FUNC(btlevtcmd_OffPartsAttribute, LW(3), 1, 50331648)
            USER_FUNC(battle_evt_majo_disp_on, LW(3), 1, LW(0), LW(1), LW(2), 1)
            USER_FUNC(btlevtcmd_SetPos, LW(3), LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_GetHomePos, LW(3), LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_SetMoveSpeed, LW(3), FLOAT(2.0))
            USER_FUNC(btlevtcmd_MovePosition, LW(3), LW(0), LW(1), LW(2), 0, -1, 0)
        END_INLINE()
        USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, 65536)
        USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, 16777216)
        USER_FUNC(battle_evt_majo_disp_on, -2, 1, LW(0), LW(1), LW(2), 1)
        USER_FUNC(btlevtcmd_SetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetMoveFrame, -2, LW(0), LW(1), LW(2), FLOAT(2.0), LW(15))
        BROTHER_EVT_ID(LW(14))
            USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), LW(15), -1, 0)
        END_BROTHER()
        USER_FUNC(btlevtcmd_ACRStart, -2, 0, LW(15), LW(15), 15)
        
        // If level 2 of the move was used, also apply hustle status to Mario.
        USER_FUNC(btlevtcmd_GetUnitWork, -2, 5, LW(0))
        IF_EQUAL(LW(0), 2)
            BROTHER_EVT_ID(LW(13))
                USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
                ADD(LW(2), 15)
                USER_FUNC(btlevtcmd_GetHeight, LW(3), LW(11))
                USER_FUNC(btlevtcmd_GetStatusMg, LW(3), LW(12))
                MULF(LW(11), LW(12))
                DIV(LW(11), 2)
                ADD(LW(1), LW(11))
                USER_FUNC(evt_eff, 0, PTR("recovery"), 6, LW(0), LW(1), LW(2), 0, 0, 0, 0, 0, 0, 0, 0)
                USER_FUNC(evt_snd_sfxon_3d, PTR("SFX_BTL_KURI_CHEER_KISS1"), LW(0), LW(1), LW(2), 0)
                USER_FUNC(ttyd::unit_party_christine::_set_hustle, LW(3))
                WAIT_FRM(40)
                USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(3), 1, 58)
                USER_FUNC(btlevtcmd_AnnounceMessage, 0, 0, 0, PTR("msg_st_chg_mario_quick"), 60)
            END_BROTHER()
        ELSE()
            SET(LW(13), -1)
        END_IF()

        USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
        SWITCH(LW(6))
            CASE_LARGE_EQUAL(2)
                DELETE_EVT(LW(14))
                // Special case for Veil.
                USER_FUNC(evtTot_LogActiveMoveStylish, -1)
                USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, PTR(&partyWeapon_VivianShadowGuard), 1, 0, 0, 0)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PTR_Y_1"))
                USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(2.0))
                USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
                USER_FUNC(btlevtcmd_AnimeWaitPlayComplete, -2, 1)
            CASE_ETC()
                USER_FUNC(evt_audience_acrobat_notry)
                USER_FUNC(btlevtcmd_WaitEventEnd, LW(14))
        END_SWITCH()
        IF_NOT_EQUAL(LW(13), -1)
            USER_FUNC(btlevtcmd_WaitEventEnd, LW(13))
        END_IF()
        USER_FUNC(evt_audience_ap_recovery)
        USER_FUNC(btlevtcmd_InviteApInfoReport)
        USER_FUNC(btlevtcmd_StartWaitEvent, LW(3))
        USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    END_IF()
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
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 3, 3, 5, 5, 7, 7, 0, MoveType::VIVIAN_BASE },
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
    .icon = IconType::PARTNER_MOVE_2,
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
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH |
        // Explicitly add this to exclude Infatuated targets.
        AttackTargetClass_Flags::ONLY_TARGET_MARIO,
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
        // Single-target instead of multiple.
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
    .ac_help_msg = "msg_ac_meromero_kiss",
    .special_property_flags = AttackSpecialProperty_Flags::UNGUARDABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    .confuse_chance = 127,
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

BattleWeapon customWeapon_VivianCurse = {
    .name = "tot_ptr5_curse",
    .icon = IconType::PARTNER_MOVE_1,
    .item_id = 0,
    .description = "tot_ptr5_curse_desc",
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
        // Single-target or multiple depending on level.
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
    .ac_help_msg = "msg_ac_meromero_kiss",
    .special_property_flags = AttackSpecialProperty_Flags::UNGUARDABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    .slow_chance = 127,
    .slow_time = 3,
    
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

BattleWeapon customWeapon_VivianNeutralize = {
    .name = "tot_ptr5_neutralize",
    .icon = IconType::PARTNER_MOVE_1,
    .item_id = 0,
    .description = "tot_ptr5_neutralize_desc",
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
        // Single-target or multiple (selectable side) depending on level.
        // In any case, can target enemy _or_ player actors, including herself.
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_mahou_no_kona",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::CANNOT_MISS,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    .allergic_chance = 127,
    .allergic_time = 3,
    
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

void* GetVivianUnhideEvt() {
    return (void*)vivian_hide_event;
}

}  // namespace mod::tot::party_vivian