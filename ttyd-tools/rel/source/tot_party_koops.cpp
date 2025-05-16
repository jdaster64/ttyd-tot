#include "tot_party_koops.h"

#include "evt_cmd.h"
#include "patch.h"
#include "tot_gsw.h"
#include "tot_manager_move.h"

#include <ttyd/ac_pendulum_crane_timing.h>
#include <ttyd/battle.h>
#include <ttyd/battle_camera.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_event_default.h>
#include <ttyd/battle_message.h>
#include <ttyd/battle_sub.h>
#include <ttyd/battle_unit.h>
#include <ttyd/battle_weapon_power.h>
#include <ttyd/evt_audience.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evt_sub.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/icondrv.h>
#include <ttyd/msgdrv.h>
#include <ttyd/unit_party_nokotarou.h>

namespace mod::tot::party_koops {

namespace {
    
// Including entire namespaces for convenience.
using namespace ::ttyd::battle_camera;
using namespace ::ttyd::battle_database_common;
using namespace ::ttyd::battle_event_cmd;
using namespace ::ttyd::battle_event_default;
using namespace ::ttyd::battle_message;
using namespace ::ttyd::battle_weapon_power;
using namespace ::ttyd::evt_audience;
using namespace ::ttyd::evt_snd;
using namespace ::ttyd::evt_sub;
using namespace ::ttyd::unit_party_nokotarou;

using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

namespace IconType = ::ttyd::icondrv::IconType;

}  // namespace

// Declaration of weapon structs.
extern BattleWeapon customWeapon_KoopsShellTossFS;
extern BattleWeapon customWeapon_KoopsShellToss;
extern BattleWeapon customWeapon_KoopsPowerShell;
extern BattleWeapon customWeapon_KoopsShellShield;
extern BattleWeapon customWeapon_KoopsShellSlam;
extern BattleWeapon customWeapon_KoopsBulkUp;
extern BattleWeapon customWeapon_KoopsWithdraw;

BattleWeapon* g_WeaponTable[] = {
    &customWeapon_KoopsShellToss, &customWeapon_KoopsPowerShell, 
    &customWeapon_KoopsWithdraw, &customWeapon_KoopsShellShield,
    &customWeapon_KoopsBulkUp, &customWeapon_KoopsShellSlam
};

void MakeSelectWeaponTable(
    ttyd::battle::BattleWorkCommand* command_work, int32_t* num_options) {
    for (int32_t i = 0; i < 6; ++i) {
        auto& weapon_entry = command_work->weapon_table[*num_options];
        BattleWeapon* weapon = g_WeaponTable[i];
        
        if (MoveManager::GetUnlockedLevel(MoveType::KOOPS_BASE + i)) {
            weapon_entry.index = MoveType::KOOPS_BASE + i;
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

BattleWeapon* GetFirstAttackWeapon() {
    return &customWeapon_KoopsShellTossFS;
}

// Sets the gauge colors for Shell Shield / Withdraw to show success window.
EVT_DECLARE_USER_FUNC(evtTot_SetPendulumAcParams, 1)
EVT_DEFINE_USER_FUNC(evtTot_SetPendulumAcParams) {
    static constexpr int16_t g_ShellShieldGaugeParams[] = {
        20, 2, 35, 4, 45, 6, 55, 8, 65, 6, 80, 4, 100, 2
    };
    static constexpr int16_t g_WithdrawGaugeParams[] = {
        35, 2, 36, 4, 65, 6, 65, 8, 65, 6, 66, 4, 100, 2
    };

    const int32_t move_type = evtGetValue(evt, evt->evtArguments[0]);
    if (move_type == MoveType::KOOPS_SHELL_SHIELD) {
        mod::writePatch(
            ttyd::ac_pendulum_crane_timing::pendulumCrane_hp_tbl,
            g_ShellShieldGaugeParams, sizeof(g_ShellShieldGaugeParams));
    } else {
        mod::writePatch(
            ttyd::ac_pendulum_crane_timing::pendulumCrane_hp_tbl,
            g_WithdrawGaugeParams, sizeof(g_WithdrawGaugeParams));
    }
    
    return 2;
}

// Sets the status parameters for Bulk Up based on the AC success + move level.
EVT_DECLARE_USER_FUNC(evtTot_MakeBulkUpWeapon, 4)
EVT_DEFINE_USER_FUNC(evtTot_MakeBulkUpWeapon) {
    auto* weapon = (BattleWeapon*)evtGetValue(evt, evt->evtArguments[0]);
    int32_t ac_result = evtGetValue(evt, evt->evtArguments[1]);
    int32_t move_level = evtGetValue(evt, evt->evtArguments[2]);
    int32_t index = evtGetValue(evt, evt->evtArguments[3]);
    
    // Make changes in place.
    weapon->atk_change_time = ac_result;
    weapon->atk_change_strength = move_level + 1;
    weapon->def_change_time = ac_result;
    weapon->def_change_strength = move_level + 1;
    // Apply statuses one at a time.
    if (index == 0) {
        weapon->atk_change_chance = 100;
        weapon->def_change_chance = 0;
    } else {
        weapon->atk_change_chance = 0;
        weapon->def_change_chance = 100;
    }
    
    return 2;
}

// Sets the initial HP for Shell Shield based on the AC result + move level.
EVT_DECLARE_USER_FUNC(evtTot_ShellShieldSetInitialHp, 2)
EVT_DEFINE_USER_FUNC(evtTot_ShellShieldSetInitialHp) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[0]));
    int32_t ac_level = evtGetValue(evt, evt->evtArguments[1]);
    int32_t starting_hp = 0;
    int32_t max_hp = 0;
    
    // Action command rating ranges from 0 - 8.
    switch (MoveManager::GetSelectedLevel(MoveType::KOOPS_SHELL_SHIELD)) {
        case 3:
            starting_hp = ac_level / 2;     // Nice = 2, Good = 3, Super = 4
            max_hp = 4;
            break;
        case 2:
            starting_hp = ac_level / 2 - 1; // Nice = 1, Good = 2, Super = 3
            max_hp = 3;
            break;
        default:
            starting_hp = ac_level / 3;     // Nice = 1, Good = 2, Super = 2
            max_hp = 2;
    }
    if (starting_hp < 1) starting_hp = 1;
    
    auto* battleWork = ttyd::battle::g_BattleWork;
    auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, unit_idx);
    
    // Set unit's maximum HP.
    unit->max_hp = max_hp;
    // Set unit's current HP.
    evtSetValue(evt, evt->evtArguments[1], starting_hp);
    btlevtcmd_SetHp(evt, isFirstCall);
    
    // Reset variable 1 to the action command level before exiting
    // (since it also controls the visuals of the shell's cracks).
    evtSetValue(evt, evt->evtArguments[1], ac_level);
    return 2;
}

