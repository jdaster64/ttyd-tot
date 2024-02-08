#include "tot_party_yoshi.h"

#include "custom_item.h"
#include "evt_cmd.h"
#include "mod_state.h"
#include "tot_move_manager.h"

#include <ttyd/battle.h>
#include <ttyd/battle_camera.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_event_default.h>
#include <ttyd/battle_icon.h>
#include <ttyd/battle_message.h>
#include <ttyd/battle_sub.h>
#include <ttyd/battle_unit.h>
#include <ttyd/battle_weapon_power.h>
#include <ttyd/evt_audience.h>
#include <ttyd/evt_eff.h>
#include <ttyd/evt_env.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evt_sub.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/icondrv.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/msgdrv.h>
#include <ttyd/npcdrv.h>
#include <ttyd/unit_party_yoshi.h>

namespace mod::tot::party_yoshi {

namespace {
    
// Including entire namespaces for convenience.
using namespace ::ttyd::battle_camera;
using namespace ::ttyd::battle_database_common;
using namespace ::ttyd::battle_event_cmd;
using namespace ::ttyd::battle_event_default;
using namespace ::ttyd::battle_icon;
using namespace ::ttyd::battle_message;
using namespace ::ttyd::battle_weapon_power;
using namespace ::ttyd::evt_audience;
using namespace ::ttyd::evt_eff;
using namespace ::ttyd::evt_env;
using namespace ::ttyd::evt_snd;
using namespace ::ttyd::evt_sub;
using namespace ::ttyd::unit_party_yoshi;

using ::mod::infinite_pit::PickRandomItem;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

namespace IconType = ::ttyd::icondrv::IconType;
namespace ItemType = ::ttyd::item_data::ItemType;

}  // namespace

// Declaration of weapon structs.
extern BattleWeapon customWeapon_YoshiGroundPound;
extern BattleWeapon customWeapon_YoshiGulp_Shot;
extern BattleWeapon customWeapon_YoshiGulp_Spew;
extern BattleWeapon customWeapon_YoshiGulp_Dmg0;
extern BattleWeapon customWeapon_YoshiGulp_Fire;
extern BattleWeapon customWeapon_YoshiGulp_Recoil;
extern BattleWeapon customWeapon_YoshiMiniEgg;
extern BattleWeapon customWeapon_YoshiStampede;
extern BattleWeapon customWeapon_YoshiEggBarrage;
extern BattleWeapon customWeapon_YoshiSwallow;

BattleWeapon* g_WeaponTable[] = {
    &customWeapon_YoshiGroundPound, &customWeapon_YoshiEggBarrage,
    &customWeapon_YoshiGulp_Shot, &customWeapon_YoshiMiniEgg,
    &customWeapon_YoshiSwallow, &customWeapon_YoshiStampede
};

void MakeSelectWeaponTable(
    ttyd::battle::BattleWorkCommand* command_work, int32_t* num_options) {
    for (int32_t i = 0; i < 6; ++i) {
        auto& weapon_entry = command_work->weapon_table[*num_options];
        BattleWeapon* weapon = g_WeaponTable[i];
        
        weapon_entry.index = MoveType::YOSHI_BASE + i;
        weapon_entry.item_id = 0;
        weapon_entry.weapon = weapon;
        weapon_entry.icon = weapon->icon;
        weapon_entry.unk_04 = 0;
        weapon_entry.unk_18 = 0;
        weapon_entry.name = ttyd::msgdrv::msgSearch(weapon->name);
        
        ++*num_options;
    }
}

EVT_DECLARE_USER_FUNC(evtTot_GetEggLayItem, 2)
EVT_DEFINE_USER_FUNC(evtTot_GetEggLayItem) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    int32_t id = evtGetValue(evt, evt->evtArguments[0]);
    id = ttyd::battle_sub::BattleTransID(evt, id);
    auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, id);
    
    int32_t item = unit->held_item;
    // No held item; pick a random item to get;
    // 30% chance of item (20% normal, 10% recipe), 10% badge, 60% coin.
    if (!item) {
        item = PickRandomItem(infinite_pit::RNG_KISS_THIEF, 20, 10, 10, 60);
        if (!item) item = ItemType::COIN;
    }
    if (item == ItemType::GOLD_BAR_X3 || !ttyd::mario_pouch::pouchGetItem(item)) {
        // Item = Shine Sprite (can't be stolen), or player's inventory is full.
        evtSetValue(evt, evt->evtArguments[1], ItemType::COIN);
    } else {
        // Remove the corresponding held/stolen item from the NPC setup,
        // if this was one of the initial enemies in the loadout.
        if (!ttyd::battle_unit::BtlUnit_CheckUnitFlag(unit, 0x40000000)) {
            if (unit->group_index >= 0) {
                battleWork->fbat_info->wBattleInfo->wHeldItems
                    [unit->group_index] = 0;
            }
        } else {
            ttyd::battle_unit::BtlUnit_OffUnitFlag(unit, 0x40000000);
            if (unit->group_index >= 0) {
                auto* npc_battle_info = battleWork->fbat_info->wBattleInfo;
                npc_battle_info->wHeldItems[unit->group_index] = 0;
                npc_battle_info->wStolenItems[unit->group_index] = 0;
            }
        }
        
        evtSetValue(evt, evt->evtArguments[1], item);
    }
    return 2;
}

EVT_DECLARE_USER_FUNC(evtTot_SetGulpStruggleParam, 2)
EVT_DEFINE_USER_FUNC(evtTot_SetGulpStruggleParam) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    int32_t id = evtGetValue(evt, evt->evtArguments[0]);
    id = ttyd::battle_sub::BattleTransID(evt, id);
    auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, id);
    
    int32_t param = evtGetValue(evt, evt->evtArguments[1]);
    
    if (unit->size_change_strength < 0 && unit->size_change_turns > 0) {
        // Enemy struggles considerably less if Tiny.
        param /= 2;
    } else {
        // Enemy struggles more at high HP.
        float factor = (float)unit->current_hp / unit->max_hp;
        if (factor < 0.5f) factor = 0.5f;
        param *= (factor * 2);
    }
        
    evtSetValue(evt, evt->evtArguments[1], param);
    return 2;
}

