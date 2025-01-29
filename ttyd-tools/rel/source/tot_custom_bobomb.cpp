#include "tot_custom_rel.h"     // For externed units

#include "evt_cmd.h"
#include "tot_generate_enemy.h"
#include "tot_manager_achievements.h"

#include <gc/types.h>
#include <ttyd/battle.h>
#include <ttyd/battle_camera.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_event_default.h>
#include <ttyd/battle_event_subset.h>
#include <ttyd/battle_sub.h>
#include <ttyd/battle_unit.h>
#include <ttyd/battle_weapon_power.h>
#include <ttyd/eff_miss_star.h>
#include <ttyd/eff_updown.h>
#include <ttyd/evt_eff.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evt_sub.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>

#include <cstdint>

namespace mod::tot::custom {

namespace {

// Using entire namespace for convenience.
using namespace ::ttyd::battle_camera;
using namespace ::ttyd::battle_database_common;
using namespace ::ttyd::battle_event_cmd;
using namespace ::ttyd::battle_event_default;
using namespace ::ttyd::battle_event_subset;
using namespace ::ttyd::evt_eff;
using namespace ::ttyd::evt_snd;
using namespace ::ttyd::evt_sub;

using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

}  // namespace

// Function / USER_FUNC declarations.
EVT_DECLARE_USER_FUNC(evtTot_CheckCanWaitToCounterattack, 1)
EVT_DECLARE_USER_FUNC(evtTot_HandleFailedChargeEffect, 1)
EVT_DECLARE_USER_FUNC(evtTot_SpawnUpDownEffect, 2)

// Unit work variable definitions.
constexpr const int32_t UW_BattleUnitType = 0;
constexpr const int32_t UW_FuseLitSfx = 1;
constexpr const int32_t UW_Angered = 2;

// Unit data.
int8_t unitBobOmb_defense[] = { 1, 1, 1, 1, 1 };
int8_t unitBobOmb_defense_attr[] = { 0, 0, 0, 0, 0 };

// Shared for both, at least for now.
StatusVulnerability unitBobOmb_status = {
     70, 100,  70,   0,  70, 150, 100, 100,
    100,  90, 100,  90, 100,  95,  80,  90,
     80, 100,  80, 100, 100,  95,
};

PoseTableEntry unitBobOmb_pose_table[] = {
    1, "BOM_N_1",
    2, "BOM_Y_1",
    9, "BOM_Y_1",
    5, "BOM_K_1",
    4, "BOM_X_1",
    3, "BOM_X_1",
    28, "BOM_S_1",
    29, "BOM_Q_1",
    30, "BOM_Q_1",
    31, "BOM_S_1",
    39, "BOM_D_1",
    50, "BOM_A_1",
    42, "BOM_R_1",
    40, "BOM_W_1",
    56, "BOM_X_1",
    57, "BOM_X_1",
    65, "BOM_T_1",
    69, "BOM_S_1",
};

PoseTableEntry unitBobOmb_pose_table_angry[] = {
    1, "BOM_N_2",
    2, "BOM_Y_2",
    9, "BOM_Y_2",
    5, "BOM_K_2",
    4, "BOM_X_2",
    3, "BOM_X_2",
    28, "BOM_S_2",
    29, "BOM_Q_2",
    30, "BOM_Q_2",
    31, "BOM_S_2",
    39, "BOM_D_2",
    50, "BOM_A_2",
    42, "BOM_R_2",
    40, "BOM_W_2",
    56, "BOM_X_2",
    57, "BOM_X_2",
    65, "BOM_T_2",
    69, "BOM_S_2",
};

BattleWeapon unitBobOmb_weapon = {
    .name = nullptr,
    .icon = 0,
    .item_id = 0,
    .description = nullptr,
    .base_accuracy = 100,
    .base_fp_cost = 0,
    .base_sp_cost = 0,
    .superguards_allowed = 1,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 1,
    .bingo_card_chance = 1,
    .unk_1b = 1,
    .damage_function = (void*)ttyd::battle_weapon_power::weaponGetPowerDefault,
    .damage_function_params = { 2, 0, 0, 0, 0, 0, 0, 0 },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags =
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_SELECT_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::SHELL_TOSS_LIKE |
        AttackTargetProperty_Flags::ONLY_FRONT |
        AttackTargetProperty_Flags::CANNOT_TARGET_CEILING,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::FREEZE_BREAK,
    .counter_resistance_flags = 0,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::PREFER_FRONT |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000,
        
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

BattleWeapon unitBobOmb_weaponBomb = {
    .name = nullptr,
    .icon = 0,
    .item_id = 0,
    .description = nullptr,
    .base_accuracy = 100,
    .base_fp_cost = 0,
    .base_sp_cost = 0,
    .superguards_allowed = 2,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 1,
    .bingo_card_chance = 1,
    .unk_1b = 1,
    .damage_function = (void*)ttyd::battle_weapon_power::weaponGetPowerDefault,
    .damage_function_params = { 5, 0, 0, 0, 0, 0, 0, 0 },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags =
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_SELECT_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::SHELL_TOSS_LIKE |
        AttackTargetProperty_Flags::ONLY_FRONT |
        AttackTargetProperty_Flags::CANNOT_TARGET_CEILING,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::FLIPS_BOMB |
        AttackSpecialProperty_Flags::FREEZE_BREAK,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::PREFER_FRONT |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000,
        
    // status chances
    
    .attack_evt_code = nullptr,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 25,
    .bg_a2_fall_weight = 25,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 0,
    .nozzle_fire_chance = 0,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

// EVTs...

EVT_BEGIN(unitBobOmb_bomb_event)
    USER_FUNC(evt_btl_camera_shake_w, 0, 5, 0, 15, 0)
    USER_FUNC(btlevtcmd_OnAttribute, -2, 16777216)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 1, 33554432)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_FuseLitSfx, LW(0))
    IF_NOT_EQUAL(LW(0), -1)
        USER_FUNC(evt_snd_sfxoff, LW(0))
    END_IF()
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_FuseLitSfx, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_ENEMY_DIE1_2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_ENM_BOMB_ATTACK2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(evt_eff, PTR(""), PTR("sandars"), 0, LW(0), LW(1), LW(2), 0, 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(btlevtcmd_StageDispellFog)
    USER_FUNC(btlevtcmd_GetCoin, -2, LW(0))
    USER_FUNC(btlevtcmd_StoreCoin, LW(0))
    USER_FUNC(btlevtcmd_GetExp, -2, LW(0))
    USER_FUNC(btlevtcmd_StoreExp, LW(0))
    IF_LARGE_EQUAL(LW(0), 1)
        DO(LW(0))
        WHILE()
    END_IF()
    WAIT_MSEC(1000)
    IF_EQUAL(LW(15), 1)
        USER_FUNC(btlevtcmd_WaitAttackEnd)
    END_IF()
    USER_FUNC(btlevtcmd_KillUnit, -2, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitBobOmb_damage_event_spark)
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_CheckDamageCode, LW(10), 256, LW(0))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evtTot_CheckCanWaitToCounterattack, LW(0))
        IF_EQUAL(LW(0), 1)
            USER_FUNC(btlevtcmd_AfterReactionEntry, -2, 52)
            RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
        ELSE()
            SET(LW(15), 1)
            RUN_CHILD_EVT(PTR(&unitBobOmb_bomb_event))
        END_IF()
    ELSE()
        RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitBobOmb_attack_event_spark)
    SET(LW(9), PTR(&unitBobOmb_weaponBomb))

    // Neither type should check for using items when angry.

    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(9))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(9), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            RETURN()
        ELSE()
            // If angry, but no valid targets (and not confused), explode.
            RUN_CHILD_EVT(unitBobOmb_bomb_event)
            RETURN()
        END_IF()
        GOTO(99)
    END_IF()

    USER_FUNC(evt_btl_camera_set_mode, 0, 8)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, LW(3))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BOMB_MOVE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.0), FLOAT(0.7))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 0, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BOMB_MOVE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.0), FLOAT(0.7))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 0, -1)
    
    // If Hyper Bob-omb was attacked, apply Charge status before attacking.
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Angered, LW(5))
    IF_EQUAL(LW(5), 1)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(5))
        IF_EQUAL(LW(5), (int32_t)BattleUnitType::TOT_HYPER_BOB_OMB)
            // Change to idle pose.
            USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 69)

            // Try to Charge (applying status directly), unless Allergic.
            USER_FUNC(btlevtcmd_CheckStatus, -2, 0, LW(5))
            IF_EQUAL(LW(5), 0)
                USER_FUNC(
                    evtTot_GetEnemyStats, 
                    (int32_t)BattleUnitType::TOT_HYPER_BOB_OMB,
                    EVT_NULLPTR, LW(5), EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR,
                    (int32_t)unitBobOmb_weaponBomb.damage_function_params[0])
                USER_FUNC(btlevtcmd_OnOffStatus, -2, 16, LW(5), LW(5), 1)
                USER_FUNC(evtTot_SpawnUpDownEffect, -2, LW(5))
                USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_CONDITION_CHARGE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
            ELSE()
                USER_FUNC(evtTot_HandleFailedChargeEffect, -2)
                USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_ATTACK_MISS2"), EVT_NULLPTR, 0, EVT_NULLPTR)
            END_IF()

            // Wait a bit, then jump again before attacking.
            WAIT_MSEC(1000)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BOMB_MOVE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.0), FLOAT(0.7))
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 0, -1)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BOMB_MOVE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.0), FLOAT(0.7))
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 0, -1)
        END_IF()
    END_IF()
    
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(10.0))
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), 10)
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    SWITCH(LW(5))
        CASE_OR(4)
            GOTO(90)
            CASE_END()
        CASE_EQUAL(3)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 38)
            GOTO(90)
            CASE_END()
        CASE_EQUAL(6)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 39)
            GOTO(90)
        CASE_EQUAL(2)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 40)
            GOTO(90)
        CASE_EQUAL(1)
            GOTO(91)
            CASE_END()
        CASE_ETC()
            GOTO(98)
            CASE_END()
    END_SWITCH()
