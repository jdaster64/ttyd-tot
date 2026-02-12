#include "tot_custom_rel.h"     // For externed unit definitions.

#include "evt_cmd.h"
#include "tot_party_mario.h"

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
constexpr const int32_t UW_NumSegments = 0;
constexpr const int32_t UW_ChildLevel = 1;
constexpr const int32_t UW_BattleUnitType = 2;

}  // namespace

// Evt / Function declarations.

extern const int32_t unitPokey_init_event[];
extern const int32_t unitPoisonPokey_init_event[];
extern const int32_t unitPokey_common_init_event[];
extern const int32_t unitPokey_attack_event[];
extern const int32_t unitPokey_damage_event[];
extern const int32_t unitPokey_wait_event[];
extern const int32_t unitPokey_set_body_params[];
extern const int32_t unitPokey_head_attack_event[];
extern const int32_t unitPokey_ram_attack_event[];
extern const int32_t unitPokey_spawn_friend_event[];
extern const int32_t unitPokey_projectile_attack_event[];
extern const int32_t unitPokey_bounce_event[];
extern const int32_t unitPokey_spiky_counter_event[];
extern const int32_t unitPokey_damage_event_bodylost[];
extern const int32_t unitPokey_damage_event_bodylost_blow[];
extern const int32_t unitPokey_damage_body_swallowed_event[];
extern const int32_t unitPokey_body_fall_event[];

// Unit data.

int8_t unitPokey_defense[] = { 0, 0, 0, 0, 0 };
int8_t unitPokey_defense_attr[] = { 0, 0, 0, 0, 0 };

StatusVulnerability unitPokey_status = {
     95,  80,  80, 100,  90, 100, 100,  60,
    100,  90, 100,  90, 100,  95,  90, 100,
     90, 100,  90, 100, 100, 100,
};
StatusVulnerability unitPoisonPokey_status = {
     75,  60,  60, 100,  70, 100, 100,  40,
    100,  85, 100,  85, 100,  90,  70,  80,
     70, 100,  70, 100, 100,  95,
};

PoseTableEntry unitPokey_4seg_pose_table[] = {
    1, "SNB_N_1",
    2, "SNB_Y_1",
    9, "SNB_Y_1",
    5, "SNB_K_1",
    4, "SNB_X_1",
    3, "SNB_X_1",
    28, "SNB_S_1",
    29, "SNB_Q_1",
    30, "SNB_Q_1",
    31, "SNB_S_1",
    39, "SNB_D_1",
    47, "SNB_W_1",
    42, "SNB_W_1",
    40, "SNB_W_1",
    56, "SNB_X_1",
    57, "SNB_X_1",
    65, "SNB_T_1",
    69, "SNB_S_1",
};
PoseTableEntry unitPokey_3seg_pose_table[] = {
    1, "SNB_N_2",
    2, "SNB_Y_2",
    9, "SNB_Y_2",
    5, "SNB_K_2",
    4, "SNB_X_2",
    3, "SNB_X_2",
    28, "SNB_S_2",
    29, "SNB_Q_2",
    30, "SNB_Q_2",
    31, "SNB_S_2",
    39, "SNB_D_2",
    47, "SNB_W_2",
    50, "SNB_A_1A",
    51, "SNB_A_1B",
    42, "SNB_W_2",
    40, "SNB_W_2",
    56, "SNB_X_2",
    57, "SNB_X_2",
    65, "SNB_T_2",
    69, "SNB_S_2",
};
PoseTableEntry unitPokey_2seg_pose_table[] = {
    1, "SNB_N_3",
    2, "SNB_Y_3",
    9, "SNB_Y_3",
    5, "SNB_K_3",
    4, "SNB_X_3",
    3, "SNB_X_3",
    28, "SNB_S_3",
    29, "SNB_Q_3",
    30, "SNB_Q_3",
    31, "SNB_S_3",
    39, "SNB_D_3",
    47, "SNB_W_3",
    50, "SNB_A_2A",
    51, "SNB_A_2B",
    42, "SNB_W_3",
    40, "SNB_W_3",
    56, "SNB_X_3",
    57, "SNB_X_3",
    65, "SNB_T_3",
    69, "SNB_S_3",
};
PoseTableEntry unitPokey_1seg_pose_table[] = {
    1, "SNB_N_4",
    2, "SNB_Y_4",
    9, "SNB_Y_4",
    5, "SNB_K_4",
    4, "SNB_X_4",
    3, "SNB_X_4",
    28, "SNB_S_4",
    29, "SNB_Q_4",
    30, "SNB_Q_4",
    31, "SNB_S_4",
    39, "SNB_D_4",
    47, "SNB_W_4",
    50, "SNB_A_3A",
    51, "SNB_A_3B",
    42, "SNB_W_4",
    40, "SNB_W_4",
    56, "SNB_X_4",
    57, "SNB_X_4",
    65, "SNB_T_4",
    69, "SNB_S_4",
};
PoseTableEntry unitPokey_parts_pose_table[] = {
    28, "SNB_B_1",
    39, "SNB_B_1",
    69, "SNB_B_1",
};

DataTableEntry unitPokey_data_table[] = {
    37, (void*)unitPokey_spiky_counter_event,
    39, (void*)unitPokey_spiky_counter_event,
    19, (void*)unitPokey_damage_event_bodylost,
    18, (void*)unitPokey_damage_event_bodylost_blow,
    16, (void*)unitPokey_damage_event_bodylost,
    48, (void*)btldefaultevt_Dummy,
    54, (void*)unitPokey_damage_body_swallowed_event,
    0, nullptr,
};

