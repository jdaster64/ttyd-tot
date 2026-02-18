#include "tot_custom_rel.h"     // For externed unit definitions.

#include "evt_cmd.h"

#include <gc/mtx.h>
#include <gc/types.h>
#include <ttyd/_core_language_libs.h>
#include <ttyd/animdrv.h>
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
#include <cstring>

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
constexpr const int32_t UW_FuseLit = 0;
constexpr const int32_t UW_FuseLength = 1;
constexpr const int32_t UW_FuseSfx = 2;
constexpr const int32_t UW_BuffCount = 3;
constexpr const int32_t UW_BattleUnitType = 4;

}  // namespace

// Evt / Function declarations.

extern const int32_t unitBulkyBobOmb_init_event[];
extern const int32_t unitBobUlk_init_event[];
extern const int32_t unitBulkyBobOmb_common_init_event[];
extern const int32_t unitBulkyBobOmb_attack_event[];
extern const int32_t unitBulkyBobOmb_damage_event[];
extern const int32_t unitBulkyBobOmb_phase_event[];
extern const int32_t unitBulkyBobOmb_wait_event[];
extern const int32_t unitBulkyBobOmb_dead_event[];
extern const int32_t unitBulkyBobOmb_explosion_event[];

EVT_DECLARE_USER_FUNC(_set_draw_callback, 0)

// Unit data.

int8_t unitBulkyBobOmb_defense[] = { 1, 1, 1, 1, 1 };
int8_t unitBulkyBobOmb_defense_attr[] = { 0, 0, 0, 0, 0 };

StatusVulnerability unitBulkyBobOmb_status = {
    90,  75,  70,   0,  70, 150, 100, 100,
   100,  80, 100,  80, 100,  95,  70,  80,
    20, 100,  70, 100, 100,  95,
};
StatusVulnerability unitBobUlk_status = {
    70,  55,  50, 100,  50, 100, 100,  90,
   100,  70, 100,  70, 100,  85,  50,  60,
    10, 100,  50, 100, 100,  60,
};