EVT_BEGIN(partyYoshiAttack_NormalAttack)
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
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, -1)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    WAIT_FRM(20)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 2, 1)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 3, -1)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 4, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 5, 100)
    USER_FUNC(btlevtcmd_AnimeSetPoseSoundTable, -2, 1, PTR(&posesound_normal_attack))
    USER_FUNC(btlevtcmd_AcSetOutputParam, 1, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_W_1"))
    BROTHER_EVT()
        SET(LW(0), 0)
        DO(25)
            ADD(LW(0), 4)
            SET(LW(1), 2387)
            MUL(LW(1), LW(0))
            DIV(LW(1), 100)
            USER_FUNC(btlevtcmd_SetUnitWork, -2, 4, LW(1))
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_R_5"))
    WAIT_FRM(25)
    USER_FUNC(evt_btl_camera_set_mode, 0, 11)
    USER_FUNC(evt_btl_camera_set_homing_unitparts, 0, -2, 1, LW(3), LW(4))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 350)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(4.0))
    USER_FUNC(btlevtcmd_GetTakeoffPosition, -2, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(0), LW(2), 1, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_J_1A"))
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.30))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 25)
    ADD(LW(1), 35)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_PARTY_BATTLE_JUMP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    BROTHER_EVT_ID(LW(15))
        USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(15))
        USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(15))
        USER_FUNC(evtTot_GetMoveSelectedLevel, MoveType::YOSHI_BASE, LW(0))
        SWITCH(LW(0))
            CASE_EQUAL(3)
                USER_FUNC(btlevtcmd_AcSetParamAll, 5, 180, 178, 23, 25, 20, 61, 1)
                USER_FUNC(btlevtcmd_AcSetGaugeParam, 60, 80, 100, 100)
            CASE_EQUAL(2)
                USER_FUNC(btlevtcmd_AcSetParamAll, 5, 180, 178, 23, 25, 25, 76, 1)
                USER_FUNC(btlevtcmd_AcSetGaugeParam, 75, 100, 100, 100)
            CASE_ETC()
                USER_FUNC(btlevtcmd_AcSetParamAll, 5, 180, 178, 23, 25, 34, 100, 1)
                USER_FUNC(btlevtcmd_AcSetGaugeParam, 100, 100, 100, 100)
        END_SWITCH()
        USER_FUNC(btlevtcmd_AcSetFlag, 72)
        USER_FUNC(btlevtcmd_SetupAC, -2, 6, 1, 0)
    END_BROTHER()
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 25, -1)
    USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_A_3"))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(6), LW(7), LW(8))
    ADD(LW(7), 40)
    SET(LW(10), FLOAT(1.0))
    BROTHER_EVT_ID(LW(15))
        USER_FUNC(btlevtcmd_StartAC, 1)
        USER_FUNC(btlevtcmd_ResultAC)
    END_BROTHER()
    USER_FUNC(btl_yoshi_yoroyoro_jump_calc_param, -2, LW(6), LW(7), LW(8), LW(10), LW(9))
    USER_FUNC(btl_yoshi_yoroyoro_jump_move, -2, LW(6), LW(7), LW(8), LW(10), LW(9))
    USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
    SET(LW(13), 1)
    USER_FUNC(btlevtcmd_AcGetOutputParam, 2, LW(14))
    LBL(50)
    IF_LARGE_EQUAL(LW(13), 2)
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
    END_SWITCH()
    USER_FUNC(btlevtcmd_AnimeSetPoseSoundTable, -2, 1, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 2, 0)
    IF_SMALL(LW(13), 2)
        USER_FUNC(btlevtcmd_ACRStart, -2, 0, 20, 20, 0)
        USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
    ELSE()
        SET(LW(6), 1)
    END_IF()
    IF_EQUAL(LW(13), 1)
        USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(15))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_J_1B"))
        IF_EQUAL(LW(6), 2)
            SET(LW(0), 3)
            SET(LW(1), 6)
            MUL(LW(15), -60)
        ELSE()
            SET(LW(0), 1)
            SET(LW(1), 9)
            MUL(LW(15), -40)
        END_IF()
        USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 12, 0)
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
        WAIT_FRM(15)
        IF_EQUAL(LW(6), 2)
            USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, -60, -25, 0)
        END_IF()
        DO(LW(0))
            SET(LW(2), LW(1))
            USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_YOSHI_ATTACK1"), 0)
            DO(LW(2))
                USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, LW(15))
                WAIT_FRM(1)
            WHILE()
        WHILE()
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
        USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 0, 0)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_A_1"))
        WAIT_FRM(15)
    ELSE()
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_A_1"))
    END_IF()
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 4, 0, 0, 0, -1)
    IF_NOT_EQUAL(LW(5), 1)
        USER_FUNC(btlevtcmd_StopAC)
        IF_EQUAL(LW(5), 3)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 38)
        END_IF()
        IF_EQUAL(LW(5), 6)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 39)
        END_IF()
        IF_EQUAL(LW(5), 2)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 40)
        END_IF()
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        SET(LW(15), LW(1))
        USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
        SUB(LW(15), LW(1))
        DIV(LW(15), 10)
        USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), LW(15), 0, 0, 0, -1)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_D_1"))
        USER_FUNC(btlevtcmd_SetDispOffset, -2, 0, -7, 0)
        USER_FUNC(evt_btl_camera_shake_h, 0, 4, 0, 10, 13)
        WAIT_FRM(40)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_A_5"))
        USER_FUNC(btlevtcmd_SetDispOffset, -2, 0, 0, 0)
        WAIT_FRM(54)
        USER_FUNC(evt_audience_ap_recovery)
        USER_FUNC(btlevtcmd_InviteApInfoReport)
        GOTO(95)
    END_IF()
    USER_FUNC(btlevtcmd_ac_timing_flag_onoff, 1, 1)
    WAIT_FRM(2)
    USER_FUNC(btlevtcmd_ac_timing_flag_onoff, 0, 1)
    USER_FUNC(btlevtcmd_GetResultAC, LW(0))
    IF_FLAG(LW(0), 0x2)
        IF_LARGE_EQUAL(LW(13), LW(14))
            USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 131328, LW(5))
        ELSE()
            USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 131072, LW(5))
        END_IF()
        USER_FUNC(evtTot_GetMoveSelectedLevel, MoveType::YOSHI_BASE, LW(0))
        // Set AC result based on number of hits, regardless of level.
        USER_FUNC(btlevtcmd_AcGetOutputParam, 2, LW(0))
        SWITCH(LW(0))
            CASE_LARGE_EQUAL(6)
                SET(LW(0), 2)
            CASE_LARGE_EQUAL(4)
                SET(LW(0), 1)
            CASE_LARGE_EQUAL(2)
                SET(LW(0), 0)
            CASE_ETC()
                SET(LW(0), -1)
        END_SWITCH()
        IF_LARGE_EQUAL(LW(0), 0)
            SET(LW(1), LW(0))
            SET(LW(2), LW(13))
            SUB(LW(2), 1)
            IF_LARGE(LW(1), LW(2))
                SET(LW(1), LW(2))
            END_IF()
            USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), LW(1), LW(6))
            USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(9), LW(1), LW(2))
            USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(9), LW(1), LW(2))
            SET(LW(1), LW(13))
            ADD(LW(1), 1)
            IF_LARGE(LW(1), LW(14))
                USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), LW(0), LW(6))
                USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), LW(6))
            END_IF()
        ELSE()
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), -1)
        END_IF()
    ELSE()
        IF_LARGE_EQUAL(LW(13), LW(14))
            USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 256, LW(5))
        ELSE()
            USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 0, LW(5))
        END_IF()
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), -1)
    END_IF()
    LBL(90)
    ADD(LW(13), 1)
    IF_SMALL_EQUAL(LW(13), LW(14))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.20))
        ADD(LW(1), 40)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, -1)
        GOTO(50)
    END_IF()
    USER_FUNC(btlevtcmd_StopAC)
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.70))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHomePos, -2, EVT_NULLPTR, LW(1), EVT_NULLPTR)
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 60)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 30, -1)
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_ACRStart, -2, 0, 15, 15, 20)
    END_BROTHER()
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 25)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 16, -1)
    USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
    SWITCH(LW(6))
        CASE_LARGE_EQUAL(2)
            USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
            USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(0), 1, 0, 0, 0)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_Y_1"))
            USER_FUNC(btlevtcmd_AnimeWaitPlayComplete, -2, 1)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_Y_1"))
            USER_FUNC(btlevtcmd_AnimeWaitPlayComplete, -2, 1)
        CASE_ETC()
            USER_FUNC(evt_audience_acrobat_notry)
    END_SWITCH()
    USER_FUNC(evt_audience_ap_recovery)
    USER_FUNC(btlevtcmd_InviteApInfoReport)
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

