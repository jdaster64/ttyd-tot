#include "tot_custom_rel.h"     // For externed unit definitions.

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
constexpr const int32_t UW_FireballsShot = 0;
constexpr const int32_t UW_FireballsFinished = 1;
constexpr const int32_t UW_TargetUnits = 2;
constexpr const int32_t UW_TargetParts = 8;

constexpr const int32_t PW_TargetsHit = 2;
constexpr const int32_t PW_FireballDone = 15;

}  // namespace

// Evt declarations.

extern const int32_t unitFireBro_init_event[];
extern const int32_t unitFireBro_attack_event[];
extern const int32_t unitFireBro_damage_event[];
extern const int32_t unitFireBro_wait_event[];
extern const int32_t unitFireBro_normal_attack_event[];
extern const int32_t unitFireBro_repeat_attack_event[];
extern const int32_t unitFireBro_barrage_attack_event[];
extern const int32_t unitFireBro_fireball_move_event[];
extern const int32_t unitFireBro_damage_check_event[];
extern const int32_t unitFireBro_damage_check_last_event[];
extern const int32_t unitFireBro_damage_check_nolast_event[];
extern const int32_t unitFireBro_barrage_judge_last_event[];

// Unit data.

int8_t unitFireBro_defense[] = { 1, 1, 1, 1, 1 };
int8_t unitFireBro_defense_attr[] = { 0, 0, 0, 0, 0 };

StatusVulnerability unitFireBro_status = {
     80,  80,  70, 100,  80, 100, 100,  70,
    100,  80, 100,  80, 100,  95,  80,  70,
     60, 100,  80, 100, 100,  95,
};

PoseTableEntry unitFireBro_pose_table[] = {
    1, "BRO_N_1",
    2, "BRO_Y_1",
    9, "BRO_Y_1",
    5, "BRO_K_1",
    4, "BRO_X_1",
    3, "BRO_X_1",
    27, "BRO_D_1",
    28, "BRO_S_1",
    29, "BRO_Q_1",
    30, "BRO_Q_1",
    31, "BRO_S_1",
    39, "BRO_D_1",
    42, "BRO_R_1",
    40, "BRO_W_1",
    56, "BRO_X_1",
    57, "BRO_X_1",
    65, "BRO_T_1",
    69, "BRO_S_1",
};

DataTableEntry unitFireBro_data_table[] = {
    0, nullptr,
};

