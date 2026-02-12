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
#include <ttyd/battle_sub.h>
#include <ttyd/battle_unit.h>
#include <ttyd/battle_weapon_power.h>
#include <ttyd/camdrv.h>
#include <ttyd/dispdrv.h>
#include <ttyd/evt_eff.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evt_sub.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/gx/GXAttr.h>
#include <ttyd/gx/GXGeometry.h>
#include <ttyd/gx/GXLight.h>
#include <ttyd/gx/GXTev.h>
#include <ttyd/gx/GXTransform.h>
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
using namespace ::ttyd::gx;

using ::ttyd::battle::g_BattleWork;
using ::ttyd::battle_unit::BattleWorkUnit;
using ::ttyd::battle_unit::BattleWorkUnitPart;
using ::ttyd::dispdrv::CameraId;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

// Constants (UW, etc.)
constexpr const int32_t UW_HiLoPosition = 0;
constexpr const int32_t UW_YarnAlpha = 1;
constexpr const int32_t UW_YarnHangPoint = 2;
constexpr const int32_t UW_NeverMove = 3;
constexpr const int32_t UW_RotationPoint = 4;
constexpr const int32_t UW_BarrageTargetUnit1 = 5;
constexpr const int32_t UW_BarrageTargetUnit2 = 6;
constexpr const int32_t UW_BarrageTargetUnit3 = 7;
constexpr const int32_t UW_BarrageTargetPart1 = 8;
constexpr const int32_t UW_BarrageTargetPart2 = 9;
constexpr const int32_t UW_BarrageTargetPart3 = 10;
constexpr const int32_t UW_BarrageHit1 = 11;
constexpr const int32_t UW_BarrageHit2 = 12;
constexpr const int32_t UW_BarrageHit3 = 13;
// new variables
constexpr const int32_t UW_BarrageType = 14;
constexpr const int32_t UW_BattleUnitType = 15;

}  // namespace

// Evt / Function declarations.

extern const int32_t unitPider_init_event[];
extern const int32_t unitArantula_init_event[];
extern const int32_t unitPider_common_init_event[];
extern const int32_t unitPider_attack_event[];
extern const int32_t unitPider_damage_event[];
extern const int32_t unitPider_wait_event[];
extern const int32_t unitPider_normal_attack_event[];
extern const int32_t unitPider_attack_common_event1[];
extern const int32_t unitPider_attack_common_event2[];
extern const int32_t unitPider_attack_common_event3[];
extern const int32_t unitPider_barrage_event[];
extern const int32_t unitPider_crazy_barrage_event[];
extern const int32_t unitPider_barrage_event_sub[];
extern const int32_t unitPider_barrage_event_sub_nolast[];
extern const int32_t unitPider_barrage_event_sub_last[];
extern const int32_t unitPider_unison_phase_event[];
extern const int32_t unitPider_entry_event[];
extern const int32_t unitPider_battle_end_event[];
extern const int32_t unitPider_dead_event[];
extern const int32_t unitPider_gale_dead_event[];
extern const int32_t unitPider_damage_check_sub_event[];
extern const int32_t unitPider_first_attack_pos_event[];
extern const int32_t unitPider_move_up_event[];
extern const int32_t unitPider_move_down_event[];
extern const int32_t unitPider_move_spin_event[];
extern const int32_t unitPider_yarn_event[];

EVT_DECLARE_USER_FUNC(unitPider_CheckLeaderOrder, 2)
EVT_DECLARE_USER_FUNC(unitPider_GetLeaderHiLoPosition, 1)
EVT_DECLARE_USER_FUNC(unitPider_GetPiderCount, 1)
EVT_DECLARE_USER_FUNC(unitPider_YarnDrawMain, 1)
EVT_DECLARE_USER_FUNC(unitPider_YarnInit, 0)

// Unit data.

int8_t unitPider_defense[] = { 0, 0, 0, 0, 0 };
int8_t unitPider_defense_attr[] = { 0, 0, 0, 0, 0 };

StatusVulnerability unitPider_status = {
     90,  90,  50, 100,  90, 100, 100,  90,
    100,  90, 100,  90, 100,  95,  90, 100,
     70, 100,  90, 100, 100,  95,
};
StatusVulnerability unitArantula_status = {
     70,  70,  30, 100,  70, 100, 100,  70,
    100,  80, 100,  80, 100,  85,  70,  80,
     30, 100,  70, 100, 100,  30,
};

PoseTableEntry unitPider_pose_table[] = {
    1, "PAI_N_1",
    2, "PAI_Y_1",
    9, "PAI_Y_1",
    5, "PAI_K_1",
    4, "PAI_X_1",
    3, "PAI_X_1",
    28, "PAI_S_1",
    29, "PAI_D_1",
    30, "PAI_D_1",
    31, "PAI_S_1",
    39, "PAI_D_1",
    50, "PAI_A_1",
    40, "PAI_W_1",
    42, "PAI_W_1",
    56, "PAI_X_1",
    57, "PAI_X_1",
    65, "PAI_S_1",
    69, "PAI_S_1",
};