LBL(90)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(10.0))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 500)
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    SET(LW(15), 0)
    RUN_CHILD_EVT(PTR(&unitBobOmb_bomb_event))
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    WAIT_MSEC(1000)
    RETURN()
LBL(91)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    SET(LW(15), 0)
    RUN_CHILD_EVT(PTR(&unitBobOmb_bomb_event))
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    WAIT_MSEC(1000)
    RETURN()
LBL(98)
    WAIT_MSEC(1000)
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitBobOmb_wait_event_spark)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_FuseLitSfx, LW(15))
    IF_EQUAL(LW(15), -1)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BOMB_WAIT1"), EVT_NULLPTR, 0, LW(15))
        USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_FuseLitSfx, LW(15))
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitBobOmb_attack_event)
    SET(LW(9), PTR(&unitBobOmb_weapon))
    USER_FUNC(btlevtcmd_EnemyItemUseCheck, -2, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(LW(0))
        USER_FUNC(btlevtcmd_StartWaitEvent, -2)
        RETURN()
    END_IF()

    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(9))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(9), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            RETURN()
        END_IF()
        GOTO(99)
    END_IF()

    // If Hyper Bob-omb, immediately light fuse and attack.
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(5))
    IF_EQUAL(LW(5), (int32_t)BattleUnitType::TOT_HYPER_BOB_OMB)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BOMB_SPARK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitBobOmb_wait_event_spark))
        USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitBobOmb_attack_event_spark))
        USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitBobOmb_damage_event_spark))
        USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitBobOmb_attack_event_spark))
        USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitBobOmb_pose_table_angry))
        USER_FUNC(btlevtcmd_OnPartsCounterAttribute, -2, 1, 8192)
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 69)
        WAIT_MSEC(400)
        
        RUN_CHILD_EVT(PTR(&unitBobOmb_attack_event_spark))
        RETURN()
    END_IF()

    USER_FUNC(evt_btl_camera_set_mode, 0, 8)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, LW(3))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BOMB_MOVE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.0), FLOAT(0.7))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 0, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BOMB_MOVE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.0), FLOAT(0.7))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 0, -1)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(10.0))
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), 10)
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), 0, LW(2), 0, -1, 0)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    SWITCH(LW(5))
        CASE_OR(4)
            GOTO(90)
            CASE_END()
        CASE_EQUAL(3)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 38)
            GOTO(90)
            CASE_END()
        CASE_EQUAL(6)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 39)
            GOTO(90)
        CASE_EQUAL(2)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 40)
            GOTO(90)
        CASE_EQUAL(1)
            GOTO(91)
            CASE_END()
        CASE_ETC()
            GOTO(98)
            CASE_END()
    END_SWITCH()
