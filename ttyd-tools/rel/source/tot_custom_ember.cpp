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
#include <ttyd/effdrv.h>
#include <ttyd/eff_fire.h>
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
constexpr const int32_t UW_BattleUnitType = 0;

EffEntry* g_FireballEff;
EffEntry* g_FlamethrowerEffs[10];

constexpr const float kFlamethrowerScale[10] = {
    1.0f, 1.25f, 1.5f, 1.75f, 2.0f, 2.25f, 2.0f, 1.75f, 1.5f, 1.25f
};

}  // namespace

// Evt / Function declarations.

EVT_DECLARE_USER_FUNC(unitEmber_copy_status, 2)
EVT_DECLARE_USER_FUNC(unitEmber_eff_fireball, 1)
EVT_DECLARE_USER_FUNC(unitEmber_eff_flamethrower, 2)

extern const int32_t unitLavaBubble_init_event[];
extern const int32_t unitEmber_init_event[];
extern const int32_t unitPhantomEmber_init_event[];
extern const int32_t unitEmber_common_init_event[];
extern const int32_t unitEmber_attack_event[];
extern const int32_t unitEmber_damage_event[];
extern const int32_t unitEmber_wait_event[];
extern const int32_t unitEmber_spawn_event[];
extern const int32_t unitEmber_fire_counter_event[];
extern const int32_t unitEmber_fire_damaged_event[];
extern const int32_t unitEmber_melee_attack_event[];
extern const int32_t unitEmber_fireball_attack_event[];
extern const int32_t unitEmber_flamethrower_attack_event[];
extern const int32_t unitEmber_flamethrower_sub_event[];

// Unit data.

int8_t unitEmber_defense[] = { 0, 0, 0, 0, 0 };
int8_t unitEmber_defense_attr[] = { 0, 3, 1, 1, 0 };

// Used for Lava Bubble and Ember.
StatusVulnerability unitEmber_status = {
     50,  90, 105, 100,  75, 100,   0,  50,
    100,  90, 100,  90, 100,  95,  75,  50,
     90, 100,  75, 100, 100,  95,
};
StatusVulnerability unitPhantomEmber_status = {
     40,  80, 100, 100,  65, 100,   0,  40,
    100,  80, 100,  80, 100,  90,  65,  40,
     80, 100,  65, 100, 100,  90,
};

PoseTableEntry unitEmber_pose_table[] = {
    1, "N_1",
    2, "Y_1",
    9, "Y_1",
    5, "K_1",
    4, "X_1",
    3, "X_1",
    28, "S_1",
    29, "Q_1",
    30, "Q_1",
    31, "A_2B",
    39, "D_1",
    50, "A_1",
    42, "R_1",
    40, "W_1",
    56, "X_1",
    57, "X_1",
    65, "T_1",
    69, "S_1",
};

DataTableEntry unitEmber_data_table[] = {
    11, (void*)unitEmber_fire_damaged_event,
    40, (void*)unitEmber_fire_counter_event,
    48, (void*)btldefaultevt_Dummy,
    0, nullptr,
};