DataTableEntry unitPider_data_table[] = {
    49, (void*)unitPider_dead_event,
    58, (void*)unitPider_gale_dead_event,
    63, (void*)unitPider_battle_end_event,
    48, (void*)unitPider_first_attack_pos_event,
    0, nullptr,
};

BattleWeapon unitPider_weaponSingle = {
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
BattleWeapon unitPider_weaponBarrage = {
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
BattleWeapon unitArantula_weaponSingle = {
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
    .damage_function_params = { 7, 0, 0, 0, 0, 0, 0, 0 },
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
BattleWeapon unitArantula_weaponBarrage = {
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
    .damage_function_params = { 4, 0, 0, 0, 0, 0, 0, 0 },
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

BattleUnitKindPart unitPider_parts[] = {
    {
        .index = 1,
        .name = "btl_un_piders",
        .model_name = "c_paid",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 40.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 50.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitPider_defense,
        .defense_attr = unitPider_defense_attr,
        .attribute_flags = 0x0000'0009 |
            PartsAttribute_Flags::HAMMERLIKE_CANNOT_TARGET |
            PartsAttribute_Flags::SHELLTOSSLIKE_CANNOT_TARGET,
        .counter_attribute_flags = 0,
        .pose_table = unitPider_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_piders",
        .model_name = "c_paid",
        .part_offset_pos = { 0.0f, 0.0f, 5.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitPider_defense,
        .defense_attr = unitPider_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitPider_pose_table,
    },
    {
        .index = 3,
        .name = "btl_un_piders",
        .model_name = "c_paid",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitPider_defense,
        .defense_attr = unitPider_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitPider_pose_table,
    },
    {
        .index = 4,
        .name = "btl_un_piders",
        .model_name = "c_paid",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitPider_defense,
        .defense_attr = unitPider_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitPider_pose_table,
    },
    {
        .index = 5,
        .name = "btl_un_piders",
        .model_name = "c_paid",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitPider_defense,
        .defense_attr = unitPider_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitPider_pose_table,
    },
};
BattleUnitKindPart unitArantula_parts[] = {
    {
        .index = 1,
        .name = "btl_un_churantalar",
        .model_name = "c_paid_b",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 40.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 50.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitPider_defense,
        .defense_attr = unitPider_defense_attr,
        .attribute_flags = 0x0000'0009 |
            PartsAttribute_Flags::HAMMERLIKE_CANNOT_TARGET |
            PartsAttribute_Flags::SHELLTOSSLIKE_CANNOT_TARGET,
        .counter_attribute_flags = 0,
        .pose_table = unitPider_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_churantalar",
        .model_name = "c_paid_b",
        .part_offset_pos = { 0.0f, 0.0f, 5.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitPider_defense,
        .defense_attr = unitPider_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitPider_pose_table,
    },
    {
        .index = 3,
        .name = "btl_un_churantalar",
        .model_name = "c_paid_b",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitPider_defense,
        .defense_attr = unitPider_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitPider_pose_table,
    },
    {
        .index = 4,
        .name = "btl_un_churantalar",
        .model_name = "c_paid_b",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitPider_defense,
        .defense_attr = unitPider_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitPider_pose_table,
    },
    {
        .index = 5,
        .name = "btl_un_churantalar",
        .model_name = "c_paid_b",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitPider_defense,
        .defense_attr = unitPider_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitPider_pose_table,
    },
};

BattleUnitKind unit_Pider = {
    .unit_type = BattleUnitType::PIDER,
    .unit_name = "btl_un_piders",
    .max_hp = 5,
    .max_fp = 0,
    .danger_hp = 1,
    .peril_hp = 1,
    .level = 1,
    .bonus_exp = 0,
    .bonus_coin = 0,
    .bonus_coin_rate = 100,
    .base_coin = 0,
    .run_rate = 70,
    .pb_soft_cap = 9999,
    .width = 46,
    .height = 56,
    .hit_offset = { 0, 56 },
    .center_offset = { 0.0f, 28.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 23.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 18.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 13.0f, 32.0f, 0.0f },
    .cut_base_offset = { 0.0f, 28.0f, 0.0f },
    .cut_width = 46.0f,
    .cut_height = 56.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_PAIDA_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::UNQUAKEABLE,
    .status_vulnerability = &unitPider_status,
    .num_parts = 5,
    .parts = unitPider_parts,
    .init_evt_code = (void*)unitPider_init_event,
    .data_table = unitPider_data_table,
};
BattleUnitKind unit_Arantula = {
    .unit_type = BattleUnitType::ARANTULA,
    .unit_name = "btl_un_churantalar",
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
    .width = 46,
    .height = 56,
    .hit_offset = { 0, 56 },
    .center_offset = { 0.0f, 28.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 23.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 18.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 13.0f, 32.0f, 0.0f },
    .cut_base_offset = { 0.0f, 28.0f, 0.0f },
    .cut_width = 46.0f,
    .cut_height = 56.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_PAIDA_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::UNQUAKEABLE,
    .status_vulnerability = &unitArantula_status,
    .num_parts = 5,
    .parts = unitArantula_parts,
    .init_evt_code = (void*)unitArantula_init_event,
    .data_table = unitPider_data_table,
};

// Function / USER_FUNC definitions.

EVT_DEFINE_USER_FUNC(unitPider_CheckLeaderOrder) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[0]));
    
    int32_t count = 0;
    for (int32_t i = 0; i < 64; ++i) {
        auto* unit = g_BattleWork->battle_units[i];
        if (unit && (
                unit->current_kind == BattleUnitType::PIDER ||
                unit->current_kind == BattleUnitType::ARANTULA)) {
            if (!ttyd::battle_unit::BtlUnit_CheckStatus(unit, StatusEffectType::OHKO))
                ++count;
            if (unit->unit_id == unit_idx) break;
        }
    }
    evtSetValue(evt, evt->evtArguments[1], count);
    return 2;
}

EVT_DEFINE_USER_FUNC(unitPider_GetLeaderHiLoPosition) {
    BattleWorkUnit* unit = nullptr;
    for (int32_t i = 0; i < 64; ++i) {
        unit = g_BattleWork->battle_units[i];
        if (unit && (
                unit->current_kind == BattleUnitType::PIDER ||
                unit->current_kind == BattleUnitType::ARANTULA
            ) && !ttyd::battle_unit::BtlUnit_CheckStatus(unit, StatusEffectType::OHKO))
            break;
    }
    int32_t value = unit->unit_work[UW_HiLoPosition];
    if (ttyd::battle_unit::BtlUnit_CanActStatus(unit)) value = !value;
    evtSetValue(evt, evt->evtArguments[0], value);
    return 2;
}

EVT_DEFINE_USER_FUNC(unitPider_GetPiderCount) {
    int32_t count = 0;
    for (int32_t i = 0; i < 64; ++i) {
        auto* unit = g_BattleWork->battle_units[i];
        if (unit && (
                unit->current_kind == BattleUnitType::PIDER ||
                unit->current_kind == BattleUnitType::ARANTULA
            ) && !ttyd::battle_unit::BtlUnit_CheckStatus(unit, StatusEffectType::OHKO))
            ++count;        
    }
    evtSetValue(evt, evt->evtArguments[0], count);
    return 2;
}

struct PiderYarnWork {
    gc::vec3 ceil_point;
    gc::vec3 hang_point;
    uint8_t alpha;
    uint8_t unused_19[3];
};

void _yarn_draw_sub(CameraId camera_id, void* user_data) {
    auto* work = reinterpret_cast<PiderYarnWork*>(user_data);
    auto* camera = ttyd::camdrv::camGetPtr(CameraId::k3d);
    uint32_t material_color = 0xffffff00U | work->alpha;
    
    ttyd::gx::GXGeometry::GXSetCullMode(GX_CULL_NONE);
    ttyd::gx::GXAttr::GXClearVtxDesc();
    ttyd::gx::GXAttr::GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    ttyd::gx::GXAttr::GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    ttyd::gx::GXLight::GXSetNumChans(1);
    ttyd::gx::GXLight::GXSetChanCtrl(GX_COLOR0A0, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHTNULL, GX_DF_CLAMP, GX_AF_NONE);
    ttyd::gx::GXLight::GXSetChanMatColor(4, &material_color);
    ttyd::gx::GXAttr::GXSetNumTexGens(0);
    ttyd::gx::GXTev::GXSetNumTevStages(1);
    ttyd::gx::GXTev::GXSetTevColorOp(0, 0, 0, 0, 1, 0);
    ttyd::gx::GXTev::GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
    ttyd::gx::GXTev::GXSetTevAlphaOp(0, 0, 0, 0, 1, 0);
    ttyd::gx::GXTev::GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RAS);
    ttyd::gx::GXTev::GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    ttyd::gx::GXTransform::GXLoadPosMtxImm(&camera->view_mtx, 0);
    
    // Draw a single quad representing the Pider's thread.
    ttyd::gx::GXGeometry::GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    ttyd::gx::GXGeometry::GX_Position3f32(
        work->ceil_point.x - 1.0f, work->ceil_point.y, work->ceil_point.z - 5.0f);
    ttyd::gx::GXGeometry::GX_Position3f32(
        work->ceil_point.x + 1.0f, work->ceil_point.y, work->ceil_point.z - 5.0f);
    ttyd::gx::GXGeometry::GX_Position3f32(
        work->hang_point.x + 1.0f, work->hang_point.y, work->hang_point.z - 5.0f);
    ttyd::gx::GXGeometry::GX_Position3f32(
        work->hang_point.x - 1.0f, work->hang_point.y, work->hang_point.z - 5.0f);
    ttyd::gx::GXGeometry::GXEnd();
}

EVT_DEFINE_USER_FUNC(unitPider_YarnDrawMain) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(evt, -2);
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, unit_idx);
    
    auto* work = reinterpret_cast<PiderYarnWork*>(unit->extra_work);
    ttyd::battle_unit::BtlUnit_GetHomePos(
        unit, &work->ceil_point.x, &work->ceil_point.y, &work->ceil_point.z);
    ttyd::battle_unit::BtlUnit_GetHomePos(
        unit, &work->hang_point.x, &work->hang_point.y, &work->hang_point.z);
    
    work->ceil_point.y = 400.0f;
    work->hang_point.y = unit->unit_work[UW_YarnHangPoint];
    work->alpha = evtGetValue(evt, evt->evtArguments[0]);
    
    ttyd::dispdrv::dispEntry(CameraId::k3d, 2, 0.0f, _yarn_draw_sub, work);
    
    return 2;
}