EVT_BEGIN(partyYoshiAttack_Nomikomi)
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 1, 0)
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_CommandPayWeaponCost, -2)
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(15))
    USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(15))
    USER_FUNC(btlevtcmd_AcGetDifficulty, LW(0))
    SUB(LW(0), 3)
    SWITCH(LW(0))
        CASE_SMALL_EQUAL(-3)
            SET(LW(0), 160)
            SET(LW(1), 40)
        CASE_EQUAL(-2)
            SET(LW(0), 140)
            SET(LW(1), 30)
        CASE_EQUAL(-1)
            SET(LW(0), 120)
            SET(LW(1), 25)
        CASE_EQUAL(0)
            SET(LW(0), 100)
            SET(LW(1), 20)
        CASE_EQUAL(1)
            SET(LW(0), 80)
            SET(LW(1), 10)
        CASE_EQUAL(2)
            SET(LW(0), 50)
            SET(LW(1), 5)
        CASE_ETC()
            SET(LW(0), 30)
            SET(LW(1), 2)
    END_SWITCH()
    USER_FUNC(btlevtcmd_AcSetParamAll, 3, LW(0), LW(1), 0, 100, EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AcSetFlag, 15)
    USER_FUNC(btlevtcmd_SetupAC, -2, 15, 1, 0)
    USER_FUNC(evt_btl_camera_set_mode, 0, 10)
    USER_FUNC(evt_btl_camera_set_homing_unitparts, 0, -2, 1, -1, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    WAIT_FRM(22)
    USER_FUNC(btlevtcmd_StartAC, 1)
    LBL(5)
    USER_FUNC(btlevtcmd_AcGetOutputParam, 0, LW(0))
    IF_EQUAL(LW(0), 0)
        WAIT_FRM(1)
        GOTO(5)
    END_IF()
    USER_FUNC(btlevtcmd_CommandPreCheckDamage, -2, LW(3), LW(4), 256, LW(5))
    IF_EQUAL(LW(5), 5)
        USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(6))
        MULF(LW(6), 40)
        USER_FUNC(_get_nomikomi_hit_position, -2, LW(3), LW(4), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), LW(6))
        USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(0), LW(2), 1, LW(15))
        USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
        USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.0))
        USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 80, -1, 0)
        USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 256, LW(5))
    END_IF()
    USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(6))
    MULF(LW(6), 40)
    USER_FUNC(_get_nomikomi_hit_position, -2, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), LW(6))
    USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(0), LW(2), 1, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 80, -1, 0)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_ResultAC)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_A_2"))
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_YOSHI_SWALLOW1"), 0)
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
        WAIT_FRM(50)
        GOTO(90)
    END_IF()
    WAIT_FRM(7)
    USER_FUNC(_get_swallow_param, LW(3), LW(7))
    IF_EQUAL(LW(7), -1)
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&customWeapon_YoshiGulp_Dmg0), int(0x80000100U), LW(5))
        WAIT_FRM(43)
        GOTO(90)
    END_IF()
    USER_FUNC(btlevtcmd_GetResultAC, LW(0))
    IF_NOT_FLAG(LW(0), 0x2)
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&customWeapon_YoshiGulp_Dmg0), int(0x80000100U), LW(5))
        WAIT_FRM(43)
        GOTO(90)
    END_IF()
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_YOSHI_SWALLOW2"), 0)
    USER_FUNC(_check_swallow_attribute, LW(3), 8, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_DamageDirect, LW(3), LW(4), 0, 0, 7, 1)
        WAIT_FRM(13)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_A_7"))
        USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_YOSHI_SWALLOW5"), 0)
        WAIT_FRM(34)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_A_5"))
        WAIT_FRM(54)
        GOTO(90)
    END_IF()
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 350)
    USER_FUNC(btlevtcmd_DamageDirect, LW(3), LW(4), 0, 0, 2, 1)
    WAIT_FRM(3)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 12)
    USER_FUNC(btlevtcmd_DivePosition, LW(3), LW(0), LW(1), LW(2), 5, 0, 0, 0, -1)
    USER_FUNC(btlevtcmd_OnAttribute, LW(3), 16777216)
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_YOSHI_SWALLOW3"), 0)
    WAIT_FRM(6)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_A_3"))
    WAIT_FRM(7)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_sub_random, 99, LW(0))
    IF_LARGE_EQUAL(LW(0), LW(7))
        INLINE_EVT()
            USER_FUNC(evt_env_blur_on, 0, 600)
        END_INLINE()
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_A_4"))
        USER_FUNC(btlevtcmd_ftomsec, 22, LW(0))
        WAIT_MSEC(LW(0))
        INLINE_EVT_ID(LW(15))
            USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_YOSHI_NOMIKOMI6"), 0)
            USER_FUNC(btlevtcmd_OffAttribute, LW(3), 16777216)
            USER_FUNC(_check_swallow_attribute, LW(3), 2, LW(0))
            IF_EQUAL(LW(0), 0)
                USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&customWeapon_YoshiGulp_Spew), int(0x80000100U), LW(5))
            ELSE()
                USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&customWeapon_YoshiGulp_Shot), int(0x80000100U), LW(5))
            END_IF()
            USER_FUNC(btlevtcmd_GetResultAC, LW(0))
            IF_FLAG(LW(0), 0x2)
                USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), 0, LW(6))
                USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), LW(6))
            ELSE()
                USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), -1)
            END_IF()
            USER_FUNC(btlevtcmd_AnimeWaitPlayComplete, -2, 1)
            USER_FUNC(btlevtcmd_ACRStart, -2, 0, 15, 15, 20)
            USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
            SWITCH(LW(6))
                CASE_LARGE_EQUAL(2)
                    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
                    USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(0), 1, 0, 0, 0)
                    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_Y_1"))
                    USER_FUNC(btlevtcmd_AnimeWaitPlayComplete, -2, 1)
                    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_Y_1"))
                    USER_FUNC(btlevtcmd_AnimeWaitPlayComplete, -2, 1)
                CASE_ETC()
                    USER_FUNC(evt_audience_acrobat_notry)
            END_SWITCH()
        END_INLINE()
        USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
        GOTO(90)
    END_IF()
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 1, 1)
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), 0, LW(6))
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
    END_BROTHER()
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_YOSHI_SWALLOW5"), 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_A_7"))
    WAIT_FRM(26)
    USER_FUNC(_check_swallow_attribute, LW(3), 1, LW(1))
    USER_FUNC(_check_swallow_attribute, LW(3), 4, LW(2))
    USER_FUNC(btlevtcmd_GetExp, LW(3), LW(0))
    USER_FUNC(btlevtcmd_StoreExp, LW(0))
    USER_FUNC(btlevtcmd_KillUnit, LW(3), 0)
    WAIT_FRM(8)
    IF_NOT_EQUAL(LW(1), 0)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_A_6"))
        USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_YOSHI_SWALLOW7"), 0)
        WAIT_FRM(50)
        GOTO(90)
    ELSE()
        IF_EQUAL(LW(2), 0)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_A_5"))
            USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_YOSHI_SWALLOW4"), 0)
            WAIT_FRM(54)
            GOTO(90)
        END_IF()
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_A_4"))
    WAIT_FRM(22)
    BROTHER_EVT()
        WAIT_FRM(18)
    END_BROTHER()
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 15)
    ADD(LW(1), 15)
    SET(LW(3), LW(0))
    SET(LW(4), LW(1))
    SET(LW(5), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(3), 240)
    USER_FUNC(evt_eff, PTR("eff"), PTR("breath_fire"), 1, LW(0), LW(1), LW(2), LW(3), LW(4), LW(5), 1, 2, 60, FLOAT(0.5), FLOAT(30.0))
    WAIT_FRM(60)
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), PTR(&customWeapon_YoshiGulp_Fire))
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(10), LW(11))
    LBL(10)
    IF_EQUAL(LW(10), -1)
        GOTO(90)
    END_IF()
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(10), LW(11), PTR(&customWeapon_YoshiGulp_Fire), int(0x80000100U), LW(5))
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(10), LW(11))
    GOTO(10)
    LBL(90)
    USER_FUNC(btlevtcmd_StopAC)
    USER_FUNC(evt_audience_ap_recovery)
    USER_FUNC(btlevtcmd_InviteApInfoReport)
    LBL(95)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(4.0))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetUnitWork, -2, 1, LW(5))
    IF_EQUAL(LW(5), 0)
        USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_GetMoveFrame, -2, LW(0), LW(1), LW(2), FLOAT(4.0), LW(5))
    IF_SMALL_EQUAL(LW(5), 20)
        USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
        GOTO(99)
    END_IF()
    BROTHER_EVT_ID(LW(15))
        SUB(LW(5), 1)
        SET(LW(6), LW(5))
        SUB(LW(6), 10)
        USER_FUNC(btlevtcmd_ACRStart, -2, LW(6), LW(5), LW(5), 0)
    END_BROTHER()
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), LW(5), -1, 0)
    USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
    USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
    IF_EQUAL(LW(6), 2)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_O_1"))
        USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, 0, 0, 0)
        WAIT_FRM(20)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_J_1B"))
        USER_FUNC(evt_sub_intpl_init, 4, 0, 360, 30)
        DO(30)
            USER_FUNC(evt_sub_intpl_get_value)
            USER_FUNC(btlevtcmd_SetRotate, -2, 0, LW(0), 0)
            WAIT_FRM(1)
        WHILE()
    END_IF()
    LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RUN_CHILD_EVT(PTR(&btldefaultevt_SuitoruBadgeEffect))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(_egg_attack_event)
    USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(6))
    MULF(LW(6), 10)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    ADD(LW(1), LW(6))
    USER_FUNC(btlevtcmd_SetPartsPos, -2, LW(12), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, LW(12), 50331648)
    USER_FUNC(btlevtcmd_SetPartsMoveSpeed, -2, LW(12), FLOAT(5.0))
    USER_FUNC(btlevtcmd_SetPartsFallAccel, -2, LW(12), FLOAT(0.10))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_GetMoveFrame, -2, LW(0), LW(1), LW(2), FLOAT(5.0), LW(5))
        USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(6))
        MULF(LW(6), 6)
        USER_FUNC(btlevtcmd_SetPartsRotateOffset, -2, LW(12), 0, LW(6), 0)
        DO(LW(5))
            USER_FUNC(btlevtcmd_AddPartsRotate, -2, LW(12), 0, 0, 48)
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_YOSHI_EGG3"), 0)
    USER_FUNC(btlevtcmd_DivePartsPosition, -2, LW(12), LW(0), LW(1), LW(2), 0, 25, 0, 0, -1)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, LW(12), 50331648)
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_YOSHI_EGG2"), 0)
    USER_FUNC(evt_eff, PTR(""), PTR("bomb"), 0, LW(0), LW(1), LW(2), FLOAT(0.60), 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(btlevtcmd_GetPartsWork, -2, LW(12), 5, LW(6))
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
        GOTO(89)
    END_IF()
    USER_FUNC(btlevtcmd_GetPartsWork, -2, LW(12), 2, LW(0))
    USER_FUNC(btlevtcmd_GetPartsWork, -2, LW(12), 6, LW(1))
    IF_EQUAL(LW(1), 0)
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(0), 1073872896, LW(5))
    ELSE()
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(0), 1073873152, LW(5))
    END_IF()
    LBL(89)
    RETURN()
