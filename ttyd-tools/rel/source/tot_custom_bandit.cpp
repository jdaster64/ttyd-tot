#include "tot_custom_rel.h"     // For externed unit definitions.

#include "common_functions.h"
#include "evt_cmd.h"
#include "tot_custom_common.h"

#include <gc/mtx.h>
#include <gc/types.h>
#include <ttyd/_core_language_libs.h>
#include <ttyd/battle.h>
#include <ttyd/battle_camera.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_event_default.h>
#include <ttyd/battle_event_subset.h>
#include <ttyd/battle_icon.h>
#include <ttyd/battle_message.h>
#include <ttyd/battle_sub.h>
#include <ttyd/battle_unit.h>
#include <ttyd/battle_weapon_power.h>
#include <ttyd/evt_eff.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evt_sub.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/icondrv.h>
#include <ttyd/item_data.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/npcdrv.h>
#include <ttyd/system.h>

#include <cstdint>

namespace mod::tot::custom {

namespace {

// Using entire namespace for convenience.
using namespace ::ttyd::battle_camera;
using namespace ::ttyd::battle_database_common;
using namespace ::ttyd::battle_event_cmd;
using namespace ::ttyd::battle_event_default;
using namespace ::ttyd::battle_event_subset;
using namespace ::ttyd::battle_icon;
using namespace ::ttyd::battle_message;
using namespace ::ttyd::evt_eff;
using namespace ::ttyd::evt_snd;
using namespace ::ttyd::evt_sub;

using ::ttyd::battle::g_BattleWork;
using ::ttyd::battle_unit::BattleWorkUnit;
using ::ttyd::battle_unit::BattleWorkUnitPart;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

namespace IconType = ttyd::icondrv::IconType;
namespace ItemType = ttyd::item_data::ItemType;

// Constants (UW, etc.)
constexpr const int32_t UW_BattleUnitType = 0;

}  // namespace

// Function / USER_FUNC declarations.

EVT_DECLARE_USER_FUNC(_steal_item, 5)

// Unit data.

int8_t unitBandit_defense[] = { 0, 0, 0, 0, 0 };
int8_t unitBandit_defense_attr[] = { 0, 0, 0, 0, 0 };

StatusVulnerability unitBandit_status = {
    100,  90,  70, 100, 100, 100, 100,  70,
    100,  95, 100,  95, 100, 100,  90,  95,
     80, 100,  90, 100, 100,  95,
};
StatusVulnerability unitBigBandit_status = {
     90,  80,  60, 100,  90, 100, 100,  60,
    100,  90, 100,  90, 100,  95,  80,  85,
     70, 100,  80, 100, 100,  95,
};
StatusVulnerability unitBadgeBandit_status = {
     80,  70,  50, 100,  80, 100, 100,  50,
    100,  85, 100,  85, 100,  90,  80,  75,
     60, 100,  70, 100, 100,  60,
};

PoseTableEntry unitBandit_pose_table[] = {
    1, "N_1",
    2, "Y_1",
    9, "Y_1",
    5, "K_1",
    4, "X_1",
    3, "X_1",
    28, "S_1",
    29, "Q_1",
    30, "Q_1",
    31, "S_1",
    39, "D_1",
    50, "A_1",
    51, "A_2",
    42, "R_1",
    41, "R_1",
    40, "W_1",
    56, "X_1",
    57, "X_1",
    65, "T_1",
    69, "S_1",
};
PoseTableEntry unitBandit_item_pose_table[] = {
    1, "N_1",
    2, "Y_1",
    9, "Y_1",
    5, "K_1",
    4, "X_1",
    3, "X_1",
    28, "S_2",
    29, "Q_1",
    30, "Q_1",
    31, "S_2",
    39, "D_1",
    50, "A_1",
    51, "A_2",
    42, "R_2",
    41, "R_2",
    40, "W_2",
    65, "S_2",
    69, "S_2",
};

BattleWeapon unitBandit_weapon = {
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
    .damage_function_params = { 5, 0, 0, 0, 0, 0, 0, 0 },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    
    .target_class_flags =
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH |
        AttackTargetClass_Flags::ENEMY_SELECT_SIDE_HOME,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::HAMMERLIKE |
        AttackTargetProperty_Flags::ONLY_FRONT,
        
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags = AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags =
        AttackCounterResistance_Flags::TOP_SPIKY |
        // Added to make sure that front-spiky counter functions properly.
        AttackCounterResistance_Flags::PREEMPTIVE_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::PREFER_FRONT |
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

BattleUnitKindPart unitBandit_parts[] = {
    {
        .index = 1,
        .name = "btl_un_borodo",
        .model_name = "c_borodo",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitBandit_defense,
        .defense_attr = unitBandit_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitBandit_pose_table,
    },
};
BattleUnitKindPart unitBigBandit_parts[] = {
    {
        .index = 1,
        .name = "btl_un_borodo_king",
        .model_name = "c_borodo_g",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitBandit_defense,
        .defense_attr = unitBandit_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitBandit_pose_table,
    },
};
BattleUnitKindPart unitBadgeBandit_parts[] = {
    {
        .index = 1,
        .name = "btl_un_badge_borodo",
        .model_name = "c_borodo_t",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitBandit_defense,
        .defense_attr = unitBandit_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitBandit_pose_table,
    },
};

// Evt definitions.

EVT_BEGIN(unitBandit_revive_event)
    USER_FUNC(btlevtcmd_OnUnitFlag, LW(10), 0x800'0000)
    SET(LW(0), 48)
LBL(5)
    WAIT_FRM(1)
    USER_FUNC(btlevtcmd_AddRotate, LW(10), 0, -15, 0)
    SUB(LW(0), 1)
    IF_LARGE(LW(0), 0)
        GOTO(5)
    END_IF()
    USER_FUNC(btlevtcmd_SetRotate, LW(10), 0, 0, 0)
    USER_FUNC(btlevtcmd_SetRotateOffset, LW(10), 0, 0, 0)
    SET(LW(0), 30)
LBL(6)
    WAIT_FRM(1)
    USER_FUNC(btlevtcmd_AddRotate, LW(10), 3, 0, 0)
    SUB(LW(0), 1)
    IF_LARGE(LW(0), 0)
        GOTO(6)
    END_IF()
    USER_FUNC(btlevtcmd_ClearAllStatus, LW(10))
    USER_FUNC(btlevtcmd_ConsumeItemReserve, LW(10), (int32_t)ItemType::LIFE_SHROOM)
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_GetPos, LW(10), LW(0), LW(1), LW(2))
        ADD(LW(1), 40)
        USER_FUNC(btlevtcmd_DispItemIcon, LW(0), LW(1), LW(2), (int32_t)ItemType::LIFE_SHROOM, 120)
    END_BROTHER()
    WAIT_FRM(90)
    SET(LW(0), 30)
LBL(7)
    WAIT_FRM(1)
    USER_FUNC(btlevtcmd_AddRotate, LW(10), -3, 0, 0)
    SUB(LW(0), 1)
    IF_LARGE(LW(0), 0)
        GOTO(7)
    END_IF()
    USER_FUNC(btlevtcmd_OnOffStatus, LW(10), 27, 0, 0, 0)
    USER_FUNC(btlevtcmd_SetHp, LW(10), 0)
    USER_FUNC(btlevtcmd_RecoverHp, LW(10), LW(11), 10)
    USER_FUNC(btlevtcmd_GetPos, LW(10), LW(0), LW(1), LW(2))
    ADD(LW(1), 30)
    USER_FUNC(evt_eff, 0, PTR("recovery"), 0, LW(0), LW(1), LW(2), 10, 0, 0, 0, 0, 0, 0, 0)
    WAIT_MSEC(2000)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(10), LW(11), 42)
    USER_FUNC(btlevtcmd_JumpSetting, LW(10), 0, FLOAT(0.0), FLOAT(0.7))
    USER_FUNC(btlevtcmd_GetPos, LW(10), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, -1)
    USER_FUNC(btlevtcmd_OffUnitFlag, LW(10), 0x800'0000)
    USER_FUNC(btlevtcmd_OffUnitFlag, LW(10), 0x4000'0000)
    USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitBandit_pose_table))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitBandit_flee_event)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BORODO_WAIT1"), EVT_NULLPTR, 0, LW(15))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("R_3"))
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 180, 0)
    WAIT_MSEC(250)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.5))
    USER_FUNC(btlevtcmd_SetMoveSoundDataWork, -2, PTR("SFX_ENM_BORODO_JUMP1"), 0, 0, -1, -1, 0, 0)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), FLOAT(20.0), -2)
    USER_FUNC(btlevtcmd_SetPartsBlur, -2, 1, 255, 255, 255, 255, 100, 100, 100, 50, 0)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 1, 0x400'0000)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BORODO_ESCAPE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(9.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, 300, LW(1), LW(2), 0, 0, 0)
    USER_FUNC(btlevtcmd_SetPartsBlur, -2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0)
    WAIT_FRM(40)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, 0x400'0000)
    USER_FUNC(evt_snd_sfxoff, LW(15))
    USER_FUNC(btlevtcmd_KillUnit, -2, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitBandit_normal_attack_event)
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), PTR(&unitBandit_weapon))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, PTR(&unitBandit_weapon), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            RETURN()
        END_IF()
        GOTO(99)
    END_IF()
    USER_FUNC(evt_btl_camera_set_mode, 0, 8)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, LW(3))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    USER_FUNC(btlevtcmd_WeaponAftereffect, PTR(&unitBandit_weapon))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, PTR(&unitBandit_weapon))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), 60)
    USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(0), LW(2), 1, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(4.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 350)
    DO(8)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BORODO_MOVE1L"), EVT_NULLPTR, 0, LW(14))
        WAIT_FRM(3)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BORODO_MOVE1R"), EVT_NULLPTR, 0, LW(14))
        WAIT_FRM(3)
    WHILE()
    USER_FUNC(evt_btl_camera_set_mode, 0, 3)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), PTR(&unitBandit_weapon), 256, LW(5))
    IF_EQUAL(LW(5), 5)
        USER_FUNC(btlevtcmd_GetStatusMg, LW(3), LW(6))
        MULF(LW(6), 40)
        USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), LW(6))
        USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(0), LW(2), 1, LW(15))
        USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BORODO_ATT1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 50)
        USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(5.0))
        USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
        USER_FUNC(evt_snd_sfxoff, LW(14))
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&unitBandit_weapon), 256, LW(5))
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BORODO_ATT1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 50)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(5.0))
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), 20)
    USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    USER_FUNC(evt_snd_sfxoff, LW(14))
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
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.4))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 30)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 18, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BORODO_MOVE1L"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 20)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BORODO_MOVE1R"), EVT_NULLPTR, 0, EVT_NULLPTR)
    WAIT_FRM(20)
    GOTO(98)
