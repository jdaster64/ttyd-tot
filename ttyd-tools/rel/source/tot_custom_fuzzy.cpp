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
constexpr const int32_t UW_BattleUnitType = 0;

}  // namespace

// Evt / Function declarations.

extern const int32_t unitFuzzy_init_event[];
extern const int32_t unitGreenFuzzy_init_event[];
extern const int32_t unitFlowerFuzzy_init_event[];
extern const int32_t unitFuzzy_common_init_event[];
extern const int32_t unitFuzzy_attack_event[];
extern const int32_t unitFuzzy_damage_event[];
extern const int32_t unitFuzzy_wait_event[];
extern const int32_t unitFuzzy_drain_attack_event[];
extern const int32_t unitFuzzy_magic_attack_event[];
extern const int32_t unitFuzzy_check_friend_event[];
extern const int32_t unitFuzzy_spawn_event[];
extern const int32_t unitFuzzy_bounce_event[];
extern const int32_t unitFuzzy_return_home_event[];
extern const int32_t unitFuzzy_counter_damage_event[];

EVT_DECLARE_USER_FUNC(unitFuzzy_copy_status, 2)
EVT_DECLARE_USER_FUNC(unitFuzzy_get_magic_weight, 2)

// Unit data.

int8_t unitFuzzy_defense[] = { 0, 0, 0, 0, 0 };
int8_t unitFuzzy_defense_attr[] = { 0, 0, 0, 0, 0 };

StatusVulnerability unitFuzzy_status = {
     95,  95,  90, 100,  95, 100, 100,  80,
    100,  90, 100,  90, 100,  95,  70,  95,
     90, 100,  90, 100, 100, 100,
};
StatusVulnerability unitGreenFuzzy_status = {
     85,  85,  80, 100,  85, 100, 100,  70,
    100,  90, 100,  90, 100,  95,  60,  85,
     80, 100,  80, 100, 100,  95,
};
StatusVulnerability unitFlowerFuzzy_status = {
     85,  85,  80, 100,  85, 100, 100,  70,
    100,  90, 100,  90, 100,  95,  60,  85,
     80, 100,  80, 100, 100,  95,
};

PoseTableEntry unitFuzzy_pose_table[] = {
    1, "CBN_N_1",
    2, "CBN_Y_1",
    9, "CBN_Y_1",
    5, "CBN_K_1",
    4, "CBN_X_1",
    3, "CBN_X_1",
    28, "CBN_S_1",
    29, "CBN_Q_1",
    30, "CBN_Q_1",
    31, "CBN_S_1",
    39, "CBN_D_1",
    50, "CBN_A_1",
    42, "CBN_R_1",
    40, "CBN_W_1",
    56, "CBN_X_1",
    57, "CBN_X_1",
    65, "CBN_T_1",
    69, "CBN_S_1",
};

DataTableEntry unitFuzzy_data_table[] = {
    25, (void*)unitFuzzy_counter_damage_event,
    26, (void*)unitFuzzy_counter_damage_event,
    27, (void*)unitFuzzy_counter_damage_event,
    28, (void*)unitFuzzy_counter_damage_event,
    29, (void*)unitFuzzy_counter_damage_event,
    30, (void*)unitFuzzy_counter_damage_event,
    31, (void*)unitFuzzy_counter_damage_event,
    32, (void*)unitFuzzy_counter_damage_event,
    33, (void*)unitFuzzy_counter_damage_event,
    34, (void*)unitFuzzy_counter_damage_event,
    35, (void*)unitFuzzy_counter_damage_event,
    36, (void*)unitFuzzy_counter_damage_event,
    48, (void*)btldefaultevt_Dummy,
    0, nullptr,
};

const PoseSoundTimingEntry unitFuzzy_pose_sound_timing_table[] = {
    { "CBN_S_1", 0.5833333f, 0, "SFX_ENM_CHORO1_MOVE1", 1 },
    { "CBN_S_1", 0.8333333f, 0, "SFX_ENM_CHORO1_WAIT2", 1 },
    { "CBN_S_1", 1.5833334f, 0, "SFX_ENM_CHORO1_WAIT1", 1 },
    { nullptr, 0.0f, 0, nullptr, 1 },
};