LBL(90)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(5.0))
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 30)
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), 0, LW(2), 0, -1, 1)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.0))
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 60)
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), 0, LW(2), 0, -1, 0)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(1.0))
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 90)
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), 0, LW(2), 0, -1, 1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("BOM_N_1"))
    WAIT_MSEC(500)
    GOTO(98)
LBL(91)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(2.0), FLOAT(0.5))
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 75)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 20, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BOMB_MOVE3"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(1.0), FLOAT(0.5))
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 100)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 20, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BOMB_MOVE3"), EVT_NULLPTR, 0, EVT_NULLPTR)
LBL(98)
    USER_FUNC(btlevtcmd_SetMoveSoundDataWork, -2, PTR("SFX_ENM_BOMB_MOVE4"), PTR("SFX_ENM_BOMB_MOVE4"), 0, 10, 10, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(5.0))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -2, 1)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitBobOmb_change_spark_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 39)
    USER_FUNC(btlevtcmd_CheckDamageCode, -2, 256, LW(0))
    IF_EQUAL(LW(0), 0)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_GetHpDamage, -2, LW(15))
    IF_LARGE_EQUAL(LW(15), 1)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BOMB_SPARK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitBobOmb_wait_event_spark))
        USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitBobOmb_attack_event_spark))
        USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitBobOmb_damage_event_spark))
        USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitBobOmb_attack_event_spark))
        USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitBobOmb_pose_table_angry))
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 39)
        USER_FUNC(btlevtcmd_OnPartsCounterAttribute, -2, 1, 8192)
        USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Angered, 1)
    END_IF()
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitBobOmb_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_CheckDamageCode, LW(10), 256, LW(0))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(btlevtcmd_GetDamageCode, LW(10), LW(0))
        SWITCH(LW(0))
            CASE_OR(24)
            CASE_OR(27)
            CASE_OR(25)
                USER_FUNC(evtTot_CheckCanWaitToCounterattack, LW(0))
                IF_EQUAL(LW(0), 1)
                    USER_FUNC(btlevtcmd_AfterReactionEntry, -2, 52)
                ELSE()
                    SET(LW(15), 1)
                    RUN_CHILD_EVT(PTR(&unitBobOmb_bomb_event))
                    RETURN()
                END_IF()
                CASE_END()
        END_SWITCH()
    END_IF()
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    USER_FUNC(btlevtcmd_CheckStatus, -2, 27, LW(0))
    IF_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_CheckActStatus, -2, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&unitBobOmb_change_spark_event))
        END_IF()
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitBobOmb_wait_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitBobOmb_common_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitBobOmb_wait_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitBobOmb_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitBobOmb_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitBobOmb_attack_event))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_FuseLitSfx, -1)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Angered, 0)
    USER_FUNC(btlevtcmd_SetRunSound, -2, PTR("SFX_ENM_BOMB_MOVE2"), PTR("SFX_ENM_BOMB_MOVE2"), 0, 4, 4)
    USER_FUNC(btlevtcmd_SetWalkSound, -2, PTR("SFX_ENM_BOMB_MOVE2"), PTR("SFX_ENM_BOMB_MOVE2"), 0, 10, 10)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitBobOmb_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType,
        (int32_t)BattleUnitType::BOB_OMB)
    RUN_CHILD_EVT(unitBobOmb_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitHyperBobOmb_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType,
        (int32_t)BattleUnitType::TOT_HYPER_BOB_OMB)
    RUN_CHILD_EVT(unitBobOmb_common_init_event)
    RETURN()
