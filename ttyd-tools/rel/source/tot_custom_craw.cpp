#include "tot_custom_rel.h"     // For externed units

#include "evt_cmd.h"

#include <gc/types.h>
#include <ttyd/battle_camera.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_event_default.h>
#include <ttyd/battle_event_subset.h>
#include <ttyd/battle_weapon_power.h>
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

// Unit work variable definitions.
constexpr const int32_t UW_BattleUnitType = 0;

// Unit data.
int8_t unitCraw_defense[] = { 0, 0, 0, 0, 0 };
int8_t unitCraw_defense_attr[] = { 0, 0, 0, 0, 0 };

// Shared for both; surprisingly Dark Craws already have high status rates.
StatusVulnerability unitCraw_status = {
     90,  80,  80, 100,  90, 100, 100,  70,
    100,  90, 100,  90, 100,  95,  70,   0, 
     60, 100,  70, 100, 100,  95
};

PoseTableEntry unitCraw_pose_table[] = {
    1, "N_1",
    2, "Y_1",
    9, "Y_1",
    5, "K_1",
    4, "X_1",
    3, "X_1",
    28, "S_2",
    29, "Q_1",
    30, "Q_1",
    31, "A_1",
    39, "D_1",
    50, "A_1",
    42, "R_1",
    40, "W_1",
    56, "X_1",
    57, "X_1",
    65, "T_1",
    69, "S_2",
};

PoseSoundTimingEntry unitCraw_pose_sound_timing_table[] = {
    { "S_2", 0.6666667f, 0, "SFX_ENM_MONBAN_WAIT1", 1 },
    { nullptr, 0.0f, 0, nullptr, 1 },
};

BattleWeapon unitCraw_weaponThrow = {
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
    .damage_function_params = { 6, 0, 0, 0, 0, 0, 0, 0 },
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
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::GROUNDS_WINGED,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM,
        
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

BattleWeapon unitCraw_weaponRam = {
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
    .damage_function_params = { 6, 0, 0, 0, 0, 0, 0, 0 },
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
        AttackTargetProperty_Flags::HAMMERLIKE |
        AttackTargetProperty_Flags::ONLY_FRONT,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::FREEZE_BREAK,
    .counter_resistance_flags = 
        AttackCounterResistance_Flags::ALL &
        ~AttackCounterResistance_Flags::PREEMPTIVE_SPIKY,
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

BattleWeapon unitCraw_weaponMultiRam = {
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
    .damage_function_params = { 6, 0, 0, 0, 0, 0, 0, 0 },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags =
        AttackTargetClass_Flags::MULTIPLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_SELECT_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::HAMMERLIKE,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::FREEZE_BREAK,
    .counter_resistance_flags = 
        AttackCounterResistance_Flags::ALL &
        ~AttackCounterResistance_Flags::PREEMPTIVE_SPIKY,
    .target_weighting_flags =
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

// EVTs...

EVT_BEGIN(unitCraw_spike_counter_event)
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_CheckStatus, -2, 10, LW(3))
    IF_NOT_EQUAL(LW(3), 0)
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 18)
        ADD(LW(1), 60)
        GOTO(90)
    END_IF()
    USER_FUNC(btlevtcmd_CheckStatus, -2, 11, LW(3))
    IF_NOT_EQUAL(LW(3), 0)
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 10)
        ADD(LW(1), 33)
        GOTO(90)
    END_IF()
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 14)
    ADD(LW(1), 48)
    GOTO(90)
    LBL(90)
    USER_FUNC(evt_eff, PTR(""), PTR("toge_flush"), 0, LW(0), LW(1), LW(2), 60, 0, 0, 0, 0, 0, 0, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitCraw_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    RETURN()
EVT_END()

EVT_BEGIN(unitCraw_normal_attack_event)
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_GetFirstAttackTarget, LW(3), LW(4))
    ELSE()
        USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
        USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(9))
        USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(9), LW(3), LW(4))
    END_IF()
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            RETURN()
        END_IF()
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(evt_btl_camera_set_mode, 0, 8)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, LW(3))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MONBAN_MOVE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_1"))
    WAIT_FRM(20)
    BROTHER_EVT()
        DO(6)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MONBAN_MOVE2R"), EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_FRM(5)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MONBAN_MOVE2L"), EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_FRM(5)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("R_2"))
    WAIT_FRM(60)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MONBAN_MOVE3"), EVT_NULLPTR, 0, LW(15))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    SET(LW(5), 30)
    USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(6))
    MULF(LW(5), LW(6))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), LW(5))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(8.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), 0, LW(2), 0, -1, 0)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), LW(9), 0, LW(5))
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
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 30)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(8.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), 0, LW(2), 0, -1, 0)
    WAIT_FRM(60)
    GOTO(98)
LBL(91)
    USER_FUNC(evt_btl_camera_shake_h, 0, 1, 1, 8, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_add_zoom, 0, -50)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 20)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 0, FLOAT(1.0), FLOAT(1.0))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 0, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MONBAN_MOVE2R"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 10)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 0, FLOAT(1.0), FLOAT(1.0))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 0, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MONBAN_MOVE2L"), EVT_NULLPTR, 0, EVT_NULLPTR)