BattleWeapon unitFuzzy_weapon = {
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
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::JUMPLIKE,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING,
    .counter_resistance_flags = 0,
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
BattleWeapon unitGreenFlowerFuzzy_weapon = {
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
        AttackSpecialProperty_Flags::DEFENSE_PIERCING,
    .counter_resistance_flags = 0,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
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
BattleWeapon unitFlowerFuzzy_weaponFpDrain = {
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
    .damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .fp_damage_function = (void*)ttyd::battle_weapon_power::weaponGetFPPowerDefault,
    .fp_damage_function_params = { 3, 0, 0, 0, 0, 0, 0, 0 },
    
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
        AttackSpecialProperty_Flags::DEFENSE_PIERCING,
    .counter_resistance_flags = 0,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
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
BattleWeapon unitFuzzy_weaponCountered = {
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
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::JUMPLIKE,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags = 0,
    .counter_resistance_flags = 0,
    .target_weighting_flags = AttackTargetWeighting_Flags::UNKNOWN_0x2000,
        
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
BattleWeapon unitFlowerFuzzy_weaponMagic = {
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
        AttackTargetClass_Flags::MULTIPLE_TARGET |
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
        AttackSpecialProperty_Flags::DEFENSE_PIERCING,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = AttackTargetWeighting_Flags::UNKNOWN_0x2000,
        
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

BattleUnitKindPart unitFuzzy_parts[] = {
    {
        .index = 1,
        .name = "btl_un_chorobon",
        .model_name = "c_chorobon",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitFuzzy_defense,
        .defense_attr = unitFuzzy_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitFuzzy_pose_table,
    },
};
BattleUnitKindPart unitGreenFuzzy_parts[] = {
    {
        .index = 1,
        .name = "btl_un_green_chorobon",
        .model_name = "c_chorobon_g",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitFuzzy_defense,
        .defense_attr = unitFuzzy_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitFuzzy_pose_table,
    },
};
BattleUnitKindPart unitFlowerFuzzy_parts[] = {
    {
        .index = 1,
        .name = "btl_un_flower_chorobon",
        .model_name = "c_chorobon_f",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitFuzzy_defense,
        .defense_attr = unitFuzzy_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitFuzzy_pose_table,
    },
};

BattleUnitKind unit_Fuzzy = {
    .unit_type = BattleUnitType::FUZZY,
    .unit_name = "btl_un_chorobon",
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
    .height = 32,
    .hit_offset = { 5, 32 },
    .center_offset = { 0.0f, 16.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 14.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 14.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 9.0f, 20.0f, 0.0f },
    .cut_base_offset = { 0.0f, 16.0f, 0.0f },
    .cut_width = 28.0f,
    .cut_height = 32.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_CHORO1_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitFuzzy_status,
    .num_parts = 1,
    .parts = unitFuzzy_parts,
    .init_evt_code = (void*)unitFuzzy_init_event,
    .data_table = unitFuzzy_data_table,
};
BattleUnitKind unit_GreenFuzzy = {
    .unit_type = BattleUnitType::GREEN_FUZZY,
    .unit_name = "btl_un_green_chorobon",
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
    .height = 32,
    .hit_offset = { 5, 32 },
    .center_offset = { 0.0f, 16.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 14.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 14.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 9.0f, 20.0f, 0.0f },
    .cut_base_offset = { 0.0f, 16.0f, 0.0f },
    .cut_width = 28.0f,
    .cut_height = 32.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_CHORO1_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitGreenFuzzy_status,
    .num_parts = 1,
    .parts = unitGreenFuzzy_parts,
    .init_evt_code = (void*)unitGreenFuzzy_init_event,
    .data_table = unitFuzzy_data_table,
};
BattleUnitKind unit_FlowerFuzzy = {
    .unit_type = BattleUnitType::FLOWER_FUZZY,
    .unit_name = "btl_un_flower_chorobon",
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
    .height = 32,
    .hit_offset = { 5, 32 },
    .center_offset = { 0.0f, 16.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 14.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 14.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 9.0f, 20.0f, 0.0f },
    .cut_base_offset = { 0.0f, 16.0f, 0.0f },
    .cut_width = 28.0f,
    .cut_height = 32.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_CHORO1_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitFlowerFuzzy_status,
    .num_parts = 1,
    .parts = unitFlowerFuzzy_parts,
    .init_evt_code = (void*)unitFlowerFuzzy_init_event,
    .data_table = unitFuzzy_data_table,
};

const BattleUnitSetup unitGreenFuzzy_spawn_entry = {
    .unit_kind_params = &unit_GreenFuzzy,
    .alliance = 1,
    .attack_phase = 0x04000004,
    .position = { -1000.0f, -1000.0f, -1000.0f },
    .addl_target_offset_x = 0,
    .unit_work = { 0, 0, 0, 0 },
    .item_drop_table = nullptr,
};

// Function / USER_FUNC definitions.

EVT_DEFINE_USER_FUNC(unitFuzzy_copy_status) {
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

EVT_DEFINE_USER_FUNC(unitFuzzy_get_magic_weight) {
    auto* unit = ttyd::battle::BattleGetUnitPtr(
        g_BattleWork, ttyd::battle_sub::BattleTransID(
            evt, evtGetValue(evt, evt->evtArguments[0])));
    
    int32_t weight = 0;
    if (unit->current_fp > 5) {
        weight = 100;
    } else if (unit->current_fp > 1) {
        weight = (unit->current_fp - 1) * 25;
    }

    evtSetValue(evt, evt->evtArguments[1], weight);
    return 2;
}

// Evt definitions.

EVT_BEGIN(unitFuzzy_attack_event)
    USER_FUNC(btlevtcmd_EnemyItemUseCheck, -2, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(LW(0))
        USER_FUNC(btlevtcmd_StartWaitEvent, -2)
        RETURN()
    END_IF()

LBL(0)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(0))
    SWITCH(LW(0))
        CASE_EQUAL((int32_t)BattleUnitType::FLOWER_FUZZY)
            SET(LW(1), 100)
            USER_FUNC(unitFuzzy_get_magic_weight, -2, LW(2))
            SUB(LW(1), LW(2))
            USER_FUNC(btlevtcmd_DrawLots, LW(0), 2, LW(1), LW(2))
            SET(LW(9), PTR(&unitGreenFlowerFuzzy_weapon))
        CASE_EQUAL((int32_t)BattleUnitType::GREEN_FUZZY)
            USER_FUNC(btlevtcmd_DrawLots, LW(0), 3, 50, 0, 50)
            SET(LW(9), PTR(&unitGreenFlowerFuzzy_weapon))
        CASE_ETC()
            SET(LW(0), 0)
            SET(LW(9), PTR(&unitFuzzy_weapon))
    END_SWITCH()

    SWITCH(LW(0))
        CASE_EQUAL(1)
            RUN_CHILD_EVT(PTR(&unitFuzzy_magic_attack_event))
        CASE_EQUAL(2)
            RUN_CHILD_EVT(PTR(&unitFuzzy_check_friend_event))
            IF_EQUAL(LW(0), 0)
                GOTO(0)
            END_IF()
        CASE_ETC()
            RUN_CHILD_EVT(PTR(&unitFuzzy_drain_attack_event))
    END_SWITCH()

LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitFuzzy_check_friend_event)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    DO(0)
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 40)
        IF_SMALL_EQUAL(LW(0), 170)
            ADD(LW(2), 10)
            USER_FUNC(btlevtcmd_CheckSpace, LW(3), LW(0), LW(1), LW(2), 40, 0, 0, 0)
            IF_EQUAL(LW(3), -1)
                RUN_CHILD_EVT(PTR(&unitFuzzy_spawn_event))
                WAIT_MSEC(500)
                SET(LW(0), 1)
                GOTO(99)
            END_IF()
        ELSE()
            DO_BREAK()
        END_IF()
    WHILE()
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
                RUN_CHILD_EVT(PTR(&unitFuzzy_spawn_event))
                WAIT_MSEC(500)
                SET(LW(0), 1)
                GOTO(99)
            END_IF()
        ELSE()
            DO_BREAK()
        END_IF()
    WHILE()
    SET(LW(0), 0)
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitFuzzy_counter_damage_event)
    USER_FUNC(btlevtcmd_SetScale, -2, FLOAT(1.0), FLOAT(1.0), FLOAT(1.0))
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_GetDamageCode, LW(10), LW(0))
    RUN_CHILD_EVT(PTR(&subsetevt_counter_damage))
    USER_FUNC(btlevtcmd_CheckDamageCode, -2, 0x200, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_CheckDamageCode, -2, 0x800, LW(0))
        IF_EQUAL(LW(0), 0)
            RETURN()
        END_IF()
    END_IF()
    RUN_CHILD_EVT(PTR(&unitFuzzy_return_home_event))
    RETURN()
EVT_END()

EVT_BEGIN(unitFuzzy_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    RETURN()
EVT_END()

EVT_BEGIN(unitFuzzy_drain_attack_event)
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
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(evt_btl_camera_set_mode, 0, 8)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, LW(3))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 0, FLOAT(4.0), FLOAT(0.6))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(5), LW(6), LW(7))
    SET(LW(6), 0)
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(5), 50)
    SET(LW(8), 50)
    RUN_CHILD_EVT(PTR(&unitFuzzy_bounce_event))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 350)
    USER_FUNC(evt_btl_camera_nomove_y_onoff, 0, 1)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 60, 0, FLOAT(0.7))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 5)
    SUB(LW(1), 10)
    SUB(LW(2), 2)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, -1)
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
    USER_FUNC(btlevtcmd_JumpSetting, -2, 0, FLOAT(3.0), FLOAT(0.2))
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, LW(3), LW(0), 5)
    SET(LW(1), 0)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 0, -1)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 0, FLOAT(1.5), FLOAT(0.2))
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, LW(3), LW(0), 30)
    SET(LW(1), 0)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 0, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_CHORO1_LANDING1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("CBN_N_1"))
    WAIT_MSEC(1000)
    GOTO(98)