BattleWeapon unitPokey_weaponTall = {
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
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
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
BattleWeapon unitPokey_weaponShort = {
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
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::ONLY_FRONT |
        AttackTargetProperty_Flags::SHELL_TOSS_LIKE |
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
BattleWeapon unitPokey_weaponHead = {
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
        AttackTargetProperty_Flags::ONLY_FRONT |
        AttackTargetProperty_Flags::SHELL_TOSS_LIKE |
        AttackTargetProperty_Flags::CANNOT_TARGET_CEILING,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::FLIPS_SHELLED,
    .counter_resistance_flags = 0,
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
BattleWeapon unitPokey_weaponProjectile = {
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
BattleWeapon unitPoisonPokey_weaponTall = {
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
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
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
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    .poison_chance = 60,
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
BattleWeapon unitPoisonPokey_weaponShort = {
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
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::ONLY_FRONT |
        AttackTargetProperty_Flags::SHELL_TOSS_LIKE |
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
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    .poison_chance = 60,
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
BattleWeapon unitPoisonPokey_weaponHead = {
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
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::ONLY_FRONT |
        AttackTargetProperty_Flags::SHELL_TOSS_LIKE |
        AttackTargetProperty_Flags::CANNOT_TARGET_CEILING,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::FLIPS_SHELLED,
    .counter_resistance_flags = 0,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    .poison_chance = 60,
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
BattleWeapon unitPoisonPokey_weaponProjectile = {
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

BattleUnitKindPart unitPokey_parts[] = {
    {
        .index = 1,
        .name = "btl_un_sambo",
        .model_name = "c_sanbo",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 84.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 90.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 90,
        .base_alpha = 255,
        .defense = unitPokey_defense,
        .defense_attr = unitPokey_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags =
            PartsCounterAttribute_Flags::TOP_SPIKY |
            PartsCounterAttribute_Flags::FRONT_SPIKY,
        .pose_table = unitPokey_4seg_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_sambo",
        .model_name = "c_sanbo",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 64.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 70.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 70,
        .base_alpha = 255,
        .defense = unitPokey_defense,
        .defense_attr = unitPokey_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000,
        .counter_attribute_flags =
            PartsCounterAttribute_Flags::TOP_SPIKY |
            PartsCounterAttribute_Flags::FRONT_SPIKY,
        .pose_table = unitPokey_3seg_pose_table,
    },
    {
        .index = 3,
        .name = "btl_un_sambo",
        .model_name = "c_sanbo",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 44.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 50.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 50,
        .base_alpha = 255,
        .defense = unitPokey_defense,
        .defense_attr = unitPokey_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000,
        .counter_attribute_flags =
            PartsCounterAttribute_Flags::TOP_SPIKY |
            PartsCounterAttribute_Flags::FRONT_SPIKY,
        .pose_table = unitPokey_2seg_pose_table,
    },
    {
        .index = 4,
        .name = "btl_un_sambo",
        .model_name = "c_sanbo",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 24.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitPokey_defense,
        .defense_attr = unitPokey_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000,
        .counter_attribute_flags =
            PartsCounterAttribute_Flags::TOP_SPIKY |
            PartsCounterAttribute_Flags::FRONT_SPIKY,
        .pose_table = unitPokey_1seg_pose_table,
    },
    {
        .index = 5,
        .name = "btl_un_sambo",
        .model_name = "c_sanbo",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitPokey_defense,
        .defense_attr = unitPokey_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitPokey_parts_pose_table,
    },
};
BattleUnitKindPart unitPoisonPokey_parts[] = {
    {
        .index = 1,
        .name = "btl_un_sambo_mummy",
        .model_name = "c_sanbo_m",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 84.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 90.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 90,
        .base_alpha = 255,
        .defense = unitPokey_defense,
        .defense_attr = unitPokey_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags =
            PartsCounterAttribute_Flags::TOP_SPIKY |
            PartsCounterAttribute_Flags::FRONT_SPIKY,
        .pose_table = unitPokey_4seg_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_sambo_mummy",
        .model_name = "c_sanbo_m",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 64.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 70.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 70,
        .base_alpha = 255,
        .defense = unitPokey_defense,
        .defense_attr = unitPokey_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000,
        .counter_attribute_flags =
            PartsCounterAttribute_Flags::TOP_SPIKY |
            PartsCounterAttribute_Flags::FRONT_SPIKY,
        .pose_table = unitPokey_3seg_pose_table,
    },
    {
        .index = 3,
        .name = "btl_un_sambo_mummy",
        .model_name = "c_sanbo_m",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 44.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 50.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 50,
        .base_alpha = 255,
        .defense = unitPokey_defense,
        .defense_attr = unitPokey_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000,
        .counter_attribute_flags =
            PartsCounterAttribute_Flags::TOP_SPIKY |
            PartsCounterAttribute_Flags::FRONT_SPIKY,
        .pose_table = unitPokey_2seg_pose_table,
    },
    {
        .index = 4,
        .name = "btl_un_sambo_mummy",
        .model_name = "c_sanbo_m",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 24.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitPokey_defense,
        .defense_attr = unitPokey_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000,
        .counter_attribute_flags =
            PartsCounterAttribute_Flags::TOP_SPIKY |
            PartsCounterAttribute_Flags::FRONT_SPIKY,
        .pose_table = unitPokey_1seg_pose_table,
    },
    {
        .index = 5,
        .name = "btl_un_sambo_mummy",
        .model_name = "c_sanbo_m",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitPokey_defense,
        .defense_attr = unitPokey_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitPokey_parts_pose_table,
    },
};

BattleUnitKind unit_Pokey = {
    .unit_type = BattleUnitType::POKEY,
    .unit_name = "btl_un_sambo",
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
    .height = 80,
    .hit_offset = { 14, 80 },
    .center_offset = { 0.0f, 40.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 13.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 13.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 10.0f, 52.0f, 0.0f },
    .cut_base_offset = { 0.0f, 40.0f, 0.0f },
    .cut_width = 26.0f,
    .cut_height = 80.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_SAMBO_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitPokey_status,
    .num_parts = 5,
    .parts = unitPokey_parts,
    .init_evt_code = (void*)unitPokey_init_event,
    .data_table = unitPokey_data_table,
};
BattleUnitKind unit_PoisonPokey = {
    .unit_type = BattleUnitType::POISON_POKEY,
    .unit_name = "btl_un_sambo_mummy",
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
    .height = 80,
    .hit_offset = { 14, 80 },
    .center_offset = { 0.0f, 40.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 13.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 13.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 10.0f, 52.0f, 0.0f },
    .cut_base_offset = { 0.0f, 40.0f, 0.0f },
    .cut_width = 26.0f,
    .cut_height = 80.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_SAMBO_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitPoisonPokey_status,
    .num_parts = 5,
    .parts = unitPoisonPokey_parts,
    .init_evt_code = (void*)unitPoisonPokey_init_event,
    .data_table = unitPokey_data_table,
};

const BattleUnitSetup unitPokey_spawn_entry = {
    .unit_kind_params = &unit_Pokey,
    .alliance = 1,
    .attack_phase = 0x04000004,
    .position = { -1000.0f, -1000.0f, -1000.0f },
    .addl_target_offset_x = 0,
    .unit_work = { 0, 0, 0, 0 },
    .item_drop_table = nullptr,
};
const BattleUnitSetup unitPoisonPokey_spawn_entry = {
    .unit_kind_params = &unit_PoisonPokey,
    .alliance = 1,
    .attack_phase = 0x04000004,
    .position = { -1000.0f, -1000.0f, -1000.0f },
    .addl_target_offset_x = 0,
    .unit_work = { 0, 0, 0, 0 },
    .item_drop_table = nullptr,
};

// Evt definitions.

EVT_BEGIN(unitPokey_set_body_params)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_NumSegments, LW(0))
    SWITCH(LW(0))
        CASE_EQUAL(1)
            USER_FUNC(btlevtcmd_SetHeight, -2, 20)
            USER_FUNC(btlevtcmd_SetCutHeight, -2, 20)
        CASE_EQUAL(2)
            USER_FUNC(btlevtcmd_SetHeight, -2, 40)
            USER_FUNC(btlevtcmd_SetCutHeight, -2, 40)
        CASE_EQUAL(3)
            USER_FUNC(btlevtcmd_SetHeight, -2, 60)
            USER_FUNC(btlevtcmd_SetCutHeight, -2, 60)
        CASE_ETC()
            USER_FUNC(btlevtcmd_SetHeight, -2, 80)
            USER_FUNC(btlevtcmd_SetCutHeight, -2, 80)
    END_SWITCH()
    RETURN()
EVT_END()

EVT_BEGIN(unitPokey_attack_event)
    USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 0, 0)
    USER_FUNC(btlevtcmd_EnemyItemUseCheck, -2, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(LW(0))
        USER_FUNC(btlevtcmd_StartWaitEvent, -2)
        RETURN()
    END_IF()

    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_NumSegments, LW(10))
    SWITCH(LW(10))
        CASE_EQUAL(1)
            SET(LW(11), 4)
        CASE_EQUAL(2)
            SET(LW(11), 3)
        CASE_EQUAL(3)
            SET(LW(11), 2)
        CASE_ETC()
            SET(LW(11), 1)
    END_SWITCH()

    IF_EQUAL(LW(10), 4)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_ChildLevel, LW(0))
        IF_SMALL_EQUAL(LW(0), 1)
            // Changed chance to 30% for both Pokeys and Poison Pokeys for convenience.
            USER_FUNC(evt_sub_random, 99, LW(0))
            IF_SMALL(LW(0), 30)
                USER_FUNC(btlevtcmd_GetPos, -2, LW(6), LW(7), LW(8))
                USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(6), 40)
                SUB(LW(8), 10)
                USER_FUNC(btlevtcmd_CheckSpace, LW(3), LW(6), LW(7), LW(8), 40, 0, 0, 0)
                IF_EQUAL(LW(3), -1)
                    IF_LARGE(LW(6), 0)
                        RUN_CHILD_EVT(PTR(&unitPokey_spawn_friend_event))
                        GOTO(99)
                    END_IF()
                END_IF()
            END_IF()
        END_IF()
    END_IF()

    // Assign weapons based on enemy type and current height.
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(12))
    IF_EQUAL(LW(12), (int32_t)BattleUnitType::POKEY)
        IF_LARGE(LW(10), 2)
            SET(LW(7), PTR(&unitPokey_weaponTall))
        ELSE()
            SET(LW(7), PTR(&unitPokey_weaponShort))
        END_IF()
        SET(LW(8), PTR(&unitPokey_weaponProjectile))
        SET(LW(9), PTR(&unitPokey_weaponHead))
    ELSE()
        IF_LARGE(LW(10), 2)
            SET(LW(7), PTR(&unitPoisonPokey_weaponTall))
        ELSE()
            SET(LW(7), PTR(&unitPoisonPokey_weaponShort))
        END_IF()
        SET(LW(8), PTR(&unitPoisonPokey_weaponProjectile))
        SET(LW(9), PTR(&unitPoisonPokey_weaponHead))
    END_IF()

    IF_LARGE_EQUAL(LW(10), 2)
        USER_FUNC(evt_sub_random, 69, LW(0))
        // Use ramming attack if there's a valid target for it 3/7 of the time.
        IF_SMALL_EQUAL(LW(0), 30)
            USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
            USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(7))
            USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(7), LW(3), LW(4))
            IF_NOT_EQUAL(LW(3), -1)
                SET(LW(9), LW(7))
                RUN_CHILD_EVT(PTR(&unitPokey_ram_attack_event))
                RETURN()
            END_IF()
        END_IF()
        SET(LW(9), LW(8))
        RUN_CHILD_EVT(PTR(&unitPokey_projectile_attack_event))
        RETURN()
    END_IF()
    RUN_CHILD_EVT(PTR(&unitPokey_head_attack_event))
    
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitPokey_head_attack_event)
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
    USER_FUNC(evt_btl_camera_set_mode, 0, 8)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, LW(3))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(2.0))
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.6))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(5), LW(6), LW(7))
    USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(5), LW(6), LW(7))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(5), 50)
    SET(LW(8), 35)
    RUN_CHILD_EVT(PTR(&unitPokey_bounce_event))
    WAIT_FRM(10)
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
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.3))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, LW(3), LW(0), 5)
    USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
    ADD(LW(1), 10)
    BROTHER_EVT()
        WAIT_FRM(2)
        USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
        SET(LW(14), -10)
        IF_EQUAL(LW(15), -1)
            SET(LW(14), 10)
        END_IF()
        SET(LW(0), 0)
        DO(15)
            ADD(LW(0), LW(14))
            USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, LW(0))
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_SAMBO_MOVE3"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 44, -1)
    USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 10, 0)
    BROTHER_EVT()
        WAIT_FRM(2)
        USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
        SET(LW(0), 180)
        DO(12)
            IF_EQUAL(LW(15), -1)
                ADD(LW(0), 15)
                IF_EQUAL(LW(0), 360)
                    SET(LW(0), 0)
                END_IF()
            ELSE()
                SUB(LW(0), 15)
                IF_EQUAL(LW(0), 0)
                    SET(LW(0), 360)
                END_IF()
            END_IF()
            USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, LW(0))
            WAIT_FRM(1)
        WHILE()
        USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 0, 0)
    END_BROTHER()
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(1.0))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, LW(3), LW(0), 30)
    USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_SAMBO_MOVE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    WAIT_FRM(30)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    GOTO(98)
