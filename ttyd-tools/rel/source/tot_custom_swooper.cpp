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
constexpr const int32_t UW_StartFlying = 0;
constexpr const int32_t UW_BattleUnitType = 1;

constexpr const int32_t Pose_A_2 = 0;
constexpr const int32_t Pose_N_1 = 1;
constexpr const int32_t Pose_S_1 = 2;
constexpr const int32_t Pose_S_2 = 3;

constexpr const char* kPoseLookupTbl[] = {
    "BSA_A_2", "CYU_A_2",
    "BSA_N_1", "CYU_N_1",
    "BSA_S_1", "CYU_S_1",
    "BSA_S_2", "CYU_S_2",
};

}  // namespace

// Evt / Function declarations.

extern const int32_t unitSwooper_init_event[];
extern const int32_t unitSwoopula_init_event[];
extern const int32_t unitSwampire_init_event[];
extern const int32_t unitSwooper_common_init_event[];
extern const int32_t unitSwooper_attack_event[];
extern const int32_t unitSwooper_damage_event[];
extern const int32_t unitSwooper_wait_event[];
extern const int32_t unitSwooper_normal_attack_event_sub[];
extern const int32_t unitSwooper_drain_attack_event_sub[];
extern const int32_t unitSwooper_damage_event_ceil[];
extern const int32_t unitSwooper_ceil_fall_ready_event[];
extern const int32_t unitSwooper_ceil_to_sky_event[];
extern const int32_t unitSwooper_counter_damage_event[];
extern const int32_t unitSwooper_first_attack_pos_event[];

EVT_DECLARE_USER_FUNC(unitSwooper_pose_lookup, 3)

// Unit data.

int8_t unitSwooper_defense[] = { 0, 0, 0, 0, 0 };
int8_t unitSwooper_defense_attr[] = { 0, 0, 0, 0, 0 };

StatusVulnerability unitSwooper_status = {
     80,  85, 105, 100,  80, 100, 100,  80,
    100,  90, 100,  90, 100,  95,  80,  80,
     95, 100,  80, 100, 100,  95,
};
StatusVulnerability unitSwoopula_status = {
     70,  75, 100, 100,  70, 100, 100,  70,
    100,  80, 100,  80, 100,  90,  70,  70,
     85, 100,  70, 100, 100,  95,
};
StatusVulnerability unitSwampire_status = {
     50,  55,  90, 100,  50, 100, 100,  50,
    100,  75, 100,  75, 100,  80,  50,  50,
     75, 100,  50, 100, 100,  60,
};

// Swampire uses Swooper's pose tables.
PoseTableEntry unitSwooper_pose_table[] = {
    1, "BSA_N_1",
    2, "BSA_Y_1",
    9, "BSA_Y_1",
    5, "BSA_K_1",
    4, "BSA_X_1",
    3, "BSA_X_1",
    28, "BSA_S_1",
    29, "BSA_Q_1",
    30, "BSA_Q_1",
    31, "BSA_S_1",
    39, "BSA_D_1",
    50, "BSA_A_1",
    42, "BSA_R_1",
    40, "BSA_W_1",
    56, "BSA_X_1",
    57, "BSA_X_1",
    65, "BSA_T_1",
    69, "BSA_S_1",
};
PoseTableEntry unitSwooper_ceil_pose_table[] = {
    1, "BSA_N_2",
    2, "BSA_Y_2",
    9, "BSA_Y_2",
    5, "BSA_K_2",
    4, "BSA_X_2",
    3, "BSA_X_2",
    28, "BSA_S_2",
    29, "BSA_Q_2",
    30, "BSA_Q_2",
    31, "BSA_S_2",
    39, "BSA_D_2",
    50, "BSA_A_1",
    42, "BSA_S_1",
    40, "BSA_S_1",
    56, "BSA_X_2",
    57, "BSA_X_2",
    65, "BSA_T_2",
    69, "BSA_S_2",
};
PoseTableEntry unitSwoopula_pose_table[] = {
    1, "CYU_N_1",
    2, "CYU_Y_1",
    9, "CYU_Y_1",
    5, "CYU_K_1",
    4, "CYU_X_1",
    3, "CYU_X_1",
    28, "CYU_S_1",
    29, "CYU_Q_1",
    30, "CYU_Q_1",
    31, "CYU_S_1",
    39, "CYU_D_1",
    50, "CYU_A_1",
    42, "CYU_R_1",
    40, "CYU_W_1",
    56, "CYU_X_1",
    57, "CYU_X_1",
    65, "CYU_T_1",
    69, "CYU_S_1",
};
PoseTableEntry unitSwoopula_ceil_pose_table[] = {
    1, "CYU_N_2",
    2, "CYU_Y_2",
    9, "CYU_Y_2",
    5, "CYU_K_2",
    4, "CYU_X_2",
    3, "CYU_X_2",
    28, "CYU_S_2",
    29, "CYU_Q_2",
    30, "CYU_Q_2",
    31, "CYU_S_2",
    39, "CYU_D_2",
    50, "CYU_A_1",
    42, "CYU_S_1",
    40, "CYU_S_1",
    56, "CYU_X_2",
    57, "CYU_X_2",
    65, "CYU_T_2",
    69, "CYU_S_2",
};