LBL(91)
    SET(LF(0), 0)
    USER_FUNC(btlevtcmd_PreCheckCounter, -2, LW(3), LW(4), LW(9), 256, LW(5))
    IF_EQUAL(LW(5), 17)
        SET(LF(0), 1)
    END_IF()
    IF_EQUAL(LW(5), 14)
        SET(LF(0), 1)
    END_IF()
    IF_EQUAL(LF(0), 1)
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
        USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
        SET(LW(9), PTR(&unitFuzzy_weaponCountered))
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_CHORO1_ATTACK1_1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetBodyId, LW(3), LW(0))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(3), LW(0), 37)
    INLINE_EVT()
        SET(LW(0), 89)
        SET(LW(1), 1)
        SET(LW(2), 2)
        SET(LW(5), LW(2))
LBL(20)
        SUB(LW(5), 1)
        IF_SMALL_EQUAL(LW(5), 0)
            SET(LW(5), LW(2))
            IF_EQUAL(LW(1), 0)
                SET(LW(1), 1)
            ELSE()
                SET(LW(1), 0)
            END_IF()
        END_IF()
        USER_FUNC(btlevtcmd_SetDispOffset, LW(3), LW(1), 0, 0)
        WAIT_FRM(1)
        SUB(LW(0), 1)
        IF_LARGE_EQUAL(LW(0), 1)
            GOTO(20)
        END_IF()
        USER_FUNC(btlevtcmd_SetDispOffset, LW(3), 0, 0, 0)
    END_INLINE()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_CHORO1_ATTACK1_2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(15))
    SET(LW(14), 20)
    IF_EQUAL(LW(15), -1)
        SET(LW(14), -20)
    END_IF()
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, LW(14))
    USER_FUNC(btlevtcmd_AnimeSetMotionSpeed, -2, 1, FLOAT(1.5))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("CBN_A_2"))
    WAIT_FRM(90)
    USER_FUNC(btlevtcmd_AnimeSetMotionSpeed, -2, 1, FLOAT(1.0))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("CBN_R_2"))
    USER_FUNC(btlevtcmd_SetScale, -2, FLOAT(1.0), FLOAT(1.0), FLOAT(1.0))
    DO(4)
        WAIT_FRM(1)
        USER_FUNC(btlevtcmd_AddScale, -2, 0, FLOAT(-0.2), 0)
    WHILE()
    USER_FUNC(btlevtcmd_SetScale, -2, FLOAT(1.0), FLOAT(0.2), FLOAT(1.0))
    DO(20)
        WAIT_FRM(1)
        USER_FUNC(btlevtcmd_AddScale, -2, FLOAT(-0.01), FLOAT(0.05), 0)
    WHILE()
    WAIT_FRM(1)
    USER_FUNC(btlevtcmd_SetScale, -2, FLOAT(1.0), FLOAT(1.0), FLOAT(1.0))
    USER_FUNC(evt_btl_camera_shake_h, 0, 1, 1, 8, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_add_zoom, 0, -50)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_CHORO1_ATTACK2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
    
    // For Flower Fuzzies, prefer draining FP if the target has any.
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(0))
    IF_EQUAL(LW(0), (int32_t)BattleUnitType::FLOWER_FUZZY)
        USER_FUNC(btlevtcmd_GetFp, LW(3), LW(0))
        IF_LARGE(LW(0), 0)
            SET(LW(9), PTR(&unitFlowerFuzzy_weaponFpDrain))
            USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
            USER_FUNC(btlevtcmd_GetFpDamage, LW(3), LW(10))
            GOTO(97)
        END_IF()
    END_IF()
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    USER_FUNC(btlevtcmd_GetHpDamage, LW(3), LW(10))

