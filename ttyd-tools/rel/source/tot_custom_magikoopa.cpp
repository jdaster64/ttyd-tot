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
#include <ttyd/eff_magic1_n64.h>
#include <ttyd/eff_pokopi_pcharge_n64.h>
#include <ttyd/eff_thunderflare_n64.h>
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
using ::ttyd::effdrv::EffEntry;
using ::ttyd::evtmgr_cmd::evtGetFloat;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetFloat;
using ::ttyd::evtmgr_cmd::evtSetValue;

// Constants (UW, etc.)
constexpr const int32_t UW_IsFlying = 0;
constexpr const int32_t UW_IsClone = 1;
constexpr const int32_t UW_KillClones = 2;
constexpr const int32_t UW_CloneKillSignalSent = 3;
constexpr const int32_t UW_Eff = 4;
constexpr const int32_t UW_MagicEff = 5;
constexpr const int32_t UW_BattleUnitType = 15;

constexpr const int32_t Pose_A_1A = 0;
constexpr const int32_t Pose_A_1B = 1;
constexpr const int32_t Pose_A_1C = 2;
constexpr const int32_t Pose_A_1D = 3;
constexpr const int32_t Pose_A_1E = 4;
constexpr const int32_t Pose_A_2A = 5;
constexpr const int32_t Pose_A_2B = 6;
constexpr const int32_t Pose_A_2C = 7;
constexpr const int32_t Pose_A_3A = 8;
constexpr const int32_t Pose_A_3B = 9;
constexpr const int32_t Pose_A_3C = 10;
constexpr const int32_t Pose_A_3D = 11;
constexpr const int32_t Pose_H_1 = 12;

constexpr const char* kPoseLookupTbl[] = {
    "KMK_A_1A", "KMR_A_1A", "KMW_A_1A", "KMG_A_1A", 
    "KMK_A_1B", "KMR_A_1B", "KMW_A_1B", "KMG_A_1B", 
    "KMK_A_1C", "KMR_A_1C", "KMW_A_1C", "KMG_A_1C", 
    "KMK_A_1D", "KMR_A_1D", "KMW_A_1D", "KMG_A_1D", 
    "KMK_A_1E", "KMR_A_1E", "KMW_A_1E", "KMG_A_1E", 
    "KMK_A_2A", "KMR_A_2A", "KMW_A_2A", "KMG_A_2A", 
    "KMK_A_2B", "KMR_A_2B", "KMW_A_2B", "KMG_A_2B", 
    "KMK_A_2C", "KMR_A_2C", "KMW_A_2C", "KMG_A_2C", 
    "KMK_A_3A", "KMR_A_3A", "KMW_A_3A", "KMG_A_3A", 
    "KMK_A_3B", "KMR_A_3B", "KMW_A_3B", "KMG_A_3B", 
    "KMK_A_3C", "KMR_A_3C", "KMW_A_3C", "KMG_A_3C", 
    "KMK_A_3D", "KMR_A_3D", "KMW_A_3D", "KMG_A_3D", 
    "KMK_H_1", "KMR_H_1", "KMW_H_1", "KMG_H_1", 
};

}  // namespace

// Evt / Function declarations.

extern const int32_t unitMagikoopa_init_event[];
extern const int32_t unitRedMagikoopa_init_event[];
extern const int32_t unitWhiteMagikoopa_init_event[];
extern const int32_t unitGreenMagikoopa_init_event[];
extern const int32_t unitMagikoopa_common_init_event[];
extern const int32_t unitMagikoopa_common_init_event2[];
extern const int32_t unitMagikoopa_attack_event[];
extern const int32_t unitMagikoopa_damage_event[];
extern const int32_t unitMagikoopa_wait_event[];
extern const int32_t unitMagikoopa_alpha_down_event[];
extern const int32_t unitMagikoopa_alpha_up_event[];
extern const int32_t unitMagikoopa_clone_dead_event[];
extern const int32_t unitMagikoopa_dead_event[];
extern const int32_t unitMagikoopa_kill_all_clones_event[];
extern const int32_t unitMagikoopa_gale_dead_event[];
extern const int32_t unitMagikoopa_all_change_pose[];
extern const int32_t unitMagikoopa_ground_magic_common_event1[];
extern const int32_t unitMagikoopa_ground_magic_common_event2[];
extern const int32_t unitMagikoopa_sky_magic_common_event[];
extern const int32_t unitMagikoopa_magic_common_event1[];
extern const int32_t unitMagikoopa_magic_common_event2[];
extern const int32_t unitMagikoopa_support_magic_event[];
extern const int32_t unitMagikoopa_first_attack_pos_event[];
extern const int32_t unitMagikoopa_fall_event[];
extern const int32_t unitMagikoopa_one_recover_event[];
extern const int32_t unitMagikoopa_all_recover_event[];
extern const int32_t unitMagikoopa_shot_miss_event[];
extern const int32_t unitMagikoopa_incoming_miss_event[];
extern const int32_t unitMagikoopa_attack_magic_sub_hit_event[];
extern const int32_t unitMagikoopa_attack_magic_sub_nohit_event[];
extern const int32_t unitMagikoopa_clone_spawn_event[];
extern const int32_t unitMagikoopa_create_clones_event[];
extern const int32_t unitMagikoopa_attack_magic_event[];
extern const int32_t unitMagikoopa_huge_magic_event[];
extern const int32_t unitMagikoopa_def_magic_event[];
extern const int32_t unitMagikoopa_electric_magic_event[];
extern const int32_t unitMagikoopa_invis_magic_event[];

EVT_DECLARE_USER_FUNC(unitMagikoopa_eff_magic_xpos, 2)
EVT_DECLARE_USER_FUNC(unitMagikoopa_eff_magic, 5)
EVT_DECLARE_USER_FUNC(unitMagikoopa_eff_staff1, 1)
EVT_DECLARE_USER_FUNC(unitMagikoopa_eff_staff2, 1)
EVT_DECLARE_USER_FUNC(unitMagikoopa_copy_status, 2)
EVT_DECLARE_USER_FUNC(unitMagikoopa_shuffle_clones, 0)
EVT_DECLARE_USER_FUNC(unitMagikoopa_get_clones, 5)
EVT_DECLARE_USER_FUNC(unitMagikoopa_pose_lookup, 3)

// Unit data.

int8_t unitMagikoopa_defense[] = { 0, 0, 0, 0, 0 };
int8_t unitMagikoopa_defense_attr[] = { 0, 0, 0, 0, 0 };
int8_t unitGreenMagikoopa_defense[] = { 1, 1, 1, 1, 1 };
int8_t unitGreenMagikoopa_defense_attr[] = { 0, 0, 0, 0, 0 };

// Used for all types.
StatusVulnerability unitMagikoopa_status = {
     70,  75,  80, 100,  70, 100, 100,  70,
    100,  90, 100,  90, 100,  95,  80,  70,
     80, 100,  80, 100, 100,  95,
};
StatusVulnerability unitMagikoopaClone_status = {
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
};

PoseTableEntry unitMagikoopa_pose_table[] = {
    1, "KMK_N_1",
    2, "KMK_Y_1",
    9, "KMK_Y_1",
    5, "KMK_K_1",
    4, "KMK_X_1",
    3, "KMK_X_1",
    28, "KMK_S_1",
    29, "KMK_Q_1",
    30, "KMK_Q_1",
    31, "KMK_S_1",
    39, "KMK_D_1",
    42, "KMK_R_1",
    40, "KMK_W_1",
    56, "KMK_X_1",
    57, "KMK_X_1",
    65, "KMK_T_1",
    69, "KMK_S_1",
};
PoseTableEntry unitMagikoopa_flying_pose_table[] = {
    1, "KMK_N_2",
    2, "KMK_Y_2",
    9, "KMK_Y_2",
    5, "KMK_K_2",
    4, "KMK_X_2",
    3, "KMK_X_2",
    28, "KMK_S_2",
    29, "KMK_Q_2",
    30, "KMK_Q_2",
    31, "KMK_S_2",
    39, "KMK_D_2",
    42, "KMK_R_2",
    40, "KMK_W_2",
    56, "KMK_X_2",
    57, "KMK_X_2",
    65, "KMK_T_2",
    69, "KMK_S_2",
};
PoseTableEntry unitRedMagikoopa_pose_table[] = {
    1, "KMR_N_1",
    2, "KMR_Y_1",
    9, "KMR_Y_1",
    5, "KMR_K_1",
    4, "KMR_X_1",
    3, "KMR_X_1",
    28, "KMR_S_1",
    29, "KMR_Q_1",
    30, "KMR_Q_1",
    31, "KMR_S_1",
    39, "KMR_D_1",
    42, "KMR_R_1",
    40, "KMR_W_1",
    56, "KMR_X_1",
    57, "KMR_X_1",
    65, "KMR_T_1",
    69, "KMR_S_1",
};
PoseTableEntry unitRedMagikoopa_flying_pose_table[] = {
    1, "KMR_N_2",
    2, "KMR_Y_2",
    9, "KMR_Y_2",
    5, "KMR_K_2",
    4, "KMR_X_2",
    3, "KMR_X_2",
    28, "KMR_S_2",
    29, "KMR_Q_2",
    30, "KMR_Q_2",
    31, "KMR_S_2",
    39, "KMR_D_2",
    42, "KMR_R_2",
    40, "KMR_W_2",
    56, "KMR_X_2",
    57, "KMR_X_2",
    65, "KMR_T_2",
    69, "KMR_S_2",
};
PoseTableEntry unitWhiteMagikoopa_pose_table[] = {
    1, "KMW_N_1",
    2, "KMW_Y_1",
    9, "KMW_Y_1",
    5, "KMW_K_1",
    4, "KMW_X_1",
    3, "KMW_X_1",
    28, "KMW_S_1",
    29, "KMW_Q_1",
    30, "KMW_Q_1",
    31, "KMW_S_1",
    39, "KMW_D_1",
    42, "KMW_R_1",
    40, "KMW_W_1",
    56, "KMW_X_1",
    57, "KMW_X_1",
    65, "KMW_T_1",
    69, "KMW_S_1",
};
PoseTableEntry unitWhiteMagikoopa_flying_pose_table[] = {
    1, "KMW_N_2",
    2, "KMW_Y_2",
    9, "KMW_Y_2",
    5, "KMW_K_2",
    4, "KMW_X_2",
    3, "KMW_X_2",
    28, "KMW_S_2",
    29, "KMW_Q_2",
    30, "KMW_Q_2",
    31, "KMW_S_2",
    39, "KMW_D_2",
    42, "KMW_R_2",
    40, "KMW_W_2",
    56, "KMW_X_2",
    57, "KMW_X_2",
    65, "KMW_T_2",
    69, "KMW_S_2",
};
PoseTableEntry unitGreenMagikoopa_pose_table[] = {
    1, "KMG_N_1",
    2, "KMG_Y_1",
    9, "KMG_Y_1",
    5, "KMG_K_1",
    4, "KMG_X_1",
    3, "KMG_X_1",
    28, "KMG_S_1",
    29, "KMG_Q_1",
    30, "KMG_Q_1",
    31, "KMG_S_1",
    39, "KMG_D_1",
    42, "KMG_R_1",
    40, "KMG_W_1",
    56, "KMG_X_1",
    57, "KMG_X_1",
    65, "KMG_T_1",
    69, "KMG_S_1",
};
PoseTableEntry unitGreenMagikoopa_flying_pose_table[] = {
    1, "KMG_N_2",
    2, "KMG_Y_2",
    9, "KMG_Y_2",
    5, "KMG_K_2",
    4, "KMG_X_2",
    3, "KMG_X_2",
    28, "KMG_S_2",
    29, "KMG_Q_2",
    30, "KMG_Q_2",
    31, "KMG_S_2",
    39, "KMG_D_2",
    42, "KMG_R_2",
    40, "KMG_W_2",
    56, "KMG_X_2",
    57, "KMG_X_2",
    65, "KMG_T_2",
    69, "KMG_S_2",
};