BattleWeapon unitLavaBubble_weaponMelee = {
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
    .element = AttackElement::FIRE,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::FREEZE_BREAK,
    .counter_resistance_flags =
        AttackCounterResistance_Flags::FIERY |
        AttackCounterResistance_Flags::ICY |
        AttackCounterResistance_Flags::POISON |
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
BattleWeapon unitLavaBubble_weaponFireball = {
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
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING |
        AttackSpecialProperty_Flags::FREEZE_BREAK,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    .burn_chance = 75,
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
BattleWeapon unitEmber_weaponMelee = {
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
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::FIRE,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::FREEZE_BREAK,
    .counter_resistance_flags =
        AttackCounterResistance_Flags::FIERY |
        AttackCounterResistance_Flags::ICY |
        AttackCounterResistance_Flags::POISON |
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
BattleWeapon unitEmber_weaponFireball = {
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
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING |
        AttackSpecialProperty_Flags::FREEZE_BREAK,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    .burn_chance = 75,
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
BattleWeapon unitEmber_weaponFlamethrower = {
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
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH |
        AttackTargetClass_Flags::ENEMY_SELECT_SIDE_CENTER,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::FIRE,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING |
        AttackSpecialProperty_Flags::FREEZE_BREAK,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    .burn_chance = 75,
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
BattleWeapon unitPhantomEmber_weaponMelee = {
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
    .element = AttackElement::FIRE,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::FREEZE_BREAK,
    .counter_resistance_flags =
        AttackCounterResistance_Flags::FIERY |
        AttackCounterResistance_Flags::ICY |
        AttackCounterResistance_Flags::POISON |
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
BattleWeapon unitPhantomEmber_weaponFireball = {
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
    .element = AttackElement::FIRE,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING |
        AttackSpecialProperty_Flags::FREEZE_BREAK,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    .burn_chance = 75,
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
BattleWeapon unitPhantomEmber_weaponFlamethrower = {
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
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::FIRE,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING |
        AttackSpecialProperty_Flags::FREEZE_BREAK,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    .burn_chance = 75,
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

BattleUnitKindPart unitLavaBubble_parts[] = {
    {
        .index = 1,
        .name = "btl_un_bubble",
        .model_name = "c_bubble",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 35.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitEmber_defense,
        .defense_attr = unitEmber_defense_attr,
        .attribute_flags = 0x0000'0009 | PartsAttribute_Flags::WEAK_TO_ICE_POWER,
        .counter_attribute_flags = PartsCounterAttribute_Flags::FIERY,
        .pose_table = unitEmber_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_bubble",
        .model_name = "c_bubble",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 15,
        .unk_32 = 15,
        .base_alpha = 255,
        .defense = unitEmber_defense,
        .defense_attr = unitEmber_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitEmber_pose_table,
    },
};
BattleUnitKindPart unitEmber_parts[] = {
    {
        .index = 1,
        .name = "btl_un_hermos",
        .model_name = "c_elmos",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 35.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitEmber_defense,
        .defense_attr = unitEmber_defense_attr,
        .attribute_flags = 0x0000'0009 | PartsAttribute_Flags::WEAK_TO_ICE_POWER,
        .counter_attribute_flags = PartsCounterAttribute_Flags::FIERY,
        .pose_table = unitEmber_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_hermos",
        .model_name = "c_elmos",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 15,
        .unk_32 = 15,
        .base_alpha = 255,
        .defense = unitEmber_defense,
        .defense_attr = unitEmber_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitEmber_pose_table,
    },
};
BattleUnitKindPart unitPhantomEmber_parts[] = {
    {
        .index = 1,
        .name = "btl_un_phantom",
        .model_name = "c_phantom",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 35.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitEmber_defense,
        .defense_attr = unitEmber_defense_attr,
        .attribute_flags = 0x0000'0009 | PartsAttribute_Flags::WEAK_TO_ICE_POWER,
        .counter_attribute_flags = PartsCounterAttribute_Flags::FIERY,
        .pose_table = unitEmber_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_phantom",
        .model_name = "c_phantom",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 15,
        .unk_32 = 15,
        .base_alpha = 255,
        .defense = unitEmber_defense,
        .defense_attr = unitEmber_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitEmber_pose_table,
    },
};

BattleUnitKind unit_LavaBubble = {
    .unit_type = BattleUnitType::LAVA_BUBBLE,
    .unit_name = "btl_un_bubble",
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
    .height = 36,
    .hit_offset = { 8, 24 },
    .center_offset = { 0.0f, 18.0f, 0.0f },
    .hp_gauge_offset = { 0, -3 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 13.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 13.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 13.0f, 23.4f, 0.0f },
    .cut_base_offset = { 0.0f, 18.0f, 0.0f },
    .cut_width = 26.0f,
    .cut_height = 36.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_BUBBLE_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::UNQUAKEABLE,
    .status_vulnerability = &unitEmber_status,
    .num_parts = 2,
    .parts = unitLavaBubble_parts,
    .init_evt_code = (void*)unitLavaBubble_init_event,
    .data_table = unitEmber_data_table,
};
BattleUnitKind unit_Ember = {
    .unit_type = BattleUnitType::EMBER,
    .unit_name = "btl_un_hermos",
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
    .height = 36,
    .hit_offset = { 8, 24 },
    .center_offset = { 0.0f, 18.0f, 0.0f },
    .hp_gauge_offset = { 0, -3 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 13.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 13.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 13.0f, 23.4f, 0.0f },
    .cut_base_offset = { 0.0f, 18.0f, 0.0f },
    .cut_width = 26.0f,
    .cut_height = 36.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_BUBBLE_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::UNQUAKEABLE,
    .status_vulnerability = &unitEmber_status,
    .num_parts = 2,
    .parts = unitEmber_parts,
    .init_evt_code = (void*)unitEmber_init_event,
    .data_table = unitEmber_data_table,
};
BattleUnitKind unit_PhantomEmber = {
    .unit_type = BattleUnitType::PHANTOM_EMBER,
    .unit_name = "btl_un_phantom",
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
    .height = 36,
    .hit_offset = { 8, 24 },
    .center_offset = { 0.0f, 18.0f, 0.0f },
    .hp_gauge_offset = { 0, -3 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 13.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 13.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 13.0f, 23.4f, 0.0f },
    .cut_base_offset = { 0.0f, 18.0f, 0.0f },
    .cut_width = 26.0f,
    .cut_height = 36.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_BUBBLE_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::UNQUAKEABLE,
    .status_vulnerability = &unitPhantomEmber_status,
    .num_parts = 2,
    .parts = unitPhantomEmber_parts,
    .init_evt_code = (void*)unitPhantomEmber_init_event,
    .data_table = unitEmber_data_table,
};

const BattleUnitSetup unitLavaBubble_spawn_entry = {
    .unit_kind_params = &unit_LavaBubble,
    .alliance = 1,
    .attack_phase = 0x04000004,
    .position = { -1000.0f, -1000.0f, -1000.0f },
    .addl_target_offset_x = 0,
    .unit_work = { 0, 0, 0, 0 },
    .item_drop_table = nullptr,
};
const BattleUnitSetup unitEmber_spawn_entry = {
    .unit_kind_params = &unit_Ember,
    .alliance = 1,
    .attack_phase = 0x04000004,
    .position = { -1000.0f, -1000.0f, -1000.0f },
    .addl_target_offset_x = 0,
    .unit_work = { 0, 0, 0, 0 },
    .item_drop_table = nullptr,
};
const BattleUnitSetup unitPhantomEmber_spawn_entry = {
    .unit_kind_params = &unit_PhantomEmber,
    .alliance = 1,
    .attack_phase = 0x04000004,
    .position = { -1000.0f, -1000.0f, -1000.0f },
    .addl_target_offset_x = 0,
    .unit_work = { 0, 0, 0, 0 },
    .item_drop_table = nullptr,
};

// Function / USER_FUNC definitions.

int32_t GetEffectType(BattleWorkUnit* unit) {
    switch (unit->true_kind) {
        case BattleUnitType::LAVA_BUBBLE:   return 5;
        case BattleUnitType::PHANTOM_EMBER: return 7;
    }
    return 6;
}

EVT_DEFINE_USER_FUNC(unitEmber_eff_fireball) {
    int32_t id = evtGetValue(evt, evt->evtArguments[0]);
    id = ttyd::battle_sub::BattleTransID(evt, id);
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, id);
    auto* part = ttyd::battle_unit::BtlUnit_GetPartsPtr(unit, 2);
    
    auto*& eff = g_FireballEff;
    
    if (isFirstCall) {
        eff = ttyd::eff_fire::effFireEntry(
            0.0f, -1000.0f, 0.0f, 1.0f, GetEffectType(unit), 0x6a);
        
        intptr_t work = reinterpret_cast<intptr_t>(eff->eff_work);
        *reinterpret_cast<float*>(work + 0x78) = 0.0f;
        *reinterpret_cast<float*>(work + 0x7c) = 0.0f;
    }
    
    intptr_t work = reinterpret_cast<intptr_t>(eff->eff_work);
    if (work != 0) {
        gc::vec3 pos;
        ttyd::battle_unit::BtlUnit_GetPartsPos(part, &pos.x, &pos.y, &pos.z);
        
        *reinterpret_cast<float*>(work + 0x04) = pos.x;
        *reinterpret_cast<float*>(work + 0x08) = pos.y;
        *reinterpret_cast<float*>(work + 0x0c) = pos.z;
        
        *reinterpret_cast<float*>(work + 0x78) += 0.1f * unit->unk_scale;
        if (*reinterpret_cast<float*>(work + 0x78) > unit->unk_scale) {
            *reinterpret_cast<float*>(work + 0x78) = unit->unk_scale;
        }
        *reinterpret_cast<float*>(work + 0x7c) += 0.1f * unit->unk_scale;
        if (*reinterpret_cast<float*>(work + 0x7c) > unit->unk_scale) {
            *reinterpret_cast<float*>(work + 0x7c) = unit->unk_scale;
        }
    }
    return 0;
}

EVT_DEFINE_USER_FUNC(unitEmber_eff_flamethrower) {
    int32_t id = evtGetValue(evt, evt->evtArguments[0]);
    id = ttyd::battle_sub::BattleTransID(evt, id);
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, id);
    
    int32_t eff_index = evtGetValue(evt, evt->evtArguments[1]);
    auto*& eff = g_FlamethrowerEffs[eff_index];
    
    if (isFirstCall) {
        eff = ttyd::eff_fire::effFireEntry(
            0.0f, -1000.0f, 0.0f, 1.0f, GetEffectType(unit), 0x6a);
        
        intptr_t work = reinterpret_cast<intptr_t>(eff->eff_work);
        *reinterpret_cast<float*>(work + 0x78) = 2.0f;
        *reinterpret_cast<float*>(work + 0x7c) = 
            unit->unk_scale * kFlamethrowerScale[eff_index];
    }
    
    intptr_t work = reinterpret_cast<intptr_t>(eff->eff_work);
    if (work != 0) {
        gc::vec3 pos;
        ttyd::battle_unit::BtlUnit_GetPos(unit, &pos.x, &pos.y, &pos.z);
        pos.y -= 15.0f;
        pos.z -= 10.0f;
        
        *reinterpret_cast<float*>(work + 0x04) =
            pos.x + 24.0 * unit->movement_params.face_direction * (eff_index + 1);
        *reinterpret_cast<float*>(work + 0x08) =
            pos.y - 2.0f * kFlamethrowerScale[eff_index];
        *reinterpret_cast<float*>(work + 0x0c) = pos.z;
    }
    return 0;
}

EVT_DEFINE_USER_FUNC(unitEmber_copy_status) {
    auto* unit1 = ttyd::battle::BattleGetUnitPtr(
        g_BattleWork, ttyd::battle_sub::BattleTransID(
            evt, evtGetValue(evt, evt->evtArguments[0])));
    auto* unit2 = ttyd::battle::BattleGetUnitPtr(
        g_BattleWork, ttyd::battle_sub::BattleTransID(
            evt, evtGetValue(evt, evt->evtArguments[1])));
        
    unit2->size_change_turns = unit1->size_change_turns;
    unit2->size_change_strength = unit1->size_change_strength;
    
    unit2->current_hp = unit1->current_hp;
    if (unit2->current_hp > unit2->max_hp) unit2->current_hp = unit2->max_hp;
    unit2->hp_gauge_params.previous_hp = unit2->current_hp;
    unit2->hp_gauge_params.target_hp = unit2->current_hp;
    
    float hp_fullness = static_cast<float>(unit2->current_hp) / unit2->max_hp;
    unit2->hp_gauge_params.fullness = hp_fullness;
    unit2->hp_gauge_params.fullness_target = hp_fullness;
    
    return 2;
}

// Evt definitions.

EVT_BEGIN(unitEmber_spawn_event)
    USER_FUNC(btlevtcmd_AnimeChangePose, LW(10), LW(11), PTR("S_1"))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(12), LW(13), LW(14))

    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(15))
    SWITCH(LW(15))
        CASE_EQUAL((int32_t)BattleUnitType::LAVA_BUBBLE)
            USER_FUNC(btlevtcmd_SpawnUnit, LW(3), PTR(&unitLavaBubble_spawn_entry), 0)
        CASE_EQUAL((int32_t)BattleUnitType::EMBER)
            USER_FUNC(btlevtcmd_SpawnUnit, LW(3), PTR(&unitEmber_spawn_entry), 0)
        CASE_ETC()
            USER_FUNC(btlevtcmd_SpawnUnit, LW(3), PTR(&unitPhantomEmber_spawn_entry), 0)
    END_SWITCH()

    USER_FUNC(unitEmber_copy_status, -2, LW(3))
    USER_FUNC(btlevtcmd_SetPos, LW(3), LW(12), LW(13), LW(14))
    USER_FUNC(btlevtcmd_JumpSetting, LW(3), 0, 10, FLOAT(0.5))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BUBBLE_SPLIT1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_SetRotateOffset, LW(3), 0, 10, 0)
        SET(LW(12), 0)
        DO(0)
            WAIT_FRM(1)
            ADD(LW(12), 1)
            IF_LARGE_EQUAL(LW(12), 30)
                DO_BREAK()
            END_IF()
            USER_FUNC(btlevtcmd_AddRotate, LW(3), 0, 0, 40)
        WHILE()
        USER_FUNC(btlevtcmd_SetRotate, LW(3), 0, 0, 0)
        USER_FUNC(btlevtcmd_SetRotateOffset, LW(3), 0, 0, 0)
    END_BROTHER()
    USER_FUNC(btlevtcmd_JumpPosition, LW(3), LW(0), LW(1), LW(2), FLOAT(30.0), -1)
    USER_FUNC(btlevtcmd_SetHomePos, LW(3), LW(0), LW(1), LW(2))
    WAIT_MSEC(1000)
    RETURN()
EVT_END()

EVT_BEGIN(unitEmber_fire_damaged_event)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_2B"))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    // Wait for max(X position, 10) frames.
    IF_SMALL(LW(0), 10)
        SET(LW(0), 10)
    END_IF()
    WAIT_FRM(LW(0))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    DO(0)
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 40)
        IF_SMALL_EQUAL(LW(0), 170)
            ADD(LW(2), 10)
            USER_FUNC(btlevtcmd_CheckSpace, LW(3), LW(0), LW(1), LW(2), 40, 0, 0, 0)
            IF_EQUAL(LW(3), -1)
                RUN_CHILD_EVT(PTR(&unitEmber_spawn_event))
                WAIT_MSEC(500)
                GOTO(99)
            END_IF()
        ELSE()
            DO_BREAK()
        END_IF()
    WHILE()
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    DO(0)
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 40)
        IF_LARGE_EQUAL(LW(0), 10)
            SUB(LW(2), 10)
            USER_FUNC(btlevtcmd_CheckSpace, LW(3), LW(0), LW(1), LW(2), 40, 0, 0, 0)
            IF_EQUAL(LW(3), -1)
                RUN_CHILD_EVT(PTR(&unitEmber_spawn_event))
                WAIT_MSEC(500)
                GOTO(99)
            END_IF()
        ELSE()
            DO_BREAK()
        END_IF()
    WHILE()
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitEmber_flamethrower_sub_event)
    BROTHER_EVT()
        USER_FUNC(unitEmber_eff_flamethrower, -2, LW(0))
    END_BROTHER()
    WAIT_FRM(120)
    RETURN()