LBL(98)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(5.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), 0, LW(2), 0, -1, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("T_1"))
    DO(3)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MONBAN_LAUGH1"), EVT_NULLPTR, 0, LW(15))
        WAIT_FRM(10)
    WHILE()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_2"))
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitCraw_multi_attack_event)
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(unitCraw_normal_attack_event)
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
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(evt_btl_camera_set_mode, 0, 8)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, LW(3))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MONBAN_MOVE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_1"))
    WAIT_FRM(20)
    BROTHER_EVT()
        DO(6)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MONBAN_MOVE2R"), EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_FRM(5)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MONBAN_MOVE2L"), EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_FRM(5)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("R_2"))
    WAIT_FRM(60)
    
    // Do a little hop before attack as tell.
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.0), FLOAT(0.7))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 0, -1)
    
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MONBAN_MOVE3"), EVT_NULLPTR, 0, LW(15))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    SET(LW(5), 30)
    USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(6))
    MULF(LW(5), LW(6))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), LW(5))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(10.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), 0, LW(2), 0, -1, 0)
    // For tracking defensive AC check.
    SET(LW(13), 0)
LBL(10)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(10.0))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), LW(9), 0, LW(5))
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
    USER_FUNC(btlevtcmd_FaceDirectionSub, LW(3), LW(0), 100)
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), 0, LW(2), 0, 0, 0)
    GOTO(97)
LBL(91)    
    IF_EQUAL(LW(13), 0)
        USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
        SET(LW(13), 1)
    END_IF()
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
LBL(97)
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    IF_NOT_EQUAL(LW(3), -1)
        GOTO(10)
    END_IF()
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 500)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(10.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, 0, 0)
    WAIT_MSEC(300)
    USER_FUNC(btlevtcmd_SetPos, -2, 250, LW(1), LW(2))
LBL(98)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(10.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), 0, LW(2), 0, -1, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("T_1"))
    DO(3)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MONBAN_LAUGH1"), EVT_NULLPTR, 0, LW(15))
        WAIT_FRM(10)
    WHILE()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_2"))
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitCraw_throw_attack_event)
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_GetFirstAttackTarget, LW(3), LW(4))
    ELSE()
        USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
        USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(9))
        USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(9), LW(3), LW(4))
    END_IF()
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            RETURN()
        END_IF()
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(evt_btl_camera_set_mode, 0, 8)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, LW(3))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MONBAN_THROW1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_2"))
    WAIT_FRM(28)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MONBAN_THROW2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 2, 16777216)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 2, 33554432)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 2, PTR("A_3"))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 5)
    ADD(LW(1), 10)
    USER_FUNC(btlevtcmd_SetPartsPos, -2, 2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetPartsRotateOffset, -2, 2, 10, 0, 0)
    IF_EQUAL(LW(15), -1)
        USER_FUNC(btlevtcmd_SetPartsRotate, -2, 2, 0, 0, -45)
    ELSE()
        USER_FUNC(btlevtcmd_SetPartsRotate, -2, 2, 0, 180, -45)
    END_IF()
    BROTHER_EVT()
        DO(30)
            USER_FUNC(btlevtcmd_AddPartsRotate, -2, 2, 0, 0, 3)
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPartsSetting, -2, 2, 30, 0, FLOAT(0.5))
    USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 2, LW(0), LW(1), LW(2), 30, -1)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), LW(9), 0, LW(5))
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
    USER_FUNC(btlevtcmd_JumpPartsContinue, -2, 2)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 2, 16777216)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 2, 33554432)
    GOTO(98)
    LBL(91)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 2, 16777216)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 2, 33554432)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    WAIT_FRM(2)
    GOTO(98)
    LBL(98)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("T_1"))
    DO(3)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MONBAN_LAUGH1"), EVT_NULLPTR, 0, LW(15))
        WAIT_FRM(10)
    WHILE()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_2"))
    LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitCraw_attack_event)
    USER_FUNC(btlevtcmd_EnemyItemUseCheck, -2, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(LW(0))
        USER_FUNC(btlevtcmd_StartWaitEvent, -2)
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(0))
    IF_EQUAL(LW(0), (int32_t)BattleUnitType::DARK_CRAW)
        SET(LW(0), 50 + 50 + 20 - 1)
    ELSE()
        SET(LW(0), 50 + 50 - 1)
    END_IF()
    USER_FUNC(evt_sub_random, LW(0), LW(1))
    IF_SMALL(LW(1), 50)
        SET(LW(9), PTR(&unitCraw_weaponThrow))
        RUN_CHILD_EVT(PTR(&unitCraw_throw_attack_event))
        GOTO(99)
    END_IF()
    IF_SMALL(LW(1), 100)
        SET(LW(9), PTR(&unitCraw_weaponRam))
        RUN_CHILD_EVT(PTR(&unitCraw_normal_attack_event))
        GOTO(99)
    END_IF()
    SET(LW(9), PTR(&unitCraw_weaponMultiRam))
    RUN_CHILD_EVT(PTR(&unitCraw_multi_attack_event))