DataTableEntry unitMagikoopa_data_table[] = {
    49, (void*)unitMagikoopa_dead_event,
    58, (void*)unitMagikoopa_gale_dead_event,
    46, (void*)unitMagikoopa_incoming_miss_event,
    48, (void*)btldefaultevt_Dummy,
    0, nullptr,
};
DataTableEntry unitMagikoopa_flying_data_table[] = {
    49, (void*)unitMagikoopa_dead_event,
    58, (void*)unitMagikoopa_gale_dead_event,
    46, (void*)unitMagikoopa_incoming_miss_event,
    14, (void*)unitMagikoopa_fall_event,
    48, (void*)btldefaultevt_Dummy,
    0, nullptr,
};
DataTableEntry unitMagikoopaClone_data_table[] = {
    46, (void*)unitMagikoopa_incoming_miss_event,
    48, (void*)btldefaultevt_Dummy,
    0, nullptr,
};
DataTableEntry unitMagikoopaClone_flying_data_table[] = {
    46, (void*)unitMagikoopa_incoming_miss_event,
    14, (void*)unitMagikoopa_fall_event,
    48, (void*)btldefaultevt_Dummy,
    0, nullptr,
};

BattleWeapon unitMagikoopa_weaponAttack = {
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
        AttackSpecialProperty_Flags::DEFENSE_PIERCING |
        AttackSpecialProperty_Flags::FREEZE_BREAK,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = AttackTargetWeighting_Flags::WEIGHTED_RANDOM,
        
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
BattleWeapon unitMagikoopa_weaponHuge = {
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
        AttackSpecialProperty_Flags::IGNORES_STATUS_CHANCE |
        AttackSpecialProperty_Flags::UNGUARDABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = AttackTargetWeighting_Flags::WEIGHTED_RANDOM,
        
    // status chances
    .size_change_chance = 100,
    .size_change_time = 3,
    .size_change_strength = 2,
    
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
BattleWeapon unitMagikoopa_weaponDef = {
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
        AttackSpecialProperty_Flags::IGNORES_STATUS_CHANCE |
        AttackSpecialProperty_Flags::UNGUARDABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = AttackTargetWeighting_Flags::WEIGHTED_RANDOM,
        
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
BattleWeapon unitMagikoopa_weaponElectric = {
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
        AttackSpecialProperty_Flags::IGNORES_STATUS_CHANCE |
        AttackSpecialProperty_Flags::UNGUARDABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = AttackTargetWeighting_Flags::WEIGHTED_RANDOM,
        
    // status chances
    .electric_chance = 100,
    .electric_time = 3,
    
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
BattleWeapon unitMagikoopa_weaponInvis = {
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
        AttackSpecialProperty_Flags::IGNORES_STATUS_CHANCE |
        AttackSpecialProperty_Flags::UNGUARDABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = AttackTargetWeighting_Flags::WEIGHTED_RANDOM,
        
    // status chances
    .invisible_chance = 100,
    .invisible_time = 3,
    
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
BattleWeapon unitMagikoopa_weaponRecoverOne = {
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
BattleWeapon unitMagikoopa_weaponRecoverAll = {
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

BattleUnitKindPart unitMagikoopa_parts[] = {
    {
        .index = 1,
        .name = "btl_un_kamec",
        .model_name = "c_kamek",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 35.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 45.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitMagikoopa_defense,
        .defense_attr = unitMagikoopa_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitMagikoopa_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_kamec",
        .model_name = "c_kamek",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitMagikoopa_defense,
        .defense_attr = unitMagikoopa_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitMagikoopa_pose_table,
    },
};
BattleUnitKindPart unitRedMagikoopa_parts[] = {
    {
        .index = 1,
        .name = "btl_un_kamec_red",
        .model_name = "c_kamek_r",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 35.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 45.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitMagikoopa_defense,
        .defense_attr = unitMagikoopa_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitRedMagikoopa_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_kamec_red",
        .model_name = "c_kamek_r",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitMagikoopa_defense,
        .defense_attr = unitMagikoopa_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitRedMagikoopa_pose_table,
    },
};
BattleUnitKindPart unitWhiteMagikoopa_parts[] = {
    {
        .index = 1,
        .name = "btl_un_kamec_white",
        .model_name = "c_kamek_w",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 35.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 45.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitMagikoopa_defense,
        .defense_attr = unitMagikoopa_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitWhiteMagikoopa_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_kamec_white",
        .model_name = "c_kamek_w",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitMagikoopa_defense,
        .defense_attr = unitMagikoopa_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitWhiteMagikoopa_pose_table,
    },
};
BattleUnitKindPart unitGreenMagikoopa_parts[] = {
    {
        .index = 1,
        .name = "btl_un_kamec_green",
        .model_name = "c_kamek_g",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 35.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 45.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitGreenMagikoopa_defense,
        .defense_attr = unitGreenMagikoopa_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitGreenMagikoopa_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_kamec_green",
        .model_name = "c_kamek_g",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitGreenMagikoopa_defense,
        .defense_attr = unitGreenMagikoopa_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitGreenMagikoopa_pose_table,
    },
};

BattleUnitKind unit_Magikoopa = {
    .unit_type = BattleUnitType::MAGIKOOPA,
    .unit_name = "btl_un_kamec",
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
    .width = 42,
    .height = 42,
    .hit_offset = { 0, 42 },
    .center_offset = { 0.0f, 21.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 21.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 21.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 21.0f, 27.3f, 0.0f },
    .cut_base_offset = { 0.0f, 21.0f, 0.0f },
    .cut_width = 42.0f,
    .cut_height = 42.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_KAMEKU_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitMagikoopa_status,
    .num_parts = 2,
    .parts = unitMagikoopa_parts,
    .init_evt_code = (void*)unitMagikoopa_init_event,
    .data_table = unitMagikoopa_data_table,
};
BattleUnitKind unit_MagikoopaClone = {
    .unit_type = BattleUnitType::MAGIKOOPA_CLONE,
    .unit_name = "btl_un_kamec_bunsin",
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
    .width = 42,
    .height = 42,
    .hit_offset = { 0, 42 },
    .center_offset = { 0.0f, 21.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 21.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 21.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 21.0f, 27.3f, 0.0f },
    .cut_base_offset = { 0.0f, 21.0f, 0.0f },
    .cut_width = 42.0f,
    .cut_height = 42.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 0,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_KAMEKU_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitMagikoopaClone_status,
    .num_parts = 2,
    .parts = unitMagikoopa_parts,
    .init_evt_code = (void*)unitMagikoopa_init_event,
    .data_table = unitMagikoopaClone_data_table,
};
BattleUnitKind unit_RedMagikoopa = {
    .unit_type = BattleUnitType::RED_MAGIKOOPA,
    .unit_name = "btl_un_kamec_red",
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
    .width = 42,
    .height = 42,
    .hit_offset = { 0, 42 },
    .center_offset = { 0.0f, 21.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 21.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 21.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 21.0f, 27.3f, 0.0f },
    .cut_base_offset = { 0.0f, 21.0f, 0.0f },
    .cut_width = 42.0f,
    .cut_height = 42.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_KAMEKU_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitMagikoopa_status,
    .num_parts = 2,
    .parts = unitRedMagikoopa_parts,
    .init_evt_code = (void*)unitRedMagikoopa_init_event,
    .data_table = unitMagikoopa_data_table,
};

// magikoopa_2   unit_kamec_red_bunsin unit_kamec_red.o
BattleUnitKind unit_RedMagikoopaClone = {
    .unit_type = BattleUnitType::RED_MAGIKOOPA_CLONE,
    .unit_name = "btl_un_kamec_red_bunsin",
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
    .width = 42,
    .height = 42,
    .hit_offset = { 0, 42 },
    .center_offset = { 0.0f, 21.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 21.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 21.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 21.0f, 27.3f, 0.0f },
    .cut_base_offset = { 0.0f, 21.0f, 0.0f },
    .cut_width = 42.0f,
    .cut_height = 42.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 0,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_KAMEKU_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitMagikoopaClone_status,
    .num_parts = 2,
    .parts = unitRedMagikoopa_parts,
    .init_evt_code = (void*)unitRedMagikoopa_init_event,
    .data_table = unitMagikoopaClone_data_table,
};

// magikoopa_3   unit_kamec_white unit_kamec_white.o
BattleUnitKind unit_WhiteMagikoopa = {
    .unit_type = BattleUnitType::WHITE_MAGIKOOPA,
    .unit_name = "btl_un_kamec_white",
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
    .width = 42,
    .height = 42,
    .hit_offset = { 0, 42 },
    .center_offset = { 0.0f, 21.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 21.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 21.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 21.0f, 27.3f, 0.0f },
    .cut_base_offset = { 0.0f, 21.0f, 0.0f },
    .cut_width = 42.0f,
    .cut_height = 42.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_KAMEKU_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitMagikoopa_status,
    .num_parts = 2,
    .parts = unitWhiteMagikoopa_parts,
    .init_evt_code = (void*)unitWhiteMagikoopa_init_event,
    .data_table = unitMagikoopa_data_table,
};

// magikoopa_3   unit_kamec_white_bunsin unit_kamec_white.o
BattleUnitKind unit_WhiteMagikoopaClone = {
    .unit_type = BattleUnitType::WHITE_MAGIKOOPA_CLONE,
    .unit_name = "btl_un_kamec_white_bunsin",
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
    .width = 42,
    .height = 42,
    .hit_offset = { 0, 42 },
    .center_offset = { 0.0f, 21.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 21.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 21.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 21.0f, 27.3f, 0.0f },
    .cut_base_offset = { 0.0f, 21.0f, 0.0f },
    .cut_width = 42.0f,
    .cut_height = 42.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 0,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_KAMEKU_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitMagikoopaClone_status,
    .num_parts = 2,
    .parts = unitWhiteMagikoopa_parts,
    .init_evt_code = (void*)unitWhiteMagikoopa_init_event,
    .data_table = unitMagikoopaClone_data_table,
};
BattleUnitKind unit_GreenMagikoopa = {
    .unit_type = BattleUnitType::GREEN_MAGIKOOPA,
    .unit_name = "btl_un_kamec_green",
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
    .width = 42,
    .height = 42,
    .hit_offset = { 0, 42 },
    .center_offset = { 0.0f, 21.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 21.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 21.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 21.0f, 27.3f, 0.0f },
    .cut_base_offset = { 0.0f, 21.0f, 0.0f },
    .cut_width = 42.0f,
    .cut_height = 42.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_KAMEKU_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitMagikoopa_status,
    .num_parts = 2,
    .parts = unitGreenMagikoopa_parts,
    .init_evt_code = (void*)unitGreenMagikoopa_init_event,
    .data_table = unitMagikoopa_data_table,
};
BattleUnitKind unit_GreenMagikoopaClone = {
    .unit_type = BattleUnitType::GREEN_MAGIKOOPA_CLONE,
    .unit_name = "btl_un_kamec_green_bunsin",
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
    .width = 42,
    .height = 42,
    .hit_offset = { 0, 42 },
    .center_offset = { 0.0f, 21.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 21.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 21.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 21.0f, 27.3f, 0.0f },
    .cut_base_offset = { 0.0f, 21.0f, 0.0f },
    .cut_width = 42.0f,
    .cut_height = 42.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 0,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_KAMEKU_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitMagikoopaClone_status,
    .num_parts = 2,
    .parts = unitGreenMagikoopa_parts,
    .init_evt_code = (void*)unitGreenMagikoopa_init_event,
    .data_table = unitMagikoopaClone_data_table,
};

const BattleUnitSetup unitMagikoopaClone_spawn_entry = {
    .unit_kind_params = &unit_MagikoopaClone,
    .alliance = 1,
    .attack_phase = 0x04000004,
    .position = { -1000.0f, -1000.0f, -1000.0f },
    .addl_target_offset_x = 0,
    .unit_work = { 0, 0, 0, 0 },
    .item_drop_table = nullptr,
};
const BattleUnitSetup unitRedMagikoopaClone_spawn_entry = {
    .unit_kind_params = &unit_RedMagikoopaClone,
    .alliance = 1,
    .attack_phase = 0x04000004,
    .position = { -1000.0f, -1000.0f, -1000.0f },
    .addl_target_offset_x = 0,
    .unit_work = { 0, 0, 0, 0 },
    .item_drop_table = nullptr,
};
const BattleUnitSetup unitWhiteMagikoopaClone_spawn_entry = {
    .unit_kind_params = &unit_WhiteMagikoopaClone,
    .alliance = 1,
    .attack_phase = 0x04000004,
    .position = { -1000.0f, -1000.0f, -1000.0f },
    .addl_target_offset_x = 0,
    .unit_work = { 0, 0, 0, 0 },
    .item_drop_table = nullptr,
};
const BattleUnitSetup unitGreenMagikoopaClone_spawn_entry = {
    .unit_kind_params = &unit_GreenMagikoopaClone,
    .alliance = 1,
    .attack_phase = 0x04000004,
    .position = { -1000.0f, -1000.0f, -1000.0f },
    .addl_target_offset_x = 0,
    .unit_work = { 0, 0, 0, 0 },
    .item_drop_table = nullptr,
};

// Function / USER_FUNC definitions.

EVT_DEFINE_USER_FUNC(unitMagikoopa_eff_magic_xpos) {
    int32_t id = evtGetValue(evt, evt->evtArguments[0]);
    id = ttyd::battle_sub::BattleTransID(evt, id);
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, id);
    
    evtSetFloat(
        evt, evt->evtArguments[1],
        *reinterpret_cast<float*>(reinterpret_cast<intptr_t>(
            reinterpret_cast<EffEntry*>(unit->unit_work[UW_MagicEff])->eff_work) + 0x10));
    
    return 2;
}

EVT_DEFINE_USER_FUNC(unitMagikoopa_eff_magic) {
    int32_t id = evtGetValue(evt, evt->evtArguments[0]);
    id = ttyd::battle_sub::BattleTransID(evt, id);
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, id);
    
    float x, y, z;
    ttyd::battle_unit::BtlUnit_GetPos(unit, &x, &y, &z);
    
    EffEntry* eff = ttyd::eff_magic1_n64::effMagic1N64Entry(
        x + 25.0f * unit->unk_scale * unit->movement_params.face_direction,
        y + 20.0f * unit->unk_scale,
        z + 5.0f,
        evtGetFloat(evt, evt->evtArguments[1]),
        evtGetFloat(evt, evt->evtArguments[2]),
        evtGetFloat(evt, evt->evtArguments[3]),
        0,
        evtGetValue(evt, evt->evtArguments[4]));
        
    unit->unit_work[UW_MagicEff] = reinterpret_cast<uint32_t>(eff);
    *reinterpret_cast<float*>(
        reinterpret_cast<intptr_t>(eff->eff_work) + 0x28) = unit->unk_scale;

    return 2;
}

EVT_DEFINE_USER_FUNC(unitMagikoopa_eff_staff1) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[0]));
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, unit_idx);
    
    if (isFirstCall) {
        unit->unit_work[UW_Eff] = reinterpret_cast<uint32_t>(
            ttyd::eff_thunderflare_n64::effThunderflareN64Entry(
                0.0f, -1000.0f, 0.0f, 1.0f, 0, 60));
    }
    
    auto* eff = reinterpret_cast<EffEntry*>(unit->unit_work[UW_Eff]);
    if (eff->flags & 1) {
        return 2;
    }
    
    float x, y, z;
    ttyd::battle_unit::BtlUnit_GetPos(unit, &x, &y, &z);
    
    intptr_t work = reinterpret_cast<intptr_t>(eff->eff_work);
    
    *reinterpret_cast<float*>(work + 0x08) = 
        x + 22.5f * unit->unk_scale * unit->movement_params.face_direction;
    *reinterpret_cast<float*>(work + 0x0c) = y + 40.0f * unit->unk_scale;
    *reinterpret_cast<float*>(work + 0x10) = z + 5.0f;
    *reinterpret_cast<float*>(work + 0x28) = unit->unk_scale;
    
    return 0;
}

EVT_DEFINE_USER_FUNC(unitMagikoopa_eff_staff2) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[0]));
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, unit_idx);
    
    if (isFirstCall) {
        unit->unit_work[UW_Eff] = reinterpret_cast<uint32_t>(
            ttyd::eff_pokopi_pcharge_n64::effPokopiPchargeN64Entry(
                0.0f, -1000.0f, 0.0f, 1.0f, 0, 60));
    }
    
    auto* eff = reinterpret_cast<EffEntry*>(unit->unit_work[UW_Eff]);
    if (eff->flags & 1) {
        return 2;
    }
    
    float x, y, z;
    ttyd::battle_unit::BtlUnit_GetPos(unit, &x, &y, &z);
    
    intptr_t work = reinterpret_cast<intptr_t>(eff->eff_work);
    
    *reinterpret_cast<float*>(work + 0x04) =
        x + 22.5f * unit->unk_scale * unit->movement_params.face_direction;
    *reinterpret_cast<float*>(work + 0x08) = y + 40.0f * unit->unk_scale;
    *reinterpret_cast<float*>(work + 0x0c) = z + 5.0f;
    *reinterpret_cast<float*>(work + 0x34) = unit->unk_scale;
    
    return 0;
}