EVT_END()

EVT_BEGIN(partyYoshiAttack_EggAttack)
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    // LW(9) = always weapon address for rest of script.
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(9))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_CommandPayWeaponCost, -2)
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(9))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(9), LW(3), LW(4))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 2, 3, LW(3))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 2, 4, LW(4))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 2, 5, 0)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 2, 6, 0)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 2, 7, 0)
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(9))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(9), LW(10), LW(11))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 7, 3, LW(10))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 7, 4, LW(11))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 7, 5, 0)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 7, 6, 0)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 7, 7, 0)
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(9))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(9), LW(10), LW(11))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 6, 3, LW(10))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 6, 4, LW(11))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 6, 5, 0)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 6, 6, 0)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 6, 7, 0)
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(9))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(9), LW(10), LW(11))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 5, 3, LW(10))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 5, 4, LW(11))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 5, 5, 0)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 5, 6, 0)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 5, 7, 0)
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(9))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(9), LW(10), LW(11))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 4, 3, LW(10))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 4, 4, LW(11))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 4, 5, 0)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 4, 6, 0)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 4, 7, 0)
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(9))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(9), LW(10), LW(11))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 3, 3, LW(10))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 3, 4, LW(11))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 3, 5, 0)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 3, 6, 0)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 3, 7, 0)
    USER_FUNC(evt_btl_camera_set_mode, 0, 10)
    USER_FUNC(evt_btl_camera_set_homing_unitparts, 0, -2, 1, -1, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 300)
    
    // Set # of eggs based on move used (2-4 for Mini-Egg, 3-5 for Egg Barrage).
    IF_EQUAL(LW(9), PTR(&customWeapon_YoshiMiniEgg))
        USER_FUNC(evtTot_GetMoveSelectedLevel, MoveType::YOSHI_MINI_EGG, LW(0))
        SET(LW(1), 1)
    ELSE()
        USER_FUNC(evtTot_GetMoveSelectedLevel, MoveType::YOSHI_EGG_BARRAGE, LW(0))
        SET(LW(1), 2)
    END_IF()
    ADD(LW(1), LW(0))

    USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
    USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
    USER_FUNC(btlevtcmd_AcGetDifficulty, LW(0))
    SUB(LW(0), 3)
    SWITCH(LW(0))
        CASE_SMALL_EQUAL(-3)
            SET(LW(0), 100)
        CASE_EQUAL(-2)
            SET(LW(0), 80)
        CASE_EQUAL(-1)
            SET(LW(0), 60)
        CASE_EQUAL(0)
            SET(LW(0), 45)
        CASE_EQUAL(1)
            SET(LW(0), 35)
        CASE_EQUAL(2)
            SET(LW(0), 25)
        CASE_ETC()
            SET(LW(0), 15)
    END_SWITCH()
    // Set timer based on number of eggs.
    MUL(LW(0), LW(1))
    USER_FUNC(btlevtcmd_AcSetParamAll, 0, LW(0), LW(1), 0, EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AcSetFlag, 0)
    USER_FUNC(btlevtcmd_AcSetOutputParam, 1, 0)
    USER_FUNC(btlevtcmd_SetupAC, -2, 20, 1, 60)
    WAIT_FRM(22)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 272, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_StartAC, 1)
    SET(LW(12), 2)
    LBL(5)
    USER_FUNC(btlevtcmd_AcGetOutputParam, 0, LW(13))
    ADD(LW(13), 1)
    IF_LARGE(LW(12), LW(13))
        USER_FUNC(btlevtcmd_AcGetOutputParam, 1, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            USER_FUNC(btlevtcmd_StopAC)
            WAIT_MSEC(500)
            USER_FUNC(btlevtcmd_AcGetOutputParam, 0, LW(13))
            IF_LARGE_EQUAL(LW(13), 1)
                USER_FUNC(btlevtcmd_AcGetOutputParam, 0, LW(13))
                ADD(LW(13), 1)
                GOTO(25)
            ELSE()
                USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(9), -1)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_A_5"))
                WAIT_FRM(54)
                USER_FUNC(evt_audience_ap_recovery)
                USER_FUNC(btlevtcmd_InviteApInfoReport)
                GOTO(95)
            END_IF()
        END_IF()
        WAIT_FRM(1)
        GOTO(5)
    END_IF()
    SET(LW(0), LW(12))
    SUB(LW(0), 1)
    SWITCH(LW(0))
        CASE_SMALL_EQUAL(2)
            SET(LW(0), 0)
        CASE_SMALL_EQUAL(3)
            SET(LW(0), 1)
        CASE_ETC()
            SET(LW(0), 2)
    END_SWITCH()
    USER_FUNC(btlevtcmd_GetResultPrizeLv, -5, LW(0), LW(6))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    ADD(LW(1), 40)
    USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(9), LW(6))
    LBL(10)
    USER_FUNC(evt_sub_random, 2, LW(0))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, LW(12), 0, LW(0))
    SWITCH(LW(0))
        CASE_EQUAL(1)
            USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, LW(12), PTR(&pose_table_egg_y))
        CASE_EQUAL(2)
            USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, LW(12), PTR(&pose_table_egg_p))
        CASE_ETC()
            USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, LW(12), PTR(&pose_table_egg_g))
    END_SWITCH()
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, LW(12), 69)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, LW(12), 1, 4)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, LW(12), 2, LW(9))
    LBL(20)
    USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(6))
    USER_FUNC(btlevtcmd_SetPartsScale, -2, LW(12), FLOAT(1.0), FLOAT(1.0), FLOAT(1.0))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_A_8"))
    WAIT_FRM(10)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_A_12"))
    USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(5))
    MULF(LW(5), 10)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), LW(5))
    ADD(LW(1), LW(5))
    USER_FUNC(btlevtcmd_SetPartsPos, -2, LW(12), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetPartsDispOffset, -2, LW(12), 0, 0, 0)
    USER_FUNC(btlevtcmd_SetPartsRotate, -2, LW(12), 0, 0, 0)
    USER_FUNC(btlevtcmd_SetPartsRotateOffset, -2, LW(12), 0, 0, 0)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, LW(12), 50331648)
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_YOSHI_EGG1"), 0)
    SET(LW(15), 0)
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_SetPartsRotateOffset, -2, LW(12), 0, 6, 0)
        DO(30)
            USER_FUNC(btlevtcmd_AddPartsRotate, -2, LW(12), 0, 0, 48)
            WAIT_FRM(1)
        WHILE()
        USER_FUNC(btlevtcmd_SetPartsRotate, -2, LW(12), 0, 0, 0)
    END_BROTHER()
    BROTHER_EVT()
        SET(LW(5), LW(12))
        SUB(LW(5), 2)
        MULF(LW(5), 10)
        ADDF(LW(5), 15)
        USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(0))
        MULF(LW(5), LW(0))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), LW(5))
        USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_SetPartsFallAccel, -2, LW(12), FLOAT(0.20))
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(12), LW(0), LW(1), LW(2), 30, -1)
    END_BROTHER()
    WAIT_FRM(10)
    ADD(LW(12), 1)
    GOTO(5)
    LBL(25)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 2, 6, -1)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 3, 6, -1)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 4, 6, -1)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 5, 6, -1)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 6, 6, -1)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 7, 6, -1)
    SET(LW(12), LW(13))
    LBL(26)
    USER_FUNC(btlevtcmd_GetPartsWork, -2, LW(12), 6, LW(0))
    IF_EQUAL(LW(0), -1)
        USER_FUNC(btlevtcmd_SetPartsWork, -2, LW(12), 6, 1)
        SET(LW(14), LW(12))
        LBL(27)
        SUB(LW(14), 1)
        IF_LARGE_EQUAL(LW(14), 2)
            USER_FUNC(btlevtcmd_GetPartsWork, -2, LW(12), 3, LW(0))
            USER_FUNC(btlevtcmd_GetPartsWork, -2, LW(14), 3, LW(1))
            IF_EQUAL(LW(0), LW(1))
                USER_FUNC(btlevtcmd_SetPartsWork, -2, LW(14), 6, 0)
            END_IF()
            GOTO(27)
        END_IF()
    END_IF()
    SUB(LW(12), 1)
    IF_LARGE_EQUAL(LW(12), 2)
        GOTO(26)
    END_IF()
    SET(LW(12), 2)
    LBL(28)
    USER_FUNC(btlevtcmd_GetPartsWork, -2, LW(12), 6, LW(0))
    SWITCH(LW(0))
        CASE_EQUAL(0)
            USER_FUNC(btlevtcmd_GetPartsWork, -2, LW(12), 3, LW(0))
            USER_FUNC(btlevtcmd_GetPartsWork, -2, LW(12), 4, LW(1))
            USER_FUNC(btlevtcmd_CommandPreCheckDamage, -2, LW(0), LW(1), 256, LW(6))
            USER_FUNC(btlevtcmd_SetPartsWork, -2, LW(12), 5, LW(6))
            IF_EQUAL(LW(6), 1)
                SET(LW(14), LW(12))
                LBL(29)
                ADD(LW(14), 1)
                IF_SMALL_EQUAL(LW(14), LW(13))
                    USER_FUNC(btlevtcmd_GetPartsWork, -2, LW(12), 3, LW(0))
                    USER_FUNC(btlevtcmd_GetPartsWork, -2, LW(14), 3, LW(1))
                    IF_EQUAL(LW(0), LW(1))
                        USER_FUNC(btlevtcmd_GetPartsWork, -2, LW(14), 6, LW(0))
                        IF_NOT_EQUAL(LW(0), 0)
                            USER_FUNC(btlevtcmd_SetPartsWork, -2, LW(14), 6, 2)
                        END_IF()
                    END_IF()
                    GOTO(29)
                END_IF()
            END_IF()
        CASE_EQUAL(1)
            USER_FUNC(btlevtcmd_GetPartsWork, -2, LW(12), 3, LW(0))
            USER_FUNC(btlevtcmd_GetPartsWork, -2, LW(12), 4, LW(1))
            USER_FUNC(btlevtcmd_CommandPreCheckDamage, -2, LW(0), LW(1), 256, LW(6))
            USER_FUNC(btlevtcmd_SetPartsWork, -2, LW(12), 5, LW(6))
        CASE_ETC()
            USER_FUNC(btlevtcmd_GetPartsWork, -2, LW(12), 3, LW(0))
            USER_FUNC(btlevtcmd_GetPartsWork, -2, LW(12), 4, LW(1))
            USER_FUNC(btlevtcmd_CommandPreCheckDamage, -2, LW(0), LW(1), 1048832, LW(6))
            USER_FUNC(btlevtcmd_SetPartsWork, -2, LW(12), 5, LW(6))
    END_SWITCH()
    IF_SMALL(LW(12), LW(13))
        ADD(LW(12), 1)
        GOTO(28)
    END_IF()
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    SET(LW(12), 2)
    LBL(30)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, LW(12), 50331648)
    USER_FUNC(btlevtcmd_GetPartsWork, -2, LW(12), 0, LW(0))
    SWITCH(LW(0))
        CASE_EQUAL(1)
            SET(LW(6), PTR("PYS_A3_1"))
            SET(LW(7), PTR("PYS_A3_2"))
            SET(LW(8), PTR("PYS_A3_3"))
        CASE_EQUAL(2)
            SET(LW(6), PTR("PYS_A4_1"))
            SET(LW(7), PTR("PYS_A4_2"))
            SET(LW(8), PTR("PYS_A4_3"))
        CASE_ETC()
            SET(LW(6), PTR("PYS_A2_1"))
            SET(LW(7), PTR("PYS_A2_2"))
            SET(LW(8), PTR("PYS_A2_3"))
    END_SWITCH()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, LW(6))
    USER_FUNC(btlevtcmd_AnimeWaitPlayComplete, -2, 1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, LW(8))
    WAIT_FRM(7)
    BROTHER_EVT_ID(LW(15))
        RUN_CHILD_EVT(PTR(&_egg_attack_event))
    END_BROTHER()
    USER_FUNC(btlevtcmd_SetPartsWork, -2, LW(12), 7, LW(15))
    USER_FUNC(btlevtcmd_AnimeWaitPlayComplete, -2, 1)
    WAIT_FRM(10)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 69)
    LBL(90)
    IF_NOT_EQUAL(LW(12), LW(13))
        ADD(LW(12), 1)
        USER_FUNC(btlevtcmd_GetPartsWork, -2, LW(12), 3, LW(3))
        USER_FUNC(btlevtcmd_GetPartsWork, -2, LW(12), 4, LW(4))
        GOTO(30)
    END_IF()
    USER_FUNC(btlevtcmd_StopAC)
    USER_FUNC(btlevtcmd_ACRStart, -2, 0, 15, 15, 20)
    USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
    SWITCH(LW(6))
        CASE_LARGE_EQUAL(2)
            USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
            USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(0), 1, 0, 0, 0)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_Y_1"))
            USER_FUNC(btlevtcmd_AnimeWaitPlayComplete, -2, 1)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_Y_1"))
            USER_FUNC(btlevtcmd_AnimeWaitPlayComplete, -2, 1)
        CASE_ETC()
            USER_FUNC(evt_audience_acrobat_notry)
    END_SWITCH()
    USER_FUNC(evt_audience_ap_recovery)
    USER_FUNC(btlevtcmd_InviteApInfoReport)
    LBL(95)
    SET(LW(12), 2)
    LBL(96)
    USER_FUNC(btlevtcmd_GetPartsWork, -2, LW(12), 7, LW(15))
    IF_NOT_EQUAL(LW(15), 0)
        USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
        USER_FUNC(btlevtcmd_SetPartsWork, -2, LW(12), 7, 0)
    END_IF()
    ADD(LW(12), 1)
    IF_SMALL_EQUAL(LW(12), LW(13))
        GOTO(96)
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
    WAIT_MSEC(2000)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(partyYoshiAttack_CallGuard)
    SET(LW(12), PTR(&customWeapon_YoshiStampede))
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(12))
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_StartWaitEvent, -2)
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(12))
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
    USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
    USER_FUNC(btlevtcmd_AcGetDifficulty, LW(0))
    SUB(LW(0), 3)
    SWITCH(LW(0))
        CASE_SMALL_EQUAL(-3)
            SET(LW(0), 280)
            SET(LW(1), 18)
            SET(LW(2), 25)
        CASE_EQUAL(-2)
            SET(LW(0), 250)
            SET(LW(1), 17)
            SET(LW(2), 27)
        CASE_EQUAL(-1)
            SET(LW(0), 240)
            SET(LW(1), 16)
            SET(LW(2), 29)
        CASE_EQUAL(0)
            SET(LW(0), 240)
            SET(LW(1), 15)
            SET(LW(2), 30)
        CASE_EQUAL(1)
            SET(LW(0), 200)
            SET(LW(1), 15)
            SET(LW(2), 30)
        CASE_EQUAL(2)
            SET(LW(0), 180)
            SET(LW(1), 15)
            SET(LW(2), 30)
        CASE_ETC()
            SET(LW(0), 150)
            SET(LW(1), 14)
            SET(LW(2), 30)
    END_SWITCH()
    USER_FUNC(evtTot_GetMoveSelectedLevel, MoveType::YOSHI_STAMPEDE, LW(6))
    SWITCH(LW(6))
        CASE_EQUAL(1)
            USER_FUNC(btlevtcmd_AcSetParamAll, 15, LW(0), 178, LW(1), LW(2), 34, 100, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_AcSetGaugeParam, 34, 67, 100, 100)
        CASE_EQUAL(2)
            USER_FUNC(btlevtcmd_AcSetParamAll, 15, LW(0), 178, LW(1), LW(2), 25, 76, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_AcSetGaugeParam, 25, 50, 75, 100)
        CASE_ETC()
            USER_FUNC(btlevtcmd_AcSetParamAll, 15, LW(0), 178, LW(1), LW(2), 20, 61, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_AcSetGaugeParam, 20, 60, 80, 100)
    END_SWITCH()
    USER_FUNC(btlevtcmd_AcSetFlag, 64)
    USER_FUNC(btlevtcmd_SetupAC, -2, 6, 1, 0)
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, 1)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 4)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 400)
    WAIT_FRM(20)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_A_10"))
    USER_FUNC(btlevtcmd_ftof, 24, LW(0))
    WAIT_FRM(LW(0))
    USER_FUNC(btlevtcmd_StartAC, 1)
    USER_FUNC(btlevtcmd_ResultAC)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_A_11"))
    USER_FUNC(btlevtcmd_GetHeight, -2, LW(6))
    USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(0))
    MULF(LW(6), LW(0))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    ADD(LW(1), LW(6))
    USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(5))
    MUL(LW(5), 30)
    USER_FUNC(evt_eff, 0, PTR("kiss"), 3, LW(0), LW(1), LW(2), LW(5), 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(evt_snd_sfxon_3d, PTR("SFX_ENM_TOGENOKO_CALL1"), LW(0), LW(1), LW(2), 0)
    USER_FUNC(btlevtcmd_ftof, 33, LW(0))
    BROTHER_EVT_ID(LW(15))
        WAIT_FRM(LW(0))
    END_BROTHER()
    USER_FUNC(btlevtcmd_ACRStart, -2, 0, LW(0), LW(0), 20)
    USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
    SWITCH(LW(6))
        CASE_LARGE_EQUAL(2)
            USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, 0, 0, 0)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_Y_1"))
            USER_FUNC(btlevtcmd_AnimeWaitPlayComplete, -2, 1)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_Y_1"))
            USER_FUNC(btlevtcmd_AnimeWaitPlayComplete, -2, 1)
            USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 43)
        CASE_ETC()
            USER_FUNC(evt_audience_acrobat_notry)
            USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
            BROTHER_EVT()
                WAIT_MSEC(166)
                USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 43)
            END_BROTHER()
    END_SWITCH()
    INLINE_EVT_ID(LW(9))
        WAIT_MSEC(1000)
        USER_FUNC(_gundan_yoshi_run_effect, -2)
    END_INLINE()
    WAIT_MSEC(166)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_shake_h, 0, 1, 0, 6000, 0)
    WAIT_MSEC(1000)
    USER_FUNC(_wait_yoshig_run, -2)
    SET(LW(15), 0)
    LBL(70)
    IF_EQUAL(LW(3), -1)
        IF_NOT_EQUAL(LW(15), 0)
            USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
        END_IF()
        USER_FUNC(btlevtcmd_WaitEventEnd, LW(9))
        GOTO(90)
    END_IF()
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), LW(12), 256, LW(6))
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
        GOTO(75)
    END_IF()
    USER_FUNC(btlevtcmd_AcGetOutputParam, 2, LW(14))
    BROTHER_EVT_ID(LW(15))
        DO(LW(14))
            IF_LARGE_EQUAL(LW(14), 2)
                USER_FUNC(btlevtcmd_AcGetOutputParam, 2, LW(0))
                IF_LARGE_EQUAL(LW(0), 2)
                    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 131072, LW(5))
                ELSE()
                    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 0, LW(5))
                END_IF()
            ELSE()
                USER_FUNC(btlevtcmd_AcGetOutputParam, 2, LW(0))
                SWITCH(LW(0))
                    CASE_LARGE_EQUAL(6)
                        SET(LW(0), 2)
                    CASE_LARGE_EQUAL(4)
                        SET(LW(0), 1)
                    CASE_LARGE_EQUAL(2)
                        SET(LW(0), 0)
                    CASE_ETC()
                        SET(LW(0), -1)
                END_SWITCH()
                IF_LARGE_EQUAL(LW(0), 0)
                    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 131328, LW(5))
                    USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), LW(0), LW(6))
                    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
                    USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
                    USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), LW(6))
                ELSE()
                    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 256, LW(5))
                    USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), -1)
                END_IF()
            END_IF()
            WAIT_FRM(10)
        WHILE()
    END_BROTHER()
    LBL(75)
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    GOTO(70)
    LBL(90)
    USER_FUNC(_wait_yoshig_complete, -2)
    USER_FUNC(evt_btl_camera_noshake, 1)
    WAIT_MSEC(500)
    USER_FUNC(evt_audience_ap_recovery)
    USER_FUNC(btlevtcmd_InviteApInfoReport)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RUN_CHILD_EVT(PTR(&btldefaultevt_SuitoruBadgeEffect))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(customAttack_Gulp)
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 1, 0)
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_CommandPayWeaponCost, -2)
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)    
    USER_FUNC(evt_btl_camera_set_mode, 0, 10)
    USER_FUNC(evt_btl_camera_set_homing_unitparts, 0, -2, 1, -1, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    WAIT_FRM(22)
    LBL(5)
    
    USER_FUNC(btlevtcmd_CommandPreCheckDamage, -2, LW(3), LW(4), 256, LW(5))
    USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(6))
    MULF(LW(6), 40)
    USER_FUNC(_get_nomikomi_hit_position, -2, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), LW(6))
    USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(0), LW(2), 1, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(5.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 50, -1, 0)
    IF_EQUAL(LW(5), 5)
        // Handle counterattacks.
        USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 256, LW(5))
    END_IF()
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))    
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_A_2"))
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_YOSHI_SWALLOW1"), 0)
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
        WAIT_FRM(50)
        GOTO(90)
    END_IF()
    WAIT_FRM(7)

    // If not swallowable at all, fails automatically.
    USER_FUNC(_get_swallow_param, LW(3), LW(7))
    IF_EQUAL(LW(7), -1)
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&customWeapon_YoshiGulp_Dmg0), int(0x80000100U), LW(5))
        WAIT_FRM(43)
        GOTO(90)
    END_IF()
        
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_YOSHI_SWALLOW2"), 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 350)
    USER_FUNC(btlevtcmd_DamageDirect, LW(3), LW(4), 0, 0, 2, 1)
    WAIT_FRM(3)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 12)
    USER_FUNC(btlevtcmd_DivePosition, LW(3), LW(0), LW(1), LW(2), 5, 0, 0, 0, -1)
    USER_FUNC(btlevtcmd_OnAttribute, LW(3), 16777216)
    // Suprress drawing enemy held item.
    USER_FUNC(btlevtcmd_OnUnitFlag, LW(3), 0x8000000)
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_YOSHI_SWALLOW3"), 0)
    
    // New AC: Stampede-style L & R mashing once enemy is in Yoshi's mouth.
    USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
    USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
    USER_FUNC(btlevtcmd_AcGetDifficulty, LW(0))
    SUB(LW(0), 3)
    SWITCH(LW(0))
        CASE_SMALL_EQUAL(-3)
            SET(LW(1), 25)
        CASE_EQUAL(-2)
            SET(LW(1), 22)
        CASE_EQUAL(-1)
            SET(LW(1), 19)
        CASE_EQUAL(0)
            SET(LW(1), 16)
        CASE_EQUAL(1)
            SET(LW(1), 15)
        CASE_EQUAL(2)
            SET(LW(1), 14)
        CASE_ETC()
            SET(LW(1), 13)
    END_SWITCH()
    SET(LW(0), 150)
    // Fight-back param scales further up at enemy HP, down for Tiny status.
    SET(LW(2), 50)
    USER_FUNC(evtTot_SetGulpStruggleParam, LW(3), LW(2))
    
    USER_FUNC(btlevtcmd_AcSetParamAll, 15, LW(0), 178, LW(1), LW(2), 1, 100, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AcSetGaugeParam, 100, 100, 100, 100)
    USER_FUNC(btlevtcmd_AcSetFlag, 1)
    USER_FUNC(btlevtcmd_SetupAC, -2, 6, 1, 0)
    WAIT_FRM(6)
    // Start flutter kick animation a second before timer elapses.
    SET(LW(7), LW(0))
    SUB(LW(7), 60)
    BROTHER_EVT()
        WAIT_FRM(LW(7))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_A_3"))
    END_BROTHER()
    USER_FUNC(btlevtcmd_StartAC, 1)
    USER_FUNC(btlevtcmd_ResultAC)
    
    WAIT_FRM(7)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    // Acts as a success percentage of swallowing the enemy.
    USER_FUNC(btlevtcmd_AcGetOutputParam, 2, LW(7))
    USER_FUNC(evt_sub_random, 99, LW(0))
    IF_LARGE_EQUAL(LW(0), LW(7))
        // If failed, drop enemy on floor for 1 damage.        
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_A_4"))
        USER_FUNC(btlevtcmd_ftomsec, 22, LW(0))
        WAIT_MSEC(LW(0))
        INLINE_EVT_ID(LW(15))
            USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_YOSHI_NOMIKOMI6"), 0)
            USER_FUNC(btlevtcmd_OffAttribute, LW(3), 16777216)
            USER_FUNC(btlevtcmd_OffUnitFlag, LW(3), 0x8000000)
            USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&customWeapon_YoshiSwallow), int(0x80000100U), LW(5))
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), -1)
            USER_FUNC(btlevtcmd_AnimeWaitPlayComplete, -2, 1)
        END_INLINE()
        USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
        GOTO(90)
    ELSE()
        USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), 0, LW(6))
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), LW(6))
    END_IF()
        
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 1, 1)
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), 0, LW(6))
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
    END_BROTHER()
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_YOSHI_SWALLOW5"), 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_A_7"))
    WAIT_FRM(26)
    USER_FUNC(_check_swallow_attribute, LW(3), 1, LW(1))
    USER_FUNC(_check_swallow_attribute, LW(3), 4, LW(2))
    USER_FUNC(evtTot_GetEggLayItem, LW(3), LW(14))
    USER_FUNC(btlevtcmd_GetExp, LW(3), LW(0))
    USER_FUNC(btlevtcmd_StoreExp, LW(0))
    USER_FUNC(btlevtcmd_KillUnit, LW(3), 0)
    WAIT_FRM(8)
    
    // Spawn egg.
    SET(LW(12), 2)
    USER_FUNC(evt_sub_random, 2, LW(0))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, LW(12), 0, LW(0))
    SWITCH(LW(0))
        CASE_EQUAL(1)
            USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, LW(12), PTR(&pose_table_egg_y))
        CASE_EQUAL(2)
            USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, LW(12), PTR(&pose_table_egg_p))
        CASE_ETC()
            USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, LW(12), PTR(&pose_table_egg_g))
    END_SWITCH()
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, LW(12), 69)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, LW(12), 1, 4)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, LW(12), 2, LW(9))
    USER_FUNC(btlevtcmd_SetPartsScale, -2, LW(12), FLOAT(1.0), FLOAT(1.0), FLOAT(1.0))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_A_8"))
    WAIT_FRM(10)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_A_12"))
    USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(5))
    MULF(LW(5), 10)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), LW(5))
    ADD(LW(1), LW(5))
    USER_FUNC(btlevtcmd_SetPartsPos, -2, LW(12), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetPartsDispOffset, -2, LW(12), 0, 0, 0)
    USER_FUNC(btlevtcmd_SetPartsRotate, -2, LW(12), 0, 0, 0)
    USER_FUNC(btlevtcmd_SetPartsRotateOffset, -2, LW(12), 0, 0, 0)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, LW(12), 50331648)
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_YOSHI_EGG1"), 0)
    
    // Spinning / flying egg animations.
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_SetPartsRotateOffset, -2, LW(12), 0, 6, 0)
        DO(30)
            USER_FUNC(btlevtcmd_AddPartsRotate, -2, LW(12), 0, 0, 48)
            WAIT_FRM(1)
        WHILE()
        USER_FUNC(btlevtcmd_SetPartsRotate, -2, LW(12), 0, 0, 0)
    END_BROTHER()
    BROTHER_EVT()
        SET(LW(5), 25)
        USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(0))
        MULF(LW(5), LW(0))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), LW(5))
        USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_SetPartsFallAccel, -2, LW(12), FLOAT(0.20))
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(12), LW(0), LW(1), LW(2), 30, -1)
    END_BROTHER()
    WAIT_FRM(20)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_S_1"))
    
    // Stylish command when egg hits ground.
    BROTHER_EVT_ID(LW(15))
        WAIT_FRM(5)
        USER_FUNC(btlevtcmd_ACRStart, -2, 0, 15, 15, 0)
        USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
        SWITCH(LW(6))
            CASE_LARGE_EQUAL(2)
                USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
                USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(0), 1, 0, 0, 0)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_Y_1"))
                USER_FUNC(btlevtcmd_AnimeWaitPlayComplete, -2, 1)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_Y_1"))
                USER_FUNC(btlevtcmd_AnimeWaitPlayComplete, -2, 1)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_S_1"))
            CASE_ETC()
                USER_FUNC(evt_audience_acrobat_notry)
        END_SWITCH()
    END_BROTHER()
    
    // Hatch egg.
    WAIT_FRM(25)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, LW(12), 50331648)
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_YOSHI_EGG2"), 0)
    USER_FUNC(evt_eff, PTR(""), PTR("bomb"), 0, LW(0), LW(1), LW(2), FLOAT(0.60), 0, 0, 0, 0, 0, 0, 0)
    WAIT_FRM(25)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHeight, -2, LW(5))
    ADD(LW(1), LW(5))
    ADD(LW(1), 5)
    USER_FUNC(btlevtcmd_BtlIconEntryItemId, LW(14), LW(0), LW(1), LW(2), LW(8))
    USER_FUNC(btlevtcmd_AnnounceSetParam, 0, LW(14))
    USER_FUNC(btlevtcmd_AnnounceMessage, 1, 0, 0, PTR("btl_msg_steal_item_get"), 90)
    USER_FUNC(btlevtcmd_BtlIconDelete, LW(8))
    
    IF_NOT_EQUAL(LW(15), 0)
        USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
    END_IF()
    
    LBL(90)
    USER_FUNC(btlevtcmd_StopAC)
    USER_FUNC(evt_audience_ap_recovery)
    USER_FUNC(btlevtcmd_InviteApInfoReport)
    LBL(95)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(4.0))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetUnitWork, -2, 1, LW(5))
    IF_EQUAL(LW(5), 0)
        USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_GetMoveFrame, -2, LW(0), LW(1), LW(2), FLOAT(4.0), LW(5))
    IF_SMALL_EQUAL(LW(5), 20)
        USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
        GOTO(99)
    END_IF()
    BROTHER_EVT_ID(LW(15))
        SUB(LW(5), 1)
        SET(LW(6), LW(5))
        SUB(LW(6), 10)
        USER_FUNC(btlevtcmd_ACRStart, -2, LW(6), LW(5), LW(5), 0)
    END_BROTHER()
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), LW(5), -1, 0)
    USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
    USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
    IF_EQUAL(LW(6), 2)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_O_1"))
        USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(12), 1, 0, 0, 0)
        WAIT_FRM(20)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_J_1B"))
        USER_FUNC(evt_sub_intpl_init, 4, 0, 360, 30)
        DO(30)
            USER_FUNC(evt_sub_intpl_get_value)
            USER_FUNC(btlevtcmd_SetRotate, -2, 0, LW(0), 0)
            WAIT_FRM(1)
        WHILE()
    END_IF()
    LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RUN_CHILD_EVT(PTR(&btldefaultevt_SuitoruBadgeEffect))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

