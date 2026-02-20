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
constexpr const int32_t UW_IsGuarding = 0;
constexpr const int32_t UW_Rotating = 1;
constexpr const int32_t UW_RotateSpd = 2;
constexpr const int32_t UW_BattleUnitType = 4;

}  // namespace

// Evt / Function declarations.

extern const int32_t unitKoopatrol_init_event[];
extern const int32_t unitDarkKoopatrol_init_event[];
extern const int32_t unitKoopatrol_common_init_event[];
extern const int32_t unitKoopatrol_attack_event[];
extern const int32_t unitKoopatrol_damage_event[];
extern const int32_t unitKoopatrol_wait_event[];
extern const int32_t unitKoopatrol_normal_attack_event[];
extern const int32_t unitKoopatrol_power_attack_event[];
extern const int32_t unitKoopatrol_charge_event[];
extern const int32_t unitKoopatrol_guard_event[];
extern const int32_t unitKoopatrol_guard_cancel_event[];
extern const int32_t unitKoopatrol_call_backup_event[];
extern const int32_t unitKoopatrol_spiky_counter_event[];
extern const int32_t unitKoopatrol_flip_event[];
extern const int32_t unitKoopatrol_wakeup_event[];

EVT_DECLARE_USER_FUNC(unitKoopatrol_GetAlliesCount, 1)

// Unit data.

int8_t unitKoopatrol_defense[] = { 2, 2, 2, 2, 2 };
int8_t unitKoopatrol_defense_attr[] = { 0, 0, 0, 0, 0 };
int8_t unitKoopatrol_flip_defense[] = { 0, 0, 0, 0, 0 };
int8_t unitKoopatrol_flip_defense_attr[] = { 0, 0, 0, 0, 0 };
int8_t unitKoopatrol_guard_defense[] = { 99, 99, 99, 99, 99 };
int8_t unitKoopatrol_guard_defense_attr[] = { 2, 2, 2, 2, 2 };

StatusVulnerability unitKoopatrol_status = {
     70,  75,  75, 100,  75, 100, 100,  70,
    100,  90, 100,  90, 100,  95,  80,  70,
     70, 100,  80, 100, 100,  95,
};
StatusVulnerability unitDarkKoopatrol_status = {
     60,  65,  65, 100,  65, 100, 100,  60,
    100,  85, 100,  85, 100,  90,  70,  60,
     60, 100,  70, 100, 100,  80,
};
StatusVulnerability unitKoopatrol_flip_status = {
    100, 100, 100, 100, 100, 100, 100, 100,
    100, 100, 100, 100, 100, 100, 100, 100,
    100, 100, 100, 100, 100, 100,
};
StatusVulnerability unitKoopatrol_guard_status = {
    100, 100, 100, 100, 100, 100, 100, 100,
    100, 100, 100, 100, 100, 100, 100, 100,
    100, 100, 100, 100, 100, 100,
};

PoseTableEntry unitKoopatrol_pose_table[] = {
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
    42, "R_1",
    40, "W_1",
    56, "X_1",
    57, "X_1",
    65, "T_1",
    69, "S_1",
};
PoseTableEntry unitKoopatrol_flip_pose_table[] = {
    1, "N_2",
    2, "Z_2",
    9, "Z_2",
    5, "K_2",
    4, "S_2",
    3, "S_2",
    28, "S_2",
    29, "Q_1",
    30, "Q_1",
    31, "D_1",
    39, "D_1",
    56, "S_2",
    57, "S_2",
    65, "S_2",
    69, "S_2",
};
PoseTableEntry unitKoopatrol_guard_pose_table[] = {
    1, "A_1",
    2, "A_1",
    9, "A_1",
    5, "A_1",
    4, "A_1",
    3, "A_1",
    28, "A_1",
    29, "A_1",
    30, "A_1",
    31, "A_1",
    39, "A_1",
    56, "A_1",
    57, "A_1",
    65, "A_1",
    69, "A_1",
};
PoseTableEntry unitKoopatrol_flip_guard_pose_table[] = {
    1, "G_1",
    2, "G_1",
    9, "G_1",
    5, "G_1",
    4, "G_1",
    3, "G_1",
    28, "G_1",
    29, "G_1",
    30, "G_1",
    31, "G_1",
    39, "G_1",
    56, "G_1",
    57, "G_1",
    65, "G_1",
    69, "G_1",
};