LBL(97)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    IF_LARGE_EQUAL(LW(10), 1)
        USER_FUNC(btlevtcmd_AddScale, -2, FLOAT(0.5), FLOAT(0.5), 0)
    END_IF()
    DO(4)
        USER_FUNC(btlevtcmd_AddScale, -2, FLOAT(-0.08), FLOAT(0.12), 0)
        WAIT_FRM(1)
    WHILE()
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 40)
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.7))
    USER_FUNC(btlevtcmd_AddScale, -2, FLOAT(0.32), FLOAT(-0.48), 0)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 20, -1)
    DO(2)
        USER_FUNC(btlevtcmd_AddScale, -2, FLOAT(0.16), FLOAT(-0.24), 0)
        WAIT_FRM(1)
    WHILE()
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 20)
    USER_FUNC(btlevtcmd_AddScale, -2, FLOAT(-0.32), FLOAT(0.48), 0)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 10, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_CHORO1_LANDING1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    DO(2)
        USER_FUNC(btlevtcmd_AddScale, -2, FLOAT(0.16), FLOAT(-0.24), 0)
        WAIT_FRM(1)
    WHILE()
    DO(2)
        USER_FUNC(btlevtcmd_AddScale, -2, FLOAT(-0.16), FLOAT(0.24), 0)
        WAIT_FRM(1)
    WHILE()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("CBN_W_1"))
    IF_LARGE_EQUAL(LW(10), 1)
        DO(20)
            USER_FUNC(btlevtcmd_AddScale, -2, FLOAT(-0.0244), FLOAT(-0.0244), 0)
            WAIT_FRM(1)
        WHILE()
        USER_FUNC(btlevtcmd_GetBodyId, -2, LW(1))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(5), LW(6), LW(7))
        ADD(LW(6), 45)
        ADD(LW(7), 10)
        IF_EQUAL(LW(9), PTR(&unitFlowerFuzzy_weaponFpDrain))
            USER_FUNC(btlevtcmd_RecoverFp, -2, LW(1), LW(10))
            USER_FUNC(evt_eff, PTR(""), PTR("recovery"), 8, LW(5), LW(6), LW(7), LW(10), 0, 0, 0, 0, 0, 0, 0)
        ELSE()
            USER_FUNC(btlevtcmd_RecoverHp, -2, LW(1), LW(10))
            USER_FUNC(evt_eff, PTR(""), PTR("recovery"), 7, LW(5), LW(6), LW(7), LW(10), 0, 0, 0, 0, 0, 0, 0)
        END_IF()
        USER_FUNC(btlevtcmd_SetScale, -2, FLOAT(1.0), FLOAT(1.0), FLOAT(1.0))
    END_IF()
    DO(2)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        ADD(LW(1), 30)
        USER_FUNC(btlevtcmd_SetMoveSoundDataWork, -2, PTR("SFX_ENM_CHORO1_ATTACK3"), 0, 0, -1, -1, 0, 0)
        DO(2)
            USER_FUNC(btlevtcmd_AddScale, -2, FLOAT(0.16), FLOAT(-0.24), 0)
            WAIT_FRM(1)
        WHILE()
        USER_FUNC(btlevtcmd_SetScale, -2, FLOAT(1.0), FLOAT(1.0), FLOAT(1.0))
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 6, -2)
        WAIT_FRM(16)
        DO(2)
            USER_FUNC(btlevtcmd_AddScale, -2, FLOAT(0.16), FLOAT(-0.24), 0)
            WAIT_FRM(1)
        WHILE()
        USER_FUNC(btlevtcmd_SetScale, -2, FLOAT(1.0), FLOAT(1.0), FLOAT(1.0))
        USER_FUNC(btlevtcmd_SetMoveSoundDataWork, -2, 0, 0, 0, -1, -1, 0, 0)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 6, -2)
    WHILE()
    DO(2)
        USER_FUNC(btlevtcmd_AddScale, -2, FLOAT(0.16), FLOAT(-0.24), 0)
        WAIT_FRM(1)
    WHILE()
    DO(2)
        USER_FUNC(btlevtcmd_AddScale, -2, FLOAT(-0.16), FLOAT(0.24), 0)
        WAIT_FRM(1)
    WHILE()
    GOTO(98)
