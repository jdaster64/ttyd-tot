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
constexpr const int32_t UW_Lakitu_IsHolding = 0;
constexpr const int32_t UW_Lakitu_SpawnEgg = 1;
constexpr const int32_t UW_Lakitu_IsCharged = 2;
constexpr const int32_t UW_Lakitu_BattleUnitType = 4;

constexpr const int32_t UW_Spiny_IsGuarding = 0;
constexpr const int32_t UW_Spiny_Rotate = 1;
constexpr const int32_t UW_Spiny_RotateSpd = 2;
constexpr const int32_t UW_Spiny_IsCharged = 3;
constexpr const int32_t UW_Spiny_BattleUnitType = 4;

}  // namespace

// Evt / Function declarations.

extern const int32_t unitLakitu_init_event[];
extern const int32_t unitDarkLakitu_init_event[];
extern const int32_t unitLakitu_common_init_event[];
extern const int32_t unitLakitu_unison_phase_event[];
extern const int32_t unitLakitu_attack_event[];
extern const int32_t unitLakitu_damage_event[];
extern const int32_t unitLakitu_wait_event[];
extern const int32_t unitLakitu_normal_attack_event[];
extern const int32_t unitLakitu_charge_event[];
extern const int32_t unitLakitu_spawn_event[];
extern const int32_t unitLakitu_spiky_counter_event[];
extern const int32_t unitLakitu_first_attack_pos_event[];

extern const int32_t unitSpiny_init_event[];
extern const int32_t unitSkyBlueSpiny_init_event[];
extern const int32_t unitSpiny_common_init_event[];
extern const int32_t unitSpiny_attack_event[];
extern const int32_t unitSpiny_damage_event[];
extern const int32_t unitSpiny_wait_event[];
extern const int32_t unitSpiny_normal_attack_event[];
extern const int32_t unitSpiny_charge_event[];
extern const int32_t unitSpiny_guard_event[];
extern const int32_t unitSpiny_guard_cancel_event[];
extern const int32_t unitSpiny_flip_event[];
extern const int32_t unitSpiny_wakeup_event[];
extern const int32_t unitSpiny_spiky_counter_event[];

EVT_DECLARE_USER_FUNC(unitLakitu_copy_status, 2)

// Unit data.

int8_t unitLakitu_defense[] = { 0, 0, 0, 0, 0 };
int8_t unitLakitu_defense_attr[] = { 0, 0, 0, 0, 0 };
int8_t unitSpiny_defense[] = { 2, 2, 2, 2, 2 };
int8_t unitSpiny_defense_attr[] = { 0, 0, 0, 0, 0 };
int8_t unitSpiny_flip_defense[] = { 0, 0, 0, 0, 0 };
int8_t unitSpiny_flip_defense_attr[] = { 0, 0, 0, 0, 0 };
int8_t unitSpiny_guard_defense[] = { 99, 99, 99, 99, 99 };
int8_t unitSpiny_guard_defense_attr[] = { 2, 2, 2, 2, 2 };

StatusVulnerability unitLakitu_status = {
     80,  90, 105, 100,  80, 100, 100, 100,
    100,  90, 100,  90, 100,  95,  80,  90,
    100, 100,  80, 100, 100,  95,
};
StatusVulnerability unitDarkLakitu_status = {
     60,  70,  95, 100,  60, 100, 100,  80,
    100,  85, 100,  85, 100,  90,  60,  70,
     80, 100,  60, 100, 100,  80,
};
StatusVulnerability unitSpiny_status = {
     80,  90,  80, 100,  80, 100, 100,  70,
    100,  90, 100,  90, 100,  95,  90, 100,
     30, 100,  90, 100, 100, 100,
};
StatusVulnerability unitSkyBlueSpiny_status = {
     60,  70,  60, 100,  60, 100, 100,  50,
    100,  85, 100,  85, 100,  90,  70,  80,
     10, 100,  70, 100, 100,  80,
};
StatusVulnerability unitSpiny_flip_status = {
    100, 100, 100, 100, 100, 100, 100, 100,
    100, 100, 100, 100, 100, 100, 100, 100,
    100, 100, 100, 100, 100, 100,
};
StatusVulnerability unitSpiny_guard_status = {
    100, 100, 100, 100, 100, 100, 100, 100,
    100, 100, 100, 100, 100, 100, 100, 100,
    100, 100, 100, 100, 100, 100,
};

PoseTableEntry unitLakitu_pose_table[] = {
    1, "JGM_N_1",
    2, "JGM_Y_1",
    9, "JGM_Y_1",
    5, "JGM_K_1",
    4, "JGM_X_1",
    3, "JGM_X_1",
    28, "JGM_S_1",
    29, "JGM_Q_1",
    30, "JGM_Q_1",
    31, "JGM_S_1",
    39, "JGM_D_1",
    42, "JGM_R_1",
    40, "JGM_W_1",
    56, "JGM_I_1",
    57, "JGM_I_1",
    65, "JGM_T_1",
    69, "JGM_S_1",
};
PoseTableEntry unitLakitu_hold_pose_table[] = {
    1, "JGM_S_2",
    2, "JGM_S_2",
    9, "JGM_S_2",
    5, "JGM_S_2",
    4, "JGM_S_2",
    3, "JGM_S_2",
    28, "JGM_S_2",
    29, "JGM_Q_1",
    30, "JGM_Q_1",
    31, "JGM_S_2",
    39, "JGM_D_1",
    42, "JGM_S_2",
    40, "JGM_S_2",
    56, "JGM_S_2",
    57, "JGM_S_2",
    65, "JGM_S_2",
    69, "JGM_S_2",
};
PoseTableEntry unitSpiny_pose_table[] = {
    1, "TGZ_N_1",
    2, "TGZ_Y_1",
    9, "TGZ_Y_1",
    5, "TGZ_K_1",
    4, "TGZ_X_1",
    3, "TGZ_X_1",
    28, "TGZ_S_1",
    29, "TGZ_Q_1",
    30, "TGZ_Q_1",
    31, "TGZ_A_1",
    39, "TGZ_D_1",
    40, "TGZ_W_1",
    42, "TGZ_R_1",
    65, "TGZ_T_1",
    69, "TGZ_S_1",
};
PoseTableEntry unitSpiny_flip_pose_table[] = {
    1, "TGZ_N_2",
    2, "TGZ_Y_2",
    9, "TGZ_Y_2",
    5, "TGZ_K_2",
    4, "TGZ_X_2",
    3, "TGZ_X_2",
    28, "TGZ_S_2",
    29, "TGZ_Q_2",
    30, "TGZ_Q_2",
    31, "TGZ_S_1",
    39, "TGZ_D_3",
    40, "TGZ_W_1",
    42, "TGZ_R_1",
    69, "TGZ_S_2",
};
PoseTableEntry unitSpiny_guard_pose_table[] = {
    1, "TGZ_P_1",
    2, "TGZ_P_1",
    9, "TGZ_P_1",
    5, "TGZ_P_1",
    4, "TGZ_P_1",
    3, "TGZ_P_1",
    28, "TGZ_P_1",
    29, "TGZ_P_1",
    30, "TGZ_P_1",
    31, "TGZ_P_1",
    39, "TGZ_P_1",
    69, "TGZ_P_1",
};