LBL(91)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    ADD(LW(1), 10)
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.3))
    BROTHER_EVT()
        WAIT_FRM(2)
        USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
        SET(LW(14), -10)
        IF_EQUAL(LW(15), -1)
            SET(LW(14), 10)
        END_IF()
        SET(LW(0), 0)
        DO(15)
            ADD(LW(0), LW(14))
            USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, LW(0))
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_SAMBO_MOVE3"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 36, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_SAMBO_ATTACK6"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(evt_btl_camera_shake_h, 0, 1, 1, 8, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_add_zoom, 0, -50)
    WAIT_FRM(2)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.9))
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 40)
    USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_SAMBO_MOVE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 30)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 16, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_SAMBO_MOVE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 20)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_SAMBO_MOVE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 10)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 8, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_SAMBO_MOVE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 4, 43)
LBL(98)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 4, 41)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.0))
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.5))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(5), LW(6), LW(7))
    SET(LW(8), 35)
    RUN_CHILD_EVT(PTR(&unitPokey_bounce_event))
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitPokey_ram_attack_event)
    SET(LW(6), LW(11))
    USER_FUNC(btlevtcmd_CheckStatus, -2, 11, LW(5))
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
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(evt_btl_camera_set_mode, 0, 8)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, LW(3))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    SWITCH(LW(10))
        CASE_EQUAL(4)
            IF_EQUAL(LW(5), 0)
                USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), 60)
            ELSE()
                USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), 42)
            END_IF()
        CASE_EQUAL(3)
            IF_EQUAL(LW(5), 0)
                USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), 40)
            ELSE()
                USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), 28)
            END_IF()
        CASE_EQUAL(2)
            IF_EQUAL(LW(5), 0)
                USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), 20)
            ELSE()
                USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), 14)
            END_IF()
    END_SWITCH()
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, LW(6), 40)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(2.0))
    USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, LW(6), 43)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    SET(LW(14), 40)
    IF_EQUAL(LW(15), -1)
        SET(LW(14), -40)
    END_IF()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_SAMBO_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 0, 0)
    USER_FUNC(evt_sub_intpl_init, 11, 0, LW(14), 40)
