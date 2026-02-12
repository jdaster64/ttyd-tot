#include "tot_custom_rel.h"     // For externed unit definitions.

#include "evt_cmd.h"

#include <gc/mtx.h>
#include <gc/types.h>
#include <ttyd/_core_language_libs.h>
#include <ttyd/battle.h>
#include <ttyd/battle_camera.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_event_default.h>
#include <ttyd/battle_event_subset.h>
#include <ttyd/battle_sub.h>
#include <ttyd/battle_unit.h>
#include <ttyd/battle_weapon_power.h>
#include <ttyd/evt_eff.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evt_sub.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
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
using namespace ::ttyd::battle_sub;
using namespace ::ttyd::battle_unit;
using namespace ::ttyd::evt_eff;
using namespace ::ttyd::evt_snd;
using namespace ::ttyd::evt_sub;

using ::ttyd::battle::g_BattleWork;
using ::ttyd::battle_unit::BattleWorkUnit;
using ::ttyd::battle_unit::BattleWorkUnitPart;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

// Constants (UW, etc.)
constexpr const int32_t UW_BattleUnitType = 0;
constexpr const int32_t UW_MoveState = 1;

}  // namespace

// Function / USER_FUNC declarations.

EVT_DECLARE_USER_FUNC(teresa_check_teresa, 2)
EVT_DECLARE_USER_FUNC(teresa_check_move, 1)
EVT_DECLARE_USER_FUNC(teresa_check_trans, 2)

// Unit data.

int8_t unitBoo_defense[] = { 0, 0, 0, 0, 0 };
int8_t unitBoo_defense_attr[] = { 0, 0, 0, 0, 0 };

StatusVulnerability unitBoo_status = {
    50,  50, 105,   0,  50, 100, 100,  50,
   100,  90, 100,  90, 100,  95,  60,  50,
    90, 100,  10, 100, 100,  10,
};
StatusVulnerability unitDarkBoo_status = {
    40,  40, 100,   0,  40, 100, 100,  40,
   100,  85, 100,  85, 100,  90,  50,  40,
    85, 100,   0, 100, 100,  10,
};

PoseTableEntry unitBoo_pose_table[] = {
    1, "N_1",
    2, "Y_1",
    9, "Y_1",
    5, "K_1",
    4, "X_1",
    3, "X_1",
    28, "S_1",
    29, "Q_1",
    30, "Q_1",
    31, "A_2",
    39, "D_1",
    40, "W_1",
    42, "R_1",
    56, "X_1",
    57, "X_1",
    65, "T_1",
    69, "S_1",
};

DataTableEntry unitBoo_data_table[] = {
    0, nullptr,
};