BattleWeapon customWeapon_YoshiGroundPound = {
    .name = "btl_wn_pys_normal",
    .icon = IconType::PARTNER_MOVE_0,
    .item_id = 0,
    .description = "msg_pys_hip_drop",
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
    .damage_function_params = { 1, 1, 1, 1, 1, 1, 0, MoveType::YOSHI_BASE },
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
    .ac_help_msg = "msg_ac_hip_drop",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::USABLE_IF_CONFUSED |
        AttackSpecialProperty_Flags::GROUNDS_WINGED |
        AttackSpecialProperty_Flags::FLIPS_SHELLED |
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::DIMINISHING_BY_HIT |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags =
        AttackCounterResistance_Flags::FRONT_SPIKY |
        AttackCounterResistance_Flags::PREEMPTIVE_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)partyYoshiAttack_NormalAttack,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 10,
    .bg_a2_fall_weight = 10,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 10,
    .nozzle_turn_chance = 10,
    .nozzle_fire_chance = 5,
    .ceiling_fall_chance = 5,
    .object_fall_chance = 5,
};

BattleWeapon customWeapon_YoshiGulp_Shot = {
    .name = "btl_wn_pys_lv1",
    .icon = IconType::PARTNER_MOVE_1,
    .item_id = 0,
    .description = "msg_pys_nomikomi",
    .base_accuracy = 100,
    .base_fp_cost = 4,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 1, 4, 2, 5, 3, 6, 0, MoveType::YOSHI_GULP },
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
        AttackTargetProperty_Flags::HAMMERLIKE,
    .element = AttackElement::NORMAL,
    .damage_pattern = 6,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_nomikomi",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags =
        AttackCounterResistance_Flags::ALL &
        ~AttackCounterResistance_Flags::PREEMPTIVE_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)partyYoshiAttack_Nomikomi,
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

