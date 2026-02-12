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

}  // namespace

// Evt / Function declarations.

extern const int32_t unitXNautPhD_init_event[];
extern const int32_t unitXNautPhD_attack_event[];
extern const int32_t unitXNautPhD_damage_event[];
extern const int32_t unitXNautPhD_wait_event[];
extern const int32_t unitXNautPhD_burn_potion_event[];
extern const int32_t unitXNautPhD_soft_potion_event[];
extern const int32_t unitXNautPhD_tiny_potion_event[];
extern const int32_t unitXNautPhD_dodgy_potion_event_self[];
extern const int32_t unitXNautPhD_dodgy_potion_event_Crump[];
extern const int32_t unitXNautPhD_regen_potion_event[];
extern const int32_t unitXNautPhD_one_recover_event[];
extern const int32_t unitXNautPhD_all_recover_event[];
extern const int32_t unitXNautPhD_attack_common_event1[];
extern const int32_t unitXNautPhD_attack_common_event2[];
extern const int32_t unitXNautPhD_attack_common_event3[];

EVT_DECLARE_USER_FUNC(unitXNautPhD_GetCrumpId, 2)

// Unit data.

int8_t unitXNautPhD_defense[] = { 0, 0, 0, 0, 0 };
int8_t unitXNautPhD_defense_attr[] = { 0, 0, 0, 0, 0 };

StatusVulnerability unitXNautPhD_status = {
     80,  80,  80, 100,  80, 100, 100,  70,
    100,  90, 100,  90, 100,  90,  70,  90,
     70, 100,  70, 100, 100,  95,
};

PoseTableEntry unitXNautPhD_pose_table[] = {
    1, "N_1",
    2, "Y_1",
    9, "Y_1",
    5, "K_1",
    4, "X_1",
    3, "X_1",
    27, "Z_1",
    28, "S_1",
    29, "D_1",
    30, "D_1",
    31, "S_1",
    39, "D_1",
    42, "R_1",
    40, "W_1",
    65, "T_1",
    69, "S_1",
};

DataTableEntry unitXNautPhD_data_table[] = {
    0, nullptr,
};

