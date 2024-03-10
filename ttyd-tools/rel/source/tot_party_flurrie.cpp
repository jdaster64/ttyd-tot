#include "tot_party_flurrie.h"

#include "evt_cmd.h"
#include "tot_move_manager.h"

#include <ttyd/battle.h>
#include <ttyd/battle_camera.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_event_default.h>
#include <ttyd/battle_sub.h>
#include <ttyd/battle_unit.h>
#include <ttyd/battle_weapon_power.h>
#include <ttyd/evt_audience.h>
#include <ttyd/evt_batstage.h>
#include <ttyd/evt_eff.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evt_sub.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/icondrv.h>
#include <ttyd/msgdrv.h>
#include <ttyd/unit_party_clauda.h>

namespace mod::tot::party_flurrie {

namespace {
    
// Including entire namespaces for convenience.
using namespace ::ttyd::battle_camera;
using namespace ::ttyd::battle_database_common;
using namespace ::ttyd::battle_event_cmd;
using namespace ::ttyd::battle_event_default;
using namespace ::ttyd::battle_weapon_power;
using namespace ::ttyd::evt_audience;
using namespace ::ttyd::evt_batstage;
using namespace ::ttyd::evt_eff;
using namespace ::ttyd::evt_snd;
using namespace ::ttyd::evt_sub;
using namespace ::ttyd::unit_party_clauda;

using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

namespace IconType = ::ttyd::icondrv::IconType;

}  // namespace

// Declaration of weapon structs.
extern BattleWeapon customWeapon_FlurrieBodySlam;
extern BattleWeapon customWeapon_FlurrieGaleForce;
extern BattleWeapon customWeapon_FlurrieLipLock;
extern BattleWeapon customWeapon_FlurrieDodgyFog;
extern BattleWeapon customWeapon_FlurrieBlizzard;
extern BattleWeapon customWeapon_FlurrieThunderStorm;

BattleWeapon* g_WeaponTable[] = {
    &customWeapon_FlurrieBodySlam, &customWeapon_FlurrieGaleForce, 
    &customWeapon_FlurrieLipLock, &customWeapon_FlurrieDodgyFog,
    &customWeapon_FlurrieBlizzard, &customWeapon_FlurrieThunderStorm
};

void MakeSelectWeaponTable(
    ttyd::battle::BattleWorkCommand* command_work, int32_t* num_options) {
    for (int32_t i = 0; i < 6; ++i) {
        auto& weapon_entry = command_work->weapon_table[*num_options];
        BattleWeapon* weapon = g_WeaponTable[i];
        
        if (MoveManager::GetUnlockedLevel(MoveType::FLURRIE_BASE + i)) {
            weapon_entry.index = MoveType::FLURRIE_BASE + i;
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

// Check whether enemies should shake during wind attacks.
EVT_DECLARE_USER_FUNC(evtTot_CheckEnemyShake, 3)
EVT_DEFINE_USER_FUNC(evtTot_CheckEnemyShake) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[0]));
    auto* battleWork = ttyd::battle::g_BattleWork;
    auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, unit_idx);
    auto* weapon = (BattleWeapon*)evtGetValue(evt, evt->evtArguments[1]);
    
    bool should_shake = true;
    if (weapon == &customWeapon_FlurrieGaleForce && 
        unit->status_vulnerability->gale_force <= 0) {
        should_shake = false;
    }
    
    evtSetValue(evt, evt->evtArguments[2], should_shake);
    
    return 2;
}

// Dynamically sets the damage and status chance parameters based on AC success.
EVT_DECLARE_USER_FUNC(evtTot_MakeBreathWeapon, 3)
EVT_DEFINE_USER_FUNC(evtTot_MakeBreathWeapon) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    auto* weapon = (BattleWeapon*)evtGetValue(evt, evt->evtArguments[0]);
    int32_t ac_result = evtGetValue(evt, evt->evtArguments[1]);
    int32_t move_type = evtGetValue(evt, evt->evtArguments[2]);
    int32_t move_level = MoveManager::GetSelectedLevel(move_type);
    
    // Make changes in place, since the parameters are unchanged between uses.
    if (move_type == MoveType::FLURRIE_GALE_FORCE) {
        weapon->gale_force_chance = ac_result;
    } else {  // Blizzard
        if (ac_result >= 75) {
            weapon->freeze_chance = ac_result * 0.9f;
            weapon->freeze_time = move_level;
            weapon->damage_function_params[0] = move_level + 2;
            battleWork->ac_manager_work.ac_result |= 2;
        } else {
            weapon->freeze_chance = 0;
            weapon->damage_function_params[0] = move_level + 1;
            battleWork->ac_manager_work.ac_result &= ~2;
        }
    }
    
    return 2;
}