BattleWeapon customWeapon_YoshiGulp_Spew = {
    .name = "btl_wn_pys_lv1",
    .icon = IconType::PARTNER_MOVE_1,
    .item_id = 0,
    .description = "msg_pys_nomikomi",
    .base_accuracy = 100,
    .base_fp_cost = 4,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 1, 4, 2, 5, 3, 6, 0, MoveType::YOSHI_GULP },
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
        AttackTargetProperty_Flags::HAMMERLIKE,
    .element = AttackElement::NORMAL,
    .damage_pattern = 5,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_nomikomi",
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
    
    .attack_evt_code = (void*)partyYoshiAttack_Nomikomi,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 10,
    .nozzle_fire_chance = 5,
    .ceiling_fall_chance = 5,
    .object_fall_chance = 0,
};

BattleWeapon customWeapon_YoshiGulp_Dmg0 = {
    .name = "btl_wn_pys_lv1",
    .icon = IconType::PARTNER_MOVE_1,
    .item_id = 0,
    .description = "msg_pys_nomikomi",
    .base_accuracy = 100,
    .base_fp_cost = 4,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)weaponGetPowerDefault,
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
        AttackTargetProperty_Flags::ONLY_FRONT |
        AttackTargetProperty_Flags::HAMMERLIKE,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_nomikomi",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING,
    .counter_resistance_flags =
        AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)partyYoshiAttack_Nomikomi,
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