BattleWeapon unitFireBro_weaponNormal = {
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
    .damage_function_params = { 3, 0, 0, 0, 0, 0, 0, 0 },
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
    .element = AttackElement::FIRE,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags = AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM,
        
    // status chances
    .burn_chance = 30,
    .burn_time = 3,
    
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
BattleWeapon unitFireBro_weaponRepeat = {
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
    .damage_function_params = { 2, 0, 0, 0, 0, 0, 0, 0 },
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
    .element = AttackElement::FIRE,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags = AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM,
        
    // status chances
    .burn_chance = 30,
    .burn_time = 3,
    
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
BattleWeapon unitFireBro_weaponBarrage = {
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
    .ac_help_msg = nullptr,
    .special_property_flags = AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM,
        
    // status chances
    .burn_chance = 30,
    .burn_time = 3,
    
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

BattleUnitKindPart unitFireBro_parts[] = {
    {
        .index = 1,
        .name = "btl_un_fire_bros",
        .model_name = "c_burosu_f",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 35.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 45.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitFireBro_defense,
        .defense_attr = unitFireBro_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitFireBro_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_fire_bros",
        .model_name = "c_burosu_f",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitFireBro_defense,
        .defense_attr = unitFireBro_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitFireBro_pose_table,
    },
    {
        .index = 3,
        .name = "btl_un_fire_bros",
        .model_name = "c_burosu_f",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitFireBro_defense,
        .defense_attr = unitFireBro_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitFireBro_pose_table,
    },
    {
        .index = 4,
        .name = "btl_un_fire_bros",
        .model_name = "c_burosu_f",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitFireBro_defense,
        .defense_attr = unitFireBro_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitFireBro_pose_table,
    },
    {
        .index = 5,
        .name = "btl_un_fire_bros",
        .model_name = "c_burosu_f",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitFireBro_defense,
        .defense_attr = unitFireBro_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitFireBro_pose_table,
    },
    {
        .index = 6,
        .name = "btl_un_fire_bros",
        .model_name = "c_burosu_f",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitFireBro_defense,
        .defense_attr = unitFireBro_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitFireBro_pose_table,
    },
    {
        .index = 7,
        .name = "btl_un_fire_bros",
        .model_name = "c_burosu_f",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitFireBro_defense,
        .defense_attr = unitFireBro_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitFireBro_pose_table,
    },
};

BattleUnitKind unit_FireBro = {
    .unit_type = BattleUnitType::FIRE_BRO,
    .unit_name = "btl_un_fire_bros",
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
    .width = 36,
    .height = 44,
    .hit_offset = { 3, 44 },
    .center_offset = { 0.0f, 22.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 18.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 15.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 10.0f, 28.0f, 0.0f },
    .cut_base_offset = { 0.0f, 22.0f, 0.0f },
    .cut_width = 36.0f,
    .cut_height = 44.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_FIREB_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitFireBro_status,
    .num_parts = 7,
    .parts = unitFireBro_parts,
    .init_evt_code = (void*)unitFireBro_init_event,
    .data_table = unitFireBro_data_table,
};

// Evt definitions.

EVT_BEGIN(unitFireBro_attack_event)
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(PTR(&unitFireBro_normal_attack_event))
        GOTO(99)
    END_IF()

    USER_FUNC(btlevtcmd_EnemyItemUseCheck, -2, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(LW(0))
        USER_FUNC(btlevtcmd_StartWaitEvent, -2)
        RETURN()
    END_IF()

    USER_FUNC(evtTot_GetPercentOfMaxHP, -2, LW(0))
    IF_SMALL_EQUAL(LW(0), 50)
        USER_FUNC(evt_sub_random, 99, LW(1))
        IF_SMALL(LW(1), 30)
            RUN_CHILD_EVT(PTR(&unitFireBro_repeat_attack_event))
        ELSE()
            RUN_CHILD_EVT(PTR(&unitFireBro_barrage_attack_event))
        END_IF()
    ELSE()
        USER_FUNC(evt_sub_random, 99, LW(1))
        IF_SMALL(LW(1), 70)
            RUN_CHILD_EVT(PTR(&unitFireBro_normal_attack_event))
        ELSE()
            RUN_CHILD_EVT(PTR(&unitFireBro_repeat_attack_event))
        END_IF()
    END_IF()
LBL(99)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitFireBro_damage_check_last_event)
    IF_EQUAL(LW(6), PTR(&unitFireBro_weaponRepeat))
        USER_FUNC(btlevtcmd_GetPartsWork, -2, 1, PW_TargetsHit, LW(0))
        IF_EQUAL(LW(0), 1)
            SET(LW(0), 1)
        ELSE()
            SET(LW(0), 0)
        END_IF()
        GOTO(10)
    END_IF()
    IF_EQUAL(LW(8), 1)
        SET(LW(13), UW_TargetUnits)
        SET(LW(14), UW_TargetParts)
        DO(0)
            USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(13), LW(1))
            USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(14), LW(2))
            IF_EQUAL(LW(3), LW(1))
                IF_EQUAL(LW(4), LW(2))
                    // Corresponding hit result var.
                    USER_FUNC(btlevtcmd_GetPartsWork, -2, 1, LW(13), LW(0))
                    DO_BREAK()
                END_IF()
            END_IF()
            ADD(LW(13), 1)
            ADD(LW(14), 1)
        WHILE()
    END_IF()
LBL(10)
    IF_EQUAL(LW(0), 1)
        USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(6))
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(6), 256, LW(5))
    ELSE()
        SET(LW(7), 1)
        RUN_CHILD_EVT(PTR(&unitFireBro_damage_check_event))
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitFireBro_damage_check_nolast_event)
    SET(LW(7), 0)
    RUN_CHILD_EVT(PTR(&unitFireBro_damage_check_event))
    RETURN()
EVT_END()

EVT_BEGIN(unitFireBro_damage_check_event)
    SET(LW(0), 0)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), LW(6), 0, LW(5))
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
    GOTO(99)
LBL(91)
    SET(LW(0), 1)
    IF_EQUAL(LW(6), PTR(&unitFireBro_weaponRepeat))
        USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, PW_TargetsHit, 1)
    END_IF()
    IF_EQUAL(LW(8), 1)
        SET(LW(13), UW_TargetUnits)
        SET(LW(14), UW_TargetParts)
        DO(0)
            USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(13), LW(1))
            USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(14), LW(2))
            IF_EQUAL(LW(3), LW(1))
                IF_EQUAL(LW(4), LW(2))
                    // Corresponding hit result var.
                    USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, LW(13), 1)
                    DO_BREAK()
                END_IF()
            END_IF()
            ADD(LW(13), 1)
            ADD(LW(14), 1)
        WHILE()
    END_IF()