PoseTableEntry unitBulkyBobOmb_pose_table_3[] = {
    1, "N_1",
    9, "Y_1",
    2, "Y_1",
    3, "K_1",
    4, "X_1",
    5, "K_1",
    9, "Z_1",
    28, "S_1",
    29, "D_1",
    35, "S_1",
    31, "S_1",
    39, "D_1",
    56, "X_1",
    57, "X_1",
    65, "S_1",
    69, "S_1",
};
PoseTableEntry unitBulkyBobOmb_pose_table_3_fire[] = {
    1, "N_1",
    9, "Y_1",
    2, "Y_1",
    3, "K_1",
    4, "X_1",
    5, "K_1",
    9, "Z_1",
    28, "S_1",
    29, "D_1",
    35, "S_1",
    31, "S_1",
    39, "D_1",
    56, "X_1",
    57, "X_1",
    65, "S_1",
    69, "S_1",
};
PoseTableEntry unitBulkyBobOmb_pose_table_3_to_2[] = {
    1, "B_4",
    9, "B_1",
    2, "B_1",
    3, "B_7",
    4, "B_10",
    5, "B_7",
    28, "B_1",
    31, "B_1",
    39, "B_1",
    56, "B_10",
    57, "B_10",
    65, "B_1",
    69, "B_1",
};
PoseTableEntry unitBulkyBobOmb_pose_table_2[] = {
    1, "N_2",
    9, "Y_2",
    2, "Y_2",
    3, "K_2",
    4, "X_2",
    5, "K_2",
    9, "Z_2B",
    28, "S_2B",
    29, "D_2",
    35, "S_2B",
    31, "S_2B",
    39, "D_2",
    56, "X_2",
    57, "X_2",
    65, "S_2B",
    69, "S_2B",
};
PoseTableEntry unitBulkyBobOmb_pose_table_2_fire[] = {
    1, "N_2",
    9, "Y_2",
    2, "Y_2",
    3, "K_2",
    4, "X_2",
    5, "K_2",
    9, "Z_2A",
    28, "S_2A",
    29, "D_2",
    35, "S_2A",
    31, "S_2A",
    39, "D_2",
    56, "X_2",
    57, "X_2",
    65, "S_2A",
    69, "S_2A",
};
PoseTableEntry unitBulkyBobOmb_pose_table_2_to_1[] = {
    1, "B_5",
    9, "B_2",
    2, "B_2",
    3, "B_8",
    4, "B_11",
    5, "B_8",
    28, "B_2",
    31, "B_2",
    39, "B_2",
    56, "B_11",
    57, "B_11",
    65, "B_2",
    69, "B_2",
};
PoseTableEntry unitBulkyBobOmb_pose_table_1[] = {
    1, "N_3",
    9, "Y_3",
    2, "Y_3",
    3, "K_3",
    4, "X_3",
    5, "K_3",
    9, "Z_3B",
    28, "S_3B",
    29, "D_3",
    35, "S_3B",
    31, "S_3B",
    39, "D_3",
    56, "X_3",
    57, "X_3",
    65, "S_3B",
    69, "S_3B",
};
PoseTableEntry unitBulkyBobOmb_pose_table_1_fire[] = {
    1, "N_3",
    9, "Y_3",
    2, "Y_3",
    3, "K_3",
    4, "X_3",
    5, "K_3",
    9, "Z_3A",
    28, "S_3A",
    29, "D_3",
    35, "S_3A",
    31, "S_3A",
    39, "D_3",
    56, "X_3",
    57, "X_3",
    65, "S_3A",
    69, "S_3A",
};
PoseTableEntry unitBulkyBobOmb_pose_table_1_to_0[] = {
    1, "B_6",
    9, "B_3",
    2, "B_3",
    3, "B_9",
    4, "B_12",
    5, "B_9",
    28, "B_3",
    31, "B_3",
    39, "B_3",
    56, "B_12",
    57, "B_12",
    65, "B_3",
    69, "B_3",
};
PoseTableEntry unitBulkyBobOmb_pose_table_0[] = {
    1, "N_4",
    9, "Y_4",
    2, "Y_4",
    3, "K_4",
    4, "X_4",
    5, "K_4",
    9, "Z_4B",
    28, "S_4B",
    29, "D_4",
    35, "S_4B",
    31, "S_4B",
    39, "D_4",
    56, "X_4",
    57, "X_4",
    65, "S_4B",
    69, "S_4B",
};
PoseTableEntry unitBulkyBobOmb_pose_table_0_fire[] = {
    1, "N_4",
    9, "Y_4",
    2, "Y_4",
    3, "K_4",
    4, "X_4",
    5, "K_4",
    9, "Z_4A",
    28, "S_4A",
    29, "D_4",
    35, "S_4A",
    31, "S_4A",
    39, "D_4",
    56, "X_4",
    57, "X_4",
    65, "S_4A",
    69, "S_4A",
};

DataTableEntry unitBulkyBobOmb_data_table[] = {
    49, (void*)unitBulkyBobOmb_dead_event,
    52, (void*)unitBulkyBobOmb_explosion_event,
    0, nullptr,
};

const PoseSoundTimingEntry unitBulkyBobOmb_pose_sound_timing_table[] = {
    { "S_3A", 0.0100000f, 0, "SFX_ENM_HEAVYB_WAIT3", 1 },
    { "S_3A", 0.2666668f, 0, "SFX_ENM_HEAVYB_WAIT3", 1 },
    { "S_3A", 0.5333336f, 0, "SFX_ENM_HEAVYB_WAIT3", 1 },
    { "S_3A", 0.8000000f, 0, "SFX_ENM_HEAVYB_WAIT3", 1 },
    { "S_4A", 0.0100000f, 0, "SFX_ENM_HEAVYB_WAIT3", 1 },
    { "S_4A", 0.1666667f, 0, "SFX_ENM_HEAVYB_WAIT3", 1 },
    { "S_4A", 0.3333334f, 0, "SFX_ENM_HEAVYB_WAIT3", 1 },
    { "S_4A", 0.5000000f, 0, "SFX_ENM_HEAVYB_WAIT3", 1 },
    { nullptr, 0.0f, 0, nullptr, 1 },
};

BattleWeapon unitBulkyBobOmb_weaponBomb = {
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
        AttackTargetClass_Flags::MULTIPLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::EXPLOSION,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::FLIPS_BOMB |
        AttackSpecialProperty_Flags::UNGUARDABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = nullptr,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 50,
    .bg_a2_fall_weight = 50,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 0,
    .nozzle_fire_chance = 0,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};