EVT_DEFINE_USER_FUNC(unitPider_YarnInit) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(evt, -2);
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, unit_idx);
    unit->extra_work = ttyd::battle::BattleAlloc(sizeof(PiderYarnWork));
    return 2;
}

// Evt definitions.

EVT_BEGIN(unitPider_attack_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BarrageHit1, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BarrageHit2, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BarrageHit3, 0)
    USER_FUNC(btlevtcmd_EnemyItemUseCheck, -2, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(LW(0))
        USER_FUNC(btlevtcmd_StartWaitEvent, -2)
        RETURN()
    END_IF()
LBL(0)

    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(0))
    IF_EQUAL(LW(0), (int32_t)BattleUnitType::PIDER)
        USER_FUNC(btlevtcmd_DrawLots, LW(0), 3, 50, 50, 0)
        SET(LW(10), PTR(&unitPider_weaponSingle))
        SET(LW(11), PTR(&unitPider_weaponBarrage))
    ELSE()
        USER_FUNC(btlevtcmd_DrawLots, LW(0), 3, 50, 0, 50)
        SET(LW(10), PTR(&unitArantula_weaponSingle))
        SET(LW(11), PTR(&unitArantula_weaponBarrage))
    END_IF()

    SWITCH(LW(0))
        CASE_EQUAL(1)
            SET(LW(9), LW(11))
            USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BarrageType, 0)
            RUN_CHILD_EVT(PTR(&unitPider_barrage_event))
        CASE_EQUAL(2)
            SET(LW(9), LW(11))
            USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BarrageType, 1)
            RUN_CHILD_EVT(PTR(&unitPider_crazy_barrage_event))
        CASE_ETC()
            SET(LW(9), LW(10))
            RUN_CHILD_EVT(PTR(&unitPider_normal_attack_event))
    END_SWITCH()
    
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitPider_battle_end_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_YarnAlpha, 254)
    RETURN()
