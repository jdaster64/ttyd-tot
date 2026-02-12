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
constexpr const int32_t UW_OnCeiling = 0;
constexpr const int32_t UW_Rotate = 0;
constexpr const int32_t UW_RotateSpeed = 0;
constexpr const int32_t UW_BattleUnitType = 3;

}  // namespace

// Evt declarations.

extern const int32_t unitBuzzyBeetle_init_event[];
extern const int32_t unitSpikeTop_init_event[];
extern const int32_t unitParabuzzy_init_event[];
extern const int32_t unitSpikyParabuzzy_init_event[];
extern const int32_t unitBuzzyBeetle_common_init_event[];
extern const int32_t unitBuzzyBeetle_init_event2[];
extern const int32_t unitBuzzyBeetle_attack_event[];
extern const int32_t unitBuzzyBeetle_damage_event[];
extern const int32_t unitBuzzyBeetle_flip_event[];
extern const int32_t unitBuzzyBeetle_wait_event[];
extern const int32_t unitBuzzyBeetle_wakeup_event[];
extern const int32_t unitBuzzyBeetle_ceil_attack_event[];
extern const int32_t unitBuzzyBeetle_ceil_damage_event[];
extern const int32_t unitBuzzyBeetle_ceil_countered_evt[];
extern const int32_t unitBuzzyBeetle_ceil_fall_ready_event[];
extern const int32_t unitSpikeTop_spiky_counter_event[];
extern const int32_t unitParabuzzy_common_init_event[];
extern const int32_t unitParabuzzy_attack_event[];
extern const int32_t unitParabuzzy_spiky_counter_event[];
extern const int32_t unitParabuzzy_first_attack_pos_event[];
extern const int32_t unitParabuzzy_fall_event[];

// Unit data.

int8_t unitBuzzyBeetle_defense[] = { 4, 99, 4, 99, 4 };
int8_t unitBuzzyBeetle_defense_attr[] = { 0, 2, 0, 2, 0 };
int8_t unitBuzzyBeetle_flip_defense[] = { 0, 0, 0, 0, 0 };
int8_t unitBuzzyBeetle_flip_defense_attr[] = { 0, 0, 0, 0, 0 };

StatusVulnerability unitBuzzyBeetle_status = { 
     75,  90,  75, 100,  75, 100,   0,  75,
    100,  90, 100,  90, 100,  95,  75,  60,
     30, 100,  75, 100, 100,  95,
};
StatusVulnerability unitParabuzzy_status = {
     75,  90, 105, 100,  75, 100,   0,  75,
    100,  90, 100,  90, 100,  95,  75,  60,
     90, 100,  75, 100, 100,  95,
};

PoseTableEntry unitBuzzyBeetle_pose_table[] = {
    1, "MET_N_1",
    2, "MET_Y_1",
    9, "MET_Y_1",
    5, "MET_K_1",
    4, "MET_X_1",
    3, "MET_X_1",
    28, "MET_S_1",
    29, "MET_Q_1",
    30, "MET_Q_1",
    31, "MET_A_1",
    39, "MET_D_1",
    50, "MET_A_1",
    40, "MET_W_1",
    42, "MET_R_1",
    56, "MET_X_1",
    57, "MET_X_1",
    65, "MET_T_1",
    69, "MET_S_1",
};
PoseTableEntry unitBuzzyBeetle_ceil_pose_table[] = {
    1, "SMT_N_1",
    2, "SMT_Y_1",
    9, "SMT_Y_1",
    5, "SMT_K_1",
    4, "SMT_X_1",
    3, "SMT_X_1",
    28, "SMT_S_1",
    29, "SMT_Q_1",
    30, "SMT_Q_1",
    31, "SMT_A_1",
    39, "SMT_D_1",
    50, "SMT_A_1",
    40, "SMT_W_1",
    42, "SMT_R_1",
    56, "SMT_X_1",
    57, "SMT_X_1",
    65, "SMT_S_1",
    69, "SMT_S_1",
};
PoseTableEntry unitBuzzyBeetle_flip_pose_table[] = {
    1, "MET_N_2",
    2, "MET_Y_2",
    9, "MET_Y_2",
    5, "MET_K_2",
    4, "MET_X_2",
    3, "MET_X_2",
    28, "MET_S_2",
    29, "MET_Q_2",
    30, "MET_Q_2",
    31, "MET_A_1",
    39, "MET_D_3",
    50, "MET_A_1",
    40, "MET_W_1",
    56, "MET_X_2",
    57, "MET_X_2",
    65, "MET_S_2",
    69, "MET_S_2",
};
PoseTableEntry unitSpikeTop_pose_table[] = {
    1, "TMT_N_1",
    2, "TMT_Y_1",
    9, "TMT_Y_1",
    5, "TMT_K_1",
    4, "TMT_X_1",
    3, "TMT_X_1",
    28, "TMT_S_1",
    29, "TMT_Q_1",
    30, "TMT_Q_1",
    31, "TMT_A_1",
    39, "TMT_D_1",
    50, "TMT_A_1",
    40, "TMT_W_1",
    42, "TMT_R_1",
    56, "TMT_X_1",
    57, "TMT_X_1",
    69, "TMT_S_1",
};
PoseTableEntry unitSpikeTop_ceil_pose_table[] = {
    1, "STM_N_1",
    2, "STM_Y_1",
    9, "STM_Y_1",
    5, "STM_K_1",
    4, "STM_X_1",
    3, "STM_X_1",
    28, "STM_S_1",
    29, "STM_Q_1",
    30, "STM_Q_1",
    31, "STM_A_1",
    39, "STM_D_1",
    50, "STM_A_1",
    40, "STM_W_1",
    42, "STM_R_1",
    56, "STM_X_1",
    57, "STM_X_1",
    69, "STM_S_1",
};
PoseTableEntry unitSpikeTop_flip_pose_table[] = {
    1, "TMT_Y_2",
    2, "TMT_Y_2",
    9, "TMT_Y_2",
    5, "TMT_S_3",
    4, "TMT_S_3",
    3, "TMT_S_3",
    28, "TMT_S_3",
    29, "TMT_Q_2",
    30, "TMT_Q_2",
    31, "TMT_A_1",
    39, "TMT_S_3",
    40, "TMT_W_1",
    42, "TMT_R_1",
    56, "TMT_S_3",
    57, "TMT_S_3",
    69, "TMT_S_3",
};
PoseTableEntry unitParabuzzy_pose_table[] = {
    1, "PMT_N_1",
    2, "PMT_Y_1",
    9, "PMT_Y_1",
    5, "PMT_K_1",
    4, "PMT_X_1",
    3, "PMT_X_1",
    28, "PMT_S_1",
    29, "PMT_Q_1",
    30, "PMT_Q_1",
    31, "PMT_S_1",
    39, "PMT_D_1",
    50, "PMT_A_1",
    42, "PMT_R_1",
    40, "PMT_W_1",
    56, "PMT_X_1",
    57, "PMT_X_1",
    65, "PMT_S_1",
    69, "PMT_S_1",
};
PoseTableEntry unitSpikyParabuzzy_pose_table[] = {
    1, "PTM_N_1",
    2, "PTM_Y_1",
    9, "PTM_Y_1",
    5, "PTM_K_1",
    4, "PTM_X_1",
    3, "PTM_X_1",
    28, "PTM_S_1",
    29, "PTM_Q_1",
    30, "PTM_Q_1",
    31, "PTM_S_1",
    39, "PTM_D_1",
    50, "PTM_A_1",
    42, "PTM_R_1",
    40, "PTM_W_1",
    56, "PTM_X_1",
    57, "PTM_X_1",
    65, "PTM_T_1",
    69, "PTM_S_1",
};