EVT_DEFINE_USER_FUNC(unitMagikoopa_copy_status) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[0]));
    auto* unit1 = ttyd::battle::BattleGetUnitPtr(g_BattleWork, unit_idx);
    auto* unit2 = ttyd::battle::BattleGetUnitPtr(
        g_BattleWork, evtGetValue(evt, evt->evtArguments[1]));
        
    memcpy(&unit2->sleep_turns, &unit1->sleep_turns, 0x1e);
    
    unit2->current_hp = unit1->current_hp;
    unit2->hp_gauge_params.previous_hp = unit1->current_hp;
    unit2->hp_gauge_params.target_hp = unit1->current_hp;
    
    float hp_fullness = static_cast<float>(unit1->current_hp) / unit1->max_hp;
    unit2->hp_gauge_params.fullness = hp_fullness;
    unit2->hp_gauge_params.fullness_target = hp_fullness;
    
    return 2;
}

EVT_DEFINE_USER_FUNC(unitMagikoopa_shuffle_clones) {
    BattleWorkUnit* units[6];
    int32_t num_units = 0;
    
    for (int32_t i = 0; i < 64; ++i) {
        auto* unit = g_BattleWork->battle_units[i];
        if (unit == nullptr) continue;
        switch (unit->current_kind) {
            case BattleUnitType::MAGIKOOPA:
            case BattleUnitType::MAGIKOOPA_CLONE:
            case BattleUnitType::RED_MAGIKOOPA:
            case BattleUnitType::RED_MAGIKOOPA_CLONE:
            case BattleUnitType::WHITE_MAGIKOOPA:
            case BattleUnitType::WHITE_MAGIKOOPA_CLONE:
            case BattleUnitType::GREEN_MAGIKOOPA:
            case BattleUnitType::GREEN_MAGIKOOPA_CLONE:
                units[num_units++] = unit;
                break;
        }
    }
    
    // Lackadaisical shuffle.
    for (int32_t i = 0; i < 10; ++i) {
        BattleWorkUnit* unit1 = units[ttyd::system::irand(num_units)];
        BattleWorkUnit* unit2 = units[ttyd::system::irand(num_units)];
        if (unit1 == unit2) continue;
        
        float temp;
        temp = unit1->home_position.x;
        unit1->home_position.x = unit2->home_position.x;
        unit2->home_position.x = temp;
        temp = unit1->home_position.z;
        unit1->home_position.z = unit2->home_position.z;
        unit2->home_position.z = temp;
        temp = unit1->position.x;
        unit1->position.x = unit2->position.x;
        unit2->position.x = temp;
        temp = unit1->position.z;
        unit1->position.z = unit2->position.z;
        unit2->position.z = temp;
    }
    
    return 2;
}