BattleWeapon unitXNautPhD_weaponFire = {
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
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::JUMPLIKE,
    .element = AttackElement::FIRE,
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
BattleWeapon unitXNautPhD_weaponSoft = {
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
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::JUMPLIKE,
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
    .def_change_chance = 75,
    .def_change_time = 3,
    .def_change_strength = -3,
    
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
BattleWeapon unitXNautPhD_weaponTiny = {
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
    .special_property_flags = AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = AttackTargetWeighting_Flags::WEIGHTED_RANDOM,
        
    // status chances
    .size_change_chance = 75,
    .size_change_time = 3,
    .size_change_strength = -2,
    
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
BattleWeapon unitXNautPhD_weaponDodgySelf = {
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
    .dodgy_chance = 100,
    .dodgy_time = 3,
    
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
BattleWeapon unitXNautPhD_weaponRegenAlly = {
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
        AttackTargetClass_Flags::CANNOT_TARGET_OPPOSING_ALLIANCE |
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
        AttackSpecialProperty_Flags::CANNOT_MISS |
        AttackSpecialProperty_Flags::UNGUARDABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::PREFER_LESS_HEALTHY |
        AttackTargetWeighting_Flags::PREFER_LOWER_HP |
        AttackTargetWeighting_Flags::PREFER_IN_PERIL,
        
    // status chances
    .hp_regen_time = 5,
    .hp_regen_strength = 2,
    
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
BattleWeapon unitXNautPhD_weaponRecoverOne = {
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
        AttackTargetClass_Flags::CANNOT_TARGET_OPPOSING_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags = 0,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::PREFER_LESS_HEALTHY |
        AttackTargetWeighting_Flags::PREFER_LOWER_HP |
        AttackTargetWeighting_Flags::PREFER_IN_PERIL,
        
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
BattleWeapon unitXNautPhD_weaponRecoverAll = {
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
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_OPPOSING_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags = 0,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = 0,
        
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

BattleUnitKindPart unitXNautPhD_parts[] = {
    {
        .index = 1,
        .name = "btl_un_gundan_zako_magician",
        .model_name = "c_zako_m",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 28.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 38.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitXNautPhD_defense,
        .defense_attr = unitXNautPhD_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitXNautPhD_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_gundan_zako_magician",
        .model_name = "c_zako_m",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitXNautPhD_defense,
        .defense_attr = unitXNautPhD_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_8000 |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitXNautPhD_pose_table,
    },
};

BattleUnitKind unit_XNautPhD = {
    .unit_type = BattleUnitType::X_NAUT_PHD,
    .unit_name = "btl_un_gundan_zako_magician",
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
    .width = 30,
    .height = 36,
    .hit_offset = { 0, 36 },
    .center_offset = { 0.0f, 18.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 15.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 15.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 12.0f, 27.0f, 0.0f },
    .cut_base_offset = { 0.0f, 18.0f, 0.0f },
    .cut_width = 30.0f,
    .cut_height = 36.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_3RD2_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitXNautPhD_status,
    .num_parts = 2,
    .parts = unitXNautPhD_parts,
    .init_evt_code = (void*)unitXNautPhD_init_event,
    .data_table = unitXNautPhD_data_table,
};

// Function / USER_FUNC definitions.

// Not used in Tower of Trials, but logic kept intact for simplicity's sake.
EVT_DEFINE_USER_FUNC(unitXNautPhD_GetCrumpId) {
    int32_t unit_id = -1;
    int32_t parts_id = 0;
    for (int32_t i = 0; i < 64; ++i) {
        auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, i);
        if (unit && unit->true_kind == BattleUnitType::LORD_CRUMP_CH_5) {
            unit_id = i;
            parts_id = ttyd::battle_unit::BtlUnit_GetBodyPartsId(unit);
            break;
        }
    }
    evtSetValue(evt, evt->evtArguments[0], unit_id);
    evtSetValue(evt, evt->evtArguments[1], parts_id);
    return 2;
}

// Evt definitions.

EVT_BEGIN(unitXNautPhD_attack_common_event1)
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    IF_EQUAL(LW(9), PTR(&unitXNautPhD_weaponTiny))
        USER_FUNC(btlevtcmd_GetEnemyBelong, LW(3), LW(10))
        IF_EQUAL(LW(10), 0)
            SET(LW(10), 125)
        ELSE()
            SET(LW(10), -125)
        END_IF()
        USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(10), 0, 1, LW(14))
        USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(14))
    ELSE()
        USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(14))
        USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(14))
    END_IF()
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD2_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        WAIT_FRM(40)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD2_ATTACK2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    END_BROTHER()
    IF_EQUAL(LW(9), PTR(&unitXNautPhD_weaponFire))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_1A"))
    END_IF()
    IF_EQUAL(LW(9), PTR(&unitXNautPhD_weaponSoft))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_2A"))
    END_IF()
    IF_EQUAL(LW(9), PTR(&unitXNautPhD_weaponTiny))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_3A"))
    END_IF()
    IF_EQUAL(LW(9), PTR(&unitXNautPhD_weaponDodgySelf))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_5A"))
    END_IF()
    IF_EQUAL(LW(9), PTR(&unitXNautPhD_weaponRecoverOne))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_4A"))
    END_IF()
    IF_EQUAL(LW(9), PTR(&unitXNautPhD_weaponRecoverAll))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_4A"))
    END_IF()
    IF_EQUAL(LW(9), PTR(&unitXNautPhD_weaponRegenAlly))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_4A"))
    END_IF()
    WAIT_FRM(80)
    RETURN()
EVT_END()

EVT_BEGIN(unitXNautPhD_attack_common_event2)
    IF_EQUAL(LW(9), PTR(&unitXNautPhD_weaponFire))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 2, PTR("B_1A"))
    END_IF()
    IF_EQUAL(LW(9), PTR(&unitXNautPhD_weaponSoft))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 2, PTR("B_1B"))
    END_IF()
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 10)
    ADD(LW(1), 20)
    USER_FUNC(btlevtcmd_SetPartsPos, -2, 2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD2_ATTACK3"), EVT_NULLPTR, 0, EVT_NULLPTR)
    IF_EQUAL(LW(9), PTR(&unitXNautPhD_weaponFire))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_1B"))
    END_IF()
    IF_EQUAL(LW(9), PTR(&unitXNautPhD_weaponSoft))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_2B"))
    END_IF()
    WAIT_FRM(24)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD2_ATTACK4"), EVT_NULLPTR, 0, EVT_NULLPTR)
    BROTHER_EVT()
        WAIT_FRM(24)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_1"))
    END_BROTHER()
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 2, 0x100'0000)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 2, 0x200'0000)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPartsSetting, -2, 2, 0, FLOAT(6.0), FLOAT(0.3))
    USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 2, LW(0), LW(1), LW(2), 0, -1)
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
LBL(90)
            USER_FUNC(btlevtcmd_JumpPartsContinue, -2, 2)
            USER_FUNC(btlevtcmd_GetPartsPos, -2, 2, LW(0), LW(1), LW(2))
            DO(12)
                USER_FUNC(evt_eff64, PTR(""), PTR("glass_n64"), 0, LW(0), LW(1), LW(2), FLOAT(1.0), 30, 0, 0, 0, 0, 0, 0)
            WHILE()
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD2_CRASH1"), EVT_NULLPTR, 0, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 2, PTR("B_2"))
            WAIT_FRM(8)
            USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 2, 0x100'0000)
            USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 2, 0x200'0000)
            GOTO(98)