const PoseSoundTimingEntry unitParabuzzy_pose_sound_timing_table[] = {
    { "PMT_S_1", 0.0100000f, 0, "SFX_ENM_PATAMET_WAIT1", 1 },
    { "PMT_W_1", 0.0100000f, 0, "SFX_ENM_PATAMET_WAIT1", 1 },
    { "PMT_R_1", 0.0100000f, 0, "SFX_ENM_PATAMET_WAIT1", 1 },
    { "PMT_A_1", 0.0100000f, 0, "SFX_ENM_PATAMET_WAIT1", 1 },
    { "PMT_K_1", 0.0100000f, 0, "SFX_ENM_PATAMET_WAIT1", 1 },
    { "PMT_N_1", 0.0100000f, 0, "SFX_ENM_PATAMET_WAIT1", 1 },
    { "PMT_X_1", 0.0100000f, 0, "SFX_ENM_PATAMET_WAIT1", 1 },
    { nullptr, 0.0f, 0, nullptr, 1 },
};
const PoseSoundTimingEntry unitSpikyParabuzzy_pose_sound_timing_table[] = {
    { "PTM_S_1", 0.0100000f, 0, "SFX_ENM_PATAMET_WAIT1", 1 },
    { "PTM_W_1", 0.0100000f, 0, "SFX_ENM_PATAMET_WAIT1", 1 },
    { "PTM_R_1", 0.0100000f, 0, "SFX_ENM_PATAMET_WAIT1", 1 },
    { "PTM_A_1", 0.0100000f, 0, "SFX_ENM_PATAMET_WAIT1", 1 },
    { "PTM_K_1", 0.0100000f, 0, "SFX_ENM_PATAMET_WAIT1", 1 },
    { "PTM_N_1", 0.0100000f, 0, "SFX_ENM_PATAMET_WAIT1", 1 },
    { "PTM_X_1", 0.0100000f, 0, "SFX_ENM_PATAMET_WAIT1", 1 },
    { "PTM_T_1", 0.0100000f, 0, "SFX_ENM_PATAMET_WAIT1", 1 },
    { nullptr, 0.0f, 0, nullptr, 1 },
};