BattleWeapon customWeapon_YoshiGulp_Fire = {
    .name = "btl_wn_pys_lv1",
    .icon = IconType::PARTNER_MOVE_1,
    .item_id = 0,
    .description = "msg_pys_nomikomi",
    .base_accuracy = 100,
    .base_fp_cost = 4,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)weaponGetPowerDefault,
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
    .element = AttackElement::FIRE,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_nomikomi",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING,
    .counter_resistance_flags =
        AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)partyYoshiAttack_Nomikomi,
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

BattleWeapon customWeapon_YoshiGulp_Recoil = {
    .name = nullptr,
    .icon = 0,
    .item_id = 0,
    .description = nullptr,
    .base_accuracy = 100,
    .base_fp_cost = 0,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 0.0,
    .stylish_multiplier = 0,
    .unk_19 = 0,
    .bingo_card_chance = 0,
    .unk_1b = 0,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 2, 2, 3, 3, 4, 4, 0, MoveType::YOSHI_GULP },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags =
        AttackTargetClass_Flags::MULTIPLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::RECOIL_DAMAGE |
        AttackTargetProperty_Flags::HAMMERLIKE,
    .element = AttackElement::NORMAL,
    // Cascading knockback effect
    .damage_pattern = 6,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = nullptr,
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