DataTableEntry unitKoopatrol_data_table[] = {
    13, (void*)unitKoopatrol_flip_event,
    37, (void*)unitKoopatrol_spiky_counter_event,
    48, (void*)btldefaultevt_Dummy,
    0, nullptr,
};
DataTableEntry unitKoopatrol_flip_data_table[] = {
    13, (void*)unitKoopatrol_flip_event,
    48, (void*)btldefaultevt_Dummy,
    0, nullptr,
};
DataTableEntry unitKoopatrol_guard_data_table[] = {
    13, (void*)unitKoopatrol_flip_event,
    37, (void*)unitKoopatrol_spiky_counter_event,
    48, (void*)btldefaultevt_Dummy,
    0, nullptr,
};

BattleWeapon unitKoopatrol_weaponNormal = {
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
    .damage_function_params = { 4, 0, 0, 0, 0, 0, 0, 0 },
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
    .damage_pattern = 8,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::FREEZE_BREAK,
    .counter_resistance_flags =
        AttackCounterResistance_Flags::TOP_SPIKY |
        // Added to make sure that front-spiky counter functions properly.
        AttackCounterResistance_Flags::PREEMPTIVE_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
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

BattleWeapon unitKoopatrol_weaponPowerShell = {
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
    .damage_function_params = { 4, 0, 0, 0, 0, 0, 0, 0 },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    
    .target_class_flags =
        AttackTargetClass_Flags::MULTIPLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH |
        AttackTargetClass_Flags::ENEMY_SELECT_SIDE_HOME,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::SHELL_TOSS_LIKE |
        AttackTargetProperty_Flags::CANNOT_TARGET_CEILING,
    .element = AttackElement::NORMAL,
    .damage_pattern = 8,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::FREEZE_BREAK,
    .counter_resistance_flags =
        AttackCounterResistance_Flags::ALL &
        ~AttackCounterResistance_Flags::PAYBACK,
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
BattleWeapon unitKoopatrol_weaponCharge = {
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
        AttackTargetClass_Flags::ONLY_TARGET_SELF |
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
    .charge_strength = 2,
    
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
BattleWeapon unitDarkKoopatrol_weaponNormal = {
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
    .damage_pattern = 8,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::FREEZE_BREAK,
    .counter_resistance_flags =
        AttackCounterResistance_Flags::TOP_SPIKY |
        // Added to make sure that front-spiky counter functions properly.
        AttackCounterResistance_Flags::PREEMPTIVE_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
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
BattleWeapon unitDarkKoopatrol_weaponPowerShell = {
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
        AttackTargetClass_Flags::MULTIPLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH |
        AttackTargetClass_Flags::ENEMY_SELECT_SIDE_HOME,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::SHELL_TOSS_LIKE |
        AttackTargetProperty_Flags::CANNOT_TARGET_CEILING,
    .element = AttackElement::NORMAL,
    .damage_pattern = 8,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::FREEZE_BREAK,
    .counter_resistance_flags =
        AttackCounterResistance_Flags::ALL &
        ~AttackCounterResistance_Flags::PAYBACK,
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
BattleWeapon unitDarkKoopatrol_weaponCharge = {
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
        AttackTargetClass_Flags::ONLY_TARGET_SELF |
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
    .charge_strength = 5,
    
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

BattleUnitKindPart unitKoopatrol_parts[] = {
    {
        .index = 1,
        .name = "btl_un_togenoko",
        .model_name = "c_togenoko",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 40.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 50.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitKoopatrol_defense,
        .defense_attr = unitKoopatrol_defense_attr,
        .attribute_flags = 0x0000'0009 | PartsAttribute_Flags::SHELLED,
        .counter_attribute_flags = PartsCounterAttribute_Flags::TOP_SPIKY,
        .pose_table = unitKoopatrol_pose_table,
    },
};
BattleUnitKindPart unitDarkKoopatrol_parts[] = {
    {
        .index = 1,
        .name = "btl_un_togenoko_ace",
        .model_name = "c_togenoko_b",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 40.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 50.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitKoopatrol_defense,
        .defense_attr = unitKoopatrol_defense_attr,
        .attribute_flags = 0x0000'0009 | PartsAttribute_Flags::SHELLED,
        .counter_attribute_flags = PartsCounterAttribute_Flags::TOP_SPIKY,
        .pose_table = unitKoopatrol_pose_table,
    },
};

BattleUnitKind unit_Koopatrol = {
    .unit_type = BattleUnitType::KOOPATROL,
    .unit_name = "btl_un_togenoko",
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
    .height = 50,
    .hit_offset = { -4, 50 },
    .center_offset = { 0.0f, 25.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 18.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 18.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 13.0f, 32.0f, 0.0f },
    .cut_base_offset = { 0.0f, 25.0f, 0.0f },
    .cut_width = 36.0f,
    .cut_height = 50.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_TOGENOKO_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitKoopatrol_status,
    .num_parts = 1,
    .parts = unitKoopatrol_parts,
    .init_evt_code = (void*)unitKoopatrol_init_event,
    .data_table = unitKoopatrol_data_table,
};
BattleUnitKind unit_DarkKoopatrol = {
    .unit_type = BattleUnitType::DARK_KOOPATROL,
    .unit_name = "btl_un_togenoko_ace",
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
    .height = 50,
    .hit_offset = { -4, 50 },
    .center_offset = { 0.0f, 25.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 18.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 18.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 13.0f, 32.0f, 0.0f },
    .cut_base_offset = { 0.0f, 25.0f, 0.0f },
    .cut_width = 36.0f,
    .cut_height = 50.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_TOGENOKO_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitDarkKoopatrol_status,
    .num_parts = 1,
    .parts = unitDarkKoopatrol_parts,
    .init_evt_code = (void*)unitDarkKoopatrol_init_event,
    .data_table = unitKoopatrol_data_table,
};

const BattleUnitSetup unitKoopatrol_spawn_entry = {
    .unit_kind_params = &unit_Koopatrol,
    .alliance = 1,
    .attack_phase = 0x04000004,
    .position = { -1000.0f, -1000.0f, -1000.0f },
    .addl_target_offset_x = 0,
    .unit_work = { 0, 0, 0, 0 },
    .item_drop_table = nullptr,
};
const BattleUnitSetup unitDarkKoopatrol_spawn_entry = {
    .unit_kind_params = &unit_DarkKoopatrol,
    .alliance = 1,
    .attack_phase = 0x04000004,
    .position = { -1000.0f, -1000.0f, -1000.0f },
    .addl_target_offset_x = 0,
    .unit_work = { 0, 0, 0, 0 },
    .item_drop_table = nullptr,
};

// Function / USER_FUNC definitions.

EVT_DEFINE_USER_FUNC(unitKoopatrol_GetAlliesCount) {
    int32_t count = 0;
    for (int32_t i = 0; i < 64; ++i) {
        auto* unit = g_BattleWork->battle_units[i];
        if (unit && unit->alliance == 1) ++count;        
    }
    evtSetValue(evt, evt->evtArguments[0], count);
    return 2;
}

// Evt definitions.

EVT_BEGIN(unitKoopatrol_attack_event)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(15))

    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        IF_EQUAL(LW(15), (int32_t)BattleUnitType::KOOPATROL)
            SET(LW(9), PTR(&unitKoopatrol_weaponNormal))
        ELSE()
            SET(LW(9), PTR(&unitDarkKoopatrol_weaponNormal))
        END_IF()
        RUN_CHILD_EVT(PTR(&unitKoopatrol_normal_attack_event))
        GOTO(99)
    END_IF()

    USER_FUNC(btlevtcmd_GetOverTurnCount, -2, LW(0))
    IF_LARGE_EQUAL(LW(0), 1)
        RUN_CHILD_EVT(PTR(&unitKoopatrol_wakeup_event))
        RETURN()
    END_IF()

    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_IsGuarding, LW(0))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(btlevtcmd_DrawLots, LW(0), 2, 80, 20)
        IF_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&unitKoopatrol_guard_cancel_event))
            GOTO(99)
        END_IF()
        GOTO(99)
    END_IF()

    USER_FUNC(btlevtcmd_CheckStatus, -2, StatusEffectType::CHARGE, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        IF_EQUAL(LW(15), (int32_t)BattleUnitType::KOOPATROL)
            USER_FUNC(btlevtcmd_DrawLots, LW(0), 2, 25, 25)
            SET(LW(10), PTR(&unitKoopatrol_weaponNormal))
            SET(LW(11), PTR(&unitKoopatrol_weaponPowerShell))
        ELSE()
            USER_FUNC(btlevtcmd_DrawLots, LW(0), 2, 50, 35)
            SET(LW(10), PTR(&unitDarkKoopatrol_weaponNormal))
            SET(LW(11), PTR(&unitDarkKoopatrol_weaponPowerShell))
        END_IF()

        IF_EQUAL(LW(0), 0)
            SET(LW(9), LW(10))
            RUN_CHILD_EVT(PTR(&unitKoopatrol_normal_attack_event))
        ELSE()
            SET(LW(9), LW(11))
            RUN_CHILD_EVT(PTR(&unitKoopatrol_power_attack_event))
        END_IF()
        GOTO(99)
    END_IF()

    USER_FUNC(btlevtcmd_EnemyItemUseCheck, -2, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(LW(0))
        USER_FUNC(btlevtcmd_StartWaitEvent, -2)
        RETURN()
    END_IF()

    // Skip checking for calling allies for Dark Koopatrols.
    IF_EQUAL(LW(15), (int32_t)BattleUnitType::DARK_KOOPATROL)
        GOTO(11)
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
                GOTO(10)
            END_IF()
        ELSE()
            GOTO(9)
        END_IF()
    WHILE()
LBL(9)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    DO(0)
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 40)
        IF_SMALL_EQUAL(LW(0), 130)
            ADD(LW(2), 10)
            USER_FUNC(btlevtcmd_CheckSpace, LW(3), LW(0), LW(1), LW(2), 40, 0, 0, 0)
            IF_EQUAL(LW(3), -1)
                GOTO(10)
            END_IF()
        ELSE()
            GOTO(11)
        END_IF()
    WHILE()

LBL(10)
    // Chance is higher of calling allies the fewer enemies are present.
    USER_FUNC(unitKoopatrol_GetAlliesCount, LW(10))
    USER_FUNC(evt_sub_random, 99, LW(11))
    SWITCH(LW(10))
        CASE_EQUAL(1)
            SET(LW(10), 40)
        CASE_EQUAL(2)
            SET(LW(10), 20)
        CASE_EQUAL(3)
            SET(LW(10), 10)
        CASE_ETC()
            SET(LW(10), 0)
    END_SWITCH()
    IF_SMALL(LW(11), LW(10))
        RUN_CHILD_EVT(PTR(&unitKoopatrol_call_backup_event))
        GOTO(99)
    END_IF()

LBL(11)
    // Can't spawn an ally, pick a normal attack or guard.
    IF_EQUAL(LW(15), (int32_t)BattleUnitType::KOOPATROL)
        USER_FUNC(btlevtcmd_DrawLots, LW(0), 4, 25, 25, 25, 15)
        SET(LW(10), PTR(&unitKoopatrol_weaponNormal))
        SET(LW(11), PTR(&unitKoopatrol_weaponPowerShell))
        SET(LW(12), PTR(&unitKoopatrol_weaponCharge))
    ELSE()
        USER_FUNC(btlevtcmd_DrawLots, LW(0), 4, 50, 35, 15, 0)
        SET(LW(10), PTR(&unitDarkKoopatrol_weaponNormal))
        SET(LW(11), PTR(&unitDarkKoopatrol_weaponPowerShell))
        SET(LW(12), PTR(&unitDarkKoopatrol_weaponCharge))
    END_IF()

    SWITCH(LW(0))
        CASE_EQUAL(1)
            SET(LW(9), LW(11))
            RUN_CHILD_EVT(PTR(&unitKoopatrol_power_attack_event))
        CASE_EQUAL(2)
            SET(LW(9), LW(12))
            RUN_CHILD_EVT(PTR(&unitKoopatrol_charge_event))
        CASE_EQUAL(3)
            RUN_CHILD_EVT(PTR(&unitKoopatrol_guard_event))
        CASE_ETC()
            SET(LW(9), LW(10))
            RUN_CHILD_EVT(PTR(&unitKoopatrol_normal_attack_event))
    END_SWITCH()

LBL(99)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitKoopatrol_charge_event)
    SET(LW(3), -2)
    SET(LW(4), 1)
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, -1)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 300)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGENOKO_CHARGE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    BROTHER_EVT()
        DO(8)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGENOKO_MOVE3"), EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_FRM(3)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGENOKO_MOVE4"), EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_FRM(3)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("R_3"))
    WAIT_FRM(48)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_1"))
    INLINE_EVT()
        WAIT_FRM(60)
        USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    END_INLINE()
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    RETURN()
EVT_END()

