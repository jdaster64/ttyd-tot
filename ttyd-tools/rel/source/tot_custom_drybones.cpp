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
constexpr const int32_t UW_BoneRotation = 0;
constexpr const int32_t UW_HasGivenRewards = 1;
constexpr const int32_t UW_ReviveTurnCount = 2;
constexpr const int32_t UW_BarrageLanded = 3;
constexpr const int32_t UW_BarrageCount = 4;
constexpr const int32_t UW_BattleUnitType = 5;

}  // namespace

// Evt / Function declarations.

extern const int32_t unitDullBones_init_event[];
extern const int32_t unitRedBones_init_event[];
extern const int32_t unitDryBones_init_event[];
extern const int32_t unitDarkBones_init_event[];
extern const int32_t unitBones_common_init_event[];
extern const int32_t unitBones_wait_event[];
extern const int32_t unitBones_attack_event[];
extern const int32_t unitBones_damage_event[];
extern const int32_t unitBones_dead_event[];
extern const int32_t unitBones_fake_death_event[];
extern const int32_t unitBones_rebirth_event[];
extern const int32_t unitBones_normal_attack_event[];
extern const int32_t unitBones_barrage_event[];
extern const int32_t unitBones_barrage_event_sub[];
extern const int32_t unitBones_bone_miss_event[];
extern const int32_t unitBones_barrage_bone_miss_event[];
extern const int32_t unitDullBones_build_event[];
extern const int32_t unitDryBones_build_event[];

EVT_DECLARE_USER_FUNC(unitBones_copy_status, 2)

// Unit data.

int8_t unitDryBones_defense[] = { 1, 1, 1, 1, 1 };
int8_t unitDryBones_defense_attr[] = { 0, 0, 0, 0, 0 };

StatusVulnerability unitDullBones_status = {
      0,  95,  50,   0,  50, 100, 100,  90,
    100, 100, 100, 100, 100,  95,  90,  10,
     90, 100,  90, 100, 100,  50,
};
StatusVulnerability unitRedBones_status = {
      0,  85,  40,   0,  40, 100,  90,  80,
    100,  90, 100,  90, 100,  90,  80,   0,
     80, 100,  80, 100, 100,   0,
};
StatusVulnerability unitDryBones_status = {
      0,  75,  30,   0,  30, 100, 100,  70,
    100,  90, 100,  90, 100,  85,  70,   0,
     70, 100,  70, 100, 100,  90,
};
StatusVulnerability unitDarkBones_status = {
      0,  65,  20,   0,  20, 100, 100,  60,
    100,  80, 100,  80, 100,  80,  60,   0,
     60, 100,  60, 100, 100,  85,
};

PoseTableEntry unitDryBones_pose_table[] = {
    1, "KRN_N_1",
    2, "KRN_Y_1",
    9, "KRN_Y_1",
    5, "KRN_K_1",
    4, "KRN_X_1",
    3, "KRN_X_1",
    27, "KRN_Z_2",
    28, "KRN_S_1",
    29, "KRN_Q_1",
    30, "KRN_Q_1",
    31, "KRN_S_1",
    39, "KRN_D_1",
    50, "KRN_A_1",
    42, "KRN_R_1",
    40, "KRN_W_1",
    56, "KRN_X_1",
    57, "KRN_X_1",
    65, "KRN_T_1",
    69, "KRN_S_1",
};

DataTableEntry unitDryBones_data_table[] = {
    49, (void*)unitBones_dead_event,
    15, (void*)unitBones_fake_death_event,
    48, (void*)btldefaultevt_Dummy,
    0, nullptr,
};