DataTableEntry unitLakitu_data_table[] = {
    37, (void*)unitLakitu_spiky_counter_event,
    48, (void*)unitLakitu_first_attack_pos_event,
    0, nullptr,
};
DataTableEntry unitSpiny_data_table[] = {
    13, (void*)unitSpiny_flip_event,
    37, (void*)unitSpiny_spiky_counter_event,
    39, (void*)unitSpiny_spiky_counter_event,
    48, (void*)btldefaultevt_Dummy,
    0, nullptr,
};
DataTableEntry unitSpiny_flip_data_table[] = {
    13, (void*)unitSpiny_flip_event,
    48, (void*)btldefaultevt_Dummy,
    0, nullptr,
};
DataTableEntry unitSpiny_guard_data_table[] = {
    37, (void*)unitSpiny_spiky_counter_event,
    39, (void*)unitSpiny_spiky_counter_event,
    48, (void*)btldefaultevt_Dummy,
    0, nullptr,
};

BattleWeapon unitLakitu_weaponAttack = {
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
BattleWeapon unitSpiny_weaponAttack = {
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
BattleWeapon unitLakituSpiny_weaponCharge = {
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

BattleUnitKindPart unitLakitu_parts[] = {
    {
        .index = 1,
        .name = "btl_un_jyugem",
        .model_name = "c_jugemu",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 35.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 50.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitLakitu_defense,
        .defense_attr = unitLakitu_defense_attr,
        .attribute_flags = 0x0000'0009 |
            PartsAttribute_Flags::HAMMERLIKE_CANNOT_TARGET |
            PartsAttribute_Flags::SHELLTOSSLIKE_CANNOT_TARGET,
        .counter_attribute_flags = 0,
        .pose_table = unitLakitu_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_jyugem",
        .model_name = "c_jugemu",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitLakitu_defense,
        .defense_attr = unitLakitu_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitLakitu_pose_table,
    },
};
BattleUnitKindPart unitDarkLakitu_parts[] = {
    {
        .index = 1,
        .name = "btl_un_hyper_jyugem",
        .model_name = "c_jugemu_b",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 35.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 50.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitLakitu_defense,
        .defense_attr = unitLakitu_defense_attr,
        .attribute_flags = 0x0000'0009 |
            PartsAttribute_Flags::HAMMERLIKE_CANNOT_TARGET |
            PartsAttribute_Flags::SHELLTOSSLIKE_CANNOT_TARGET,
        .counter_attribute_flags = 0,
        .pose_table = unitLakitu_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_hyper_jyugem",
        .model_name = "c_jugemu_b",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitLakitu_defense,
        .defense_attr = unitLakitu_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitLakitu_pose_table,
    },
};
BattleUnitKindPart unitSpiny_parts[] = {
    {
        .index = 1,
        .name = "btl_un_togezo",
        .model_name = "c_togezo",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitSpiny_defense,
        .defense_attr = unitSpiny_defense_attr,
        .attribute_flags = 0x0000'0009 | PartsAttribute_Flags::SHELLED,
        .counter_attribute_flags =
            PartsCounterAttribute_Flags::TOP_SPIKY |
            PartsCounterAttribute_Flags::FRONT_SPIKY,
        .pose_table = unitSpiny_pose_table,
    },
};
BattleUnitKindPart unitSkyBlueSpiny_parts[] = {
    {
        .index = 1,
        .name = "btl_un_hyper_togezo",
        .model_name = "c_togezo_b",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitSpiny_defense,
        .defense_attr = unitSpiny_defense_attr,
        .attribute_flags = 0x0000'0009 | PartsAttribute_Flags::SHELLED,
        .counter_attribute_flags =
            PartsCounterAttribute_Flags::TOP_SPIKY |
            PartsCounterAttribute_Flags::FRONT_SPIKY,
        .pose_table = unitSpiny_pose_table,
    },
};

BattleUnitKind unit_Lakitu = {
    .unit_type = BattleUnitType::LAKITU,
    .unit_name = "btl_un_jyugem",
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
    .width = 32,
    .height = 42,
    .hit_offset = { 4, 42 },
    .center_offset = { 0.0f, 21.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 16.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 16.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 10.0f, 38.0f, 0.0f },
    .cut_base_offset = { 0.0f, 21.0f, 0.0f },
    .cut_width = 32.0f,
    .cut_height = 42.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_JUGEMU_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::UNQUAKEABLE,
    .status_vulnerability = &unitLakitu_status,
    .num_parts = 2,
    .parts = unitLakitu_parts,
    .init_evt_code = (void*)unitLakitu_init_event,
    .data_table = unitLakitu_data_table,
};
BattleUnitKind unit_DarkLakitu = {
    .unit_type = BattleUnitType::DARK_LAKITU,
    .unit_name = "btl_un_hyper_jyugem",
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
    .width = 32,
    .height = 42,
    .hit_offset = { 4, 42 },
    .center_offset = { 0.0f, 21.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 16.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 16.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 10.0f, 38.0f, 0.0f },
    .cut_base_offset = { 0.0f, 21.0f, 0.0f },
    .cut_width = 32.0f,
    .cut_height = 42.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_JUGEMU_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::UNQUAKEABLE,
    .status_vulnerability = &unitDarkLakitu_status,
    .num_parts = 2,
    .parts = unitDarkLakitu_parts,
    .init_evt_code = (void*)unitDarkLakitu_init_event,
    .data_table = unitLakitu_data_table,
};
BattleUnitKind unit_Spiny = {
    .unit_type = BattleUnitType::SPINY,
    .unit_name = "btl_un_togezo",
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
    .width = 32,
    .height = 30,
    .hit_offset = { 4, 30 },
    .center_offset = { 0.0f, 15.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 16.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 16.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 12.0f, 16.0f, 0.0f },
    .cut_base_offset = { 0.0f, 15.0f, 0.0f },
    .cut_width = 32.0f,
    .cut_height = 30.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_TOGEZO_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitSpiny_status,
    .num_parts = 1,
    .parts = unitSpiny_parts,
    .init_evt_code = (void*)unitSpiny_init_event,
    .data_table = unitSpiny_data_table,
};
BattleUnitKind unit_SkyBlueSpiny = {
    .unit_type = BattleUnitType::SKY_BLUE_SPINY,
    .unit_name = "btl_un_hyper_togezo",
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
    .width = 32,
    .height = 30,
    .hit_offset = { 4, 30 },
    .center_offset = { 0.0f, 15.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 16.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 16.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 12.0f, 16.0f, 0.0f },
    .cut_base_offset = { 0.0f, 15.0f, 0.0f },
    .cut_width = 32.0f,
    .cut_height = 30.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_TOGEZO_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitSkyBlueSpiny_status,
    .num_parts = 1,
    .parts = unitSkyBlueSpiny_parts,
    .init_evt_code = (void*)unitSkyBlueSpiny_init_event,
    .data_table = unitSpiny_data_table,
};

const BattleUnitSetup unitSpiny_spawn_entry = {
    .unit_kind_params = &unit_Spiny,
    .alliance = 1,
    .attack_phase = 0x04000004,
    .position = { -1000.0f, -1000.0f, -1000.0f },
    .addl_target_offset_x = 0,
    .unit_work = { 0, 0, 0, 0 },
    .item_drop_table = nullptr,
};
const BattleUnitSetup unitSkyBlueSpiny_spawn_entry = {
    .unit_kind_params = &unit_SkyBlueSpiny,
    .alliance = 1,
    .attack_phase = 0x04000004,
    .position = { -1000.0f, -1000.0f, -1000.0f },
    .addl_target_offset_x = 0,
    .unit_work = { 0, 0, 0, 0 },
    .item_drop_table = nullptr,
};

// Function / USER_FUNC definitions.

EVT_DEFINE_USER_FUNC(unitLakitu_copy_status) {
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

EVT_BEGIN(unitLakitu_attack_event)
    // Changed: Check for Charge status before trying to spawn a Spiny.
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Lakitu_IsCharged, LW(0))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Lakitu_SpawnEgg, 0)
        RUN_CHILD_EVT(PTR(&unitLakitu_normal_attack_event))
        GOTO(99)
    END_IF()

    USER_FUNC(btlevtcmd_EnemyItemUseCheck, -2, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(LW(0))
        GOTO(99)
    END_IF()

    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Lakitu_BattleUnitType, LW(0))
    IF_EQUAL(LW(0), (int32_t)BattleUnitType::LAKITU)
        USER_FUNC(btlevtcmd_DrawLots, LW(0), 2, 20, 80)
    ELSE()
        USER_FUNC(btlevtcmd_get_turn, LW(0))
        IF_SMALL_EQUAL(LW(0), 1)
            USER_FUNC(btlevtcmd_DrawLots, LW(0), 2, 40, 90)
        ELSE()
            USER_FUNC(btlevtcmd_DrawLots, LW(0), 2, 40, 70)
        END_IF()
    END_IF()

    SWITCH(LW(0))
        CASE_EQUAL(0)
            // Check for whether there is space to spawn a Spiny.
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
            SET(LW(1), 0)
            DO(0)
                USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 40)
                SET(LW(4), LW(0))
                USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(5))
                MUL(LW(4), LW(5))
                IF_SMALL(LW(4), 0)
                    SUB(LW(2), 10)
                    USER_FUNC(btlevtcmd_CheckSpace, LW(3), LW(0), LW(1), LW(2), 40, 0, 0, 0)
                    IF_EQUAL(LW(3), -1)
                        RUN_CHILD_EVT(PTR(&unitLakitu_spawn_event))
                        GOTO(99)
                    END_IF()
                ELSE()
                    GOTO(9)
                END_IF()
            WHILE()