LBL(0)
    USER_FUNC(evt_sub_intpl_get_value)
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, LW(0))
    WAIT_FRM(1)
    IF_EQUAL(LW(1), 1)
        GOTO(0)
    END_IF()
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    SET(LW(13), 40)
    SET(LW(14), -90)
    IF_EQUAL(LW(15), -1)
        SET(LW(13), -40)
        SET(LW(14), 90)
    END_IF()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_SAMBO_ATTACK4"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(evt_sub_intpl_init, 1, LW(13), LW(14), 10)
LBL(3)
    USER_FUNC(evt_sub_intpl_get_value)
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, LW(0))
    WAIT_FRM(1)
    IF_EQUAL(LW(1), 1)
        GOTO(3)
    END_IF()
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
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    SET(LW(14), -90)
    IF_EQUAL(LW(15), -1)
        SET(LW(14), 90)
    END_IF()
    USER_FUNC(evt_sub_intpl_init, 0, LW(14), 0, 10)
LBL(4)
    USER_FUNC(evt_sub_intpl_get_value)
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, LW(0))
    WAIT_FRM(1)
    IF_EQUAL(LW(1), 1)
        GOTO(4)
    END_IF()
    GOTO(98)
LBL(91)
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 90)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, LW(6), 43)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    SET(LW(14), -90)
    IF_EQUAL(LW(15), -1)
        SET(LW(14), 90)
    END_IF()
    USER_FUNC(evt_sub_intpl_init, 0, LW(14), 0, 20)
LBL(5)
    USER_FUNC(evt_sub_intpl_get_value)
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, LW(0))
    WAIT_FRM(1)
    IF_EQUAL(LW(1), 1)
        GOTO(5)
    END_IF()
LBL(98)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, LW(6), 41)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(4.0))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitPokey_projectile_attack_event)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_NumSegments, LW(10))
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
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_CheckStatus, -2, 11, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_SetPartsScale, -2, 5, FLOAT(0.7), FLOAT(0.7), FLOAT(1.0))
    ELSE()
        USER_FUNC(btlevtcmd_SetPartsScale, -2, 5, FLOAT(1.0), FLOAT(1.0), FLOAT(1.0))
    END_IF()
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_NumSegments, LW(10))
    SUB(LW(10), 1)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_NumSegments, LW(10))
    RUN_CHILD_EVT(PTR(&unitPokey_set_body_params))
    SWITCH(LW(10))
        CASE_EQUAL(3)
            SET(LW(7), 1)
            SET(LW(6), 2)
            USER_FUNC(btlevtcmd_SetStatusIconOffset, -2, 14, 60)
            USER_FUNC(btlevtcmd_SetCutBaseOffset, -2, 0, 30, 0)
            USER_FUNC(btlevtcmd_SetKissHitOffset, -2, 10, 36, 0)
        CASE_EQUAL(2)
            SET(LW(7), 2)
            SET(LW(6), 3)
            USER_FUNC(btlevtcmd_SetStatusIconOffset, -2, 14, 40)
            USER_FUNC(btlevtcmd_SetCutBaseOffset, -2, 0, 20, 0)
            USER_FUNC(btlevtcmd_SetKissHitOffset, -2, 10, 26, 0)
        CASE_EQUAL(1)
            SET(LW(7), 3)
            SET(LW(6), 4)
            USER_FUNC(btlevtcmd_SetSwallowParam, -2, 50)
            USER_FUNC(btlevtcmd_SetSwallowAttribute, -2, 2)
            USER_FUNC(btlevtcmd_SetStatusIconOffset, -2, 14, 20)
            USER_FUNC(btlevtcmd_SetCutBaseOffset, -2, 0, 10, 0)
            USER_FUNC(btlevtcmd_SetKissHitOffset, -2, 10, 16, 0)
    END_SWITCH()
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, LW(7), 9)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, LW(7), 0x301'0000)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, LW(6), 9)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, LW(6), 0x101'0000)
    USER_FUNC(btlevtcmd_CheckStatus, -2, 11, LW(5))
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        IF_EQUAL(LW(5), 0)
            SET(LW(13), 20)
            SET(LW(11), 20)
        ELSE()
            SET(LW(13), 14)
            SET(LW(11), 14)
        END_IF()
        MUL(LW(13), LW(10))
        SUB(LW(13), LW(11))
        ADD(LW(1), LW(13))
        USER_FUNC(btlevtcmd_SetPartsPos, -2, 5, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 5, 0x100'0000)
        USER_FUNC(btlevtcmd_SetPartsFallAccel, -2, 5, FLOAT(0.6))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 5, PTR("SNB_B_1"))
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 5, LW(0), LW(1), LW(2), 30, -1)
    END_BROTHER()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_SAMBO_ATTACK3"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, LW(6), 50)
    WAIT_FRM(24)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_SAMBO_ATTACK4"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_SAMBO_ATTACK5"), EVT_NULLPTR, 0, LW(14))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, LW(6), 51)
    WAIT_FRM(2)
    WAIT_FRM(5)
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
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetStageSize, LW(0), EVT_NULLPTR, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(12))
    MUL(LW(0), LW(12))
    SUB(LW(1), 5)
    USER_FUNC(btlevtcmd_SetPartsMoveSpeed, -2, 5, FLOAT(10.0))
    USER_FUNC(btlevtcmd_SetPartsFallAccel, -2, 5, FLOAT(0.1))
    USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 5, LW(0), LW(1), LW(2), 0, -1)
    USER_FUNC(evt_snd_sfxoff, LW(14))
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 5, 0x100'0000)
    GOTO(99)