BattleWeapon unitBoo_weaponAttack = {
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
        AttackSpecialProperty_Flags::FREEZE_BREAK,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
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
BattleWeapon unitBoo_weaponMove = {
    .name = nullptr,
    .icon = 0,
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
    .damage_function = nullptr,
    .damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    
    .target_class_flags =
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_OPPOSING_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags = AttackSpecialProperty_Flags::CANNOT_MISS,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = 0,
        
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
BattleWeapon unitBoo_weaponInvisSelf = {
    .name = nullptr,
    .icon = 0,
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
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::CANNOT_MISS |
        AttackSpecialProperty_Flags::IGNORES_STATUS_CHANCE |
        AttackSpecialProperty_Flags::UNGUARDABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = 0,
        
    // status chances
    .invisible_chance = 100,
    .invisible_time = 2,
    
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
BattleWeapon unitBoo_weaponInvisAlly = {
    .name = nullptr,
    .icon = 0,
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
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::CANNOT_MISS |
        AttackSpecialProperty_Flags::IGNORES_STATUS_CHANCE |
        AttackSpecialProperty_Flags::UNGUARDABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = AttackTargetWeighting_Flags::WEIGHTED_RANDOM,
        
    // status chances
    .invisible_chance = 100,
    .invisible_time = 2,
    
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

BattleUnitKindPart unitBoo_parts[] = {
    {
        .index = 1,
        .name = "btl_un_teresa",
        .model_name = "c_teresa",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitBoo_defense,
        .defense_attr = unitBoo_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitBoo_pose_table,
    },
};
BattleUnitKindPart unitDarkBoo_parts[] = {
    {
        .index = 1,
        .name = "btl_un_purple_teresa",
        .model_name = "c_teresa_p",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitBoo_defense,
        .defense_attr = unitBoo_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitBoo_pose_table,
    },
};

// Evt definitions.

EVT_BEGIN(unitBoo_move_event_sub)
    USER_FUNC(btlevtcmd_GetUnitId, -2, LW(1))
    IF_EQUAL(LW(3), LW(1))
        SET(LW(0), 0)
        SET(LW(1), 0)
    ELSE()
        USER_FUNC(evt_sub_random, 20, LW(0))
        ADD(LW(0), 60)
        SET(LW(1), 12)
    END_IF()
    WAIT_FRM(LW(0))
    USER_FUNC(btlevtcmd_SetUnitWork, LW(3), UW_MoveState, 1)
    USER_FUNC(btlevtcmd_OnPartsAttribute, LW(3), 1, 0x60'0000)
    BROTHER_EVT()
        DO(60)
            USER_FUNC(btlevtcmd_AddRotate, LW(3), 0, LW(1), 0)
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()
    BROTHER_EVT()
        WAIT_FRM(30)
        USER_FUNC(btlevtcmd_GetUnitId, -2, LW(0))
        IF_EQUAL(LW(3), LW(0))
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("T_1"))
        END_IF()
    END_BROTHER()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TELESA_MOVE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetPartsPos, LW(3), 1, LW(0), LW(1), LW(2))
    SET(LW(1), 40)
    USER_FUNC(btlevtcmd_DivePosition, LW(3), LW(0), LW(1), LW(2), 60, 10, 4, 0, -1)
    USER_FUNC(btlevtcmd_SetHomePos, LW(3), LW(0), LW(1), LW(2))
    RETURN()
EVT_END()

EVT_BEGIN(unitBoo_move_event)
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), PTR(&unitBoo_weaponMove))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, PTR(&unitBoo_weaponMove), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            RETURN()
        END_IF()
        GOTO(99)
    END_IF()
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    WAIT_FRM(30)
    SET(LW(12), 80)
LBL(0)
    USER_FUNC(teresa_check_teresa, LW(3), LW(5))
    IF_EQUAL(LW(5), 1)
        USER_FUNC(btlevtcmd_GetUnitWork, LW(3), UW_MoveState, LW(6))
        IF_EQUAL(LW(6), 0)
            RUN_EVT(PTR(&unitBoo_move_event_sub))
            ADD(LW(12), 20)
        END_IF()
    END_IF()
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    IF_NOT_EQUAL(LW(3), -1)
        GOTO(0)
    END_IF()
    WAIT_FRM(LW(12))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_1"))
    RETURN()
EVT_END()

EVT_BEGIN(unitBoo_invis_self_event)
    SET(LW(3), -2)
    SET(LW(4), 1)
    USER_FUNC(btlevtcmd_CheckStatus, -2, 18, LW(8))
    USER_FUNC(btlevtcmd_WeaponAftereffect, PTR(&unitBoo_weaponInvisSelf))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, PTR(&unitBoo_weaponInvisSelf))
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, -1)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 300)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 30)
    SUB(LW(1), 10)
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 30, 10, 4, 0, -1)
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TELESA_MOVE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    IF_EQUAL(LW(8), 0)
        USER_FUNC(btlevtcmd_SetAlpha, -2, 1, 255)
        SET(LW(7), 256)
        BROTHER_EVT()
            DO(16)
                WAIT_FRM(1)
                SUB(LW(7), 8)
                USER_FUNC(btlevtcmd_SetAlpha, -2, 1, LW(7))
            WHILE()
        END_BROTHER()
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_1"))
    USER_FUNC(btlevtcmd_SetPartsRotate, -2, 1, 0, 180, 0)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 10, 0, 5, 0, -1)
    WAIT_FRM(10)
    DO(6)
        WAIT_FRM(1)
        USER_FUNC(btlevtcmd_AddPartsRotate, -2, 1, 0, 15, 0)
    WHILE()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_1"))
    DO(6)
        WAIT_FRM(1)
        USER_FUNC(btlevtcmd_AddPartsRotate, -2, 1, 0, 15, 0)
    WHILE()
    WAIT_FRM(12)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_1"))
    USER_FUNC(btlevtcmd_SetAlpha, -2, 1, 255)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), PTR(&unitBoo_weaponInvisSelf))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&unitBoo_weaponInvisSelf), 256, LW(5))
    RETURN()