EVT_END()

DataTableEntry unitBobOmb_data_table[] = {
    48, (void*)btldefaultevt_Dummy,
    52, (void*)unitBobOmb_attack_event_spark,
    0, nullptr,
};

BattleUnitKindPart unitBobOmb_parts = {
    .index = 1,
    .name = "btl_un_bomhei",
    .model_name = "c_bomhey",
    .part_offset_pos = { 0.0f, 0.0f, 0.0f },
    .part_hit_base_offset = { 0.0f, 25.0f, 0.0f },
    .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
    .unk_30 = 20,
    .unk_32 = 30,
    .base_alpha = 255,
    .defense = unitBobOmb_defense,
    .defense_attr = unitBobOmb_defense_attr,
    .attribute_flags = 0x0000'0009,
    .counter_attribute_flags = 0x0000'1000,
    .pose_table = unitBobOmb_pose_table,
};

BattleUnitKindPart unitHyperBobOmb_parts = {
    .index = 1,
    .name = "btl_un_sinnosuke",     // Replaces Bald Cleft
    .model_name = "c_bomhey_h",
    .part_offset_pos = { 0.0f, 0.0f, 0.0f },
    .part_hit_base_offset = { 0.0f, 25.0f, 0.0f },
    .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
    .unk_30 = 20,
    .unk_32 = 30,
    .base_alpha = 255,
    .defense = unitBobOmb_defense,
    .defense_attr = unitBobOmb_defense_attr,
    .attribute_flags = 0x0000'0009,
    .counter_attribute_flags = 0x0000'1000,
    .pose_table = unitBobOmb_pose_table,
};