DataTableEntry unitSwooper_data_table[] = {
    25, (void*)unitSwooper_counter_damage_event,
    26, (void*)unitSwooper_counter_damage_event,
    27, (void*)unitSwooper_counter_damage_event,
    28, (void*)unitSwooper_counter_damage_event,
    29, (void*)unitSwooper_counter_damage_event,
    30, (void*)unitSwooper_counter_damage_event,
    31, (void*)unitSwooper_counter_damage_event,
    32, (void*)unitSwooper_counter_damage_event,
    33, (void*)unitSwooper_counter_damage_event,
    34, (void*)unitSwooper_counter_damage_event,
    35, (void*)unitSwooper_counter_damage_event,
    36, (void*)unitSwooper_counter_damage_event,
    48, (void*)unitSwooper_first_attack_pos_event,
    0, nullptr,
};

// Also used for Swampire.
const PoseSoundTimingEntry unitSwooper_pose_sound_timing_table[] = {
    { "BSA_S_1", 0.0100000f, 0, "SFX_ENM_BASA_WAIT1", 1 },
    { "BSA_W_1", 0.0100000f, 0, "SFX_ENM_BASA_MOVE1", 1 },
    { "BSA_R_1", 0.0100000f, 0, "SFX_ENM_BASA_MOVE1", 1 },
    { "BSA_T_1", 0.0100000f, 0, "SFX_ENM_BASA_WAIT1", 1 },
    { "BSA_A_1", 0.0100000f, 0, "SFX_ENM_BASA_WAIT1", 1 },
    { "BSA_N_1", 0.0100000f, 0, "SFX_ENM_BASA_WAIT1", 1 },
    { "BSA_K_1", 0.0100000f, 0, "SFX_ENM_BASA_WAIT1", 1 },
    { "BSA_X_1", 0.0100000f, 0, "SFX_ENM_BASA_WAIT1", 1 },
    { nullptr, 0.0f, 0, nullptr, 1 },
};
const PoseSoundTimingEntry unitSwoopula_pose_sound_timing_table[] = {
    { "CYU_S_1", 0.0100000f, 0, "SFX_ENM_BASA_WAIT1", 1 },
    { "CYU_W_1", 0.0100000f, 0, "SFX_ENM_BASA_MOVE1", 1 },
    { "CYU_R_1", 0.0100000f, 0, "SFX_ENM_BASA_MOVE1", 1 },
    { "CYU_T_1", 0.0100000f, 0, "SFX_ENM_BASA_WAIT1", 1 },
    { "CYU_A_1", 0.0100000f, 0, "SFX_ENM_BASA_WAIT1", 1 },
    { "CYU_N_1", 0.0100000f, 0, "SFX_ENM_BASA_WAIT1", 1 },
    { "CYU_K_1", 0.0100000f, 0, "SFX_ENM_BASA_WAIT1", 1 },
    { "CYU_X_1", 0.0100000f, 0, "SFX_ENM_BASA_WAIT1", 1 },
    { nullptr, 0.0f, 0, nullptr, 1 },
};

