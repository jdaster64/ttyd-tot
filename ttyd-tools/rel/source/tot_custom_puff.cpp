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
#include <ttyd/effdrv.h>
#include <ttyd/eff_kumokumo_n64.h>
#include <ttyd/eff_vapor_n64.h>
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
using ::ttyd::effdrv::EffEntry;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

// Constants (UW, etc.)
constexpr const int32_t UW_FloatingState = 0;
constexpr const int32_t UW_EffPtr = 1;
constexpr const int32_t UW_BattleUnitType = 2;

}  // namespace

// Evt / Function declarations.

extern const int32_t unitDarkPuff_init_event[];
extern const int32_t unitRuffPuff_init_event[];
extern const int32_t unitIcePuff_init_event[];
extern const int32_t unitPoisonPuff_init_event[];
extern const int32_t unitPuff_common_init_event[];
extern const int32_t unitPuff_unison_phase_event[];
extern const int32_t unitPuff_attack_event[];
extern const int32_t unitPuff_damage_event[];
extern const int32_t unitPuff_wait_event[];
extern const int32_t unitPuff_normal_attack_event[];
extern const int32_t unitPuff_charge_event[];
extern const int32_t unitRuffPuff_special_attack_event[];
extern const int32_t unitIcePuff_special_attack_event[];
extern const int32_t unitPoisonPuff_special_attack_event[];
extern const int32_t unitPuff_barrier_event[];
extern const int32_t unitPuff_counter_event[];
extern const int32_t unitPuff_first_attack_pos_event[];

EVT_DECLARE_USER_FUNC(eff_ice_barrier, 1);
EVT_DECLARE_USER_FUNC(eff_ice_barrier_end, 1);
EVT_DECLARE_USER_FUNC(eff_poison_barrier, 1);
EVT_DECLARE_USER_FUNC(eff_poison_breath, 1);

// Unit data.

int8_t unitDarkPuff_defense[] = { 0, 0, 0, 0, 0 };
int8_t unitDarkPuff_defense_attr[] = { 0, 0, 0, 0, 0 };
int8_t unitRuffPuff_defense[] = { 0, 0, 0, 0, 0 };
int8_t unitRuffPuff_defense_attr[] = { 0, 0, 0, 0, 3 };
int8_t unitIcePuff_defense[] = { 0, 0, 0, 0, 0 };
int8_t unitIcePuff_defense_attr[] = { 0, 1, 3, 0, 0 };
int8_t unitPoisonPuff_defense[] = { 0, 0, 0, 0, 0 };
int8_t unitPoisonPuff_defense_attr[] = { 0, 0, 0, 0, 0 };

StatusVulnerability unitDarkPuff_status = {
     95,  95, 105, 100,  90, 100, 100, 100,
    100,  95, 100,  95, 100,  95,  80,  90,
    105, 100,  90, 100, 100,  95,
};
StatusVulnerability unitRuffPuff_status = {
     75,  75, 100, 100,  70,   0, 100, 100,
    100,  90, 100,  90, 100,  90,  60,  70,
     95, 100,  70, 100, 100,  95,
};
StatusVulnerability unitIcePuff_status = {
     65,  65,  95, 100,  60,   0, 100,   0,
    100,  75, 100,  75, 100,  90,  65,  70,
     85, 100,  65, 100, 100,  95,
};
StatusVulnerability unitPoisonPuff_status = {
     65,  65,  90,   0,  60, 100, 100, 100,
    100,  80, 100,  80, 100,  80,  50,  60,
     85, 100,  60, 100, 100,  20,
};

PoseTableEntry unitPuff_pose_table[] = {
    1, "N_1",
    2, "Y_1",
    9, "Y_1",
    5, "K_1",
    4, "X_1",
    3, "X_1",
    28, "S_1",
    29, "Q_1",
    30, "Q_1",
    31, "A_1",
    39, "D_1",
    40, "W_1",
    42, "R_1",
    56, "X_1",
    57, "X_1",
    65, "T_1",
    69, "S_1",
};

DataTableEntry unitDarkPuff_data_table[] = {
    42, (void*)unitPuff_counter_event,
    48, (void*)unitPuff_first_attack_pos_event,
    0, nullptr,
};
DataTableEntry unitRuffPuff_data_table[] = {
    42, (void*)unitPuff_counter_event,
    48, (void*)unitPuff_first_attack_pos_event,
    0, nullptr,
};
DataTableEntry unitIcePuff_data_table[] = {
    41, (void*)unitPuff_counter_event,
    48, (void*)unitPuff_first_attack_pos_event,
    0, nullptr,
};
DataTableEntry unitPoisonPuff_data_table[] = {
    43, (void*)unitPuff_counter_event,
    48, (void*)unitPuff_first_attack_pos_event,
    0, nullptr,
};