EVT_END()

EVT_BEGIN(unitEmber_flamethrower_attack_event)
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
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, LW(3))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_1A"))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BUBBLE_MOVE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_DivePosition, -2, -40, LW(1), -15, 30, 0, 4, 0, -1)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BUBBLE_ATTACK4"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_2A"))
    DO(20)
        USER_FUNC(btlevtcmd_AddPos, -2, 2, 0, 0)
        WAIT_FRM(2)
        USER_FUNC(btlevtcmd_AddPos, -2, -2, 0, 0)
        WAIT_FRM(2)
    WHILE()
    BROTHER_EVT()
        SET(LW(0), 0)
        DO(10)
            RUN_EVT(PTR(&unitEmber_flamethrower_sub_event))
            WAIT_FRM(2)
            ADD(LW(0), 1)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BUBBLE_ATTACK2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_2B"))
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    SET(LW(10), 0)
LBL(0)
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
    // Fixed original logic, which just skipped all other enemies on a miss.
    GOTO(97)
LBL(91)
    IF_EQUAL(LW(10), 0)
        USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
        // Only increment LW(10) here to avoid DAS.
        ADD(LW(10), 1)
    END_IF()
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    GOTO(97)
LBL(97)
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    IF_NOT_EQUAL(LW(3), -1)
        GOTO(0)
    END_IF()