EVT_BEGIN(partyClaudaAttack_NormalAttack)    
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
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
    USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
    USER_FUNC(btlevtcmd_AcGetDifficulty, LW(0))
    USER_FUNC(btlevtcmd_AcSetParamAll, LW(3), LW(4), 180, LW(0), EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AcSetFlag, 0)
    USER_FUNC(btlevtcmd_SetupAC, -2, 8, 1, 0)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    WAIT_FRM(20)
    USER_FUNC(btlevtcmd_StartAC, 1)
    USER_FUNC(btlevtcmd_ResultAC)
    USER_FUNC(btlevtcmd_StopAC)
    USER_FUNC(btlevtcmd_OffUnitFlag, -2, 2097152)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PWD_A1_1"))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_CLAUD_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.20))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    ADD(LW(2), 5)
    USER_FUNC(btlevtcmd_GetResultAC, LW(5))
    IF_NOT_FLAG(LW(5), 0x2)
        USER_FUNC(evt_sub_random, 40, LW(5))
        SUB(LW(5), 20)
        ADD(LW(0), LW(5))
        USER_FUNC(evt_sub_random, 20, LW(5))
        SUB(LW(5), 10)
        ADD(LW(1), LW(5))
    END_IF()
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 60, -1)
    USER_FUNC(btlevtcmd_OnUnitFlag, -2, 2097152)
    USER_FUNC(btlevtcmd_CommandPreCheckDamage, -2, LW(3), LW(4), 256, LW(5))
    IF_NOT_EQUAL(LW(5), 1)
        GOTO(10)
    END_IF()
    USER_FUNC(btlevtcmd_GetResultAC, LW(0))
    IF_FLAG(LW(0), 0x2)
        GOTO(20)
    END_IF()
    SET(LW(5), 1)
    LBL(10)
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
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PWD_A1_2"))
    USER_FUNC(btlevtcmd_JumpContinue, -2)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PWD_D_3"))
    USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), -1)
    BROTHER_EVT_ID(LW(15))
        USER_FUNC(evt_sub_intpl_init_float, 11, FLOAT(1.0), FLOAT(0.40), 10)
        DO(10)
            USER_FUNC(evt_sub_intpl_get_float, LW(5), EVT_NULLPTR)
            USER_FUNC(btlevtcmd_GetScale, -2, LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_SetScale, -2, LW(0), LW(5), LW(2))
            WAIT_FRM(1)
        WHILE()
        USER_FUNC(evt_sub_intpl_init_float, 11, FLOAT(0.40), FLOAT(1.0), 6)
        DO(6)
            USER_FUNC(evt_sub_intpl_get_float, LW(5), EVT_NULLPTR)
            USER_FUNC(btlevtcmd_GetScale, -2, LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_SetScale, -2, LW(0), LW(5), LW(2))
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()
    BROTHER_EVT()
        USER_FUNC(evt_sub_intpl_init_float, 11, FLOAT(1.0), FLOAT(1.5), 10)
        DO(10)
            USER_FUNC(evt_sub_intpl_get_float, LW(5), EVT_NULLPTR)
            USER_FUNC(btlevtcmd_GetScale, -2, LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_SetScale, -2, LW(5), LW(1), LW(2))
            WAIT_FRM(1)
        WHILE()
        USER_FUNC(evt_sub_intpl_init_float, 11, FLOAT(1.5), FLOAT(1.0), 6)
        DO(6)
            USER_FUNC(evt_sub_intpl_get_float, LW(5), EVT_NULLPTR)
            USER_FUNC(btlevtcmd_GetScale, -2, LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_SetScale, -2, LW(5), LW(1), LW(2))
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
    WAIT_FRM(30)
    USER_FUNC(evt_audience_ap_recovery)
    USER_FUNC(btlevtcmd_InviteApInfoReport)
    GOTO(95)
    LBL(20)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_CLAUD_ATT2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_CLAUD_ATTACK2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetResultAC, LW(0))
    IF_FLAG(LW(0), 0x2)
        USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 131328, LW(5))
        USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), 0, LW(6))
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), LW(6))
    ELSE()
        USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 256, LW(5))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), -1)
    END_IF()
    USER_FUNC(btlevtcmd_JumpContinue, -2)
    BROTHER_EVT_ID(LW(14))
        USER_FUNC(btlevtcmd_ACRStart, -2, 0, 16, 16, 20)
    END_BROTHER()
    USER_FUNC(evt_btl_camera_shake_h, 0, 8, 0, 30, 13)
    BROTHER_EVT_ID(LW(15))
        USER_FUNC(evt_sub_intpl_init_float, 11, FLOAT(1.0), FLOAT(0.40), 10)
        DO(10)
            USER_FUNC(evt_sub_intpl_get_float, LW(5), EVT_NULLPTR)
            USER_FUNC(btlevtcmd_GetScale, -2, LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_SetScale, -2, LW(0), LW(5), LW(2))
            WAIT_FRM(1)
        WHILE()
        USER_FUNC(evt_sub_intpl_init_float, 11, FLOAT(0.40), FLOAT(1.0), 6)
        DO(6)
            USER_FUNC(evt_sub_intpl_get_float, LW(5), EVT_NULLPTR)
            USER_FUNC(btlevtcmd_GetScale, -2, LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_SetScale, -2, LW(0), LW(5), LW(2))
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()
    BROTHER_EVT()
        USER_FUNC(evt_sub_intpl_init_float, 11, FLOAT(1.0), FLOAT(1.5), 10)
        DO(10)
            USER_FUNC(evt_sub_intpl_get_float, LW(5), EVT_NULLPTR)
            USER_FUNC(btlevtcmd_GetScale, -2, LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_SetScale, -2, LW(5), LW(1), LW(2))
            WAIT_FRM(1)
        WHILE()
        USER_FUNC(evt_sub_intpl_init_float, 11, FLOAT(1.5), FLOAT(1.0), 6)
        DO(6)
            USER_FUNC(evt_sub_intpl_get_float, LW(5), EVT_NULLPTR)
            USER_FUNC(btlevtcmd_GetScale, -2, LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_SetScale, -2, LW(5), LW(1), LW(2))
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_WaitEventEnd, LW(14))
    USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
    IF_NOT_EQUAL(LW(6), 2)
        USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
        USER_FUNC(evt_audience_acrobat_notry)
        GOTO(95)
    END_IF()
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 20)
    ADD(LW(1), 20)
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.30))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 18, -1)
    USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, FLOAT(-60.0), 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PWD_Y_1"))
    WAIT_MSEC(500)
    USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
    LBL(95)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(6.0))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetMoveFrame, -2, LW(0), LW(1), LW(2), FLOAT(6.0), LW(15))
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), LW(15), 0, 4, 0, -1)
    LBL(99)
    USER_FUNC(evt_audience_ap_recovery)
    USER_FUNC(btlevtcmd_InviteApInfoReport)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RUN_CHILD_EVT(PTR(&btldefaultevt_SuitoruBadgeEffect))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(partyClaudaAttack_BreathAttack)
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(12))
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    WAIT_FRM(20)
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, -2)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 300)
    
    // Set Action Command based on weapon used.
    USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
    USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
    USER_FUNC(btlevtcmd_AcGetDifficulty, LW(1))
    SWITCH(LW(12))
        CASE_EQUAL(PTR(&customWeapon_FlurrieGaleForce))
            USER_FUNC(btlevtcmd_AcSetParamAll, 1, 300, 4, 40, 60, 80, 100, LW(1))
            USER_FUNC(btlevtcmd_SetupAC, -2, 14, 1, 0)
        CASE_EQUAL(PTR(&customWeapon_FlurrieBlizzard))
            USER_FUNC(btlevtcmd_AcSetParamAll, 1, 300, 4, 75, 100, 100, 100, LW(1))
            USER_FUNC(btlevtcmd_SetupAC, -2, 14, 1, 0)
        CASE_ETC()
            // Snow Whirled action command; 1 damage per cycle.
            SUB(LW(1), 3)
            MUL(LW(1), -10)
            ADD(LW(1), 180)
            USER_FUNC(btlevtcmd_AcSetParamAll, LW(1), 1, 4, -3, -417, EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_AcSetFlag, 7)
    END_SWITCH()
       
    SET(LW(15), -1)
    IF_NOT_EQUAL(LW(12), PTR(&customWeapon_FlurrieThunderStorm))
        IF_EQUAL(LW(12), PTR(&customWeapon_FlurrieGaleForce))
            USER_FUNC(_clauda_breath_effect_ready)
        END_IF()
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PWD_A2_1"))
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_CLAUD_BREATH1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_AnimeWaitPlayComplete, -2, 1)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PWD_A2_2"))
        USER_FUNC(evt_btl_camera_nomove_x_onoff, 0, 1)
        USER_FUNC(evt_btl_camera_nomove_y_onoff, 0, 1)
        USER_FUNC(evt_btl_camera_nomove_z_onoff, 0, 1)
        USER_FUNC(btlevtcmd_AnimeWaitPlayComplete, -2, 1)
        USER_FUNC(evt_btl_camera_set_mode, 0, 0)
        USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
        USER_FUNC(evt_btl_camera_set_zoom, 0, 0)
        USER_FUNC(evt_btl_camera_nomove_x_onoff, 0, 0)
        USER_FUNC(evt_btl_camera_nomove_y_onoff, 0, 0)
        USER_FUNC(evt_btl_camera_nomove_z_onoff, 0, 0)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PWD_A2_3"))
        INLINE_EVT()
            WAIT_MSEC(500)
            USER_FUNC(btlevtcmd_StageDispellFog)
        END_INLINE()
        
        IF_EQUAL(LW(12), PTR(&customWeapon_FlurrieGaleForce))
            USER_FUNC(_clauda_breath_effect_fire, LW(13))
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_CLAUD_BREATH2"), EVT_NULLPTR, 0, LW(14))
            BROTHER_EVT_ID(LW(15))
                SET(LW(14), -1)
                LBL(5)
                USER_FUNC(btlevtcmd_AcGetOutputParam, 1, LW(0))
                WAIT_FRM(1)
                GOTO(5)
            END_BROTHER()
        ELSE()
            // Ice Storm effect.
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
            USER_FUNC(evt_snd_sfxon_3d, PTR("SFX_ITEM_ICE_WIND1"), LW(0), LW(1), LW(2), LW(14))
            USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(3))
            USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(4))
            SET(LW(5), LW(3))
            MUL(LW(5), 10)
            ADD(LW(1), LW(5))
            MUL(LW(5), LW(4))
            MUL(LW(5), 2)
            ADD(LW(0), LW(5))
            ADD(LW(2), 20)
            IF_LARGE(LW(4), 0)
                SET(LW(5), 0)
            ELSE()
                SET(LW(5), 1)
            END_IF()
            USER_FUNC(evt_eff, 0, PTR("ibuki"), LW(5), LW(0), LW(1), LW(2), 400, 0, 0, 0, 0, 0, 0, 0)
        
            BROTHER_EVT()
                WAIT_FRM(400)
                USER_FUNC(evt_snd_sfxoff, LW(14))
            END_BROTHER()
            
            // Would be cool to have an effect to turn the enemies blue, but
            // it's not worth the extra complexity / space for now.
        END_IF()
    ELSE()
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PWD_A2_1"))
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_CLAUD_BREATH1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(evt_btl_camera_nomove_x_onoff, 0, 1)
        USER_FUNC(evt_btl_camera_nomove_y_onoff, 0, 1)
        USER_FUNC(evt_btl_camera_nomove_z_onoff, 0, 1)
        USER_FUNC(btlevtcmd_AnimeWaitPlayComplete, -2, 1)
        WAIT_FRM(30)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PWD_A4_2"))
        USER_FUNC(evt_btl_camera_set_mode, 0, 0)
        USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
        USER_FUNC(evt_btl_camera_set_zoom, 0, 0)
        USER_FUNC(evt_btl_camera_nomove_x_onoff, 0, 0)
        USER_FUNC(evt_btl_camera_nomove_y_onoff, 0, 0)
        USER_FUNC(evt_btl_camera_nomove_z_onoff, 0, 0)
        
        WAIT_FRM(10)
        USER_FUNC(evt_snd_sfxon, PTR("SFX_ITEM_QUAKE1"), LW(14))
        USER_FUNC(btlevtcmd_SetupAC, -2, 10, 1, 0)
        WAIT_FRM(20)
        
        USER_FUNC(evt_btl_camera_shake_h, 0, 1, 0, 10000, 0)
        
        BROTHER_EVT_ID(LW(15))
            LBL(6)
            USER_FUNC(evt_batstage_set_stg_dark, 80, 35, 0)
            WAIT_FRM(35)
            USER_FUNC(evt_batstage_set_stg_dark, 20, 35, 0)
            WAIT_FRM(35)
            GOTO(6)
        END_BROTHER()
    END_IF()
    
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(12))
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    LBL(8)
    IF_NOT_EQUAL(LW(3), -1)
        // Determine whether enemies should shake.
        USER_FUNC(evtTot_CheckEnemyShake, LW(3), LW(12), LW(0))
        IF_LARGE_EQUAL(LW(0), 1)
            USER_FUNC(btlevtcmd_DamageDirect, LW(3), LW(4), 0, 0, 3, 1)
        END_IF()
        USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
        GOTO(8)
    END_IF()
    
    USER_FUNC(btlevtcmd_StartAC, 1)
    USER_FUNC(btlevtcmd_ResultAC)
    
    IF_EQUAL(LW(12), PTR(&customWeapon_FlurrieThunderStorm))
        // Get rid of effects.
        USER_FUNC(evt_snd_sfxoff, LW(14))
        USER_FUNC(evt_batstage_return_stg_dark_base, 60, 0)
        USER_FUNC(evt_btl_camera_noshake, 0)
    END_IF()
    IF_NOT_EQUAL(LW(15), -1)
        DELETE_EVT(LW(15))
    END_IF()
    
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    USER_FUNC(btlevtcmd_StopAC)
    SET(LW(11), 99)    
    
    // Make weapon structs / set AC prize level based on AC output params.
    USER_FUNC(btlevtcmd_AcGetOutputParam, 2, LW(0))
    SET(LW(8), -1)
    SWITCH(LW(12))
        CASE_EQUAL(PTR(&customWeapon_FlurrieGaleForce))
            USER_FUNC(evtTot_MakeBreathWeapon, LW(12), LW(0), MoveType::FLURRIE_GALE_FORCE)
            IF_LARGE_EQUAL(LW(0), 1)
                SET(LW(8), 0)
            END_IF()
        CASE_EQUAL(PTR(&customWeapon_FlurrieBlizzard))
            USER_FUNC(evtTot_MakeBreathWeapon, LW(12), LW(0), MoveType::FLURRIE_BLIZZARD)
            IF_LARGE_EQUAL(LW(0), 75)
                SET(LW(8), 0)
            END_IF()
        CASE_ETC()
            USER_FUNC(btlevtcmd_AcGetOutputParam, 1, LW(0))
            IF_LARGE_EQUAL(LW(0), 4)
                // Prize level = Nice for 4, Good for 6, Great for 8...
                SET(LW(8), LW(0))
                SUB(LW(8), 4)
                DIV(LW(8), 2)
                IF_LARGE(LW(8), 4)
                    SET(LW(8), 4)
                END_IF()
            END_IF()
    END_SWITCH()
    
    WAIT_FRM(60)
    
    IF_EQUAL(LW(12), PTR(&customWeapon_FlurrieThunderStorm))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PWD_A4_3"))
    END_IF()
    
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(12))
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    SET(LW(10), 0)
    LBL(10)
    
    // Thunderbolt effect.
    IF_EQUAL(LW(12), PTR(&customWeapon_FlurrieThunderStorm))
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetWidth, LW(3), LW(5))
        USER_FUNC(btlevtcmd_GetHeight, LW(3), LW(6))
        USER_FUNC(evt_sub_random, 2, LW(7))
        ADD(LW(7), 3)
        USER_FUNC(
            evt_eff64, PTR(""), PTR("thunder_n64"), 
            LW(7), LW(0), LW(1), LW(2), LW(5), LW(6), 0, 0, 0, 0, 0, 0)
        USER_FUNC(evt_snd_sfxon_3d, PTR("SFX_ITEM_ICE_THUNDER_FALL1"), LW(0), LW(1), LW(2), 0)
        USER_FUNC(evt_btl_camera_shake_h, 0, 3, 0, 25, 13)
        USER_FUNC(evt_sub_rumble_onoff, 0, 0)
        WAIT_MSEC(83)
        BROTHER_EVT()
            WAIT_MSEC(167)
            USER_FUNC(evt_sub_rumble_onoff, 1, 0)
        END_BROTHER()
    END_IF()
    
    USER_FUNC(btlevtcmd_SetDispOffset, LW(3), 0, 0, 0)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), LW(12), 256, LW(6))
    IF_NOT_EQUAL(LW(6), 1)
        IF_EQUAL(LW(6), 3)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 38)
            GOTO(51)
        END_IF()
        IF_EQUAL(LW(6), 6)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 39)
            GOTO(51)
        END_IF()
        IF_EQUAL(LW(6), 2)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 40)
            GOTO(51)
        END_IF()
        GOTO(50)
    END_IF()
    
    IF_LARGE_EQUAL(LW(8), 0)
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 131328, LW(5))
        
        // Only show 'Nice' effect for Gale Force if the status procs.
        IF_EQUAL(LW(12), PTR(&customWeapon_FlurrieGaleForce))
            USER_FUNC(btlevtcmd_GetDamageCode, LW(3), LW(0))
            IF_EQUAL(LW(0), 29)
                SET(LW(0), 0)
            ELSE()
                SET(LW(0), -1)
            END_IF()
        ELSE()
            SET(LW(0), LW(8))
        END_IF()

        IF_LARGE_EQUAL(LW(0), 0)
            USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), LW(0), LW(6))
            USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), LW(6))
        END_IF()
    ELSE()
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 256, LW(5))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), -1)
    END_IF()
    LBL(50)
    WAIT_MSEC(300)
    LBL(51)
    SUB(LW(11), 1)
    IF_LARGE_EQUAL(LW(11), 1)
        LBL(52)
        USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
        IF_NOT_EQUAL(LW(3), -1)
            ADD(LW(10), 1)
            GOTO(10)
        END_IF()
    END_IF()
    LBL(80)
    // Got rid of wait here so new attacks end a bit earlier.
    LBL(90)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    
    IF_EQUAL(LW(12), PTR(&customWeapon_FlurrieGaleForce))
        // Retained wait after Gale Force, but made it a bit shorter.
        WAIT_MSEC(1000)
        USER_FUNC(evt_snd_sfxoff, LW(14))
        USER_FUNC(evt_eff_delete_ptr, LW(13))
    END_IF()
        
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 43)
    USER_FUNC(btlevtcmd_ACRStart, -2, 0, 25, 25, 20)
    USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
    IF_NOT_EQUAL(LW(6), 2)
        USER_FUNC(evt_audience_acrobat_notry)
        WAIT_MSEC(500)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, 0, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PWD_Y_1"))
    WAIT_MSEC(1000)
    LBL(99)
    USER_FUNC(evt_audience_ap_recovery)
    USER_FUNC(btlevtcmd_InviteApInfoReport)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RUN_CHILD_EVT(PTR(&btldefaultevt_SuitoruBadgeEffect))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(partyClaudaAttack_PredationAttack)
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
    USER_FUNC(evt_btl_camera_set_zoom, 0, 300)
    USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
    USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
    USER_FUNC(btlevtcmd_AcGetDifficulty, LW(0))
    SUB(LW(0), 3)
    SWITCH(LW(0))
        CASE_SMALL_EQUAL(-3)
            SET(LW(0), 120)
            SET(LW(1), 0)
        CASE_EQUAL(-2)
            SET(LW(0), 100)
            SET(LW(1), 0)
        CASE_EQUAL(-1)
            SET(LW(0), 80)
            SET(LW(1), 1)
        CASE_EQUAL(0)
            SET(LW(0), 70)
            SET(LW(1), 2)
        CASE_EQUAL(1)
            SET(LW(0), 66)
            SET(LW(1), 2)
        CASE_EQUAL(2)
            SET(LW(0), 62)
            SET(LW(1), 2)
        CASE_ETC()
            SET(LW(0), 58)
            SET(LW(1), 2)
    END_SWITCH()
    // Change gauge parameters based on move level.
    USER_FUNC(evtTot_GetMoveSelectedLevel, MoveType::FLURRIE_LIP_LOCK, LW(5))
    SWITCH(LW(5))
        CASE_EQUAL(1)
            USER_FUNC(btlevtcmd_AcSetParamAll, 0, LW(0), LW(1), 3, 3, EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_AcSetGaugeParam, 100, 100, 100, 100)
        CASE_EQUAL(2)
            USER_FUNC(btlevtcmd_AcSetParamAll, 0, LW(0), LW(1), 5, 3, EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_AcSetGaugeParam, 60, 100, 100, 100)
        CASE_ETC()
            USER_FUNC(btlevtcmd_AcSetParamAll, 0, LW(0), LW(1), 7, 3, EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_AcSetGaugeParam, 42, 71, 100, 100)
    END_SWITCH()
    USER_FUNC(btlevtcmd_AcSetFlag, 0)
    USER_FUNC(btlevtcmd_SetupAC, -2, 17, 1, 60)
    WAIT_FRM(22)
    USER_FUNC(btlevtcmd_CommandPreCheckDamage, -2, LW(3), LW(4), 256, LW(5))
    IF_EQUAL(LW(5), 5)
        USER_FUNC(btlevtcmd_GetStatusMg, LW(3), LW(6))
        MULF(LW(6), 40)
        USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), LW(6))
        USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(0), LW(2), 1, LW(15))
        USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
        USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.0))
        USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 40, 0, 5, 0, -1)
        USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 256, LW(5))
    END_IF()
    USER_FUNC(_get_clauda_kiss_hit_position, -2, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(6))
    MULF(LW(6), 20)
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), LW(6))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(0), LW(2), 1, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    ADD(LW(2), 5)
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 40, 0, 5, 0, -1)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PWD_A3_1"))
    USER_FUNC(btlevtcmd_AnimeWaitPlayComplete, -2, 1)
    USER_FUNC(btlevtcmd_CommandPreCheckDamage, -2, LW(3), LW(4), 256, LW(5))
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
        IF_EQUAL(LW(5), 3)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 38)
        END_IF()
        IF_EQUAL(LW(5), 6)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 39)
        END_IF()
        IF_EQUAL(LW(5), 2)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 40)
        END_IF()
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PWD_A3_2"))
        USER_FUNC(btlevtcmd_AnimeWaitPlayComplete, -2, 1)
        GOTO(90)
    END_IF()
    USER_FUNC(btlevtcmd_CommandPreCheckCounter, -2, LW(3), LW(4), 256, LW(0))
    SWITCH(LW(0))
        CASE_OR(7)
        CASE_OR(8)
        CASE_OR(9)
        CASE_OR(10)
        CASE_OR(11)
        CASE_OR(12)
        CASE_OR(13)
        CASE_OR(14)
        CASE_OR(15)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PWD_A3_2"))
            USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 256, LW(5))
            CASE_END()
    END_SWITCH()
    USER_FUNC(btlevtcmd_OffUnitFlag, -2, 2097152)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_CLAUD_KISS1"), EVT_NULLPTR, 0, LW(14))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PWD_A3_2"))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(evt_eff, PTR(""), PTR("kiss"), 2, LW(0), LW(1), LW(2), LW(6), 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(btlevtcmd_DamageDirect, LW(3), LW(4), 0, 0, 2, 1)
    USER_FUNC(btlevtcmd_StartAC, 1)
    BROTHER_EVT_ID(LW(15))
        USER_FUNC(btlevtcmd_SetRGB, LW(3), LW(4), 255, 255, 255)
        DO(45)
            USER_FUNC(btlevtcmd_GetRGB, LW(3), LW(4), LW(0), LW(1), LW(2))
            SUB(LW(1), 2)
            IF_SMALL(LW(1), 190)
                SET(LW(1), 190)
            END_IF()
            SUB(LW(2), 4)
            USER_FUNC(btlevtcmd_SetRGB, LW(3), LW(4), LW(0), LW(1), LW(2))
            WAIT_FRM(1)
        WHILE()
        DO(45)
            USER_FUNC(btlevtcmd_GetRGB, LW(3), LW(4), LW(0), LW(1), LW(2))
            SUB(LW(0), 2)
            SUB(LW(1), 2)
            ADD(LW(2), 3)
            IF_SMALL(LW(1), 0)
                SET(LW(1), 0)
            END_IF()
            USER_FUNC(btlevtcmd_SetRGB, LW(3), LW(4), LW(0), LW(1), LW(2))
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_ResultAC)
    USER_FUNC(evt_snd_sfxoff, LW(14))
    DELETE_EVT(LW(15))
    USER_FUNC(btlevtcmd_GetResultAC, LW(9))
    IF_FLAG(LW(9), 0x2)
        USER_FUNC(btlevtcmd_AcGetOutputParam, 0, LW(8))
        IF_LARGE_EQUAL(LW(8), 3)
            // Set AC result to Nice for 4, Good for 6, Great for 8 damage.
            SUB(LW(8), 3)
            DIV(LW(8), 2)
            USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), LW(8), LW(6))
            USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), LW(6))
        ELSE()
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), -1)
        END_IF()
    ELSE()
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), -1)
    END_IF()
    USER_FUNC(btlevtcmd_OnUnitFlag, -2, 2097152)
    USER_FUNC(btlevtcmd_SetRGB, LW(3), LW(4), 255, 255, 255)
    USER_FUNC(btlevtcmd_AcGetOutputParam, 0, LW(0))
    USER_FUNC(_make_kiss_weapon, LW(12), LW(0))
    IF_FLAG(LW(9), 0x2)
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 131328, LW(5))
    ELSE()
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 256, LW(5))
    END_IF()
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_GetHpDamage, LW(3), LW(0))
        WAIT_FRM(50)
        IF_LARGE_EQUAL(LW(0), 1)
            USER_FUNC(btlevtcmd_GetPos, -2, LW(5), LW(6), LW(7))
            USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(8))
            MULF(LW(8), 40)
            ADD(LW(6), LW(8))
            USER_FUNC(btlevtcmd_RecoverHp, -2, 1, LW(0))
            USER_FUNC(evt_eff, PTR(""), PTR("recovery"), 7, LW(5), LW(6), LW(7), LW(0), 0, 0, 0, 0, 0, 0, 0)
        END_IF()
    END_BROTHER()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_CLAUD_KISS2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PWD_A3_3"))
    WAIT_FRM(5)
    USER_FUNC(btlevtcmd_ACRStart, -2, 0, 29, 29, 0)
    USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
    IF_EQUAL(LW(6), 2)
        USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, 0, 0, 0)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PWD_A_7"))
        WAIT_FRM(56)
    ELSE()
        USER_FUNC(evt_audience_acrobat_notry)
        WAIT_FRM(60)
    END_IF()
    LBL(90)
    USER_FUNC(btlevtcmd_StopAC)
    USER_FUNC(evt_audience_ap_recovery)
    USER_FUNC(btlevtcmd_InviteApInfoReport)
    LBL(95)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 40, 0, 5, 0, -1)
    LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RUN_CHILD_EVT(PTR(&btldefaultevt_SuitoruBadgeEffect))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(partyClaudaAttack_KumoGuard)
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
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), EVT_NULLPTR, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(1), EVT_NULLPTR, EVT_NULLPTR)
    ADD(LW(0), LW(1))
    DIV(LW(0), 2)
    USER_FUNC(evt_btl_camera_set_mode, 0, 3)
    USER_FUNC(evt_btl_camera_set_moveto, 1, LW(0), 72, 400, LW(0), 42, 0, 30, 5)
    USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
    USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
    USER_FUNC(btlevtcmd_AcSetParamAll, 3, 0, 40, 0, 1, 1, 2, 8)
    USER_FUNC(btlevtcmd_AcSetFlag, 5)
    USER_FUNC(btlevtcmd_SetupAC, -2, 18, 1, 0)
    WAIT_FRM(1)
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PWD_A4_2"))
    USER_FUNC(btlevtcmd_StartAC, 1)
    USER_FUNC(btlevtcmd_ResultAC)
    WAIT_FRM(60)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PWD_A4_3"))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_CLAUD_HIDE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    WAIT_MSEC(150)
    USER_FUNC(btlevtcmd_GetResultAC, LW(6))
    USER_FUNC(btlevtcmd_StopAC)
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHeight, LW(3), LW(7))
    DIV(LW(7), 2)
    ADD(LW(1), LW(7))
    ADD(LW(2), 20)
    USER_FUNC(evt_eff, PTR(""), PTR("kemuri_test"), 8, LW(0), LW(1), LW(2), FLOAT(1.0), 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(btlevtcmd_AcGetOutputParam, 0, LW(0))
    ADD(LW(0), 1)
    USER_FUNC(_make_kumoguard_weapon, LW(12), LW(0))
    USER_FUNC(btlevtcmd_AcGetOutputParam, 0, LW(0))
    IF_LARGE_EQUAL(LW(0), 1)
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 131328, LW(5))
        SUB(LW(0), 1)
        USER_FUNC(btlevtcmd_GetResultPrizeLv, -2, LW(0), LW(6))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), LW(6))
    ELSE()
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 256, LW(5))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), -1)
    END_IF()
    USER_FUNC(btlevtcmd_ACRStart, -2, 0, 29, 29, 20)
    USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
    IF_EQUAL(LW(6), 2)
        USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, 0, 0, 0)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PWD_A_7"))
        WAIT_MSEC(1000)
    ELSE()
        USER_FUNC(evt_audience_acrobat_notry)
        WAIT_MSEC(1000)
    END_IF()
    LBL(99)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_audience_ap_recovery)
    USER_FUNC(btlevtcmd_InviteApInfoReport)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    // Also apply Dodgy status to Flurrie herself.
    USER_FUNC(btlevtcmd_CheckDamage, -2, -2, 1, LW(12), 256, LW(5))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