LBL(91)
            IF_EQUAL(LW(9), PTR(&unitXNautPhD_weaponFire))
                USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
                USER_FUNC(evt_eff, PTR(""), PTR("kemuri_test"), 16, LW(0), LW(1), LW(2), FLOAT(1.0), 0, 0, 0, 0, 0, 0, 0)
            END_IF()
            USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
            DO(12)
                USER_FUNC(evt_eff64, PTR(""), PTR("glass_n64"), 0, LW(0), LW(1), LW(2), FLOAT(1.0), 30, 0, 0, 0, 0, 0, 0)
            WHILE()
            USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
            USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD2_CRASH1"), EVT_NULLPTR, 0, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 2, PTR("B_2"))
            WAIT_FRM(8)
            USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 2, 0x100'0000)
            USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 2, 0x200'0000)
            GOTO(98)
LBL(98)
            WAIT_FRM(10)
            USER_FUNC(evt_btl_camera_set_mode, 0, 0)
            WAIT_FRM(30)
LBL(99)
            USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
            RETURN()
EVT_END()

EVT_BEGIN(unitXNautPhD_attack_common_event3)
    IF_EQUAL(LW(9), PTR(&unitXNautPhD_weaponDodgySelf))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 2, PTR("B_1D"))
    END_IF()
    IF_EQUAL(LW(9), PTR(&unitXNautPhD_weaponRegenAlly))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 2, PTR("B_1C"))
    END_IF()
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 10)
    ADD(LW(1), 20)
    USER_FUNC(btlevtcmd_SetPartsPos, -2, 2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD2_ATTACK3"), EVT_NULLPTR, 0, EVT_NULLPTR)
    IF_EQUAL(LW(9), PTR(&unitXNautPhD_weaponDodgySelf))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_5C"))
    END_IF()
    IF_EQUAL(LW(9), PTR(&unitXNautPhD_weaponRegenAlly))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_4C"))
    END_IF()
    WAIT_FRM(24)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD2_ATTACK4"), EVT_NULLPTR, 0, EVT_NULLPTR)
    BROTHER_EVT()
        WAIT_FRM(24)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_1"))
    END_BROTHER()
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 2, 0x100'0000)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 2, 0x200'0000)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPartsSetting, -2, 2, 0, FLOAT(1.0), FLOAT(0.5))
    USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 2, LW(0), 0, LW(2), 0, -1)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    DO(12)
        USER_FUNC(evt_eff64, PTR(""), PTR("glass_n64"), 0, LW(0), 0, LW(2), FLOAT(1.0), 30, 0, 0, 0, 0, 0, 0)
    WHILE()
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
LBL(90)
            GOTO(98)
LBL(91)
            USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
            USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
            GOTO(98)
LBL(98)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD2_CRASH1"), EVT_NULLPTR, 0, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 2, PTR("B_2"))
            WAIT_FRM(8)
            USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 2, 0x100'0000)
            USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 2, 0x200'0000)
            WAIT_FRM(10)
            USER_FUNC(evt_btl_camera_set_mode, 0, 0)
            WAIT_FRM(30)
LBL(99)
            USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
            RETURN()
EVT_END()