BattleWeapon unitBuzzyBeetle_weaponGrounded = {
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
    .counter_resistance_flags = 
        AttackCounterResistance_Flags::ALL &
        ~AttackCounterResistance_Flags::PAYBACK,
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
BattleWeapon unitBuzzyBeetle_weaponCeil = {
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
    .counter_resistance_flags = 
        AttackCounterResistance_Flags::ALL &
        ~AttackCounterResistance_Flags::PAYBACK,
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
BattleWeapon unitBuzzyBeetle_weaponCeilSpiky = {
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
    .counter_resistance_flags = 
        AttackCounterResistance_Flags::ALL &
        ~AttackCounterResistance_Flags::PAYBACK,
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
BattleWeapon unitBuzzyBeetle_weaponPara = {
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
    .counter_resistance_flags = 
        AttackCounterResistance_Flags::FIERY |
        AttackCounterResistance_Flags::EXPLOSIVE |
        AttackCounterResistance_Flags::VOLATILE_EXPLOSIVE |
        AttackCounterResistance_Flags::HOLD_FAST,
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

BattleUnitKindPart unitBuzzyBeetle_parts[] = {
    {
        .index = 1,
        .name = "btl_un_met",
        .model_name = "c_met",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 20.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 25.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitBuzzyBeetle_defense,
        .defense_attr = unitBuzzyBeetle_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitBuzzyBeetle_pose_table,
    },
};
BattleUnitKindPart unitSpikeTop_parts[] = {
    {
        .index = 1,
        .name = "btl_un_togemet",
        .model_name = "c_met",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 23.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 28.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitBuzzyBeetle_defense,
        .defense_attr = unitBuzzyBeetle_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = PartsCounterAttribute_Flags::TOP_SPIKY,
        .pose_table = unitSpikeTop_pose_table,
    },
};
BattleUnitKindPart unitParabuzzy_parts[] = {
    {
        .index = 1,
        .name = "btl_un_patamet",
        .model_name = "c_met",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 20.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 25.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitBuzzyBeetle_defense,
        .defense_attr = unitBuzzyBeetle_defense_attr,
        .attribute_flags = 0x0000'0009 |
            PartsAttribute_Flags::WINGED |
            PartsAttribute_Flags::HAMMERLIKE_CANNOT_TARGET |
            PartsAttribute_Flags::SHELLTOSSLIKE_CANNOT_TARGET,
        .counter_attribute_flags = 0,
        .pose_table = unitParabuzzy_pose_table,
    },
};
BattleUnitKindPart unitSpikyParabuzzy_parts[] = {
    {
        .index = 1,
        .name = "btl_un_patatogemet",
        .model_name = "c_met",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 23.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 28.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitBuzzyBeetle_defense,
        .defense_attr = unitBuzzyBeetle_defense_attr,
        .attribute_flags = 0x0000'0009 |
            PartsAttribute_Flags::WINGED |
            PartsAttribute_Flags::HAMMERLIKE_CANNOT_TARGET |
            PartsAttribute_Flags::SHELLTOSSLIKE_CANNOT_TARGET,
        .counter_attribute_flags = PartsCounterAttribute_Flags::TOP_SPIKY,
        .pose_table = unitSpikyParabuzzy_pose_table,
    },
};

DataTableEntry unitBuzzyBeetle_data_table[3] = {
    13, (void*)unitBuzzyBeetle_flip_event,
    48, (void*)btldefaultevt_Dummy,
    0, nullptr,
};
DataTableEntry unitParabuzzy_data_table[3] = {
    14, (void*)unitParabuzzy_fall_event,
    48, (void*)unitParabuzzy_first_attack_pos_event,
    0, nullptr,
};
DataTableEntry unitBuzzyBeetle_ceil_data_table[14] = {
    25, (void*)unitBuzzyBeetle_ceil_countered_evt,
    26, (void*)unitBuzzyBeetle_ceil_countered_evt,
    27, (void*)unitBuzzyBeetle_ceil_countered_evt,
    28, (void*)unitBuzzyBeetle_ceil_countered_evt,
    29, (void*)unitBuzzyBeetle_ceil_countered_evt,
    30, (void*)unitBuzzyBeetle_ceil_countered_evt,
    31, (void*)unitBuzzyBeetle_ceil_countered_evt,
    32, (void*)unitBuzzyBeetle_ceil_countered_evt,
    33, (void*)unitBuzzyBeetle_ceil_countered_evt,
    34, (void*)unitBuzzyBeetle_ceil_countered_evt,
    35, (void*)unitBuzzyBeetle_ceil_countered_evt,
    36, (void*)unitBuzzyBeetle_ceil_countered_evt,
    48, (void*)btldefaultevt_Dummy,
    0, nullptr,
};

DataTableEntry unitSpikeTop_data_table[4] = {
    13, (void*)unitBuzzyBeetle_flip_event,
    37, (void*)unitSpikeTop_spiky_counter_event,
    48, (void*)btldefaultevt_Dummy,
    0, nullptr,
};
DataTableEntry unitSpikyParabuzzy_data_table[4] = {
    14, (void*)unitParabuzzy_fall_event,
    37, (void*)unitSpikeTop_spiky_counter_event,
    48, (void*)unitParabuzzy_first_attack_pos_event,
    0, nullptr,
};
// Doesn't seem like there's a Spiky counter here?
DataTableEntry unitSpikeTop_ceil_data_table[14] = {
    25, (void*)unitBuzzyBeetle_ceil_countered_evt,
    26, (void*)unitBuzzyBeetle_ceil_countered_evt,
    27, (void*)unitBuzzyBeetle_ceil_countered_evt,
    28, (void*)unitBuzzyBeetle_ceil_countered_evt,
    29, (void*)unitBuzzyBeetle_ceil_countered_evt,
    30, (void*)unitBuzzyBeetle_ceil_countered_evt,
    31, (void*)unitBuzzyBeetle_ceil_countered_evt,
    32, (void*)unitBuzzyBeetle_ceil_countered_evt,
    33, (void*)unitBuzzyBeetle_ceil_countered_evt,
    34, (void*)unitBuzzyBeetle_ceil_countered_evt,
    35, (void*)unitBuzzyBeetle_ceil_countered_evt,
    36, (void*)unitBuzzyBeetle_ceil_countered_evt,
    48, (void*)btldefaultevt_Dummy,
    0, nullptr,
};

BattleUnitKind unit_BuzzyBeetle = {
    .unit_type = BattleUnitType::BUZZY_BEETLE,
    .unit_name = "btl_un_met",
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
    .width = 28,
    .height = 20,
    .hit_offset = { 5, 20 },
    .center_offset = { 0.0f, 10.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 14.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 14.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 12.0f, 15.0f, 0.0f },
    .cut_base_offset = { 0.0f, 10.0f, 0.0f },
    .cut_width = 28.0f,
    .cut_height = 20.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_MET_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitBuzzyBeetle_status,
    .num_parts = 1,
    .parts = unitBuzzyBeetle_parts,
    .init_evt_code = (void*)unitBuzzyBeetle_init_event,
    .data_table = unitBuzzyBeetle_data_table,
};
BattleUnitKind unit_SpikeTop = {
    .unit_type = BattleUnitType::SPIKE_TOP,
    .unit_name = "btl_un_togemet",
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
    .width = 28,
    .height = 26,
    .hit_offset = { 5, 26 },
    .center_offset = { 0.0f, 13.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 14.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 14.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 14.0f, 16.9f, 0.0f },
    .cut_base_offset = { 0.0f, 13.0f, 0.0f },
    .cut_width = 28.0f,
    .cut_height = 26.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_MET_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitBuzzyBeetle_status,
    .num_parts = 1,
    .parts = unitSpikeTop_parts,
    .init_evt_code = (void*)unitSpikeTop_init_event,
    .data_table = unitSpikeTop_data_table,
};
BattleUnitKind unit_Parabuzzy = {
    .unit_type = BattleUnitType::PARABUZZY,
    .unit_name = "btl_un_patamet",
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
    .width = 28,
    .height = 30,
    .hit_offset = { 5, 30 },
    .center_offset = { 0.0f, 17.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 14.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 14.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 14.0f, 19.5f, 0.0f },
    .cut_base_offset = { 0.0f, 17.0f, 0.0f },
    .cut_width = 28.0f,
    .cut_height = 30.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_MET_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::UNQUAKEABLE,
    .status_vulnerability = &unitParabuzzy_status,
    .num_parts = 1,
    .parts = unitParabuzzy_parts,
    .init_evt_code = (void*)unitParabuzzy_init_event,
    .data_table = unitParabuzzy_data_table,
};
BattleUnitKind unit_SpikyParabuzzy = {
    .unit_type = BattleUnitType::SPIKY_PARABUZZY,
    .unit_name = "btl_un_patatogemet",
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
    .width = 28,
    .height = 30,
    .hit_offset = { 5, 30 },
    .center_offset = { 0.0f, 17.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 14.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 14.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 14.0f, 19.5f, 0.0f },
    .cut_base_offset = { 0.0f, 17.0f, 0.0f },
    .cut_width = 28.0f,
    .cut_height = 30.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_MET_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::UNQUAKEABLE,
    .status_vulnerability = &unitParabuzzy_status,
    .num_parts = 1,
    .parts = unitSpikyParabuzzy_parts,
    .init_evt_code = (void*)unitSpikyParabuzzy_init_event,
    .data_table = unitSpikyParabuzzy_data_table,
};

// Evt definitions.

EVT_BEGIN(unitBuzzyBeetle_wakeup_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_GetOverTurnCount, LW(10), LW(0))
    SUB(LW(0), 1)
    USER_FUNC(btlevtcmd_SetOverTurnCount, LW(10), LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_StartWaitEvent, -2)
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_SetPartsDefenceTable, LW(10), LW(11), PTR(&unitBuzzyBeetle_defense))
    USER_FUNC(btlevtcmd_SetPartsDefenceAttrTable, LW(10), LW(11), PTR(&unitBuzzyBeetle_defense_attr))
    USER_FUNC(btlevtcmd_OffStatusFlag, LW(10), 1)
    
    USER_FUNC(btlevtcmd_GetUnitWork, -2, (int32_t)UW_BattleUnitType, LW(8))
    IF_EQUAL(LW(8), (int32_t)BattleUnitType::BUZZY_BEETLE)
        USER_FUNC(btlevtcmd_AnimeSetPoseTable, LW(10), LW(11), PTR(&unitBuzzyBeetle_pose_table))
    ELSE()
        USER_FUNC(btlevtcmd_AnimeSetPoseTable, LW(10), LW(11), PTR(&unitSpikeTop_pose_table))
    END_IF()

    USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(10), LW(11), 69)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_INSIDE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(1.0))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 18, -1)
    USER_FUNC(btlevtcmd_SetPos, -2, LW(0), 0, LW(2))

    IF_EQUAL(LW(8), (int32_t)BattleUnitType::SPIKE_TOP)
        USER_FUNC(btlevtcmd_OnPartsCounterAttribute, -2, 1, 1)
        USER_FUNC(btlevtcmd_SetHomePos, -2, LW(0), 0, LW(2))
    END_IF()

    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitBuzzyBeetle_ceil_fall_ready_event)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, (int32_t)UW_BattleUnitType, LW(8))

    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_FALL1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, 0)
    USER_FUNC(btlevtcmd_SetJumpSpeed, -2, 0)
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.2))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    IF_EQUAL(LW(8), (int32_t)BattleUnitType::BUZZY_BEETLE)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 15, LW(2), 15, -1)
    ELSE()
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 20, LW(2), 15, -1)
    END_IF()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MET_LANDING1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_OffAttribute, -2, 2)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, 0x40'0000)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, 0x20'0000)

    IF_EQUAL(LW(8), (int32_t)BattleUnitType::BUZZY_BEETLE)
        USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitBuzzyBeetle_flip_pose_table))
        USER_FUNC(btlevtcmd_ChangeDataTable, -2, PTR(&unitBuzzyBeetle_data_table))
        USER_FUNC(btlevtcmd_SetKissHitOffset, -2, 12, 15, 0)
    ELSE()
        USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitSpikeTop_flip_pose_table))
        USER_FUNC(btlevtcmd_ChangeDataTable, -2, PTR(&unitSpikeTop_data_table))
        USER_FUNC(btlevtcmd_SetKissHitOffset, -2, 14, 17, 0)
    END_IF()

    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_OnCeiling, 0)
    USER_FUNC(btlevtcmd_SetHitOffset, -2, 1, 0, 0, 0)
    USER_FUNC(btlevtcmd_SetHitCursorOffset, -2, 1, 0, 0, 0)
    RUN_CHILD_EVT(PTR(&unitBuzzyBeetle_init_event2))
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_SetPartsDefenceTable, LW(10), LW(11), PTR(&unitBuzzyBeetle_flip_defense))
    USER_FUNC(btlevtcmd_SetPartsDefenceAttrTable, LW(10), LW(11), PTR(&unitBuzzyBeetle_flip_defense_attr))

    IF_EQUAL(LW(8), (int32_t)BattleUnitType::BUZZY_BEETLE)
        USER_FUNC(btlevtcmd_AnimeSetPoseTable, LW(10), LW(11), PTR(&unitBuzzyBeetle_flip_pose_table))
        SET(LW(3), 0)
    ELSE()
        USER_FUNC(btlevtcmd_AnimeSetPoseTable, LW(10), LW(11), PTR(&unitSpikeTop_flip_pose_table))
        SET(LW(3), 5)
    END_IF()
    
    USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(10), LW(11), 69)
    USER_FUNC(btlevtcmd_OnStatusFlag, LW(10), 1)
    USER_FUNC(btlevtcmd_SetOverTurnCount, LW(10), 1)
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(1.0))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(3), LW(2), 18, -1)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(3), LW(2), 12, -1)
    USER_FUNC(btlevtcmd_SetHomePos, -2, LW(0), LW(3), LW(2))
    RETURN()