BattleWeapon unitRuffPuff_weaponNormal = {
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
        AttackSpecialProperty_Flags::FLIPS_SHELLED |
        AttackSpecialProperty_Flags::GROUNDS_WINGED,
    .counter_resistance_flags = AttackCounterResistance_Flags::ELECTRIC,
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
BattleWeapon unitIcePuff_weaponNormal = {
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
        AttackSpecialProperty_Flags::FLIPS_SHELLED |
        AttackSpecialProperty_Flags::GROUNDS_WINGED,
    .counter_resistance_flags = AttackCounterResistance_Flags::ICY,
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
BattleWeapon unitPoisonPuff_weaponNormal = {
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
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::JUMPLIKE,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::FLIPS_SHELLED |
        AttackSpecialProperty_Flags::GROUNDS_WINGED,
    .counter_resistance_flags = AttackCounterResistance_Flags::POISON,
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
BattleWeapon unitRuffPuff_weaponSpecial = {
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
        AttackTargetClass_Flags::MULTIPLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH |
        AttackTargetClass_Flags::ENEMY_SELECT_SIDE_CENTER,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::ELECTRIC,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::PREFER_FRONT |
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
BattleWeapon unitIcePuff_weaponSpecial = {
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
        AttackTargetClass_Flags::MULTIPLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH |
        AttackTargetClass_Flags::ENEMY_SELECT_SIDE_CENTER,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::JUMPLIKE,
    .element = AttackElement::ICE,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::PREFER_FRONT |
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM,
        
    // status chances
    .freeze_chance = 60,
    .freeze_time = 2,
    
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
BattleWeapon unitPoisonPuff_weaponSpecial = {
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
        AttackTargetClass_Flags::MULTIPLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH |
        AttackTargetClass_Flags::ENEMY_SELECT_SIDE_CENTER,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::PREFER_FRONT |
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM,
        
    // status chances
    .poison_chance = 70,
    .poison_time = 10,
    .poison_strength = 1,
    
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

BattleUnitKindPart unitDarkPuff_parts[] = {
    {
        .index = 1,
        .name = "btl_un_monochrome_kurokumorn",
        .model_name = "c_kmoon_wb",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 30,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitDarkPuff_defense,
        .defense_attr = unitDarkPuff_defense_attr,
        .attribute_flags = 0x0000'0009 |
            PartsAttribute_Flags::HAMMERLIKE_CANNOT_TARGET |
            PartsAttribute_Flags::SHELLTOSSLIKE_CANNOT_TARGET,
        .counter_attribute_flags = 0,
        .pose_table = unitPuff_pose_table,
    },
};
BattleUnitKindPart unitRuffPuff_parts[] = {
    {
        .index = 1,
        .name = "btl_un_kurokumorn",
        .model_name = "c_kmoon",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 30,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitRuffPuff_defense,
        .defense_attr = unitRuffPuff_defense_attr,
        .attribute_flags = 0x0000'0009 |
            PartsAttribute_Flags::HAMMERLIKE_CANNOT_TARGET |
            PartsAttribute_Flags::SHELLTOSSLIKE_CANNOT_TARGET,
        .counter_attribute_flags = 0,
        .pose_table = unitPuff_pose_table,
    },
};
BattleUnitKindPart unitIcePuff_parts[] = {
    {
        .index = 1,
        .name = "btl_un_bllizard",
        .model_name = "c_kmoon_b",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 30,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitIcePuff_defense,
        .defense_attr = unitIcePuff_defense_attr,
        .attribute_flags = 0x0000'0009 |
            PartsAttribute_Flags::HAMMERLIKE_CANNOT_TARGET |
            PartsAttribute_Flags::SHELLTOSSLIKE_CANNOT_TARGET,
        .counter_attribute_flags = 0,
        .pose_table = unitPuff_pose_table,
    },
};
BattleUnitKindPart unitPoisonPuff_parts[] = {
    {
        .index = 1,
        .name = "btl_un_dokugassun",
        .model_name = "c_kmoon_g",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 30,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitPoisonPuff_defense,
        .defense_attr = unitPoisonPuff_defense_attr,
        .attribute_flags = 0x0000'0009 |
            PartsAttribute_Flags::HAMMERLIKE_CANNOT_TARGET |
            PartsAttribute_Flags::SHELLTOSSLIKE_CANNOT_TARGET,
        .counter_attribute_flags = 0,
        .pose_table = unitPuff_pose_table,
    },
};


BattleUnitKind unit_DarkPuff = {
    .unit_type = BattleUnitType::DARK_PUFF,
    .unit_name = "btl_un_monochrome_kurokumorn",
    .max_hp = 5,
    .max_fp = 0,
    .danger_hp = 1,
    .peril_hp = 1,
    .level = 1,
    .bonus_exp = 0,
    .bonus_coin = 0,
    .bonus_coin_rate = 100,
    .base_coin = 0,
    .run_rate = 60,
    .pb_soft_cap = 9999,
    .width = 35,
    .height = 30,
    .hit_offset = { 4, 30 },
    .center_offset = { 0.0f, 15.0f, 0.0f },
    .hp_gauge_offset = { 0, -3 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 18.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 17.5f, 0.0f, 0.0f },
    .kiss_hit_offset = { 17.5f, 19.5f, 0.0f },
    .cut_base_offset = { 0.0f, 15.0f, 0.0f },
    .cut_width = 35.0f,
    .cut_height = 30.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_KUMO_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::UNQUAKEABLE,
    .status_vulnerability = &unitDarkPuff_status,
    .num_parts = 1,
    .parts = unitDarkPuff_parts,
    .init_evt_code = (void*)unitDarkPuff_init_event,
    .data_table = unitDarkPuff_data_table,
};
BattleUnitKind unit_RuffPuff = {
    .unit_type = BattleUnitType::RUFF_PUFF,
    .unit_name = "btl_un_kurokumorn",
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
    .width = 35,
    .height = 30,
    .hit_offset = { 4, 30 },
    .center_offset = { 0.0f, 15.0f, 0.0f },
    .hp_gauge_offset = { 0, -3 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 18.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 17.5f, 0.0f, 0.0f },
    .kiss_hit_offset = { 17.5f, 19.5f, 0.0f },
    .cut_base_offset = { 0.0f, 15.0f, 0.0f },
    .cut_width = 35.0f,
    .cut_height = 30.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_KUMO_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::UNQUAKEABLE,
    .status_vulnerability = &unitRuffPuff_status,
    .num_parts = 1,
    .parts = unitRuffPuff_parts,
    .init_evt_code = (void*)unitRuffPuff_init_event,
    .data_table = unitRuffPuff_data_table,
};
BattleUnitKind unit_IcePuff = {
    .unit_type = BattleUnitType::ICE_PUFF,
    .unit_name = "btl_un_bllizard",
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
    .width = 35,
    .height = 30,
    .hit_offset = { 4, 30 },
    .center_offset = { 0.0f, 15.0f, 0.0f },
    .hp_gauge_offset = { 0, -3 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 18.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 17.5f, 0.0f, 0.0f },
    .kiss_hit_offset = { 17.5f, 19.5f, 0.0f },
    .cut_base_offset = { 0.0f, 15.0f, 0.0f },
    .cut_width = 35.0f,
    .cut_height = 30.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_KUMO_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::UNQUAKEABLE,
    .status_vulnerability = &unitIcePuff_status,
    .num_parts = 1,
    .parts = unitIcePuff_parts,
    .init_evt_code = (void*)unitIcePuff_init_event,
    .data_table = unitIcePuff_data_table,
};
BattleUnitKind unit_PoisonPuff = {
    .unit_type = BattleUnitType::POISON_PUFF,
    .unit_name = "btl_un_dokugassun",
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
    .width = 35,
    .height = 30,
    .hit_offset = { 4, 30 },
    .center_offset = { 0.0f, 15.0f, 0.0f },
    .hp_gauge_offset = { 0, -3 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 18.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 17.5f, 0.0f, 0.0f },
    .kiss_hit_offset = { 17.5f, 19.5f, 0.0f },
    .cut_base_offset = { 0.0f, 15.0f, 0.0f },
    .cut_width = 35.0f,
    .cut_height = 30.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_KUMO_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::UNQUAKEABLE,
    .status_vulnerability = &unitPoisonPuff_status,
    .num_parts = 1,
    .parts = unitPoisonPuff_parts,
    .init_evt_code = (void*)unitPoisonPuff_init_event,
    .data_table = unitPoisonPuff_data_table,
};

// Function / USER_FUNC definitions.

EVT_DEFINE_USER_FUNC(eff_ice_barrier) {
    int32_t id = evtGetValue(evt, evt->evtArguments[0]);
    id = ttyd::battle_sub::BattleTransID(evt, id);
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, id);
    
    if (unit && !unit->unit_work[UW_EffPtr]) {
        unit->unit_work[UW_EffPtr] = reinterpret_cast<uint32_t>(
            ttyd::eff_kumokumo_n64::effKumokumoN64Entry(
                0.0f, -1000.0f, 0.0f, 0.0f, 0.0f, 1.0f, 3, 1000));
    }
    
    intptr_t work = reinterpret_cast<intptr_t>(
        reinterpret_cast<EffEntry*>(unit->unit_work[UW_EffPtr])->eff_work);
    
    gc::vec3 pos;
    ttyd::battle_unit::BtlUnit_GetPos(unit, &pos.x, &pos.y, &pos.z);
    
    *reinterpret_cast<float*>(work + 0x04) = pos.x;
    *reinterpret_cast<float*>(work + 0x08) = pos.y + 10.0f * unit->unk_scale;
    *reinterpret_cast<float*>(work + 0x0c) = pos.z;
    *reinterpret_cast<float*>(work + 0x5c) = unit->unk_scale;
    
    return 2;    
}

EVT_DEFINE_USER_FUNC(eff_ice_barrier_end) {
    int32_t id = evtGetValue(evt, evt->evtArguments[0]);
    id = ttyd::battle_sub::BattleTransID(evt, id);
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, id);
    
    if (unit->unit_work[UW_EffPtr] != 0) {
        ttyd::effdrv::effDelete(
            reinterpret_cast<EffEntry*>(unit->unit_work[UW_EffPtr]));
        unit->unit_work[UW_EffPtr] = 0;
    }
    
    return 2;
}

EVT_DEFINE_USER_FUNC(eff_poison_barrier) {
    int32_t id = evtGetValue(evt, evt->evtArguments[0]);
    id = ttyd::battle_sub::BattleTransID(evt, id);
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, id);
    
    for (int32_t i = 0; i < 3; ++i) {
        gc::vec3 pos;
        ttyd::battle_unit::BtlUnit_GetPos(unit, &pos.x, &pos.y, &pos.z);
        pos.x += ttyd::system::irand(20) - 10;
        pos.y += ttyd::system::irand(30);
        
        auto* eff = ttyd::eff_vapor_n64::effVaporN64Entry(
            pos.x, pos.y, pos.z,
            0.5f + (0.5f * ttyd::system::irand(32768)) / 32767.0f,
            2, (i + 1) * 10);

        const uint32_t color = 0x207364ffU;
        *(int32_t*)((uintptr_t)eff->eff_work + 0x18) = (color >> 24) & 0xff;
        *(int32_t*)((uintptr_t)eff->eff_work + 0x1c) = (color >> 16) & 0xff;
        *(int32_t*)((uintptr_t)eff->eff_work + 0x20) = (color >> 8)  & 0xff;
        *(int32_t*)((uintptr_t)eff->eff_work + 0x24) = (color >> 0)  & 0xff;
        *(int32_t*)((uintptr_t)eff->eff_work + 0x28) = (color >> 24) & 0xff;
        *(int32_t*)((uintptr_t)eff->eff_work + 0x2c) = (color >> 16) & 0xff;
        *(int32_t*)((uintptr_t)eff->eff_work + 0x30) = (color >> 8)  & 0xff;
        *(int32_t*)((uintptr_t)eff->eff_work + 0x34) = (color >> 0)  & 0xff;
        
        *(float*)((uintptr_t)eff->eff_work + 0x40) = unit->unk_scale;
    }
    
    return 2;
}

EVT_DEFINE_USER_FUNC(eff_poison_breath) {
    float x_base = evtGetValue(evt, evt->evtArguments[0]) ? 125.0f : -125.0f;
    
    auto* eff = ttyd::eff_vapor_n64::effVaporN64Entry(
        x_base + ttyd::system::irand(150) - 75,
        ttyd::system::irand(70),
        ttyd::system::irand(80) - 40,
        2.5f + ttyd::system::irand(32768) / 32767.0f, 0, 15);

    const uint32_t color = 0x207364ffU;
    *(int32_t*)((uintptr_t)eff->eff_work + 0x18) = (color >> 24) & 0xff;
    *(int32_t*)((uintptr_t)eff->eff_work + 0x1c) = (color >> 16) & 0xff;
    *(int32_t*)((uintptr_t)eff->eff_work + 0x20) = (color >> 8)  & 0xff;
    *(int32_t*)((uintptr_t)eff->eff_work + 0x24) = (color >> 0)  & 0xff;
    *(int32_t*)((uintptr_t)eff->eff_work + 0x28) = (color >> 24) & 0xff;
    *(int32_t*)((uintptr_t)eff->eff_work + 0x2c) = (color >> 16) & 0xff;
    *(int32_t*)((uintptr_t)eff->eff_work + 0x30) = (color >> 8)  & 0xff;
    *(int32_t*)((uintptr_t)eff->eff_work + 0x34) = (color >> 0)  & 0xff;
    
    return 2;
}

// Evt definitions.

EVT_BEGIN(unitPuff_barrier_event)
LBL(0)
    WAIT_FRM(31)
    USER_FUNC(btlevtcmd_GetUnitId, -2, LW(0))
    IF_EQUAL(LW(0), -1)
        GOTO(99)
    END_IF()
        
    USER_FUNC(btlevtcmd_CheckPartsCounterAttribute, -2, 1, (int32_t)PartsCounterAttribute_Flags::ELECTRIC, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        ADD(LW(1), 20)
        USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(3))
        USER_FUNC(evt_eff64, PTR(""), PTR("akari_charge_n64"), 0, LW(0), LW(1), LW(2), LW(3), 30, 0, 0, 0, 0, 0, 0)
    END_IF()
    
    USER_FUNC(btlevtcmd_CheckPartsCounterAttribute, -2, 1, (int32_t)PartsCounterAttribute_Flags::ICY, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(eff_ice_barrier, -2)
    ELSE()
        USER_FUNC(eff_ice_barrier_end, -2)
    END_IF()
    
    USER_FUNC(btlevtcmd_CheckPartsCounterAttribute, -2, 1, (int32_t)PartsCounterAttribute_Flags::POISON_STATUS, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(eff_poison_barrier, -2)
    END_IF()
        
    GOTO(0)
LBL(99)
    RETURN()
EVT_END()

// Note: LW(10) should be set to the type of charge to apply before calling.
EVT_BEGIN(unitPuff_charge_event)
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, -1)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 300)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_KUMO_CHARGE1"), EVT_NULLPTR, 0, EVT_NULLPTR)

    // TODO: Patch OnPartsCounterAttribute to use evtGetValue instead of reading directly??
    SWITCH(LW(10))
        CASE_EQUAL((int32_t)PartsCounterAttribute_Flags::ELECTRIC)
            USER_FUNC(btlevtcmd_OnPartsCounterAttribute, -2, 1, (int32_t)PartsCounterAttribute_Flags::ELECTRIC)
        CASE_EQUAL((int32_t)PartsCounterAttribute_Flags::ICY)
            USER_FUNC(btlevtcmd_OnPartsCounterAttribute, -2, 1, (int32_t)PartsCounterAttribute_Flags::ICY)
        CASE_ETC()
            USER_FUNC(btlevtcmd_OnPartsCounterAttribute, -2, 1, (int32_t)PartsCounterAttribute_Flags::POISON_STATUS)
    END_SWITCH()

    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_2A"))
    WAIT_FRM(20)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_2B"))
    WAIT_FRM(60)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_1"))
    WAIT_MSEC(600)
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitPuff_normal_attack_event)
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
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_KUMO_MOVE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    BROTHER_EVT()
        DO(10)
            USER_FUNC(btlevtcmd_SetDispOffset, -2, 1, 0, 0)
            WAIT_FRM(2)
            USER_FUNC(btlevtcmd_SetDispOffset, -2, -1, 0, 0)
            WAIT_FRM(2)
        WHILE()
        USER_FUNC(btlevtcmd_SetDispOffset, -2, 0, 0, 0)
    END_BROTHER()
    BROTHER_EVT()
        DO(40)
            USER_FUNC(btlevtcmd_AddScale, -2, FLOAT(-0.01), 0, 0)
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_KUMO_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_2A"))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), -40)
    SET(LW(1), 60)
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 30, 0, 4, 0, -1)
    WAIT_FRM(10)
    USER_FUNC(btlevtcmd_SetScale, -2, FLOAT(1.0), FLOAT(1.0), FLOAT(1.0))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_KUMO_MOVE2"), EVT_NULLPTR, 0, LW(15))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_1A"))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(10), LW(11), LW(12))
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(10), LW(11), LW(12), 40, -40, 1, 0, -1)
    USER_FUNC(evt_snd_sfxoff, LW(15))
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
            SET(LW(15), 1)
            CASE_END()
    END_SWITCH()