LBL(98)
    WAIT_FRM(180)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_1A"))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BUBBLE_MOVE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 30, 0, 4, 0, -1)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    WAIT_MSEC(400)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitEmber_fireball_attack_event)
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
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("T_1"))
        WAIT_FRM(30)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_1"))
    END_BROTHER()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BUBBLE_ATTACK2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 2, 0x200'0000)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 5)
    USER_FUNC(btlevtcmd_SetPartsPos, -2, 2, LW(0), LW(1), LW(2))
    BROTHER_EVT()
        USER_FUNC(unitEmber_eff_fireball, -2)
    END_BROTHER()
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 0)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(10), LW(11), LW(12))
    SUBF(LW(10), LW(0))
    SUBF(LW(12), LW(2))
    MULF(LW(10), FLOAT(0.7))
    MULF(LW(12), FLOAT(0.6))
    ADDF(LW(0), LW(10))
    ADDF(LW(2), LW(12))
    USER_FUNC(btlevtcmd_DivePartsPosition, -2, 2, LW(0), LW(1), LW(2), 40, 0, 4, 0, -1)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 5)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(10), LW(11), LW(12))
    SUBF(LW(10), LW(0))
    SUBF(LW(12), LW(2))
    MULF(LW(10), FLOAT(0.4))
    MULF(LW(12), FLOAT(0.4))
    ADDF(LW(0), LW(10))
    ADDF(LW(2), LW(12))
    USER_FUNC(btlevtcmd_DivePartsPosition, -2, 2, LW(0), LW(1), LW(2), 20, 0, 4, 0, -1)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetPos, -2, EVT_NULLPTR, LW(1), EVT_NULLPTR)
    USER_FUNC(btlevtcmd_DivePartsPosition, -2, 2, LW(0), LW(1), LW(2), 30, 0, 1, 0, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BUBBLE_ATTACK3"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 2, 0x200'0000)
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
    GOTO(98)
LBL(91)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(evt_eff, PTR(""), PTR("kemuri_test"), 16, LW(0), LW(1), LW(2), FLOAT(1.0), 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    GOTO(98)
LBL(98)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    WAIT_MSEC(400)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitEmber_melee_attack_event)
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
    BROTHER_EVT_ID(LW(15))
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BUBBLE_MOVE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        WAIT_FRM(120)
    END_BROTHER()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_1A"))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 13)
    SUB(LW(1), 20)
    IF_SMALL(LW(1), 0)
        SET(LW(1), 0)
    END_IF()
    ADD(LW(2), 5)
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 30, 0, 0, 0, -1)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 5)
    SUB(LW(1), 20)
    IF_SMALL(LW(1), 0)
        SET(LW(1), 0)
    END_IF()
    ADD(LW(2), 5)
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 30, 0, 4, 0, -1)
    DELETE_EVT(LW(15))
    WAIT_FRM(30)
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
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BUBBLE_FIRE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    DO(4)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_1B"))
        WAIT_FRM(10)
    WHILE()
    GOTO(98)