EVT_END()

EVT_BEGIN(unitBuzzyBeetle_attack_event)
    USER_FUNC(btlevtcmd_GetOverTurnCount, -2, LW(0))
    IF_LARGE_EQUAL(LW(0), 1)
        RUN_CHILD_EVT(PTR(&unitBuzzyBeetle_wakeup_event))
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_GetFirstAttackTarget, LW(3), LW(4))
        WAIT_FRM(30)
    ELSE()
        USER_FUNC(btlevtcmd_EnemyItemUseCheck, -2, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(LW(0))
            USER_FUNC(btlevtcmd_StartWaitEvent, -2)
            RETURN()
        END_IF()
        USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
        USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), PTR(&unitBuzzyBeetle_weaponGrounded))
        USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, PTR(&unitBuzzyBeetle_weaponGrounded), LW(3), LW(4))
    END_IF()
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
    USER_FUNC(btlevtcmd_WeaponAftereffect, PTR(&unitBuzzyBeetle_weaponGrounded))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, PTR(&unitBuzzyBeetle_weaponGrounded))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    WAIT_MSEC(100)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MET_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.0), FLOAT(0.7))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 0, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MET_SHELL1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 50)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MET_ATTACK2"), EVT_NULLPTR, 0, LW(15))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Rotate, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_RotateSpeed, 0)
    BROTHER_EVT()
        DO(0)
            USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Rotate, LW(15))
            IF_EQUAL(LW(15), 1)
                USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
                DO_BREAK()
            END_IF()
            WAIT_FRM(1)
            USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_RotateSpeed, LW(14))
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, LW(14), 0)
        WHILE()
    END_BROTHER()
    WAIT_MSEC(100)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_RotateSpeed, 10)
    WAIT_MSEC(100)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_RotateSpeed, 20)
    WAIT_MSEC(100)
    BROTHER_EVT_ID(LW(13))
        DO(0)
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
            ADD(LW(1), 10)
            USER_FUNC(evt_eff, PTR(""), PTR("kemuri_test"), 0, LW(0), LW(1), LW(2), FLOAT(0.3), 0, 0, 0, 0, 0, 0, 0)
            WAIT_FRM(3)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(8.0))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetPos, -2, EVT_NULLPTR, LW(5), EVT_NULLPTR)
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(5), LW(2), 0, 0, 0)
    DELETE_EVT(LW(13))
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), PTR(&unitBuzzyBeetle_weaponGrounded), 256, LW(5))
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
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 100)
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), 0, LW(2), 0, 0, 0)
    WAIT_MSEC(250)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_RotateSpeed, 10)
    WAIT_MSEC(250)
    USER_FUNC(evt_snd_sfxoff, LW(15))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Rotate, 1)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 69)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MET_JUMP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(10.0), FLOAT(0.4))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 40)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MET_JUMP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 20, -1)
    GOTO(98)