LBL(90)
    GOTO(98)
LBL(91)
    USER_FUNC(evt_btl_camera_shake_h, 0, 1, 1, 8, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_add_zoom, 0, -50)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    GOTO(98)
LBL(98)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    ADD(LW(1), 40)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(10), LW(11), LW(12))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(10), 40)
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(10), LW(1), LW(12), 40, 0, 4, 0, -1)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_KUMO_MOVE3"), EVT_NULLPTR, 0, LW(15))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 40, 40, 4, 0, -1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_1"))
    USER_FUNC(evt_snd_sfxoff, LW(15))
    WAIT_MSEC(250)
    USER_FUNC(btlevtcmd_SetScale, -2, 1, 1, 1)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitRuffPuff_special_attack_event)
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
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_KUMO_CHARGE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_2B"))
    WAIT_FRM(30)
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_3A"))
    WAIT_FRM(20)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_3B"))
    USER_FUNC(btlevtcmd_OffPartsCounterAttribute, -2, 1, (int32_t)PartsCounterAttribute_Flags::ELECTRIC)
    BROTHER_EVT()
        USER_FUNC(evtTot_CheckSpeciesIsEnemy, LW(3), LW(10))
        IF_EQUAL(LW(10), 1)
            SET(LW(10), 125)
        ELSE()
            SET(LW(10), -125)
        END_IF()
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_KUMO_CHARGE_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        SET(LW(0), LW(10))
        SET(LW(2), 0)
        ADD(LW(0), -60)
        ADD(LW(2), -60)
        USER_FUNC(evt_eff64, PTR(""), PTR("pokopi_thunder_n64"), 0, LW(0), 210, LW(2), LW(0), 0, LW(2), FLOAT(4.0), 20, 0, 0, 0)
        SET(LW(0), LW(10))
        SET(LW(2), 0)
        ADD(LW(0), 60)
        ADD(LW(2), 60)
        USER_FUNC(evt_eff64, PTR(""), PTR("pokopi_thunder_n64"), 0, LW(0), 210, LW(2), LW(0), 0, LW(2), FLOAT(4.0), 20, 0, 0, 0)
        WAIT_FRM(12)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_KUMO_CHARGE_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        SET(LW(0), LW(10))
        SET(LW(2), 0)
        ADD(LW(0), 0)
        ADD(LW(2), -75)
        USER_FUNC(evt_eff64, PTR(""), PTR("pokopi_thunder_n64"), 0, LW(0), 210, LW(2), LW(0), 0, LW(2), FLOAT(4.0), 20, 0, 0, 0)
        SET(LW(0), LW(10))
        SET(LW(2), 0)
        ADD(LW(0), 0)
        ADD(LW(2), 75)
        USER_FUNC(evt_eff64, PTR(""), PTR("pokopi_thunder_n64"), 0, LW(0), 210, LW(2), LW(0), 0, LW(2), FLOAT(4.0), 20, 0, 0, 0)
        WAIT_FRM(12)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_KUMO_CHARGE_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        SET(LW(0), LW(10))
        SET(LW(2), 0)
        ADD(LW(0), 60)
        ADD(LW(2), -60)
        USER_FUNC(evt_eff64, PTR(""), PTR("pokopi_thunder_n64"), 0, LW(0), 210, LW(2), LW(0), 0, LW(2), FLOAT(4.0), 20, 0, 0, 0)
        SET(LW(0), LW(10))
        SET(LW(2), 0)
        ADD(LW(0), -60)
        ADD(LW(2), 60)
        USER_FUNC(evt_eff64, PTR(""), PTR("pokopi_thunder_n64"), 0, LW(0), 210, LW(2), LW(0), 0, LW(2), FLOAT(4.0), 20, 0, 0, 0)
        WAIT_FRM(12)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_KUMO_CHARGE_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        SET(LW(0), LW(10))
        SET(LW(2), 0)
        ADD(LW(0), 75)
        ADD(LW(2), 0)
        USER_FUNC(evt_eff64, PTR(""), PTR("pokopi_thunder_n64"), 0, LW(0), 210, LW(2), LW(0), 0, LW(2), FLOAT(4.0), 20, 0, 0, 0)
        SET(LW(0), LW(10))
        SET(LW(2), 0)
        ADD(LW(0), -75)
        ADD(LW(2), 0)
        USER_FUNC(evt_eff64, PTR(""), PTR("pokopi_thunder_n64"), 0, LW(0), 210, LW(2), LW(0), 0, LW(2), FLOAT(4.0), 20, 0, 0, 0)
        WAIT_FRM(12)
    END_BROTHER()
    WAIT_FRM(10)
    USER_FUNC(evt_btl_camera_shake_h, 0, 5, 0, 30, 0)
    SET(LW(6), 0)