BattleWeapon unitBulkyBobOmb_weaponBuff1 = {
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
        AttackTargetClass_Flags::MULTIPLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_SAME_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::CANNOT_MISS |
        AttackSpecialProperty_Flags::UNGUARDABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = 0,
        
    // status chances
    .def_change_chance = 100,
    .def_change_time = 3,
    .def_change_strength = 3,
    
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
BattleWeapon unitBulkyBobOmb_weaponBuff2 = {
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
        AttackTargetClass_Flags::MULTIPLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_SAME_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::CANNOT_MISS |
        AttackSpecialProperty_Flags::UNGUARDABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = 0,
        
    // status chances
    .atk_change_chance = 100,
    .atk_change_time = 3,
    .atk_change_strength = 3,
    
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
BattleWeapon unitBulkyBobOmb_weaponBuff3 = {
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
        AttackTargetClass_Flags::MULTIPLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_SAME_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::CANNOT_MISS |
        AttackSpecialProperty_Flags::UNGUARDABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = 0,
        
    // status chances
    .charge_strength = 3,
    
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
BattleWeapon unitBobUlk_weaponBomb = {
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
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::EXPLOSION,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::FLIPS_BOMB |
        AttackSpecialProperty_Flags::UNGUARDABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = nullptr,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 50,
    .bg_a2_fall_weight = 50,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 0,
    .nozzle_fire_chance = 0,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};
BattleWeapon unitBobUlk_weaponBuff1 = {
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
        AttackTargetClass_Flags::MULTIPLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_SAME_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::CANNOT_MISS |
        AttackSpecialProperty_Flags::UNGUARDABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = 0,
        
    // status chances
    .def_change_chance = 100,
    .def_change_time = 3,
    .def_change_strength = 3,
    
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
BattleWeapon unitBobUlk_weaponBuff2 = {
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
        AttackTargetClass_Flags::MULTIPLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_SAME_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::CANNOT_MISS |
        AttackSpecialProperty_Flags::UNGUARDABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = 0,
        
    // status chances
    .atk_change_chance = 100,
    .atk_change_time = 3,
    .atk_change_strength = 3,
    
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
BattleWeapon unitBobUlk_weaponBuff3 = {
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
        AttackTargetClass_Flags::MULTIPLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_SAME_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::CANNOT_MISS |
        AttackSpecialProperty_Flags::UNGUARDABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = 0,
        
    // status chances
    .charge_strength = 9,
    
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

BattleUnitKindPart unitBulkyBobOmb_parts[] = {
    {
        .index = 1,
        .name = "btl_un_heavy_bom",
        .model_name = "c_heavy",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 50.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 65.0f, 0.0f },
        .unk_30 = 65,
        .unk_32 = 70,
        .base_alpha = 255,
        .defense = unitBulkyBobOmb_defense,
        .defense_attr = unitBulkyBobOmb_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitBulkyBobOmb_pose_table_3,
    },
    {
        .index = 2,
        .name = "btl_un_heavy_bom",
        .model_name = "c_heavy",
        .part_offset_pos = { 0.0f, 80.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 15,
        .unk_32 = 15,
        .base_alpha = 255,
        .defense = unitBulkyBobOmb_defense,
        .defense_attr = unitBulkyBobOmb_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_200_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitBulkyBobOmb_pose_table_3,
    },
};
BattleUnitKindPart unitBobUlk_parts[] = {
    {
        .index = 1,
        .name = "btl_un_giant_bomb",
        .model_name = "c_giant",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 50.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 65.0f, 0.0f },
        .unk_30 = 65,
        .unk_32 = 70,
        .base_alpha = 255,
        .defense = unitBulkyBobOmb_defense,
        .defense_attr = unitBulkyBobOmb_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitBulkyBobOmb_pose_table_3,
    },
    {
        .index = 2,
        .name = "btl_un_giant_bomb",
        .model_name = "c_giant",
        .part_offset_pos = { 0.0f, 80.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 15,
        .unk_32 = 15,
        .base_alpha = 255,
        .defense = unitBulkyBobOmb_defense,
        .defense_attr = unitBulkyBobOmb_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_200_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitBulkyBobOmb_pose_table_3,
    },
};

BattleUnitKind unit_BulkyBobOmb = {
    .unit_type = BattleUnitType::BULKY_BOB_OMB,
    .unit_name = "btl_un_heavy_bom",
    .max_hp = 5,
    .max_fp = 0,
    .danger_hp = 1,
    .peril_hp = 1,
    .level = 1,
    .bonus_exp = 0,
    .bonus_coin = 0,
    .bonus_coin_rate = 100,
    .base_coin = 0,
    .run_rate = 40,
    .pb_soft_cap = 9999,
    .width = 54,
    .height = 70,
    .hit_offset = { 7, 70 },
    .center_offset = { 0.0f, 35.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 27.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 27.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 27.0f, 45.5f, 0.0f },
    .cut_base_offset = { 0.0f, 35.0f, 0.0f },
    .cut_width = 54.0f,
    .cut_height = 70.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_HEAVYB_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitBulkyBobOmb_status,
    .num_parts = 2,
    .parts = unitBulkyBobOmb_parts,
    .init_evt_code = (void*)unitBulkyBobOmb_init_event,
    .data_table = unitBulkyBobOmb_data_table,
};
BattleUnitKind unit_BobUlk = {
    .unit_type = BattleUnitType::BOB_ULK,
    .unit_name = "btl_un_giant_bomb",
    .max_hp = 5,
    .max_fp = 0,
    .danger_hp = 1,
    .peril_hp = 1,
    .level = 1,
    .bonus_exp = 0,
    .bonus_coin = 0,
    .bonus_coin_rate = 100,
    .base_coin = 0,
    .run_rate = 40,
    .pb_soft_cap = 9999,
    .width = 54,
    .height = 70,
    .hit_offset = { 7, 70 },
    .center_offset = { 0.0f, 35.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 27.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 27.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 27.0f, 45.5f, 0.0f },
    .cut_base_offset = { 0.0f, 35.0f, 0.0f },
    .cut_width = 54.0f,
    .cut_height = 70.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_HEAVYB_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitBobUlk_status,
    .num_parts = 2,
    .parts = unitBobUlk_parts,
    .init_evt_code = (void*)unitBobUlk_init_event,
    .data_table = unitBulkyBobOmb_data_table,
};

// Function / USER_FUNC definitions.

void _fire_point_callback(BattleWorkUnitPart* part, int32_t group_idx) {
    auto* unit = part->unit_owner;
    if (unit->unit_work[0] == 0) return;
    
    const char* comp_group_name;
    switch (unit->unit_work[1]) {
        case 0:     comp_group_name = "locator11";  break;
        case 1:     comp_group_name = "locator10";  break;
        case 2:     comp_group_name = "locator9";   break;
        default:    comp_group_name = "locator8";   break;
    }
    
    const char* group_name = ttyd::animdrv::animPoseGetGroupName(
        part->anim_pose_id, group_idx);
        
    if (strcmp(comp_group_name, group_name) == 0) {
        auto* part2 = ttyd::battle_unit::BtlUnit_GetPartsPtr(unit, 2);
        int32_t pose_id = part2->anim_pose_id;
        int32_t shape_idx = ttyd::animdrv::animPoseGetShapeIdx(
            pose_id, "pPlaneShape3");
        ttyd::animdrv::animPoseDrawShape(pose_id, shape_idx);
    }
}

EVT_DEFINE_USER_FUNC(_set_draw_callback) {
    int32_t id = ttyd::battle_sub::BattleTransID(evt, -2);
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, id);
    auto* part = ttyd::battle_unit::BtlUnit_GetPartsPtr(unit, 1);
    ttyd::animdrv::animPoseSetDispCallBack(
        part->anim_pose_id, (void*)_fire_point_callback, (void*)part);

    part = ttyd::battle_unit::BtlUnit_GetPartsPtr(unit, 2);
    part->part_attribute_flags |= PartsAttribute_Flags::UNK_100_0000;
    return 2;
}

// Evt definitions.

EVT_BEGIN(unitBulkyBobOmb_attack_event)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_HEAVYB_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_FuseLit, LW(0))
    IF_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_FuseLength, LW(0))
        SWITCH(LW(0))
            CASE_EQUAL(3)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("G_1"))
            CASE_EQUAL(2)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("G_2"))
            CASE_EQUAL(1)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("G_3"))
            CASE_EQUAL(0)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("G_4"))
        END_SWITCH()
        WAIT_FRM(40)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_HEAVYB_WAIT1"), EVT_NULLPTR, 0, LW(0))
        USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_FuseSfx, LW(0))
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_HEAVYB_ATTACK2"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_FuseLit, 1)
        USER_FUNC(btlevtcmd_OnUnitFlag, -2, 16)
    ELSE()
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_FuseLength, LW(0))
        SWITCH(LW(0))
            CASE_EQUAL(3)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("G_1"))
            CASE_EQUAL(2)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("G_2"))
            CASE_EQUAL(1)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("G_3"))
            CASE_EQUAL(0)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("G_4"))
        END_SWITCH()
        WAIT_FRM(40)

        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(0))
        IF_EQUAL(LW(0), (int32_t)BattleUnitType::BULKY_BOB_OMB)
            SET(LW(10), PTR(&unitBulkyBobOmb_weaponBuff1))
            SET(LW(11), PTR(&unitBulkyBobOmb_weaponBuff2))
            SET(LW(12), PTR(&unitBulkyBobOmb_weaponBuff3))
        ELSE()
            SET(LW(10), PTR(&unitBobUlk_weaponBuff1))
            SET(LW(11), PTR(&unitBobUlk_weaponBuff2))
            SET(LW(12), PTR(&unitBobUlk_weaponBuff3))
        END_IF()

        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BuffCount, LW(0))
        SWITCH(LW(0))
            CASE_EQUAL(0)
                SET(LW(12), LW(10))
                USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BuffCount, 1)
            CASE_EQUAL(1)
                SET(LW(12), LW(11))
                USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BuffCount, 2)
            CASE_EQUAL(2)
                USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BuffCount, 0)
        END_SWITCH()

        USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
        USER_FUNC(btlevtcmd_CheckDamage, -2, -2, 1, LW(12), 256, LW(5))
    END_IF()
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitBulkyBobOmb_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_CheckStatus, -2, 9, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_FuseLit, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_FuseSfx, LW(0))
            USER_FUNC(evt_snd_sfxoff, LW(0))
            USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_FuseSfx, -1)
            USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_FuseLit, 0)
            USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_FuseLength, LW(0))
            SWITCH(LW(0))
                CASE_EQUAL(2)
                    USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitBulkyBobOmb_pose_table_2))
                CASE_EQUAL(1)
                    USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitBulkyBobOmb_pose_table_1))
                CASE_EQUAL(0)
                    USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitBulkyBobOmb_pose_table_0))
            END_SWITCH()
        END_IF()
    END_IF()
    USER_FUNC(btlevtcmd_CheckDamageCode, LW(10), 256, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_GetDamageCode, LW(10), LW(0))
        SWITCH(LW(0))
            CASE_OR(27)
            CASE_OR(24)
            CASE_OR(25)
            CASE_OR(52)
            CASE_OR(46)
            CASE_OR(50)
            CASE_OR(54)
                GOTO(50)
                CASE_END()
        END_SWITCH()
    END_IF()
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    RETURN()
LBL(50)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 29)
    RUN_CHILD_EVT(PTR(&unitBulkyBobOmb_dead_event))
    RETURN()