BattleUnitKind unit_BobOmb = {
    .unit_type = BattleUnitType::BOB_OMB,
    .unit_name = "btl_un_bomhei",
    .max_hp = 4,
    .max_fp = 0,
    .danger_hp = 1,
    .peril_hp = 1,
    .level = 12,
    .bonus_exp = 0,
    .bonus_coin = 0,
    .bonus_coin_rate = 100,
    .base_coin = 0,
    .run_rate = 50,
    .pb_soft_cap = 9999,
    .width = 32,
    .height = 30,
    .hit_offset = { 4, 30 },
    .center_offset = { 0.0f, 15.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 16.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 16.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 11.0f, 16.0f, 10.0f },
    .cut_base_offset = { 2.0f, 15.0f, 0.0f },
    .cut_width = 32.0f,
    .cut_height = 30.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_BOMB_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitBobOmb_status,
    .num_parts = 1,
    .parts = &unitBobOmb_parts,
    .init_evt_code = (void*)unitBobOmb_init_event,
    .data_table = unitBobOmb_data_table,
};

BattleUnitKind unit_HyperBobOmb = {
    .unit_type = BattleUnitType::TOT_HYPER_BOB_OMB,
    .unit_name = "btl_un_sinnosuke",        // Replaces Bald Cleft.
    .max_hp = 4,
    .max_fp = 0,
    .danger_hp = 1,
    .peril_hp = 1,
    .level = 12,
    .bonus_exp = 0,
    .bonus_coin = 0,
    .bonus_coin_rate = 100,
    .base_coin = 0,
    .run_rate = 50,
    .pb_soft_cap = 9999,
    .width = 32,
    .height = 30,
    .hit_offset = { 4, 30 },
    .center_offset = { 0.0f, 15.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 16.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 16.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 11.0f, 16.0f, 10.0f },
    .cut_base_offset = { 2.0f, 15.0f, 0.0f },
    .cut_width = 32.0f,
    .cut_height = 30.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_BOMB_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitBobOmb_status,
    .num_parts = 1,
    .parts = &unitHyperBobOmb_parts,
    .init_evt_code = (void*)unitHyperBobOmb_init_event,
    .data_table = unitBobOmb_data_table,
};

EVT_DEFINE_USER_FUNC(evtTot_CheckCanWaitToCounterattack) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    int32_t id = ttyd::battle_sub::BattleTransID(evt, -2);
    auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, id);

    // If the attack that landed was 'indirect' (volatile explosive resistance)
    // and the user is able to act and won't die anyway,
    // don't explode immediately, but queue a counter-attack instead.
    bool can_counterattack =
        unit->current_hp > 0 &&
        ttyd::battle_unit::BtlUnit_CanActStatus(unit) && 
        !ttyd::battle_unit::BtlUnit_CheckStatus(unit, StatusEffectType::OHKO) &&
        (unit->last_target_weapon_cr_flags &
            AttackCounterResistance_Flags::VOLATILE_EXPLOSIVE) != 0;
    evtSetValue(evt, evt->evtArguments[0], can_counterattack);

    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_HandleFailedChargeEffect) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    int32_t id = evtGetValue(evt, evt->evtArguments[0]);
    id = ttyd::battle_sub::BattleTransID(evt, id);
    auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, id);
    
    float x, y, z;
    ttyd::battle_unit::BtlUnit_GetPos(unit, &x, &y, &z);
    y += 0.5f * ttyd::battle_unit::BtlUnit_GetHeight(unit);
    
    ttyd::eff_miss_star::effMissStarEntry(x, y, z, 0, 1, 1);

    // Treat as failed Charge for purposes of achievement.
    AchievementsManager::MarkCompleted(AchievementId::V2_MISC_ALLERGIC);
    
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_SpawnUpDownEffect) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    int32_t id = evtGetValue(evt, evt->evtArguments[0]);
    id = ttyd::battle_sub::BattleTransID(evt, id);
    auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, id);
    
    int32_t strength = evtGetValue(evt, evt->evtArguments[1]);
    
    float x, y, z;
    ttyd::battle_unit::BtlUnit_GetPos(unit, &x, &y, &z);
    y += 0.5f * ttyd::battle_unit::BtlUnit_GetHeight(unit);
    z += 40.0f;
    
    ttyd::eff_updown::effUpdownEntry(x, y, z, 0, strength, 60);
    
    return 2;
}

}  // namespace mod::tot::custom