EVT_BEGIN(unitKoopatrol_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    RETURN()
EVT_END()

EVT_BEGIN(unitKoopatrol_spiky_counter_event)
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_CheckStatus, -2, 10, LW(3))
    IF_NOT_EQUAL(LW(3), 0)
        ADD(LW(1), 70)
        GOTO(90)
    END_IF()
    USER_FUNC(btlevtcmd_CheckStatus, -2, 11, LW(3))
    IF_NOT_EQUAL(LW(3), 0)
        ADD(LW(1), 40)
        GOTO(90)
    END_IF()
    ADD(LW(1), 55)
    GOTO(90)
LBL(90)
    USER_FUNC(evt_eff, PTR(""), PTR("toge_flush"), 0, LW(0), LW(1), LW(2), 60, 0, 0, 0, 0, 0, 0, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitKoopatrol_flip_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_SetHitOffset, LW(10), LW(11), 0, -20, 0)
    USER_FUNC(btlevtcmd_SetPartsDefenceTable, LW(10), LW(11), PTR(&unitKoopatrol_flip_defense))
    USER_FUNC(btlevtcmd_SetPartsDefenceAttrTable, LW(10), LW(11), PTR(&unitKoopatrol_flip_defense_attr))
    USER_FUNC(btlevtcmd_SetRegistStatus, LW(10), PTR(&unitKoopatrol_flip_status))
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_IsGuarding, LW(0))
    IF_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_AnimeSetPoseTable, LW(10), LW(11), PTR(&unitKoopatrol_flip_pose_table))
        USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(10), LW(11), 39)
    ELSE()
        USER_FUNC(btlevtcmd_AnimeSetPoseTable, LW(10), LW(11), PTR(&unitKoopatrol_flip_guard_pose_table))
        USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(10), LW(11), 69)
    END_IF()
    USER_FUNC(btlevtcmd_ChangeDataTable, LW(10), PTR(&unitKoopatrol_flip_data_table))
    USER_FUNC(btlevtcmd_OnStatusFlag, LW(10), 1)
    USER_FUNC(btlevtcmd_OffPartsCounterAttribute, LW(10), LW(11), 1)
    USER_FUNC(btlevtcmd_SetOverTurnCount, LW(10), 2)
    WAIT_FRM(2)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_INSIDE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetFallAccel, LW(10), FLOAT(0.7))
    USER_FUNC(btlevtcmd_GetPos, LW(10), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPosition, LW(10), LW(0), LW(1), LW(2), 15, -1)
    USER_FUNC(btlevtcmd_JumpPosition, LW(10), LW(0), LW(1), LW(2), 10, -1)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(10), LW(11), 69)
    RETURN()