BattleWeapon unitSwooper_weaponNormal = {
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
        // Added to make sure that front-spiky counter functions properly.
        AttackCounterResistance_Flags::PREEMPTIVE_SPIKY,
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
BattleWeapon unitSwooper_weaponDrain = {
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
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING,
    .counter_resistance_flags =
        // Added to make sure that front-spiky counter functions properly.
        AttackCounterResistance_Flags::PREEMPTIVE_SPIKY,
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
BattleWeapon unitSwooper_weaponCountered = {
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
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
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

BattleUnitKindPart unitSwooper_parts[] = {
    {
        .index = 1,
        .name = "btl_un_basabasa",
        .model_name = "c_basabasa",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitSwooper_defense,
        .defense_attr = unitSwooper_defense_attr,
        .attribute_flags = 0x0000'0009 |
            PartsAttribute_Flags::HAMMERLIKE_CANNOT_TARGET |
            PartsAttribute_Flags::SHELLTOSSLIKE_CANNOT_TARGET,
        .counter_attribute_flags = 0,
        .pose_table = unitSwooper_ceil_pose_table,
    },
};
BattleUnitKindPart unitSwoopula_parts[] = {
    {
        .index = 1,
        .name = "btl_un_basabasa_chururu",
        .model_name = "c_cyuru",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitSwooper_defense,
        .defense_attr = unitSwooper_defense_attr,
        .attribute_flags = 0x0000'0009 |
            PartsAttribute_Flags::HAMMERLIKE_CANNOT_TARGET |
            PartsAttribute_Flags::SHELLTOSSLIKE_CANNOT_TARGET,
        .counter_attribute_flags = 0,
        .pose_table = unitSwoopula_ceil_pose_table,
    },
};
BattleUnitKindPart unitSwampire_parts[] = {
    {
        .index = 1,
        .name = "btl_un_basabasa_green",
        .model_name = "c_basabasa_g",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitSwooper_defense,
        .defense_attr = unitSwooper_defense_attr,
        .attribute_flags = 0x0000'0009 |
            PartsAttribute_Flags::HAMMERLIKE_CANNOT_TARGET |
            PartsAttribute_Flags::SHELLTOSSLIKE_CANNOT_TARGET,
        .counter_attribute_flags = 0,
        .pose_table = unitSwooper_ceil_pose_table,
    },
};

BattleUnitKind unit_Swooper = {
    .unit_type = BattleUnitType::SWOOPER,
    .unit_name = "btl_un_basabasa",
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
    .height = 32,
    .hit_offset = { 6, 32 },
    .center_offset = { 0.0f, 16.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 16.0f, 0.0f, -15.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 16.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 16.0f, 20.8f, 0.0f },
    .cut_base_offset = { 0.0f, 16.0f, 0.0f },
    .cut_width = 32.0f,
    .cut_height = 32.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_BASA_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::OUT_OF_REACH,
    .status_vulnerability = &unitSwooper_status,
    .num_parts = 1,
    .parts = unitSwooper_parts,
    .init_evt_code = (void*)unitSwooper_init_event,
    .data_table = unitSwooper_data_table,
};
BattleUnitKind unit_Swoopula = {
    .unit_type = BattleUnitType::SWOOPULA,
    .unit_name = "btl_un_basabasa_chururu",
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
    .height = 32,
    .hit_offset = { 6, 32 },
    .center_offset = { 0.0f, 16.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 16.0f, 0.0f, -15.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 16.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 16.0f, 20.8f, 0.0f },
    .cut_base_offset = { 0.0f, 16.0f, 0.0f },
    .cut_width = 32.0f,
    .cut_height = 32.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_BASA_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::OUT_OF_REACH,
    .status_vulnerability = &unitSwoopula_status,
    .num_parts = 1,
    .parts = unitSwoopula_parts,
    .init_evt_code = (void*)unitSwoopula_init_event,
    .data_table = unitSwooper_data_table,
};
BattleUnitKind unit_Swampire = {
    .unit_type = BattleUnitType::SWAMPIRE,
    .unit_name = "btl_un_basabasa_green",
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
    .height = 32,
    .hit_offset = { 6, 32 },
    .center_offset = { 0.0f, 16.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 16.0f, 0.0f, -15.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 16.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 16.0f, 20.8f, 0.0f },
    .cut_base_offset = { 0.0f, 16.0f, 0.0f },
    .cut_width = 32.0f,
    .cut_height = 32.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_BASA_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::OUT_OF_REACH,
    .status_vulnerability = &unitSwampire_status,
    .num_parts = 1,
    .parts = unitSwampire_parts,
    .init_evt_code = (void*)unitSwampire_init_event,
    .data_table = unitSwooper_data_table,
};

// Function / USER_FUNC definitions.

EVT_DEFINE_USER_FUNC(unitSwooper_pose_lookup) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[0]));
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, unit_idx);

    int32_t lookup = evtGetValue(evt, evt->evtArguments[1]);
    int32_t offset = unit->true_kind == BattleUnitType::SWOOPULA ? 1 : 0;

    evtSetValue(evt, evt->evtArguments[2], PTR(kPoseLookupTbl[lookup * 2 + offset]));
    return 2;
}