LBL(91)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetPartsMoveSpeed, -2, 5, FLOAT(10.0))
    USER_FUNC(btlevtcmd_SetPartsFallAccel, -2, 5, FLOAT(0.1))
    USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 5, LW(0), LW(1), LW(2), 0, -1)
    USER_FUNC(evt_btl_camera_shake_h, 0, 1, 1, 8, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_add_zoom, 0, -50)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, LW(6), 43)
    IF_EQUAL(LW(5), 21)
        USER_FUNC(evt_snd_sfxoff, LW(14))
        USER_FUNC(btlevtcmd_GetBodyId, -2, LW(11))
        USER_FUNC(btlevtcmd_GetHitPos, -2, LW(11), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_SetPartsMoveSpeed, -2, 5, FLOAT(4.5))
        USER_FUNC(btlevtcmd_SetPartsFallAccel, -2, 5, FLOAT(0.4))
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 5, LW(0), LW(1), LW(2), 40, -1)
        INLINE_EVT()
            USER_FUNC(btlevtcmd_GetPartsPos, -2, 5, LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 55)
            USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_SetPartsMoveSpeed, -2, 5, FLOAT(3.0))
            USER_FUNC(btlevtcmd_SetPartsFallAccel, -2, 5, FLOAT(0.1))
            USER_FUNC(btlevtcmd_DivePartsPosition, -2, 5, LW(0), LW(1), LW(2), 0, 25, 0, 0, -1)
            USER_FUNC(btlevtcmd_SetPartsMoveSpeed, -2, 5, FLOAT(2.0))
            USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 24)
            USER_FUNC(btlevtcmd_DivePartsPosition, -2, 5, LW(0), LW(1), LW(2), 0, 21, 0, 0, -1)
            USER_FUNC(btlevtcmd_SetPartsMoveSpeed, -2, 5, FLOAT(1.5))
            USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 20)
            USER_FUNC(btlevtcmd_DivePartsPosition, -2, 5, LW(0), LW(1), LW(2), 0, 20, 0, 0, -1)
            USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 5, 0x100'0000)
        END_INLINE()
        USER_FUNC(btlevtcmd_CheckDamage, -2, -2, LW(11), LW(9), 256, LW(5))
        RETURN()
    END_IF()
    USER_FUNC(evt_snd_sfxoff, LW(14))
    USER_FUNC(btlevtcmd_GetPartsPos, -2, 5, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, LW(3), LW(0), 55)
    USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetPartsMoveSpeed, -2, 5, FLOAT(3.0))
    USER_FUNC(btlevtcmd_SetPartsFallAccel, -2, 5, FLOAT(0.1))
    USER_FUNC(btlevtcmd_DivePartsPosition, -2, 5, LW(0), LW(1), LW(2), 0, 25, 0, 0, -1)
    USER_FUNC(btlevtcmd_SetPartsMoveSpeed, -2, 5, FLOAT(2.0))
    USER_FUNC(btlevtcmd_FaceDirectionSub, LW(3), LW(0), 24)
    USER_FUNC(btlevtcmd_DivePartsPosition, -2, 5, LW(0), LW(1), LW(2), 0, 21, 0, 0, -1)
    USER_FUNC(btlevtcmd_SetPartsMoveSpeed, -2, 5, FLOAT(1.5))
    USER_FUNC(btlevtcmd_FaceDirectionSub, LW(3), LW(0), 20)
    USER_FUNC(btlevtcmd_DivePartsPosition, -2, 5, LW(0), LW(1), LW(2), 0, 20, 0, 0, -1)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 5, 0x100'0000)
LBL(98)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitPokey_spawn_friend_event)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("SNB_S_1"))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_SAMBO_SEND_GROUP1_3"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(evt_sub_intpl_init, 11, 0, -20, 40)
LBL(0)
    USER_FUNC(evt_sub_intpl_get_value)
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, LW(0))
    WAIT_FRM(1)
    IF_EQUAL(LW(1), 1)
        GOTO(0)
    END_IF()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_SAMBO_SEND_GROUP1_2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(evt_sub_intpl_init, 11, -20, 30, 40)
LBL(1)
    USER_FUNC(evt_sub_intpl_get_value)
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, LW(0))
    WAIT_FRM(1)
    IF_EQUAL(LW(1), 1)
        GOTO(1)
    END_IF()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_SAMBO_SEND_GROUP1_1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(evt_sub_intpl_init, 11, 30, -40, 40)
LBL(2)
    USER_FUNC(evt_sub_intpl_get_value)
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, LW(0))
    WAIT_FRM(1)
    IF_EQUAL(LW(1), 1)
        GOTO(2)
    END_IF()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_SAMBO_SEND_GROUP2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(evt_sub_intpl_init, 11, -40, 90, 20)
LBL(3)
    USER_FUNC(evt_sub_intpl_get_value)
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, LW(0))
    WAIT_FRM(1)
    IF_EQUAL(LW(1), 1)
        GOTO(3)
    END_IF()
    WAIT_FRM(40)
    INLINE_EVT()
        USER_FUNC(evt_btl_camera_shake_h, 0, 2, 0, 10, 13)
    END_INLINE()

    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(0))
    IF_EQUAL(LW(0), (int32_t)BattleUnitType::POKEY)
        USER_FUNC(btlevtcmd_SpawnUnit, LW(3), PTR(&unitPokey_spawn_entry), 0)
    ELSE()
        USER_FUNC(btlevtcmd_SpawnUnit, LW(3), PTR(&unitPoisonPokey_spawn_entry), 0)
    END_IF()

    SET(LW(0), LW(6))
    SET(LW(1), LW(7))
    SET(LW(2), LW(8))
    SUB(LW(1), 80)
    USER_FUNC(btlevtcmd_SetPos, LW(3), LW(0), LW(1), LW(2))
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
        USER_FUNC(evt_eff, 0, PTR("kemuri_test"), 14, LW(0), LW(1), LW(2), FLOAT(1.5), 0, 0, 0, 0, 0, 0, 0)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_SAMBO_SEND_GROUP3"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_AnimeChangePose, LW(3), 1, PTR("SNB_D_1"))
        USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_SetFallAccel, LW(3), FLOAT(0.7))
        USER_FUNC(btlevtcmd_JumpPosition, LW(3), LW(6), LW(7), LW(8), 20, -1)
        USER_FUNC(btlevtcmd_SetHomePos, LW(3), LW(6), LW(7), LW(8))
        WAIT_FRM(40)
        USER_FUNC(btlevtcmd_StartWaitEvent, LW(3))
    END_BROTHER()
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_ChildLevel, LW(4))
    ADD(LW(4), 1)
    USER_FUNC(btlevtcmd_SetUnitWork, LW(3), UW_ChildLevel, LW(4))
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_sub_intpl_init, 4, 90, -30, 14)
LBL(4)
    USER_FUNC(evt_sub_intpl_get_value)
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, LW(0))
    WAIT_FRM(1)
    IF_EQUAL(LW(1), 1)
        GOTO(4)
    END_IF()
    WAIT_FRM(40)
    USER_FUNC(evt_sub_intpl_init, 11, -30, 0, 20)