EVT_BEGIN(partyNokotarouAttack_FirstAttack)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.0), FLOAT(0.70))
    USER_FUNC(btlevtcmd_GetFirstAttackTarget, LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_WeaponAftereffect, PTR(&customWeapon_KoopsShellTossFS))
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, LW(3), -1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 300)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    SET(LW(0), 45)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 1, 45)
    RUN_CHILD_EVT(PTR(&_koura_rotate_start))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 250)
    USER_FUNC(btlevtcmd_SetPos, -2, LW(0), LW(1), LW(2))
    WAIT_FRM(50)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 50)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(5), LW(2))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(6.0))
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_NOKO_ATTACK2"), 0)
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, 0, 0)
    RUN_CHILD_EVT(PTR(&_koura_rotate_stop))
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), PTR(&customWeapon_KoopsShellTossFS))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&customWeapon_KoopsShellTossFS), 131328, LW(5))
    SET(LW(0), 45)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 1, 45)
    RUN_CHILD_EVT(PTR(&_koura_rotate_start))
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.70))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    SET(LW(1), 0)
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 30)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 18, -1)
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.60))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 20)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 8)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 8, -1)
    RUN_CHILD_EVT(PTR(&_koura_rotate_stop))
    RUN_CHILD_EVT(PTR(&_restore_koura_pose))
    USER_FUNC(evt_audience_ap_recovery)
    USER_FUNC(btlevtcmd_InviteApInfoReport)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(6.0))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RUN_CHILD_EVT(PTR(&btldefaultevt_SuitoruBadgeEffect))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(partyNokotarouAttack_NormalAttack)
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    ELSE()
        USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
        USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(12))
        USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(12), LW(3), LW(4))
    END_IF()
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_CommandPayWeaponCost, -2)
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, 1)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    USER_FUNC(evt_btl_camera_set_posoffset, 0, 20, 0, 0)
    WAIT_FRM(20)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    RUN_CHILD_EVT(PTR(&_change_koura_pose))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 50)
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_NOKO_ATTACK1"), LW(13))
    SET(LW(0), 15)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 1, 45)
    RUN_CHILD_EVT(PTR(&_koura_rotate_start))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
        USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
        USER_FUNC(btlevtcmd_AcGetDifficulty, LW(0))
        SUB(LW(0), 3)
        SWITCH(LW(0))
            CASE_SMALL_EQUAL(-3)
                SET(LW(0), 360)
                SET(LW(1), 100)
            CASE_EQUAL(-2)
                SET(LW(0), 280)
                SET(LW(1), 150)
            CASE_EQUAL(-1)
                SET(LW(0), 240)
                SET(LW(1), 200)
            CASE_EQUAL(0)
                SET(LW(0), 200)
                SET(LW(1), 250)
            CASE_EQUAL(1)
                SET(LW(0), 200)
                SET(LW(1), 360)
            CASE_EQUAL(2)
                SET(LW(0), 200)
                SET(LW(1), 600)
            CASE_ETC()
                SET(LW(0), 200)
                SET(LW(1), 960)
        END_SWITCH()
        USER_FUNC(btlevtcmd_AcSetParamAll, 2, LW(0), 0, 1, LW(1), EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_AcSetFlag, 3)
        USER_FUNC(btlevtcmd_SetupAC, -2, 19, 1, 60)
        WAIT_FRM(22)
    ELSE()
        SET(LW(0), 100)
    END_IF()

    BROTHER_EVT_ID(LW(15))
        DO(10000)
            IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
                USER_FUNC(btlevtcmd_AcGetOutputParam, 0, LW(0))
            ELSE()
                SET(LW(0), 1)
            END_IF()
            IF_NOT_EQUAL(LW(0), 0)
                DO_BREAK()
            END_IF()
            WAIT_FRM(1)
        WHILE()
        SET(LW(6), 0)
        DO(10000)
            ADD(LW(6), 1)
            IF_LARGE(LW(6), 100)
                SET(LW(6), 100)
            END_IF()
            USER_FUNC(btlevtcmd_SetUnitWork, -2, 5, LW(6))
            USER_FUNC(btlevtcmd_GetUnitWork, -2, 4, LW(0))
            USER_FUNC(btlevtcmd_GetUnitWork, -2, 5, LW(1))
            MUL(LW(0), LW(1))
            DIV(LW(0), 100)
            USER_FUNC(evt_snd_sfx_pit, LW(13), LW(0))
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()


    USER_FUNC(btlevtcmd_CommandPreCheckDamage, -2, LW(3), LW(4), 256, LW(6))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 4)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 400)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_StartAC, 1)
        USER_FUNC(btlevtcmd_ResultAC)
    ELSE()
        WAIT_FRM(80)
    END_IF()
    DELETE_EVT(LW(15))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 5, 0)
    USER_FUNC(evt_snd_sfxoff, LW(13))
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_NOKO_ATTACK2"), 0)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_posoffset, 0, 0, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 50)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(10.0))
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    ADD(LW(2), 5)
    USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetMoveFrame, -2, LW(0), LW(1), LW(2), 0, LW(13))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), LW(13), 0, 0)
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
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            USER_FUNC(btlevtcmd_StopAC)
        END_IF()
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 40)
        USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
        RUN_CHILD_EVT(PTR(&_koura_rotate_stop))
        RUN_CHILD_EVT(PTR(&_restore_koura_pose))
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 43)
        WAIT_FRM(30)
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            USER_FUNC(evt_audience_ap_recovery)
            USER_FUNC(btlevtcmd_InviteApInfoReport)
        END_IF()
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
        USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(6.0))
        USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
        GOTO(99)
    END_IF()
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_CommandPreCheckCounter, -2, LW(3), LW(4), 256, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_SetUnitWork, -2, 1, 0)
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
        RUN_CHILD_EVT(PTR(&_koura_rotate_stop))
    END_IF()
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_GetResultAC, LW(0))
        IF_FLAG(LW(0), 0x2)
            USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 131328, LW(5))
            USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), 0, LW(6))
            USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), LW(6))
        ELSE()
            USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 256, LW(5))
            USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), -1)
        END_IF()
    ELSE()
        USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 131328, LW(5))
    END_IF()
LBL(90)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_StopAC)
    END_IF()
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.70))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 30)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        BROTHER_EVT()
            USER_FUNC(btlevtcmd_ACRStart, -2, 0, 17, 17, 0)
        END_BROTHER()
    END_IF()
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 18, -1)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
    ELSE()
        SET(LW(6), 0)
    END_IF()
    IF_EQUAL(LW(6), 2)
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
        USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(0), 1, 0, 0, 0)
        USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 10, 0)
        DO(9)
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, 20)
            WAIT_FRM(1)
        WHILE()
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.70))
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 180, 0)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PNK_E_2"))
        BROTHER_EVT()
            USER_FUNC(btlevtcmd_SetUnitWork, -2, 1, 45)
            LBL(91)
            USER_FUNC(btlevtcmd_GetUnitWork, -2, 1, LW(0))
            IF_SMALL_EQUAL(LW(0), 0)
                GOTO(92)
            END_IF()
            SUB(LW(0), 1)
            USER_FUNC(btlevtcmd_SetUnitWork, -2, 1, LW(0))
            WAIT_FRM(2)
            GOTO(91)
            LBL(92)
        END_BROTHER()
        USER_FUNC(btlevtcmd_ACRStart, -2, 60, 90, 90, 0)
        USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
        IF_EQUAL(LW(6), 1)
            GOTO(94)
        END_IF()
        IF_NOT_EQUAL(LW(6), 2)
            GOTO(94)
        END_IF()
        USER_FUNC(evtTot_LogActiveMoveStylish, 0)
        RUN_CHILD_EVT(PTR(&_koura_rotate_stop))
        DO(6)
            WAIT_FRM(1)
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, 45, 0)
        WHILE()
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PNK_E_3"))
        DO(6)
            WAIT_FRM(1)
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, 15, 0)
        WHILE()
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
        USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(0), 1, 0, 0, 0)
        WAIT_FRM(60)
        GOTO(95)
        LBL(94)
        RUN_CHILD_EVT(PTR(&_koura_rotate_stop))
        USER_FUNC(evt_audience_acrobat_notry)
        RUN_CHILD_EVT(PTR(&_restore_koura_pose))
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 43)
        WAIT_FRM(30)
        GOTO(95)
    END_IF()
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.60))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 20)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 8)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 8, -1)
    RUN_CHILD_EVT(PTR(&_restore_koura_pose))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 43)
    WAIT_FRM(30)