// Evt definitions.

EVT_BEGIN(unitSwooper_normal_attack_event_sub)
    USER_FUNC(evt_btl_camera_shake_h, 0, 1, 1, 8, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_add_zoom, 0, -50)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.5))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 50)
    ADD(LW(1), 10)
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 0, 0, 4, 0, -1)
    WAIT_MSEC(200)
    SET(LW(0), 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitSwooper_drain_attack_event_sub)
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
        SET(LW(9), PTR(&unitSwooper_weaponCountered))
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
        SET(LW(0), 1)
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, -45)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BASA_SUCK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(unitSwooper_pose_lookup, -2, Pose_A_2, LW(13))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, LW(13))
    DO(2)
        DO(10)
            USER_FUNC(btlevtcmd_AddScale, -2, 0, FLOAT(0.05), 0)
            WAIT_FRM(1)
        WHILE()
        DO(10)
            USER_FUNC(btlevtcmd_AddScale, -2, 0, FLOAT(-0.05), 0)
            WAIT_FRM(1)
        WHILE()
    WHILE()
    USER_FUNC(evt_btl_camera_shake_h, 0, 1, 1, 8, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_add_zoom, 0, -50)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(4.0))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 15)
    ADD(LW(2), 10)
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 0, 0, 0, 0, 0)
    USER_FUNC(btlevtcmd_GetDamage, LW(3), LW(10))
    IF_LARGE_EQUAL(LW(10), 1)
        USER_FUNC(unitSwooper_pose_lookup, -2, Pose_N_1, LW(13))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, LW(13))
        WAIT_MSEC(300)
        USER_FUNC(btlevtcmd_RecoverHp, -2, 1, LW(10))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(5), LW(6), LW(7))
        ADD(LW(6), 20)
        ADD(LW(7), 10)
        USER_FUNC(evt_eff, PTR(""), PTR("recovery"), 7, LW(5), LW(6), LW(7), LW(10), 0, 0, 0, 0, 0, 0, 0)
        SET(LW(0), 0)
        DO(0)
            IF_LARGE_EQUAL(LW(0), 5)
                DO_BREAK()
            END_IF()
            USER_FUNC(btlevtcmd_AddScale, -2, FLOAT(-0.1), FLOAT(0.1), 0)
            ADD(LW(0), 1)
            WAIT_FRM(1)
        WHILE()
        SET(LW(0), 0)
        DO(0)
            IF_LARGE_EQUAL(LW(0), 10)
                DO_BREAK()
            END_IF()
            USER_FUNC(btlevtcmd_AddScale, -2, FLOAT(0.1), FLOAT(-0.1), 0)
            ADD(LW(0), 1)
            WAIT_FRM(1)
        WHILE()
        SET(LW(0), 0)
        DO(0)
            IF_LARGE_EQUAL(LW(0), 5)
                DO_BREAK()
            END_IF()
            USER_FUNC(btlevtcmd_AddScale, -2, FLOAT(-0.1), FLOAT(0.1), 0)
            ADD(LW(0), 1)
            WAIT_FRM(1)
        WHILE()
        USER_FUNC(btlevtcmd_SetScale, -2, 1, 1, 1)
        WAIT_MSEC(1000)
    END_IF()
    WAIT_MSEC(500)
    SET(LW(0), 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitSwooper_attack_event)
    USER_FUNC(btlevtcmd_EnemyItemUseCheck, -2, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(LW(0))
        USER_FUNC(btlevtcmd_StartWaitEvent, -2)
        RETURN()
    END_IF()

    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(0))
    SWITCH(LW(0))
        CASE_EQUAL((int32_t)BattleUnitType::SWOOPER)
            USER_FUNC(btlevtcmd_DrawLots, LW(14), 2, 50, 0)
        CASE_EQUAL((int32_t)BattleUnitType::SWOOPULA)
            USER_FUNC(btlevtcmd_DrawLots, LW(14), 2, 0, 50)
        CASE_EQUAL((int32_t)BattleUnitType::SWAMPIRE)
            // TODO: Maybe don't do this outside of testing.
            USER_FUNC(btlevtcmd_DrawLots, LW(14), 2, 50, 50)
    END_SWITCH()
    IF_EQUAL(LW(14), 0)
        SET(LW(9), PTR(&unitSwooper_weaponNormal))
    ELSE()
        SET(LW(9), PTR(&unitSwooper_weaponDrain))
    END_IF()

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
    USER_FUNC(btlevtcmd_CheckAttribute, -2, 2, LW(0))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
        WAIT_FRM(1)
        USER_FUNC(btlevtcmd_AddPos, -2, 0, -30, 0)
        USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(2.5))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 30)
        SUB(LW(1), 40)
        USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 0, -16, 0, 0, -1)
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    ADD(LW(1), -6)
    DO(2)
        ADD(LW(1), 13)
        USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 10, 10, 0, 0, -1)
        ADD(LW(1), -13)
        USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 10, -10, 0, 0, -1)
    WHILE()

    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.5))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    SUB(LW(1), 10)
    IF_EQUAL(LW(14), 1)
        // For drain attack, add an additional X / Z offset.
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 15)
        SUB(LW(2), 10)
    END_IF()
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 0, -16, 0, 0, -1)

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
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.5))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 50)
    ADD(LW(1), 10)
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 0, 0, 0, 0, -1)
    WAIT_MSEC(200)
    GOTO(98)