LBL(91)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), PTR(&unitBandit_weapon))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&unitBandit_weapon), 256, LW(5))
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.25))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), 40)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("O_1"))
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(0))
        IF_LARGE_EQUAL(LW(0), 0)
            SET(LW(5), 0)
            SET(LW(6), 360)
        ELSE()
            SET(LW(5), 360)
            SET(LW(6), 0)
        END_IF()
        USER_FUNC(btlevtcmd_SetRotateOffsetFromCenterOffset, -2)
        USER_FUNC(evt_sub_intpl_init, 13, LW(5), LW(6), 24)
        DO(25)
            USER_FUNC(evt_sub_intpl_get_value)
            USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, LW(0))
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 36, -1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("X_1"))
    WAIT_MSEC(1000)
LBL(98)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(4.0))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitBandit_steal_attack_event)
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), PTR(&unitBandit_weapon))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, PTR(&unitBandit_weapon), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            RETURN()
        END_IF()
        GOTO(99)
    END_IF()
    USER_FUNC(evt_btl_camera_set_mode, 0, 8)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, LW(3))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    USER_FUNC(btlevtcmd_WeaponAftereffect, PTR(&unitBandit_weapon))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, PTR(&unitBandit_weapon))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), 60)
    USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(0), LW(2), 1, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(4.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 350)
    DO(8)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BORODO_MOVE1L"), EVT_NULLPTR, 0, LW(14))
        WAIT_FRM(3)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BORODO_MOVE1R"), EVT_NULLPTR, 0, LW(14))
        WAIT_FRM(3)
    WHILE()
    USER_FUNC(evt_btl_camera_set_mode, 0, 3)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), PTR(&unitBandit_weapon), 256, LW(5))
    IF_EQUAL(LW(5), 5)
        USER_FUNC(btlevtcmd_GetStatusMg, LW(3), LW(6))
        MULF(LW(6), 40)
        USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), LW(6))
        USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(0), LW(2), 1, LW(15))
        USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BORODO_ATT1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 50)
        USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(5.0))
        USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
        USER_FUNC(evt_snd_sfxoff, LW(14))
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&unitBandit_weapon), 256, LW(5))
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BORODO_ATT1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 50)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(5.0))
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), 20)
    USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    USER_FUNC(evt_snd_sfxoff, LW(14))
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
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.4))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 30)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 18, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BORODO_MOVE1L"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 20)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BORODO_MOVE1R"), EVT_NULLPTR, 0, EVT_NULLPTR)
    WAIT_FRM(20)
    GOTO(98)