LBL(95)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(evt_audience_ap_recovery)
        USER_FUNC(btlevtcmd_InviteApInfoReport)
    END_IF()
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(6.0))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RUN_CHILD_EVT(PTR(&btldefaultevt_SuitoruBadgeEffect))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(partyNokotarouAttack_SyubibinKoura)
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    ELSE()
        USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
        USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(12))
        USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(12), LW(3), LW(4))
    END_IF()
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_CommandPayWeaponCost, -2)
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, 1)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    USER_FUNC(evt_btl_camera_set_posoffset, 0, 20, 0, 0)
    WAIT_FRM(20)
    RUN_CHILD_EVT(PTR(&_change_koura_pose))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 50)
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_NOKO_SHUBIBIN1"), LW(13))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 5, 0)
    SET(LW(0), 15)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 1, 45)
    RUN_CHILD_EVT(PTR(&_koura_rotate_start))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
        USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
        USER_FUNC(btlevtcmd_AcGetDifficulty, LW(0))
        SUB(LW(0), 3)
        SWITCH(LW(0))
            CASE_SMALL_EQUAL(-3)
                SET(LW(0), 480)
                SET(LW(1), 240)
            CASE_EQUAL(-2)
                SET(LW(0), 300)
                SET(LW(1), 180)
            CASE_EQUAL(-1)
                SET(LW(0), 240)
                SET(LW(1), 120)
            CASE_EQUAL(0)
                SET(LW(0), 180)
                SET(LW(1), 105)
            CASE_EQUAL(1)
                SET(LW(0), 150)
                SET(LW(1), 90)
            CASE_EQUAL(2)
                SET(LW(0), 120)
                SET(LW(1), 60)
            CASE_ETC()
                SET(LW(0), 100)
                SET(LW(1), 30)
        END_SWITCH()
        USER_FUNC(btlevtcmd_AcSetParamAll, 2, LW(0), 1, 95, LW(1), EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_AcSetFlag, 3)
        USER_FUNC(btlevtcmd_SetupAC, -2, 19, 1, 60)
        WAIT_FRM(22)
    ELSE()
        SET(LW(0), 100)
    END_IF()

    BROTHER_EVT_ID(LW(15))
        DO(10000)
            IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
                USER_FUNC(btlevtcmd_AcGetOutputParam, 0, LW(0))
            ELSE()
                SET(LW(0), 1)
            END_IF()
            IF_NOT_EQUAL(LW(0), 0)
                DO_BREAK()
            END_IF()
            WAIT_FRM(1)
        WHILE()
        SET(LW(6), 0)
        DO(10000)
            ADD(LW(6), 1)
            IF_LARGE(LW(6), 200)
                SET(LW(6), 200)
            END_IF()
            SET(LW(7), LW(6))
            DIV(LW(7), 2)
            USER_FUNC(btlevtcmd_SetUnitWork, -2, 5, LW(7))
            USER_FUNC(btlevtcmd_GetUnitWork, -2, 4, LW(0))
            USER_FUNC(btlevtcmd_GetUnitWork, -2, 5, LW(1))
            MUL(LW(0), LW(1))
            DIV(LW(0), 100)
            USER_FUNC(evt_snd_sfx_pit, LW(13), LW(0))
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()

    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 4)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 400)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_StartAC, 1)
        USER_FUNC(btlevtcmd_ResultAC)
        USER_FUNC(btlevtcmd_StopAC)
    ELSE()
        WAIT_FRM(80)
    END_IF()
    DELETE_EVT(LW(15))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 5, 0)
    USER_FUNC(evt_snd_sfxoff, LW(13))
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_NOKO_SHUBIBIN2"), 0)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_posoffset, 0, 0, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 50)
    SET(LW(10), 0)
LBL(10)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(10.0))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(5), LW(5))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, 0, 0)
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
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 0)
    USER_FUNC(btlevtcmd_CommandPreCheckCounter, -2, LW(3), LW(4), 256, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_SetUnitWork, -2, 1, 0)
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
        RUN_CHILD_EVT(PTR(&_koura_rotate_stop))
    END_IF()
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_GetResultAC, LW(0))
        IF_FLAG(LW(0), 0x2)
            USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 131328, LW(5))
            USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), 0, LW(6))
            USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), LW(6))
        ELSE()
            USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 256, LW(5))
            USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), -1)
        END_IF()
    ELSE()
        USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 131328, LW(5))
    END_IF()
LBL(50)
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    IF_NOT_EQUAL(LW(3), -1)
        ADD(LW(10), 1)
        GOTO(10)
    END_IF()
LBL(80)
    USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
    SET(LW(0), 400)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 1)
        MUL(LW(0), -1)
    END_IF()
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, 0, 0)
LBL(90)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    MUL(LW(0), -1)
    USER_FUNC(btlevtcmd_SetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, 0, 0)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_ACRStart, -2, 0, 9, 9, 0)
        USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
    ELSE()
        SET(LW(6), 0)
    END_IF()
    IF_EQUAL(LW(6), 2)
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
        USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(0), 1, 0, 0, 0)
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
        USER_FUNC(btlevtcmd_SetUnitWork, -2, 1, 12)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.30))
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 30, -1)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PNK_E_1"))
        WAIT_FRM(10)
        USER_FUNC(btlevtcmd_ACRStart, -2, 49, 79, 79, 0)
        USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
        IF_NOT_EQUAL(LW(6), 2)
            RUN_CHILD_EVT(PTR(&_koura_rotate_stop))
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PNK_E_2"))
            DO(6)
                WAIT_FRM(1)
                USER_FUNC(btlevtcmd_AddRotate, -2, 0, 45, 0)
            WHILE()
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PNK_S_1"))
            DO(6)
                WAIT_FRM(1)
                USER_FUNC(btlevtcmd_AddRotate, -2, 0, 15, 0)
            WHILE()
            USER_FUNC(evt_audience_acrobat_notry)
            GOTO(99)
        END_IF()
        USER_FUNC(evtTot_LogActiveMoveStylish, 0)
        RUN_CHILD_EVT(PTR(&_koura_rotate_stop))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PNK_E_2"))
        DO(6)
            WAIT_FRM(1)
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, 45, 0)
        WHILE()
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PNK_E_3"))
        DO(6)
            WAIT_FRM(1)
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, 15, 0)
        WHILE()
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
        USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(0), 1, 0, 0, 0)
        WAIT_FRM(60)
        GOTO(99)
    END_IF()
    RUN_CHILD_EVT(PTR(&_restore_koura_pose))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 43)
    WAIT_FRM(30)
