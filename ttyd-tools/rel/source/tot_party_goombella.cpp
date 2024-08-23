#include "tot_party_goombella.h"

#include "evt_cmd.h"
#include "patch.h"
#include "patches_battle.h"
#include "tot_manager_achievements.h"
#include "tot_manager_move.h"

#include <gc/types.h>
#include <ttyd/battle.h>
#include <ttyd/battle_camera.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_event_default.h>
#include <ttyd/battle_event_subset.h>
#include <ttyd/battle_mario.h>
#include <ttyd/battle_message.h>
#include <ttyd/battle_monosiri.h>
#include <ttyd/battle_unit.h>
#include <ttyd/battle_weapon_power.h>
#include <ttyd/evt_audience.h>
#include <ttyd/evt_eff.h>
#include <ttyd/evt_msg.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/icondrv.h>
#include <ttyd/msgdrv.h>
#include <ttyd/unit_party_christine.h>

namespace mod::infinite_pit {

extern const int32_t g_ac_monosiri_target_WhiteReticleScale;
extern const int32_t g_ac_monosiri_target_GreyReticleScale;
extern const int32_t g_ac_monosiri_target_ReticleZoomSpeed;

}  // namespace mod::infinite_pit

namespace mod::tot::party_goombella {

namespace {

// Including entire namespaces for convenience.
using namespace ::ttyd::battle_camera;
using namespace ::ttyd::battle_database_common;
using namespace ::ttyd::battle_event_cmd;
using namespace ::ttyd::battle_event_default;
using namespace ::ttyd::battle_event_subset;
using namespace ::ttyd::battle_mario;
using namespace ::ttyd::battle_message;
using namespace ::ttyd::battle_weapon_power;
using namespace ::ttyd::evt_audience;
using namespace ::ttyd::evt_eff;
using namespace ::ttyd::evt_msg;
using namespace ::ttyd::evt_snd;
using namespace ::ttyd::unit_party_christine;

using ::ttyd::battle::g_BattleWork;
using ::ttyd::battle_unit::BattleWorkUnit;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

namespace IconType = ::ttyd::icondrv::IconType;
namespace StatusEffectType = ::ttyd::battle_database_common::StatusEffectType;

}  // namespace

// Declaration of weapon structs.
extern BattleWeapon customWeapon_GoombellaHeadbonk;
extern BattleWeapon customWeapon_GoombellaTattle;
extern BattleWeapon customWeapon_GoombellaMultibonk;
extern BattleWeapon customWeapon_GoombellaRallyWink;
extern BattleWeapon customWeapon_GoombellaScopeOut;
extern BattleWeapon customWeapon_GoombellaIronbonk;

BattleWeapon* g_WeaponTable[] = {
    &customWeapon_GoombellaHeadbonk, &customWeapon_GoombellaTattle,
    &customWeapon_GoombellaIronbonk, &customWeapon_GoombellaScopeOut,
    &customWeapon_GoombellaMultibonk, &customWeapon_GoombellaRallyWink
};

void MakeSelectWeaponTable(
    ttyd::battle::BattleWorkCommand* command_work, int32_t* num_options) {
    for (int32_t i = 0; i < 6; ++i) {
        auto& weapon_entry = command_work->weapon_table[*num_options];
        BattleWeapon* weapon = g_WeaponTable[i];
        
        if (MoveManager::GetUnlockedLevel(MoveType::GOOMBELLA_BASE + i)) {
            weapon_entry.index = MoveType::GOOMBELLA_BASE + i;
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

// Sets Tattle flags for both the current AND original type of an enemy.
EVT_DECLARE_USER_FUNC(evtTot_SetTattleLogFlags, 1)
EVT_DEFINE_USER_FUNC(evtTot_SetTattleLogFlags) {
    uint32_t unit_idx = evtGetValue(evt, evt->evtArguments[0]);
    BattleWorkUnit* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, unit_idx);
    ttyd::battle_monosiri::battleSetUnitMonosiriFlag(unit->current_kind);
    ttyd::battle_monosiri::battleSetUnitMonosiriFlag(unit->true_kind);
    return 2;
}

// Changes the Tattle action command's reticle size to fit the camera view.
EVT_DECLARE_USER_FUNC(evtTot_SetReticleParams, 1)
EVT_DEFINE_USER_FUNC(evtTot_SetReticleParams) {
    static const gc::vec3 k_TattleScale = { 1.0f, 1.0f, 1.0f };
    static const gc::vec3 k_ScopeOutScale = { 2.5f, 2.5f, 2.5f };
    static const float k_TattleZoomSpeed = 0.01f;
    static const float k_ScopeOutZoomSpeed = 0.025f;
    
    int32_t move_type = evtGetValue(evt, evt->evtArguments[0]);
    if (move_type == MoveType::GOOMBELLA_TATTLE) {
        mod::patch::writePatch(
            (void*)mod::infinite_pit::g_ac_monosiri_target_WhiteReticleScale,
            &k_TattleScale, sizeof(k_TattleScale));
        mod::patch::writePatch(
            (void*)mod::infinite_pit::g_ac_monosiri_target_GreyReticleScale,
            &k_TattleScale, sizeof(k_TattleScale));
        mod::patch::writePatch(
            (void*)mod::infinite_pit::g_ac_monosiri_target_ReticleZoomSpeed,
            &k_TattleZoomSpeed, sizeof(float));
    } else {  // Scope Out
        mod::patch::writePatch(
            (void*)mod::infinite_pit::g_ac_monosiri_target_WhiteReticleScale,
            &k_ScopeOutScale, sizeof(k_ScopeOutScale));
        mod::patch::writePatch(
            (void*)mod::infinite_pit::g_ac_monosiri_target_GreyReticleScale,
            &k_ScopeOutScale, sizeof(k_ScopeOutScale));
        mod::patch::writePatch(
            (void*)mod::infinite_pit::g_ac_monosiri_target_ReticleZoomSpeed,
            &k_ScopeOutZoomSpeed, sizeof(float));
    }
    return 2;
}

EVT_BEGIN(partyChristineAttack_NormalAttack)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.0), FLOAT(0.70))
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_CommandPayWeaponCost, -2)
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    USER_FUNC(evt_btl_camera_set_mode, 0, 11)
    USER_FUNC(evt_btl_camera_set_homing_unitparts, 0, -2, 1, LW(3), LW(4))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
    USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
    USER_FUNC(btlevtcmd_SetupAC, -2, 1, 1, 0)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.0))
    USER_FUNC(btlevtcmd_GetTakeoffPosition, -2, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(0), LW(2), 1, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_GetMoveFrame, -2, LW(0), LW(1), LW(2), 0, LW(14))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), LW(14), -1, 0)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    
    // For Ironbonk, show an arc of orange stars to signify that it pierces.
    IF_EQUAL(LW(12), PTR(&customWeapon_GoombellaIronbonk))
        BROTHER_EVT()
            SET(LW(13), 2)
            USER_FUNC(_jump_star_effect, -2, LW(13))
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_MARIO_HAMMER_SHINE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        END_BROTHER()
    END_IF()
    
    USER_FUNC(btlevtcmd_StartAC, 1)
    SET(LW(10), 0)
    LBL(10)
    IF_LARGE_EQUAL(LW(10), 1)
        USER_FUNC(btlevtcmd_CommandPreCheckDamage, -2, LW(3), LW(4), 1048832, LW(5))
    ELSE()
        USER_FUNC(btlevtcmd_CommandPreCheckDamage, -2, LW(3), LW(4), 256, LW(5))
    END_IF()
    SWITCH(LW(5))
        CASE_OR(2)
        CASE_OR(3)
        CASE_OR(6)
        CASE_OR(4)
            CASE_END()
        CASE_EQUAL(1)
        CASE_ETC()
            WAIT_MSEC(1000)
    END_SWITCH()
    IF_SMALL_EQUAL(LW(10), 0)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKR_A_2B"))
        BROTHER_EVT()
            WAIT_FRM(2)
            USER_FUNC(btlevtcmd_ACRStart, -2, 0, 7, 7, 0)
        END_BROTHER()
        WAIT_FRM(10)
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKR_J_1A"))
    INLINE_EVT()
        WAIT_FRM(15)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_KURI_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    END_INLINE()
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.0), FLOAT(0.70))
    IF_LARGE_EQUAL(LW(10), 1)
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
        ADD(LW(2), 5)
        USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 14, 0)
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
        BROTHER_EVT()
            WAIT_FRM(15)
            DO(6)
                USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(0))
                MUL(LW(0), -30)
                USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, LW(0))
                WAIT_FRM(1)
            WHILE()
        END_BROTHER()
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_PARTY_BATTLE_JUMP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_MarioJumpPosition, -2, LW(0), LW(1), LW(2), 36, 3, 0)
    ELSE()
        USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 14, 0)
        USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
        IF_EQUAL(LW(6), 2)
            USER_FUNC(evtTot_LogActiveMoveStylish, 0)
            BROTHER_EVT()
                USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
                USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(0), 1, 0, 0, 0)
            END_BROTHER()
            BROTHER_EVT()
                USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
                IF_EQUAL(LW(15), 1)
                    SET(LW(9), -15)
                ELSE()
                    SET(LW(9), 15)
                END_IF()
                USER_FUNC(btlevtcmd_GetPosFloat, -2, LW(5), LW(6), LW(7))
                SET(LW(8), 0)
                DO(36)
                    USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, LW(9))
                    WAIT_FRM(1)
                WHILE()
            END_BROTHER()
        ELSE()
            BROTHER_EVT()
                USER_FUNC(btlevtcmd_GetPosFloat, -2, LW(5), LW(6), LW(7))
                SET(LW(8), 0)
                DO(36)
                    USER_FUNC(btlevtcmd_GetPosFloat, -2, LW(12), LW(13), LW(14))
                    USER_FUNC(krb_get_dir, LW(5), LW(6), LW(12), LW(13), LW(8))
                    USER_FUNC(btlevtcmd_GetRotate, -2, LW(0), LW(1), LW(2))
                    USER_FUNC(btlevtcmd_SetRotate, -2, LW(0), LW(1), LW(8))
                    SETF(LW(5), LW(12))
                    SETF(LW(6), LW(13))
                    WAIT_FRM(1)
                WHILE()
            END_BROTHER()
        END_IF()
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
        ADD(LW(2), 5)
        SET(LW(8), 36)
        USER_FUNC(btlevtcmd_MarioJumpParam, -2, LW(0), LW(1), LW(2), LW(8))
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_PARTY_BATTLE_JUMP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_MarioJumpPosition, -2, LW(0), LW(1), LW(2), LW(8), 0, 0)
    END_IF()
    IF_NOT_EQUAL(LW(5), 1)
        IF_EQUAL(LW(5), 3)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 38)
        END_IF()
        IF_EQUAL(LW(5), 6)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 39)
        END_IF()
        IF_EQUAL(LW(5), 2)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 40)
        END_IF()
        USER_FUNC(btlevtcmd_StopAC)
        USER_FUNC(btlevtcmd_JumpContinue, -2)
        USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 0, 0)
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKR_S_1"))
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.70))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_MoveDirectionAdd, -2, LW(0), 30)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, -1)
        USER_FUNC(btlevtcmd_MoveDirectionAdd, -2, LW(0), 20)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 10, -1)
        WAIT_MSEC(500)
        GOTO(95)
    END_IF()
    USER_FUNC(btlevtcmd_ac_timing_flag_onoff, 1, 1)
    USER_FUNC(btlevtcmd_ac_timing_get_success_frame, LW(0))
    DIV(LW(0), 2)
    WAIT_FRM(LW(0))
    USER_FUNC(btlevtcmd_ac_timing_flag_onoff, 0, 1)
    USER_FUNC(btlevtcmd_ResultAC)
    USER_FUNC(btlevtcmd_SetScale, -2, FLOAT(1.0), FLOAT(1.0), FLOAT(1.0))    
    USER_FUNC(btlevtcmd_GetResultAC, LW(6))
    IF_NOT_FLAG(LW(6), 0x2)
        USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 256, LW(5))
    
        // Layer Piercing Blow sound effect, if move had an effect.
        IF_EQUAL(LW(12), PTR(&customWeapon_GoombellaIronbonk))
            IF_EQUAL(LW(5), 18)
                USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_MARIO_HAMMER_TURANUKI1"), EVT_NULLPTR, 0, EVT_NULLPTR)
            END_IF()
        END_IF()
        
        IF_EQUAL(LW(10), 0)
            USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), -1)
        END_IF()
        GOTO(90)
    END_IF()    
    USER_FUNC(btlevtcmd_GetResultCountAC, LW(6))
    IF_SMALL_EQUAL(LW(6), 1)
        INLINE_EVT()
            USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 0)
            USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
            WAIT_FRM(10)
            USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
            USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
        END_INLINE()
        USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 131072, LW(5))

        // Layer Piercing Blow sound effect, if move had an effect.
        IF_EQUAL(LW(12), PTR(&customWeapon_GoombellaIronbonk))
            IF_EQUAL(LW(5), 18)
                USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_MARIO_HAMMER_TURANUKI1"), EVT_NULLPTR, 0, EVT_NULLPTR)
            END_IF()
        END_IF()
        
        USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), 0, LW(6))
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), LW(6))
        ADD(LW(10), 1)
        GOTO(10)
    ELSE()
        USER_FUNC(evt_btl_camera_set_mode, 0, 0)
        USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 131328, LW(5))
        USER_FUNC(btlevtcmd_GetResultAC, LW(0))

        // Layer Piercing Blow sound effect, if move had an effect.
        IF_EQUAL(LW(12), PTR(&customWeapon_GoombellaIronbonk))
            IF_EQUAL(LW(5), 18)
                USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_MARIO_HAMMER_TURANUKI1"), EVT_NULLPTR, 0, EVT_NULLPTR)
            END_IF()
        END_IF()
    END_IF()
    LBL(90)
    USER_FUNC(btlevtcmd_StopAC)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_audience_ap_recovery)
    USER_FUNC(btlevtcmd_InviteApInfoReport)
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_GetRotate, -2, EVT_NULLPTR, EVT_NULLPTR, LW(6))
        IF_SMALL(LW(6), 0)
            MUL(LW(6), -1)
        END_IF()
        USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(0))
        MUL(LW(6), LW(0))
        DIV(LW(6), 15)
        SET(LW(7), 15)
        LBL(20)
        USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, LW(6))
        SUB(LW(7), 1)
        WAIT_FRM(1)
        IF_LARGE_EQUAL(LW(7), 1)
            GOTO(20)
        END_IF()
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    END_BROTHER()
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.70))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHomePos, -2, EVT_NULLPTR, LW(1), EVT_NULLPTR)
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 60)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 30, -1)
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.60))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 20)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 8)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 8, -1)
    LBL(95)
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