EVT_DEFINE_USER_FUNC(unitMagikoopa_get_clones) {
    int32_t units[4];
    int32_t num_units = 0;
    
    for (int32_t i = 0; i < 64; ++i) {
        auto* unit = g_BattleWork->battle_units[i];
        if (unit && (
                unit->current_kind == BattleUnitType::MAGIKOOPA_CLONE ||
                unit->current_kind == BattleUnitType::RED_MAGIKOOPA_CLONE ||
                unit->current_kind == BattleUnitType::WHITE_MAGIKOOPA_CLONE ||
                unit->current_kind == BattleUnitType::GREEN_MAGIKOOPA_CLONE))
            units[num_units++] = i;
    }
    
    for (int32_t i = 0; i < 4; ++i) {
        if (i < num_units) {
            evtSetValue(evt, evt->evtArguments[i], units[i]);
        } else {
            evtSetValue(evt, evt->evtArguments[i], -1);
        }
    }
    evtSetValue(evt, evt->evtArguments[4], num_units);
    
    return 2;
}

EVT_DEFINE_USER_FUNC(unitMagikoopa_pose_lookup) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[0]));
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, unit_idx);
    int32_t offset = 0;
    switch (unit->true_kind) {
        case BattleUnitType::MAGIKOOPA:
        case BattleUnitType::MAGIKOOPA_CLONE:
            offset = 0;
            break;
        case BattleUnitType::RED_MAGIKOOPA:
        case BattleUnitType::RED_MAGIKOOPA_CLONE:
            offset = 1;
            break;
        case BattleUnitType::WHITE_MAGIKOOPA:
        case BattleUnitType::WHITE_MAGIKOOPA_CLONE:
            offset = 2;
            break;
        case BattleUnitType::GREEN_MAGIKOOPA:
        case BattleUnitType::GREEN_MAGIKOOPA_CLONE:
            offset = 3;
            break;
    }

    int32_t lookup = evtGetValue(evt, evt->evtArguments[1]);

    evtSetValue(evt, evt->evtArguments[2], PTR(kPoseLookupTbl[lookup * 4 + offset]));
    return 2;
}

// Evt definitions.

EVT_BEGIN(unitMagikoopa_alpha_down_event)
    IF_EQUAL(LW(0), -1)
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_SetAlpha, LW(0), 1, 254)
    USER_FUNC(btlevtcmd_AnimeFlagOnOff, LW(0), 1, 1, 1)
    WAIT_FRM(1)
    USER_FUNC(evt_sub_intpl_init, 0, 254, 0, 30)
    DO(30)
        USER_FUNC(evt_sub_intpl_get_value_para, LW(1), LW(2))
        USER_FUNC(btlevtcmd_SetAlpha, LW(0), 1, LW(1))
        WAIT_FRM(1)
    WHILE()
    USER_FUNC(btlevtcmd_OnPartsAttribute, LW(0), 1, 0x200'0000)
    USER_FUNC(btlevtcmd_SetAlpha, LW(0), 1, 0)
    USER_FUNC(btlevtcmd_AnimeFlagOnOff, LW(0), 1, 1, 1)
    WAIT_FRM(1)
    RETURN()
EVT_END()

EVT_BEGIN(unitMagikoopa_alpha_up_event)
    IF_EQUAL(LW(0), -1)
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_OffPartsAttribute, LW(0), 1, 0x200'0000)
    USER_FUNC(btlevtcmd_SetAlpha, LW(0), 1, 0)
    USER_FUNC(btlevtcmd_AnimeFlagOnOff, LW(0), 1, 1, 1)
    WAIT_FRM(1)
    USER_FUNC(evt_sub_intpl_init, 0, 0, 254, 30)
    DO(30)
        USER_FUNC(evt_sub_intpl_get_value_para, LW(1), LW(2))
        USER_FUNC(btlevtcmd_SetAlpha, LW(0), 1, LW(1))
        WAIT_FRM(1)
    WHILE()
    USER_FUNC(btlevtcmd_SetAlpha, LW(0), 1, 255)
    USER_FUNC(btlevtcmd_AnimeFlagOnOff, LW(0), 1, 1, 1)
    WAIT_FRM(1)
    RETURN()
EVT_END()

EVT_BEGIN(unitMagikoopa_clone_dead_event)
    USER_FUNC(btlevtcmd_GetUnitId, LW(0), LW(0))
    IF_EQUAL(LW(0), -1)
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_GetUnitWork, LW(0), UW_CloneKillSignalSent, LW(1))
    IF_EQUAL(LW(1), 1)
        GOTO(10)
    END_IF()
    USER_FUNC(btlevtcmd_SetUnitWork, LW(0), UW_CloneKillSignalSent, 1)
    RUN_CHILD_EVT(PTR(&unitMagikoopa_alpha_down_event))