LBL(99)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(evt_audience_ap_recovery)
        USER_FUNC(btlevtcmd_InviteApInfoReport)
    END_IF()
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RUN_CHILD_EVT(PTR(&btldefaultevt_SuitoruBadgeEffect))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(partyNokotarouAttack_KouraGuard)
    // Set gauge color parameters.
    USER_FUNC(evtTot_SetPendulumAcParams, MoveType::KOOPS_SHELL_SHIELD)
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    USER_FUNC(btlevtcmd_CommandPayWeaponCost, -2)
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, -2)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    USER_FUNC(evt_btl_camera_set_posoffset, 0, 20, 0, 0)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(_check_guard_koura, -3, LW(10))
    IF_NOT_EQUAL(LW(10), -1)
        USER_FUNC(btlevtcmd_RunDataEventChild, LW(10), 62)
    END_IF()
    USER_FUNC(_color_lv_set, -3, 0, LW(14))
    USER_FUNC(_color_lv_set, -3, 1, 255)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PNK_A_2"))
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_SpawnUnit, LW(10), PTR(&entry_koura), 0)
    USER_FUNC(btlevtcmd_OnAttribute, LW(10), 16777216)
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    ADD(LW(2), 5)
    ADD(LW(1), 240)
    USER_FUNC(btlevtcmd_SetPos, LW(10), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_OffAttribute, LW(10), 16777216)
    USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
    USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
    USER_FUNC(btlevtcmd_GetBodyId, LW(10), LW(11))
    USER_FUNC(btlevtcmd_AcGetDifficulty, LW(0))
    SUB(LW(0), 3)
    SWITCH(LW(0))
        CASE_SMALL_EQUAL(-3)
            SET(LW(0), 360)
            SET(LW(1), 480)
        CASE_EQUAL(-2)
            SET(LW(0), 180)
            SET(LW(1), 360)
        CASE_EQUAL(-1)
            SET(LW(0), 120)
            SET(LW(1), 300)
        CASE_EQUAL(0)
            SET(LW(0), 90)
            SET(LW(1), 240)
        CASE_EQUAL(1)
            SET(LW(0), 70)
            SET(LW(1), 200)
        CASE_EQUAL(2)
            SET(LW(0), 40)
            SET(LW(1), 120)
        CASE_ETC()
            SET(LW(0), 20)
            SET(LW(1), 80)
    END_SWITCH()
    USER_FUNC(btlevtcmd_AcSetParamAll, 1, LW(10), LW(11), 30, LW(0), LW(1), EVT_NULLPTR, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AcSetFlag, 0)
    USER_FUNC(btlevtcmd_SetupAC, -2, 12, 1, 0)
    USER_FUNC(btlevtcmd_GetPos, LW(10), LW(0), LW(1), LW(2))
    SET(LW(1), 125)
    USER_FUNC(btlevtcmd_DivePosition, LW(10), LW(0), LW(1), LW(2), 60, 0, 13, 0, -1)
    USER_FUNC(btlevtcmd_StartAC, 1)
    USER_FUNC(btlevtcmd_ResultAC)
    USER_FUNC(btlevtcmd_GetResultAC, LW(6))
    IF_EQUAL(0, 0)
        BROTHER_EVT()
            WAIT_FRM(10)
            USER_FUNC(btlevtcmd_AnimeChangePose, -3, 1, PTR("M_N_7"))
            WAIT_FRM(35)
            USER_FUNC(btlevtcmd_AnimeChangePose, -3, 1, PTR("M_B_1"))
            WAIT_FRM(15)
            USER_FUNC(btlevtcmd_AnimeChangePoseType, -3, 1, 43)
        END_BROTHER()
        USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
        USER_FUNC(evt_btl_camera_set_zoom, 0, 400)
        USER_FUNC(evt_btl_camera_set_posoffset, 0, 0, 0, 0)
        USER_FUNC(btlevtcmd_SetFallAccel, LW(10), FLOAT(0.40))
        USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
        ADD(LW(2), 5)
        USER_FUNC(btlevtcmd_GetPos, LW(10), LW(0), EVT_NULLPTR, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_snd_se, LW(10), PTR("SFX_BTL_NOKO_GUARD2"), EVT_NULLPTR, 0, LW(5))
        BROTHER_EVT()
            WAIT_FRM(2)
            USER_FUNC(btlevtcmd_ACRStart, -2, 48, 67, 67, 0)
            USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
            IF_EQUAL(LW(6), 2)
                USER_FUNC(evtTot_LogActiveMoveStylish, 0)
                USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
                USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(0), 1, 0, 0, 0)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PNK_O_1"))
                WAIT_MSEC(1000)
            ELSE()
                USER_FUNC(evt_audience_acrobat_notry)
            END_IF()
        END_BROTHER()
        USER_FUNC(btlevtcmd_FallPosition, LW(10), LW(0), LW(1), LW(2), 60)
        USER_FUNC(evt_snd_sfxoff, LW(5))
        USER_FUNC(btlevtcmd_SetHomePos, LW(10), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_AnimeChangePose, LW(10), 1, PTR("PKO_F_1"))
        USER_FUNC(btlevtcmd_snd_se, LW(10), PTR("SFX_BTL_NOKO_GUARD1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(evt_btl_camera_shake_h, 0, 2, 0, 10, 13)
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
        USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
        BROTHER_EVT()
            SET(LW(9), LW(10))
            USER_FUNC(btlevtcmd_GetRotate, LW(9), EVT_NULLPTR, EVT_NULLPTR, LW(8))
            IF_SMALL(LW(8), 180)
                USER_FUNC(evt_sub_intpl_init, 11, LW(8), 0, 50)
            ELSE()
                USER_FUNC(evt_sub_intpl_init, 11, LW(8), 360, 50)
            END_IF()
            DO(50)
                USER_FUNC(evt_sub_intpl_get_value_para, LW(1), LW(2))
                USER_FUNC(btlevtcmd_SetRotate, LW(9), EVT_NULLPTR, EVT_NULLPTR, LW(1))
                WAIT_FRM(1)
            WHILE()
        END_BROTHER()
        IF_FLAG(LW(6), 0x2)
            USER_FUNC(btlevtcmd_AcGetOutputParam, 1, LW(0))
            SWITCH(LW(0))
                CASE_LARGE_EQUAL(8)
                    SET(LW(0), 2)
                CASE_LARGE_EQUAL(6)
                    SET(LW(0), 1)
                CASE_LARGE_EQUAL(4)
                    SET(LW(0), 0)
                CASE_ETC()
                    SET(LW(0), -1)
            END_SWITCH()
            IF_LARGE_EQUAL(LW(0), 0)
                USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), LW(0), LW(7))
                USER_FUNC(btlevtcmd_GetPos, LW(10), LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_ACSuccessEffect, LW(7), LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), LW(7))
            ELSE()
                USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), -1)
            END_IF()
        ELSE()
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), -1)
        END_IF()
        WAIT_FRM(50)
        USER_FUNC(btlevtcmd_OnAttribute, LW(3), 32)
        USER_FUNC(btlevtcmd_GetUnitId, LW(3), LW(0))
        USER_FUNC(btlevtcmd_SetUnitWork, LW(10), 0, LW(0))
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 43)
        USER_FUNC(btlevtcmd_SetUnitWork, LW(10), 2, 3)
        BROTHER_EVT_ID(LW(15))
            SET(LW(5), 5)
            DO(LW(5))
                USER_FUNC(btlevtcmd_GetUnitWork, LW(10), 2, LW(10))
                IF_NOT_EQUAL(LW(0), 3)
                    DO_BREAK()
                END_IF()
                WAIT_FRM(1)
                SET(LW(5), 5)
            WHILE()
        END_BROTHER()
        USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
        USER_FUNC(btlevtcmd_SetUnitWork, LW(10), 2, 0)
        IF_FLAG(LW(6), 0x2)
            USER_FUNC(btlevtcmd_AcGetOutputParam, 1, LW(0))
        ELSE()
            USER_FUNC(btlevtcmd_AcGetOutputParam, 2, LW(0))
        END_IF()
        USER_FUNC(evtTot_ShellShieldSetInitialHp, LW(10), LW(0))
        USER_FUNC(btlevtcmd_RunDataEventChild, LW(10), 61)
        USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(10), 1, 43)
        USER_FUNC(btlevtcmd_OnPartsAttribute, LW(3), 1, 16777216)
        USER_FUNC(btlevtcmd_OnAttribute, LW(3), 96)
        USER_FUNC(btlevtcmd_StartWaitEvent, LW(3))
        USER_FUNC(_check_mario_move_count, LW(0))
        IF_LARGE_EQUAL(LW(0), 1)
            USER_FUNC(evt_btl_camera_set_mode, 0, 0)
            USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
            USER_FUNC(evt_btl_camera_off_posoffset_manual, 0)
            WAIT_MSEC(500)
            USER_FUNC(btlevtcmd_RunDataEventChild, LW(10), 59)
            WAIT_FRM(60)
        END_IF()
        USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
        USER_FUNC(evt_audience_ap_recovery)
        USER_FUNC(btlevtcmd_InviteApInfoReport)
        USER_FUNC(btlevtcmd_StartWaitEvent, -2)
        USER_FUNC(evt_btl_camera_set_mode, 0, 0)
        USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
        USER_FUNC(evt_btl_camera_off_posoffset_manual, 0)
        RETURN()
    END_IF()
    BROTHER_EVT_ID(LW(15))
        WAIT_FRM(10)
        USER_FUNC(btlevtcmd_AnimeChangePose, -3, 1, PTR("M_N_7"))
        WAIT_FRM(35)
        USER_FUNC(btlevtcmd_AnimeChangePose, -3, 1, PTR("M_B_1"))
        WAIT_FRM(150)
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -3, 1, 43)
    END_BROTHER()
    USER_FUNC(btlevtcmd_snd_se, LW(10), PTR("SFX_BTL_NOKO_GUARD2"), EVT_NULLPTR, 0, LW(5))
    USER_FUNC(btlevtcmd_SetFallAccel, LW(10), FLOAT(0.40))
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    ADD(LW(2), 5)
    USER_FUNC(btlevtcmd_FallPosition, LW(10), LW(0), LW(1), LW(2), 60)
    USER_FUNC(btlevtcmd_SetHomePos, LW(10), LW(0), LW(1), LW(2))
    USER_FUNC(evt_snd_sfxoff, LW(5))
    USER_FUNC(evt_btl_camera_shake_h, 0, 2, 0, 10, 13)
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    USER_FUNC(btlevtcmd_SetRotate, LW(10), 0, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, LW(10), 1, PTR("PKO_F_1"))
    WAIT_FRM(50)
    USER_FUNC(btlevtcmd_AnimeChangePose, LW(10), 1, PTR("PKO_H_1"))
    USER_FUNC(btlevtcmd_AnimeChangePose, LW(10), 2, PTR("PKO_H_2"))
    USER_FUNC(btlevtcmd_OffPartsAttribute, LW(10), 2, 16777216)
    USER_FUNC(btlevtcmd_SetPartsRotateOffset, LW(10), 1, 20, 0, 0)
    USER_FUNC(btlevtcmd_SetPartsRotateOffset, LW(10), 2, -20, 0, 0)
    DO(20)
        USER_FUNC(btlevtcmd_AddPartsRotate, LW(10), 1, 0, 0, 2)
        USER_FUNC(btlevtcmd_AddPartsRotate, LW(10), 2, 0, 0, -2)
        WAIT_FRM(1)
    WHILE()
    DO(10)
        USER_FUNC(btlevtcmd_OnPartsAttribute, LW(10), 1, 16777216)
        USER_FUNC(btlevtcmd_OnPartsAttribute, LW(10), 2, 16777216)
        WAIT_FRM(2)
        USER_FUNC(btlevtcmd_OffPartsAttribute, LW(10), 1, 16777216)
        USER_FUNC(btlevtcmd_OffPartsAttribute, LW(10), 2, 16777216)
        WAIT_FRM(2)
    WHILE()
    USER_FUNC(btlevtcmd_KillUnit, LW(10), 0)
    USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
    LBL(99)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_off_posoffset_manual, 0)
    USER_FUNC(_color_lv_set, -3, 1, LW(14))
    USER_FUNC(evt_audience_ap_recovery)
    USER_FUNC(btlevtcmd_InviteApInfoReport)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(partyNokotarouAttack_TsuranukiKoura)
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    ELSE()
        USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
        USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(12))
        USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(12), LW(3), LW(4))
    END_IF()
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_CommandPayWeaponCost, -2)
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, 1)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    USER_FUNC(evt_btl_camera_set_posoffset, 0, 20, 0, 0)
    WAIT_FRM(20)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 4)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 400)

    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
        USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
        USER_FUNC(btlevtcmd_AcGetDifficulty, LW(0))
        SUB(LW(0), 3)
        SWITCH(LW(0))
            CASE_SMALL_EQUAL(-3)
                SET(LW(0), 25)
                SET(LW(1), 15)
            CASE_EQUAL(-2)
                SET(LW(0), 24)
                SET(LW(1), 21)
            CASE_EQUAL(-1)
                SET(LW(0), 22)
                SET(LW(1), 24)
            CASE_EQUAL(0)
                SET(LW(0), 20)
                SET(LW(1), 25)
            CASE_EQUAL(1)
                SET(LW(0), 17)
                SET(LW(1), 27)
            CASE_EQUAL(2)
                SET(LW(0), 15)
                SET(LW(1), 30)
            CASE_ETC()
                SET(LW(0), 12)
                SET(LW(1), 30)
        END_SWITCH()
        // Change gauge parameters and difficulty based on move level.
        USER_FUNC(evtTot_GetMoveSelectedLevel, MoveType::KOOPS_SHELL_SLAM, LW(5))
        SWITCH(LW(5))
            CASE_EQUAL(1)
                ADD(LW(0), 3)
                SUB(LW(1), 3)
                USER_FUNC(btlevtcmd_AcSetParamAll, 7, 200, 178, LW(0), LW(1), 34, 100, EVT_NULLPTR)
                USER_FUNC(btlevtcmd_AcSetGaugeParam, 100, 100, 100, 100)
            CASE_EQUAL(2)
                ADD(LW(0), 2)
                SUB(LW(1), 1)
                USER_FUNC(btlevtcmd_AcSetParamAll, 7, 200, 178, LW(0), LW(1), 20, 61, EVT_NULLPTR)
                USER_FUNC(btlevtcmd_AcSetGaugeParam, 60, 100, 100, 100)
            CASE_ETC()
                USER_FUNC(btlevtcmd_AcSetParamAll, 7, 200, 178, LW(0), LW(1), 15, 43, EVT_NULLPTR)
                USER_FUNC(btlevtcmd_AcSetGaugeParam, 42, 71, 100, 100)
        END_SWITCH()
        USER_FUNC(btlevtcmd_AcSetFlag, 0)
        USER_FUNC(btlevtcmd_SetupAC, -2, 6, 1, 0)
        WAIT_FRM(22)
    END_IF()

    USER_FUNC(btlevtcmd_SetUnitWork, -2, 3, -1)

    BROTHER_EVT_ID(LW(15))