EVT_END()

EVT_BEGIN(unitPider_damage_check_sub_event)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), LW(9), 0, LW(0))
    SWITCH(LW(0))
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
            GOTO(99)
            CASE_END()
    END_SWITCH()
LBL(90)
    USER_FUNC(btlevtcmd_JumpPartsContinue, -2, LW(7))
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, LW(7), 0x100'0000)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, LW(7), 0x200'0000)
    GOTO(99)
LBL(91)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, LW(7), 0x100'0000)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, LW(7), 0x200'0000)
    IF_EQUAL(LW(10), 1)
        SET(LW(0), UW_BarrageTargetUnit1)
        SET(LW(1), UW_BarrageTargetPart1)
        DO(0)
            USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(0), LW(2))
            IF_EQUAL(LW(3), LW(2))
                USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(1), LW(2))
                IF_EQUAL(LW(4), LW(2))
                    ADD(LW(0), 6)  // Get corresponding 'hit' state
                    USER_FUNC(btlevtcmd_SetUnitWork, -2, LW(0), 1)
                    DO_BREAK()
                END_IF()
            END_IF()
            ADD(LW(0), 1)
            ADD(LW(1), 1)
        WHILE()
    END_IF()
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
    IF_EQUAL(LW(8), 1)
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(0))
    ELSE()
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 0, LW(0))
    END_IF()
    GOTO(99)
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitPider_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    RETURN()
EVT_END()

EVT_BEGIN(unitPider_dead_event)
    SET(LW(0), 48)
    USER_FUNC(btlevtcmd_snd_se, LW(10), PTR("SFX_BTL_ENEMY_DIE1_1"), EVT_NULLPTR, 0, EVT_NULLPTR)