LBL(10)
    USER_FUNC(btlevtcmd_WaitAttackEnd)
    USER_FUNC(btlevtcmd_KillUnit, LW(0), 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitMagikoopa_dead_event)
    USER_FUNC(btlevtcmd_CheckDataOfDataTable, LW(10), LW(0), 50)
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_RunDataEventChild, LW(10), 50)
    END_IF()
    SET(LW(0), 48)
    USER_FUNC(btlevtcmd_OnUnitFlag, LW(10), 0x800'0000)
    USER_FUNC(btlevtcmd_snd_se, LW(10), PTR("SFX_BTL_ENEMY_DIE1_1"), EVT_NULLPTR, 0, EVT_NULLPTR)
LBL(10)
    WAIT_FRM(1)
    USER_FUNC(btlevtcmd_AddRotate, LW(10), 0, -15, 0)
    SUB(LW(0), 1)
    IF_LARGE(LW(0), 0)
        GOTO(10)
    END_IF()
    USER_FUNC(btlevtcmd_SetRotate, LW(10), 0, 0, 0)
    USER_FUNC(btlevtcmd_snd_se, LW(10), PTR("SFX_BTL_ENEMY_DIE1_2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetPos, LW(10), LW(0), LW(1), LW(2))
    ADD(LW(2), 10)
    USER_FUNC(evt_eff, 0, PTR("kemuri_test"), 4, LW(0), LW(1), LW(2), FLOAT(0.8), 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(btlevtcmd_GetCoin, LW(10), LW(0))
    USER_FUNC(btlevtcmd_StoreCoin, LW(0))
    USER_FUNC(btlevtcmd_GetExp, LW(10), LW(0))
    USER_FUNC(btlevtcmd_StoreExp, LW(0))
    USER_FUNC(btlevtcmd_GetPos, LW(10), LW(3), LW(1), LW(2))
    ADD(LW(2), 10)
    ADD(LW(1), 30)
    USER_FUNC(_add_star_point_disp_offset, LW(10), LW(3), LW(1), LW(2))
    USER_FUNC(evt_eff, PTR(""), PTR("star_point"), 0, LW(3), LW(1), LW(2), LW(0), 0, 0, 0, 0, 0, 0, 0)
    IF_LARGE_EQUAL(LW(0), 1)
        DO(LW(0))
        WHILE()
    END_IF()
    USER_FUNC(btlevtcmd_SetRotateOffset, LW(10), 0, 0, 0)
    SET(LW(0), 30)
LBL(20)
    WAIT_FRM(1)
    USER_FUNC(btlevtcmd_AddRotate, LW(10), 3, 0, 0)
    SUB(LW(0), 1)
    IF_LARGE(LW(0), 0)
        GOTO(20)
    END_IF()
    USER_FUNC(btlevtcmd_GetBackItem, LW(10))
    USER_FUNC(btlevtcmd_ClearAllStatus, LW(10))
    USER_FUNC(btlevtcmd_OnAttribute, LW(10), 0x100'0000)
    USER_FUNC(btlevtcmd_OnAttribute, LW(10), 0x2000'0000)
    RUN_CHILD_EVT(PTR(&unitMagikoopa_kill_all_clones_event))
    USER_FUNC(btlevtcmd_CheckDamageCode, LW(10), 1024, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_KillUnit, LW(10), 0)
    ELSE()
        USER_FUNC(btlevtcmd_KillUnit, LW(10), 2)
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitMagikoopa_kill_all_clones_event)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_IsClone, LW(0))
    IF_EQUAL(LW(0), 0)
        USER_FUNC(unitMagikoopa_get_clones, LW(1), LW(2), LW(3), LW(4), LW(0))
        IF_LARGE_EQUAL(LW(0), 1)
            USER_FUNC(btlevtcmd_WaitAttackEnd)
            USER_FUNC(unitMagikoopa_get_clones, LW(1), LW(2), LW(3), LW(4), LW(0))
            SET(LW(0), LW(1))
            RUN_EVT(PTR(&unitMagikoopa_clone_dead_event))
            SET(LW(0), LW(2))
            RUN_EVT(PTR(&unitMagikoopa_clone_dead_event))
            SET(LW(0), LW(3))
            RUN_EVT(PTR(&unitMagikoopa_clone_dead_event))
            SET(LW(0), LW(4))
            RUN_EVT(PTR(&unitMagikoopa_clone_dead_event))
            DO(0)
                USER_FUNC(unitMagikoopa_get_clones, LW(1), LW(2), LW(3), LW(4), LW(0))
                IF_EQUAL(LW(0), 0)
                    DO_BREAK()
                END_IF()
                WAIT_FRM(1)
            WHILE()
        END_IF()
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitMagikoopa_gale_dead_event)
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_CLAUD_BREATH3"), 0)
    USER_FUNC(btlevtcmd_GetBodyId, -2, LW(0))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, LW(0), 39)
    BROTHER_EVT()
LBL(0)
        USER_FUNC(btlevtcmd_AddRotate, -2, 0, 20, 0)
        WAIT_FRM(1)
        GOTO(0)
    END_BROTHER()
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(2.0))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetStageSize, LW(0), EVT_NULLPTR, EVT_NULLPTR)
    ADD(LW(1), 40)
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 0, -10, 2, 0, -1)
    
    // Do not award coins / EXP.
    // USER_FUNC(btlevtcmd_GetCoin, LW(10), LW(0))
    // USER_FUNC(btlevtcmd_StoreCoin, LW(0))
    // USER_FUNC(btlevtcmd_GetExp, LW(10), LW(0))
    // USER_FUNC(btlevtcmd_StoreExp, LW(0))
    
    RUN_CHILD_EVT(PTR(&unitMagikoopa_kill_all_clones_event))
    USER_FUNC(btlevtcmd_KillUnit, -2, 0)
    RETURN()
EVT_END()

// Assumes pose id is in LW(0) and clone ids are in LW(11) through LW(14).
EVT_BEGIN(unitMagikoopa_all_change_pose)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, LW(0))
    IF_NOT_EQUAL(LW(11), -1)
        USER_FUNC(btlevtcmd_AnimeChangePose, LW(11), 1, LW(0))
    END_IF()
    IF_NOT_EQUAL(LW(12), -1)
        USER_FUNC(btlevtcmd_AnimeChangePose, LW(12), 1, LW(0))
    END_IF()
    IF_NOT_EQUAL(LW(13), -1)
        USER_FUNC(btlevtcmd_AnimeChangePose, LW(13), 1, LW(0))
    END_IF()
    IF_NOT_EQUAL(LW(14), -1)
        USER_FUNC(btlevtcmd_AnimeChangePose, LW(14), 1, LW(0))
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitMagikoopa_ground_magic_common_event1)
    BROTHER_EVT()
        DO(2)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_KAMEKU_MAGIC1"), EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_FRM(40)
        WHILE()
    END_BROTHER()
    USER_FUNC(unitMagikoopa_get_clones, LW(11), LW(12), LW(13), LW(14), LW(0))
    IF_EQUAL(LW(15), 0)
        BROTHER_EVT()
            USER_FUNC(unitMagikoopa_eff_staff1, -2)
        END_BROTHER()
        BROTHER_EVT()
            IF_NOT_EQUAL(LW(11), -1)
                USER_FUNC(unitMagikoopa_eff_staff1, LW(11))
            END_IF()
        END_BROTHER()
        BROTHER_EVT()
            IF_NOT_EQUAL(LW(12), -1)
                USER_FUNC(unitMagikoopa_eff_staff1, LW(12))
            END_IF()
        END_BROTHER()
        BROTHER_EVT()
            IF_NOT_EQUAL(LW(13), -1)
                USER_FUNC(unitMagikoopa_eff_staff1, LW(13))
            END_IF()
        END_BROTHER()
        BROTHER_EVT()
            IF_NOT_EQUAL(LW(14), -1)
                USER_FUNC(unitMagikoopa_eff_staff1, LW(14))
            END_IF()
        END_BROTHER()
    ELSE()
        BROTHER_EVT()
            USER_FUNC(unitMagikoopa_eff_staff2, -2)
        END_BROTHER()
    END_IF()
    USER_FUNC(unitMagikoopa_pose_lookup, -2, Pose_A_1A, LW(0))
    RUN_CHILD_EVT(PTR(&unitMagikoopa_all_change_pose))
    WAIT_FRM(60)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_KAMEKU_MAGIC4"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(unitMagikoopa_pose_lookup, -2, Pose_A_1B, LW(0))
    RUN_CHILD_EVT(PTR(&unitMagikoopa_all_change_pose))
    WAIT_FRM(20)
    USER_FUNC(unitMagikoopa_pose_lookup, -2, Pose_A_1C, LW(0))
    RUN_CHILD_EVT(PTR(&unitMagikoopa_all_change_pose))
    WAIT_FRM(30)
    BROTHER_EVT()
        WAIT_FRM(10)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_KAMEKU_MAGIC8"), EVT_NULLPTR, 0, EVT_NULLPTR)
    END_BROTHER()
    USER_FUNC(unitMagikoopa_pose_lookup, -2, Pose_A_1D, LW(0))
    RUN_CHILD_EVT(PTR(&unitMagikoopa_all_change_pose))
    WAIT_FRM(20)
    USER_FUNC(unitMagikoopa_pose_lookup, -2, Pose_A_1E, LW(0))
    RUN_CHILD_EVT(PTR(&unitMagikoopa_all_change_pose))
    RETURN()
EVT_END()

EVT_BEGIN(unitMagikoopa_ground_magic_common_event2)
    BROTHER_EVT()
        DO(2)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_KAMEKU_MAGIC1"), EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_FRM(40)
        WHILE()
    END_BROTHER()
    BROTHER_EVT()
        USER_FUNC(unitMagikoopa_eff_staff1, -2)
    END_BROTHER()
    USER_FUNC(unitMagikoopa_pose_lookup, -2, Pose_A_1A, LW(0))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, LW(0))
    WAIT_FRM(60)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_KAMEKU_MAGIC6"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(unitMagikoopa_pose_lookup, -2, Pose_A_2A, LW(0))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, LW(0))
    WAIT_FRM(40)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_KAMEKU_MAGIC7"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(unitMagikoopa_pose_lookup, -2, Pose_A_2B, LW(0))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, LW(0))
    WAIT_FRM(20)
    USER_FUNC(unitMagikoopa_pose_lookup, -2, Pose_A_2C, LW(0))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, LW(0))
    RETURN()
EVT_END()