BattleWeapon unitDryBones_weaponNormal = {
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
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
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
BattleWeapon unitDryBones_weaponBarrage = {
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
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
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

BattleUnitKindPart unitDullBones_parts[] = {
    {
        .index = 1,
        .name = "btl_un_honenoko",
        .model_name = "c_karon_g_h",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 35.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 45.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitDryBones_defense,
        .defense_attr = unitDryBones_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitDryBones_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_honenoko",
        .model_name = "c_karon_g_h",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitDryBones_defense,
        .defense_attr = unitDryBones_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitDryBones_pose_table,
    },
    {
        .index = 3,
        .name = "btl_un_honenoko",
        .model_name = "c_nagehone_g",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitDryBones_defense,
        .defense_attr = unitDryBones_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitDryBones_pose_table,
    },
    {
        .index = 4,
        .name = "btl_un_honenoko",
        .model_name = "c_karon_g_h",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitDryBones_defense,
        .defense_attr = unitDryBones_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitDryBones_pose_table,
    },
};
BattleUnitKindPart unitRedBones_parts[] = {
    {
        .index = 1,
        .name = "btl_un_red_honenoko",
        .model_name = "c_karon_r_h",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 35.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 45.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitDryBones_defense,
        .defense_attr = unitDryBones_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitDryBones_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_red_honenoko",
        .model_name = "c_karon_r_h",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitDryBones_defense,
        .defense_attr = unitDryBones_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitDryBones_pose_table,
    },
    {
        .index = 3,
        .name = "btl_un_red_honenoko",
        .model_name = "c_nagehone_g",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitDryBones_defense,
        .defense_attr = unitDryBones_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitDryBones_pose_table,
    },
    {
        .index = 4,
        .name = "btl_un_red_honenoko",
        .model_name = "c_karon_g_h",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitDryBones_defense,
        .defense_attr = unitDryBones_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitDryBones_pose_table,
    },
};
BattleUnitKindPart unitDryBones_parts[] = {
    {
        .index = 1,
        .name = "btl_un_karon",
        .model_name = "c_karon_h",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 35.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 45.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitDryBones_defense,
        .defense_attr = unitDryBones_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitDryBones_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_karon",
        .model_name = "c_karon_h",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitDryBones_defense,
        .defense_attr = unitDryBones_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitDryBones_pose_table,
    },
    {
        .index = 3,
        .name = "btl_un_karon",
        .model_name = "c_karon_h",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitDryBones_defense,
        .defense_attr = unitDryBones_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitDryBones_pose_table,
    },
    {
        .index = 4,
        .name = "btl_un_karon",
        .model_name = "c_karon_h",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitDryBones_defense,
        .defense_attr = unitDryBones_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitDryBones_pose_table,
    },
    {
        .index = 5,
        .name = "btl_un_karon",
        .model_name = "c_nagehone",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitDryBones_defense,
        .defense_attr = unitDryBones_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitDryBones_pose_table,
    },
    {
        .index = 6,
        .name = "btl_un_karon",
        .model_name = "c_karon_h",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitDryBones_defense,
        .defense_attr = unitDryBones_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitDryBones_pose_table,
    },
};
BattleUnitKindPart unitDarkBones_parts[] = {
    {
        .index = 1,
        .name = "btl_un_black_karon",
        .model_name = "c_karon_b_h",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 35.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 45.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitDryBones_defense,
        .defense_attr = unitDryBones_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitDryBones_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_black_karon",
        .model_name = "c_karon_b_h",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitDryBones_defense,
        .defense_attr = unitDryBones_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitDryBones_pose_table,
    },
    {
        .index = 3,
        .name = "btl_un_black_karon",
        .model_name = "c_karon_b_h",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitDryBones_defense,
        .defense_attr = unitDryBones_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitDryBones_pose_table,
    },
    {
        .index = 4,
        .name = "btl_un_black_karon",
        .model_name = "c_karon_b_h",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitDryBones_defense,
        .defense_attr = unitDryBones_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitDryBones_pose_table,
    },
    {
        .index = 5,
        .name = "btl_un_black_karon",
        .model_name = "c_nagehone",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitDryBones_defense,
        .defense_attr = unitDryBones_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitDryBones_pose_table,
    },
    {
        .index = 6,
        .name = "btl_un_black_karon",
        .model_name = "c_karon_h",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitDryBones_defense,
        .defense_attr = unitDryBones_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitDryBones_pose_table,
    },
};

BattleUnitKind unit_DullBones = {
    .unit_type = BattleUnitType::DULL_BONES,
    .unit_name = "btl_un_honenoko",
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
    .width = 36,
    .height = 40,
    .hit_offset = { -4, 40 },
    .center_offset = { 0.0f, 20.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 18.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 15.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 9.0f, 24.0f, 0.0f },
    .cut_base_offset = { 0.0f, 20.0f, 0.0f },
    .cut_width = 36.0f,
    .cut_height = 40.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_HONENOKO_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitDullBones_status,
    .num_parts = 4,
    .parts = unitDullBones_parts,
    .init_evt_code = (void*)unitDullBones_init_event,
    .data_table = unitDryBones_data_table,
};
BattleUnitKind unit_RedBones = {
    .unit_type = BattleUnitType::RED_BONES,
    .unit_name = "btl_un_red_honenoko",
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
    .height = 40,
    .hit_offset = { -4, 40 },
    .center_offset = { 0.0f, 20.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 18.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 15.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 9.0f, 24.0f, 0.0f },
    .cut_base_offset = { 0.0f, 20.0f, 0.0f },
    .cut_width = 36.0f,
    .cut_height = 40.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_HONENOKO_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::UNDEAD,
    .status_vulnerability = &unitRedBones_status,
    .num_parts = 4,
    .parts = unitRedBones_parts,
    .init_evt_code = (void*)unitRedBones_init_event,
    .data_table = unitDryBones_data_table,
};
BattleUnitKind unit_DryBones = {
    .unit_type = BattleUnitType::DRY_BONES,
    .unit_name = "btl_un_karon",
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
    .height = 40,
    .hit_offset = { -4, 40 },
    .center_offset = { 0.0f, 20.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 18.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 15.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 9.0f, 24.0f, 0.0f },
    .cut_base_offset = { 0.0f, 20.0f, 0.0f },
    .cut_width = 36.0f,
    .cut_height = 40.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_HONENOKO_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::UNDEAD,
    .status_vulnerability = &unitDryBones_status,
    .num_parts = 6,
    .parts = unitDryBones_parts,
    .init_evt_code = (void*)unitDryBones_init_event,
    .data_table = unitDryBones_data_table,
};
BattleUnitKind unit_DarkBones = {
    .unit_type = BattleUnitType::DARK_BONES,
    .unit_name = "btl_un_black_karon",
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
    .height = 40,
    .hit_offset = { -4, 40 },
    .center_offset = { 0.0f, 20.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 18.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 15.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 9.0f, 24.0f, 0.0f },
    .cut_base_offset = { 0.0f, 20.0f, 0.0f },
    .cut_width = 36.0f,
    .cut_height = 40.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_HONENOKO_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::UNDEAD,
    .status_vulnerability = &unitDarkBones_status,
    .num_parts = 6,
    .parts = unitDarkBones_parts,
    .init_evt_code = (void*)unitDarkBones_init_event,
    .data_table = unitDryBones_data_table,
};

const BattleUnitSetup unitDullBones_spawn_entry = {
    .unit_kind_params = &unit_DullBones,
    .alliance = 1,
    .attack_phase = 0x04000004,
    .position = { -1000.0f, -1000.0f, -1000.0f },
    .addl_target_offset_x = 0,
    .unit_work = { 0, 0, 0, 0 },
    .item_drop_table = nullptr,
};
const BattleUnitSetup unitDryBones_spawn_entry = {
    .unit_kind_params = &unit_DryBones,
    .alliance = 1,
    .attack_phase = 0x04000004,
    .position = { -1000.0f, -1000.0f, -1000.0f },
    .addl_target_offset_x = 0,
    .unit_work = { 0, 0, 0, 0 },
    .item_drop_table = nullptr,
};

// Function / USER_FUNC definitions.

EVT_DEFINE_USER_FUNC(unitBones_copy_status) {
    auto* unit1 = ttyd::battle::BattleGetUnitPtr(
        g_BattleWork, ttyd::battle_sub::BattleTransID(
            evt, evtGetValue(evt, evt->evtArguments[0])));
    auto* unit2 = ttyd::battle::BattleGetUnitPtr(
        g_BattleWork, ttyd::battle_sub::BattleTransID(
            evt, evtGetValue(evt, evt->evtArguments[1])));
        
    unit2->size_change_turns = unit1->size_change_turns;
    unit2->size_change_strength = unit1->size_change_strength;
    
    return 2;
}

// Evt definitions.

EVT_BEGIN(unitBones_attack_event)
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(PTR(&unitBones_normal_attack_event))
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_ReviveTurnCount, LW(0))
    IF_LARGE_EQUAL(LW(0), 1)
        RUN_CHILD_EVT(PTR(&unitBones_rebirth_event))
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_EnemyItemUseCheck, -2, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(LW(0))
        USER_FUNC(btlevtcmd_StartWaitEvent, -2)
        RETURN()
    END_IF()

LBL(0)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(0))
    SWITCH(LW(0))
        CASE_EQUAL((int32_t)BattleUnitType::DULL_BONES)
            USER_FUNC(btlevtcmd_DrawLots, LW(0), 3, 85, 0, 15)
            SET(LW(15), PTR(&unitDullBones_build_event))
        CASE_EQUAL((int32_t)BattleUnitType::RED_BONES)
            USER_FUNC(btlevtcmd_DrawLots, LW(0), 3, 60, 0, 40)
            SET(LW(15), PTR(&unitDullBones_build_event))
        CASE_EQUAL((int32_t)BattleUnitType::DRY_BONES)
            USER_FUNC(btlevtcmd_DrawLots, LW(0), 3, 50, 30, 20)
            SET(LW(15), PTR(&unitDryBones_build_event))
        CASE_ETC()
            USER_FUNC(btlevtcmd_DrawLots, LW(0), 3, 50, 30, 20)
            SET(LW(15), PTR(&unitDryBones_build_event))
    END_SWITCH()

    IF_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(PTR(&unitBones_normal_attack_event))
        GOTO(99)
    END_IF()
    IF_EQUAL(LW(0), 1)
        RUN_CHILD_EVT(PTR(&unitBones_barrage_event))
        GOTO(99)
    END_IF()
    
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    DO(0)
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 40)
        SET(LW(4), LW(0))
        USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(5))
        MUL(LW(4), LW(5))
        IF_SMALL(LW(4), 0)
            SUB(LW(2), 10)
            USER_FUNC(btlevtcmd_CheckSpace, LW(3), LW(0), LW(1), LW(2), 40, 0, 0, 0)
            IF_EQUAL(LW(3), -1)
                RUN_CHILD_EVT(LW(15))
                WAIT_MSEC(500)
                GOTO(99)
            END_IF()
        ELSE()
            DO_BREAK()
        END_IF()
    WHILE()
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    DO(0)
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 40)
        IF_SMALL_EQUAL(LW(0), 170)
            ADD(LW(2), 10)
            USER_FUNC(btlevtcmd_CheckSpace, LW(3), LW(0), LW(1), LW(2), 40, 0, 0, 0)
            IF_EQUAL(LW(3), -1)
                USER_FUNC(btlevtcmd_SetRotate, -2, 0, 180, 0)
                RUN_CHILD_EVT(LW(15))
                USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
                WAIT_MSEC(500)
                GOTO(99)
            END_IF()
        ELSE()
            DO_BREAK()
        END_IF()
    WHILE()
    GOTO(0)