LBL(10)
    WAIT_FRM(1)
    USER_FUNC(btlevtcmd_AddRotate, LW(10), 0, -15, 0)
    SUB(LW(0), 1)
    IF_LARGE(LW(0), 0)
        GOTO(10)
    END_IF()
    USER_FUNC(btlevtcmd_SetRotate, LW(10), 0, 0, 0)
    USER_FUNC(btlevtcmd_snd_se, LW(10), PTR("SFX_BTL_ENEMY_DIE1_2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetPos, LW(10), LW(0), LW(1), LW(2))
    ADD(LW(2), 10)
    USER_FUNC(evt_eff64, PTR(""), PTR("expbom_n64"), LW(0), LW(1), LW(2), 0, 0, 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(btlevtcmd_GetCoin, LW(10), LW(0))
    USER_FUNC(btlevtcmd_StoreCoin, LW(0))
    USER_FUNC(btlevtcmd_GetExp, LW(10), LW(0))
    USER_FUNC(btlevtcmd_StoreExp, LW(0))
    USER_FUNC(btlevtcmd_GetPos, LW(10), LW(3), LW(1), LW(2))
    ADD(LW(2), 10)
    ADD(LW(1), 30)
    USER_FUNC(_add_star_point_disp_offset, LW(10), LW(3), LW(1), LW(2))
    USER_FUNC(evt_eff, PTR(""), PTR("star_point"), 0, LW(3), LW(1), LW(2), LW(0), 0, 0, 0, 0, 0, 0, 0)
    IF_LARGE_EQUAL(LW(0), 1)
        DO(LW(0))
        WHILE()
    END_IF()
    USER_FUNC(btlevtcmd_SetRotateOffset, LW(10), 0, 0, 0)
    SET(LW(0), 30)
LBL(20)
    WAIT_FRM(1)
    USER_FUNC(btlevtcmd_AddRotate, LW(10), 3, 0, 0)
    SUB(LW(0), 1)
    IF_LARGE(LW(0), 0)
        GOTO(20)
    END_IF()
    USER_FUNC(btlevtcmd_OnPartsAttribute, LW(10), 1, 0x100'0000)
    USER_FUNC(btlevtcmd_OnPartsAttribute, LW(10), 1, 0x200'0000)
    USER_FUNC(btlevtcmd_OnPartsAttribute, LW(10), 1, 0x1'0000)
    USER_FUNC(btlevtcmd_OnAttribute, LW(10), 0x1000'0000)
    USER_FUNC(btlevtcmd_SetMaxMoveCount, LW(10), 0)
    USER_FUNC(btlevtcmd_OnAttribute, LW(10), 0x200'0000)
    USER_FUNC(btlevtcmd_SetPos, LW(10), 0, -1000, 0)
    USER_FUNC(btlevtcmd_KillUnit, LW(10), 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitPider_gale_dead_event)
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_CLAUD_BREATH3"), 0)
    USER_FUNC(btlevtcmd_GetBodyId, -2, LW(0))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, LW(0), 39)
    BROTHER_EVT()
LBL(0)
        USER_FUNC(btlevtcmd_AddRotate, -2, 0, 20, 0)
        WAIT_FRM(1)
        GOTO(0)
    END_BROTHER()
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(2.0))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetStageSize, LW(0), EVT_NULLPTR, EVT_NULLPTR)
    ADD(LW(1), 40)
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 0, -10, 2, 0, -1)

    // Disable coins / EXP from Gale Force.
    // USER_FUNC(btlevtcmd_GetCoin, LW(10), LW(0))
    // USER_FUNC(btlevtcmd_StoreCoin, LW(0))
    // USER_FUNC(btlevtcmd_GetExp, LW(10), LW(0))
    // USER_FUNC(btlevtcmd_StoreExp, LW(0))
    
    USER_FUNC(btlevtcmd_OnPartsAttribute, LW(10), 1, 0x100'0000)
    USER_FUNC(btlevtcmd_OnPartsAttribute, LW(10), 1, 0x1'0000)
    USER_FUNC(btlevtcmd_OnAttribute, LW(10), 0x1000'0000)
    USER_FUNC(btlevtcmd_SetMaxMoveCount, LW(10), 0)
    USER_FUNC(btlevtcmd_OnAttribute, LW(10), 0x200'0000)
    USER_FUNC(btlevtcmd_SetPos, LW(10), 0, -1000, 0)
    USER_FUNC(btlevtcmd_KillUnit, LW(10), 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitPider_first_attack_pos_event)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    SET(LW(1), 10)
    USER_FUNC(btlevtcmd_SetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_WaitAttackEnd)
    RETURN()
EVT_END()

EVT_BEGIN(unitPider_entry_event)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    SET(LW(1), 180)
    USER_FUNC(btlevtcmd_SetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_YarnHangPoint, 400)
    DIV(LW(0), 10)
    ADD(LW(0), 80)
    WAIT_FRM(LW(0))
    USER_FUNC(evt_sub_intpl_init, 4, 400, 0, 40)
    DO(40)
        USER_FUNC(evt_sub_intpl_get_value)
        USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_YarnHangPoint, LW(0))
        WAIT_FRM(1)
    WHILE()
    RETURN()
EVT_END()

EVT_BEGIN(unitPider_move_up_event)
    RUN_EVT(PTR(&unitPider_move_spin_event))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    SET(LW(1), 70)
    BROTHER_EVT()
        DO(4)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_PAIDA_MOVE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_FRM(7)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 30, 10, 4, 0, -1)
    USER_FUNC(btlevtcmd_SetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 1, 0x60'0000)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_HiLoPosition, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitPider_move_down_event)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_NeverMove, LW(0))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_NeverMove, 0)
        BROTHER_EVT()
            DO(4)
                USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_PAIDA_MOVE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
                WAIT_FRM(7)
            WHILE()
        END_BROTHER()
    ELSE()
        BROTHER_EVT()
            DO(4)
                USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_PAIDA_MOVE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
                WAIT_FRM(7)
            WHILE()
        END_BROTHER()
    END_IF()
    RUN_CHILD_EVT(PTR(&unitPider_move_spin_event))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    SET(LW(1), 10)
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 30, -10, 4, 0, -1)
    USER_FUNC(btlevtcmd_SetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, 0x60'0000)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_HiLoPosition, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitPider_normal_attack_event)
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        GOTO(99)
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
    SET(LW(7), 3)
    RUN_CHILD_EVT(PTR(&unitPider_attack_common_event1))
    RUN_EVT(PTR(&unitPider_attack_common_event2))
    SET(LW(8), 1)
    SET(LW(5), 5)
    RUN_CHILD_EVT(PTR(&unitPider_attack_common_event3))
    SET(LW(10), 0)
    RUN_CHILD_EVT(PTR(&unitPider_damage_check_sub_event))
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    WAIT_MSEC(500)
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitPider_attack_common_event1)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(8))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(8))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    ADD(LW(2), FLOAT(3.0))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PAI_A_1"))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 2, PTR("PAI_A_2a"))
    USER_FUNC(btlevtcmd_SetPartsPos, -2, 2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 2, 0x100'0000)
    USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(1))
    SETF(LW(0), FLOAT(30.0))
    MULF(LW(0), LW(1))
    USER_FUNC(btlevtcmd_SetPartsRotateOffset, -2, 2, 0, LW(0), 0)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_PAIDA_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(evt_sub_intpl_init, 4, 0, 110, 5)
    DO(5)
        USER_FUNC(evt_sub_intpl_get_value)
        MUL(LW(0), LW(8))
        USER_FUNC(btlevtcmd_SetPartsRotate, -2, 2, 0, 0, LW(0))
        WAIT_FRM(1)
    WHILE()
    WAIT_FRM(20)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_PAIDA_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(evt_sub_intpl_init, 4, 110, 70, 5)
    DO(5)
        USER_FUNC(evt_sub_intpl_get_value)
        MUL(LW(0), LW(8))
        USER_FUNC(btlevtcmd_SetPartsRotate, -2, 2, 0, 0, LW(0))
        WAIT_FRM(1)
    WHILE()
    WAIT_FRM(15)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_PAIDA_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(evt_sub_intpl_init, 4, 70, 100, 5)
    DO(5)
        USER_FUNC(evt_sub_intpl_get_value)
        MUL(LW(0), LW(8))
        USER_FUNC(btlevtcmd_SetPartsRotate, -2, 2, 0, 0, LW(0))
        WAIT_FRM(1)
    WHILE()
    WAIT_FRM(10)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_RotationPoint, LW(0))
    RETURN()
EVT_END()

EVT_BEGIN(unitPider_attack_common_event2)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(8))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(8))
    USER_FUNC(evt_sub_intpl_init, 4, 100, 0, 15)
    DO(15)
        USER_FUNC(evt_sub_intpl_get_value)
        MUL(LW(0), LW(8))
        USER_FUNC(btlevtcmd_SetPartsRotate, -2, 2, 0, 0, LW(0))
        WAIT_FRM(1)
    WHILE()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PAI_S_1"))
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 2, 0x100'0000)
    RETURN()
EVT_END()

EVT_BEGIN(unitPider_attack_common_event3)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_PAIDA_ATTACK2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, LW(7), PTR("PAI_A_3a"))
    
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BarrageType, LW(0))
    IF_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        ADD(LW(1), 20)
        USER_FUNC(btlevtcmd_JumpPartsSetting, -2, LW(7), 0, FLOAT(6.0), FLOAT(0.25))
    ELSE()
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        SWITCH(LW(5))
            CASE_EQUAL(5)
                ADD(LW(1), 20)
                USER_FUNC(btlevtcmd_JumpPartsSetting, -2, LW(7), 0, FLOAT(6.0), FLOAT(0.25))
            CASE_EQUAL(6)
                ADD(LW(1), 30)
                USER_FUNC(btlevtcmd_JumpPartsSetting, -2, LW(7), 0, FLOAT(4.0), FLOAT(0.25))
            CASE_EQUAL(7)
                ADD(LW(1), 15)
                USER_FUNC(btlevtcmd_JumpPartsSetting, -2, LW(7), 0, FLOAT(8.0), FLOAT(0.25))
        END_SWITCH()
    END_IF()

    USER_FUNC(btlevtcmd_SetPartsPos, -2, LW(7), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, LW(7), 0x100'0000)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, LW(7), 0x200'0000)
    USER_FUNC(btlevtcmd_SetPartsScale, -2, LW(7), FLOAT(2.0), FLOAT(2.0), FLOAT(2.0))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(7), LW(0), LW(1), LW(2), 0, -1)
    RETURN()