LBL(98)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_nomove_y_onoff, 0, 0)
    RUN_CHILD_EVT(PTR(&unitFuzzy_return_home_event))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitFuzzy_bounce_event)
    DO(2)
        USER_FUNC(btlevtcmd_AddScale, -2, FLOAT(0.16), FLOAT(-0.24), 0)
        WAIT_FRM(1)
    WHILE()
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
        USER_FUNC(btlevtcmd_SetScale, -2, FLOAT(1.0), FLOAT(1.0), FLOAT(1.0))
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(6), LW(2), 0, -1)
        DO(2)
            USER_FUNC(btlevtcmd_AddScale, -2, FLOAT(0.16), FLOAT(-0.24), 0)
            WAIT_FRM(1)
        WHILE()
        GOTO(0)
    ELSE()
        USER_FUNC(btlevtcmd_SetScale, -2, FLOAT(1.0), FLOAT(1.0), FLOAT(1.0))
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(5), LW(6), LW(7), 0, -1)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_CHORO1_LANDING1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        DO(2)
            USER_FUNC(btlevtcmd_AddScale, -2, FLOAT(0.16), FLOAT(-0.24), 0)
            WAIT_FRM(1)
        WHILE()
    END_IF()
    DO(2)
        USER_FUNC(btlevtcmd_AddScale, -2, FLOAT(-0.16), FLOAT(0.24), 0)
        WAIT_FRM(1)
    WHILE()
    RETURN()