LBL(91)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), PTR(&unitBandit_weapon))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&unitBandit_weapon), 256, LW(5))
    SET(LW(8), 0)
    USER_FUNC(btlevtcmd_GetResultACDefence, LW(7))
    IF_SMALL_EQUAL(LW(7), 3)
        USER_FUNC(_steal_item, -2, LW(3), LW(8), LW(9), LW(7))
        IF_NOT_EQUAL(LW(8), 0)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BORODO_SUCCESS1"), EVT_NULLPTR, 0, EVT_NULLPTR)
            INLINE_EVT()
                // Don't display stolen-item message if target is an enemy.
                USER_FUNC(evtTot_CheckSpeciesIsEnemy, LW(3), LW(0))
                IF_EQUAL(LW(0), 0)
                    WAIT_FRM(20)
                    USER_FUNC(btlevtcmd_AnnounceSetParam, 0, LW(8))
                    USER_FUNC(btlevtcmd_AnnounceMessage, 1, 0, 0, PTR("btl_msg_steal_item_lost"), 90)
                END_IF()
            END_INLINE()
            INLINE_EVT()
                USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_BtlIconEntry, LW(9), LW(0), LW(1), LW(2), LW(10))
                USER_FUNC(btlevtcmd_BtlIconSetFallAccel, LW(10), FLOAT(0.5))
                USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), 30)
                USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_BtlIconJumpPosition, LW(10), LW(0), LW(1), LW(2), 20)
                USER_FUNC(btlevtcmd_BtlIconDelete, LW(10))
                USER_FUNC(btlevtcmd_OnUnitFlag, -2, 0x4000'0000)
            END_INLINE()
        END_IF()
    END_IF()
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.5))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), 30)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 10, -1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("O_1"))
    WAIT_FRM(10)
    IF_EQUAL(LW(8), 0)
        WAIT_FRM(20)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BORODO_INCLINE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("B_1"))
        WAIT_FRM(52)
        USER_FUNC(evt_btl_camera_set_mode, 0, 0)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("R_4"))
        USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(4.0))
        USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("O_2"))
    WAIT_FRM(14)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BORODO_LAUGH1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitBandit_item_pose_table))
    GOTO(98)