LBL(91)
    IF_EQUAL(LW(14), 0)
        RUN_CHILD_EVT(PTR(&unitSwooper_normal_attack_event_sub))
    ELSE()
        RUN_CHILD_EVT(PTR(&unitSwooper_drain_attack_event_sub))
    END_IF()
    IF_NOT_EQUAL(LW(0), 0)
        RETURN()
    END_IF()
LBL(98)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(6.0))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_CheckAttribute, -2, 2, LW(7))
    IF_EQUAL(LW(7), 1)
        SUB(LW(1), 30)
    END_IF()
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 0, -10, 0, 0, -1)
LBL(99)
    USER_FUNC(btlevtcmd_CheckAttribute, -2, 2, LW(0))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(unitSwooper_pose_lookup, -2, Pose_S_2, LW(13))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, LW(13))
        WAIT_FRM(1)
        USER_FUNC(btlevtcmd_AddPos, -2, 0, 30, 0)
    ELSE()
        USER_FUNC(unitSwooper_pose_lookup, -2, Pose_S_1, LW(13))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, LW(13))
        WAIT_FRM(1)
    END_IF()
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitSwooper_ceil_fall_ready_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    RUN_CHILD_EVT(PTR(&unitSwooper_ceil_to_sky_event))
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 60, 0, 0, 0, -1)
    RETURN()
EVT_END()