LBL(9)
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
            SET(LW(1), 0)
            DO(0)
                USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 40)
                IF_SMALL_EQUAL(LW(0), 150)
                    ADD(LW(2), 10)
                    USER_FUNC(btlevtcmd_CheckSpace, LW(3), LW(0), LW(1), LW(2), 40, 0, 0, 0)
                    IF_EQUAL(LW(3), -1)
                        RUN_CHILD_EVT(PTR(&unitLakitu_spawn_event))
                        GOTO(99)
                    END_IF()
                ELSE()
                    GOTO(10)
                END_IF()
            WHILE()
    END_SWITCH()

LBL(10)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Lakitu_BattleUnitType, LW(0))
    IF_EQUAL(LW(0), (int32_t)BattleUnitType::LAKITU)
        USER_FUNC(btlevtcmd_DrawLots, LW(0), 3, 65, 15, 0)
    ELSE()
        USER_FUNC(btlevtcmd_get_turn, LW(0))
        IF_SMALL_EQUAL(LW(0), 1)
            USER_FUNC(btlevtcmd_DrawLots, LW(0), 3, 30, 30, 30)
        ELSE()
            USER_FUNC(btlevtcmd_DrawLots, LW(0), 3, 30, 30, 10)
        END_IF()
    END_IF()
    SWITCH(LW(0))
        CASE_EQUAL(0)
            USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Lakitu_SpawnEgg, 0)
            RUN_CHILD_EVT(PTR(&unitLakitu_normal_attack_event))
        CASE_EQUAL(1)
            USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Lakitu_SpawnEgg, 1)
            RUN_CHILD_EVT(PTR(&unitLakitu_normal_attack_event))
        CASE_EQUAL(2)
            USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Lakitu_IsCharged, 1)
            SET(LW(9), PTR(&unitLakituSpiny_weaponCharge))
            RUN_CHILD_EVT(PTR(&unitLakitu_charge_event))
    END_SWITCH()

LBL(99)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitLakitu_spiky_counter_event)
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_CheckStatus, -2, 10, LW(3))
    IF_NOT_EQUAL(LW(3), 0)
        ADD(LW(1), 45)
        GOTO(90)
    END_IF()
    USER_FUNC(btlevtcmd_CheckStatus, -2, 11, LW(3))
    IF_NOT_EQUAL(LW(3), 0)
        ADD(LW(1), 25)
        GOTO(90)
    END_IF()
    ADD(LW(1), 35)
    GOTO(90)
LBL(90)
    USER_FUNC(evt_eff, PTR(""), PTR("toge_flush"), 0, LW(0), LW(1), LW(2), 60, 0, 0, 0, 0, 0, 0, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitLakitu_charge_event)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, -1)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 300)
    WAIT_FRM(40)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    ADD(LW(1), 15)
    ADD(LW(2), 10)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_CHARGE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(evt_eff64, PTR(""), PTR("crystal_n64"), 7, LW(0), LW(1), LW(2), FLOAT(1.5), 60, 0, 0, 0, 0, 0, 0)
    WAIT_MSEC(1000)
    INLINE_EVT()
        USER_FUNC(evt_btl_camera_set_mode, 0, 3)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_JumpSetting, -2, 20, 0, FLOAT(1.0))
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, -1)
        WAIT_MSEC(500)
        USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    END_INLINE()
    USER_FUNC(btlevtcmd_CheckDamage, -2, -2, 1, LW(9), 256, LW(5))
    RETURN()