LBL(98)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(4.0))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
LBL(99)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitBandit_attack_event)
    USER_FUNC(btlevtcmd_CheckUnitFlag, -2, 0x4000'0000, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(PTR(&unitBandit_flee_event))
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_EnemyItemUseCheck, -2, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(LW(0))
        USER_FUNC(btlevtcmd_StartWaitEvent, -2)
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(PTR(&unitBandit_normal_attack_event))
        RETURN()
    END_IF()

    // Set steal probability based on enemy type.
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(0))
    SWITCH(LW(0))
        CASE_EQUAL((int32_t)BattleUnitType::BANDIT)
            SET(LW(0), 225 - 1)
            SET(LW(1), 25)
        CASE_EQUAL((int32_t)BattleUnitType::BIG_BANDIT)
            SET(LW(0), 215 - 1)
            SET(LW(1), 75)
        CASE_ETC()
            SET(LW(0), 210 - 1)
            SET(LW(1), 70)
    END_SWITCH()

    USER_FUNC(evt_sub_random, LW(0), LW(0))
    IF_SMALL(LW(0), LW(1))
        RUN_CHILD_EVT(PTR(&unitBandit_normal_attack_event))
    ELSE()
        RUN_CHILD_EVT(PTR(&unitBandit_steal_attack_event))
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitBandit_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    RETURN()
EVT_END()