EVT_BEGIN(unitMagikoopa_sky_magic_common_event)
    BROTHER_EVT()
        DO(2)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_KAMEKU_MAGIC1"), EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_FRM(40)
        WHILE()
    END_BROTHER()
    USER_FUNC(unitMagikoopa_get_clones, LW(11), LW(12), LW(13), LW(14), LW(0))
    IF_EQUAL(LW(15), 0)
        BROTHER_EVT()
            USER_FUNC(unitMagikoopa_eff_staff1, -2)
        END_BROTHER()
        BROTHER_EVT()
            IF_NOT_EQUAL(LW(11), -1)
                USER_FUNC(unitMagikoopa_eff_staff1, LW(11))
            END_IF()
        END_BROTHER()
        BROTHER_EVT()
            IF_NOT_EQUAL(LW(12), -1)
                USER_FUNC(unitMagikoopa_eff_staff1, LW(12))
            END_IF()
        END_BROTHER()
        BROTHER_EVT()
            IF_NOT_EQUAL(LW(13), -1)
                USER_FUNC(unitMagikoopa_eff_staff1, LW(13))
            END_IF()
        END_BROTHER()
        BROTHER_EVT()
            IF_NOT_EQUAL(LW(14), -1)
                USER_FUNC(unitMagikoopa_eff_staff1, LW(14))
            END_IF()
        END_BROTHER()
    ELSE()
        BROTHER_EVT()
            USER_FUNC(unitMagikoopa_eff_staff2, -2)
        END_BROTHER()
    END_IF()
    USER_FUNC(unitMagikoopa_pose_lookup, -2, Pose_A_3A, LW(0))
    RUN_CHILD_EVT(PTR(&unitMagikoopa_all_change_pose))
    WAIT_FRM(60)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_KAMEKU_MAGIC8"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(unitMagikoopa_pose_lookup, -2, Pose_A_3B, LW(0))
    RUN_CHILD_EVT(PTR(&unitMagikoopa_all_change_pose))
    WAIT_FRM(35)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_KAMEKU_MAGIC7"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(unitMagikoopa_pose_lookup, -2, Pose_A_3C, LW(0))
    RUN_CHILD_EVT(PTR(&unitMagikoopa_all_change_pose))
    WAIT_FRM(15)
    USER_FUNC(unitMagikoopa_pose_lookup, -2, Pose_A_3D, LW(0))
    RUN_CHILD_EVT(PTR(&unitMagikoopa_all_change_pose))
    RETURN()
EVT_END()

EVT_BEGIN(unitMagikoopa_first_attack_pos_event)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    SET(LW(1), 10)
    USER_FUNC(btlevtcmd_SetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_WaitAttackEnd)
    RETURN()
EVT_END()

EVT_BEGIN(unitMagikoopa_fall_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_OffAttribute, -2, 4)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, 0x800)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, 0x20'0000)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, 0x40'0000)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 39)
    WAIT_MSEC(500)

    // Switch table based on enemy type.
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(0))
    SWITCH(LW(0))
        CASE_EQUAL((int32_t)BattleUnitType::MAGIKOOPA)
            USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitMagikoopa_pose_table))
        CASE_EQUAL((int32_t)BattleUnitType::RED_MAGIKOOPA)
            USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitRedMagikoopa_pose_table))
        CASE_EQUAL((int32_t)BattleUnitType::WHITE_MAGIKOOPA)
            USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitWhiteMagikoopa_pose_table))
        CASE_ETC()
            USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitGreenMagikoopa_pose_table))
    END_SWITCH()

    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_IsClone, LW(0))
    IF_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_ChangeDataTable, -2, PTR(&unitMagikoopa_data_table))
    ELSE()
        USER_FUNC(btlevtcmd_ChangeDataTable, -2, PTR(&unitMagikoopaClone_data_table))
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 39)
    INLINE_EVT()
        USER_FUNC(unitMagikoopa_pose_lookup, -2, Pose_H_1, LW(0))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 2, LW(0))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_SetPartsPos, -2, 2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 2, 0x100'0000)
        WAIT_FRM(20)
        USER_FUNC(evt_sub_intpl_init, 0, 255, 0, 30)
        DO(30)
            USER_FUNC(evt_sub_intpl_get_value_para, LW(1), LW(2))
            USER_FUNC(btlevtcmd_SetAlpha, -2, 2, LW(1))
            WAIT_FRM(1)
        WHILE()
        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 2, 0x100'0000)
    END_INLINE()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_FALL1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(1.0))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 10, -1)
    USER_FUNC(btlevtcmd_SetHomePos, -2, LW(0), 0, LW(2))
    WAIT_MSEC(500)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_IsFlying, 0)
    RUN_CHILD_EVT(PTR(&unitMagikoopa_common_init_event2))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 39)
    RETURN()
EVT_END()

EVT_BEGIN(unitMagikoopa_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_KillClones, 0)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_IsClone, LW(0))
    IF_EQUAL(LW(0), 0)
        USER_FUNC(unitMagikoopa_get_clones, LW(1), LW(2), LW(3), LW(4), LW(0))
        IF_LARGE_EQUAL(LW(0), 1)
            USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_KillClones, 1)
        END_IF()
    ELSE()
        SET(LW(0), -2)
        RUN_CHILD_EVT(PTR(&unitMagikoopa_clone_dead_event))
        RETURN()
    END_IF()
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    USER_FUNC(btlevtcmd_GetUnitId, -2, LW(0))
    IF_NOT_EQUAL(LW(0), -1)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_KillClones, LW(0))
        IF_EQUAL(LW(0), 1)
            RUN_CHILD_EVT(PTR(&unitMagikoopa_kill_all_clones_event))
        END_IF()
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitMagikoopa_magic_common_event1)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_IsFlying, LW(0))
    IF_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_GetUnitId, -2, LW(13))
        IF_NOT_EQUAL(LW(3), LW(13))
            RUN_CHILD_EVT(PTR(&unitMagikoopa_ground_magic_common_event1))
        ELSE()
            RUN_CHILD_EVT(PTR(&unitMagikoopa_ground_magic_common_event2))
        END_IF()
    ELSE()
        RUN_CHILD_EVT(PTR(&unitMagikoopa_sky_magic_common_event))
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitMagikoopa_magic_common_event2)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_IsFlying, LW(0))
    IF_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(PTR(&unitMagikoopa_ground_magic_common_event2))
    ELSE()
        RUN_CHILD_EVT(PTR(&unitMagikoopa_sky_magic_common_event))
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitMagikoopa_one_recover_event)
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(9))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(9), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            SET(LW(0), 1)
            RETURN()
        END_IF()
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_GetHp, LW(3), LW(0))
    USER_FUNC(btlevtcmd_GetMaxHp, LW(3), LW(1))
    IF_EQUAL(LW(0), LW(1))
        SET(LW(0), 0)
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    SET(LW(15), 0)
    RUN_CHILD_EVT(PTR(&unitMagikoopa_magic_common_event1))
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_RecoverHp, LW(3), LW(4), 8)
    USER_FUNC(btlevtcmd_GetPartsPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHeight, LW(3), LW(5))
    USER_FUNC(btlevtcmd_GetStatusMg, LW(3), LW(6))
    MULF(LW(5), LW(6))
    ADD(LW(1), LW(5))
    USER_FUNC(evt_eff, PTR(""), PTR("recovery"), 0, LW(0), LW(1), LW(2), 8, 0, 0, 0, 0, 0, 0, 0)
    WAIT_FRM(40)
LBL(99)
    SET(LW(0), 1)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitMagikoopa_all_recover_event)
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(9))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(9), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            SET(LW(0), 1)
            RETURN()
        END_IF()
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
LBL(5)
    USER_FUNC(btlevtcmd_GetHp, LW(3), LW(0))
    USER_FUNC(btlevtcmd_GetMaxHp, LW(3), LW(1))
    IF_SMALL(LW(0), LW(1))
        GOTO(8)
    END_IF()
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    IF_EQUAL(LW(3), -1)  // Bugfix: Original has this condition flipped!
        SET(LW(0), 0)
        RETURN()
    END_IF()
    GOTO(5)
LBL(8)
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    RUN_CHILD_EVT(PTR(&unitMagikoopa_magic_common_event2))
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(9))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(9), LW(3), LW(4))
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
LBL(10)
    BROTHER_EVT()
        USER_FUNC(evt_sub_random, 10, LW(0))
        WAIT_FRM(LW(0))
        USER_FUNC(btlevtcmd_RecoverHp, LW(3), LW(4), 4)
        USER_FUNC(btlevtcmd_GetPartsPos, LW(3), LW(4), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_GetHeight, LW(3), LW(5))
        USER_FUNC(btlevtcmd_GetStatusMg, LW(3), LW(6))
        MULF(LW(5), LW(6))
        ADD(LW(1), LW(5))
        USER_FUNC(evt_eff, PTR(""), PTR("recovery"), 0, LW(0), LW(1), LW(2), 4, 0, 0, 0, 0, 0, 0, 0)
    END_BROTHER()
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    IF_NOT_EQUAL(LW(3), -1)
        GOTO(10)
    END_IF()
    SET(LW(0), 1)
    WAIT_FRM(120)
    RETURN()
EVT_END()