LBL(91)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), PTR(&unitBuzzyBeetle_weaponGrounded))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&unitBuzzyBeetle_weaponGrounded), 256, LW(5))
    USER_FUNC(evt_snd_sfxoff, LW(15))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_RotateSpeed, 10)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(3.0), FLOAT(0.2))
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 50)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MET_MOVE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 20, -1)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.5), FLOAT(0.2))
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 75)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MET_MOVE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 20, -1)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Rotate, 1)
    GOTO(98)
LBL(98)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(4.0))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitBuzzyBeetle_ceil_attack_event)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, (int32_t)UW_BattleUnitType, LW(6))
    IF_EQUAL(LW(6), (int32_t)BattleUnitType::BUZZY_BEETLE)
        SET(LW(14), PTR(&unitBuzzyBeetle_weaponCeil))
    ELSE()
        SET(LW(14), PTR(&unitBuzzyBeetle_weaponCeilSpiky))
    END_IF()

    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_GetFirstAttackTarget, LW(3), LW(4))
    ELSE()
        USER_FUNC(btlevtcmd_EnemyItemUseCheck, -2, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(LW(0))
            USER_FUNC(btlevtcmd_StartWaitEvent, -2)
            RETURN()
        END_IF()
        USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
        USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(14))
        USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(14), LW(3), LW(4))
    END_IF()
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            RETURN()
        END_IF()
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_GetPos, -2, LW(7), LW(8), LW(9))
    USER_FUNC(evt_btl_camera_set_mode, 0, 8)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, LW(3))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    USER_FUNC(evt_btl_camera_nomove_y_onoff, 0, 1)
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(14))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(14))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    WAIT_MSEC(100)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(10), LW(11), LW(12))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(5.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(11), LW(12), 60, -1, 1)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 50)
    WAIT_MSEC(250)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MET_FALL1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, 0)
    USER_FUNC(btlevtcmd_SetJumpSpeed, -2, 0)
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.2))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 18, LW(2), 15, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MET_LANDING1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), LW(14), 256, LW(5))
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
    GOTO(98)