LBL(0)
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
    IF_EQUAL(LW(3), -1)
        GOTO(98)
    END_IF()
    GOTO(0)
LBL(98)
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_1"))
    WAIT_MSEC(250)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitIcePuff_special_attack_event)
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
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, -1)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(btlevtcmd_OffPartsCounterAttribute, -2, 1, (int32_t)PartsCounterAttribute_Flags::ICY)
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("R_1"))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_KUMO_MOVE3"), EVT_NULLPTR, 0, LW(15))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_DivePosition, -2, -40, 15, LW(2), 30, 0, 4, 0, -1)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BLIZ_KUMO_CHARGE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_2B"))
    WAIT_FRM(60)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_KUMO_MOVE4"), EVT_NULLPTR, 0, LW(15))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(5))
    SET(LW(6), LW(5))
    MULF(LW(6), FLOAT(5.0))
    ADD(LW(1), LW(6))
    MULF(LW(5), FLOAT(2.0))
    USER_FUNC(evtTot_CheckSpeciesIsEnemy, LW(3), LW(10))
    IF_EQUAL(LW(10), 1)
        USER_FUNC(evt_eff64, PTR(""), PTR("kumokumo_n64"), 4, LW(0), LW(1), LW(2), 0, 0, LW(5), 140, 0, 0, 0, 0)
    ELSE()
        USER_FUNC(evt_eff64, PTR(""), PTR("kumokumo_n64"), 4, LW(0), LW(1), LW(2), 180, 0, LW(5), 140, 0, 0, 0, 0)
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_3A"))
    WAIT_FRM(20)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_3B"))
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    WAIT_FRM(20)
    SET(LW(6), 0)