LBL(5)
    USER_FUNC(evt_sub_intpl_get_value)
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, LW(0))
    WAIT_FRM(1)
    IF_EQUAL(LW(1), 1)
        GOTO(5)
    END_IF()
    WAIT_FRM(40)
    RETURN()
EVT_END()

EVT_BEGIN(unitPokey_damage_event)
    SET(LW(10), -2)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_NumSegments, LW(0))
    SWITCH(LW(0))
        CASE_EQUAL(1)
            SET(LW(11), 4)
        CASE_EQUAL(2)
            SET(LW(11), 3)
        CASE_EQUAL(3)
            SET(LW(11), 2)
        CASE_ETC()
            SET(LW(11), 1)
    END_SWITCH()
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    RETURN()
EVT_END()

EVT_BEGIN(unitPokey_damage_event_bodylost)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_NumSegments, LW(0))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(btlevtcmd_GetDamage, LW(10), LW(0))
        IF_LARGE_EQUAL(LW(0), 1)
            USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(10), LW(11), LW(14))
            WAIT_FRM(30)
            IF_NOT_EQUAL(LW(15), -1)
                USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(10), LW(11), LW(15))
                WAIT_FRM(30)
            END_IF()
        ELSE()
            RUN_CHILD_EVT(PTR(&subsetevt_nodamage_motion))
        END_IF()
    ELSE()
        SET(LW(12), LW(11))
        USER_FUNC(btlevtcmd_GetUnitWork, LW(10), UW_NumSegments, LW(0))
        SUB(LW(0), 1)
        USER_FUNC(btlevtcmd_SetUnitWork, LW(10), UW_NumSegments, LW(0))
        RUN_CHILD_EVT(PTR(&unitPokey_set_body_params))
        SWITCH(LW(0))
            CASE_EQUAL(1)
                SET(LW(11), 4)
                USER_FUNC(btlevtcmd_SetSwallowParam, -2, 50)
                USER_FUNC(btlevtcmd_SetSwallowAttribute, -2, 2)
                USER_FUNC(btlevtcmd_SetStatusIconOffset, -2, 14, 20)
                USER_FUNC(btlevtcmd_SetCutBaseOffset, -2, 0, 10, 0)
                USER_FUNC(btlevtcmd_SetKissHitOffset, -2, 10, 16, 0)
            CASE_EQUAL(2)
                SET(LW(11), 3)
                USER_FUNC(btlevtcmd_SetStatusIconOffset, -2, 14, 40)
                USER_FUNC(btlevtcmd_SetCutBaseOffset, -2, 0, 20, 0)
                USER_FUNC(btlevtcmd_SetKissHitOffset, -2, 10, 26, 0)
            CASE_EQUAL(3)
                SET(LW(11), 2)
                USER_FUNC(btlevtcmd_SetStatusIconOffset, -2, 14, 60)
                USER_FUNC(btlevtcmd_SetCutBaseOffset, -2, 0, 30, 0)
                USER_FUNC(btlevtcmd_SetKissHitOffset, -2, 10, 36, 0)
            CASE_ETC()
                SET(LW(11), 1)
                USER_FUNC(btlevtcmd_SetStatusIconOffset, -2, 14, 80)
                USER_FUNC(btlevtcmd_SetCutBaseOffset, -2, 0, 40, 0)
                USER_FUNC(btlevtcmd_SetKissHitOffset, -2, 10, 52, 0)
        END_SWITCH()
        USER_FUNC(btlevtcmd_OffPartsAttribute, LW(10), LW(12), 9)
        USER_FUNC(btlevtcmd_OnPartsAttribute, LW(10), LW(12), 0x301'0000)
        USER_FUNC(btlevtcmd_OnPartsAttribute, LW(10), LW(11), 9)
        USER_FUNC(btlevtcmd_OffPartsAttribute, LW(10), LW(11), 0x101'0000)
        USER_FUNC(btlevtcmd_GetPos, LW(10), LW(0), LW(1), LW(2))
        BROTHER_EVT()
            WAIT_FRM(4)
            RUN_CHILD_EVT(PTR(&unitPokey_body_fall_event))
        END_BROTHER()
        USER_FUNC(btlevtcmd_GetPos, LW(10), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_SetPartsPos, LW(10), 5, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_OffPartsAttribute, LW(10), 5, 0x100'0000)
        USER_FUNC(btlevtcmd_SetPartsFallAccel, LW(10), 5, FLOAT(0.4))
        USER_FUNC(btlevtcmd_GetPos, LW(10), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_FaceDirectionSub, LW(10), LW(0), 25)
        USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_JumpPartsPosition, LW(10), 5, LW(0), LW(1), LW(2), 16, -1)
        USER_FUNC(btlevtcmd_FaceDirectionSub, LW(10), LW(0), 16)
        USER_FUNC(btlevtcmd_JumpPartsPosition, LW(10), 5, LW(0), LW(1), LW(2), 12, -1)
        DO(40)
            USER_FUNC(btlevtcmd_OnPartsAttribute, LW(10), 5, 0x100'0000)
            WAIT_FRM(1)
            USER_FUNC(btlevtcmd_OffPartsAttribute, LW(10), 5, 0x100'0000)
            WAIT_FRM(1)
        WHILE()
        USER_FUNC(btlevtcmd_OnPartsAttribute, LW(10), 5, 0x100'0000)
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitPokey_damage_event_bodylost_blow)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_NumSegments, LW(0))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(btlevtcmd_GetDamage, LW(10), LW(0))
        IF_LARGE_EQUAL(LW(0), 1)
            RUN_CHILD_EVT(PTR(&subsetevt_shot_damage))
        ELSE()
            RUN_CHILD_EVT(PTR(&subsetevt_nodamage_motion))
        END_IF()
    ELSE()
        SET(LW(12), LW(11))
        USER_FUNC(btlevtcmd_GetUnitWork, LW(10), UW_NumSegments, LW(0))
        SUB(LW(0), 1)
        USER_FUNC(btlevtcmd_SetUnitWork, LW(10), UW_NumSegments, LW(0))
        RUN_CHILD_EVT(PTR(&unitPokey_set_body_params))
        SWITCH(LW(0))
            CASE_EQUAL(1)
                SET(LW(11), 4)
                USER_FUNC(btlevtcmd_SetSwallowParam, -2, 50)
                USER_FUNC(btlevtcmd_SetSwallowAttribute, -2, 2)
                USER_FUNC(btlevtcmd_SetStatusIconOffset, -2, 14, 20)
                USER_FUNC(btlevtcmd_SetCutBaseOffset, -2, 0, 10, 0)
                USER_FUNC(btlevtcmd_SetKissHitOffset, -2, 10, 16, 0)
            CASE_EQUAL(2)
                SET(LW(11), 3)
                USER_FUNC(btlevtcmd_SetStatusIconOffset, -2, 14, 40)
                USER_FUNC(btlevtcmd_SetCutBaseOffset, -2, 0, 20, 0)
                USER_FUNC(btlevtcmd_SetKissHitOffset, -2, 10, 26, 0)
            CASE_EQUAL(3)
                SET(LW(11), 2)
                USER_FUNC(btlevtcmd_SetStatusIconOffset, -2, 14, 60)
                USER_FUNC(btlevtcmd_SetCutBaseOffset, -2, 0, 30, 0)
                USER_FUNC(btlevtcmd_SetKissHitOffset, -2, 10, 36, 0)
            CASE_ETC()
                SET(LW(11), 1)
                USER_FUNC(btlevtcmd_SetStatusIconOffset, -2, 14, 80)
                USER_FUNC(btlevtcmd_SetCutBaseOffset, -2, 0, 40, 0)
                USER_FUNC(btlevtcmd_SetKissHitOffset, -2, 10, 52, 0)
        END_SWITCH()
        USER_FUNC(btlevtcmd_OffPartsAttribute, LW(10), LW(12), 9)
        USER_FUNC(btlevtcmd_OnPartsAttribute, LW(10), LW(12), 0x301'0000)
        USER_FUNC(btlevtcmd_OnPartsAttribute, LW(10), LW(11), 9)
        USER_FUNC(btlevtcmd_OffPartsAttribute, LW(10), LW(11), 0x101'0000)
        USER_FUNC(btlevtcmd_GetPos, LW(10), LW(0), LW(1), LW(2))
        BROTHER_EVT()
            WAIT_FRM(4)
            RUN_CHILD_EVT(PTR(&unitPokey_body_fall_event))
        END_BROTHER()
        USER_FUNC(btlevtcmd_GetPos, LW(10), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_SetPartsPos, LW(10), 5, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_OffPartsAttribute, LW(10), 5, 0x100'0000)
        USER_FUNC(btlevtcmd_CheckDamagePattern, LW(10), 15, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            SET(LW(13), PTR(&tot::party_mario::customWeapon_SuperHammerRecoil))
        ELSE()
            SET(LW(13), PTR(&tot::party_mario::customWeapon_UltraHammerRecoil))
        END_IF()
        USER_FUNC(btlevtcmd_GetFriendBelong, LW(10), LW(0))
        USER_FUNC(btlevtcmd_SamplingEnemy, LW(10), LW(0), LW(13))
        USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
LBL(0)
        IF_EQUAL(LW(3), -1)
            GOTO(50)
        END_IF()
        USER_FUNC(btlevtcmd_GetUnitId, -2, LW(10))
        IF_NOT_EQUAL(LW(10), LW(3))
            USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
            GOTO(0)
        END_IF()
LBL(10)
        USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
        IF_EQUAL(LW(3), -1)
            GOTO(50)
        END_IF()
        USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetHomePos, LW(10), EVT_NULLPTR, LW(1), EVT_NULLPTR)
        ADD(LW(1), 15)
        USER_FUNC(btlevtcmd_JumpPartsSetting, LW(10), 5, 0, FLOAT(5.0), FLOAT(0.01))
        USER_FUNC(btlevtcmd_JumpPartsPosition, LW(10), 5, LW(0), LW(1), LW(2), 0, -1)
        USER_FUNC(btlevtcmd_CheckDamage, LW(10), LW(3), LW(4), LW(13), 256, LW(5))
        GOTO(10)
LBL(50)
        USER_FUNC(btlevtcmd_GetStageSize, LW(5), EVT_NULLPTR, EVT_NULLPTR)
        SET(LW(0), 0)
        USER_FUNC(btlevtcmd_FaceDirectionSub, LW(10), LW(0), LW(5))
        USER_FUNC(btlevtcmd_GetPos, LW(10), EVT_NULLPTR, LW(1), EVT_NULLPTR)
        USER_FUNC(btlevtcmd_JumpPartsSetting, LW(10), 5, 0, FLOAT(5.0), FLOAT(0.01))
        USER_FUNC(btlevtcmd_JumpPartsPosition, LW(10), 5, LW(0), LW(1), LW(2), 0, -1)
        USER_FUNC(btlevtcmd_OnPartsAttribute, LW(10), 5, 0x100'0000)

        // TODO: Surely this isn't necessary for a part???
        // USER_FUNC(btlevtcmd_CheckDamagePattern, LW(10), 16, LW(5))
        // IF_NOT_EQUAL(LW(5), 0)
        //     USER_FUNC(btlevtcmd_CheckDamage, -2, LW(10), LW(11), PTR(&marioWeapon_UltraHammerFinish), 131328, LW(5))
        // END_IF()
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitPokey_damage_body_swallowed_event)
    SET(LW(14), 39)
    SET(LW(15), -1)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_NumSegments, LW(0))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(btlevtcmd_GetDamage, LW(10), LW(0))
        IF_LARGE_EQUAL(LW(0), 1)
            USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(10), LW(11), LW(14))
            WAIT_FRM(30)
            IF_NOT_EQUAL(LW(15), -1)
                USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(10), LW(11), LW(15))
                WAIT_FRM(30)
            END_IF()
        ELSE()
            RUN_CHILD_EVT(PTR(&subsetevt_nodamage_motion))
        END_IF()
    ELSE()
        WAIT_FRM(3)
        SET(LW(12), LW(11))
        USER_FUNC(btlevtcmd_GetUnitWork, LW(10), UW_NumSegments, LW(0))
        SUB(LW(0), 1)
        USER_FUNC(btlevtcmd_SetUnitWork, LW(10), UW_NumSegments, LW(0))
        RUN_CHILD_EVT(PTR(&unitPokey_set_body_params))
        SWITCH(LW(0))
            CASE_EQUAL(1)
                SET(LW(11), 4)
                USER_FUNC(btlevtcmd_SetSwallowParam, -2, 50)
                USER_FUNC(btlevtcmd_SetSwallowAttribute, -2, 2)
                USER_FUNC(btlevtcmd_SetStatusIconOffset, -2, 14, 20)
                USER_FUNC(btlevtcmd_SetCutBaseOffset, -2, 0, 10, 0)
                USER_FUNC(btlevtcmd_SetKissHitOffset, -2, 10, 16, 0)
            CASE_EQUAL(2)
                SET(LW(11), 3)
                USER_FUNC(btlevtcmd_SetStatusIconOffset, -2, 14, 40)
                USER_FUNC(btlevtcmd_SetCutBaseOffset, -2, 0, 20, 0)
                USER_FUNC(btlevtcmd_SetKissHitOffset, -2, 10, 26, 0)
            CASE_EQUAL(3)
                SET(LW(11), 2)
                USER_FUNC(btlevtcmd_SetStatusIconOffset, -2, 14, 60)
                USER_FUNC(btlevtcmd_SetCutBaseOffset, -2, 0, 30, 0)
                USER_FUNC(btlevtcmd_SetKissHitOffset, -2, 10, 36, 0)
            CASE_ETC()
                SET(LW(11), 1)
                USER_FUNC(btlevtcmd_SetStatusIconOffset, -2, 14, 80)
                USER_FUNC(btlevtcmd_SetCutBaseOffset, -2, 0, 40, 0)
                USER_FUNC(btlevtcmd_SetKissHitOffset, -2, 10, 52, 0)
        END_SWITCH()
        BROTHER_EVT()
            WAIT_FRM(3)
            USER_FUNC(btlevtcmd_OffPartsAttribute, LW(10), LW(12), 9)
            USER_FUNC(btlevtcmd_OnPartsAttribute, LW(10), LW(12), 0x301'0000)
            USER_FUNC(btlevtcmd_OnPartsAttribute, LW(10), LW(11), 9)
            USER_FUNC(btlevtcmd_OffPartsAttribute, LW(10), LW(11), 0x101'0000)
            USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(10), LW(11), LW(14))
            WAIT_FRM(1)
            RUN_CHILD_EVT(PTR(&unitPokey_body_fall_event))
        END_BROTHER()
        USER_FUNC(btlevtcmd_GetPos, LW(10), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_SetPartsPos, LW(10), 5, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_OffPartsAttribute, LW(10), 5, 0x100'0000)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 25)
        USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_DivePartsPosition, -2, 5, LW(0), LW(1), LW(2), 5, 0, 0, 0, -1)
        USER_FUNC(btlevtcmd_OnPartsAttribute, LW(10), 5, 0x100'0000)
        WAIT_FRM(20)
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitPokey_spiky_counter_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(10), LW(11), 47)
    USER_FUNC(btlevtcmd_GetPos, LW(10), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHeight, LW(10), LW(3))
    USER_FUNC(btlevtcmd_GetStatusMg, LW(10), LW(4))
    MULF(LW(3), LW(4))
    ADD(LW(1), LW(3))
    ADD(LW(1), 5)
    ADD(LW(2), 10)
    USER_FUNC(evt_eff, PTR(""), PTR("toge_flush"), 0, LW(0), LW(1), LW(2), 60, 0, 0, 0, 0, 0, 0, 0)
    WAIT_FRM(60)
    RETURN()
EVT_END()

EVT_BEGIN(unitPokey_bounce_event)
LBL(0)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    SET(LW(11), LW(5))
    SUB(LW(11), LW(0))
    SET(LW(12), LW(11))
    IF_SMALL(LW(12), 0)
        MUL(LW(12), -1)
    END_IF()
    IF_LARGE(LW(12), LW(8))
        IF_LARGE_EQUAL(LW(11), 0)
            ADD(LW(0), LW(8))
        ELSE()
            SUB(LW(0), LW(8))
        END_IF()
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_SAMBO_MOVE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(6), LW(2), 0, -1)
        GOTO(0)
    ELSE()
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_SAMBO_MOVE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(5), LW(6), LW(7), 0, -1)
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitPokey_body_fall_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(10), LW(11), LW(14))
    USER_FUNC(btlevtcmd_GetPos, LW(10), LW(0), LW(1), LW(2))
    ADD(LW(1), 18)
    USER_FUNC(btlevtcmd_SetPos, LW(10), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetPos, LW(10), LW(0), LW(1), LW(2))
    SET(LW(3), LW(1))
    WAIT_FRM(2)
    SUB(LW(3), 1)
    USER_FUNC(btlevtcmd_SetPos, LW(10), LW(0), LW(3), LW(2))
    WAIT_FRM(3)
    SUB(LW(3), 2)
    USER_FUNC(btlevtcmd_SetPos, LW(10), LW(0), LW(3), LW(2))
    WAIT_FRM(2)
    SUB(LW(3), 3)
    USER_FUNC(btlevtcmd_SetPos, LW(10), LW(0), LW(3), LW(2))
    WAIT_FRM(1)
    SUB(LW(3), 5)
    USER_FUNC(btlevtcmd_SetPos, LW(10), LW(0), LW(3), LW(2))
    WAIT_FRM(1)
    SUB(LW(3), 7)
    USER_FUNC(btlevtcmd_SetPos, LW(10), LW(0), LW(3), LW(2))
    WAIT_FRM(1)
    ADD(LW(3), 2)
    USER_FUNC(btlevtcmd_SetPos, LW(10), LW(0), LW(3), LW(2))
    WAIT_FRM(1)
    ADD(LW(3), 3)
    USER_FUNC(btlevtcmd_SetPos, LW(10), LW(0), LW(3), LW(2))
    WAIT_FRM(1)
    ADD(LW(3), 3)
    USER_FUNC(btlevtcmd_SetPos, LW(10), LW(0), LW(3), LW(2))
    WAIT_FRM(1)
    SUB(LW(3), 3)
    USER_FUNC(btlevtcmd_SetPos, LW(10), LW(0), LW(3), LW(2))
    WAIT_FRM(1)
    SUB(LW(3), 3)
    USER_FUNC(btlevtcmd_SetPos, LW(10), LW(0), LW(3), LW(2))
    WAIT_FRM(1)
    SUB(LW(3), 2)
    USER_FUNC(btlevtcmd_SetPos, LW(10), LW(0), LW(3), LW(2))
    RETURN()
EVT_END()

EVT_BEGIN(unitPokey_wait_event)
    SET(LW(10), -2)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_NumSegments, LW(0))
    SWITCH(LW(0))
        CASE_EQUAL(1)
            SET(LW(11), 4)
        CASE_EQUAL(2)
            SET(LW(11), 3)
        CASE_EQUAL(3)
            SET(LW(11), 2)
        CASE_ETC()
            SET(LW(11), 1)
    END_SWITCH()
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, LW(10), LW(11))
    RETURN()
EVT_END()

EVT_BEGIN(unitPokey_common_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitPokey_wait_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitPokey_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitPokey_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitPokey_attack_event))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_NumSegments, 4)
    RUN_CHILD_EVT(PTR(&unitPokey_set_body_params))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_ChildLevel, 0)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 5, 69)
    USER_FUNC(btlevtcmd_SetRunSound, -2, PTR("SFX_ENM_SAMBO_MOVE1"), PTR("SFX_ENM_SAMBO_MOVE1"), 0, 6, 6)
    USER_FUNC(btlevtcmd_SetWalkSound, -2, PTR("SFX_ENM_SAMBO_MOVE1"), PTR("SFX_ENM_SAMBO_MOVE1"), 0, 6, 6)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitPokey_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::POKEY)
    RUN_CHILD_EVT(unitPokey_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitPoisonPokey_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::POISON_POKEY)
    RUN_CHILD_EVT(unitPokey_common_init_event)
    RETURN()
EVT_END()

}  // namespace mod::tot::custom