EVT_END()

EVT_BEGIN(unitLakitu_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Lakitu_IsHolding, LW(0))
    IF_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    ELSE()
        USER_FUNC(btlevtcmd_GetDamage, LW(10), LW(0))
        IF_LARGE_EQUAL(LW(0), 1)
            SET(LW(15), 1)
        ELSE()
            SET(LW(15), 0)
        END_IF()
        USER_FUNC(btlevtcmd_CheckStatus, LW(10), 27, LW(14))
        OR(LW(15), LW(14))
        USER_FUNC(btlevtcmd_CheckStatus, LW(10), 1, LW(14))
        OR(LW(15), LW(14))
        USER_FUNC(btlevtcmd_CheckStatus, LW(10), 3, LW(14))
        OR(LW(15), LW(14))
        USER_FUNC(btlevtcmd_CheckStatus, LW(10), 5, LW(14))
        OR(LW(15), LW(14))
        USER_FUNC(btlevtcmd_CheckStatus, LW(10), 6, LW(14))
        OR(LW(15), LW(14))
        USER_FUNC(btlevtcmd_CheckStatus, LW(10), 9, LW(14))
        OR(LW(15), LW(14))
        USER_FUNC(btlevtcmd_CheckStatus, LW(10), 2, LW(14))
        OR(LW(15), LW(14))
        IF_NOT_EQUAL(LW(15), 0)
            INLINE_EVT()
                USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 2, 0x300'0000)
                USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
                ADD(LW(1), 35)
                USER_FUNC(btlevtcmd_SetPartsPos, -2, 2, LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_GetPos, LW(10), LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_FaceDirectionSub, LW(10), LW(0), 200)
                USER_FUNC(btlevtcmd_JumpPartsSetting, LW(10), 2, 0, FLOAT(5.0), FLOAT(0.3))
                USER_FUNC(btlevtcmd_JumpPartsPosition, LW(10), 2, LW(0), 0, LW(2), 0, -1)
                USER_FUNC(btlevtcmd_OnPartsAttribute, LW(10), 2, 0x100'0000)
                USER_FUNC(btlevtcmd_OnPartsAttribute, LW(10), 2, 0x200'0000)
            END_INLINE()
            USER_FUNC(btlevtcmd_SetUnitWork, LW(10), UW_Lakitu_IsHolding, 0)
            USER_FUNC(btlevtcmd_OffPartsCounterAttribute, LW(10), 1, 1)
            USER_FUNC(btlevtcmd_AnimeSetPoseTable, LW(10), 1, PTR(&unitLakitu_pose_table))
            USER_FUNC(btlevtcmd_SetHitOffset, -2, 1, 0, 0, 0)
            RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
        ELSE()
            USER_FUNC(btlevtcmd_GetDamageCode, LW(10), LW(0))
            IF_EQUAL(LW(0), 22)
                GOTO(90)
            END_IF()
            IF_EQUAL(LW(0), 30)
                GOTO(90)
            END_IF()
            IF_EQUAL(LW(0), 31)
                GOTO(90)
            END_IF()
            IF_EQUAL(LW(0), 38)
                GOTO(90)
            END_IF()
            IF_EQUAL(LW(0), 39)
                GOTO(90)
            END_IF()
            IF_EQUAL(LW(0), 41)
                GOTO(90)
            END_IF()
            USER_FUNC(btlevtcmd_CheckStatus, LW(10), 4, LW(15))
            USER_FUNC(btlevtcmd_CheckStatus, LW(10), 8, LW(14))
            OR(LW(15), LW(14))
            USER_FUNC(btlevtcmd_CheckStatus, LW(10), 11, LW(14))
            OR(LW(15), LW(14))
            USER_FUNC(btlevtcmd_CheckStatus, LW(10), 15, LW(14))
            OR(LW(15), LW(14))
            USER_FUNC(btlevtcmd_CheckStatus, LW(10), 20, LW(14))
            OR(LW(15), LW(14))
            USER_FUNC(btlevtcmd_CheckStatus, LW(10), 0, LW(14))
            OR(LW(15), LW(14))
            IF_NOT_EQUAL(LW(15), 0)
                INLINE_EVT()
                    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 2, 0x300'0000)
                    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
                    ADD(LW(1), 35)
                    USER_FUNC(btlevtcmd_SetPartsPos, -2, 2, LW(0), LW(1), LW(2))
                    USER_FUNC(btlevtcmd_JumpPartsSetting, LW(10), 2, 30, 0, FLOAT(0.1))
                    USER_FUNC(btlevtcmd_JumpPartsPosition, LW(10), 2, LW(0), LW(1), LW(2), 30, -1)
                    USER_FUNC(btlevtcmd_OnPartsAttribute, LW(10), 2, 0x100'0000)
                    USER_FUNC(btlevtcmd_OnPartsAttribute, LW(10), 2, 0x200'0000)
                END_INLINE()
            END_IF()
LBL(90)
            RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
        END_IF()
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitLakitu_first_attack_pos_event)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    SET(LW(1), 10)
    USER_FUNC(btlevtcmd_SetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_WaitAttackEnd)
    RETURN()
EVT_END()

EVT_BEGIN(unitLakitu_spawn_event)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(0), LW(2), 1, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Lakitu_IsHolding, LW(10))
    IF_EQUAL(LW(10), 0)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_JUGEM_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("JGM_A_1"))
        WAIT_FRM(40)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_JUGEM_ATTACK2"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("JGM_A_2"))
        WAIT_FRM(30)
    ELSE()
        USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Lakitu_IsHolding, 0)
        USER_FUNC(btlevtcmd_OffPartsCounterAttribute, -2, 1, 1)
        USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitLakitu_pose_table))
        USER_FUNC(btlevtcmd_SetHitOffset, -2, 1, 0, 0, 0)
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("JGM_A_3"))
    WAIT_FRM(2)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_JUGEM_THROW2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 2, 0x300'0000)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(10), LW(11), LW(12))
    ADD(LW(11), 35)
    USER_FUNC(btlevtcmd_SetPartsPos, -2, 2, LW(10), LW(11), LW(12))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 20)
    USER_FUNC(btlevtcmd_JumpPartsSetting, -2, 2, 45, 0, FLOAT(0.3))
    USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 2, LW(0), LW(1), LW(2), 45, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_JUGEM_JUMP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 15)
    USER_FUNC(btlevtcmd_JumpPartsSetting, -2, 2, 15, 0, FLOAT(0.3))
    USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 2, LW(0), LW(1), LW(2), 15, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_JUGEM_JUMP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    WAIT_FRM(20)

    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Lakitu_BattleUnitType, LW(4))
    IF_EQUAL(LW(4), (int32_t)BattleUnitType::LAKITU)
        USER_FUNC(btlevtcmd_SpawnUnit, LW(3), PTR(&unitSpiny_spawn_entry), 0)
    ELSE()
        USER_FUNC(btlevtcmd_SpawnUnit, LW(3), PTR(&unitSkyBlueSpiny_spawn_entry), 0)
    END_IF()

    USER_FUNC(unitLakitu_copy_status, -2, LW(3))
    USER_FUNC(btlevtcmd_GetBodyId, LW(3), LW(4))
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 2, 0x300'0000)
    USER_FUNC(btlevtcmd_SetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetHomePos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_AnimeChangePose, LW(3), LW(4), PTR("TGZ_P_1"))
    USER_FUNC(btlevtcmd_snd_se, LW(3), PTR("SFX_ENM_TOGEZO_TURN2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetRotate, LW(3), 0, 0, 0)
    USER_FUNC(btlevtcmd_SetRotateOffset, LW(3), 0, 15, 0)
    DO(5)
        USER_FUNC(btlevtcmd_AddRotate, LW(3), 0, 0, -36)
        WAIT_FRM(1)
    WHILE()
    USER_FUNC(btlevtcmd_AnimeChangePose, LW(3), LW(4), PTR("TGZ_S_1"))
    DO(5)
        USER_FUNC(btlevtcmd_AddRotate, LW(3), 0, 0, -36)
        WAIT_FRM(1)
    WHILE()
    USER_FUNC(btlevtcmd_SetRotateOffset, LW(3), 0, 0, 0)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    WAIT_FRM(10)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitLakitu_common_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitLakitu_wait_event))
    USER_FUNC(btlevtcmd_SetEventUnisonPhase, -2, PTR(&unitLakitu_unison_phase_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitLakitu_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitLakitu_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitLakitu_attack_event))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Lakitu_IsHolding, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Lakitu_IsCharged, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 2, PTR("JGM_P_1"))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitLakitu_normal_attack_event)
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), PTR(&unitLakitu_weaponAttack))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, PTR(&unitLakitu_weaponAttack), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            RETURN()
        END_IF()
        GOTO(99)
    END_IF()
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_WeaponAftereffect, PTR(&unitLakitu_weaponAttack))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, PTR(&unitLakitu_weaponAttack))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Lakitu_IsHolding, LW(10))
    IF_EQUAL(LW(10), 0)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_JUGEM_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("JGM_A_1"))
        WAIT_FRM(40)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_JUGEM_ATTACK2"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("JGM_A_2"))
        WAIT_FRM(30)
    ELSE()
        USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Lakitu_IsHolding, 0)
        USER_FUNC(btlevtcmd_OffPartsCounterAttribute, -2, 1, 1)
        USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitLakitu_pose_table))
        USER_FUNC(btlevtcmd_SetHitOffset, -2, 1, 0, 0, 0)
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("JGM_A_3"))
    WAIT_FRM(2)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 2, 0x300'0000)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    ADD(LW(1), 35)
    USER_FUNC(btlevtcmd_SetPartsPos, -2, 2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_JUGEM_THROW1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPartsSetting, -2, 2, 45, 0, FLOAT(0.3))
    USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 2, LW(0), LW(1), LW(2), 45, -1)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), PTR(&unitLakitu_weaponAttack), 256, LW(5))
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
    USER_FUNC(btlevtcmd_JumpPartsContinue, -2, 2)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_JUGEM_JUMP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    GOTO(98)