EVT_END()

EVT_BEGIN(unitFuzzy_return_home_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 0, FLOAT(4.0), FLOAT(0.6))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(5), LW(6), LW(7))
    SET(LW(8), 40)
    RUN_CHILD_EVT(PTR(&unitFuzzy_bounce_event))
    RETURN()
EVT_END()

EVT_BEGIN(unitFuzzy_wait_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitFuzzy_magic_attack_event)
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), PTR(&unitFlowerFuzzy_weaponMagic))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, PTR(&unitFlowerFuzzy_weaponMagic), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            RETURN()
        END_IF()
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_WeaponAftereffect, PTR(&unitFlowerFuzzy_weaponMagic))
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, PTR(&unitFlowerFuzzy_weaponMagic))
    SET(LW(0), 2)
    MUL(LW(0), -1)
    USER_FUNC(btlevtcmd_RecoverFp, -2, 1, LW(0))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpSetting, -2, 0, 0, FLOAT(0.1))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), FLOAT(50.0), -1)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(evt_eff64, PTR(""), PTR("shock_n64"), 1, LW(0), LW(1), LW(2), 20, 20, 0, 0, 0, 0, 0, 0)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_CHORO2_MAGIC1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    WAIT_FRM(10)
    USER_FUNC(evt_eff64, PTR(""), PTR("shock_n64"), 1, LW(0), LW(1), LW(2), 20, 20, 0, 0, 0, 0, 0, 0)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_CHORO2_MAGIC1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    WAIT_FRM(10)
    USER_FUNC(evt_eff64, PTR(""), PTR("shock_n64"), 1, LW(0), LW(1), LW(2), 20, 20, 0, 0, 0, 0, 0, 0)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_CHORO2_MAGIC1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    WAIT_FRM(20)
    USER_FUNC(evt_btl_camera_shake_h, 0, 3, 0, 30, 0)
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    SET(LW(10), 0)
LBL(0)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), PTR(&unitFlowerFuzzy_weaponMagic), 256, LW(5))
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
    IF_EQUAL(LW(10), 0)
        USER_FUNC(btlevtcmd_ResultACDefence, LW(3), PTR(&unitFlowerFuzzy_weaponMagic))
        // Only increment LW(10) here to avoid DAS.
        ADD(LW(10), 1)
    END_IF()
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&unitFlowerFuzzy_weaponMagic), 256, LW(5))
    GOTO(97)