EVT_END()

EVT_BEGIN(unitKoopatrol_call_backup_event)
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, -1)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGENOKO_CALL1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("B_1"))
    WAIT_FRM(40)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_1"))

    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(15))
    IF_EQUAL(LW(15), (int32_t)BattleUnitType::KOOPATROL)
        USER_FUNC(btlevtcmd_SpawnUnit, LW(3), PTR(&unitKoopatrol_spawn_entry), 0)
    ELSE()
        USER_FUNC(btlevtcmd_SpawnUnit, LW(3), PTR(&unitDarkKoopatrol_spawn_entry), 0)
    END_IF()

    USER_FUNC(btlevtcmd_SetPos, LW(3), 300, LW(1), LW(2))
    WAIT_FRM(10)
    USER_FUNC(btlevtcmd_AnimeChangePose, LW(3), 1, PTR("R_1"))
    USER_FUNC(btlevtcmd_SetMoveSpeed, LW(3), 8)
    USER_FUNC(btlevtcmd_MovePosition, LW(3), LW(0), LW(1), LW(2), 0, -1, 0)
    USER_FUNC(btlevtcmd_SetHomePos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_AnimeChangePose, LW(3), 1, PTR("S_1"))
    WAIT_MSEC(500)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    WAIT_MSEC(300)
    RETURN()