LBL(91)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), PTR(&unitLakitu_weaponAttack))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&unitLakitu_weaponAttack), 256, LW(5))
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Lakitu_SpawnEgg, LW(0))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(btlevtcmd_CheckDamageCode, LW(3), 0x4'0000, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
            SET(LW(1), 0)
            DO(0)
                USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 40)
                SET(LW(4), LW(0))
                USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(5))
                MUL(LW(4), LW(5))
                IF_SMALL(LW(4), 0)
                    SUB(LW(2), 10)
                    USER_FUNC(btlevtcmd_CheckSpace, LW(3), LW(0), LW(1), LW(2), 40, 0, 0, 0)
                    IF_EQUAL(LW(3), -1)
                        GOTO(97)
                    END_IF()
                ELSE()
                    GOTO(96)
                END_IF()
            WHILE()
LBL(96)
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
            SET(LW(1), 0)
            DO(0)
                USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 40)
                IF_SMALL_EQUAL(LW(0), 150)
                    ADD(LW(2), 10)
                    USER_FUNC(btlevtcmd_CheckSpace, LW(3), LW(0), LW(1), LW(2), 40, 0, 0, 0)
                    IF_EQUAL(LW(3), -1)
                        GOTO(97)
                    END_IF()
                ELSE()
                    GOTO(98)
                END_IF()
            WHILE()
        END_IF()
    END_IF()
    GOTO(98)