EVT_BEGIN(unitBandit_wait_event)
    USER_FUNC(btlevtcmd_CheckUnitFlag, -2, 0x4000'0000, LW(0))
    IF_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitBandit_pose_table))
    ELSE()
        USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitBandit_item_pose_table))
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitBandit_common_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitBandit_wait_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitBandit_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitBandit_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitBandit_attack_event))
    USER_FUNC(btlevtcmd_SetRunSound, -2, PTR("SFX_ENM_BORODO_MOVE1L"), PTR("SFX_ENM_BORODO_MOVE1R"), 0, 6, 6)
    USER_FUNC(btlevtcmd_SetWalkSound, -2, PTR("SFX_ENM_BORODO_MOVE1L"), PTR("SFX_ENM_BORODO_MOVE1R"), 0, 10, 10)
    USER_FUNC(btlevtcmd_CheckUnitFlag, -2, 0x4000'0000, LW(0))
    IF_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitBandit_pose_table))
    ELSE()
        USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitBandit_item_pose_table))
    END_IF()
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitBandit_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::BANDIT)
    RUN_CHILD_EVT(unitBandit_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitBigBandit_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::BIG_BANDIT)
    RUN_CHILD_EVT(unitBandit_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitBadgeBandit_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::BADGE_BANDIT)
    RUN_CHILD_EVT(unitBandit_common_init_event)
    RETURN()
EVT_END()

// BattleUnitKind, Setup structs.

DataTableEntry unitBandit_data_table[] = {
    51, (void*)unitBandit_revive_event,
    0, nullptr,
};

BattleUnitKind unit_Bandit = {
    .unit_type = BattleUnitType::BANDIT,
    .unit_name = "btl_un_borodo",
    .max_hp = 5,
    .max_fp = 0,
    .danger_hp = 1,
    .peril_hp = 1,
    .level = 1,
    .bonus_exp = 0,
    .bonus_coin = 0,
    .bonus_coin_rate = 100,
    .base_coin = 0,
    .run_rate = 50,
    .pb_soft_cap = 9999,
    .width = 26,
    .height = 34,
    .hit_offset = { 0, 34 },
    .center_offset = { 0.0f, 17.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 13.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 13.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 9.0f, 26.0f, 0.0f },
    .cut_base_offset = { 0.0f, 17.0f, 0.0f },
    .cut_width = 26.0f,
    .cut_height = 34.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_BORODO_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitBandit_status,
    .num_parts = 1,
    .parts = unitBandit_parts,
    .init_evt_code = (void*)unitBandit_init_event,
    .data_table = unitBandit_data_table,
};