BattleWeapon customWeapon_FlurrieBodySlam = {
    .name = "btl_wn_pwd_normal",
    .icon = IconType::PARTNER_MOVE_0,
    .item_id = 0,
    .description = "msg_pwd_body_press",
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
    .damage_function_params = { 2, 3, 3, 5, 4, 8, 0, MoveType::FLURRIE_BASE },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags =
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0xa,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_body_press",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::USABLE_IF_CONFUSED |
        AttackSpecialProperty_Flags::GROUNDS_WINGED |
        AttackSpecialProperty_Flags::FLIPS_SHELLED |
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags =
        AttackCounterResistance_Flags::FRONT_SPIKY |
        AttackCounterResistance_Flags::PREEMPTIVE_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)partyClaudaAttack_NormalAttack,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 20,
    .bg_a2_fall_weight = 20,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 20,
    .nozzle_turn_chance = 10,
    .nozzle_fire_chance = 2,
    .ceiling_fall_chance = 5,
    .object_fall_chance = 10,
};

BattleWeapon customWeapon_FlurrieGaleForce = {
    .name = "btl_wn_pwd_lv1",
    .icon = IconType::PARTNER_MOVE_1,
    .item_id = 0,
    .description = "msg_pwd_breath",
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
    .damage_pattern = 0x19,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_breath",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::UNKNOWN_GALE_FORCE |
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    .gale_force_chance = 100,
    
    .attack_evt_code = (void*)partyClaudaAttack_BreathAttack,
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

BattleWeapon customWeapon_FlurrieLipLock = {
    .name = "btl_wn_pwd_lv2",
    .icon = IconType::PARTNER_MOVE_1,
    .item_id = 0,
    .description = "msg_pwd_sexy_kiss",
    .base_accuracy = 100,
    .base_fp_cost = 3,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)weaponGetACOutputParam,  // param0 + 1
    .damage_function_params = { 0, 1, 0, 0, 0, 0, 0, 0 },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 3, 3, 0, 0, 0, 0, 0, 0 },
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
    .ac_help_msg = "msg_ac_sexy_kiss",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = 
        AttackCounterResistance_Flags::TOP_SPIKY |
        // Additional resistances.
        AttackCounterResistance_Flags::FRONT_SPIKY |
        AttackCounterResistance_Flags::FIERY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)partyClaudaAttack_PredationAttack,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 0,
    .nozzle_fire_chance = 5,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