LBL(5)
        IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
            USER_FUNC(btlevtcmd_AcGetOutputParam, 0, LW(0))
        ELSE()
            SET(LW(0), 1)
        END_IF()
        IF_EQUAL(LW(0), 0)
            WAIT_FRM(1)
            GOTO(5)
        END_IF()
        RUN_CHILD_EVT(PTR(&_change_koura_pose_fast))
        USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, 0, 1)
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 50)
        USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_NOKO_IPPATSU1"), LW(13))
        USER_FUNC(btlevtcmd_SetUnitWork, -2, 3, LW(13))
        USER_FUNC(btlevtcmd_SetUnitWork, -2, 5, 0)
        SET(LW(0), 15)
        USER_FUNC(btlevtcmd_SetUnitWork, -2, 1, 15)
        RUN_CHILD_EVT(PTR(&_koura_rotate_start))
    END_BROTHER()
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
        USER_FUNC(_tsuranuki_effect_control, -2)
    END_BROTHER()
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_StartAC, 1)
    END_IF()
    BROTHER_EVT_ID(LW(14))
        SET(LW(5), 5)
        DO(LW(5))
            IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
                USER_FUNC(btlevtcmd_AcGetOutputParam, 2, LW(0))
            ELSE()
                SET(LW(0), 4)
            END_IF()
            SWITCH(LW(0))
                CASE_SMALL_EQUAL(1)
                    USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, 1, 0)
                CASE_EQUAL(2)
                    USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, 1, 1)
                    USER_FUNC(btlevtcmd_SetUnitWork, -2, 1, 24)
                CASE_EQUAL(3)
                    USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, 1, 2)
                    USER_FUNC(btlevtcmd_SetUnitWork, -2, 1, 36)
                CASE_EQUAL(4)
                    USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, 1, 3)
                    USER_FUNC(btlevtcmd_SetUnitWork, -2, 1, 45)
            END_SWITCH()
            USER_FUNC(btlevtcmd_GetUnitWork, -2, 3, LW(13))
            IF_NOT_EQUAL(LW(13), -1)
                IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
                    USER_FUNC(btlevtcmd_AcGetOutputParam, 1, LW(1))
                ELSE()
                    SET(LW(1), 100)
                END_IF()
                USER_FUNC(btlevtcmd_SetUnitWork, -2, 5, LW(1))
                USER_FUNC(btlevtcmd_GetUnitWork, -2, 4, LW(0))
                USER_FUNC(btlevtcmd_GetUnitWork, -2, 5, LW(1))
                MUL(LW(0), LW(1))
                DIV(LW(0), 100)
                USER_FUNC(evt_snd_sfx_pit, LW(13), LW(0))
            END_IF()
            WAIT_FRM(1)
            SET(LW(5), 5)
        WHILE()
    END_BROTHER()
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_ResultAC)
    ELSE()
        WAIT_FRM(80)
    END_IF()
    DELETE_EVT(LW(14))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_AcGetOutputParam, 0, LW(0))
    ELSE()
        SET(LW(0), 1)
    END_IF()
    IF_EQUAL(LW(0), 0)
        DELETE_EVT(LW(15))
        USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, 0, 0)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PNK_K_2"))
        DO(10)
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, 12, 0)
            WAIT_FRM(1)
        WHILE()
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PNK_K_1"))
        DO(16)
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, 20, 0)
            WAIT_FRM(1)
        WHILE()
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PNK_H_1"))
        DO(14)
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, 20, 0)
            WAIT_FRM(1)
        WHILE()
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
        USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_ACROBAT_SHAKE1"), 0)
        WAIT_FRM(40)
        USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, 2, 1)
        RUN_CHILD_EVT(PTR(&_restore_koura_pose))
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 43)
        WAIT_FRM(30)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, 0, 0)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, 3, LW(13))
    USER_FUNC(evt_snd_sfxoff, LW(13))
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_NOKO_IPPATSU2"), 0)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_StopAC)
    END_IF()
    DELETE_EVT(LW(15))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 5, 0)
    USER_FUNC(evt_snd_sfxoff, LW(13))
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_NOKO_SHUBIBIN2"), 0)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_posoffset, 0, 0, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 50)
    SET(LW(10), 0)
    LBL(10)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(10.0))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), EVT_NULLPTR, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, 0, 0)
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
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 0)
    USER_FUNC(btlevtcmd_CommandPreCheckCounter, -2, LW(3), LW(4), 256, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, 2, 1)
        USER_FUNC(btlevtcmd_SetUnitWork, -2, 1, 0)
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
        RUN_CHILD_EVT(PTR(&_koura_rotate_stop))
    END_IF()
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_GetResultAC, LW(0))
        IF_FLAG(LW(0), 0x2)
            USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 131328, LW(5))
            USER_FUNC(btlevtcmd_AcGetOutputParam, 2, LW(9))
            IF_LARGE_EQUAL(LW(9), 4)
                SUB(LW(9), 4)
                // Set AC result to Nice for 4, Good for 6, Great for 8
                DIV(LW(9), 2)
                USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), LW(9), LW(6))
                USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
                USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), LW(6))
            ELSE()
                USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
                USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), -1)
            END_IF()
        ELSE()
            USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 256, LW(5))
            USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), -1)
        END_IF()
    ELSE()
        USER_FUNC(evtTot_GetMoveSelectedLevel, MoveType::KOOPS_SHELL_SLAM, LW(9))
        MUL(LW(9), 2)
        ADD(LW(9), 2)
        USER_FUNC(btlevtcmd_AcSetOutputParam, 2, LW(9))
        USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 131328, LW(5))
    END_IF()