BattleUnitKind unit_BigBandit = {
    .unit_type = BattleUnitType::BIG_BANDIT,
    .unit_name = "btl_un_borodo_king",
    .max_hp = 5,
    .max_fp = 0,
    .danger_hp = 1,
    .peril_hp = 1,
    .level = 1,
    .bonus_exp = 0,
    .bonus_coin = 0,
    .bonus_coin_rate = 100,
    .base_coin = 0,
    .run_rate = 50,
    .pb_soft_cap = 9999,
    .width = 26,
    .height = 34,
    .hit_offset = { 0, 34 },
    .center_offset = { 0.0f, 17.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 13.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 13.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 9.0f, 26.0f, 0.0f },
    .cut_base_offset = { 0.0f, 17.0f, 0.0f },
    .cut_width = 26.0f,
    .cut_height = 34.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_BORODO_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitBigBandit_status,
    .num_parts = 1,
    .parts = unitBigBandit_parts,
    .init_evt_code = (void*)unitBigBandit_init_event,
    .data_table = unitBandit_data_table,
};

BattleUnitKind unit_BadgeBandit = {
    .unit_type = BattleUnitType::BADGE_BANDIT,
    .unit_name = "btl_un_badge_borodo",
    .max_hp = 5,
    .max_fp = 0,
    .danger_hp = 1,
    .peril_hp = 1,
    .level = 1,
    .bonus_exp = 0,
    .bonus_coin = 0,
    .bonus_coin_rate = 100,
    .base_coin = 0,
    .run_rate = 50,
    .pb_soft_cap = 9999,
    .width = 26,
    .height = 34,
    .hit_offset = { 0, 34 },
    .center_offset = { 0.0f, 17.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 13.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 13.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 9.0f, 26.0f, 0.0f },
    .cut_base_offset = { 0.0f, 17.0f, 0.0f },
    .cut_width = 26.0f,
    .cut_height = 34.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_BORODO_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitBadgeBandit_status,
    .num_parts = 1,
    .parts = unitBadgeBandit_parts,
    .init_evt_code = (void*)unitBadgeBandit_init_event,
    .data_table = unitBandit_data_table,
};

// Function / USER_FUNC definitions.

namespace StealBooty {
    enum e {
        COINS = 0,
        ITEM,
        BADGE
    };
}
namespace StealTarget {
    enum e {
        ENEMY = 0,
        MARIO,
        PARTNER
    };
}