LBL(91)
    USER_FUNC(evt_btl_camera_shake_h, 0, 1, 1, 8, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_add_zoom, 0, -50)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BUBBLE_FIRE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_1B"))
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    WAIT_FRM(10)
    DO(3)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_1B"))
        WAIT_FRM(10)
    WHILE()
    GOTO(98)
LBL(98)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BUBBLE_MOVE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    BROTHER_EVT()
        WAIT_FRM(5)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_1A"))
    END_BROTHER()
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 40)
    ADD(LW(1), 20)
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 30, 20, 0, 0, -1)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 100)
    ADD(LW(1), 10)
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 30, -30, 0, 0, -1)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 40, 25, 4, 0, -1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_1"))
    WAIT_MSEC(400)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitEmber_attack_event)
    USER_FUNC(btlevtcmd_EnemyItemUseCheck, -2, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(LW(0))
        GOTO(99)
        RETURN()
    END_IF()
    
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(0))
    SWITCH(LW(0))
        CASE_EQUAL((int32_t)BattleUnitType::LAVA_BUBBLE)
            USER_FUNC(btlevtcmd_DrawLots, LW(0), 3, 70, 30, 0)
            SET(LW(10), PTR(&unitLavaBubble_weaponMelee))
            SET(LW(11), PTR(&unitLavaBubble_weaponFireball))
            SET(LW(12), PTR(&unitEmber_weaponFlamethrower))  // should never hit
        CASE_EQUAL((int32_t)BattleUnitType::EMBER)
            USER_FUNC(btlevtcmd_DrawLots, LW(0), 3, 50, 30, 20)
            SET(LW(10), PTR(&unitEmber_weaponMelee))
            SET(LW(11), PTR(&unitEmber_weaponFireball))
            SET(LW(12), PTR(&unitEmber_weaponFlamethrower))
        CASE_ETC()
            USER_FUNC(btlevtcmd_DrawLots, LW(0), 3, 55, 30, 15)
            SET(LW(10), PTR(&unitPhantomEmber_weaponMelee))
            SET(LW(11), PTR(&unitPhantomEmber_weaponFireball))
            SET(LW(12), PTR(&unitPhantomEmber_weaponFlamethrower))
    END_SWITCH()

    SWITCH(LW(0))
        CASE_EQUAL(0)
            SET(LW(9), LW(10))
            RUN_CHILD_EVT(PTR(&unitEmber_melee_attack_event))
        CASE_EQUAL(1)
            SET(LW(9), LW(11))
            RUN_CHILD_EVT(PTR(&unitEmber_fireball_attack_event))
        CASE_ETC()
            SET(LW(9), LW(12))
            RUN_CHILD_EVT(PTR(&unitEmber_flamethrower_attack_event))
    END_SWITCH()
    
LBL(99)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitEmber_fire_counter_event)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BUBBLE_FIRE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_2B"))
    WAIT_MSEC(1000)
    RETURN()
EVT_END()

EVT_BEGIN(unitEmber_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    RETURN()
EVT_END()

EVT_BEGIN(unitEmber_wait_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitEmber_common_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitEmber_wait_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitEmber_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitEmber_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitEmber_attack_event))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetPos, -2, LW(0), 10, LW(2))
    USER_FUNC(btlevtcmd_SetHomePos, -2, LW(0), 10, LW(2))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitLavaBubble_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::LAVA_BUBBLE)
    RUN_CHILD_EVT(unitEmber_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitEmber_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::EMBER)
    RUN_CHILD_EVT(unitEmber_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitPhantomEmber_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::PHANTOM_EMBER)
    RUN_CHILD_EVT(unitEmber_common_init_event)
    RETURN()
EVT_END()

}  // namespace mod::tot::custom