EVT_BEGIN(partyChristineAttack_Monosiri)
    USER_FUNC(evtTot_SetReticleParams, MoveType::GOOMBELLA_TATTLE)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.0), FLOAT(0.70))
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    USER_FUNC(btlevtcmd_CommandPayWeaponCost, -2)
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, -2)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 350)
    USER_FUNC(evt_btl_camera_set_posoffset, 0, 40, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKR_2A_1"))
    WAIT_MSEC(500)
    DO(2)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKR_2A_2"))
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_KURI_LEARNED1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        WAIT_MSEC(400)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKR_2A_3"))
        WAIT_MSEC(350)
    WHILE()
    USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
    USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
    USER_FUNC(btlevtcmd_AcSetParamAll, LW(3), LW(4), EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AcSetFlag, 0)
    USER_FUNC(btlevtcmd_SetupAC, -2, 21, 1, 0)
    BROTHER_EVT()
        USER_FUNC(evt_btl_camera_set_mode, 0, 0)
        USER_FUNC(evt_btl_camera_set_posoffset, 0, 0, 0, 0)
    END_BROTHER()
    WAIT_FRM(22)
    USER_FUNC(btlevtcmd_StartAC, 1)
    USER_FUNC(btlevtcmd_ResultAC)
    USER_FUNC(btlevtcmd_GetResultAC, LW(6))
    IF_FLAG(LW(6), 0x2)
        USER_FUNC(btlevtcmd_GetResultPrizeLv, -5, 0, LW(6))
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), LW(6))
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
    ELSE()
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), -1)
    END_IF()
    USER_FUNC(btlevtcmd_GetResultAC, LW(6))
    IF_NOT_FLAG(LW(6), 0x2)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(evt_eff_fukidashi, 2, PTR(""), 0, 2, 0, LW(0), LW(1), LW(2), 30, 0, 56)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_KURI_FAIL_LEARNED1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKR_2A_4"))
        WAIT_FRM(56)
        GOTO(90)
    END_IF()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_KURI_LEARNED2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKR_2A_4"))
    WAIT_FRM(3)
    BROTHER_EVT_ID(LW(15))
        WAIT_FRM(53)
    END_BROTHER()
    USER_FUNC(btlevtcmd_ftof, 0, LW(0))
    USER_FUNC(btlevtcmd_ftof, 53, LW(1))
    USER_FUNC(btlevtcmd_ACRStart, -2, LW(0), LW(1), LW(1), 0)
    USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
    IF_LARGE_EQUAL(LW(6), 2)
        DELETE_EVT(LW(15))
        USER_FUNC(evtTot_LogActiveMoveStylish, 0)
        USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, 0, 0, 0)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKR_Y_1"))
        USER_FUNC(btlevtcmd_ftomsec, 28, LW(0))
        WAIT_MSEC(LW(0))
    ELSE()
        USER_FUNC(evt_audience_acrobat_notry)
        USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
    END_IF()
    USER_FUNC(btlevtcmd_SetTalkPose, -2, PTR("PKR_2A_5"))
    USER_FUNC(btlevtcmd_SetStayPose, -2, PTR("PKR_2A_5"))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 0, 0)
    RUN_EVT(PTR(&christine_dictionary_event))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 0, 1)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_off_posoffset_manual, 0)
    LBL(10)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, 0, LW(0))
    IF_NOT_EQUAL(LW(0), 3)
        WAIT_FRM(1)
        GOTO(10)
    END_IF()
    USER_FUNC(btlevtcmd_get_monosiri_msg_no, LW(3), LW(4), LW(0))
    USER_FUNC(evt_msg_print, 2, LW(0), 0, -2)

    // Mark Tattle log flags.
    USER_FUNC(evtTot_SetTattleLogFlags, LW(3))
    // Check for completion of Tattle Log.
    USER_FUNC(evtTot_CheckCompletedAchievement,
        AchievementId::META_TATTLE_LOG_BASIC, EVT_NULLPTR)
    USER_FUNC(evtTot_CheckCompletedAchievement,
        AchievementId::META_TATTLE_LOG_ALL, EVT_NULLPTR)

    USER_FUNC(btlevtcmd_SetTalkPose, -2, PTR("PKR_T_1"))
    USER_FUNC(btlevtcmd_SetStayPose, -2, PTR("PKR_S_1"))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 0, 4)
    LBL(20)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, 0, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        WAIT_FRM(1)
        GOTO(20)
    END_IF()
    LBL(90)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_off_posoffset_manual, 0)
    USER_FUNC(evt_audience_ap_recovery)
    USER_FUNC(btlevtcmd_InviteApInfoReport)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(partyChristineAttack_RenzokuAttack)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.0), FLOAT(0.70))
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_CommandPayWeaponCost, -2)
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    USER_FUNC(evt_btl_camera_set_mode, 0, 11)
    USER_FUNC(evt_btl_camera_set_homing_unitparts, 0, -2, 1, LW(3), LW(4))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    USER_FUNC(evt_btl_camera_off_posoffset_manual, 0)
    USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
    USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
    USER_FUNC(btlevtcmd_SetupAC, -2, 1, 1, 0)
    USER_FUNC(btlevtcmd_ac_timing_flag_onoff, 1, 2)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.0))
    USER_FUNC(btlevtcmd_GetTakeoffPosition, -2, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(0), LW(2), 1, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKR_A_2B"))
    USER_FUNC(btlevtcmd_ACRStart, -2, 0, 10, 10, 0)
    WAIT_FRM(10)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_StartAC, 1)
    SET(LW(10), 0)
    LBL(10)
    IF_LARGE_EQUAL(LW(10), 1)
        USER_FUNC(btlevtcmd_CommandPreCheckDamage, -2, LW(3), LW(4), 1048832, LW(5))
    ELSE()
        USER_FUNC(btlevtcmd_CommandPreCheckDamage, -2, LW(3), LW(4), 256, LW(5))
    END_IF()
    SWITCH(LW(5))
        CASE_OR(2)
        CASE_OR(3)
        CASE_OR(6)
        CASE_OR(4)
            CASE_END()
        CASE_EQUAL(1)
        CASE_ETC()
            WAIT_MSEC(1000)
    END_SWITCH()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKR_J_1A"))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 3, 0)
    IF_LARGE_EQUAL(LW(10), 1)
        USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 14, 0)
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
        BROTHER_EVT()
            WAIT_FRM(15)
            DO(6)
                USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(0))
                MUL(LW(0), -30)
                USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, LW(0))
                WAIT_FRM(1)
            WHILE()
        END_BROTHER()
        INLINE_EVT()
            WAIT_FRM(15)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_KURI_ATTACK2"), EVT_NULLPTR, 0, EVT_NULLPTR)
        END_INLINE()
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
        ADD(LW(2), 5)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_PARTY_BATTLE_JUMP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_MarioJumpPosition, -2, LW(0), LW(1), LW(2), 36, 3, 0)
    ELSE()
        USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
        ADD(LW(2), 5)
        SET(LW(8), 40)
        USER_FUNC(btlevtcmd_MarioJumpParam, -2, LW(0), LW(1), LW(2), LW(8))
        USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 14, 0)
        BROTHER_EVT()
            IF_LARGE_EQUAL(LW(6), 2)
                // First part of Stylish.
                USER_FUNC(evtTot_LogActiveMoveStylish, 1)
                USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, 0, 0, 0)
                WAIT_FRM(20)
            END_IF()
            USER_FUNC(btlevtcmd_GetPosFloat, -2, LW(5), LW(6), LW(7))
            SET(LW(15), 0)
            DO(LW(8))
                USER_FUNC(btlevtcmd_GetPosFloat, -2, LW(12), LW(13), LW(14))
                USER_FUNC(krb_get_dir, LW(5), LW(6), LW(12), LW(13), LW(15))
                USER_FUNC(btlevtcmd_GetRotate, -2, LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_GetUnitWork, -2, 3, LW(9))
                ADD(LW(15), LW(9))
                USER_FUNC(btlevtcmd_SetRotate, -2, LW(0), LW(1), LW(15))
                SETF(LW(5), LW(12))
                SETF(LW(6), LW(13))
                WAIT_FRM(1)
            WHILE()
        END_BROTHER()
        INLINE_EVT()
            IF_LARGE_EQUAL(LW(6), 2)
                WAIT_FRM(20)
            END_IF()
            WAIT_FRM(15)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_KURI_ATTACK2"), EVT_NULLPTR, 0, EVT_NULLPTR)
        END_INLINE()
        IF_LARGE_EQUAL(LW(6), 2)
            BROTHER_EVT()
                USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(5))
                MUL(LW(5), -18)
                DO(20)
                    USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, LW(5))
                    WAIT_FRM(1)
                WHILE()
            END_BROTHER()
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, 0)
            BROTHER_EVT()
                USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
                USER_FUNC(evt_eff64, PTR(""), PTR("kemuri2_n64"), 2, LW(0), LW(1), LW(2), 0, 0, 0, 0, 0, 0, 0, 0)
            END_BROTHER()
            SET(LW(9), LW(8))
            SUB(LW(9), 1)
            USER_FUNC(btlevtcmd_SetUnitWork, -2, 2, LW(9))
            BROTHER_EVT()
                DO(LW(9))
                    WAIT_FRM(1)
                    USER_FUNC(btlevtcmd_GetUnitWork, -2, 2, LW(0))
                    SUB(LW(0), 1)
                    USER_FUNC(btlevtcmd_SetUnitWork, -2, 2, LW(0))
                WHILE()
            END_BROTHER()
            BROTHER_EVT_ID(LW(15))
                DO(1)
                    USER_FUNC(btlevtcmd_GetUnitWork, -2, 2, LW(0))
                    IF_LARGE(LW(0), 15)
                        SUB(LW(0), 15)
                        USER_FUNC(btlevtcmd_ACRStart, -2, 0, LW(0), LW(0), 0)
                        USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
                        SWITCH(LW(6))
                            CASE_LARGE_EQUAL(2)
                                // Second part of Stylish.
                                USER_FUNC(evtTot_LogActiveMoveStylish, 2)
                                USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
                                USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(0), 1, 0, 0, 0)
                                DO(15)
                                    USER_FUNC(btlevtcmd_AddRotate, -2, 0, 24, 0)
                                    USER_FUNC(btlevtcmd_GetUnitWork, -2, 3, LW(0))
                                    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(1))
                                    IF_LARGE_EQUAL(LW(1), 0)
                                        ADD(LW(0), -24)
                                    ELSE()
                                        ADD(LW(0), 24)
                                    END_IF()
                                    USER_FUNC(btlevtcmd_SetUnitWork, -2, 3, LW(0))
                                    WAIT_FRM(1)
                                WHILE()
                            CASE_ETC()
                        END_SWITCH()
                    END_IF()
                WHILE()
            END_BROTHER()
            USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
            ADD(LW(2), 5)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_PARTY_BATTLE_JUMP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_MarioJumpPosition, -2, LW(0), LW(1), LW(2), LW(8), 0, 0)
            DELETE_EVT(15)
        ELSE()
            BROTHER_EVT()
                USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
                USER_FUNC(evt_eff64, PTR(""), PTR("kemuri2_n64"), 2, LW(0), LW(1), LW(2), 0, 0, 0, 0, 0, 0, 0, 0)
            END_BROTHER()
            USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
            ADD(LW(2), 5)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_PARTY_BATTLE_JUMP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_MarioJumpPosition, -2, LW(0), LW(1), LW(2), LW(8), 0, 0)
        END_IF()
    END_IF()
    IF_NOT_EQUAL(LW(5), 1)
        IF_EQUAL(LW(5), 3)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 38)
        END_IF()
        IF_EQUAL(LW(5), 6)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 39)
        END_IF()
        IF_EQUAL(LW(5), 2)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 40)
        END_IF()
        USER_FUNC(btlevtcmd_StopAC)
        USER_FUNC(btlevtcmd_JumpContinue, -2)
        USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 0, 0)
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKR_S_1"))
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.70))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_MoveDirectionAdd, -2, LW(0), 30)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, -1)
        USER_FUNC(btlevtcmd_MoveDirectionAdd, -2, LW(0), 20)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 10, -1)
        WAIT_MSEC(500)
        GOTO(95)
    END_IF()
    USER_FUNC(btlevtcmd_ac_timing_flag_onoff, 1, 1)
    USER_FUNC(btlevtcmd_ac_timing_get_success_frame, LW(0))
    DIV(LW(0), 2)
    WAIT_FRM(LW(0))
    USER_FUNC(btlevtcmd_ac_timing_flag_onoff, 0, 1)
    USER_FUNC(btlevtcmd_ResultAC)
    USER_FUNC(btlevtcmd_SetScale, -2, FLOAT(1.0), FLOAT(1.0), FLOAT(1.0))
    USER_FUNC(btlevtcmd_GetResultAC, LW(6))
    IF_NOT_FLAG(LW(6), 0x2)
        USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 256, LW(5))
        IF_SMALL_EQUAL(LW(10), 0)
            USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), -1)
        ELSE()
            SET(LW(6), LW(10))
            SUB(LW(6), 1)
            USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), LW(6), LW(6))
            USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), LW(6))
        END_IF()
        GOTO(90)
    END_IF()
    USER_FUNC(btlevtcmd_GetResultCountAC, LW(6))
    USER_FUNC(mario_get_renzoku_count_max, LW(3), LW(6), LW(7))
    IF_SMALL(LW(6), LW(7))
        INLINE_EVT()
            USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 0)
            USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
            WAIT_FRM(10)
            USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
            USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
        END_INLINE()
        USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 131072, LW(5))
        USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), LW(10), LW(6))
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
        ADD(LW(10), 1)
        USER_FUNC(btlevtcmd_StartAC, 0)
        GOTO(10)
    ELSE()
        USER_FUNC(evt_btl_camera_set_mode, 0, 0)
        USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 131328, LW(5))
        USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), LW(10), LW(6))
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), LW(6))
    END_IF()
    LBL(90)
    USER_FUNC(btlevtcmd_StopAC)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_GetRotate, -2, EVT_NULLPTR, EVT_NULLPTR, LW(6))
        IF_SMALL(LW(6), 0)
            MUL(LW(6), -1)
        END_IF()
        USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(0))
        MUL(LW(6), LW(0))
        DIV(LW(6), 15)
        SET(LW(7), 15)
        LBL(20)
        USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, LW(6))
        SUB(LW(7), 1)
        WAIT_FRM(1)
        IF_LARGE_EQUAL(LW(7), 1)
            GOTO(20)
        END_IF()
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    END_BROTHER()
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.70))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHomePos, -2, EVT_NULLPTR, LW(1), EVT_NULLPTR)
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 60)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 30, -1)
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.60))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 20)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 8)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 8, -1)
    LBL(95)
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