EVT_END()

EVT_BEGIN(unitBoo_invis_ally_event)
LBL(0)
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), PTR(&unitBoo_weaponInvisAlly))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, PTR(&unitBoo_weaponInvisAlly), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(teresa_check_teresa, LW(3), LW(0))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(btlevtcmd_CheckStatus, LW(3), 18, LW(0))
        IF_EQUAL(LW(0), 0)
            GOTO(1)
        END_IF()
    END_IF()
    GOTO(0)
LBL(1)
    USER_FUNC(btlevtcmd_WeaponAftereffect, PTR(&unitBoo_weaponInvisAlly))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, PTR(&unitBoo_weaponInvisAlly))
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, -1)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 300)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(10), LW(11), LW(12))
    IF_SMALL_EQUAL(LW(0), LW(10))
        USER_FUNC(btlevtcmd_SetPartsRotate, -2, 1, 0, 0, 0)
    ELSE()
        USER_FUNC(btlevtcmd_SetPartsRotate, -2, 1, 0, 180, 0)
    END_IF()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TELESA_MOVE3"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetAlpha, -2, 1, 255)
    SET(LW(7), 256)
    BROTHER_EVT()
        DO(16)
            WAIT_FRM(1)
            SUB(LW(7), 16)
            USER_FUNC(btlevtcmd_SetAlpha, -2, 1, LW(7))
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, LW(3), LW(0), 40)
    SUB(LW(1), 10)
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 60, -5, 4, 0, -1)
    USER_FUNC(btlevtcmd_SetPartsRotate, -2, 1, 0, 0, 0)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TELESA_MOVE4"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetAlpha, -2, 1, 0)
    SET(LW(7), -1)
    DO(16)
        WAIT_FRM(1)
        ADD(LW(7), 16)
        USER_FUNC(btlevtcmd_SetAlpha, -2, 1, LW(7))
    WHILE()
    USER_FUNC(btlevtcmd_SetAlpha, -2, 1, 255)
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 20)
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 40, 0, 0, 0, -1)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), PTR(&unitBoo_weaponInvisSelf))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&unitBoo_weaponInvisSelf), 256, LW(5))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 30, -5, 4, 0, -1)
    USER_FUNC(btlevtcmd_SetPartsRotate, -2, 1, 0, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_1"))
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    WAIT_FRM(20)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitBoo_normal_attack_event)
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_GetFirstAttackTarget, LW(3), LW(4))
    ELSE()
        USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
        USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), PTR(&unitBoo_weaponAttack))
        USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, PTR(&unitBoo_weaponAttack), LW(3), LW(4))
    END_IF()
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            RETURN()
        END_IF()
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_WeaponAftereffect, PTR(&unitBoo_weaponAttack))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, PTR(&unitBoo_weaponAttack))
    USER_FUNC(evt_btl_camera_set_mode, 0, 8)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, LW(3))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TELESA_MOVE3"), EVT_NULLPTR, 0, EVT_NULLPTR)
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_SetAlpha, -2, 1, 255)
        SET(LW(7), 256)
        DO(16)
            WAIT_FRM(1)
            SUB(LW(7), 16)
            USER_FUNC(btlevtcmd_SetAlpha, -2, 1, LW(7))
        WHILE()
        WAIT_FRM(20)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TELESA_MOVE4"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_1"))
        USER_FUNC(btlevtcmd_SetAlpha, -2, 1, 0)
        SET(LW(7), -1)
        DO(8)
            WAIT_FRM(1)
            ADD(LW(7), 32)
            USER_FUNC(btlevtcmd_SetAlpha, -2, 1, LW(7))
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), 15)
    ADD(LW(2), 5)
    SUB(LW(1), 10)
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 60, -5, 4, 0, -1)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_SetAlpha, -2, 1, 255)
    WAIT_FRM(15)
    BROTHER_EVT()
        WAIT_FRM(9)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TELESA_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    END_BROTHER()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_2"))
    WAIT_FRM(15)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), PTR(&unitBoo_weaponAttack), 0, LW(5))
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
LBL(90)
            WAIT_FRM(30)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("X_1"))
            WAIT_FRM(60)
            GOTO(98)