EVT_BEGIN(unitMagikoopa_support_magic_event)
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(9))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(9), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            SET(LW(0), 1)
            RETURN()
        END_IF()
        GOTO(99)
    END_IF()
    IF_EQUAL(LW(15), 10)
        USER_FUNC(btlevtcmd_CheckStatus, LW(3), 10, LW(0))
    END_IF()
    IF_EQUAL(LW(15), 14)
        USER_FUNC(btlevtcmd_CheckStatus, LW(3), 14, LW(0))
    END_IF()
    IF_EQUAL(LW(15), 6)
        USER_FUNC(btlevtcmd_CheckStatus, LW(3), 6, LW(0))
    END_IF()
    IF_EQUAL(LW(15), 18)
        USER_FUNC(btlevtcmd_CheckStatus, LW(3), 18, LW(0))
    END_IF()
    IF_EQUAL(LW(0), 1)
        SET(LW(0), 0)
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    SET(LW(15), 0)
    RUN_CHILD_EVT(PTR(&unitMagikoopa_magic_common_event1))
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    WAIT_FRM(30)
LBL(99)
    SET(LW(0), 1)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitMagikoopa_shot_miss_event)
    IF_EQUAL(LW(5), 3)
        USER_FUNC(btlevtcmd_StartAvoid, LW(3), 38)
    END_IF()
    IF_EQUAL(LW(5), 6)
        USER_FUNC(btlevtcmd_StartAvoid, LW(3), 39)
    END_IF()
    IF_EQUAL(LW(5), 2)
        USER_FUNC(btlevtcmd_StartAvoid, LW(3), 40)
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitMagikoopa_incoming_miss_event)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_IsClone, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        SET(LW(0), -2)
        RUN_CHILD_EVT(PTR(&unitMagikoopa_clone_dead_event))
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitMagikoopa_attack_magic_sub_hit_event)
    IF_EQUAL(LW(14), -1)
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_GetPos, LW(14), LW(0), LW(1), LW(2))
    ADD(LW(1), 10)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(10), LW(11), LW(12))
    USER_FUNC(unitMagikoopa_eff_magic, LW(14), LW(10), LW(11), LW(12), 45)
    RETURN()
EVT_END()

EVT_BEGIN(unitMagikoopa_attack_magic_sub_nohit_event)
    IF_EQUAL(LW(14), -1)
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_GetPos, LW(14), LW(0), LW(1), LW(2))
    ADD(LW(1), 10)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(10), LW(11), LW(12))
    USER_FUNC(btlevtcmd_GetStageSize, LW(10), EVT_NULLPTR, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetFaceDirection, LW(14), LW(0))
    MUL(LW(10), LW(0))
    SUB(LW(11), 5)
    USER_FUNC(unitMagikoopa_eff_magic, LW(14), LW(10), LW(11), LW(12), 60)
    RETURN()
EVT_END()

EVT_BEGIN(unitMagikoopa_clone_spawn_event)
    // Switch table based on enemy type.
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(15))
    SWITCH(LW(15))
        CASE_EQUAL((int32_t)BattleUnitType::MAGIKOOPA)
            USER_FUNC(btlevtcmd_SpawnUnit, LW(3), PTR(&unitMagikoopaClone_spawn_entry), 0)
        CASE_EQUAL((int32_t)BattleUnitType::RED_MAGIKOOPA)
            USER_FUNC(btlevtcmd_SpawnUnit, LW(3), PTR(&unitRedMagikoopaClone_spawn_entry), 0)
        CASE_EQUAL((int32_t)BattleUnitType::WHITE_MAGIKOOPA)
            USER_FUNC(btlevtcmd_SpawnUnit, LW(3), PTR(&unitWhiteMagikoopaClone_spawn_entry), 0)
        CASE_ETC()
            USER_FUNC(btlevtcmd_SpawnUnit, LW(3), PTR(&unitGreenMagikoopaClone_spawn_entry), 0)
    END_SWITCH()

    USER_FUNC(btlevtcmd_SetAlpha, LW(3), 1, 0)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_IsFlying, LW(10))
    USER_FUNC(btlevtcmd_SetUnitWork, LW(3), UW_IsFlying, LW(10))
    WAIT_FRM(2)
    USER_FUNC(btlevtcmd_SetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetHomePos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetUnitId, -2, LW(13))
    USER_FUNC(unitMagikoopa_copy_status, LW(13), LW(3))
    USER_FUNC(btlevtcmd_OffAttribute, LW(3), 0x200'0000)
    USER_FUNC(btlevtcmd_SetUnitWork, LW(3), UW_IsClone, 1)
    USER_FUNC(btlevtcmd_SetEventAttack, LW(3), PTR(&unitMagikoopa_attack_event))
    USER_FUNC(btlevtcmd_SetRegistStatus, LW(3), PTR(&unitMagikoopaClone_status))
    USER_FUNC(btlevtcmd_OnPartsAttribute, LW(3), 1, 0x4000)
    RETURN()
EVT_END()

EVT_BEGIN(unitMagikoopa_create_clones_event)
    SET(LW(3), -2)
    SET(LW(4), 1)
    SET(LW(15), 1)
    RUN_CHILD_EVT(PTR(&unitMagikoopa_magic_common_event1))
    USER_FUNC(evt_btl_camera_set_mode, 0, 3)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_KAMEKU_OFFSHOOT1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    SET(LW(0), -2)
    RUN_CHILD_EVT(PTR(&unitMagikoopa_alpha_down_event))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    DO(0)
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 40)
        SET(LW(4), LW(0))
        IF_LARGE(LW(4), 0)
            SUB(LW(2), 10)
            USER_FUNC(btlevtcmd_CheckSpace, LW(3), LW(0), LW(1), LW(2), 40, 0, 0, 0)
            IF_EQUAL(LW(3), -1)
                RUN_CHILD_EVT(PTR(&unitMagikoopa_clone_spawn_event))
            END_IF()
        ELSE()
            DO_BREAK()
        END_IF()
    WHILE()
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    DO(0)
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 40)
        IF_SMALL_EQUAL(LW(0), 170)
            ADD(LW(2), 10)
            USER_FUNC(btlevtcmd_CheckSpace, LW(3), LW(0), LW(1), LW(2), 40, 0, 0, 0)
            IF_EQUAL(LW(3), -1)
                RUN_CHILD_EVT(PTR(&unitMagikoopa_clone_spawn_event))
            END_IF()
        ELSE()
            DO_BREAK()
        END_IF()
    WHILE()
    USER_FUNC(unitMagikoopa_shuffle_clones)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_KAMEKU_OFFSHOOT2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    SET(LW(0), -2)
    RUN_EVT(PTR(&unitMagikoopa_alpha_up_event))
    USER_FUNC(unitMagikoopa_get_clones, LW(1), LW(2), LW(3), LW(4), LW(0))
    SET(LW(0), LW(1))
    RUN_EVT(PTR(&unitMagikoopa_alpha_up_event))
    SET(LW(0), LW(2))
    RUN_EVT(PTR(&unitMagikoopa_alpha_up_event))
    SET(LW(0), LW(3))
    RUN_EVT(PTR(&unitMagikoopa_alpha_up_event))
    SET(LW(0), LW(4))
    RUN_EVT(PTR(&unitMagikoopa_alpha_up_event))
    WAIT_FRM(60)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitMagikoopa_attack_magic_event)
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
    SET(LW(15), 0)
    RUN_CHILD_EVT(PTR(&unitMagikoopa_magic_common_event1))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_KAMEKU_MAGIC3_2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), LW(9), 0, LW(5))
    SWITCH(LW(5))
        CASE_OR(4)
            GOTO(90)
            CASE_END()
        CASE_EQUAL(3)
            GOTO(90)
            CASE_END()
        CASE_EQUAL(6)
            GOTO(90)
        CASE_EQUAL(2)
            GOTO(90)
        CASE_EQUAL(1)
            GOTO(91)
            CASE_END()
        CASE_ETC()
            GOTO(98)
            CASE_END()
LBL(90)
            BROTHER_EVT()
                WAIT_FRM(1)
                DO(0)
                    USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(0))
                    IF_EQUAL(LW(0), -1)
                        USER_FUNC(unitMagikoopa_eff_magic_xpos, -2, LW(0))
                        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(6), LW(7), LW(8))
                        IF_SMALL_EQUAL(LW(0), LW(6))
                            RUN_CHILD_EVT(PTR(&unitMagikoopa_shot_miss_event))
                            DO_BREAK()
                        END_IF()
                    ELSE()
                        USER_FUNC(unitMagikoopa_eff_magic_xpos, -2, LW(0))
                        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(6), LW(7), LW(8))
                        IF_LARGE_EQUAL(LW(0), LW(6))
                            RUN_CHILD_EVT(PTR(&unitMagikoopa_shot_miss_event))
                            DO_BREAK()
                        END_IF()
                    END_IF()
                    WAIT_FRM(1)
                WHILE()
            END_BROTHER()
            USER_FUNC(unitMagikoopa_get_clones, LW(6), LW(7), LW(8), LW(13), LW(0))
            SET(LW(14), -2)
            RUN_CHILD_EVT(PTR(&unitMagikoopa_attack_magic_sub_nohit_event))
            SET(LW(14), LW(6))
            RUN_CHILD_EVT(PTR(&unitMagikoopa_attack_magic_sub_nohit_event))
            SET(LW(14), LW(7))
            RUN_CHILD_EVT(PTR(&unitMagikoopa_attack_magic_sub_nohit_event))
            SET(LW(14), LW(8))
            RUN_CHILD_EVT(PTR(&unitMagikoopa_attack_magic_sub_nohit_event))
            SET(LW(14), LW(13))
            RUN_CHILD_EVT(PTR(&unitMagikoopa_attack_magic_sub_nohit_event))
            WAIT_FRM(60)
            GOTO(98)
LBL(91)
            USER_FUNC(unitMagikoopa_get_clones, LW(6), LW(7), LW(8), LW(13), LW(0))
            SET(LW(14), -2)
            RUN_CHILD_EVT(PTR(&unitMagikoopa_attack_magic_sub_hit_event))
            SET(LW(14), LW(6))
            RUN_CHILD_EVT(PTR(&unitMagikoopa_attack_magic_sub_hit_event))
            SET(LW(14), LW(7))
            RUN_CHILD_EVT(PTR(&unitMagikoopa_attack_magic_sub_hit_event))
            SET(LW(14), LW(8))
            RUN_CHILD_EVT(PTR(&unitMagikoopa_attack_magic_sub_hit_event))
            SET(LW(14), LW(13))
            RUN_CHILD_EVT(PTR(&unitMagikoopa_attack_magic_sub_hit_event))
            WAIT_FRM(45)
            USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
            USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
            GOTO(98)