LBL(50)
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    IF_NOT_EQUAL(LW(3), -1)
        ADD(LW(10), 1)
        GOTO(10)
    END_IF()
LBL(80)
    USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
    SET(LW(0), 400)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 1)
        MUL(LW(0), -1)
    END_IF()
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, 0, 0)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, 2, 1)
    WAIT_MSEC(100)
LBL(90)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    MUL(LW(0), -1)
    USER_FUNC(btlevtcmd_SetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, 0, 0)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_ACRStart, -2, 0, 9, 9, 0)
        USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
        IF_EQUAL(LW(6), 2)
            USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
            USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(0), 1, 0, 0, 0)
            USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
            USER_FUNC(btlevtcmd_SetUnitWork, -2, 1, 12)
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.30))
            USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 30, -1)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PNK_E_1"))
            WAIT_FRM(10)
            USER_FUNC(btlevtcmd_ACRStart, -2, 49, 79, 79, 0)
            USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
            IF_NOT_EQUAL(LW(6), 2)
                USER_FUNC(evt_audience_acrobat_notry)
                RUN_CHILD_EVT(PTR(&_koura_rotate_stop))
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PNK_E_2"))
                DO(6)
                    WAIT_FRM(1)
                    USER_FUNC(btlevtcmd_AddRotate, -2, 0, 45, 0)
                WHILE()
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PNK_S_1"))
                DO(6)
                    WAIT_FRM(1)
                    USER_FUNC(btlevtcmd_AddRotate, -2, 0, 15, 0)
                WHILE()
                GOTO(99)
            END_IF()
            USER_FUNC(evtTot_LogActiveMoveStylish, 0)
            RUN_CHILD_EVT(PTR(&_koura_rotate_stop))
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PNK_E_2"))
            DO(6)
                WAIT_FRM(1)
                USER_FUNC(btlevtcmd_AddRotate, -2, 0, 45, 0)
            WHILE()
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PNK_E_3"))
            DO(6)
                WAIT_FRM(1)
                USER_FUNC(btlevtcmd_AddRotate, -2, 0, 15, 0)
            WHILE()
            USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
            USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(0), 1, 0, 0, 0)
            WAIT_FRM(60)
            GOTO(99)
        ELSE()
            USER_FUNC(evt_audience_acrobat_notry)
        END_IF()
    END_IF()
    RUN_CHILD_EVT(PTR(&_restore_koura_pose))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 43)
    WAIT_FRM(30)