LBL(91)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(14))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(14), 256, LW(5))
    GOTO(98)
LBL(98)

    IF_EQUAL(LW(6), (int32_t)BattleUnitType::BUZZY_BEETLE)
        USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, -18, 0)
        SETF(LW(12), FLOAT(30.0))
        SETF(LW(13), FLOAT(0.0))
        DO(60)
            USER_FUNC(evt_sub_get_sincos, LW(13), LW(14), LW(15))
            MULF(LW(14), LW(12))
            USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, LW(14))
            SUBF(LW(12), FLOAT(0.5))
            ADDF(LW(13), FLOAT(10.0))
            WAIT_FRM(1)
        WHILE()
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
        USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitBuzzyBeetle_pose_table))
        USER_FUNC(btlevtcmd_ChangeDataTable, -2, PTR(&unitBuzzyBeetle_data_table))
        USER_FUNC(btlevtcmd_SetKissHitOffset, -2, 12, 15, 0)
    ELSE()
        WAIT_FRM(60)
        USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitSpikeTop_pose_table))
        USER_FUNC(btlevtcmd_ChangeDataTable, -2, PTR(&unitSpikeTop_data_table))
        USER_FUNC(btlevtcmd_SetKissHitOffset, -2, 14, 17, 0)
    END_IF()
    
    USER_FUNC(btlevtcmd_OffAttribute, -2, 2)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, 0x40'0000)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, 0x20'0000)

    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_OnCeiling, 0)
    USER_FUNC(btlevtcmd_SetHitOffset, -2, 1, 0, 0, 0)
    USER_FUNC(btlevtcmd_SetHitCursorOffset, -2, 1, 0, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 69)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(3.0), FLOAT(0.2))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 25)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MET_JUMP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 20, -1)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.5), FLOAT(0.2))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 25)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MET_JUMP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 20, -1)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_SetHomePos, -2, LW(7), 0, LW(9))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(4.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(7), 0, LW(9), 0, -1, 0)
    RUN_CHILD_EVT(PTR(&unitBuzzyBeetle_init_event2))
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitParabuzzy_attack_event)
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_GetFirstAttackTarget, LW(3), LW(4))
    ELSE()
        USER_FUNC(btlevtcmd_EnemyItemUseCheck, -2, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(LW(0))
            USER_FUNC(btlevtcmd_StartWaitEvent, -2)
            RETURN()
        END_IF()
        USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
        USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), PTR(&unitBuzzyBeetle_weaponPara))
        USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, PTR(&unitBuzzyBeetle_weaponPara), LW(3), LW(4))
    END_IF()
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
    USER_FUNC(btlevtcmd_WeaponAftereffect, PTR(&unitBuzzyBeetle_weaponPara))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, PTR(&unitBuzzyBeetle_weaponPara))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_PATAMET_MOVE1"), EVT_NULLPTR, 0, LW(15))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    ADD(LW(1), 40)
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 30, FLOAT(1.0), 0, 0, -1)
    WAIT_MSEC(250)
    USER_FUNC(evt_snd_sfxoff, LW(15))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_PATAMET_MOVE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(9.0))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 50)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), 10)
    SUB(LW(1), 30)
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 0, -10, 0, 0, -1)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), PTR(&unitBuzzyBeetle_weaponPara), 256, LW(5))
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
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(6.0))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetPos, LW(3), EVT_NULLPTR, LW(5), EVT_NULLPTR)
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 75)
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(5), LW(2), 0, 0, 0, 0, -1)
    WAIT_MSEC(300)
    GOTO(98)
LBL(91)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), PTR(&unitBuzzyBeetle_weaponPara))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&unitBuzzyBeetle_weaponPara), 256, LW(5))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_PATAMET_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    GOTO(98)
LBL(98)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_add_zoom, 0, -50)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(6.0))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 0, -10, 0, 0, -1)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitBuzzyBeetle_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    RETURN()
EVT_END()

EVT_BEGIN(unitBuzzyBeetle_ceil_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    USER_FUNC(btlevtcmd_CheckDamageCode, -2, 0x200, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_CheckDamageCode, -2, 0x800, LW(0))
        IF_EQUAL(LW(0), 0)
            RETURN()
        END_IF()
    END_IF()
    USER_FUNC(btlevtcmd_GetTotalDamage, -2, LW(12))
    IF_EQUAL(LW(12), 0)
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_CheckAttribute, -2, 2, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(PTR(&unitBuzzyBeetle_ceil_fall_ready_event))
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitSpikeTop_spiky_counter_event)
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_CheckStatus, -2, 10, LW(3))
    IF_NOT_EQUAL(LW(3), 0)
        ADD(LW(1), 38)
        GOTO(90)
    END_IF()
    USER_FUNC(btlevtcmd_CheckStatus, -2, 11, LW(3))
    IF_NOT_EQUAL(LW(3), 0)
        ADD(LW(1), 18)
        GOTO(90)
    END_IF()
    ADD(LW(1), 28)
    GOTO(90)