LBL(97)
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    IF_NOT_EQUAL(LW(3), -1)
        GOTO(0)
    END_IF()
LBL(99)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitFuzzy_spawn_event)
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, -1)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    WAIT_MSEC(500)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("CBN_K_1"))
    SET(LW(12), 120)
    SET(LW(13), 45)
    SET(LW(14), 10)
    SET(LW(15), 10)
    USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 10, 0)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_CHORO2_SPLIT1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    DO(0)
        WAIT_FRM(1)
        SUB(LW(12), 1)
        IF_SMALL_EQUAL(LW(12), 1)
            DO_BREAK()
        END_IF()
        SUB(LW(14), 1)
        IF_SMALL_EQUAL(LW(14), 0)
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, LW(13))
            SUB(LW(15), 1)
            SET(LW(14), LW(15))
            IF_EQUAL(LW(15), 7)
                USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_CHORO2_SPLIT1_2"), EVT_NULLPTR, 0, EVT_NULLPTR)
            END_IF()
        END_IF()
    WHILE()
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("CBN_S_1"))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(7), LW(8), LW(9))
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_JumpSetting, -2, 0, 10, FLOAT(0.5))
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(7), LW(8), LW(9), FLOAT(20.0), -1)
    END_BROTHER()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_CHORO2_SPLIT2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SpawnUnit, LW(3), PTR(&unitGreenFuzzy_spawn_entry), 0)
    USER_FUNC(unitFuzzy_copy_status, -2, LW(3))
    USER_FUNC(btlevtcmd_SetPos, LW(3), LW(7), LW(8), LW(9))
    USER_FUNC(btlevtcmd_JumpSetting, LW(3), 0, 10, FLOAT(0.5))
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_SetRotateOffset, LW(3), 0, 10, 0)
        SET(LW(12), 0)
        DO(0)
            WAIT_FRM(1)
            ADD(LW(12), 1)
            IF_LARGE_EQUAL(LW(12), 30)
                DO_BREAK()
            END_IF()
            USER_FUNC(btlevtcmd_AddRotate, LW(3), 0, 0, 20)
        WHILE()
        USER_FUNC(btlevtcmd_SetRotate, LW(3), 0, 0, 0)
        USER_FUNC(btlevtcmd_SetRotateOffset, LW(3), 0, 0, 0)
    END_BROTHER()
    USER_FUNC(btlevtcmd_JumpPosition, LW(3), LW(0), LW(1), LW(2), FLOAT(30.0), -1)
    USER_FUNC(btlevtcmd_SetHomePos, LW(3), LW(0), LW(1), LW(2))
    WAIT_MSEC(1000)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitFuzzy_common_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitFuzzy_wait_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitFuzzy_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitFuzzy_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitFuzzy_attack_event))
    USER_FUNC(btlevtcmd_AnimeSetPoseSoundTable, -2, 1, PTR(&unitFuzzy_pose_sound_timing_table))
    USER_FUNC(btlevtcmd_SetJumpSound, -2, PTR("SFX_ENM_CHORO1_MOVE1"), 0)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitFuzzy_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::FUZZY)
    RUN_CHILD_EVT(unitFuzzy_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitGreenFuzzy_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::GREEN_FUZZY)
    RUN_CHILD_EVT(unitFuzzy_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitFlowerFuzzy_init_event)
    USER_FUNC(btlevtcmd_SetMaxFp, -2, 5)
    USER_FUNC(btlevtcmd_SetFp, -2, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::FLOWER_FUZZY)
    RUN_CHILD_EVT(unitFuzzy_common_init_event)
    RETURN()
EVT_END()

}  // namespace mod::tot::custom