LBL(99)
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(evt_audience_ap_recovery)
        USER_FUNC(btlevtcmd_InviteApInfoReport)
    END_IF()
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RUN_CHILD_EVT(PTR(&btldefaultevt_SuitoruBadgeEffect))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(customAttack_BulkUp)
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    ELSE()
        // Boss can only use this move on self.
        SET(LW(3), -2)
        SET(LW(4), 1)
    END_IF()
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    USER_FUNC(btlevtcmd_CommandPayWeaponCost, -2)
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, -2)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    USER_FUNC(evt_btl_camera_set_posoffset, 0, 20, 0, 0)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    
    // Hold Fast AC.
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
        USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
        USER_FUNC(btlevtcmd_AcSetParamAll, 3, 0, 40, 1, 0, 1, 2, 0)
        USER_FUNC(btlevtcmd_AcSetFlag, 8)
        USER_FUNC(btlevtcmd_SetupAC, -2, 18, 1, 0)
        WAIT_FRM(32)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PNK_A_2"))
        WAIT_FRM(20)
        USER_FUNC(btlevtcmd_StartAC, 1)
        USER_FUNC(btlevtcmd_ResultAC)
        USER_FUNC(btlevtcmd_GetResultAC, LW(0))
        USER_FUNC(btlevtcmd_StopAC)
    
        IF_NOT_FLAG(LW(0), 0x2)
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), -1)
        ELSE()
            USER_FUNC(btlevtcmd_AcGetOutputParam, 0, LW(0))
            SUB(LW(0), 1)
            USER_FUNC(btlevtcmd_GetResultPrizeLv, -1, LW(0), LW(6))
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), LW(6))
        END_IF()
        WAIT_FRM(2)
        USER_FUNC(btlevtcmd_ACRStart, -2, 0, 12, 12, 0)
        USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
        SWITCH(LW(6))
            CASE_LARGE_EQUAL(2)
                USER_FUNC(evtTot_LogActiveMoveStylish, 0)
                USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
                USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(0), 1, 0, 0, 0)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PNK_O_1"))
                WAIT_FRM(30)
            CASE_ETC()
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PNK_S_1"))
                USER_FUNC(evt_audience_acrobat_notry)
                WAIT_FRM(20)
        END_SWITCH()
        SET(LW(8), 30)
    ELSE()
        WAIT_FRM(10)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PNK_A_2"))
        WAIT_FRM(50)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PNK_S_1"))
        USER_FUNC(btlevtcmd_AcSetOutputParam, 0, 3)
        SET(LW(8), 10)
    END_IF()

    USER_FUNC(btlevtcmd_AcGetOutputParam, 0, LW(6))
    ADD(LW(6), 1)
    USER_FUNC(evtTot_GetMoveSelectedLevel, MoveType::KOOPS_BULK_UP, LW(7))
    
    INLINE_EVT()
        WAIT_FRM(LW(8))
        USER_FUNC(evt_btl_camera_set_mode, 0, 0)
        USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
        USER_FUNC(evt_btl_camera_off_posoffset_manual, 0)
    END_INLINE()
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(evt_audience_ap_recovery)
        USER_FUNC(btlevtcmd_InviteApInfoReport)
        WAIT_FRM(LW(8))
    END_IF()

    // Spinning animation similar to Defend command, speeding up over a second.
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PNK_I_1"))
    USER_FUNC(btlevtcmd_GetUnitId, -3, LW(8))
    IF_EQUAL(LW(3), LW(8))
        USER_FUNC(btlevtcmd_AnimeChangePose, -3, 1, PTR("M_I_1"))
    END_IF()
    USER_FUNC(btlevtcmd_snd_se, LW(3), PTR("SFX_BTL_MARIO_DEFENCE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    SET(LW(8), 31)
    DO(60)
        SET(LW(9), LW(8))
        DIV(LW(9), 2)
        ADD(LW(8), 1)
        USER_FUNC(btlevtcmd_AddRotate, LW(3), 0, LW(9), 0)
        WAIT_FRM(1)
    WHILE()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PNK_S_1"))
        
    // Apply ATK and DEF buffs separately.
    INLINE_EVT()
        USER_FUNC(evtTot_MakeBulkUpWeapon, LW(12), LW(6), LW(7), 0)
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 256, LW(5))
    END_INLINE()
    INLINE_EVT()
        WAIT_FRM(24)
        USER_FUNC(evtTot_MakeBulkUpWeapon, LW(12), LW(6), LW(7), 1)
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 256, LW(5))
    END_INLINE()
    WAIT_FRM(80)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(customAttack_Withdraw)
    // Set gauge color parameters.
    USER_FUNC(evtTot_SetPendulumAcParams, MoveType::KOOPS_WITHDRAW)
    
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    IF_EQUAL((int32_t)GSW_Battle_DooplissMove, 0)
        USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    ELSE()
        // Boss can only use this move on self.
        SET(LW(3), -2)
        SET(LW(4), 1)
    END_IF()
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    USER_FUNC(btlevtcmd_CommandPayWeaponCost, -2)
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, -2)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    USER_FUNC(evt_btl_camera_set_posoffset, 0, 20, 0, 0)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PNK_A_2"))
    WAIT_FRM(30)
    // Spawn an invisible Shell Shield for the AC????
    USER_FUNC(btlevtcmd_SpawnUnit, LW(10), PTR(&entry_koura), 0)
    USER_FUNC(btlevtcmd_OnAttribute, LW(10), 16777216)
    USER_FUNC(btlevtcmd_GetBodyId, LW(10), LW(11))
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    ADD(LW(2), 5)
    ADD(LW(1), 240)
    USER_FUNC(btlevtcmd_SetPos, LW(10), LW(0), LW(1), LW(2))
    
    USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
    USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
    USER_FUNC(btlevtcmd_AcGetDifficulty, LW(0))
    SUB(LW(0), 3)
    SWITCH(LW(0))
        CASE_SMALL_EQUAL(-3)
            SET(LW(0), 360)
            SET(LW(1), 480)
        CASE_EQUAL(-2)
            SET(LW(0), 180)
            SET(LW(1), 360)
        CASE_EQUAL(-1)
            SET(LW(0), 120)
            SET(LW(1), 300)
        CASE_EQUAL(0)
            SET(LW(0), 90)
            SET(LW(1), 240)
        CASE_EQUAL(1)
            SET(LW(0), 70)
            SET(LW(1), 200)
        CASE_EQUAL(2)
            SET(LW(0), 40)
            SET(LW(1), 120)
        CASE_ETC()
            SET(LW(0), 20)
            SET(LW(1), 80)
    END_SWITCH()
    USER_FUNC(btlevtcmd_AcSetParamAll, 1, LW(10), LW(11), 30, LW(0), LW(1), EVT_NULLPTR, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AcSetFlag, 0)
    USER_FUNC(btlevtcmd_SetupAC, -2, 12, 1, 0)
    
    WAIT_FRM(30)
    
    USER_FUNC(btlevtcmd_StartAC, 1)
    USER_FUNC(btlevtcmd_ResultAC)
    USER_FUNC(btlevtcmd_GetResultAC, LW(6))
    
    // Considered success if the gauge is manually stopped in the pink/red area.
    SET(LW(7), 0)
    IF_FLAG(LW(6), 0x2)
        USER_FUNC(btlevtcmd_AcGetOutputParam, 1, LW(6))
        IF_LARGE_EQUAL(LW(6), 6)
            SET(LW(7), 1)
            // Set up Stylish for toward end of spinning animation, if success.
            USER_FUNC(btlevtcmd_SetUnitWork, -2, 5, 0)
            BROTHER_EVT()
                WAIT_FRM(2)
                USER_FUNC(btlevtcmd_ACRStart, -2, 35, 55, 55, 0)
                USER_FUNC(btlevtcmd_ACRGetResult, LW(14), LW(15))
                IF_EQUAL(LW(14), 2)
                    USER_FUNC(evtTot_LogActiveMoveStylish, 0)
                    USER_FUNC(btlevtcmd_SetUnitWork, -2, 5, 1)
                END_IF()
            END_BROTHER()
        END_IF()
    END_IF()
    USER_FUNC(btlevtcmd_StopAC)

    // Spin with hands above head.
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PNK_I_1"))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_MARIO_DEFENCE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    SET(LW(8), 31)
    DO(60)
        SET(LW(9), LW(8))
        DIV(LW(9), 2)
        ADD(LW(8), 1)
        USER_FUNC(btlevtcmd_AddRotate, -2, 0, LW(9), 0)
        WAIT_FRM(1)
    WHILE()

    IF_EQUAL(LW(7), 0)
        // Failure; slowly spin to a stop and get in dizzy animation.
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PNK_X_1"))
        SET(LW(8), 90)
        DO(96)
            SET(LW(9), LW(8))
            DIV(LW(9), 2)
            IF_LARGE(LW(8), 18)
                SUB(LW(8), 1)
            END_IF()
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, LW(9), 0)
            WAIT_FRM(1)
        WHILE()
        WAIT_FRM(30)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PNK_S_1"))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), -1)
        WAIT_FRM(20)
    ELSE()
        USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), 0, LW(7))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_ACSuccessEffect, LW(7), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), LW(7))
        
        // Success; enter shell. If Stylish, keep spinning for a bit longer.
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PNK_A_1"))
        
        USER_FUNC(btlevtcmd_GetUnitWork, -2, 5, LW(5))
        IF_EQUAL(LW(5), 1)
            SET(LW(6), 90)
            SET(LW(7), 1)
            SET(LW(8), 60)
            SET(LW(13), 20)
            BROTHER_EVT()
                WAIT_FRM(5)
                USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, 0, 0, 0)
            END_BROTHER()
        ELSE()
            SET(LW(6), 90)
            SET(LW(7), 4)
            SET(LW(8), 10)
            SET(LW(13), 40)
            USER_FUNC(evt_audience_acrobat_notry)
        END_IF()
        
        DO(LW(8))
            SET(LW(9), LW(6))
            DIV(LW(9), 2)
            SUB(LW(6), LW(7))
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, LW(9), 0)
            WAIT_FRM(1)
        WHILE()
            
        WAIT_FRM(LW(13))
        USER_FUNC(btlevtcmd_AnnounceMessage, 0, 0, 0, PTR("tot_status_withdraw"), 60)
        
        // Disable all damage and status.
        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 1, int(0xe0000000))
        // Turn off ability to be flipped by jumps, etc.
        USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, int(0x1000))
    END_IF()
    
    // Kill invisible Shell Shield actor.
    USER_FUNC(btlevtcmd_KillUnit, LW(10), 0)
    
    USER_FUNC(evt_audience_ap_recovery)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_off_posoffset_manual, 0)
    
    USER_FUNC(btlevtcmd_InviteApInfoReport)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