BattleWeapon customWeapon_YoshiMiniEgg = {
    .name = "btl_wn_pys_lv2",
    .icon = IconType::PARTNER_MOVE_2,
    .item_id = 0,
    .description = "msg_pys_wonder_egg",
    .base_accuracy = 100,
    .base_fp_cost = 3,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = nullptr,
    .damage_function_params = { 1, 1, 1, 1, 1, 1, 0, MoveType::YOSHI_MINI_EGG },
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
    .ac_help_msg = "msg_ac_wonder_egg",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::BADGE_BUFFABLE |
        AttackSpecialProperty_Flags::STATUS_BUFFABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000,
        
    // status chances
    .size_change_chance = 100,
    .size_change_time = 3,
    .size_change_strength = -2,
    
    .attack_evt_code = (void*)partyYoshiAttack_EggAttack,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 6,
    .nozzle_fire_chance = 3,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

BattleWeapon customWeapon_YoshiStampede = {
    .name = "btl_wn_pys_lv3",
    .icon = IconType::PARTNER_MOVE_3,
    .item_id = 0,
    .description = "msg_pys_taigun_yoshi",
    .base_accuracy = 100,
    .base_fp_cost = 6,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 1, 1, 1, 1, 1, 1, 0, MoveType::YOSHI_STAMPEDE },
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
        // Cannot target flying enemies.
        AttackTargetProperty_Flags::HAMMERLIKE,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_taigun_yoshi",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::DIMINISHING_BY_HIT |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)partyYoshiAttack_CallGuard,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 20,
    .bg_a2_fall_weight = 20,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 20,
    .nozzle_turn_chance = 40,
    .nozzle_fire_chance = 5,
    .ceiling_fall_chance = 5,
    .object_fall_chance = 20,
};

BattleWeapon customWeapon_YoshiEggBarrage = {
    .name = "btl_wn_pys_lv2",
    .icon = IconType::PARTNER_MOVE_1,
    .item_id = 0,
    .description = "msg_pys_wonder_egg",
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
    .damage_function_params = { 2, 2, 2, 2, 2, 2, 0, MoveType::YOSHI_EGG_BARRAGE },
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
    .ac_help_msg = "msg_ac_wonder_egg",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::BADGE_BUFFABLE |
        AttackSpecialProperty_Flags::STATUS_BUFFABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000,
        
    // status chances
    
    .attack_evt_code = (void*)partyYoshiAttack_EggAttack,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 6,
    .nozzle_fire_chance = 3,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

BattleWeapon customWeapon_YoshiSwallow = {
    .name = "btl_wn_pys_lv1",
    .icon = IconType::PARTNER_MOVE_2,
    .item_id = 0,
    .description = "msg_pys_nomikomi",
    .base_accuracy = 100,
    .base_fp_cost = 4,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 1, 4, 2, 5, 3, 6, 0, MoveType::YOSHI_SWALLOW },
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
        AttackTargetProperty_Flags::HAMMERLIKE,
    .element = AttackElement::NORMAL,
    .damage_pattern = 5,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_taigun_yoshi",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags =
        AttackCounterResistance_Flags::ALL &
        ~AttackCounterResistance_Flags::PREEMPTIVE_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)customAttack_Gulp,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 0,
    .bg_a2_fall_weight = 0,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 10,
    .nozzle_fire_chance = 5,
    .ceiling_fall_chance = 5,
    .object_fall_chance = 0,
};

}  // namespace mod::tot::party_yoshi