LBL(99)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitDullBones_build_event)
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, -1)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    SET(LW(10), 0)
    DO(6)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KRN_A_3"))
        WAIT_FRM(5)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KRN_A_4"))
        WAIT_FRM(15)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_HONENOKO_SEND_GROUP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        SWITCH(LW(10))
            CASE_EQUAL(0)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 3, PTR("KRN_H_4"))
            CASE_EQUAL(1)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 3, PTR("KRN_H_5"))
            CASE_EQUAL(2)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 3, PTR("KRN_H_5"))
            CASE_EQUAL(3)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 3, PTR("KRN_H_3"))
            CASE_EQUAL(4)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 3, PTR("KRN_H_3"))
            CASE_ETC()
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 3, PTR("KRN_H_2"))
        END_SWITCH()
        USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 3, 0x300'0000)
        USER_FUNC(btlevtcmd_GetPartsPos, -2, 1, LW(4), LW(5), LW(6))
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(4), 15)
        ADD(LW(5), 14)
        USER_FUNC(btlevtcmd_SetPartsPos, -2, 3, LW(4), LW(5), LW(6))
        BROTHER_EVT()
            USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BoneRotation, 0)
            SET(LW(15), 0)
            DO(0)
                USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BoneRotation, LW(14))
                IF_EQUAL(LW(14), 1)
                    DO_BREAK()
                END_IF()
                USER_FUNC(btlevtcmd_SetPartsRotate, -2, 3, 0, 0, LW(15))
                ADD(LW(15), 25)
                WAIT_FRM(1)
            WHILE()
        END_BROTHER()
        USER_FUNC(btlevtcmd_JumpPartsSetting, -2, 3, 0, FLOAT(6.0), FLOAT(1.0))
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 3, LW(0), LW(1), LW(2), 20, 0)
        USER_FUNC(btlevtcmd_GetPartsPos, -2, 3, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_SetPartsPos, -2, 4, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_HONENOKO_SEND_GROUP2"), EVT_NULLPTR, 0, EVT_NULLPTR)
        SWITCH(LW(10))
            CASE_EQUAL(0)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 4, PTR("KRN_C_5"))
                USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 4, 0x300'0000)
            CASE_EQUAL(1)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 4, PTR("KRN_C_4"))
            CASE_EQUAL(2)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 4, PTR("KRN_C_3"))
            CASE_EQUAL(3)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 4, PTR("KRN_C_2"))
            CASE_EQUAL(4)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 4, PTR("KRN_C_1"))
            CASE_ETC()
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 4, PTR("KRN_Z_2"))
        END_SWITCH()
        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 3, 0x300'0000)
        ADD(LW(10), 1)
    WHILE()
    WAIT_FRM(15)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 4, PTR("KRN_D_3"))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_HONENOKO_UNITE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    WAIT_FRM(60)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 4, PTR("KRN_D_4"))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_HONENOKO_UNITE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KRN_S_1"))
    USER_FUNC(btlevtcmd_SpawnUnit, LW(3), PTR(&unitDullBones_spawn_entry), 0)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 4, 0x300'0000)
    USER_FUNC(btlevtcmd_SetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetHomePos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(unitBones_copy_status, -2, LW(3))
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitDryBones_build_event)
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, -1)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    SET(LW(10), 0)
    DO(6)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KRN_A_3"))
        WAIT_FRM(5)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KRN_A_4"))
        WAIT_FRM(15)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_HONENOKO_SEND_GROUP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        SWITCH(LW(10))
            CASE_EQUAL(0)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 5, PTR("KRN_H_4"))
            CASE_EQUAL(1)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 5, PTR("KRN_H_5"))
            CASE_EQUAL(2)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 5, PTR("KRN_H_5"))
            CASE_EQUAL(3)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 5, PTR("KRN_H_3"))
            CASE_EQUAL(4)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 5, PTR("KRN_H_3"))
            CASE_ETC()
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 5, PTR("KRN_H_2"))
        END_SWITCH()
        USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 5, 0x300'0000)
        USER_FUNC(btlevtcmd_GetPartsPos, -2, 1, LW(4), LW(5), LW(6))
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(4), 15)
        ADD(LW(5), 14)
        USER_FUNC(btlevtcmd_SetPartsPos, -2, 5, LW(4), LW(5), LW(6))
        BROTHER_EVT()
            USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BoneRotation, 0)
            SET(LW(15), 0)
            DO(0)
                USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BoneRotation, LW(14))
                IF_EQUAL(LW(14), 1)
                    DO_BREAK()
                END_IF()
                USER_FUNC(btlevtcmd_SetPartsRotate, -2, 5, 0, 0, LW(15))
                ADD(LW(15), 25)
                WAIT_FRM(1)
            WHILE()
        END_BROTHER()
        USER_FUNC(btlevtcmd_JumpPartsSetting, -2, 5, 0, FLOAT(6.0), FLOAT(1.0))
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 5, LW(0), LW(1), LW(2), 20, 0)
        USER_FUNC(btlevtcmd_GetPartsPos, -2, 5, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_SetPartsPos, -2, 6, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_HONENOKO_SEND_GROUP2"), EVT_NULLPTR, 0, EVT_NULLPTR)
        SWITCH(LW(10))
            CASE_EQUAL(0)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 6, PTR("KRN_C_5"))
                USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 6, 0x300'0000)
            CASE_EQUAL(1)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 6, PTR("KRN_C_4"))
            CASE_EQUAL(2)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 6, PTR("KRN_C_3"))
            CASE_EQUAL(3)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 6, PTR("KRN_C_2"))
            CASE_EQUAL(4)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 6, PTR("KRN_C_1"))
            CASE_ETC()
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 6, PTR("KRN_Z_2"))
        END_SWITCH()
        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 5, 0x300'0000)
        ADD(LW(10), 1)
    WHILE()
    WAIT_FRM(15)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 6, PTR("KRN_D_3"))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_HONENOKO_UNITE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    WAIT_FRM(60)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 6, PTR("KRN_D_4"))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_HONENOKO_UNITE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KRN_S_1"))
    USER_FUNC(btlevtcmd_SpawnUnit, LW(3), PTR(&unitDryBones_spawn_entry), 0)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 6, 0x300'0000)
    USER_FUNC(btlevtcmd_SetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetHomePos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(unitBones_copy_status, -2, LW(3))
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitBones_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    RETURN()
EVT_END()

EVT_BEGIN(unitBones_dead_event)
    SET(LW(10), -2)
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
    USER_FUNC(evt_eff, 0, PTR("kemuri_test"), 4, LW(0), LW(1), LW(2), FLOAT(0.8), 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_HasGivenRewards, LW(0))
    IF_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_HasGivenRewards, 1)
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
    USER_FUNC(btlevtcmd_GetBackItem, LW(10))
    USER_FUNC(btlevtcmd_CheckDamageCode, LW(10), 1024, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_KillUnit, LW(10), 0)
    ELSE()
        USER_FUNC(btlevtcmd_KillUnit, LW(10), 2)
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitBones_fake_death_event)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(2), LW(3), LW(4))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(4.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(2), LW(3), LW(4), 0, -1, 0)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_HasGivenRewards, LW(0))
    IF_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_HasGivenRewards, 1)
        SET(LW(10), -2)
        USER_FUNC(btlevtcmd_snd_se, LW(10), PTR("SFX_BTL_ENEMY_DIE1_2"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_GetPos, LW(10), LW(0), LW(1), LW(2))
        ADD(LW(2), 10)
        USER_FUNC(evt_eff, 0, PTR("kemuri_test"), 4, LW(0), LW(1), LW(2), FLOAT(0.8), 0, 0, 0, 0, 0, 0, 0)
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
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KRN_D_2"))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_HONENOKO_CRUMBLE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    WAIT_MSEC(1000)
    USER_FUNC(btlevtcmd_KillUnit, -2, 2)
    USER_FUNC(evt_sub_random, 2, LW(0))
    ADD(LW(0), 2)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_ReviveTurnCount, LW(0))
    USER_FUNC(btlevtcmd_SetHitOffset, LW(10), LW(11), 0, -20, 0)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitBones_normal_attack_event)
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        WAIT_MSEC(750)
        USER_FUNC(btlevtcmd_GetFirstAttackTarget, LW(3), LW(4))
    ELSE()
        USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
        USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), PTR(&unitDryBones_weaponNormal))
        USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, PTR(&unitDryBones_weaponNormal), LW(3), LW(4))
    END_IF()
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            RETURN()
        END_IF()
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_WeaponAftereffect, PTR(&unitDryBones_weaponNormal))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, PTR(&unitDryBones_weaponNormal))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    WAIT_MSEC(500)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KRN_A_1"))
    WAIT_MSEC(250)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KRN_A_2"))
    WAIT_MSEC(250)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_HONENOKO_SEND_GROUP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_HONENOKO_BONE_THROW1"), EVT_NULLPTR, 0, LW(15))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 2, PTR("KRN_H_1"))
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 2, 0x100'0000)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 2, 0x200'0000)
    USER_FUNC(btlevtcmd_GetPartsPos, -2, 1, LW(0), LW(1), LW(2))
    
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BoneRotation, 0)
        DO(0)
            USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BoneRotation, LW(14))
            IF_EQUAL(LW(14), 1)
                DO_BREAK()
            END_IF()
            USER_FUNC(btlevtcmd_AddPartsRotate, -2, 2, 0, 0, 25)
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(10))
    SWITCH(LW(10))
        CASE_OR((int32_t)BattleUnitType::DULL_BONES)
        CASE_OR((int32_t)BattleUnitType::RED_BONES)
            SETF(LW(10), FLOAT(6.0))
            CASE_END()
        CASE_ETC()
            SETF(LW(10), FLOAT(8.0))
    END_SWITCH()

    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), PTR(&unitDryBones_weaponNormal), 256, LW(5))
    SWITCH(LW(5))
        CASE_OR(4)
            GOTO(90)
            CASE_END()
        CASE_EQUAL(3)
            GOTO(90)
            CASE_END()
        CASE_EQUAL(6)
            GOTO(90)
        CASE_EQUAL(2)
            GOTO(90)
        CASE_EQUAL(1)
            GOTO(91)
            CASE_END()
        CASE_ETC()
            GOTO(98)
            CASE_END()
    END_SWITCH()