LBL(98)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(6))
    IF_EQUAL(LW(7), 1)
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(6), 256, LW(5))
    ELSE()
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(6), 0, LW(5))
    END_IF()
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitFireBro_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    RETURN()
EVT_END()

EVT_BEGIN(unitFireBro_fireball_move_event)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_FIREB_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, LW(15), PTR("BRO_A_1B"))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, LW(15), PW_FireballDone, 0)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    ADD(LW(1), 15)
    USER_FUNC(btlevtcmd_SetPartsPos, -2, LW(15), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetPartsRotate, -2, LW(15), 0, 0, -180)
    USER_FUNC(btlevtcmd_SetPartsRotateOffset, -2, LW(15), 0, 0, 0)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, LW(15), 0x100'0000)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, LW(15), 0x200'0000)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(9), LW(10), LW(11))
    SUB(LW(9), LW(0))
    SET(LW(10), 0)
    BROTHER_EVT()
        DO(0)
            WAIT_FRM(1)
            USER_FUNC(btlevtcmd_GetPartsPos, -2, LW(15), LW(0), LW(12), LW(13))
            SWITCH(LW(10))
                CASE_EQUAL(0)
                    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(11), LW(12), LW(13))
                    IF_LARGE_EQUAL(LW(9), 0)
                        IF_SMALL_EQUAL(LW(0), LW(11))
                            SWITCH_BREAK()
                        END_IF()
                    ELSE()
                        IF_LARGE_EQUAL(LW(0), LW(11))
                            SWITCH_BREAK()
                        END_IF()
                    END_IF()
                    IF_EQUAL(LW(14), 0)
                        RUN_CHILD_EVT(PTR(&unitFireBro_damage_check_nolast_event))
                    ELSE()
                        RUN_CHILD_EVT(PTR(&unitFireBro_damage_check_last_event))
                    END_IF()
                    IF_EQUAL(LW(0), 1)
                        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, LW(15), 0x100'0000)
                        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, LW(15), 0x200'0000)
                        SET(LW(10), 2)
                    ELSE()
                        SET(LW(10), 1)
                    END_IF()
                    CASE_END()
                CASE_EQUAL(1)
                    USER_FUNC(btlevtcmd_GetPartsPos, -2, LW(15), LW(11), LW(12), LW(13))
                    IF_LARGE_EQUAL(LW(11), 250)
                        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, LW(15), 0x100'0000)
                        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, LW(15), 0x200'0000)
                        SET(LW(10), 2)
                    END_IF()
                    IF_SMALL_EQUAL(LW(11), -250)
                        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, LW(15), 0x100'0000)
                        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, LW(15), 0x200'0000)
                        SET(LW(10), 2)
                    END_IF()
                    CASE_END()
                CASE_EQUAL(2)
                    USER_FUNC(btlevtcmd_SetPartsWork, -2, LW(15), PW_FireballDone, 1)
                    DO_BREAK()
                    CASE_END()
            END_SWITCH()
        WHILE()
    END_BROTHER()
    BROTHER_EVT()
        DO(0)
            WAIT_FRM(1)
            USER_FUNC(btlevtcmd_AddPartsRotate, -2, LW(15), 0, 0, 16)
        WHILE()
    END_BROTHER()
    USER_FUNC(evt_sub_random, 1, LW(11))
    IF_EQUAL(LW(11), 0)
        USER_FUNC(btlevtcmd_GetPartsPos, -2, LW(15), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_JumpPartsSetting, -2, LW(15), 0, 6, FLOAT(0.6))
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 100)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(15), LW(0), 0, LW(2), 0, -1)
        USER_FUNC(btlevtcmd_GetPartsWork, -2, LW(15), PW_FireballDone, LW(10))
        IF_EQUAL(LW(10), 1)
            GOTO(99)
        END_IF()
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_FIREB_ATTACK2"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 100)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(15), LW(0), 0, LW(2), 0, -1)
        USER_FUNC(btlevtcmd_GetPartsWork, -2, LW(15), PW_FireballDone, LW(10))
        IF_EQUAL(LW(10), 1)
            GOTO(99)
        END_IF()
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_FIREB_ATTACK2"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 100)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(15), LW(0), 0, LW(2), 0, -1)
        USER_FUNC(btlevtcmd_GetPartsWork, -2, LW(15), PW_FireballDone, LW(10))
        IF_EQUAL(LW(10), 1)
            GOTO(99)
        END_IF()
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_FIREB_ATTACK2"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 100)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(15), LW(0), 0, LW(2), 0, -1)
        USER_FUNC(btlevtcmd_GetPartsWork, -2, LW(15), PW_FireballDone, LW(10))
        IF_EQUAL(LW(10), 1)
            GOTO(99)
        END_IF()
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_FIREB_ATTACK2"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 100)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(15), LW(0), 0, LW(2), 0, -1)
        USER_FUNC(btlevtcmd_GetPartsWork, -2, LW(15), PW_FireballDone, LW(10))
        IF_EQUAL(LW(10), 1)
            GOTO(99)
        END_IF()
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_FIREB_ATTACK2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    ELSE()
        USER_FUNC(btlevtcmd_GetPartsPos, -2, LW(15), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_JumpPartsSetting, -2, LW(15), 0, 5, FLOAT(0.6))
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 100)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(15), LW(0), 0, LW(2), 0, -1)
        USER_FUNC(btlevtcmd_GetPartsWork, -2, LW(15), PW_FireballDone, LW(10))
        IF_EQUAL(LW(10), 1)
            GOTO(99)
        END_IF()
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_FIREB_ATTACK2"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 100)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(15), LW(0), 0, LW(2), 0, -1)
        USER_FUNC(btlevtcmd_GetPartsWork, -2, LW(15), PW_FireballDone, LW(10))
        IF_EQUAL(LW(10), 1)
            GOTO(99)
        END_IF()
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_FIREB_ATTACK2"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 100)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(15), LW(0), 0, LW(2), 0, -1)
        USER_FUNC(btlevtcmd_GetPartsWork, -2, LW(15), PW_FireballDone, LW(10))
        IF_EQUAL(LW(10), 1)
            GOTO(99)
        END_IF()
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_FIREB_ATTACK2"), EVT_NULLPTR, 0, EVT_NULLPTR)
        IF_EQUAL(LW(10), 2)
            GOTO(99)
        END_IF()
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 100)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(15), LW(0), 0, LW(2), 0, -1)
        USER_FUNC(btlevtcmd_GetPartsWork, -2, LW(15), PW_FireballDone, LW(10))
        IF_EQUAL(LW(10), 1)
            GOTO(99)
        END_IF()
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_FIREB_ATTACK2"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 100)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(15), LW(0), 0, LW(2), 0, -1)
        USER_FUNC(btlevtcmd_GetPartsWork, -2, LW(15), PW_FireballDone, LW(10))
        IF_EQUAL(LW(10), 1)
            GOTO(99)
        END_IF()
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_FIREB_ATTACK2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    END_IF()
LBL(99)
    WAIT_FRM(6)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, LW(15), 0x100'0000)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, LW(15), 0x200'0000)
    USER_FUNC(btlevtcmd_AddUnitWork, -2, UW_FireballsFinished, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitFireBro_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitFireBro_wait_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitFireBro_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitFireBro_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitFireBro_attack_event))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitFireBro_barrage_judge_last_event)
    SET(LW(0), LW(10))
    SET(LW(1), LW(11))
    DO(0)
        IF_LARGE_EQUAL(LW(0), UW_TargetUnits + 5)
            SET(LW(14), 1)
            DO_BREAK()
        END_IF()
        ADD(LW(0), 1)
        ADD(LW(1), 1)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(0), LW(5))
        USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(1), LW(6))
        IF_EQUAL(LW(3), LW(5))
            IF_EQUAL(LW(4), LW(6))
                SET(LW(14), 0)
                DO_BREAK()
            END_IF()
        END_IF()
    WHILE()
    RETURN()