BattleWeapon customWeapon_FlurrieDodgyFog = {
    .name = "btl_wn_pwd_lv3",
    .icon = IconType::PARTNER_MOVE_1,
    .item_id = 0,
    .description = "msg_pwd_kumogakure",
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
    .ac_help_msg = "msg_ac_kumogakure",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::CANNOT_MISS,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    .dodgy_chance = 100,
    .dodgy_time = 2,
    
    .attack_evt_code = (void*)partyClaudaAttack_KumoGuard,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 0,
    .nozzle_fire_chance = 5,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

BattleWeapon customWeapon_FlurrieBlizzard = {
    .name = "tot_ptr3_blizzard",
    .icon = IconType::PARTNER_MOVE_2,
    .item_id = 0,
    .description = "tot_ptr3_blizzard_desc",
    .base_accuracy = 100,
    .base_fp_cost = 4,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    // Will be overridden by evtTot_MakeBlizzardWeapon.
    .damage_function = (void*)weaponGetPowerDefault,
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
    .element = AttackElement::ICE,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_breath",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    .freeze_chance = 100,
    .freeze_time = 1,
    
    .attack_evt_code = (void*)partyClaudaAttack_BreathAttack,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 20,
    .nozzle_fire_chance = 10,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

BattleWeapon customWeapon_FlurrieThunderStorm = {
    .name = "tot_ptr3_thunder",
    .icon = IconType::PARTNER_MOVE_3,
    .item_id = 0,
    .description = "tot_ptr3_thunder_desc",
    .base_accuracy = 100,
    .base_fp_cost = 4,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    // Use AC output param 1 (number of Snow Whirled rotations).
    .damage_function = (void*)weaponGetACOutputParam,
    .damage_function_params = { 1, 0, 0, 0, 0, 0, 0, 0 },
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
    .element = AttackElement::ELECTRIC,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "tot_ptr3_thunder_ac",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING |
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)partyClaudaAttack_BreathAttack,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 10,
    .bg_a2_fall_weight = 10,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 10,
    .nozzle_turn_chance = 20,
    .nozzle_fire_chance = 10,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 10,
};

}  // namespace mod::tot::party_flurrie