BattleWeapon customWeapon_KoopsShellToss = {
    .name = "btl_wn_pnk_normal",
    .icon = IconType::PARTNER_MOVE_0,
    .item_id = 0,
    .description = "msg_pnk_normal_attack",
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
    .damage_function_params = { 2, 3, 3, 5, 4, 8, 0, MoveType::KOOPS_BASE },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags =
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::ONLY_FRONT |
        AttackTargetProperty_Flags::SHELL_TOSS_LIKE |
        AttackTargetProperty_Flags::CANNOT_TARGET_CEILING,
    .element = AttackElement::NORMAL,
    .damage_pattern = 8,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_hammer",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::USABLE_IF_CONFUSED |
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags =
        AttackCounterResistance_Flags::ALL &
        ~AttackCounterResistance_Flags::PAYBACK,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)partyNokotarouAttack_NormalAttack,
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

BattleWeapon customWeapon_KoopsShellTossFS = {
    .name = nullptr,
    .icon = IconType::PARTNER_MOVE_0,
    .item_id = 0,
    .description = nullptr,
    .base_accuracy = 100,
    .base_fp_cost = 0,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 1,
    .bingo_card_chance = 1,
    .unk_1b = 1,
    .damage_function = (void*)GetWeaponPowerFromMaxLevel,
    .damage_function_params = { 3, 3, 3, 3, 3, 3, 0, 0 },
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
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::ONLY_FRONT |
        AttackTargetProperty_Flags::SHELL_TOSS_LIKE |
        AttackTargetProperty_Flags::CANNOT_TARGET_CEILING,
    .element = AttackElement::NORMAL,
    .damage_pattern = 8,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::CANNOT_MISS |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)partyNokotarouAttack_FirstAttack,
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

BattleWeapon customWeapon_KoopsPowerShell = {
    .name = "btl_wn_pnk_lv1",
    .icon = IconType::PARTNER_MOVE_1,
    .item_id = 0,
    .description = "msg_pnk_syubibin_koura",
    .base_accuracy = 100,
    .base_fp_cost = 3,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 1, 2, 2, 3, 3, 5, 0, MoveType::KOOPS_POWER_SHELL },
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
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::SHELL_TOSS_LIKE |
        AttackTargetProperty_Flags::CANNOT_TARGET_CEILING,
    .element = AttackElement::NORMAL,
    .damage_pattern = 8,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_koura_shubibin",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = 
        AttackCounterResistance_Flags::ALL &
        ~AttackCounterResistance_Flags::PAYBACK,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)partyNokotarouAttack_SyubibinKoura,
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

BattleWeapon customWeapon_KoopsShellShield = {
    .name = "btl_wn_pnk_lv2",
    .icon = IconType::PARTNER_MOVE_2,
    .item_id = 0,
    .description = "msg_pnk_koura_no_mamori",
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
    .damage_function_params = { 0, 2, 2, 3, 3, 4, 0, 0 },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags =
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::ONLY_TARGET_MARIO |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_SAME_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_koura_no_mamori",
    .special_property_flags = AttackSpecialProperty_Flags::UNGUARDABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)partyNokotarouAttack_KouraGuard,
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

BattleWeapon customWeapon_KoopsShellSlam = {
    .name = "btl_wn_pnk_lv3_tsuranuki",
    .icon = IconType::PARTNER_MOVE_3,
    .item_id = 0,
    .description = "msg_pnk_koura_tsuranuki",
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
    // Maximum damage: 4, 6, 8
    .damage_function_params = { 2, 0, 0, 0, 0, 0, 0, 0 },
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
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::SHELL_TOSS_LIKE |
        AttackTargetProperty_Flags::CANNOT_TARGET_CEILING,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_tsuranuki_koura",
    .special_property_flags = 
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING |
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::TOT_INCREASING_BY_TARGET,
    .counter_resistance_flags =
        AttackCounterResistance_Flags::ALL &
        ~AttackCounterResistance_Flags::PAYBACK,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)partyNokotarouAttack_TsuranukiKoura,
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

BattleWeapon customWeapon_KoopsBulkUp = {
    .name = "tot_ptr2_bulkup",
    .icon = IconType::PARTNER_MOVE_2,
    .item_id = 0,
    .description = "tot_ptr2_bulkup_desc",
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
    .damage_function_params = { 0, 2, 2, 3, 3, 4, 0, 0 },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags =
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_OPPOSING_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_ENEMY |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_SAME_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_bom_counter",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::CANNOT_MISS,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    .atk_change_chance = 0,
    .atk_change_time = 1,
    .atk_change_strength = 1,
    .def_change_chance = 0,
    .def_change_time = 1,
    .def_change_strength = 1,
    
    .attack_evt_code = (void*)customAttack_BulkUp,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 5,
    .nozzle_fire_chance = 5,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

BattleWeapon customWeapon_KoopsWithdraw = {
    .name = "tot_ptr2_withdraw",
    .icon = IconType::PARTNER_MOVE_1,
    .item_id = 0,
    .description = "tot_ptr2_withdraw_desc",
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
    .damage_function_params = { 0, 2, 2, 3, 3, 4, 0, 0 },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags =
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::ONLY_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_SAME_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_koura_no_mamori",
    .special_property_flags = AttackSpecialProperty_Flags::UNGUARDABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)customAttack_Withdraw,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 5,
    .nozzle_fire_chance = 5,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

}  // namespace mod::tot::party_koops