EVT_END()

EVT_BEGIN(unitKoopatrol_guard_cancel_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    BROTHER_EVT()
        WAIT_FRM(10)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGENOKO_SHELL2"), EVT_NULLPTR, 0, EVT_NULLPTR)
        WAIT_FRM(5)
        USER_FUNC(btlevtcmd_AnimeChangePose, LW(10), LW(11), PTR("S_1"))
    END_BROTHER()
    USER_FUNC(btlevtcmd_GetPos, LW(10), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpSetting, LW(10), 20, 0, FLOAT(0.5))
    USER_FUNC(btlevtcmd_JumpPosition, LW(10), LW(0), LW(1), LW(2), 20, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGENOKO_MOVE6"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetPartsDefenceTable, LW(10), LW(11), PTR(&unitKoopatrol_defense))
    USER_FUNC(btlevtcmd_SetPartsDefenceAttrTable, LW(10), LW(11), PTR(&unitKoopatrol_defense_attr))

    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(0))
    IF_EQUAL(LW(0), (int32_t)BattleUnitType::KOOPATROL)
        USER_FUNC(btlevtcmd_SetRegistStatus, LW(10), PTR(&unitKoopatrol_status))
    ELSE()
        USER_FUNC(btlevtcmd_SetRegistStatus, LW(10), PTR(&unitDarkKoopatrol_status))
    END_IF()

    USER_FUNC(btlevtcmd_AnimeSetPoseTable, LW(10), LW(11), PTR(&unitKoopatrol_pose_table))
    USER_FUNC(btlevtcmd_ChangeDataTable, LW(10), PTR(&unitKoopatrol_data_table))
    USER_FUNC(btlevtcmd_SetUnitWork, LW(10), 0, 0)
    USER_FUNC(btlevtcmd_SetHitOffset, LW(10), LW(11), 0, 0, 0)
    WAIT_MSEC(300)
    RETURN()
EVT_END()

EVT_BEGIN(unitKoopatrol_guard_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    BROTHER_EVT()
        WAIT_FRM(10)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGENOKO_SHELL1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        WAIT_FRM(5)
        USER_FUNC(btlevtcmd_AnimeChangePose, LW(10), LW(11), PTR("A_1"))
    END_BROTHER()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGENOKO_MOVE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetPos, LW(10), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpSetting, LW(10), 20, 0, FLOAT(0.5))
    USER_FUNC(btlevtcmd_JumpPosition, LW(10), LW(0), LW(1), LW(2), 20, -1)
    SETF(LW(12), FLOAT(30.0))
    SETF(LW(13), FLOAT(0.0))
    DO(15)
        USER_FUNC(evt_sub_get_sincos, LW(13), LW(14), LW(15))
        MULF(LW(14), LW(12))
        USER_FUNC(btlevtcmd_SetRotate, LW(10), 0, 0, LW(14))
        SUBF(LW(12), FLOAT(2.0))
        ADDF(LW(13), FLOAT(30.0))
        WAIT_FRM(1)
    WHILE()
    USER_FUNC(btlevtcmd_SetRotate, LW(10), 0, 0, 0)
    USER_FUNC(btlevtcmd_SetPartsDefenceTable, LW(10), LW(11), PTR(&unitKoopatrol_guard_defense))
    USER_FUNC(btlevtcmd_SetPartsDefenceAttrTable, LW(10), LW(11), PTR(&unitKoopatrol_guard_defense_attr))
    USER_FUNC(btlevtcmd_SetRegistStatus, LW(10), PTR(&unitKoopatrol_guard_status))
    USER_FUNC(btlevtcmd_AnimeSetPoseTable, LW(10), LW(11), PTR(&unitKoopatrol_guard_pose_table))
    USER_FUNC(btlevtcmd_ChangeDataTable, LW(10), PTR(&unitKoopatrol_guard_data_table))
    USER_FUNC(btlevtcmd_SetUnitWork, LW(10), 0, 1)
    USER_FUNC(btlevtcmd_SetHitOffset, LW(10), LW(11), 0, -20, 0)
    WAIT_MSEC(300)
    RETURN()
EVT_END()

EVT_BEGIN(unitKoopatrol_normal_attack_event)
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        WAIT_MSEC(750)
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
        GOTO(98)
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
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    BROTHER_EVT()
        DO(10)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGENOKO_MOVE3"), EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_FRM(3)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGENOKO_MOVE4"), EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_FRM(3)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("R_3"))
    WAIT_FRM(60)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGENOKO_MOVE5"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_2"))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 10)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(10.0))
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
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 500)
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), 0, LW(2), 0, -1, 0)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("R_1"))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, -1)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetPos, -2, 300, LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(10.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_1"))
    GOTO(98)
LBL(91)
    USER_FUNC(evt_btl_camera_shake_h, 0, 1, 1, 8, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_add_zoom, 0, -50)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_1"))
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(10), LW(11), LW(12))
    SUBF(LW(10), LW(0))
    SUBF(LW(11), LW(1))
    SUBF(LW(12), LW(2))
    MULF(LW(10), FLOAT(0.75))
    MULF(LW(11), FLOAT(0.75))
    MULF(LW(12), FLOAT(0.75))
    ADDF(LW(0), LW(10))
    ADDF(LW(1), LW(11))
    ADDF(LW(2), LW(12))
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, 0, FLOAT(0.2))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGENOKO_MOVE6"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, 0, FLOAT(0.2))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGENOKO_MOVE6"), EVT_NULLPTR, 0, EVT_NULLPTR)
    GOTO(98)