LBL(99)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitCraw_wait_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitCraw_common_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitCraw_wait_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitCraw_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitCraw_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitCraw_attack_event))
    USER_FUNC(btlevtcmd_SetRunSound, -2, PTR("SFX_ENM_MONBAN_MOVE2R"), PTR("SFX_ENM_MONBAN_MOVE2L"), 0, 5, 5)
    USER_FUNC(btlevtcmd_SetWalkSound, -2, PTR("SFX_ENM_MONBAN_MOVE2R"), PTR("SFX_ENM_MONBAN_MOVE2L"), 0, 10, 10)
    USER_FUNC(btlevtcmd_AnimeSetPoseSoundTable, -2, 1, PTR(&unitCraw_pose_sound_timing_table))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitCraw_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType,
        (int32_t)BattleUnitType::GUS)
    RUN_CHILD_EVT(unitCraw_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitDarkCraw_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType,
        (int32_t)BattleUnitType::DARK_CRAW)
    RUN_CHILD_EVT(unitCraw_common_init_event)
    RETURN()
EVT_END()

DataTableEntry unitCraw_data_table[] = {
    37, (void*)unitCraw_spike_counter_event,
    48, (void*)btldefaultevt_Dummy,
    0, nullptr,
};

BattleUnitKindPart unitCraw_parts[] = {
    {
        .index = 1,
        .name = "btl_un_monban",
        .model_name = "c_monban",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 8.0f, 35.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 45.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitCraw_defense,
        .defense_attr = unitCraw_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0x0000'0001,
        .pose_table = unitCraw_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_monban",
        .model_name = "c_monban",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitCraw_defense,
        .defense_attr = unitCraw_defense_attr,
        .attribute_flags = 0x1301'0000,
        .counter_attribute_flags = 0,
        .pose_table = unitCraw_pose_table,
    },
};

BattleUnitKindPart unitDarkCraw_parts[] = {
    {
        .index = 1,
        .name = "btl_un_dark_keeper",
        .model_name = "c_monban_t",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 8.0f, 35.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 45.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitCraw_defense,
        .defense_attr = unitCraw_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0x0000'0001,
        .pose_table = unitCraw_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_dark_keeper",
        .model_name = "c_monban_t",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitCraw_defense,
        .defense_attr = unitCraw_defense_attr,
        .attribute_flags = 0x1301'0000,
        .counter_attribute_flags = 0,
        .pose_table = unitCraw_pose_table,
    },
};

BattleUnitKind unit_Craw = {
    .unit_type = BattleUnitType::GUS,
    .unit_name = "btl_un_monban",
    .max_hp = 20,
    .max_fp = 0,
    .danger_hp = 1,
    .peril_hp = 1,
    .level = 45,
    .bonus_exp = 0,
    .bonus_coin = 0,
    .bonus_coin_rate = 100,
    .base_coin = 0,
    .run_rate = 70,
    .pb_soft_cap = 9999,
    .width = 31,
    .height = 46,
    .hit_offset = { 0, 46 },
    .center_offset = { 0.0f, 20.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 16.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 15.5f, 0.0f, 0.0f },
    .kiss_hit_offset = { 15.5f, 29.9f, 10.0f },
    .cut_base_offset = { 0.0f, 20.0f, 0.0f },
    .cut_width = 28.0f,
    .cut_height = 40.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_MONBAN_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitCraw_status,
    .num_parts = 2,
    .parts = unitCraw_parts,
    .init_evt_code = (void*)unitCraw_init_event,
    .data_table = unitCraw_data_table,
};

BattleUnitKind unit_DarkCraw = {
    .unit_type = BattleUnitType::DARK_CRAW,
    .unit_name = "btl_un_dark_keeper",
    .max_hp = 20,
    .max_fp = 0,
    .danger_hp = 1,
    .peril_hp = 1,
    .level = 28,
    .bonus_exp = 0,
    .bonus_coin = 0,
    .bonus_coin_rate = 100,
    .base_coin = 0,
    .run_rate = 50,
    .pb_soft_cap = 9999,
    .width = 31,
    .height = 46,
    .hit_offset = { 0, 46 },
    .center_offset = { 0.0f, 20.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 16.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 15.5f, 0.0f, 0.0f },
    .kiss_hit_offset = { 15.5f, 29.9f, 10.0f },
    .cut_base_offset = { 0.0f, 20.0f, 0.0f },
    .cut_width = 28.0f,
    .cut_height = 40.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_MONBAN_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitCraw_status,
    .num_parts = 2,
    .parts = unitDarkCraw_parts,
    .init_evt_code = (void*)unitDarkCraw_init_event,
    .data_table = unitCraw_data_table,
};

}  // namespace mod::tot::custom