LBL(97)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_JUGEM_REBOUND1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 20)
    USER_FUNC(btlevtcmd_JumpPartsSetting, -2, 2, 45, 0, FLOAT(0.3))
    USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 2, LW(0), LW(1), LW(2), 45, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_JUGEM_JUMP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 20)
    USER_FUNC(btlevtcmd_JumpPartsSetting, -2, 2, 15, 0, FLOAT(0.3))
    USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 2, LW(0), LW(1), LW(2), 15, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_JUGEM_JUMP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    WAIT_FRM(20)

    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Lakitu_BattleUnitType, LW(4))
    IF_EQUAL(LW(4), (int32_t)BattleUnitType::LAKITU)
        USER_FUNC(btlevtcmd_SpawnUnit, LW(3), PTR(&unitSpiny_spawn_entry), 0)
    ELSE()
        USER_FUNC(btlevtcmd_SpawnUnit, LW(3), PTR(&unitSkyBlueSpiny_spawn_entry), 0)
    END_IF()

    USER_FUNC(unitLakitu_copy_status, -2, LW(3))
    USER_FUNC(btlevtcmd_GetBodyId, LW(3), LW(4))
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 2, 0x300'0000)
    USER_FUNC(btlevtcmd_SetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetHomePos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_AnimeChangePose, LW(3), LW(4), PTR("TGZ_P_1"))
    USER_FUNC(btlevtcmd_snd_se, LW(3), PTR("SFX_ENM_TOGEZO_TURN2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetRotate, LW(3), 0, 0, 0)
    USER_FUNC(btlevtcmd_SetRotateOffset, LW(3), 0, 15, 0)
    DO(5)
        USER_FUNC(btlevtcmd_AddRotate, LW(3), 0, 0, -36)
        WAIT_FRM(1)
    WHILE()
    USER_FUNC(btlevtcmd_AnimeChangePose, LW(3), LW(4), PTR("TGZ_S_1"))
    DO(5)
        USER_FUNC(btlevtcmd_AddRotate, LW(3), 0, 0, -36)
        WAIT_FRM(1)
    WHILE()
    USER_FUNC(btlevtcmd_SetRotateOffset, LW(3), 0, 0, 0)
    GOTO(99)
LBL(98)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_GetPartsPos, -2, 2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 150)
    USER_FUNC(btlevtcmd_JumpPartsSetting, -2, 2, 0, FLOAT(5.0), FLOAT(0.3))
    USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 2, LW(0), 0, LW(2), 0, -1)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 2, 0x300'0000)
    WAIT_FRM(10)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitLakitu_unison_phase_event)
    USER_FUNC(btlevtcmd_CheckPhase, LW(0), 0x400'0001)
    IF_EQUAL(LW(0), 0)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_get_turn, LW(0))
    IF_EQUAL(LW(0), 1)
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
    SET(LW(15), 0)
    USER_FUNC(btlevtcmd_CheckStatus, -2, 3, LW(14))
    OR(LW(15), LW(14))
    USER_FUNC(btlevtcmd_CheckStatus, -2, 5, LW(14))
    OR(LW(15), LW(14))
    USER_FUNC(btlevtcmd_CheckStatus, -2, 6, LW(14))
    OR(LW(15), LW(14))
    IF_NOT_EQUAL(LW(15), 0)
        GOTO(99)
    END_IF()
    USER_FUNC(evt_sub_random, 99, LW(0))
    IF_SMALL(LW(0), 50)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_JUGEM_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("JGM_A_1"))
        WAIT_FRM(40)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_JUGEM_HOLD1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("JGM_A_2"))
        WAIT_FRM(30)
        USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Lakitu_IsHolding, 1)
        USER_FUNC(btlevtcmd_OnPartsCounterAttribute, -2, 1, 1)
        USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitLakitu_hold_pose_table))
        USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
        USER_FUNC(btlevtcmd_SetHitOffset, -2, 1, 0, 20, 0)
    END_IF()
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitLakitu_wait_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitLakitu_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Lakitu_BattleUnitType, (int32_t)BattleUnitType::LAKITU)
    RUN_CHILD_EVT(unitLakitu_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitDarkLakitu_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Lakitu_BattleUnitType, (int32_t)BattleUnitType::DARK_LAKITU)
    RUN_CHILD_EVT(unitLakitu_common_init_event)
    RETURN()
EVT_END()



EVT_BEGIN(unitSpiny_attack_event)
    USER_FUNC(btlevtcmd_GetOverTurnCount, -2, LW(0))
    IF_LARGE_EQUAL(LW(0), 1)
        RUN_CHILD_EVT(PTR(&unitSpiny_wakeup_event))
        RETURN()
    END_IF()

    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Spiny_IsGuarding, LW(0))
    IF_EQUAL(LW(0), 1)
        RUN_CHILD_EVT(PTR(&unitSpiny_guard_cancel_event))
        GOTO(99)
    END_IF()

    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Spiny_IsCharged, LW(0))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Spiny_IsCharged, 0)
        RUN_CHILD_EVT(PTR(&unitSpiny_normal_attack_event))
        GOTO(99)
    END_IF()

    USER_FUNC(btlevtcmd_EnemyItemUseCheck, -2, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(LW(0))
        USER_FUNC(btlevtcmd_StartWaitEvent, -2)
        RETURN()
    END_IF()

    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Spiny_BattleUnitType, LW(0))
    IF_EQUAL(LW(0), (int32_t)BattleUnitType::SPINY)
        USER_FUNC(btlevtcmd_DrawLots, LW(0), 3, 40, 25, 0)
    ELSE()
        USER_FUNC(btlevtcmd_get_turn, LW(0))
        IF_SMALL_EQUAL(LW(0), 1)
            USER_FUNC(btlevtcmd_DrawLots, LW(0), 3, 40, 25, 20)
        ELSE()
            USER_FUNC(btlevtcmd_DrawLots, LW(0), 3, 40, 25, 6)
        END_IF()
    END_IF()

    SWITCH(LW(0))
        CASE_EQUAL(0)
            RUN_CHILD_EVT(PTR(&unitSpiny_normal_attack_event))
        CASE_EQUAL(1)
            RUN_CHILD_EVT(PTR(&unitSpiny_guard_event))
        CASE_EQUAL(2)
            USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Spiny_IsCharged, 1)
            SET(LW(9), PTR(&unitLakituSpiny_weaponCharge))
            RUN_CHILD_EVT(PTR(&unitSpiny_charge_event))
    END_SWITCH()
    
LBL(99)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitSpiny_charge_event)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, -1)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 300)
    WAIT_FRM(40)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    ADD(LW(1), 15)
    ADD(LW(2), 10)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_CHARGE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(evt_eff64, PTR(""), PTR("crystal_n64"), 7, LW(0), LW(1), LW(2), FLOAT(1.5), 60, 0, 0, 0, 0, 0, 0)
    WAIT_MSEC(1000)
    INLINE_EVT()
        USER_FUNC(evt_btl_camera_set_mode, 0, 3)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_JumpSetting, -2, 20, 0, FLOAT(1.0))
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, -1)
        WAIT_MSEC(500)
        USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    END_INLINE()
    USER_FUNC(btlevtcmd_CheckDamage, -2, -2, 1, LW(9), 256, LW(5))
    RETURN()
EVT_END()

EVT_BEGIN(unitSpiny_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    RETURN()
EVT_END()

EVT_BEGIN(unitSpiny_spiky_counter_event)
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_CheckStatus, -2, 10, LW(3))
    IF_NOT_EQUAL(LW(3), 0)
        ADD(LW(1), 45)
        GOTO(90)
    END_IF()
    USER_FUNC(btlevtcmd_CheckStatus, -2, 11, LW(3))
    IF_NOT_EQUAL(LW(3), 0)
        ADD(LW(1), 25)
        GOTO(90)
    END_IF()
    ADD(LW(1), 35)
    GOTO(90)