LBL(0)
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
    IF_EQUAL(LW(3), -1)
        GOTO(98)
    END_IF()
    GOTO(0)
LBL(98)
    WAIT_FRM(60)
    USER_FUNC(evt_snd_sfxoff, LW(15))
    WAIT_FRM(60)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("R_1"))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_KUMO_MOVE3"), EVT_NULLPTR, 0, LW(15))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 30, 0, 4, 0, -1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_1"))
    WAIT_MSEC(250)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitPoisonPuff_special_attack_event)
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
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("R_1"))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_KUMO_MOVE3"), EVT_NULLPTR, 0, LW(15))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_DivePosition, -2, -40, LW(1), LW(2), 30, 0, 4, 0, -1)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_DOKU_KUMO_CHARGE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_2B"))
    WAIT_FRM(60)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_DOKU_KUMO_CHARGE_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_3A"))
    WAIT_FRM(20)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_3B"))
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_OffPartsCounterAttribute, -2, 1, (int32_t)PartsCounterAttribute_Flags::POISON_STATUS)
    BROTHER_EVT()
        USER_FUNC(evtTot_CheckSpeciesIsEnemy, LW(3), LW(10))
        IF_EQUAL(LW(10), 1)
            DO(20)
                USER_FUNC(eff_poison_breath, 1)
                WAIT_FRM(3)
            WHILE()
        ELSE()
            DO(20)
                USER_FUNC(eff_poison_breath, 0)
                WAIT_FRM(3)
            WHILE()
        END_IF()
    END_BROTHER()
    WAIT_FRM(10)
    SET(LW(6), 0)