EVT_END()

EVT_BEGIN(unitFireBro_barrage_attack_event)
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), PTR(&unitFireBro_weaponBarrage))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, PTR(&unitFireBro_weaponBarrage), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            RETURN()
        END_IF()
        GOTO(99)
    END_IF()
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_WeaponAftereffect, PTR(&unitFireBro_weaponBarrage))
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, PTR(&unitFireBro_weaponBarrage))
    
    // Reworked code relating to target selection to properly handle 'last' hits.
    USER_FUNC(evtTot_SelectMultihitTargetsX6, LW(0), LW(1), LW(2), LW(3), LW(4), LW(5), LW(6), LW(7), LW(8), LW(9), LW(10), LW(11))

    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_TargetUnits, LW(0))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_TargetUnits + 1, LW(2))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_TargetUnits + 2, LW(4))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_TargetUnits + 3, LW(6))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_TargetUnits + 4, LW(8))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_TargetUnits + 5, LW(10))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_TargetParts, LW(1))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_TargetParts + 1, LW(3))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_TargetParts + 2, LW(5))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_TargetParts + 3, LW(7))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_TargetParts + 4, LW(9))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_TargetParts + 5, LW(11))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, PW_TargetsHit, 0)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, PW_TargetsHit + 1, 0)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, PW_TargetsHit + 2, 0)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, PW_TargetsHit + 3, 0)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, PW_TargetsHit + 4, 0)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, PW_TargetsHit + 5, 0)

    WAIT_FRM(12)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_FireballsShot, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_FireballsFinished, 0)
    
    SET(LW(15), 2)
    SET(LW(10), UW_TargetUnits)
    SET(LW(11), UW_TargetParts)
    DO(0)
        // Wait for the previous shot to land before shooting the next.
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_FireballsShot, LW(1))
        IF_LARGE_EQUAL(LW(1), 6)
            DO(0)
                WAIT_FRM(1)
                USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_FireballsShot, LW(1))
                USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_FireballsFinished, LW(2))
                SUB(LW(1), 6)
                IF_LARGE(LW(2), LW(1))
                    WAIT_FRM(3)
                    DO_BREAK()
                END_IF()
            WHILE()
        END_IF()

        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("BRO_A_2A"))
        WAIT_FRM(20)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(10), LW(3))
        USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(11), LW(4))
        USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(14))
        USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(14))
        RUN_CHILD_EVT(PTR(&unitFireBro_barrage_judge_last_event))
        SET(LW(6), PTR(&unitFireBro_weaponBarrage))
        SET(LW(8), 1)
        RUN_EVT(PTR(&unitFireBro_fireball_move_event))
        USER_FUNC(btlevtcmd_AddUnitWork, -2, UW_FireballsShot, 1)
        WAIT_FRM(15)
        ADD(LW(15), 1)
        ADD(LW(10), 1)
        ADD(LW(11), 1)
        IF_LARGE(LW(15), 7)
            DO_BREAK()
        END_IF()
    WHILE()
    
    DO(0)
        WAIT_FRM(1)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_FireballsFinished, LW(0))
        IF_LARGE_EQUAL(LW(0), 6)
            DO_BREAK()
        END_IF()
    WHILE()

    WAIT_FRM(30)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitFireBro_normal_attack_event)
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_GetFirstAttackTarget, LW(3), LW(4))
    ELSE()
        USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
        USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), PTR(&unitFireBro_weaponNormal))
        USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, PTR(&unitFireBro_weaponNormal), LW(3), LW(4))
    END_IF()

    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            RETURN()
        END_IF()
        GOTO(99)
    END_IF()

    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_WeaponAftereffect, PTR(&unitFireBro_weaponNormal))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, PTR(&unitFireBro_weaponNormal))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_FireballsFinished, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("BRO_A_1A"))
    WAIT_FRM(40)
    SET(LW(15), 2)
    SET(LW(14), 1)
    SET(LW(6), PTR(&unitFireBro_weaponNormal))
    SET(LW(8), 0)
    RUN_EVT(PTR(&unitFireBro_fireball_move_event))
    DO(0)
        WAIT_FRM(1)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_FireballsFinished, LW(0))
        IF_LARGE_EQUAL(LW(0), 1)
            DO_BREAK()
        END_IF()
    WHILE()
    WAIT_FRM(20)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitFireBro_repeat_attack_event)
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), PTR(&unitFireBro_weaponRepeat))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, PTR(&unitFireBro_weaponRepeat), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            RETURN()
        END_IF()
        GOTO(99)
    END_IF()
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_WeaponAftereffect, PTR(&unitFireBro_weaponRepeat))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, PTR(&unitFireBro_weaponRepeat))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_FireballsFinished, 0)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, PW_TargetsHit, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("BRO_A_2A"))
    WAIT_FRM(20)
    SET(LW(15), 2)
    SET(LW(14), 0)
    SET(LW(6), PTR(&unitFireBro_weaponRepeat))
    SET(LW(8), 0)
    RUN_EVT(PTR(&unitFireBro_fireball_move_event))
    WAIT_FRM(12)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("BRO_A_2A"))
    WAIT_FRM(20)
    SET(LW(15), 3)
    SET(LW(14), 1)
    SET(LW(6), PTR(&unitFireBro_weaponRepeat))
    SET(LW(8), 0)
    RUN_EVT(PTR(&unitFireBro_fireball_move_event))
    DO(0)
        WAIT_FRM(1)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_FireballsFinished, LW(0))
        IF_LARGE_EQUAL(LW(0), 2)
            DO_BREAK()
        END_IF()
    WHILE()
    WAIT_FRM(20)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitFireBro_wait_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    RETURN()
EVT_END()

}  // namespace mod::tot::custom