LBL(91)
            USER_FUNC(evt_btl_camera_shake_h, 0, 1, 1, 8, 0)
            USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
            USER_FUNC(evt_btl_camera_add_zoom, 0, -50)
            USER_FUNC(btlevtcmd_ResultACDefence, LW(3), PTR(&unitBoo_weaponAttack))
            USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&unitBoo_weaponAttack), 256, LW(5))
            WAIT_FRM(30)
            GOTO(98)
LBL(98)
            USER_FUNC(evt_btl_camera_set_mode, 0, 0)
            USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
            USER_FUNC(btlevtcmd_AddPartsRotate, -2, 1, 0, 180, 0)
            USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 40, 0, 4, 0, -1)
            USER_FUNC(btlevtcmd_SetPartsRotate, -2, 1, 0, 0, 0)
            USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
            RETURN()
EVT_END()

EVT_BEGIN(unitBoo_attack_event)
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(PTR(&unitBoo_normal_attack_event))
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_EnemyItemUseCheck, -2, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(LW(0))
        USER_FUNC(btlevtcmd_StartWaitEvent, -2)
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_CheckStatus, -2, 18, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        SET(LW(13), 0)
    ELSE()
        SET(LW(13), 1)
    END_IF()
    USER_FUNC(btlevtcmd_CheckStatus, -2, 5, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        SET(LW(14), 0)
    ELSE()
        USER_FUNC(teresa_check_trans, -2, LW(14))
    END_IF()
    USER_FUNC(btlevtcmd_CheckStatus, -2, 5, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        SET(LW(15), 0)
    ELSE()
        USER_FUNC(teresa_check_move, LW(15))
    END_IF()
    SET(LW(0), 0)
    ADD(LW(0), 50)
    ADD(LW(0), 25)
    ADD(LW(0), 25)
    ADD(LW(0), 15)
    SUB(LW(0), 1)
    USER_FUNC(evt_sub_random, LW(0), LW(1))
    SET(LW(0), 25)
    IF_SMALL(LW(1), LW(0))
        IF_EQUAL(LW(13), 1)
            RUN_CHILD_EVT(PTR(&unitBoo_invis_self_event))
            GOTO(99)
        END_IF()
    END_IF()
    ADD(LW(0), 25)
    IF_SMALL(LW(1), LW(0))
        IF_EQUAL(LW(14), 1)
            RUN_CHILD_EVT(PTR(&unitBoo_invis_ally_event))
            GOTO(99)
        END_IF()
    END_IF()
    ADD(LW(0), 15)
    IF_SMALL(LW(1), LW(0))
        IF_EQUAL(LW(15), 1)
            RUN_CHILD_EVT(PTR(&unitBoo_move_event))
            GOTO(99)
        END_IF()
    END_IF()
    RUN_CHILD_EVT(PTR(&unitBoo_normal_attack_event))
    GOTO(99)
LBL(99)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitBoo_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    RETURN()
EVT_END()

EVT_BEGIN(unitBoo_wait_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitBoo_common_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitBoo_wait_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitBoo_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitBoo_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitBoo_attack_event))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_MoveState, 0)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    SET(LW(1), 15)
    USER_FUNC(btlevtcmd_SetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitBoo_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType,
        (int32_t)BattleUnitType::BOO)
    RUN_CHILD_EVT(unitBoo_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitDarkBoo_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType,
        (int32_t)BattleUnitType::DARK_BOO)
    RUN_CHILD_EVT(unitBoo_common_init_event)
    RETURN()
EVT_END()

// BattleUnitKind, Setup structs.