// Reworked to allow enemies to steal from each other (Confusion / Infatuate).
EVT_DEFINE_USER_FUNC(_steal_item) {
    int32_t self_id = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[0]));
    auto* self = ttyd::battle::BattleGetUnitPtr(g_BattleWork, self_id);
    
    int32_t target_id = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[1]));
    auto* target = ttyd::battle::BattleGetUnitPtr(g_BattleWork, target_id);
    
    evtSetValue(evt, evt->evtArguments[2], 0);
    evtSetValue(evt, evt->evtArguments[3], 0);
    evtSetValue(evt, evt->evtArguments[4], 0);
    
    if (!self || !target) return 2;

    auto* pouch = ttyd::mario_pouch::pouchGetPtr();

    int32_t steal_class = StealBooty::COINS;
    // 60% chance to steal an item or badge if able, otherwise steal coins.
    if (ttyd::system::irand(100) >= 40) {
        switch (self->current_kind) {
            case BattleUnitType::BIG_BANDIT:
                steal_class = StealBooty::ITEM;
                break;
            case BattleUnitType::BADGE_BANDIT:
                steal_class = StealBooty::BADGE;
                break;
        }
    }

    int32_t target_class = StealTarget::ENEMY;
    if (target->true_kind == BattleUnitType::MARIO) {
        target_class = StealTarget::MARIO;
    } else if (target->true_kind >= BattleUnitType::GOOMBELLA) {
        target_class = StealTarget::PARTNER;
    }

    if (steal_class == StealBooty::COINS) {
        // Can only steal coins from Mario or partner.
        if (target_class != StealTarget::ENEMY) {
            int32_t current_coins = pouch->coins;
            int32_t max_coins =
                self->current_kind == BattleUnitType::BANDIT ? 5 : 10;

            int32_t stolen_coins = Min(current_coins, max_coins);
            if (stolen_coins > 0) {
                evtSetValue(evt, evt->evtArguments[2], ItemType::COIN);
                evtSetValue(evt, evt->evtArguments[3], IconType::COIN);
                evtSetValue(evt, evt->evtArguments[4], stolen_coins);
                self->held_item = ItemType::COIN;
                self->stolen_coins = stolen_coins;
                ttyd::mario_pouch::pouchSetCoin(current_coins - stolen_coins);
            }
        }
    } else {
        int32_t item_type = 0;

        if (target_class != StealTarget::ENEMY) {
            // Select an item / badge to steal from the party's inventory.
            if (steal_class == StealBooty::ITEM) {
                int32_t item_count = 0;
                for (int32_t i = 0; i < 20; ++i) {
                    if (pouch->items[i]) ++item_count;
                }
                int32_t r = ttyd::system::irand(item_count);
                for (int32_t i = 0; i < 20; ++i) {
                    if (pouch->items[i] && --r < 0) {
                        item_type = pouch->items[i];
                        break;
                    }
                }
            } else {
                int32_t badge_count = 0;
                for (int32_t i = 0; i < 200; ++i) {
                    if (pouch->badges[i]) ++badge_count;
                }
                int32_t r = ttyd::system::irand(badge_count);
                for (int32_t i = 0; i < 200; ++i) {
                    if (pouch->badges[i] && --r < 0) {
                        item_type = pouch->badges[i];
                        break;
                    }
                }
            }
            
            if (item_type && ttyd::mario_pouch::pouchRemoveItem(item_type)) {
                self->held_item = item_type;
                if (item_type >= ItemType::POWER_JUMP) {
                    if (target_class == 1) {
                        ttyd::battle::BtlUnit_EquipItem(target, 3, 0);
                    } else {
                        ttyd::battle::BtlUnit_EquipItem(target, 5, 0);
                    }
                    ttyd::mario_pouch::pouchReviseMarioParam();
                    ttyd::mario_pouch::pouchRevisePartyParam();
                }
            }
        } else {
            // See if the enemy has a viable item to steal, and do so if so.
            if (target->held_item && !(
                    steal_class != StealBooty::BADGE && 
                    target->held_item >= ItemType::POWER_JUMP)) {
                
                item_type = target->held_item;
                target->held_item = 0;
                self->held_item = item_type;

                // Remove the target's held/stolen items from field info,
                // assuming they were present at the start of battle.
                if (!ttyd::battle_unit::BtlUnit_CheckUnitFlag(target, 0x4000'0000)) {
                    if (target->group_index >= 0) {
                        g_BattleWork->fbat_info->wBattleInfo->wHeldItems
                            [target->group_index] = 0;
                    }
                } else {
                    ttyd::battle_unit::BtlUnit_OffUnitFlag(target, 0x4000'0000);
                    if (target->group_index >= 0) {
                        auto* npc_battle_info = g_BattleWork->fbat_info->wBattleInfo;
                        npc_battle_info->wHeldItems[target->group_index] = 0;
                        npc_battle_info->wStolenItems[target->group_index] = 0;
                    }
                }

                if (item_type >= ItemType::POWER_JUMP) {
                    ttyd::battle::BtlUnit_EquipItem(target, 1, 0);
                }
            }
        }

        if (item_type) {
            evtSetValue(evt, evt->evtArguments[2], item_type);
            evtSetValue(evt, evt->evtArguments[3], 
                ttyd::item_data::itemDataTable[item_type].icon_id);
        }
    }
    
    return 2;
}

}  // namespace mod::tot::custom