EVT_END()

EVT_BEGIN(unitPider_barrage_event)
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
    USER_FUNC(evtTot_SelectMultihitTargetsX3, LW(0), LW(1), LW(2), LW(3), LW(4), LW(5))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BarrageTargetUnit1, LW(0))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BarrageTargetUnit2, LW(2))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BarrageTargetUnit3, LW(4))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BarrageTargetPart1, LW(1))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BarrageTargetPart2, LW(3))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BarrageTargetPart3, LW(5))

    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BarrageTargetUnit1, LW(3))
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BarrageTargetPart1, LW(4))
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    SET(LW(5), UW_BarrageTargetUnit1)
    SET(LW(6), UW_BarrageTargetPart1)
    SET(LW(7), 3)
    RUN_CHILD_EVT(PTR(&unitPider_attack_common_event1))
    DO(3)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(5), LW(3))
        USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(6), LW(4))
        RUN_CHILD_EVT(PTR(&unitPider_attack_common_event3))
        RUN_EVT(PTR(&unitPider_barrage_event_sub))
        ADD(LW(5), 1)
        ADD(LW(6), 1)
        ADD(LW(7), 1)
        WAIT_FRM(30)
    WHILE()
    RUN_EVT(PTR(&unitPider_attack_common_event2))
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    WAIT_MSEC(250)
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitPider_crazy_barrage_event)
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
    USER_FUNC(evtTot_SelectMultihitTargetsX3, LW(0), LW(1), LW(2), LW(3), LW(4), LW(5))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BarrageTargetUnit1, LW(0))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BarrageTargetUnit2, LW(2))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BarrageTargetUnit3, LW(4))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BarrageTargetPart1, LW(1))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BarrageTargetPart2, LW(3))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BarrageTargetPart3, LW(5))

    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BarrageTargetUnit1, LW(3))
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BarrageTargetPart1, LW(4))
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    SET(LW(5), UW_BarrageTargetUnit1)
    SET(LW(6), UW_BarrageTargetPart1)
    SET(LW(7), 3)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(5), LW(3))
    USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(6), LW(4))
    RUN_CHILD_EVT(PTR(&unitPider_attack_common_event1))
    RUN_CHILD_EVT(PTR(&unitPider_attack_common_event3))
    RUN_EVT(PTR(&unitPider_barrage_event_sub))
    WAIT_FRM(1)
    ADD(LW(5), 1)
    ADD(LW(6), 1)
    ADD(LW(7), 1)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(5), LW(3))
    USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(6), LW(4))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(8))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(8))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_PAIDA_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(evt_sub_intpl_init, 4, 100, -200, 20)
    BROTHER_EVT()
        DO(20)
            USER_FUNC(evt_sub_intpl_get_value)
            MUL(LW(0), LW(8))
            USER_FUNC(btlevtcmd_SetPartsRotate, -2, 2, 0, 0, LW(0))
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()
    WAIT_FRM(15)
    RUN_CHILD_EVT(PTR(&unitPider_attack_common_event3))
    RUN_EVT(PTR(&unitPider_barrage_event_sub))
    WAIT_FRM(1)
    ADD(LW(5), 1)
    ADD(LW(6), 1)
    ADD(LW(7), 1)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(5), LW(3))
    USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(6), LW(4))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(8))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(8))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_PAIDA_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(evt_sub_intpl_init, 4, -200, 170, 20)
    BROTHER_EVT()
        DO(20)
            USER_FUNC(evt_sub_intpl_get_value)
            MUL(LW(0), LW(8))
            USER_FUNC(btlevtcmd_SetPartsRotate, -2, 2, 0, 0, LW(0))
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()
    WAIT_FRM(15)
    RUN_CHILD_EVT(PTR(&unitPider_attack_common_event3))
    RUN_CHILD_EVT(PTR(&unitPider_barrage_event_sub))
    WAIT_FRM(1)
    ADD(LW(5), 1)
    ADD(LW(6), 1)
    ADD(LW(7), 1)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(8))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(8))
    USER_FUNC(evt_sub_intpl_init, 4, 170, 0, 15)
    DO(15)
        USER_FUNC(evt_sub_intpl_get_value)
        MUL(LW(0), LW(8))
        USER_FUNC(btlevtcmd_SetPartsRotate, -2, 2, 0, 0, LW(0))
        WAIT_FRM(1)
    WHILE()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PAI_S_1"))
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 2, 0x100'0000)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    WAIT_MSEC(250)
LBL(99)
    RETURN()