BattleUnitKind unit_Boo = {
    .unit_type = BattleUnitType::BOO,
    .unit_name = "btl_un_teresa",
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
    .width = 34,
    .height = 32,
    .hit_offset = { 0, 32 },
    .center_offset = { 0.0f, 16.0f, 0.0f },
    .hp_gauge_offset = { 0, -3 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 17.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 17.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 17.0f, 20.8f, 0.0f },
    .cut_base_offset = { 0.0f, 16.0f, 0.0f },
    .cut_width = 34.0f,
    .cut_height = 32.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_TELESA_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::UNQUAKEABLE,
    .status_vulnerability = &unitBoo_status,
    .num_parts = 1,
    .parts = unitBoo_parts,
    .init_evt_code = (void*)unitBoo_init_event,
    .data_table = unitBoo_data_table,
};
BattleUnitKind unit_DarkBoo = {
    .unit_type = BattleUnitType::DARK_BOO,
    .unit_name = "btl_un_purple_teresa",
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
    .width = 34,
    .height = 32,
    .hit_offset = { 0, 32 },
    .center_offset = { 0.0f, 16.0f, 0.0f },
    .hp_gauge_offset = { 0, -3 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 17.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 17.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 17.0f, 20.8f, 0.0f },
    .cut_base_offset = { 0.0f, 16.0f, 0.0f },
    .cut_width = 34.0f,
    .cut_height = 32.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_TELESA_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::UNQUAKEABLE,
    .status_vulnerability = &unitDarkBoo_status,
    .num_parts = 1,
    .parts = unitDarkBoo_parts,
    .init_evt_code = (void*)unitDarkBoo_init_event,
    .data_table = unitBoo_data_table,
};

// Function / USER_FUNC definitions.

EVT_DEFINE_USER_FUNC(teresa_check_teresa) {
    int32_t unit_id = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[0]));
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, unit_id);
    
    bool is_boo = (unit != nullptr && (
        unit->current_kind == BattleUnitType::BOO ||
        unit->current_kind == BattleUnitType::DARK_BOO));
    evtSetValue(evt, evt->evtArguments[1], is_boo);
    
    return 2;
}

// Checks whether there are any valid targets for a Boo to move upward.
EVT_DEFINE_USER_FUNC(teresa_check_move) {
    int32_t self_id = ttyd::battle_sub::BattleTransID(evt, -2);
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, self_id);
    int32_t alliance = unit->alliance;
    
    int32_t num_targets = 0;
    for (int32_t i = 0; i < 64; ++i) {
        auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, i);
        
        // Valid targets must be Boos on my alliance that haven't already moved.
        if (!unit || unit->alliance != alliance ||
            !(unit->current_kind == BattleUnitType::BOO ||
              unit->current_kind == BattleUnitType::DARK_BOO) ||
            unit->unit_work[UW_MoveState] != 0)
            continue;
            
        ++num_targets;
    }
    evtSetValue(evt, evt->evtArguments[0], num_targets > 0);
    return 2;
}

// Checks whether there are any valid other targets for a Boo to turn invisible.
EVT_DEFINE_USER_FUNC(teresa_check_trans) {
    int32_t self_id = ttyd::battle_sub::BattleTransID(evt, -2);
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, self_id);

    // Cannot target anything else if Infatuated, return early.
    if (unit->alliance == 0) {
        evtSetValue(evt, evt->evtArguments[1], 0);
        return 2;
    }
    
    int32_t num_targets = 0;
    for (int32_t i = 0; i < 64; ++i) {
        auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, i);
        
        // Valid targets must be other enemy Boos that aren't already invisible.
        if (!unit || i == self_id || unit->alliance != 1 ||
            !(unit->current_kind == BattleUnitType::BOO ||
              unit->current_kind == BattleUnitType::DARK_BOO) ||
            ttyd::battle_unit::BtlUnit_CheckStatus(unit, StatusEffectType::INVISIBLE))
            continue;
            
        ++num_targets;
    }
    evtSetValue(evt, evt->evtArguments[1], num_targets > 0);
    return 2;
}

}  // namespace mod::tot::custom