EVT_BEGIN(unitXNautPhD_one_recover_event)
    SET(LW(9), PTR(&unitXNautPhD_weaponRecoverOne))
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
    RUN_CHILD_EVT(PTR(&unitXNautPhD_attack_common_event1))
    BROTHER_EVT()
        WAIT_FRM(10)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD2_SHAKE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        WAIT_FRM(30)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD2_SHAKE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        WAIT_FRM(30)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD2_SHAKE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    END_BROTHER()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_4B"))
    WAIT_FRM(80)
    USER_FUNC(btlevtcmd_RecoverHp, LW(3), LW(4), 10)
    USER_FUNC(btlevtcmd_GetPartsPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHeight, LW(3), LW(5))
    USER_FUNC(btlevtcmd_GetStatusMg, LW(3), LW(6))
    MULF(LW(5), LW(6))
    ADD(LW(1), LW(5))
    USER_FUNC(evt_eff, PTR(""), PTR("recovery"), 0, LW(0), LW(1), LW(2), 10, 0, 0, 0, 0, 0, 0, 0)
    WAIT_FRM(30)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    WAIT_FRM(60)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitXNautPhD_all_recover_event)
    SET(LW(9), PTR(&unitXNautPhD_weaponRecoverAll))
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
    RUN_CHILD_EVT(PTR(&unitXNautPhD_attack_common_event1))
    BROTHER_EVT()
        WAIT_FRM(10)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD2_SHAKE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        WAIT_FRM(30)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD2_SHAKE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        WAIT_FRM(30)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD2_SHAKE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    END_BROTHER()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_4B"))
    WAIT_FRM(80)
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
LBL(0)
    BROTHER_EVT()
        USER_FUNC(evt_sub_random, 10, LW(10))
        ADD(LW(10), 5)
        WAIT_FRM(LW(10))
        USER_FUNC(btlevtcmd_RecoverHp, LW(3), LW(4), 5)
        USER_FUNC(btlevtcmd_GetPartsPos, LW(3), LW(4), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetHeight, LW(3), LW(5))
        USER_FUNC(btlevtcmd_GetStatusMg, LW(3), LW(6))
        MULF(LW(5), LW(6))
        ADD(LW(1), LW(5))
        USER_FUNC(evt_eff, PTR(""), PTR("recovery"), 0, LW(0), LW(1), LW(2), 5, 0, 0, 0, 0, 0, 0, 0)
    END_BROTHER()
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    IF_NOT_EQUAL(LW(3), -1)
        GOTO(0)
    END_IF()
    WAIT_FRM(90)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    WAIT_FRM(60)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitXNautPhD_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    RETURN()
EVT_END()

EVT_BEGIN(unitXNautPhD_burn_potion_event)
    SET(LW(9), PTR(&unitXNautPhD_weaponFire))
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
    RUN_CHILD_EVT(PTR(&unitXNautPhD_attack_common_event1))
    RUN_CHILD_EVT(PTR(&unitXNautPhD_attack_common_event2))
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitXNautPhD_soft_potion_event)
    SET(LW(9), PTR(&unitXNautPhD_weaponSoft))
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
    RUN_CHILD_EVT(PTR(&unitXNautPhD_attack_common_event1))
    RUN_CHILD_EVT(PTR(&unitXNautPhD_attack_common_event2))
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitXNautPhD_tiny_potion_event)
    SET(LW(9), PTR(&unitXNautPhD_weaponTiny))
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
    RUN_CHILD_EVT(PTR(&unitXNautPhD_attack_common_event1))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 2, PTR("B_1E"))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 10)
    ADD(LW(1), 20)
    USER_FUNC(btlevtcmd_SetPartsPos, -2, 2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD2_ATTACK3"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_3C"))
    WAIT_FRM(24)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD2_ATTACK4"), EVT_NULLPTR, 0, EVT_NULLPTR)
    BROTHER_EVT()
        WAIT_FRM(24)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_1"))
    END_BROTHER()
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 2, 0x100'0000)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 2, 0x200'0000)
    USER_FUNC(btlevtcmd_GetEnemyBelong, LW(3), LW(10))
    IF_EQUAL(LW(10), 0)
        SET(LW(10), 125)
    ELSE()
        SET(LW(10), -125)
    END_IF()
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPartsSetting, -2, 2, 0, FLOAT(6.0), FLOAT(0.3))
    USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 2, LW(10), 0, 0, 0, -1)
    USER_FUNC(evt_eff, PTR(""), PTR("kemuri_test"), 20, LW(10), 50, 0, FLOAT(2.4), 0, 0, 0, 0, 0, 0, 0)
    DO(12)
        USER_FUNC(evt_eff64, PTR(""), PTR("glass_n64"), 0, LW(10), 0, 0, FLOAT(1.0), 30, 0, 0, 0, 0, 0, 0)
    WHILE()
    INLINE_EVT()
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD2_CRASH1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 2, PTR("B_2"))
        WAIT_FRM(8)
        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 2, 0x100'0000)
        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 2, 0x200'0000)
    END_INLINE()
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
LBL(90)
            GOTO(97)
LBL(91)
            IF_EQUAL(LW(10), 0)
                USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
            END_IF()
            USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
            GOTO(97)
LBL(97)
            USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
            IF_NOT_EQUAL(LW(3), -1)
                ADD(LW(10), 1)
                GOTO(0)
            END_IF()
LBL(98)
            USER_FUNC(evt_btl_camera_set_mode, 0, 0)
            WAIT_FRM(60)
LBL(99)
            USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
            RETURN()
EVT_END()