LBL(90)
    BROTHER_EVT()
        DO(0)
            USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(0))
            IF_EQUAL(LW(0), -1)
                USER_FUNC(btlevtcmd_GetPartsPos, -2, 2, LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(6), LW(7), LW(8))
                IF_SMALL_EQUAL(LW(0), LW(6))
                    RUN_CHILD_EVT(PTR(&unitBones_bone_miss_event))
                    DO_BREAK()
                END_IF()
            ELSE()
                USER_FUNC(btlevtcmd_GetPartsPos, -2, 2, LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(6), LW(7), LW(8))
                IF_LARGE_EQUAL(LW(0), LW(6))
                    RUN_CHILD_EVT(PTR(&unitBones_bone_miss_event))
                    DO_BREAK()
                END_IF()
            END_IF()
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 15)
    ADD(LW(1), 14)
    USER_FUNC(btlevtcmd_SetPartsPos, -2, 2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetStageSize, LW(0), EVT_NULLPTR, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(9))
    MUL(LW(0), LW(9))
    USER_FUNC(btlevtcmd_JumpPartsSetting, -2, 2, 0, LW(10), FLOAT(0.05))
    USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 2, LW(0), LW(1), LW(2), 0, 0)
    USER_FUNC(evt_snd_sfxoff, LW(15))
    GOTO(98)