EVT_BEGIN(unitSwooper_ceil_to_sky_event)
    USER_FUNC(btlevtcmd_CheckAttribute, -2, 2, LW(0))
    IF_EQUAL(LW(0), 0)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_OffAttribute, -2, 2)
    USER_FUNC(btlevtcmd_OnAttribute, -2, 4)
    USER_FUNC(btlevtcmd_SetPossessionItemOffset, -2, 0, 0, 0)
    USER_FUNC(btlevtcmd_SetEventCeilFall, -2, 0)

    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(0))
    IF_EQUAL(LW(0), (int32_t)BattleUnitType::SWOOPULA)
        USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitSwoopula_pose_table))
    ELSE()
        USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitSwooper_pose_table))
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    WAIT_FRM(1)
    USER_FUNC(btlevtcmd_AddPos, -2, 0, -30, 0)
    USER_FUNC(btlevtcmd_AddHomePos, -2, 0, -30, 0)
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitSwooper_damage_event))
    USER_FUNC(btlevtcmd_SetHitOffset, -2, 1, 0, 0, 0)
    USER_FUNC(btlevtcmd_SetHitCursorOffset, -2, 1, 0, 30, 0)
    USER_FUNC(btlevtcmd_SetKissHitOffset, -2, 16, 21, 0)
    USER_FUNC(btlevtcmd_SetStatusIconOffset, -2, 6, 32)
    USER_FUNC(btlevtcmd_AddHomePos, -2, 0, -40, 0)
LBL(99)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    RETURN()
EVT_END()

EVT_BEGIN(unitSwooper_counter_damage_event)
    RUN_CHILD_EVT(PTR(&unitSwooper_ceil_to_sky_event))
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_GetDamageCode, LW(10), LW(0))
    RUN_CHILD_EVT(PTR(&subsetevt_counter_damage))
    RETURN()
EVT_END()

EVT_BEGIN(unitSwooper_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    RETURN()
EVT_END()

EVT_BEGIN(unitSwooper_damage_event_ceil)
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
    USER_FUNC(btlevtcmd_GetTotalDamage, LW(10), LW(12))
    IF_EQUAL(LW(12), 0)
        RETURN()
    END_IF()
    RUN_CHILD_EVT(PTR(&unitSwooper_ceil_to_sky_event))
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 60, 0, 0, 0, -1)
    RETURN()
EVT_END()

EVT_BEGIN(unitSwooper_first_attack_pos_event)
    RUN_CHILD_EVT(PTR(&unitSwooper_ceil_to_sky_event))
    USER_FUNC(btlevtcmd_SetPos, -2, LW(0), 10, LW(2))
    USER_FUNC(btlevtcmd_WaitAttackEnd)
    RETURN()
EVT_END()

EVT_BEGIN(unitSwooper_wait_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitSwooper_common_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitSwooper_wait_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitSwooper_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitSwooper_damage_event_ceil))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitSwooper_attack_event))
    USER_FUNC(btlevtcmd_SetEventCeilFall, -2, PTR(&unitSwooper_ceil_fall_ready_event))
    USER_FUNC(btlevtcmd_SetPossessionItemOffset, -2, 0, -20, 0)
    USER_FUNC(btlevtcmd_SetHitOffset, -2, 1, 0, -50, 0)
    USER_FUNC(btlevtcmd_SetHitCursorOffset, -2, 1, 0, -30, 0)
    USER_FUNC(btlevtcmd_SetStatusIconOffset, -2, 6, 10)
    USER_FUNC(btlevtcmd_SetKissHitOffset, -2, 16, -18, 0)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_StartFlying, LW(0))
    IF_EQUAL(LW(0), 1)
        RUN_CHILD_EVT(PTR(&unitSwooper_ceil_to_sky_event))
        USER_FUNC(btlevtcmd_SetPos, -2, LW(0), LW(1), LW(2))
    END_IF()
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitSwooper_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::SWOOPER)
    USER_FUNC(btlevtcmd_AnimeSetPoseSoundTable, -2, 1, PTR(&unitSwooper_pose_sound_timing_table))
    RUN_CHILD_EVT(unitSwooper_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitSwoopula_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::SWOOPULA)
    USER_FUNC(btlevtcmd_AnimeSetPoseSoundTable, -2, 1, PTR(&unitSwoopula_pose_sound_timing_table))
    RUN_CHILD_EVT(unitSwooper_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitSwampire_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::SWAMPIRE)
    USER_FUNC(btlevtcmd_AnimeSetPoseSoundTable, -2, 1, PTR(&unitSwooper_pose_sound_timing_table))
    RUN_CHILD_EVT(unitSwooper_common_init_event)
    RETURN()
EVT_END()

}  // namespace mod::tot::custom