EVT_END()

// Do pre-damage / damage checks, depending on whether this is the last
// time the particular target is being hit or not.
EVT_BEGIN(unitPider_barrage_event_sub)
    SET(LW(0), LW(5))
    SET(LW(1), LW(6))
    USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(0), LW(3))
    USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(1), LW(4))
    DO(0)
        IF_LARGE_EQUAL(LW(0), UW_BarrageTargetUnit3)
            RUN_CHILD_EVT(PTR(&unitPider_barrage_event_sub_last))
            DO_BREAK()
        END_IF()
        ADD(LW(0), 1)
        ADD(LW(1), 1)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(0), LW(8))
        USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(1), LW(10))
        IF_EQUAL(LW(3), LW(8))
            IF_EQUAL(LW(4), LW(10))
                RUN_CHILD_EVT(PTR(&unitPider_barrage_event_sub_nolast))
                DO_BREAK()
            END_IF()
        END_IF()
    WHILE()
    RETURN()
EVT_END()

// Forces a hit on the last if the character's been hit before.
EVT_BEGIN(unitPider_barrage_event_sub_last)
    SET(LW(0), UW_BarrageTargetUnit1)
    SET(LW(1), UW_BarrageTargetPart1)
    DO(0)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(0), LW(2))
        IF_EQUAL(LW(3), LW(2))
            USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(1), LW(2))
            IF_EQUAL(LW(4), LW(2))
                ADD(LW(0), 6)  // Get corresponding 'hit' state
                USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(0), LW(2))
                DO_BREAK()
            END_IF()
        END_IF()
        ADD(LW(0), 1)
        ADD(LW(1), 1)
    WHILE()
    IF_EQUAL(LW(2), 1)
        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, LW(7), 0x100'0000)
        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, LW(7), 0x200'0000)
        USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(0))
    ELSE()
        SET(LW(8), 1)
        SET(LW(10), 0)
        RUN_CHILD_EVT(PTR(&unitPider_damage_check_sub_event))
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitPider_barrage_event_sub_nolast)
    SET(LW(8), 0)
    SET(LW(10), 1)
    RUN_CHILD_EVT(PTR(&unitPider_damage_check_sub_event))
    RETURN()