LBL(91)
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 15)
    ADD(LW(1), 14)
    USER_FUNC(btlevtcmd_SetPartsPos, -2, 2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPartsSetting, -2, 2, 0, LW(10), FLOAT(0.2))
    USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 2, LW(0), LW(1), LW(2), 0, 0)
    USER_FUNC(evt_snd_sfxoff, LW(15))
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), PTR(&unitDryBones_weaponNormal))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&unitDryBones_weaponNormal), 256, LW(5))
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), -50)
    USER_FUNC(btlevtcmd_JumpPartsSetting, -2, 2, 0, FLOAT(2.0), FLOAT(0.3))
    USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 2, LW(0), LW(1), LW(2), 0, 0)
    GOTO(98)
LBL(98)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BoneRotation, 1)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 2, 0x100'0000)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 2, 0x200'0000)
    WAIT_FRM(10)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitBones_rebirth_event)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_ReviveTurnCount, LW(0))
    SUB(LW(0), 1)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_ReviveTurnCount, LW(0))
    IF_LARGE_EQUAL(LW(0), 1)
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KRN_D_3"))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_HONENOKO_UNITE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    WAIT_FRM(60)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KRN_D_4"))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_HONENOKO_UNITE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_OffAttribute, -2, 0x2'0000)
    USER_FUNC(btlevtcmd_GetMaxHp, -2, LW(0))
    USER_FUNC(btlevtcmd_SetHp, -2, LW(0))
    USER_FUNC(btlevtcmd_SetHitOffset, -2, 1, 0, 0, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitBones_bone_miss_event)
    IF_EQUAL(LW(5), 3)
        USER_FUNC(btlevtcmd_StartAvoid, LW(3), 38)
    END_IF()
    IF_EQUAL(LW(5), 6)
        USER_FUNC(btlevtcmd_StartAvoid, LW(3), 39)
    END_IF()
    IF_EQUAL(LW(5), 2)
        USER_FUNC(btlevtcmd_StartAvoid, LW(3), 40)
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitBones_barrage_bone_miss_event)
    IF_EQUAL(LW(11), 3)
        USER_FUNC(btlevtcmd_StartAvoid, LW(3), 38)
    END_IF()
    IF_EQUAL(LW(11), 6)
        USER_FUNC(btlevtcmd_StartAvoid, LW(3), 39)
    END_IF()
    IF_EQUAL(LW(11), 2)
        USER_FUNC(btlevtcmd_StartAvoid, LW(3), 40)
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitBones_wait_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitBones_barrage_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BarrageLanded, 0)
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), PTR(&unitDryBones_weaponBarrage))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, PTR(&unitDryBones_weaponBarrage), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            RETURN()
        END_IF()
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BarrageCount, 0)
    USER_FUNC(btlevtcmd_WeaponAftereffect, PTR(&unitDryBones_weaponBarrage))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, PTR(&unitDryBones_weaponBarrage))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    SET(LW(12), 2)
    RUN_EVT(PTR(&unitBones_barrage_event_sub))
    WAIT_FRM(40)
    SET(LW(12), 3)
    RUN_EVT(PTR(&unitBones_barrage_event_sub))
    WAIT_FRM(35)
    SET(LW(12), 4)
    RUN_EVT(PTR(&unitBones_barrage_event_sub))
    DO(0)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BarrageCount, LW(0))
        IF_EQUAL(LW(0), -1)
            DO_BREAK()
        END_IF()
        WAIT_FRM(1)
    WHILE()
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    WAIT_MSEC(300)
    RETURN()