EVT_BEGIN(partyChristineAttack_Kiss)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.0), FLOAT(0.70))
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    USER_FUNC(btlevtcmd_CommandPayWeaponCost, -2)
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, -2)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    RUN_CHILD_EVT(PTR(&unk_evt_803537c4))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_CalculateFaceDirection, LW(3), LW(4), -2, -1, 16, LW(15))
        USER_FUNC(btlevtcmd_ChangeFaceDirection, LW(3), LW(15))
    END_IF()
    USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
    USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
    USER_FUNC(btlevtcmd_AcGetDifficulty, LW(0))
    SUB(LW(0), 3)
    SWITCH(LW(0))
        CASE_SMALL_EQUAL(-3)
            SET(LW(0), 27)
            SET(LW(1), 20)
        CASE_EQUAL(-2)
            SET(LW(0), 25)
            SET(LW(1), 28)
        CASE_EQUAL(-1)
            SET(LW(0), 23)
            SET(LW(1), 30)
        CASE_EQUAL(0)
            SET(LW(0), 23)
            SET(LW(1), 30)
        CASE_EQUAL(1)
            SET(LW(0), 23)
            SET(LW(1), 30)
        CASE_EQUAL(2)
            SET(LW(0), 23)
            SET(LW(1), 30)
        CASE_ETC()
            SET(LW(0), 23)
            SET(LW(1), 32)
    END_SWITCH()
    USER_FUNC(btlevtcmd_AcSetParamAll, 16, 220, 70, 0, 0, LW(0), LW(1), 0)
    USER_FUNC(btlevtcmd_AcSetFlag, 0)
    USER_FUNC(btlevtcmd_SetupAC, -2, 5, 3, 60)
    WAIT_FRM(22)
    USER_FUNC(btlevtcmd_StartAC, 1)
    USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(6))
    MULF(LW(6), 12)
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), LW(6))
    USER_FUNC(btlevtcmd_GetStatusMg, LW(3), LW(6))
    MULF(LW(6), 12)
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), LW(6))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 200, -1, 0)
    USER_FUNC(btlevtcmd_ResultAC)
    USER_FUNC(btlevtcmd_GetResultAC, LW(0))
    IF_FLAG(LW(0), 0x2)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetHeight, -2, LW(6))
        ADD(LW(1), LW(6))
        USER_FUNC(btlevtcmd_GetResultPrizeLv, -5, 0, LW(6))
        USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
    END_IF()
    USER_FUNC(btlevtcmd_GetResultAC, LW(6))
    USER_FUNC(btlevtcmd_StopAC)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 350)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_KURI_KISS1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetBodyId, -2, LW(9))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, LW(9), PTR("PKR_O_1"))
    WAIT_MSEC(200)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 15)
    ADD(LW(1), 30)
    USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(14))
    MUL(LW(14), 45)
    USER_FUNC(evt_eff, PTR(""), PTR("kiss"), 1, LW(0), LW(1), LW(2), LW(14), 0, 0, 0, 0, 0, 0, 0)
    INLINE_EVT_ID(LW(15))
        IF_FLAG(LW(6), 0x2)
            RUN_CHILD_EVT(PTR(&unk_evt_803537c4))
            IF_NOT_EQUAL(LW(0), 0)
                USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(3), LW(4), 58)
            END_IF()
            USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
            ADD(LW(2), 15)
            USER_FUNC(btlevtcmd_GetHeight, LW(3), LW(15))
            USER_FUNC(btlevtcmd_GetStatusMg, LW(3), LW(14))
            MULF(LW(15), LW(14))
            DIV(LW(15), 2)
            ADD(LW(1), LW(15))
            USER_FUNC(evt_eff, 0, PTR("recovery"), 6, LW(0), LW(1), LW(2), 0, 0, 0, 0, 0, 0, 0, 0)
            USER_FUNC(evt_snd_sfxon_3d, PTR("SFX_BTL_KURI_CHEER_KISS1"), LW(0), LW(1), LW(2), 0)
            WAIT_FRM(40)
            USER_FUNC(btlevtcmd_GetResultPrizeLv, -5, 0, LW(6))
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, PTR(&customWeapon_GoombellaRallyWink), LW(6))
            USER_FUNC(_set_hustle, LW(3))
        ELSE()
            WAIT_FRM(40)
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, PTR(&customWeapon_GoombellaRallyWink), -1)
            USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&customWeapon_GoombellaRallyWink), 536871168, LW(5))
        END_IF()
    END_INLINE()
    USER_FUNC(btlevtcmd_ftof, 24, LW(0))
    USER_FUNC(btlevtcmd_ftof, 48, LW(1))

    WAIT_FRM(LW(0))
    // Disable Stylish entirely.
    // USER_FUNC(btlevtcmd_ACRStart, -2, LW(0), LW(1), LW(1), 0)
    // USER_FUNC(btlevtcmd_ACRGetResult, LW(7), LW(8))
    // IF_LARGE_EQUAL(LW(7), 2)
    //     USER_FUNC(evtTot_LogActiveMoveStylish, 0)
    //     USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
    //     USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(0), 1, 0, 0, 0)
    //     USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKR_J_1B"))
    //     WAIT_MSEC(166)
    //     USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKR_Y_1"))
    //     WAIT_MSEC(167)
    // ELSE()
    //     USER_FUNC(evt_audience_acrobat_notry)
    //     WAIT_FRM(20)
    // END_IF()

    IF_FLAG(LW(6), 0x2)
        USER_FUNC(btlevtcmd_AnnounceMessage, 0, 0, 0, PTR("msg_st_chg_mario_quick"), 60)
    ELSE()
        WAIT_MSEC(500)
    END_IF()
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    USER_FUNC(evt_audience_ap_recovery)
    USER_FUNC(btlevtcmd_InviteApInfoReport)
    RUN_CHILD_EVT(PTR(&unk_evt_803537c4))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_ResetFaceDirection, LW(3))
        USER_FUNC(btlevtcmd_StartWaitEvent, LW(3))
    END_IF()
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(6.0))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(customAttack_ScopeOut)
    USER_FUNC(evtTot_SetReticleParams, MoveType::GOOMBELLA_SCOPE_OUT)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.0), FLOAT(0.70))
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    USER_FUNC(btlevtcmd_CommandPayWeaponCost, -2)
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    
    // Hard zoom in on Goombella (similar to the one from Appeal).
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    SET(LW(5), LW(1))
    SET(LW(6), LW(2))
    ADD(LW(1), 43)
    ADD(LW(5), 25)
    ADD(LW(2), 250)
    USER_FUNC(evt_btl_camera_set_mode, 0, 3)
    USER_FUNC(evt_btl_camera_set_moveto, 0, LW(0), LW(1), LW(2), LW(0), LW(5), LW(6), 30, 11)
    WAIT_FRM(30)
    
    // Eye glint (might be a less ominous SFX choice but I'm drawing a blank)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_CheckStatus, -2, 10, LW(6))
    USER_FUNC(btlevtcmd_CheckStatus, -2, 11, LW(7))
    ADD(LW(2), 10)  // Render in front of character
    IF_NOT_EQUAL(LW(6), 0)
        ADD(LW(0), 12)
        ADD(LW(1), 35)
    ELSE()
        IF_EQUAL(LW(7), 0)
            ADD(LW(0), 7)
            ADD(LW(1), 20)
        ELSE()
            ADD(LW(0), 3)
            ADD(LW(1), 10)
        END_IF()
    END_IF()
    USER_FUNC(evt_eff, PTR(""), PTR("toge_flush"), 0, LW(0), LW(1), LW(2), 60, 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(evt_snd_sfxon_3d, PTR("SFX_BOSS_QWEN_EYE_SHINE1"), LW(0), LW(1), LW(2), 0)
    // Stylish command a bit after glint
    WAIT_FRM(2)
    USER_FUNC(btlevtcmd_ACRStart, -2, 13, 23, 23, 0)
    USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
    IF_LARGE_EQUAL(LW(6), 2)
        USER_FUNC(evtTot_LogActiveMoveStylish, 0)
        USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, 0, 0, 0)
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 66)
    ELSE()
        USER_FUNC(evt_audience_acrobat_notry)
    END_IF()
    WAIT_FRM(50)
    
    // Hard cut to zoomed in view of target until AC is finished.
    BROTHER_EVT()
        WAIT_FRM(5)
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
        SET(LW(5), LW(1))
        SET(LW(6), LW(2))
        ADD(LW(1), 13)
        ADD(LW(5), -5)
        ADD(LW(2), 250)
        USER_FUNC(evt_btl_camera_set_mode, 0, 3)
        USER_FUNC(evt_btl_camera_set_moveto, 0, LW(0), LW(1), LW(2), LW(0), LW(5), LW(6), 1, 11)
    END_BROTHER()
    
    USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
    USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
    USER_FUNC(btlevtcmd_AcSetParamAll, LW(3), LW(4), EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AcSetFlag, 0)
    USER_FUNC(btlevtcmd_SetupAC, -2, 21, 1, 0)
    WAIT_FRM(22)
    USER_FUNC(btlevtcmd_StartAC, 1)
    USER_FUNC(btlevtcmd_ResultAC)
    
    BROTHER_EVT()
        USER_FUNC(evt_btl_camera_set_mode, 0, 0)
        USER_FUNC(evt_btl_camera_set_posoffset, 0, 0, 0, 0)
    END_BROTHER()
    
    SET(LW(15), 0)
    USER_FUNC(btlevtcmd_GetResultAC, LW(6))
    IF_FLAG(LW(6), 0x2)
        USER_FUNC(btlevtcmd_GetResultPrizeLv, -5, 0, LW(6))
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), LW(6))
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
        
        // Fake a damaged state + apply the custom "Scoped" / "Scoped+" status.
        USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(3), LW(4), 39)
        USER_FUNC(evtTot_GetMoveSelectedLevel, MoveType::GOOMBELLA_SCOPE_OUT, LW(15))
        IF_EQUAL(LW(15), 1)
            USER_FUNC(infinite_pit::battle::evtTot_ApplyCustomStatus,
                LW(3), LW(4), ttyd::battle_unit::BattleUnitStatus_Flags::SCOPED,
                /* splash colors */ 0xdcdcdc, 0x605000,
                PTR("SFX_CONDITION_COUNTER1"), 
                PTR("tot_ptr1_scope_out_effect_msg1"))
        ELSE()
            USER_FUNC(infinite_pit::battle::evtTot_ApplyCustomStatus,
                LW(3), LW(4), ttyd::battle_unit::BattleUnitStatus_Flags::SCOPED_PLUS,
                /* splash colors */ 0xdcdcdc, 0x605000,
                PTR("SFX_CONDITION_COUNTER1"), 
                PTR("tot_ptr1_scope_out_effect_msg2"))
        END_IF()
        SET(LW(15), 1)
    ELSE()
        // Disappointed AC result / animation.
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), -1)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(evt_eff_fukidashi, 2, PTR(""), 0, 2, 0, LW(0), LW(1), LW(2), 30, 0, 56)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_KURI_FAIL_LEARNED1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKR_D_2"))
        WAIT_FRM(20)
    END_IF()

    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_off_posoffset_manual, 0)
    WAIT_FRM(60)
    
    // End enemy fake damage state.
    IF_EQUAL(LW(15), 1)
        USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, LW(3), 1)
    END_IF()
        
    USER_FUNC(evt_audience_ap_recovery)
    USER_FUNC(btlevtcmd_InviteApInfoReport)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