LBL(90)
    USER_FUNC(evt_eff, PTR(""), PTR("toge_flush"), 0, LW(0), LW(1), LW(2), 60, 0, 0, 0, 0, 0, 0, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitParabuzzy_spiky_counter_event)
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_CheckStatus, -2, 10, LW(3))
    IF_NOT_EQUAL(LW(3), 0)
        ADD(LW(1), 50)
        GOTO(90)
    END_IF()
    USER_FUNC(btlevtcmd_CheckStatus, -2, 11, LW(3))
    IF_NOT_EQUAL(LW(3), 0)
        ADD(LW(1), 30)
        GOTO(90)
    END_IF()
    ADD(LW(1), 40)
    GOTO(90)
LBL(90)
    USER_FUNC(evt_eff, PTR(""), PTR("toge_flush"), 0, LW(0), LW(1), LW(2), 60, 0, 0, 0, 0, 0, 0, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitParabuzzy_first_attack_pos_event)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    SET(LW(1), 10)
    USER_FUNC(btlevtcmd_SetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_WaitAttackEnd)
    RETURN()
EVT_END()

EVT_BEGIN(unitBuzzyBeetle_wait_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitBuzzyBeetle_init_event2)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_OnCeiling, LW(15))
    IF_EQUAL(LW(15), 0)
        USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitBuzzyBeetle_wait_event))
        USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitBuzzyBeetle_attack_event))
        USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitBuzzyBeetle_damage_event))
        USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitBuzzyBeetle_attack_event))
        USER_FUNC(btlevtcmd_SetEventCeilFall, -2, 0)
        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 1, 0x1000)
        USER_FUNC(btlevtcmd_SetHitCursorOffset, -2, 1, 0, 0, 0)
    ELSE()
        USER_FUNC(btlevtcmd_OnAttribute, -2, 2)
        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 1, 0x40'0000)
        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 1, 0x20'0000)

        USER_FUNC(btlevtcmd_GetUnitWork, -2, (int32_t)UW_BattleUnitType, LW(0))
        IF_EQUAL(LW(0), (int32_t)BattleUnitType::BUZZY_BEETLE)
            USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitBuzzyBeetle_ceil_pose_table))
            USER_FUNC(btlevtcmd_ChangeDataTable, -2, PTR(&unitBuzzyBeetle_ceil_data_table))
            USER_FUNC(btlevtcmd_SetKissHitOffset, -2, 12, -15, 0)
        ELSE()
            USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitSpikeTop_ceil_pose_table))
            USER_FUNC(btlevtcmd_ChangeDataTable, -2, PTR(&unitSpikeTop_ceil_pose_table))
            USER_FUNC(btlevtcmd_SetKissHitOffset, -2, 14, -17, 0)
        END_IF()

        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        SET(LW(1), 130)
        USER_FUNC(btlevtcmd_SetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_SetHomePos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitBuzzyBeetle_wait_event))
        USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitBuzzyBeetle_ceil_attack_event))
        USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitBuzzyBeetle_ceil_damage_event))
        USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitBuzzyBeetle_ceil_attack_event))
        USER_FUNC(btlevtcmd_SetEventCeilFall, -2, PTR(&unitBuzzyBeetle_ceil_fall_ready_event))
        USER_FUNC(btlevtcmd_SetHitOffset, -2, 1, 0, -50, 0)
        USER_FUNC(btlevtcmd_SetHitCursorOffset, -2, 1, 0, -50, 0)
    END_IF()
    USER_FUNC(btlevtcmd_SetRunSound, -2, PTR("SFX_ENM_MET_MOVE1L"), PTR("SFX_ENM_MET_MOVE1R"), 0, 3, 3)
    USER_FUNC(btlevtcmd_SetWalkSound, -2, PTR("SFX_ENM_MET_MOVE1L"), PTR("SFX_ENM_MET_MOVE1R"), 0, 6, 6)
    USER_FUNC(btlevtcmd_SetOverTurnCount, -2, 0)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitBuzzyBeetle_common_init_event)
    RUN_CHILD_EVT(&unitBuzzyBeetle_init_event2)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitBuzzyBeetle_flip_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_SetPartsDefenceTable, LW(10), LW(11), PTR(&unitBuzzyBeetle_flip_defense))
    USER_FUNC(btlevtcmd_SetPartsDefenceAttrTable, LW(10), LW(11), PTR(&unitBuzzyBeetle_flip_defense_attr))

    USER_FUNC(btlevtcmd_GetUnitWork, -2, (int32_t)UW_BattleUnitType, LW(6))
    IF_EQUAL(LW(6), (int32_t)BattleUnitType::BUZZY_BEETLE)
        USER_FUNC(btlevtcmd_AnimeSetPoseTable, LW(10), LW(11), PTR(&unitBuzzyBeetle_flip_pose_table))
        SET(LW(3), 0)
    ELSE()
        USER_FUNC(btlevtcmd_AnimeSetPoseTable, LW(10), LW(11), PTR(&unitSpikeTop_flip_pose_table))
        SET(LW(3), 5)
    END_IF()

    USER_FUNC(btlevtcmd_OnStatusFlag, LW(10), 1)
    USER_FUNC(btlevtcmd_SetOverTurnCount, LW(10), 1)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(10), LW(11), 39)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_INSIDE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    WAIT_FRM(4)
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(1.0))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(3), LW(2), 18, -1)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(3), LW(2), 12, -1)

    IF_EQUAL(LW(6), (int32_t)BattleUnitType::SPIKE_TOP)
        USER_FUNC(btlevtcmd_SetHomePos, -2, LW(0), 5, LW(2))
        USER_FUNC(btlevtcmd_OffPartsCounterAttribute, -2, 1, 1)
    END_IF()

    RETURN()
EVT_END()