EVT_END()

EVT_BEGIN(unitBones_barrage_event_sub)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KRN_A_1"))
    WAIT_MSEC(250)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KRN_A_2"))
    WAIT_MSEC(250)
    USER_FUNC(btlevtcmd_AddUnitWork, -2, UW_BarrageCount, 1)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BarrageCount, LW(13))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_HONENOKO_SEND_GROUP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_HONENOKO_BONE_THROW3"), EVT_NULLPTR, 0, LW(15))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, LW(12), PTR("KRN_H_1"))
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, LW(12), 0x100'0000)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, LW(12), 0x200'0000)
    USER_FUNC(btlevtcmd_GetPartsPos, -2, 1, LW(0), LW(1), LW(2))
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BoneRotation, 0)
        DO(0)
            USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BoneRotation, LW(14))
            IF_EQUAL(LW(14), 1)
                DO_BREAK()
            END_IF()
            USER_FUNC(btlevtcmd_AddPartsRotate, -2, LW(12), 0, 0, 25)
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()
    IF_EQUAL(LW(13), 3)
        USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), PTR(&unitDryBones_weaponBarrage), 256, LW(11))
    ELSE()
        USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), PTR(&unitDryBones_weaponBarrage), 256, LW(11))
    END_IF()
    IF_EQUAL(LW(13), 3)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BarrageLanded, LW(14))
        IF_EQUAL(LW(14), 1)
            GOTO(91)
        END_IF()
    END_IF()
    SWITCH(LW(11))
        CASE_OR(4)
            GOTO(90)
            CASE_END()
        CASE_EQUAL(3)
            GOTO(90)
            CASE_END()
        CASE_EQUAL(6)
            GOTO(90)
        CASE_EQUAL(2)
            GOTO(90)
        CASE_EQUAL(1)
            GOTO(91)
            CASE_END()
        CASE_ETC()
            GOTO(98)
            CASE_END()
    END_SWITCH()