LBL(98)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    WAIT_MSEC(500)
    RETURN()
EVT_END()

EVT_BEGIN(unitKoopatrol_power_attack_event)
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
    USER_FUNC(evt_btl_camera_set_mode, 0, 8)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, LW(3))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    WAIT_MSEC(100)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGENOKO_MOVE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.0), FLOAT(0.7))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 0, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGENOKO_SHELL1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_1"))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGENOKO_MOVE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Rotating, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_RotateSpd, 10)
    BROTHER_EVT()
        DO(0)
            USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Rotating, LW(15))
            IF_EQUAL(LW(15), 1)
                USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
                DO_BREAK()
            END_IF()
            WAIT_FRM(1)
            USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_RotateSpd, LW(14))
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, LW(14), 0)
        WHILE()
    END_BROTHER()
    WAIT_MSEC(200)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_RotateSpd, 20)
    WAIT_MSEC(200)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_RotateSpd, 40)
    WAIT_MSEC(200)
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    BROTHER_EVT_ID(LW(13))
        DO(0)
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
            ADD(LW(1), 10)
            USER_FUNC(evt_eff, PTR(""), PTR("kemuri_test"), 0, LW(0), LW(1), LW(2), FLOAT(0.3), 0, 0, 0, 0, 0, 0, 0)
            WAIT_FRM(3)
        WHILE()
    END_BROTHER()
    
    BROTHER_EVT_ID(LW(11))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 500)
        USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(12.0))
        USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, 0, 0)
        WAIT_MSEC(500)
    END_BROTHER()

    // Only check for guard result at most once.
    SET(LW(6), 0)