LBL(90)
    USER_FUNC(evt_eff, PTR(""), PTR("toge_flush"), 0, LW(0), LW(1), LW(2), 60, 0, 0, 0, 0, 0, 0, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitSpiny_flip_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_SetPartsDefenceTable, LW(10), LW(11), PTR(&unitSpiny_flip_defense))
    USER_FUNC(btlevtcmd_SetPartsDefenceAttrTable, LW(10), LW(11), PTR(&unitSpiny_flip_defense_attr))
    USER_FUNC(btlevtcmd_SetRegistStatus, LW(10), PTR(&unitSpiny_flip_status))
    USER_FUNC(btlevtcmd_AnimeSetPoseTable, LW(10), LW(11), PTR(&unitSpiny_flip_pose_table))
    USER_FUNC(btlevtcmd_OnStatusFlag, LW(10), 1)
    USER_FUNC(btlevtcmd_OffPartsCounterAttribute, LW(10), LW(11), 1)
    USER_FUNC(btlevtcmd_OffPartsCounterAttribute, LW(10), LW(11), 4)
    USER_FUNC(btlevtcmd_ChangeDataTable, LW(10), PTR(&unitSpiny_flip_data_table))
    USER_FUNC(btlevtcmd_SetOverTurnCount, LW(10), 1)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(10), LW(11), 39)
    USER_FUNC(evt_snd_sfxon, PTR("SFX_CONDITION_INSIDE"), 0)
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(1.0))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 18, -1)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
    RETURN()
EVT_END()

EVT_BEGIN(unitSpiny_guard_cancel_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGEZO_TURN2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 15, 0)
    DO(5)
        USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, -36)
        WAIT_FRM(1)
    WHILE()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("TGZ_S_1"))
    DO(5)
        USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, -36)
        WAIT_FRM(1)
    WHILE()
    WAIT_FRM(5)
    USER_FUNC(btlevtcmd_SetPartsDefenceTable, LW(10), LW(11), PTR(&unitSpiny_defense))
    USER_FUNC(btlevtcmd_SetPartsDefenceAttrTable, LW(10), LW(11), PTR(&unitSpiny_defense_attr))
    
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Spiny_BattleUnitType, LW(0))
    IF_EQUAL(LW(0), (int32_t)BattleUnitType::SPINY)
        USER_FUNC(btlevtcmd_SetRegistStatus, LW(10), PTR(&unitSpiny_status))
    ELSE()
        USER_FUNC(btlevtcmd_SetRegistStatus, LW(10), PTR(&unitSkyBlueSpiny_status))
    END_IF()

    USER_FUNC(btlevtcmd_AnimeSetPoseTable, LW(10), LW(11), PTR(&unitSpiny_pose_table))
    USER_FUNC(btlevtcmd_ChangeDataTable, LW(10), PTR(&unitSpiny_data_table))
    USER_FUNC(btlevtcmd_OnPartsAttribute, LW(10), LW(11), 0x1000)
    USER_FUNC(btlevtcmd_SetUnitWork, LW(10), UW_Spiny_IsGuarding, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitSpiny_guard_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGEZO_TURN1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 15, 0)
    DO(5)
        USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, 36)
        WAIT_FRM(1)
    WHILE()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("TGZ_P_1"))
    DO(5)
        USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, 36)
        WAIT_FRM(1)
    WHILE()
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    WAIT_FRM(5)
    USER_FUNC(btlevtcmd_SetPartsDefenceTable, LW(10), LW(11), PTR(&unitSpiny_guard_defense))
    USER_FUNC(btlevtcmd_SetPartsDefenceAttrTable, LW(10), LW(11), PTR(&unitSpiny_guard_defense_attr))
    USER_FUNC(btlevtcmd_SetRegistStatus, LW(10), PTR(&unitSpiny_guard_status))
    USER_FUNC(btlevtcmd_AnimeSetPoseTable, LW(10), LW(11), PTR(&unitSpiny_guard_pose_table))
    USER_FUNC(btlevtcmd_ChangeDataTable, LW(10), PTR(&unitSpiny_guard_data_table))
    USER_FUNC(btlevtcmd_OffPartsAttribute, LW(10), LW(11), 0x1000)
    USER_FUNC(btlevtcmd_SetUnitWork, LW(10), UW_Spiny_IsGuarding, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitSpiny_common_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitSpiny_wait_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitSpiny_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitSpiny_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitSpiny_attack_event))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Spiny_IsGuarding, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Spiny_IsCharged, 0)
    USER_FUNC(btlevtcmd_SetRunSound, -2, PTR("SFX_ENM_TOGEZO_MOVE1L"), PTR("SFX_ENM_TOGEZO_MOVE1R"), 0, 10, 10)
    USER_FUNC(btlevtcmd_SetWalkSound, -2, PTR("SFX_ENM_TOGEZO_MOVE1L"), PTR("SFX_ENM_TOGEZO_MOVE1R"), 0, 6, 6)
    USER_FUNC(btlevtcmd_SetJumpSound, -2, PTR("SFX_ENM_KURI_JUMP1"), PTR("SFX_ENM_KURI_LANDING1"))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitSpiny_normal_attack_event)
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_GetFirstAttackTarget, LW(3), LW(4))
    ELSE()
        USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
        USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), PTR(&unitSpiny_weaponAttack))
        USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, PTR(&unitSpiny_weaponAttack), LW(3), LW(4))
    END_IF()
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            RETURN()
        END_IF()
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_WeaponAftereffect, PTR(&unitSpiny_weaponAttack))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, PTR(&unitSpiny_weaponAttack))
    USER_FUNC(evt_btl_camera_set_mode, 0, 8)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, LW(3))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(14))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(14))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGEZO_TURN1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 15, 0)
    DO(5)
        SET(LW(0), -36)
        MUL(LW(0), LW(14))
        USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, LW(0))
        WAIT_FRM(1)
    WHILE()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("TGZ_P_1"))
    DO(5)
        SET(LW(0), -36)
        MUL(LW(0), LW(14))
        USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, LW(0))
        WAIT_FRM(1)
    WHILE()
    WAIT_FRM(5)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Spiny_Rotate, 1)
    SET(LW(0), -36)
    MUL(LW(0), LW(14))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Spiny_RotateSpd, LW(0))
    BROTHER_EVT()
        DO(0)
            USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Spiny_Rotate, LW(0))
            IF_EQUAL(LW(0), 0)
                DO_BREAK()
            END_IF()
            USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Spiny_RotateSpd, LW(0))
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, LW(0))
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()
    WAIT_FRM(10)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGEZO_ROLL1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpSetting, -2, 30, 0, FLOAT(0.15))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 30, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGEZO_ROLL2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(9.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), 0, LW(2), 0, 0, 1)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), PTR(&unitSpiny_weaponAttack), 0, LW(5))
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
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGEZO_ROLL5"), EVT_NULLPTR, 0, EVT_NULLPTR)
    BROTHER_EVT()
        SET(LW(0), -18)
        MUL(LW(0), LW(14))
        USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Spiny_RotateSpd, LW(0))
        SET(LW(0), 1)
        MUL(LW(0), LW(14))
        DO(18)
            USER_FUNC(btlevtcmd_AddUnitWork, -2, UW_Spiny_RotateSpd, LW(0))
            WAIT_FRM(3)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 50)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(2.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), 0, LW(2), 0, 0, 1)
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 26)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(1.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), 0, LW(2), 0, 0, 1)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Spiny_Rotate, 0)
    WAIT_FRM(10)
    GOTO(97)