LBL(0)
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
    IF_EQUAL(LW(3), -1)
        GOTO(98)
    END_IF()
    GOTO(0)
LBL(98)
    WAIT_FRM(60)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("R_1"))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_KUMO_MOVE3"), EVT_NULLPTR, 0, LW(15))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 30, 0, 4, 0, -1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_1"))
    WAIT_MSEC(250)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitPuff_attack_event)
    USER_FUNC(btlevtcmd_EnemyItemUseCheck, -2, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(LW(0))
        USER_FUNC(btlevtcmd_StartWaitEvent, -2)
        RETURN()
    END_IF()

    USER_FUNC(btlevtcmd_CheckPartsCounterAttribute, -2, 1, (int32_t)PartsCounterAttribute_Flags::ELECTRIC, LW(0))
    IF_EQUAL(LW(0), 1)
        SET(LW(9), PTR(&unitRuffPuff_weaponSpecial))
        RUN_CHILD_EVT(PTR(&unitRuffPuff_special_attack_event))
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_CheckPartsCounterAttribute, -2, 1, (int32_t)PartsCounterAttribute_Flags::ICY, LW(0))
    IF_EQUAL(LW(0), 1)
        SET(LW(9), PTR(&unitIcePuff_weaponSpecial))
        RUN_CHILD_EVT(PTR(&unitIcePuff_special_attack_event))
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_CheckPartsCounterAttribute, -2, 1, (int32_t)PartsCounterAttribute_Flags::POISON_STATUS, LW(0))
    IF_EQUAL(LW(0), 1)
        SET(LW(9), PTR(&unitPoisonPuff_weaponSpecial))
        RUN_CHILD_EVT(PTR(&unitPoisonPuff_special_attack_event))
        GOTO(99)
    END_IF()

    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(0))
    SWITCH(LW(0))
        CASE_EQUAL((int32_t)BattleUnitType::DARK_PUFF)
            USER_FUNC(btlevtcmd_DrawLots, LW(0), 2, 50, 50)
            SET(LW(9), PTR(&unitRuffPuff_weaponNormal))
            SET(LW(10), (int32_t)PartsCounterAttribute_Flags::ELECTRIC)

        CASE_EQUAL((int32_t)BattleUnitType::RUFF_PUFF)
            USER_FUNC(btlevtcmd_DrawLots, LW(0), 2, 60, 40)
            SET(LW(9), PTR(&unitRuffPuff_weaponNormal))
            SET(LW(10), (int32_t)PartsCounterAttribute_Flags::ELECTRIC)

        CASE_EQUAL((int32_t)BattleUnitType::ICE_PUFF)
            USER_FUNC(btlevtcmd_DrawLots, LW(0), 2, 70, 30)
            SET(LW(9), PTR(&unitIcePuff_weaponNormal))
            SET(LW(10), (int32_t)PartsCounterAttribute_Flags::ICY)
            
        CASE_ETC()
            USER_FUNC(btlevtcmd_DrawLots, LW(0), 2, 65, 35)
            SET(LW(9), PTR(&unitPoisonPuff_weaponNormal))
            SET(LW(10), (int32_t)PartsCounterAttribute_Flags::POISON_STATUS)
    END_SWITCH()

    IF_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(PTR(&unitPuff_normal_attack_event))
    ELSE()
        RUN_CHILD_EVT(PTR(&unitPuff_charge_event))
    END_IF()
    