LBL(10)
    // Process each hit only once the Koopatrol passes by.
    DO(0)
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), EVT_NULLPTR, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(1), EVT_NULLPTR, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(2))
        MUL(LW(0), LW(2))
        MUL(LW(1), LW(2))
        IF_LARGE_EQUAL(LW(1), LW(0))
            DO_BREAK()
        END_IF()
        WAIT_FRM(1)
    WHILE()

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
    GOTO(97)
LBL(91)
    IF_EQUAL(LW(6), 0)
        USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
        SET(LW(6), 1)
    END_IF()
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    GOTO(97)
LBL(97)
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    IF_NOT_EQUAL(LW(3), -1)
        GOTO(10)
    END_IF()

    // Wait for the above move event to end.
    DO(0)
        CHK_EVT(LW(11), LW(0))
        IF_EQUAL(LW(0), 0)
            DO_BREAK()
        END_IF()
        WAIT_FRM(1)
    WHILE()

LBL(98)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetPos, -2, 250, LW(1), LW(2))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGENOKO_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(12.0))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, 0, 0)
    DELETE_EVT(LW(13))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGENOKO_SHELL2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGENOKO_ATTACK2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(10.0), FLOAT(0.4))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, -1)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Rotating, 1)
    WAIT_FRM(3)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitKoopatrol_wait_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitKoopatrol_wakeup_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_GetOverTurnCount, LW(10), LW(0))
    SUB(LW(0), 1)
    USER_FUNC(btlevtcmd_SetOverTurnCount, LW(10), LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_StartWaitEvent, LW(10))
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_SetPartsDefenceTable, LW(10), LW(11), PTR(&unitKoopatrol_defense))
    USER_FUNC(btlevtcmd_SetPartsDefenceAttrTable, LW(10), LW(11), PTR(&unitKoopatrol_defense_attr))

    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(0))
    IF_EQUAL(LW(0), (int32_t)BattleUnitType::KOOPATROL)
        USER_FUNC(btlevtcmd_SetRegistStatus, LW(10), PTR(&unitKoopatrol_status))
    ELSE()
        USER_FUNC(btlevtcmd_SetRegistStatus, LW(10), PTR(&unitDarkKoopatrol_status))
    END_IF()

    USER_FUNC(btlevtcmd_AnimeSetPoseTable, LW(10), LW(11), PTR(&unitKoopatrol_pose_table))
    USER_FUNC(btlevtcmd_ChangeDataTable, LW(10), PTR(&unitKoopatrol_data_table))
    USER_FUNC(btlevtcmd_OffStatusFlag, LW(10), 1)
    USER_FUNC(btlevtcmd_OnPartsCounterAttribute, LW(10), LW(11), 1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_INSIDE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, LW(10), LW(11), PTR("S_1"))
    USER_FUNC(btlevtcmd_SetFallAccel, LW(10), FLOAT(1.0))
    USER_FUNC(btlevtcmd_GetPos, LW(10), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPosition, LW(10), LW(0), LW(1), LW(2), 12, -1)
    USER_FUNC(btlevtcmd_SetHitOffset, LW(10), LW(11), 0, 0, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_IsGuarding, 0)
    USER_FUNC(btlevtcmd_StartWaitEvent, LW(10))
    RETURN()
EVT_END()

EVT_BEGIN(unitKoopatrol_common_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitKoopatrol_wait_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitKoopatrol_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitKoopatrol_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitKoopatrol_attack_event))
    USER_FUNC(btlevtcmd_SetRunSound, -2, PTR("SFX_ENM_TOGENOKO_MOVE3"), PTR("SFX_ENM_TOGENOKO_MOVE4"), 0, 3, 3)
    USER_FUNC(btlevtcmd_SetWalkSound, -2, PTR("SFX_ENM_TOGENOKO_MOVE3"), PTR("SFX_ENM_TOGENOKO_MOVE4"), 0, 6, 6)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitKoopatrol_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::KOOPATROL)
    RUN_CHILD_EVT(unitKoopatrol_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitDarkKoopatrol_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::DARK_KOOPATROL)
    RUN_CHILD_EVT(unitKoopatrol_common_init_event)
    RETURN()
EVT_END()

}  // namespace mod::tot::custom