LBL(91)
    USER_FUNC(evt_btl_camera_shake_h, 0, 1, 1, 8, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_add_zoom, 0, -50)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGEZO_ROLL4"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), PTR(&unitSpiny_weaponAttack))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&unitSpiny_weaponAttack), 256, LW(5))
    SET(LW(0), 36)
    MUL(LW(0), LW(14))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Spiny_RotateSpd, -36)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 30)
    ADD(LW(2), 10)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 30, 0, FLOAT(0.5))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 30, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGEZO_JUMP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 30)
    ADD(LW(2), 5)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 15, 0, FLOAT(0.5))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 15, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGEZO_JUMP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    BROTHER_EVT()
        SET(LW(0), 18)
        MUL(LW(0), LW(14))
        USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Spiny_RotateSpd, LW(0))
        SET(LW(0), -1)
        MUL(LW(0), LW(14))
        DO(18)
            USER_FUNC(btlevtcmd_AddUnitWork, -2, UW_Spiny_RotateSpd, LW(0))
            WAIT_FRM(2)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 18)
    ADD(LW(2), 10)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(1.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), 0, LW(2), 0, 0, 1)
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 9)
    ADD(LW(2), 5)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(0.5))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), 0, LW(2), 0, 0, 1)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Spiny_Rotate, 0)
    WAIT_FRM(3)

    // Check whether to roll out into a guard or not.
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Spiny_BattleUnitType, LW(0))
    IF_EQUAL(LW(0), (int32_t)BattleUnitType::SPINY)
        SET(LW(1), 30)
    ELSE()
        SET(LW(1), 50)
    END_IF()
    USER_FUNC(evt_sub_random, 99, LW(0))
        IF_SMALL(LW(0), LW(1))
            GOTO(98)
        ELSE()
            GOTO(97)
        END_IF()
    END_IF()
    
LBL(97)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGEZO_TURN2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 15, 0)
    DO(5)
        SET(LW(0), 36)
        MUL(LW(0), LW(14))
        USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, LW(0))
        WAIT_FRM(1)
    WHILE()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("TGZ_S_1"))
    DO(5)
        SET(LW(0), 36)
        MUL(LW(0), LW(14))
        USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, LW(0))
        WAIT_FRM(1)
    WHILE()
    WAIT_FRM(5)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(6.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("TGZ_S_1"))
    WAIT_MSEC(250)
    GOTO(99)

LBL(98)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGEZO_ROLL3"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Spiny_Rotate, 1)
    SET(LW(0), 36)
    MUL(LW(0), LW(14))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Spiny_RotateSpd, LW(0))
    BROTHER_EVT()
        DO(0)
            USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Spiny_Rotate, LW(0))
            IF_EQUAL(LW(0), 0)
                USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
                DO_BREAK()
            END_IF()
            USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Spiny_RotateSpd, LW(0))
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, LW(0))
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(6.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, 0, 1)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Spiny_Rotate, 0)
    USER_FUNC(btlevtcmd_SetPartsDefenceTable, -2, 1, PTR(&unitSpiny_guard_defense))
    USER_FUNC(btlevtcmd_SetPartsDefenceAttrTable, -2, 1, PTR(&unitSpiny_guard_defense_attr))
    USER_FUNC(btlevtcmd_SetRegistStatus, -2, PTR(&unitSpiny_guard_status))
    USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitSpiny_guard_pose_table))
    USER_FUNC(btlevtcmd_ChangeDataTable, -2, PTR(&unitSpiny_guard_data_table))
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, 0x1000)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Spiny_IsGuarding, 1)
    WAIT_MSEC(250)
    GOTO(99)

LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitSpiny_wait_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitSpiny_wakeup_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_GetOverTurnCount, LW(10), LW(0))
    SUB(LW(0), 1)
    USER_FUNC(btlevtcmd_SetOverTurnCount, LW(10), LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_StartWaitEvent, LW(10))
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_SetPartsDefenceTable, LW(10), LW(11), PTR(&unitSpiny_defense))
    USER_FUNC(btlevtcmd_SetPartsDefenceAttrTable, LW(10), LW(11), PTR(&unitSpiny_defense_attr))

    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Spiny_BattleUnitType, LW(0))
    IF_EQUAL(LW(0), (int32_t)BattleUnitType::SPINY)
        USER_FUNC(btlevtcmd_SetRegistStatus, LW(10), PTR(&unitSpiny_status))
    ELSE()
        USER_FUNC(btlevtcmd_SetRegistStatus, LW(10), PTR(&unitSkyBlueSpiny_status))
    END_IF()

    USER_FUNC(btlevtcmd_OffStatusFlag, LW(10), 1)
    USER_FUNC(btlevtcmd_AnimeSetPoseTable, LW(10), LW(11), PTR(&unitSpiny_pose_table))
    USER_FUNC(btlevtcmd_OnPartsCounterAttribute, LW(10), LW(11), 1)
    USER_FUNC(btlevtcmd_OnPartsCounterAttribute, LW(10), LW(11), 4)
    USER_FUNC(btlevtcmd_ChangeDataTable, LW(10), PTR(&unitSpiny_data_table))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(10), LW(11), 69)
    USER_FUNC(btlevtcmd_SetFallAccel, LW(10), FLOAT(1.0))
    USER_FUNC(btlevtcmd_GetPos, LW(10), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPosition, LW(10), LW(0), LW(1), LW(2), 18, -1)
    USER_FUNC(btlevtcmd_StartWaitEvent, LW(10))
    RETURN()
EVT_END()

EVT_BEGIN(unitSpiny_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Spiny_BattleUnitType, (int32_t)BattleUnitType::SPINY)
    RUN_CHILD_EVT(unitSpiny_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitSkyBlueSpiny_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Spiny_BattleUnitType, (int32_t)BattleUnitType::SKY_BLUE_SPINY)
    RUN_CHILD_EVT(unitSpiny_common_init_event)
    RETURN()
EVT_END()

}  // namespace mod::tot::custom