LBL(90)
    BROTHER_EVT()
        DO(0)
            USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(0))
            IF_EQUAL(LW(0), -1)
                USER_FUNC(btlevtcmd_GetPartsPos, -2, LW(12), LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(6), LW(7), LW(8))
                IF_SMALL_EQUAL(LW(0), LW(6))
                    RUN_CHILD_EVT(PTR(&unitBones_barrage_bone_miss_event))
                    DO_BREAK()
                END_IF()
            ELSE()
                USER_FUNC(btlevtcmd_GetPartsPos, -2, LW(12), LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(6), LW(7), LW(8))
                IF_LARGE_EQUAL(LW(0), LW(6))
                    RUN_CHILD_EVT(PTR(&unitBones_barrage_bone_miss_event))
                    DO_BREAK()
                END_IF()
            END_IF()
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 15)
    ADD(LW(1), 14)
    USER_FUNC(btlevtcmd_SetPartsPos, -2, LW(12), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetStageSize, LW(0), EVT_NULLPTR, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(9))
    MUL(LW(0), LW(9))
    USER_FUNC(btlevtcmd_JumpPartsSetting, -2, LW(12), 0, FLOAT(8.0), FLOAT(0.05))
    USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(12), LW(0), LW(1), LW(2), 0, 0)
    USER_FUNC(evt_snd_sfxoff, LW(15))
    GOTO(98)