BattleWeapon customWeapon_GoombellaHeadbonk = {
    .name = "btl_wn_pkr_normal",
    .icon = IconType::PARTNER_MOVE_0,
    .item_id = 0,
    .description = "msg_pkr_normal_jump",
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
    .damage_function_params = { 2, 2, 3, 3, 4, 4, 0, MoveType::GOOMBELLA_BASE },
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
        AttackTargetProperty_Flags::JUMPLIKE |
        AttackTargetProperty_Flags::CANNOT_TARGET_CEILING,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_zutsuki",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::USABLE_IF_CONFUSED |
        AttackSpecialProperty_Flags::GROUNDS_WINGED |
        AttackSpecialProperty_Flags::FLIPS_SHELLED |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags =
        AttackCounterResistance_Flags::FRONT_SPIKY |
        AttackCounterResistance_Flags::PREEMPTIVE_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)partyChristineAttack_NormalAttack,
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

BattleWeapon customWeapon_GoombellaTattle = {
    .name = "btl_wn_pkr_lv1",
    .icon = IconType::PARTNER_MOVE_0,
    .item_id = 0,
    .description = "msg_pkr_monosiri",
    .base_accuracy = 100,
    .base_fp_cost = 0,
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
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::TATTLE_LIKE,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_monoshiri",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::CANNOT_MISS,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000,
        
    // status chances
    
    .attack_evt_code = (void*)partyChristineAttack_Monosiri,
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

BattleWeapon customWeapon_GoombellaMultibonk = {
    .name = "btl_wn_pkr_lv2",
    .icon = IconType::PARTNER_MOVE_2,
    .item_id = 0,
    .description = "msg_pkr_renzoku_zutsuki",
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
    .damage_function_params = { 1, 1, 2, 2, 3, 3, 0, MoveType::GOOMBELLA_MULTIBONK },
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
        AttackTargetProperty_Flags::JUMPLIKE |
        AttackTargetProperty_Flags::CANNOT_TARGET_CEILING,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_zutsuki",
    .special_property_flags = 
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::GROUNDS_WINGED |
        AttackSpecialProperty_Flags::FLIPS_SHELLED |
        AttackSpecialProperty_Flags::DIMINISHING_BY_HIT |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags =
        AttackCounterResistance_Flags::FRONT_SPIKY |
        AttackCounterResistance_Flags::PREEMPTIVE_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000,
        
    // status chances
    
    .attack_evt_code = (void*)partyChristineAttack_RenzokuAttack,
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

BattleWeapon customWeapon_GoombellaRallyWink = {
    .name = "btl_wn_pkr_lv3",
    .icon = IconType::PARTNER_MOVE_3,
    .item_id = 0,
    .description = "msg_pkr_nage_kiss",
    .base_accuracy = 100,
    .base_fp_cost = 4,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 0,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = nullptr,
    .damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
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
        AttackTargetProperty_Flags::TARGET_SAME_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_kiss",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::IGNORES_STATUS_CHANCE |
        AttackSpecialProperty_Flags::CANNOT_MISS,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = 0,
        
    // status chances (not used?)
    .fast_chance = 100,
    .fast_time = 2,
    
    .attack_evt_code = (void*)partyChristineAttack_Kiss,
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

BattleWeapon customWeapon_GoombellaScopeOut = {
    .name = "tot_ptr1_scope_out",
    .icon = IconType::PARTNER_MOVE_1,
    .item_id = 0,
    .description = "tot_ptr1_scope_out_desc",
    .base_accuracy = 100,
    .base_fp_cost = 0,
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
    .ac_help_msg = "msg_ac_monoshiri",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::CANNOT_MISS,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000,
        
    // status chances
    
    .attack_evt_code = (void*)customAttack_ScopeOut,
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

BattleWeapon customWeapon_GoombellaIronbonk = {
    .name = "tot_ptr1_ironbonk",
    .icon = IconType::PARTNER_MOVE_1,
    .item_id = 0,
    .description = "tot_ptr1_ironbonk_desc",
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
    .damage_function_params = { 2, 2, 3, 3, 4, 4, 0, MoveType::GOOMBELLA_IRONBONK },
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
        AttackTargetProperty_Flags::JUMPLIKE |
        AttackTargetProperty_Flags::CANNOT_TARGET_CEILING,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_zutsuki",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::USABLE_IF_CONFUSED |
        AttackSpecialProperty_Flags::GROUNDS_WINGED |
        AttackSpecialProperty_Flags::FLIPS_SHELLED |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)partyChristineAttack_NormalAttack,
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

}  // namespace mod::tot::party_goombella