LBL(99)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitPuff_counter_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitPuff_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_CheckDamageCode, -2, 0x200, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_CheckDamageCode, -2, 0x800, LW(0))
        IF_EQUAL(LW(0), 0)
            // Shouldn't have any effect on non-Ice Puffs.
            USER_FUNC(btlevtcmd_OffPartsCounterAttribute, -2, 1, (int32_t)PartsCounterAttribute_Flags::ICY)
            USER_FUNC(eff_ice_barrier_end, -2)
        END_IF()
    END_IF()
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    RETURN()
EVT_END()

EVT_BEGIN(unitPuff_first_attack_pos_event)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    SET(LW(1), 10)
    USER_FUNC(btlevtcmd_SetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_WaitAttackEnd)
    RETURN()
EVT_END()

EVT_BEGIN(unitPuff_unison_phase_event)
    USER_FUNC(btlevtcmd_CheckPhase, LW(0), 0x400'0001)
    IF_EQUAL(LW(0), 0)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_CheckActStatus, -2, LW(0))
    IF_EQUAL(LW(0), 0)
        GOTO(99)
    END_IF()
        
    USER_FUNC(btlevtcmd_CheckPartsCounterAttribute, -2, 1, (int32_t)PartsCounterAttribute_Flags::ELECTRIC, LW(0))
    IF_EQUAL(LW(0), 1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_CheckPartsCounterAttribute, -2, 1, (int32_t)PartsCounterAttribute_Flags::ICY, LW(0))
    IF_EQUAL(LW(0), 1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_CheckPartsCounterAttribute, -2, 1, (int32_t)PartsCounterAttribute_Flags::POISON_STATUS, LW(0))
    IF_EQUAL(LW(0), 1)
        GOTO(99)
    END_IF()
        
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(0))
    SET(LW(1), 50)
    IF_EQUAL(LW(0), (int32_t)BattleUnitType::POISON_PUFF)
        SET(LW(1), 25)
    END_IF()
    USER_FUNC(evt_sub_random, 99, LW(0))
    
    IF_SMALL(LW(0), LW(1))
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_FloatingState, LW(0))
        IF_EQUAL(LW(0), 1)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_KUMO_MOVE4"), EVT_NULLPTR, 0, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
            SET(LW(1), 40)
            USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 60, 10, 4, 0, -1)
            USER_FUNC(btlevtcmd_SetHomePos, -2, LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 1, 0x60'0000)
            USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_FloatingState, 0)
        ELSE()
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_KUMO_MOVE5"), EVT_NULLPTR, 0, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
            SET(LW(1), 10)
            USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 60, -10, 4, 0, -1)
            USER_FUNC(btlevtcmd_SetHomePos, -2, LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, 0x60'0000)
            USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_FloatingState, 1)
        END_IF()
    END_IF()
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitPuff_wait_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    USER_FUNC(btlevtcmd_SetScale, -2, 1, 1, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitPuff_common_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitPuff_wait_event))
    USER_FUNC(btlevtcmd_SetEventUnisonPhase, -2, PTR(&unitPuff_unison_phase_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitPuff_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitPuff_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitPuff_attack_event))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_FloatingState, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_EffPtr, 0)
    RUN_EVT(PTR(&unitPuff_barrier_event))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitDarkPuff_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::DARK_PUFF)
    RUN_CHILD_EVT(unitPuff_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitRuffPuff_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::RUFF_PUFF)
    RUN_CHILD_EVT(unitPuff_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitIcePuff_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::ICE_PUFF)
    RUN_CHILD_EVT(unitPuff_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitPoisonPuff_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::POISON_PUFF)
    RUN_CHILD_EVT(unitPuff_common_init_event)
    RETURN()
EVT_END()

}  // namespace mod::tot::custom