EVT_END()

EVT_BEGIN(unitBulkyBobOmb_dead_event)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_FuseSfx, LW(0))
    IF_NOT_EQUAL(LW(0), -1)
        USER_FUNC(evt_snd_sfxoff, LW(0))
    END_IF()
    USER_FUNC(btlevtcmd_GetDamageCode, LW(10), LW(0))
    SWITCH(LW(0))
        CASE_OR(27)
        CASE_OR(24)
        CASE_OR(25)
        CASE_OR(52)
        CASE_OR(46)
        CASE_OR(50)
        CASE_OR(54)
            GOTO(50)
            CASE_END()
    END_SWITCH()
LBL(10)
    SET(LW(10), -2)
    RUN_CHILD_EVT(PTR(&subsetevt_dead_core))
    RETURN()
LBL(50)
    USER_FUNC(btlevtcmd_AfterReactionEntry, -2, 52)
    RETURN()
EVT_END()

EVT_BEGIN(unitBulkyBobOmb_explosion_event)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(0))
    IF_EQUAL(LW(0), (int32_t)BattleUnitType::BULKY_BOB_OMB)
        SET(LW(9), PTR(&unitBulkyBobOmb_weaponBomb))
    ELSE()
        SET(LW(9), PTR(&unitBobUlk_weaponBomb))
    END_IF()

    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_FuseSfx, LW(0))
    USER_FUNC(evt_snd_sfxoff, LW(0))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_FuseSfx, -1)
    USER_FUNC(evt_btl_camera_shake_h, 0, 10, 0, 60, 4)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_HEAVYB_ATTACK3"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHeight, -2, LW(5))
    DIV(LW(5), 2)
    USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(6))
    MULF(LW(5), LW(6))
    ADD(LW(1), LW(5))
    USER_FUNC(evt_eff, 0, PTR("bomb"), 5, LW(0), LW(1), LW(2), FLOAT(2.0), 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(btlevtcmd_StageDispellFog)
    WAIT_MSEC(300)
    USER_FUNC(evt_eff, 0, PTR("bomb"), 4, -40, 30, 20, FLOAT(2.0), 0, 0, 0, 0, 0, 0, 0)
    WAIT_MSEC(125)
    USER_FUNC(evt_eff, 0, PTR("bomb"), 4, 40, 90, 20, FLOAT(2.0), 0, 0, 0, 0, 0, 0, 0)
    WAIT_MSEC(125)
    USER_FUNC(evt_eff, 0, PTR("bomb"), 4, 40, 30, 20, FLOAT(2.0), 0, 0, 0, 0, 0, 0, 0)
    WAIT_MSEC(125)
    USER_FUNC(evt_eff, 0, PTR("bomb"), 4, -40, 90, 20, FLOAT(2.0), 0, 0, 0, 0, 0, 0, 0)
    WAIT_MSEC(125)
    USER_FUNC(evt_eff, 0, PTR("bomb"), 4, -100, 60, 20, FLOAT(2.0), 0, 0, 0, 0, 0, 0, 0)
    WAIT_MSEC(125)
    USER_FUNC(evt_eff, 0, PTR("bomb"), 4, 100, 60, 20, FLOAT(2.0), 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(9))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(9), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    SET(LW(10), 0)
LBL(10)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    IF_EQUAL(LW(5), 1)
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    END_IF()
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    IF_NOT_EQUAL(LW(3), -1)
        ADD(LW(10), 1)
        GOTO(10)
    END_IF()
LBL(99)
    SET(LW(10), -2)
    RUN_CHILD_EVT(PTR(&subsetevt_dead_core))
    USER_FUNC(btlevtcmd_KillUnit, -2, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitBulkyBobOmb_phase_event)
    USER_FUNC(btlevtcmd_CheckPhase, LW(0), 0x400'0003)
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_FuseLit, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            USER_FUNC(btlevtcmd_PhaseEventStartDeclare, -2)
            USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_FuseLength, LW(0))
            IF_EQUAL(LW(0), 0)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("K_4"))
                USER_FUNC(btlevtcmd_AfterReactionEntry, -2, 52)
                RETURN()
            END_IF()
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_HEAVYB_WAIT2"), EVT_NULLPTR, 0, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_FuseLength, LW(0))
            SWITCH(LW(0))
                CASE_EQUAL(3)
                    USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitBulkyBobOmb_pose_table_3_to_2))
                CASE_EQUAL(2)
                    USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitBulkyBobOmb_pose_table_2_to_1))
                CASE_EQUAL(1)
                    USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitBulkyBobOmb_pose_table_1_to_0))
            END_SWITCH()
            USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
            WAIT_FRM(40)
            USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_FuseLength, LW(0))
            SUB(LW(0), 1)
            USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_FuseLength, LW(0))
            SWITCH(LW(0))
                CASE_EQUAL(2)
                    USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitBulkyBobOmb_pose_table_2_fire))
                CASE_EQUAL(1)
                    USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitBulkyBobOmb_pose_table_1_fire))
                CASE_EQUAL(0)
                    USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitBulkyBobOmb_pose_table_0_fire))
            END_SWITCH()
            USER_FUNC(btlevtcmd_StartWaitEvent, -2)
            RETURN()
        END_IF()
    END_IF()
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitBulkyBobOmb_wait_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitBulkyBobOmb_common_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitBulkyBobOmb_wait_event))
    USER_FUNC(btlevtcmd_SetEventPhase, -2, PTR(&unitBulkyBobOmb_phase_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitBulkyBobOmb_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitBulkyBobOmb_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&btldefaultevt_Confuse))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_FuseLit, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_FuseLength, 3)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_FuseSfx, -1)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BuffCount, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 2, PTR("F_1"))
    USER_FUNC(btlevtcmd_AnimeSetPoseSoundTable, -2, 1, PTR(&unitBulkyBobOmb_pose_sound_timing_table))
    USER_FUNC(_set_draw_callback)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitBulkyBobOmb_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::BULKY_BOB_OMB)
    RUN_CHILD_EVT(unitBulkyBobOmb_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitBobUlk_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::BOB_ULK)
    RUN_CHILD_EVT(unitBulkyBobOmb_common_init_event)
    RETURN()
EVT_END()

}  // namespace mod::tot::custom