LBL(91)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BarrageLanded, 1)
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 15)
    ADD(LW(1), 14)
    USER_FUNC(btlevtcmd_SetPartsPos, -2, LW(12), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPartsSetting, -2, LW(12), 0, FLOAT(8.0), FLOAT(0.2))
    USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(12), LW(0), LW(1), LW(2), 0, 0)
    USER_FUNC(evt_snd_sfxoff, LW(15))
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), PTR(&unitDryBones_weaponBarrage))
    IF_EQUAL(LW(13), 3)
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&unitDryBones_weaponBarrage), 256, LW(5))
    ELSE()
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&unitDryBones_weaponBarrage), 0, LW(5))
    END_IF()
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), -50)
    USER_FUNC(btlevtcmd_JumpPartsSetting, -2, LW(12), 0, FLOAT(2.0), FLOAT(0.3))
    USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(12), LW(0), LW(1), LW(2), 0, 0)
    GOTO(98)
LBL(98)
    IF_EQUAL(LW(13), 3)
        USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BoneRotation, 1)
        USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BarrageCount, -1)
    END_IF()
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, LW(12), 0x100'0000)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, LW(12), 0x200'0000)
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitBones_common_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitBones_wait_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitBones_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitBones_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitBones_attack_event))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_HasGivenRewards, 0)
    USER_FUNC(btlevtcmd_SetJumpSound, -2, PTR("0"), PTR("SFX_ENM_HONENOKO_LANDING1"))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitDullBones_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::DULL_BONES)
    RUN_CHILD_EVT(unitBones_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitRedBones_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::RED_BONES)
    RUN_CHILD_EVT(unitBones_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitDryBones_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::DRY_BONES)
    RUN_CHILD_EVT(unitBones_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitDarkBones_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::DARK_BONES)
    RUN_CHILD_EVT(unitBones_common_init_event)
    RETURN()
EVT_END()

}  // namespace mod::tot::custom