EVT_END()

EVT_BEGIN(unitPider_unison_phase_event)
    USER_FUNC(btlevtcmd_CheckPhase, LW(0), 0x400'0001)
    IF_EQUAL(LW(0), 0)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_CheckStatus, -2, 27, LW(0))
    IF_EQUAL(LW(0), 1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_CheckActStatus, -2, LW(0))
    IF_EQUAL(LW(0), 0)
        GOTO(99)
    END_IF()
    USER_FUNC(unitPider_CheckLeaderOrder, -2, LW(14))
    IF_EQUAL(LW(14), 1)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_HiLoPosition, LW(0))
        IF_EQUAL(LW(0), 1)
            RUN_CHILD_EVT(PTR(&unitPider_move_up_event))
        ELSE()
            RUN_CHILD_EVT(PTR(&unitPider_move_down_event))
        END_IF()
        GOTO(99)
    END_IF()
    USER_FUNC(unitPider_GetLeaderHiLoPosition, LW(13))
    USER_FUNC(unitPider_GetPiderCount, LW(15))
    MOD(LW(15), 2)
    IF_EQUAL(LW(15), 1)
        IF_EQUAL(LW(13), 0)
            RUN_CHILD_EVT(PTR(&unitPider_move_up_event))
            GOTO(99)
        ELSE()
            RUN_CHILD_EVT(PTR(&unitPider_move_down_event))
            GOTO(99)
        END_IF()
    ELSE()
        MOD(LW(14), 2)
        IF_EQUAL(LW(14), 1)
            IF_EQUAL(LW(13), 0)
                RUN_CHILD_EVT(PTR(&unitPider_move_up_event))
                GOTO(99)
            ELSE()
                RUN_CHILD_EVT(PTR(&unitPider_move_down_event))
                GOTO(99)
            END_IF()
        ELSE()
            IF_EQUAL(LW(13), 0)
                RUN_CHILD_EVT(PTR(&unitPider_move_down_event))
                GOTO(99)
            ELSE()
                RUN_CHILD_EVT(PTR(&unitPider_move_up_event))
                GOTO(99)
            END_IF()
        END_IF()
    END_IF()
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitPider_wait_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitPider_yarn_event)
LBL(0)
    USER_FUNC(btlevtcmd_GetUnitId, -2, LW(0))
    IF_EQUAL(LW(0), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_YarnAlpha, LW(0))
    IF_SMALL(LW(0), 255)
        IF_LARGE(LW(0), 2)
            SUB(LW(0), 2)
            USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_YarnAlpha, LW(0))
        END_IF()
    END_IF()
    USER_FUNC(unitPider_YarnDrawMain, LW(0))
    WAIT_FRM(1)
    GOTO(0)
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitPider_move_spin_event)
    INLINE_EVT()
        DO(10)
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, 6, 0)
            WAIT_FRM(1)
        WHILE()
        DO(22)
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, -5, 0)
            WAIT_FRM(1)
        WHILE()
        DO(20)
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, 4, 0)
            WAIT_FRM(1)
        WHILE()
        DO(16)
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, -3, 0)
            WAIT_FRM(1)
        WHILE()
        DO(14)
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, 2, 0)
            WAIT_FRM(1)
        WHILE()
        DO(10)
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, -1, 0)
            WAIT_FRM(1)
        WHILE()
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    END_INLINE()
    RETURN()
EVT_END()

EVT_BEGIN(unitPider_common_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitPider_wait_event))
    USER_FUNC(btlevtcmd_SetEventUnisonPhase, -2, PTR(&unitPider_unison_phase_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitPider_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitPider_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitPider_attack_event))
    USER_FUNC(btlevtcmd_SetEventEntry, -2, PTR(&unitPider_entry_event))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_HiLoPosition, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_NeverMove, 1)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_YarnHangPoint, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_YarnAlpha, 255)
    USER_FUNC(unitPider_YarnInit)
    RUN_EVT(PTR(&unitPider_yarn_event))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitPider_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::PIDER)
    RUN_CHILD_EVT(unitPider_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitArantula_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::ARANTULA)
    RUN_CHILD_EVT(unitPider_common_init_event)
    RETURN()
EVT_END()

}  // namespace mod::tot::custom