EVT_BEGIN(unitXNautPhD_dodgy_potion_event_self)
    SET(LW(3), -2)
    SET(LW(4), 1)
    SET(LW(9), PTR(&unitXNautPhD_weaponDodgySelf))
    RUN_CHILD_EVT(PTR(&unitXNautPhD_attack_common_event1))
    BROTHER_EVT()
        WAIT_FRM(10)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD2_BOTTLE3"), EVT_NULLPTR, 0, EVT_NULLPTR)
        WAIT_FRM(30)
        DO(3)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD2_BOTTLE4"), EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_FRM(20)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_5B"))
    WAIT_FRM(120)
    INLINE_EVT()
        WAIT_FRM(5)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD2_BOTTLE5"), EVT_NULLPTR, 0, EVT_NULLPTR)
        WAIT_FRM(55)
        USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    END_INLINE()
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    RETURN()
EVT_END()

EVT_BEGIN(unitXNautPhD_dodgy_potion_event_Crump)
    SET(LW(9), PTR(&unitXNautPhD_weaponDodgySelf))
    USER_FUNC(unitXNautPhD_GetCrumpId, LW(3), LW(4))
    RUN_CHILD_EVT(PTR(&unitXNautPhD_attack_common_event1))
    RUN_CHILD_EVT(PTR(&unitXNautPhD_attack_common_event3))
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitXNautPhD_regen_potion_event)
    SET(LW(9), PTR(&unitXNautPhD_weaponRegenAlly))
    USER_FUNC(unitXNautPhD_GetCrumpId, LW(3), LW(4))
    RUN_CHILD_EVT(PTR(&unitXNautPhD_attack_common_event1))
    RUN_CHILD_EVT(PTR(&unitXNautPhD_attack_common_event3))
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitXNautPhD_attack_event)
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(PTR(&unitXNautPhD_burn_potion_event))
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_EnemyItemUseCheck, -2, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(LW(0))
        USER_FUNC(btlevtcmd_StartWaitEvent, -2)
        RETURN()
    END_IF()
    USER_FUNC(unitXNautPhD_GetCrumpId, LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(10)
    ELSE()
        GOTO(20)
    END_IF()

LBL(10)
    SET(LW(1), 0)

    USER_FUNC(btlevtcmd_CheckStatus, -2, StatusEffectType::DODGY, LW(1))
    IF_EQUAL(LW(1), 0)
        SET(LW(1), 10)
    END_IF()

    USER_FUNC(btlevtcmd_DrawLots, LW(0), 6, 20, 10, LW(1), 20, 10, 5)
    SWITCH(LW(0))
        CASE_EQUAL(1)
            RUN_CHILD_EVT(PTR(&unitXNautPhD_soft_potion_event))
        CASE_EQUAL(2)
            RUN_CHILD_EVT(PTR(&unitXNautPhD_dodgy_potion_event_self))
        CASE_EQUAL(3)
            RUN_CHILD_EVT(PTR(&unitXNautPhD_tiny_potion_event))
        CASE_EQUAL(4)
            RUN_CHILD_EVT(PTR(&unitXNautPhD_one_recover_event))
        CASE_EQUAL(5)
            RUN_CHILD_EVT(PTR(&unitXNautPhD_all_recover_event))
        CASE_ETC()
            RUN_CHILD_EVT(PTR(&unitXNautPhD_burn_potion_event))
    END_SWITCH()
    GOTO(99)
    
LBL(20)

    USER_FUNC(btlevtcmd_CheckStatus, LW(3), StatusEffectType::DODGY, LW(1))
    IF_EQUAL(LW(1), 0)
        SET(LW(1), 20)
    END_IF()
    USER_FUNC(btlevtcmd_CheckStatus, LW(3), StatusEffectType::HP_REGEN, LW(2))
    IF_EQUAL(LW(2), 0)
        SET(LW(2), 20)
    END_IF()

    USER_FUNC(btlevtcmd_DrawLots, LW(0), 3, 60, LW(1), LW(2))
    SWITCH(LW(0))
        CASE_EQUAL(1)
            RUN_CHILD_EVT(PTR(&unitXNautPhD_dodgy_potion_event_Crump))
        CASE_EQUAL(2)
            RUN_CHILD_EVT(PTR(&unitXNautPhD_regen_potion_event))
        CASE_ETC()
            RUN_CHILD_EVT(PTR(&unitXNautPhD_burn_potion_event))
    END_SWITCH()
    GOTO(99)

LBL(99)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitXNautPhD_wait_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitXNautPhD_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitXNautPhD_wait_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitXNautPhD_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitXNautPhD_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitXNautPhD_attack_event))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

}  // namespace mod::tot::custom