EVT_BEGIN(unitParabuzzy_fall_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_OffAttribute, LW(10), 4)
    USER_FUNC(btlevtcmd_OffPartsAttribute, LW(10), LW(11), 0x800)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(10), LW(11), 39)

    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(0))
    IF_EQUAL(LW(0), (int32_t)BattleUnitType::PARABUZZY)
        SET(LW(0), (int32_t)BattleUnitType::BUZZY_BEETLE)
        USER_FUNC(btlevtcmd_AnimeSetPoseTable, LW(10), LW(11), PTR(&unitBuzzyBeetle_pose_table))
        USER_FUNC(btlevtcmd_ChangeDataTable, LW(10), PTR(&unitBuzzyBeetle_data_table))
        USER_FUNC(btlevtcmd_ReplaceParts, LW(10), LW(11), PTR(&unitBuzzyBeetle_parts), 1)
    ELSE()
        SET(LW(0), (int32_t)BattleUnitType::SPIKE_TOP)
        USER_FUNC(btlevtcmd_AnimeSetPoseTable, LW(10), LW(11), PTR(&unitSpikeTop_pose_table))
        USER_FUNC(btlevtcmd_ChangeDataTable, LW(10), PTR(&unitSpikeTop_data_table))
        USER_FUNC(btlevtcmd_ReplaceParts, LW(10), LW(11), PTR(&unitSpikeTop_parts), 1)
    END_IF()
    USER_FUNC(btlevtcmd_SetRegistStatus, LW(10), PTR(&unitBuzzyBeetle_status))
    USER_FUNC(btlevtcmd_ChangeKind, LW(10), LW(0))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, LW(0))
    
    USER_FUNC(btlevtcmd_OnUnitFlag, LW(10), 4)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(10), LW(11), 39)
    USER_FUNC(btlevtcmd_GetPos, LW(10), LW(0), LW(1), LW(2))
    SET(LW(1), 0)
    USER_FUNC(btlevtcmd_SetFallAccel, LW(10), FLOAT(1.0))
    USER_FUNC(btlevtcmd_JumpPosition, LW(10), LW(0), LW(1), LW(2), 10, -1)
    USER_FUNC(btlevtcmd_SetHomePos, LW(10), LW(0), LW(1), LW(2))

    USER_FUNC(btlevtcmd_GetHpDamageCount, LW(10), LW(0))
    IF_SMALL_EQUAL(LW(0), 1)
        RUN_CHILD_EVT(PTR(&unitBuzzyBeetle_common_init_event))
        USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(10), LW(11), 39)
        RETURN()
    ELSE()
        RUN_CHILD_EVT(PTR(&unitBuzzyBeetle_common_init_event))
        USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(10), LW(11), 39)
        RUN_CHILD_EVT(PTR(&unitBuzzyBeetle_flip_event))
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitBuzzyBeetle_ceil_countered_evt)
    USER_FUNC(btlevtcmd_OffAttribute, -2, 2)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, 0x40'0000)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, 0x20'0000)

    USER_FUNC(btlevtcmd_GetUnitWork, -2, (int32_t)UW_BattleUnitType, LW(0))
    IF_EQUAL(LW(0), (int32_t)BattleUnitType::BUZZY_BEETLE)
        USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitBuzzyBeetle_pose_table))
        USER_FUNC(btlevtcmd_ChangeDataTable, -2, PTR(&unitBuzzyBeetle_data_table))
        USER_FUNC(btlevtcmd_SetKissHitOffset, -2, 12, 15, 0)
    ELSE()
        USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitSpikeTop_pose_table))
        USER_FUNC(btlevtcmd_ChangeDataTable, -2, PTR(&unitSpikeTop_data_table))
        USER_FUNC(btlevtcmd_SetKissHitOffset, -2, 14, 17, 0)
    END_IF()

    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_OnCeiling, 0)
    USER_FUNC(btlevtcmd_SetHitOffset, -2, 1, 0, 0, 0)
    USER_FUNC(btlevtcmd_SetHitCursorOffset, -2, 1, 0, 0, 0)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetHomePos, -2, LW(0), 0, LW(2))
    RUN_CHILD_EVT(PTR(&unitBuzzyBeetle_init_event2))
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_GetDamageCode, LW(10), LW(0))
    RUN_CHILD_EVT(PTR(&subsetevt_counter_damage))
    RETURN()
EVT_END()

EVT_BEGIN(unitParabuzzy_common_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitBuzzyBeetle_wait_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitParabuzzy_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitBuzzyBeetle_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitParabuzzy_attack_event))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitBuzzyBeetle_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType,
        (int32_t)BattleUnitType::BUZZY_BEETLE)
    RUN_CHILD_EVT(unitBuzzyBeetle_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitSpikeTop_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType,
        (int32_t)BattleUnitType::SPIKE_TOP)
    RUN_CHILD_EVT(unitBuzzyBeetle_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitParabuzzy_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType,
        (int32_t)BattleUnitType::PARABUZZY)
    RUN_CHILD_EVT(unitParabuzzy_common_init_event)
    USER_FUNC(btlevtcmd_AnimeSetPoseSoundTable, -2, 1, PTR(&unitParabuzzy_pose_sound_timing_table))
    RETURN()
EVT_END()

EVT_BEGIN(unitSpikyParabuzzy_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType,
        (int32_t)BattleUnitType::SPIKY_PARABUZZY)
    RUN_CHILD_EVT(unitParabuzzy_common_init_event)
    USER_FUNC(btlevtcmd_AnimeSetPoseSoundTable, -2, 1, PTR(&unitSpikyParabuzzy_pose_sound_timing_table))
    RETURN()
EVT_END()

}  // namespace mod::tot::custom