LBL(98)
            WAIT_FRM(30)
            USER_FUNC(unitMagikoopa_get_clones, LW(6), LW(7), LW(8), LW(9), LW(0))
            USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 43)
            IF_NOT_EQUAL(LW(6), -1)
                USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(6), 1, 43)
            END_IF()
            IF_NOT_EQUAL(LW(7), -1)
                USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(7), 1, 43)
            END_IF()
            IF_NOT_EQUAL(LW(8), -1)
                USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(8), 1, 43)
            END_IF()
            IF_NOT_EQUAL(LW(9), -1)
                USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(9), 1, 43)
            END_IF()
LBL(99)
            USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
            RETURN()
EVT_END()

EVT_BEGIN(unitMagikoopa_huge_magic_event)
    SET(LW(15), 10)
    RUN_CHILD_EVT(PTR(&unitMagikoopa_support_magic_event))
    RETURN()
EVT_END()

EVT_BEGIN(unitMagikoopa_def_magic_event)
    SET(LW(15), 14)
    RUN_CHILD_EVT(PTR(&unitMagikoopa_support_magic_event))
    RETURN()
EVT_END()

EVT_BEGIN(unitMagikoopa_electric_magic_event)
    SET(LW(15), 6)
    RUN_CHILD_EVT(PTR(&unitMagikoopa_support_magic_event))
    RETURN()
EVT_END()

EVT_BEGIN(unitMagikoopa_invis_magic_event)
    SET(LW(15), 18)
    RUN_CHILD_EVT(PTR(&unitMagikoopa_support_magic_event))
    RETURN()
EVT_END()

EVT_BEGIN(unitMagikoopa_attack_event)
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        SET(LW(9), PTR(&unitMagikoopa_weaponAttack))
        RUN_CHILD_EVT(PTR(&unitMagikoopa_attack_magic_event))
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_IsClone, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_EnemyItemUseCheck, -2, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(LW(0))
        GOTO(99)
    END_IF()
    USER_FUNC(evtTot_CheckNumEnemiesRemaining, LW(0))
    IF_EQUAL(LW(0), 1)
LBL(10)
        // Change weighting to use DrawLots.
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(0))
        SWITCH(LW(0))
            CASE_OR((int32_t)BattleUnitType::MAGIKOOPA)
            CASE_OR((int32_t)BattleUnitType::WHITE_MAGIKOOPA)
                USER_FUNC(btlevtcmd_DrawLots, LW(0), 3, 40, 20, 40)
                CASE_END()
            CASE_ETC()
                USER_FUNC(btlevtcmd_DrawLots, LW(0), 3, 40, 0, 40)
        END_SWITCH()
        
        SWITCH(LW(0))
            CASE_EQUAL(0)
                SET(LW(9), PTR(&unitMagikoopa_weaponAttack))
                RUN_CHILD_EVT(PTR(&unitMagikoopa_attack_magic_event))
            CASE_EQUAL(1)
                SET(LW(9), PTR(&unitMagikoopa_weaponRecoverOne))
                RUN_CHILD_EVT(PTR(&unitMagikoopa_one_recover_event))
                IF_EQUAL(LW(0), 0)
                    GOTO(10)
                END_IF()
            CASE_ETC()
                RUN_CHILD_EVT(PTR(&unitMagikoopa_create_clones_event))
        END_SWITCH()
        GOTO(99)
        
    ELSE()
        USER_FUNC(unitMagikoopa_get_clones, LW(1), LW(2), LW(3), LW(4), LW(0))
        IF_LARGE_EQUAL(LW(0), 1)
            USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
            IF_NOT_EQUAL(LW(0), 0)
                RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
                GOTO(99)
            END_IF()
            SET(LW(9), PTR(&unitMagikoopa_weaponAttack))
            RUN_CHILD_EVT(PTR(&unitMagikoopa_attack_magic_event))
            GOTO(99)
        ELSE()
LBL(20)
            // Change weighting to use DrawLots.
            USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(0))
            SWITCH(LW(0))
                CASE_EQUAL((int32_t)BattleUnitType::MAGIKOOPA)
                    USER_FUNC(btlevtcmd_DrawLots, LW(0), 7, 50, 10, 10, 10, 10, 10, 10)
                CASE_EQUAL((int32_t)BattleUnitType::RED_MAGIKOOPA)
                    USER_FUNC(btlevtcmd_DrawLots, LW(0), 7, 50, 20, 20, 0, 0, 0, 0)
                CASE_EQUAL((int32_t)BattleUnitType::WHITE_MAGIKOOPA)
                    USER_FUNC(btlevtcmd_DrawLots, LW(0), 7, 50, 0, 0, 0, 0, 20, 20)
                CASE_ETC()
                    USER_FUNC(btlevtcmd_DrawLots, LW(0), 7, 50, 0, 0, 20, 20, 0, 0)
            END_SWITCH()
            
            SWITCH(LW(0))
                CASE_EQUAL(1)
                    SET(LW(9), PTR(&unitMagikoopa_weaponHuge))
                    RUN_CHILD_EVT(PTR(&unitMagikoopa_huge_magic_event))
                CASE_EQUAL(2)
                    SET(LW(9), PTR(&unitMagikoopa_weaponDef))
                    RUN_CHILD_EVT(PTR(&unitMagikoopa_def_magic_event))
                CASE_EQUAL(3)
                    SET(LW(9), PTR(&unitMagikoopa_weaponElectric))
                    RUN_CHILD_EVT(PTR(&unitMagikoopa_electric_magic_event))
                CASE_EQUAL(4)
                    SET(LW(9), PTR(&unitMagikoopa_weaponInvis))
                    RUN_CHILD_EVT(PTR(&unitMagikoopa_invis_magic_event))
                CASE_EQUAL(5)
                    SET(LW(9), PTR(&unitMagikoopa_weaponRecoverOne))
                    RUN_CHILD_EVT(PTR(&unitMagikoopa_one_recover_event))
                CASE_EQUAL(6)
                    SET(LW(9), PTR(&unitMagikoopa_weaponRecoverAll))
                    RUN_CHILD_EVT(PTR(&unitMagikoopa_all_recover_event))
                CASE_ETC()
                    SET(LW(9), PTR(&unitMagikoopa_weaponAttack))
                    RUN_CHILD_EVT(PTR(&unitMagikoopa_attack_magic_event))
                    SET(LW(0), 1)
            END_SWITCH()

            // If move was unsuccessful, keep picking random moves.
            IF_EQUAL(LW(0), 0)
                GOTO(20)
            END_IF()
            GOTO(99)
            
        END_IF()
    END_IF()
LBL(99)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitMagikoopa_wait_event)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_IsClone, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        RETURN()
    END_IF()
    USER_FUNC(unitMagikoopa_get_clones, LW(1), LW(2), LW(3), LW(4), LW(0))
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    IF_NOT_EQUAL(LW(1), -1)
        USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, LW(1), 1)
    END_IF()
    IF_NOT_EQUAL(LW(2), -1)
        USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, LW(2), 1)
    END_IF()
    IF_NOT_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, LW(3), 1)
    END_IF()
    IF_NOT_EQUAL(LW(4), -1)
        USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, LW(4), 1)
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitMagikoopa_common_init_event2)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitMagikoopa_wait_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitMagikoopa_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitMagikoopa_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitMagikoopa_attack_event))
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_IsFlying, LW(0))
    IF_NOT_EQUAL(LW(0), 0)

        // Switch table based on enemy type.
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(0))
        SWITCH(LW(0))
            CASE_EQUAL((int32_t)BattleUnitType::MAGIKOOPA)
                USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitMagikoopa_flying_pose_table))
            CASE_EQUAL((int32_t)BattleUnitType::RED_MAGIKOOPA)
                USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitRedMagikoopa_flying_pose_table))
            CASE_EQUAL((int32_t)BattleUnitType::WHITE_MAGIKOOPA)
                USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitWhiteMagikoopa_flying_pose_table))
            CASE_ETC()
                USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitGreenMagikoopa_flying_pose_table))
        END_SWITCH()

        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_IsClone, LW(0))
        IF_EQUAL(LW(0), 0)
            USER_FUNC(btlevtcmd_ChangeDataTable, -2, PTR(&unitMagikoopa_flying_data_table))
        ELSE()
            USER_FUNC(btlevtcmd_ChangeDataTable, -2, PTR(&unitMagikoopaClone_flying_data_table))
        END_IF()
        USER_FUNC(btlevtcmd_OnAttribute, -2, 4)
        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 1, 0x800)
        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 1, 0x20'0000)
        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 1, 0x40'0000)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_SetPos, -2, LW(0), 40, LW(2))
        USER_FUNC(btlevtcmd_SetHomePos, -2, LW(0), 40, LW(2))
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitMagikoopa_common_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_IsClone, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_CloneKillSignalSent, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Eff, 0)
    RUN_CHILD_EVT(PTR(&unitMagikoopa_common_init_event2))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitMagikoopa_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::MAGIKOOPA)
    RUN_CHILD_EVT(unitMagikoopa_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitRedMagikoopa_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::RED_MAGIKOOPA)
    RUN_CHILD_EVT(unitMagikoopa_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitWhiteMagikoopa_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::WHITE_MAGIKOOPA)
    RUN_CHILD_EVT(unitMagikoopa_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitGreenMagikoopa_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::GREEN_MAGIKOOPA)
    RUN_CHILD_EVT(unitMagikoopa_common_init_event)
    RETURN()
EVT_END()

}  // namespace mod::tot::custom