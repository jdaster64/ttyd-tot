#include "tot_custom_rel.h"     // For externed unit definitions.

#include "common_functions.h"
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
#include <ttyd/eff_mahorn2.h>
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
using ::ttyd::evtmgr_cmd::evtSetFloat;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

// Constants (UW, etc.)
constexpr const int32_t UW_HandPtrs = 0;
constexpr const int32_t UW_HandPtrsMax = UW_HandPtrs + 4;
constexpr const int32_t UW_IsClone = 4;
constexpr const int32_t UW_KillClones = 5;
constexpr const int32_t UW_CloneKillSignalSent = 6;
constexpr const int32_t UW_MagicEff = 7;
constexpr const int32_t UW_BattleUnitType = 15;

}  // namespace

// Evt / Function declarations.

extern const int32_t unitWizzerd_init_event[];
extern const int32_t unitDarkWizzerd_init_event[];
extern const int32_t unitEliteWizzerd_init_event[];
extern const int32_t unitWizzerd_common_init_event[];
extern const int32_t unitWizzerd_attack_event[];
extern const int32_t unitWizzerd_damage_event[];
extern const int32_t unitWizzerd_wait_event[];
extern const int32_t unitWizzerd_punch_attack_event[];
extern const int32_t unitWizzerd_laser_attack_event[];
extern const int32_t unitWizzerd_confuse_attack_event[];
extern const int32_t unitWizzerd_thunder_attack_event[];
extern const int32_t unitWizzerd_status_magic_event[];
extern const int32_t unitWizzerd_def_magic_event[];
extern const int32_t unitWizzerd_dodgy_magic_event[];
extern const int32_t unitWizzerd_invis_magic_event[];
extern const int32_t unitWizzerd_recover_magic_event[];
extern const int32_t unitWizzerd_attack_common_event[];
extern const int32_t unitWizzerd_attack_common_event_sub_nohit[];
extern const int32_t unitWizzerd_attack_common_event_sub_hit[];
extern const int32_t unitWizzerd_attack_common_event_sub1[];
extern const int32_t unitWizzerd_attack_common_event_sub2[];
extern const int32_t unitWizzerd_attack_all_common_event[];
extern const int32_t unitWizzerd_support_common_event2[];
extern const int32_t unitWizzerd_clone_spawn_event[];
extern const int32_t unitWizzerd_create_clones_event[];
extern const int32_t unitWizzerd_alpha_up_event[];
extern const int32_t unitWizzerd_alpha_down_event[];
extern const int32_t unitWizzerd_clone_dead_event[];
extern const int32_t unitWizzerd_kill_all_clones_event[];
extern const int32_t unitWizzerd_dead_event[];
extern const int32_t unitWizzerd_gale_dead_event[];
extern const int32_t unitWizzerd_incoming_miss_event[];
extern const int32_t unitWizzerd_shot_miss_event[];
extern const int32_t unitWizzerd_unitWizzerd_HandMain_event[];
extern const int32_t unitWizzerdClone_attack_event[];

EVT_DECLARE_USER_FUNC(unitWizzerd_ShuffleClones, 0)
EVT_DECLARE_USER_FUNC(unitWizzerd_CopyStatus, 2)
EVT_DECLARE_USER_FUNC(unitWizzerd_EffMagicGetXpos, 2)
EVT_DECLARE_USER_FUNC(unitWizzerd_EffMagic, 5)
EVT_DECLARE_USER_FUNC(unitWizzerd_GetClones, 5)
EVT_DECLARE_USER_FUNC(unitWizzerd_SetHandDest, 5)
EVT_DECLARE_USER_FUNC(unitWizzerd_HandMain, 1)
EVT_DECLARE_USER_FUNC(unitWizzerd_HandEnd, 1)
EVT_DECLARE_USER_FUNC(unitWizzerd_HandInit, 1)

// Unit data.

int8_t unitWizzerd_defense[] = { 3, 3, 3, 3, 3 };
int8_t unitWizzerd_defense_attr[] = { 0, 0, 0, 0, 0 };

StatusVulnerability unitWizzerd_status = {
     10,  50,  90, 100,  40, 100, 100,  50,
    100,  85, 100,  85, 100,  90,  40,  30,
     85, 100,  40, 100, 100,  75,
};
StatusVulnerability unitDarkWizzerd_status = {
     10,  50,  90, 100,  40, 100, 100,  50,
    100,  85, 100,  85, 100,  90,  40,  30,
     85, 100,  40, 100, 100,  75,
};
StatusVulnerability unitEliteWizzerd_status = {
      0,  30,  80, 100,  20, 100, 100,  40,
    100,  75, 100,  75, 100,  80,  20,  10,
     75, 100,  20, 100, 100,  60,
};
StatusVulnerability unitWizzerdClone_status = {
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
};

PoseTableEntry unitWizzerd_pose_table[] = {
    1, "MAH_N_1",
    2, "MAH_Y_1",
    9, "MAH_Y_1",
    5, "MAH_K_1",
    4, "MAH_X_1",
    3, "MAH_X_1",
    28, "MAH_S_1",
    29, "MAH_Q_1",
    30, "MAH_Q_1",
    31, "MAH_A_1",
    39, "MAH_D_1",
    40, "MAH_W_1",
    42, "MAH_R_1",
    56, "MAH_X_1",
    57, "MAH_X_1",
    65, "MAH_T_1",
    69, "MAH_S_1",
};

DataTableEntry unitWizzerd_data_table[] = {
    46, (void*)unitWizzerd_incoming_miss_event,
    48, (void*)btldefaultevt_Dummy,
    0, nullptr,
};
DataTableEntry unitDarkEliteWizzerd_data_table[] = {
    49, (void*)unitWizzerd_dead_event,
    58, (void*)unitWizzerd_gale_dead_event,
    46, (void*)unitWizzerd_incoming_miss_event,
    48, (void*)btldefaultevt_Dummy,
    0, nullptr,
};
DataTableEntry unitWizzerdClone_data_table[] = {
    46, (void*)unitWizzerd_incoming_miss_event,
    48, (void*)btldefaultevt_Dummy,
    0, nullptr,
};

BattleWeapon unitWizzerd_weaponPunch = {
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
    .damage_function_params = { 6, 0, 0, 0, 0, 0, 0, 0 },
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
        AttackCounterResistance_Flags::TOP_SPIKY |
        // Added to make sure that front-spiky counter functions properly.
        AttackCounterResistance_Flags::PREEMPTIVE_SPIKY,
    .target_weighting_flags = AttackTargetWeighting_Flags::PREFER_FRONT,
        
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
BattleWeapon unitWizzerd_weaponLaser = {
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
        AttackSpecialProperty_Flags::DEFENSE_PIERCING,
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
BattleWeapon unitWizzerd_weaponConfuse = {
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
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::FREEZE_BREAK,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = 
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM,
        
    // status chances
    .confuse_chance = 50,
    .confuse_time = 3,
    
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
BattleWeapon unitWizzerd_weaponThunder = {
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
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::ELECTRIC,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags = AttackSpecialProperty_Flags::ALL_BUFFABLE,
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
BattleWeapon unitWizzerd_weaponHuge = {
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
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM,
        
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
BattleWeapon unitWizzerd_weaponDef = {
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
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM,
        
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
BattleWeapon unitWizzerd_weaponDodgy = {
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
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM,
        
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
BattleWeapon unitWizzerd_weaponInvis = {
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
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM,
        
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
BattleWeapon unitWizzerd_weaponRecover = {
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

BattleUnitKindPart unitWizzerd_parts[] = {
    {
        .index = 1,
        .name = "btl_un_mahorn",
        .model_name = "c_maho",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 30.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 40.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitWizzerd_defense,
        .defense_attr = unitWizzerd_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitWizzerd_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_mahorn",
        .model_name = "c_maho",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 5.0f, 30.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 35.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitWizzerd_defense,
        .defense_attr = unitWizzerd_defense_attr,
        .attribute_flags =
            0x0000'0009 |
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_200_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitWizzerd_pose_table,
    },
    {
        .index = 3,
        .name = "btl_un_mahorn",
        .model_name = "c_maho",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 5.0f, 30.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 35.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitWizzerd_defense,
        .defense_attr = unitWizzerd_defense_attr,
        .attribute_flags =
            0x0000'0009 |
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_200_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitWizzerd_pose_table,
    },
    {
        .index = 4,
        .name = "btl_un_mahorn",
        .model_name = "c_maho",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 5.0f, 30.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 35.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitWizzerd_defense,
        .defense_attr = unitWizzerd_defense_attr,
        .attribute_flags =
            0x0000'0009 |
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_200_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitWizzerd_pose_table,
    },
    {
        .index = 5,
        .name = "btl_un_mahorn",
        .model_name = "c_maho",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 5.0f, 30.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 35.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitWizzerd_defense,
        .defense_attr = unitWizzerd_defense_attr,
        .attribute_flags =
            0x0000'0009 |
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_200_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitWizzerd_pose_table,
    },
};
BattleUnitKindPart unitDarkWizzerd_parts[] = {
    {
        .index = 1,
        .name = "btl_un_super_mahorn",
        .model_name = "c_maho_b",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 30.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 40.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitWizzerd_defense,
        .defense_attr = unitWizzerd_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitWizzerd_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_super_mahorn",
        .model_name = "c_maho_b",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 5.0f, 30.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 35.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitWizzerd_defense,
        .defense_attr = unitWizzerd_defense_attr,
        .attribute_flags =
            0x0000'0009 |
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_200_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitWizzerd_pose_table,
    },
    {
        .index = 3,
        .name = "btl_un_super_mahorn",
        .model_name = "c_maho_b",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 5.0f, 30.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 35.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitWizzerd_defense,
        .defense_attr = unitWizzerd_defense_attr,
        .attribute_flags =
            0x0000'0009 |
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_200_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitWizzerd_pose_table,
    },
    {
        .index = 4,
        .name = "btl_un_super_mahorn",
        .model_name = "c_maho_b",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 5.0f, 30.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 35.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitWizzerd_defense,
        .defense_attr = unitWizzerd_defense_attr,
        .attribute_flags =
            0x0000'0009 |
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_200_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitWizzerd_pose_table,
    },
    {
        .index = 5,
        .name = "btl_un_super_mahorn",
        .model_name = "c_maho_b",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 5.0f, 30.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 35.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitWizzerd_defense,
        .defense_attr = unitWizzerd_defense_attr,
        .attribute_flags =
            0x0000'0009 |
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_200_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitWizzerd_pose_table,
    },
};
BattleUnitKindPart unitEliteWizzerd_parts[] = {
    {
        .index = 1,
        .name = "btl_un_mahorn_custom",
        .model_name = "c_maho_w",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 30.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 40.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitWizzerd_defense,
        .defense_attr = unitWizzerd_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitWizzerd_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_mahorn_custom",
        .model_name = "c_maho_w",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 5.0f, 30.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 35.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitWizzerd_defense,
        .defense_attr = unitWizzerd_defense_attr,
        .attribute_flags =
            0x0000'0009 |
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_200_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitWizzerd_pose_table,
    },
    {
        .index = 3,
        .name = "btl_un_mahorn_custom",
        .model_name = "c_maho_w",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 5.0f, 30.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 35.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitWizzerd_defense,
        .defense_attr = unitWizzerd_defense_attr,
        .attribute_flags =
            0x0000'0009 |
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_200_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitWizzerd_pose_table,
    },
    {
        .index = 4,
        .name = "btl_un_mahorn_custom",
        .model_name = "c_maho_w",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 5.0f, 30.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 35.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitWizzerd_defense,
        .defense_attr = unitWizzerd_defense_attr,
        .attribute_flags =
            0x0000'0009 |
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_200_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitWizzerd_pose_table,
    },
    {
        .index = 5,
        .name = "btl_un_mahorn_custom",
        .model_name = "c_maho_w",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 5.0f, 30.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 35.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitWizzerd_defense,
        .defense_attr = unitWizzerd_defense_attr,
        .attribute_flags =
            0x0000'0009 |
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_200_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitWizzerd_pose_table,
    },
};

BattleUnitKind unit_Wizzerd = {
    .unit_type = BattleUnitType::WIZZERD,
    .unit_name = "btl_un_mahorn",
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
    .width = 48,
    .height = 44,
    .hit_offset = { 0, 44 },
    .center_offset = { 0.0f, 22.0f, 0.0f },
    .hp_gauge_offset = { 0, -9 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 24.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 24.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 16.0f, 28.6f, 10.0f },
    .cut_base_offset = { 0.0f, 22.0f, 0.0f },
    .cut_width = 48.0f,
    .cut_height = 44.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 0,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_MAHO_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::UNQUAKEABLE,
    .status_vulnerability = &unitWizzerd_status,
    .num_parts = 5,
    .parts = unitWizzerd_parts,
    .init_evt_code = (void*)unitWizzerd_init_event,
    .data_table = unitWizzerd_data_table,
};
BattleUnitKind unit_DarkWizzerd = {
    .unit_type = BattleUnitType::DARK_WIZZERD,
    .unit_name = "btl_un_super_mahorn",
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
    .width = 48,
    .height = 44,
    .hit_offset = { 0, 44 },
    .center_offset = { 0.0f, 22.0f, 0.0f },
    .hp_gauge_offset = { 0, -9 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 24.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 24.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 16.0f, 28.6f, 10.0f },
    .cut_base_offset = { 0.0f, 22.0f, 0.0f },
    .cut_width = 48.0f,
    .cut_height = 44.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 0,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_MAHO_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::UNQUAKEABLE,
    .status_vulnerability = &unitDarkWizzerd_status,
    .num_parts = 5,
    .parts = unitDarkWizzerd_parts,
    .init_evt_code = (void*)unitDarkWizzerd_init_event,
    .data_table = unitDarkEliteWizzerd_data_table,
};
BattleUnitKind unit_DarkWizzerdClone = {
    .unit_type = BattleUnitType::DARK_WIZZERD_CLONE,
    .unit_name = "btl_un_super_mahorn_bunsin",
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
    .width = 48,
    .height = 44,
    .hit_offset = { 0, 44 },
    .center_offset = { 0.0f, 22.0f, 0.0f },
    .hp_gauge_offset = { 0, -9 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 24.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 24.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 16.0f, 28.6f, 10.0f },
    .cut_base_offset = { 0.0f, 22.0f, 0.0f },
    .cut_width = 48.0f,
    .cut_height = 44.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 0,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_MAHO_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::UNQUAKEABLE,
    .status_vulnerability = &unitWizzerdClone_status,
    .num_parts = 5,
    .parts = unitDarkWizzerd_parts,
    .init_evt_code = (void*)unitDarkWizzerd_init_event,
    .data_table = unitWizzerdClone_data_table,
};
BattleUnitKind unit_EliteWizzerd = {
    .unit_type = BattleUnitType::ELITE_WIZZERD,
    .unit_name = "btl_un_mahorn_custom",
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
    .width = 48,
    .height = 44,
    .hit_offset = { 0, 44 },
    .center_offset = { 0.0f, 22.0f, 0.0f },
    .hp_gauge_offset = { 0, -9 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 24.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 24.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 16.0f, 28.6f, 10.0f },
    .cut_base_offset = { 0.0f, 22.0f, 0.0f },
    .cut_width = 48.0f,
    .cut_height = 44.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 0,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_MAHO_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::UNQUAKEABLE,
    .status_vulnerability = &unitEliteWizzerd_status,
    .num_parts = 5,
    .parts = unitEliteWizzerd_parts,
    .init_evt_code = (void*)unitEliteWizzerd_init_event,
    .data_table = unitDarkEliteWizzerd_data_table,
};
BattleUnitKind unit_EliteWizzerdClone = {
    .unit_type = BattleUnitType::ELITE_WIZZERD_CLONE,
    .unit_name = "btl_un_mahorn_custom_bunsin",
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
    .width = 48,
    .height = 44,
    .hit_offset = { 0, 44 },
    .center_offset = { 0.0f, 22.0f, 0.0f },
    .hp_gauge_offset = { 0, -9 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 24.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 24.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 16.0f, 28.6f, 10.0f },
    .cut_base_offset = { 0.0f, 22.0f, 0.0f },
    .cut_width = 48.0f,
    .cut_height = 44.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 0,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_MAHO_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::UNQUAKEABLE,
    .status_vulnerability = &unitWizzerdClone_status,
    .num_parts = 5,
    .parts = unitEliteWizzerd_parts,
    .init_evt_code = (void*)unitEliteWizzerd_init_event,
    .data_table = unitWizzerdClone_data_table,
};

const BattleUnitSetup unitDarkWizzerdClone_spawn_entry = {
    .unit_kind_params = &unit_DarkWizzerdClone,
    .alliance = 1,
    .attack_phase = 0x04000004,
    .position = { -1000.0f, -1000.0f, -1000.0f },
    .addl_target_offset_x = 0,
    .unit_work = { 0, 0, 0, 0 },
    .item_drop_table = nullptr,
};
const BattleUnitSetup unitEliteWizzerdClone_spawn_entry = {
    .unit_kind_params = &unit_EliteWizzerdClone,
    .alliance = 1,
    .attack_phase = 0x04000004,
    .position = { -1000.0f, -1000.0f, -1000.0f },
    .addl_target_offset_x = 0,
    .unit_work = { 0, 0, 0, 0 },
    .item_drop_table = nullptr,
};

// Function / USER_FUNC definitions.

EVT_DEFINE_USER_FUNC(unitWizzerd_CopyStatus) {
    auto* unit1 = ttyd::battle::BattleGetUnitPtr(
        g_BattleWork, evtGetValue(evt, evt->evtArguments[0]));
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

EVT_DEFINE_USER_FUNC(unitWizzerd_ShuffleClones) {
    BattleWorkUnit* units[6];
    int32_t num_units = 0;
    
    for (int32_t i = 0; i < 64; ++i) {
        auto* unit = g_BattleWork->battle_units[i];
        if (unit && (
                unit->current_kind == BattleUnitType::DARK_WIZZERD ||
                unit->current_kind == BattleUnitType::DARK_WIZZERD_CLONE ||
                unit->current_kind == BattleUnitType::ELITE_WIZZERD ||
                unit->current_kind == BattleUnitType::ELITE_WIZZERD_CLONE))
            units[num_units++] = unit;
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

EVT_DEFINE_USER_FUNC(unitWizzerd_EffMagicGetXpos) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[0]));
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, unit_idx);
    
    auto* eff = reinterpret_cast<EffEntry*>(unit->unit_work[UW_MagicEff]);
    auto eff_data_ptr = reinterpret_cast<uintptr_t>(eff->eff_work);
    int32_t eff_num = 0;
    
    for (eff_num = 0; eff_num < eff->eff_count; ++eff_num) {
        if (*reinterpret_cast<int32_t*>(eff_data_ptr + 0x2c) == 3) {
            evtSetFloat(evt, evt->evtArguments[1],
                *reinterpret_cast<float*>(eff_data_ptr + 4));
            break;
        }
        eff_data_ptr += 0x3c;
    }
    if (eff_num == eff->eff_count) {
        evtSetFloat(evt, evt->evtArguments[1], 0.0f);
    }
    
    return 2;
}

EVT_DEFINE_USER_FUNC(unitWizzerd_EffMagic) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[0]));
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, unit_idx);
    
    gc::vec3 target_pos = {
        evtGetFloat(evt, evt->evtArguments[1]),
        evtGetFloat(evt, evt->evtArguments[2]),
        evtGetFloat(evt, evt->evtArguments[3])
    };
    int32_t frame_count = evtGetValue(evt, evt->evtArguments[4]);
    
    gc::vec3 pos;
    ttyd::battle_unit::BtlUnit_GetPos(unit, &pos.x, &pos.y, &pos.z);
    pos.x += unit->unk_scale * unit->movement_params.face_direction;
    pos.y += unit->unk_scale * 18.0f;
    pos.z += 5.0f;
    
    auto* eff = ttyd::eff_mahorn2::effMahorn2Entry(
        pos.x, pos.y, pos.z, target_pos.x, pos.y, pos.z,
        unit->unk_scale, AbsF((target_pos.x - pos.x) / frame_count), 0);
    
    unit->unit_work[UW_MagicEff] = reinterpret_cast<uint32_t>(eff);
    return 2;
}

EVT_DEFINE_USER_FUNC(unitWizzerd_GetClones) {
    int32_t units[4];
    int32_t num_units = 0;
    
    for (int32_t i = 0; i < 64; ++i) {
        auto* unit = g_BattleWork->battle_units[i];
        if (unit && (
                unit->current_kind == BattleUnitType::DARK_WIZZERD_CLONE ||
                unit->current_kind == BattleUnitType::ELITE_WIZZERD_CLONE))
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

struct HandWork {
    gc::vec3 target_pos;
};

EVT_DEFINE_USER_FUNC(unitWizzerd_SetHandDest) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[0]));
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, unit_idx);
    
    int32_t part_id = evtGetValue(evt, evt->evtArguments[1]);
    gc::vec3 target_pos = {
        evtGetFloat(evt, evt->evtArguments[2]),
        evtGetFloat(evt, evt->evtArguments[3]),
        evtGetFloat(evt, evt->evtArguments[4])
    };
    
    int32_t result = 0;
    
    if (unit == nullptr) {
        result = 0;
    } else {
        const int32_t uw_index = part_id - 2 + UW_HandPtrs;
        auto* work = reinterpret_cast<HandWork*>(unit->unit_work[uw_index]);
        
        if (ttyd::battle_unit::BtlUnit_CheckStatus(unit, StatusEffectType::HUGE)) {
            work->target_pos.x = -target_pos.x * 1.25f;
            work->target_pos.y = target_pos.y * 1.25f;
            work->target_pos.z = target_pos.z * 1.25f;
        } else if (ttyd::battle_unit::BtlUnit_CheckStatus(unit, StatusEffectType::TINY)) {
            work->target_pos.x = -target_pos.x * 0.75f;
            work->target_pos.y = target_pos.y * 0.75f;
            work->target_pos.z = target_pos.z * 0.75f;
        } else {
            work->target_pos.x = -target_pos.x;
            work->target_pos.y = target_pos.y;
            work->target_pos.z = target_pos.z;
        }
        
        result = 2;
    }
    
    return result;
}

EVT_DEFINE_USER_FUNC(unitWizzerd_HandMain) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[0]));
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, unit_idx);
    
    int32_t result = 0;
    
    int8_t turns, strength;
    
    if (unit == nullptr) {
        result = 0;
    } else if (
        ttyd::battle_unit::BtlUnit_GetStatus(
            unit, StatusEffectType::SLEEP, &turns, &strength); turns > 0) {
        result = 2;
    } else if (
        ttyd::battle_unit::BtlUnit_GetStatus(
            unit, StatusEffectType::STOP, &turns, &strength); turns > 0) {
        result = 2;
    } else if (
        ttyd::battle_unit::BtlUnit_GetStatus(
            unit, StatusEffectType::FREEZE, &turns, &strength); turns > 0) {
        result = 2;
    } else {
        // Make hands slowly approach their target position.
        for (int32_t i = UW_HandPtrs; i < UW_HandPtrsMax; ++i) {
            auto* part = ttyd::battle_unit::BtlUnit_GetPartsPtr(unit, i + 2);
            auto* work = reinterpret_cast<HandWork*>(unit->unit_work[i]);
            
            gc::vec3 scaled_offset = {
                part->position_offset.x * part->base_scale.x,
                part->position_offset.y * part->base_scale.y,
                part->position_offset.z * part->base_scale.z,
            };
            
            part->position_offset.x =
                scaled_offset.x + (work->target_pos.x - scaled_offset.x) / 10.0f;
            part->position_offset.y =
                scaled_offset.y + (work->target_pos.y - scaled_offset.y) / 10.0f;
            part->position_offset.z =
                scaled_offset.z + (work->target_pos.z - scaled_offset.z) / 10.0f;
        }
        
        result = 2;
    }
    
    return result;
}

EVT_DEFINE_USER_FUNC(unitWizzerd_HandEnd) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[0]));
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, unit_idx);
    
    for (int32_t i = UW_HandPtrs; i < UW_HandPtrsMax; ++i) {
        void* work = reinterpret_cast<void*>(unit->unit_work[i]);
        if (!work) {
            ttyd::battle::BattleFree(work);
            unit->unit_work[i] = 0;
        }
    }
    
    return 2;
}

EVT_DEFINE_USER_FUNC(unitWizzerd_HandInit) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[0]));
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, unit_idx);
    
    for (int32_t i = UW_HandPtrs; i < UW_HandPtrsMax; ++i) {
        auto* part = ttyd::battle_unit::BtlUnit_GetPartsPtr(unit, i + 2);
        ttyd::battle_unit::BtlUnit_SetAnim(part, "MAH_Z_2");
        
        auto* work = reinterpret_cast<HandWork*>(
            ttyd::battle::BattleAlloc(sizeof(HandWork)));
        unit->unit_work[i] = reinterpret_cast<uint32_t>(work);
        work->target_pos.x = 0.0f;
        work->target_pos.y = 0.0f;
        work->target_pos.z = 0.0f;
    }
    
    return 2;
}

// Evt definitions.

EVT_BEGIN(unitWizzerd_alpha_up_event)
    IF_EQUAL(LW(0), -1)
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_OffPartsAttribute, LW(0), 1, 0x200'0000)
    USER_FUNC(btlevtcmd_OffPartsAttribute, LW(0), 2, 0x200'0000)
    USER_FUNC(btlevtcmd_OffPartsAttribute, LW(0), 3, 0x200'0000)
    USER_FUNC(btlevtcmd_OffPartsAttribute, LW(0), 4, 0x200'0000)
    USER_FUNC(btlevtcmd_OffPartsAttribute, LW(0), 5, 0x200'0000)
    USER_FUNC(btlevtcmd_SetAlpha, LW(0), 1, 0)
    USER_FUNC(btlevtcmd_AnimeFlagOnOff, LW(0), 1, 1, 1)
    WAIT_FRM(1)
    USER_FUNC(evt_sub_intpl_init, 0, 0, 254, 30)
    DO(30)
        USER_FUNC(evt_sub_intpl_get_value_para, LW(1), LW(2))
        USER_FUNC(btlevtcmd_SetAlpha, LW(0), 1, LW(1))
        USER_FUNC(btlevtcmd_SetAlpha, LW(0), 2, LW(1))
        USER_FUNC(btlevtcmd_SetAlpha, LW(0), 3, LW(1))
        USER_FUNC(btlevtcmd_SetAlpha, LW(0), 4, LW(1))
        USER_FUNC(btlevtcmd_SetAlpha, LW(0), 5, LW(1))
        WAIT_FRM(1)
    WHILE()
    USER_FUNC(btlevtcmd_SetAlpha, LW(0), 1, 255)
    USER_FUNC(btlevtcmd_SetAlpha, LW(0), 2, 255)
    USER_FUNC(btlevtcmd_SetAlpha, LW(0), 3, 255)
    USER_FUNC(btlevtcmd_SetAlpha, LW(0), 4, 255)
    USER_FUNC(btlevtcmd_SetAlpha, LW(0), 5, 255)
    USER_FUNC(btlevtcmd_AnimeFlagOnOff, LW(0), 1, 1, 1)
    WAIT_FRM(1)
    RETURN()
EVT_END()

EVT_BEGIN(unitWizzerd_alpha_down_event)
    IF_EQUAL(LW(0), -1)
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_SetAlpha, LW(0), 1, 254)
    USER_FUNC(btlevtcmd_AnimeFlagOnOff, LW(0), 1, 1, 1)
    WAIT_FRM(1)
    USER_FUNC(evt_sub_intpl_init, 0, 255, 0, 30)
    DO(30)
        USER_FUNC(evt_sub_intpl_get_value_para, LW(1), LW(2))
        USER_FUNC(btlevtcmd_SetAlpha, LW(0), 1, LW(1))
        USER_FUNC(btlevtcmd_SetAlpha, LW(0), 2, LW(1))
        USER_FUNC(btlevtcmd_SetAlpha, LW(0), 3, LW(1))
        USER_FUNC(btlevtcmd_SetAlpha, LW(0), 4, LW(1))
        USER_FUNC(btlevtcmd_SetAlpha, LW(0), 5, LW(1))
        WAIT_FRM(1)
    WHILE()
    USER_FUNC(btlevtcmd_OnPartsAttribute, LW(0), 1, 0x200'0000)
    USER_FUNC(btlevtcmd_OnPartsAttribute, LW(0), 2, 0x200'0000)
    USER_FUNC(btlevtcmd_OnPartsAttribute, LW(0), 3, 0x200'0000)
    USER_FUNC(btlevtcmd_OnPartsAttribute, LW(0), 4, 0x200'0000)
    USER_FUNC(btlevtcmd_OnPartsAttribute, LW(0), 5, 0x200'0000)
    USER_FUNC(btlevtcmd_SetAlpha, LW(0), 1, 0)
    USER_FUNC(btlevtcmd_AnimeFlagOnOff, LW(0), 1, 1, 1)
    WAIT_FRM(1)
    RETURN()
EVT_END()

EVT_BEGIN(unitWizzerd_shot_miss_event)
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

EVT_BEGIN(unitWizzerd_attack_common_event_sub1)
    IF_EQUAL(LW(15), -1)
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_StopWaitEvent, LW(15))
    USER_FUNC(btlevtcmd_GetPos, LW(15), LW(0), LW(1), LW(2))
    ADD(LW(1), 40)
    USER_FUNC(evt_eff64, PTR(""), PTR("akari_charge_n64"), 0, LW(0), LW(1), LW(2), FLOAT(1.0), 70, 0, 0, 0, 0, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, LW(15), 1, PTR("MAH_A_4"))
    USER_FUNC(btlevtcmd_AnimeChangePose, LW(15), 2, PTR("MAH_S_5A"))
    USER_FUNC(btlevtcmd_AnimeChangePose, LW(15), 3, PTR("MAH_S_5A"))
    USER_FUNC(btlevtcmd_AnimeChangePose, LW(15), 4, PTR("MAH_S_5A"))
    USER_FUNC(btlevtcmd_AnimeChangePose, LW(15), 5, PTR("MAH_S_5A"))
    USER_FUNC(unitWizzerd_SetHandDest, LW(15), 2, FLOAT(-16.0), FLOAT(29.0), FLOAT(-5.0))
    USER_FUNC(unitWizzerd_SetHandDest, LW(15), 3, FLOAT(16.0), FLOAT(29.0), FLOAT(5.0))
    USER_FUNC(unitWizzerd_SetHandDest, LW(15), 4, FLOAT(-16.0), FLOAT(11.0), FLOAT(-5.0))
    USER_FUNC(unitWizzerd_SetHandDest, LW(15), 5, FLOAT(16.0), FLOAT(11.0), FLOAT(5.0))
    WAIT_FRM(50)
    RETURN()
EVT_END()

EVT_BEGIN(unitWizzerd_attack_common_event_sub2)
    IF_EQUAL(LW(15), -1)
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_StopWaitEvent, LW(15))
    USER_FUNC(btlevtcmd_AnimeChangePose, LW(15), 1, PTR("MAH_A_1"))
    INLINE_EVT()
        DO(2)
            SETF(LW(0), FLOAT(-16.0))
            SETF(LW(1), FLOAT(20.0))
            USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(2))
            MULF(LW(1), LW(2))
            USER_FUNC(btlevtcmd_AnimeChangePose, LW(15), 2, PTR("MAH_S_2B"))
            ADD(LW(0), LW(1))
            USER_FUNC(unitWizzerd_SetHandDest, LW(15), 2, LW(0), FLOAT(29.0), FLOAT(-5.0))
            WAIT_FRM(20)
            USER_FUNC(btlevtcmd_AnimeChangePose, LW(15), 2, PTR("MAH_S_2A"))
            SUB(LW(0), LW(1))
            USER_FUNC(unitWizzerd_SetHandDest, LW(15), 2, LW(0), FLOAT(29.0), FLOAT(-5.0))
            WAIT_FRM(20)
        WHILE()
    END_INLINE()
    INLINE_EVT()
        WAIT_FRM(7)
        DO(2)
            SETF(LW(0), FLOAT(16.0))
            SETF(LW(1), FLOAT(20.0))
            USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(2))
            MULF(LW(1), LW(2))
            USER_FUNC(btlevtcmd_AnimeChangePose, LW(15), 5, PTR("MAH_S_2B"))
            ADD(LW(0), LW(1))
            USER_FUNC(unitWizzerd_SetHandDest, LW(15), 5, LW(0), FLOAT(11.0), FLOAT(5.0))
            WAIT_FRM(20)
            USER_FUNC(btlevtcmd_AnimeChangePose, LW(15), 5, PTR("MAH_S_2A"))
            SUB(LW(0), LW(1))
            USER_FUNC(unitWizzerd_SetHandDest, LW(15), 5, LW(0), FLOAT(11.0), FLOAT(5.0))
            WAIT_FRM(20)
        WHILE()
    END_INLINE()
    INLINE_EVT()
        WAIT_FRM(10)
        DO(2)
            SETF(LW(0), FLOAT(-16.0))
            SETF(LW(1), FLOAT(20.0))
            USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(2))
            MULF(LW(1), LW(2))
            USER_FUNC(btlevtcmd_AnimeChangePose, LW(15), 4, PTR("MAH_S_2B"))
            ADD(LW(0), LW(1))
            USER_FUNC(unitWizzerd_SetHandDest, LW(15), 4, LW(0), FLOAT(11.0), FLOAT(-5.0))
            WAIT_FRM(20)
            USER_FUNC(btlevtcmd_AnimeChangePose, LW(15), 4, PTR("MAH_S_2A"))
            SUB(LW(0), LW(1))
            USER_FUNC(unitWizzerd_SetHandDest, LW(15), 4, LW(0), FLOAT(11.0), FLOAT(-5.0))
            WAIT_FRM(20)
        WHILE()
    END_INLINE()
    INLINE_EVT()
        WAIT_FRM(15)
        DO(2)
            SETF(LW(0), FLOAT(16.0))
            SETF(LW(1), FLOAT(20.0))
            USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(2))
            MULF(LW(1), LW(2))
            USER_FUNC(btlevtcmd_AnimeChangePose, LW(15), 3, PTR("MAH_S_2B"))
            ADD(LW(0), LW(1))
            USER_FUNC(unitWizzerd_SetHandDest, LW(15), 3, LW(0), FLOAT(29.0), FLOAT(5.0))
            WAIT_FRM(20)
            USER_FUNC(btlevtcmd_AnimeChangePose, LW(15), 3, PTR("MAH_S_2A"))
            SUB(LW(0), LW(1))
            USER_FUNC(unitWizzerd_SetHandDest, LW(15), 3, LW(0), FLOAT(29.0), FLOAT(5.0))
            WAIT_FRM(20)
        WHILE()
    END_INLINE()
    WAIT_FRM(55)
    RETURN()
EVT_END()

EVT_BEGIN(unitWizzerd_attack_common_event_sub_hit)
    IF_EQUAL(LW(15), -1)
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_GetPos, LW(15), LW(0), LW(1), LW(2))
    ADD(LW(1), 10)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(10), LW(11), LW(12))
    USER_FUNC(unitWizzerd_EffMagic, LW(15), LW(10), LW(11), LW(12), 20)
    RUN_CHILD_EVT(PTR(&unitWizzerd_attack_common_event_sub2))
    WAIT_FRM(5)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MAHO_MAGIC1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MAHO_MAGIC3"), EVT_NULLPTR, 0, EVT_NULLPTR)
    WAIT_FRM(20)
    RETURN()
EVT_END()

EVT_BEGIN(unitWizzerd_attack_common_event_sub_nohit)
    IF_EQUAL(LW(15), -1)
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_GetPos, LW(15), LW(0), LW(1), LW(2))
    ADD(LW(1), 10)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(10), LW(11), LW(12))
    USER_FUNC(btlevtcmd_GetStageSize, LW(10), EVT_NULLPTR, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetFaceDirection, LW(15), LW(13))
    MUL(LW(10), LW(13))
    SUB(LW(11), 5)
    USER_FUNC(unitWizzerd_EffMagic, LW(15), LW(10), LW(11), LW(12), 40)
    RUN_CHILD_EVT(PTR(&unitWizzerd_attack_common_event_sub2))
    WAIT_FRM(5)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MAHO_MAGIC1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MAHO_MAGIC3"), EVT_NULLPTR, 0, EVT_NULLPTR)
    WAIT_FRM(40)
    RETURN()
EVT_END()

EVT_BEGIN(unitWizzerd_attack_all_common_event)
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MAHO_CHARGE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    SET(LW(15), -2)
    RUN_CHILD_EVT(PTR(&unitWizzerd_attack_common_event_sub1))
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MAHO_MAGIC2"), EVT_NULLPTR, 0, EVT_NULLPTR)
        WAIT_FRM(25)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MAHO_MAGIC2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    END_BROTHER()
    SET(LW(15), -2)
    RUN_CHILD_EVT(PTR(&unitWizzerd_attack_common_event_sub2))
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    SET(LW(6), 0)
LBL(10)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), LW(9), 0, LW(5))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(evt_eff64, PTR(""), PTR("thunder_n64"), 4, LW(0), LW(1), LW(2), 0, 0, 0, 0, 0, 0, 0, 0)
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
            GOTO(10)
LBL(98)
            WAIT_FRM(30)
            RETURN()
EVT_END()

EVT_BEGIN(unitWizzerd_attack_common_event)
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MAHO_CHARGE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(unitWizzerd_GetClones, LW(11), LW(12), LW(13), LW(14), LW(0))
    SET(LW(15), LW(11))
    RUN_EVT(PTR(&unitWizzerd_attack_common_event_sub1))
    SET(LW(15), LW(12))
    RUN_EVT(PTR(&unitWizzerd_attack_common_event_sub1))
    SET(LW(15), LW(13))
    RUN_EVT(PTR(&unitWizzerd_attack_common_event_sub1))
    SET(LW(15), LW(14))
    RUN_EVT(PTR(&unitWizzerd_attack_common_event_sub1))
    SET(LW(15), -2)
    RUN_CHILD_EVT(PTR(&unitWizzerd_attack_common_event_sub1))
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MAHO_MAGIC2"), EVT_NULLPTR, 0, EVT_NULLPTR)
        WAIT_FRM(25)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MAHO_MAGIC2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    END_BROTHER()
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
                WAIT_FRM(60)
                DO(0)
                    USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(0))
                    IF_EQUAL(LW(0), -1)
                        USER_FUNC(unitWizzerd_EffMagicGetXpos, -2, LW(0))
                        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(6), LW(7), LW(8))
                        IF_SMALL_EQUAL(LW(0), LW(6))
                            RUN_CHILD_EVT(PTR(&unitWizzerd_shot_miss_event))
                            DO_BREAK()
                        END_IF()
                    ELSE()
                        USER_FUNC(unitWizzerd_EffMagicGetXpos, -2, LW(0))
                        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(6), LW(7), LW(8))
                        IF_LARGE_EQUAL(LW(0), LW(6))
                            RUN_CHILD_EVT(PTR(&unitWizzerd_shot_miss_event))
                            DO_BREAK()
                        END_IF()
                    END_IF()
                    WAIT_FRM(1)
                WHILE()
            END_BROTHER()
            USER_FUNC(unitWizzerd_GetClones, LW(11), LW(12), LW(13), LW(14), LW(0))
            SET(LW(15), LW(11))
            RUN_EVT(PTR(&unitWizzerd_attack_common_event_sub_nohit))
            SET(LW(15), LW(12))
            RUN_EVT(PTR(&unitWizzerd_attack_common_event_sub_nohit))
            SET(LW(15), LW(13))
            RUN_EVT(PTR(&unitWizzerd_attack_common_event_sub_nohit))
            SET(LW(15), LW(14))
            RUN_EVT(PTR(&unitWizzerd_attack_common_event_sub_nohit))
            SET(LW(15), -2)
            RUN_CHILD_EVT(PTR(&unitWizzerd_attack_common_event_sub_nohit))
            GOTO(98)
LBL(91)
            USER_FUNC(unitWizzerd_GetClones, LW(11), LW(12), LW(13), LW(14), LW(0))
            SET(LW(15), LW(11))
            RUN_EVT(PTR(&unitWizzerd_attack_common_event_sub_hit))
            SET(LW(15), LW(12))
            RUN_EVT(PTR(&unitWizzerd_attack_common_event_sub_hit))
            SET(LW(15), LW(13))
            RUN_EVT(PTR(&unitWizzerd_attack_common_event_sub_hit))
            SET(LW(15), LW(14))
            RUN_EVT(PTR(&unitWizzerd_attack_common_event_sub_hit))
            SET(LW(15), -2)
            RUN_CHILD_EVT(PTR(&unitWizzerd_attack_common_event_sub_hit))
            USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
            USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
            GOTO(98)
LBL(98)
            USER_FUNC(unitWizzerd_GetClones, LW(11), LW(12), LW(13), LW(14), LW(0))
            IF_NOT_EQUAL(LW(11), -1)
                USER_FUNC(btlevtcmd_StartWaitEvent, LW(11))
            END_IF()
            IF_NOT_EQUAL(LW(12), -1)
                USER_FUNC(btlevtcmd_StartWaitEvent, LW(12))
            END_IF()
            IF_NOT_EQUAL(LW(13), -1)
                USER_FUNC(btlevtcmd_StartWaitEvent, LW(13))
            END_IF()
            IF_NOT_EQUAL(LW(14), -1)
                USER_FUNC(btlevtcmd_StartWaitEvent, LW(14))
            END_IF()
            WAIT_FRM(30)
            RETURN()
EVT_END()

EVT_BEGIN(unitWizzerd_punch_attack_event)
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
    USER_FUNC(unitWizzerd_SetHandDest, -2, 2, FLOAT(-16.0), FLOAT(29.0), FLOAT(-5.0))
    USER_FUNC(unitWizzerd_SetHandDest, -2, 3, FLOAT(16.0), FLOAT(29.0), FLOAT(5.0))
    USER_FUNC(unitWizzerd_SetHandDest, -2, 4, FLOAT(-16.0), FLOAT(11.0), FLOAT(-5.0))
    USER_FUNC(unitWizzerd_SetHandDest, -2, 5, FLOAT(16.0), FLOAT(11.0), FLOAT(5.0))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MAHO_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    IF_EQUAL(LW(15), -1)
        ADD(LW(0), 35)
    ELSE()
        SUB(LW(0), 35)
    END_IF()
    USER_FUNC(btlevtcmd_JumpSetting, -2, 0, 4, FLOAT(0.001))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 20, LW(2), 0, -1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("MAH_S_1"))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 2, PTR("MAH_Z_2"))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 3, PTR("MAH_Z_2"))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 4, PTR("MAH_Z_2"))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 5, PTR("MAH_Z_2"))
    SET(LW(10), 2)
    DO(4)
        USER_FUNC(btlevtcmd_SetPartsBlur, -2, LW(10), 255, 255, 255, 255, 255, 255, 255, 100, 0)
        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, LW(10), 0x400'0000)
        ADD(LW(10), 1)
    WHILE()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MAHO_ATTACK2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    DO(25)
        SET(LW(10), 2)
        DO(4)
            USER_FUNC(btlevtcmd_GetPartsPos, -2, LW(10), LW(0), LW(1), LW(2))
            IF_EQUAL(LW(15), -1)
                USER_FUNC(evt_sub_random, 120, LW(11))
                ADD(LW(11), -90)
            ELSE()
                USER_FUNC(evt_sub_random, 120, LW(11))
                MUL(LW(11), -1)
                ADD(LW(11), 90)
            END_IF()
            USER_FUNC(evt_sub_random, 60, LW(12))
            ADD(LW(12), -20)
            USER_FUNC(unitWizzerd_SetHandDest, -2, LW(10), LW(11), LW(12), LW(2))
            ADD(LW(10), 1)
        WHILE()
        WAIT_FRM(2)
    WHILE()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("MAH_S_1"))
    USER_FUNC(unitWizzerd_SetHandDest, -2, 2, FLOAT(-16.0), FLOAT(29.0), FLOAT(-5.0))
    USER_FUNC(unitWizzerd_SetHandDest, -2, 3, FLOAT(16.0), FLOAT(29.0), FLOAT(5.0))
    USER_FUNC(unitWizzerd_SetHandDest, -2, 4, FLOAT(-16.0), FLOAT(11.0), FLOAT(-5.0))
    USER_FUNC(unitWizzerd_SetHandDest, -2, 5, FLOAT(16.0), FLOAT(11.0), FLOAT(5.0))
    SET(LW(10), 2)
    DO(4)
        USER_FUNC(btlevtcmd_OffPartsAttribute, -2, LW(10), 0x400'0000)
        ADD(LW(10), 1)
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
            WAIT_FRM(30)
            GOTO(98)
LBL(91)
            USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
            USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
            WAIT_FRM(30)
            GOTO(98)
LBL(98)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MAHO_ATTACK1_2"), EVT_NULLPTR, 0, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_JumpSetting, -2, 0, 6, FLOAT(0.001))
            USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 0, -1)
            WAIT_FRM(45)
LBL(99)
            USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
            RETURN()
EVT_END()

EVT_BEGIN(unitWizzerd_laser_attack_event)
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
    RUN_CHILD_EVT(PTR(&unitWizzerd_attack_common_event))
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitWizzerd_confuse_attack_event)
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
    RUN_CHILD_EVT(PTR(&unitWizzerd_attack_common_event))
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitWizzerd_thunder_attack_event)
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
    RUN_CHILD_EVT(PTR(&unitWizzerd_attack_all_common_event))
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitWizzerd_support_common_event1)
    // Set if can only use on self.
    IF_EQUAL(LW(10), 1)
        SET(LW(3), -2)
        SET(LW(4), 1)
    ELSE()
        USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
        USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(9))
        USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(9), LW(3), LW(4))
        IF_EQUAL(LW(3), -1)
            USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
            IF_NOT_EQUAL(LW(0), 0)
                RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
                SET(LW(0), 0)
                RETURN()
            END_IF()
            SET(LW(0), 0)
            RETURN()
        END_IF()
    END_IF()
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    SET(LW(0), 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitWizzerd_support_common_event2)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MAHO_CHARGE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    ADD(LW(1), 40)
    USER_FUNC(evt_eff64, PTR(""), PTR("akari_charge_n64"), 0, LW(0), LW(1), LW(2), FLOAT(1.0), 70, 0, 0, 0, 0, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("MAH_A_4"))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 2, PTR("MAH_S_5A"))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 3, PTR("MAH_S_5A"))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 4, PTR("MAH_S_5A"))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 5, PTR("MAH_S_5A"))
    USER_FUNC(unitWizzerd_SetHandDest, -2, 2, FLOAT(-16.0), FLOAT(29.0), FLOAT(-5.0))
    USER_FUNC(unitWizzerd_SetHandDest, -2, 3, FLOAT(16.0), FLOAT(29.0), FLOAT(5.0))
    USER_FUNC(unitWizzerd_SetHandDest, -2, 4, FLOAT(-16.0), FLOAT(11.0), FLOAT(-5.0))
    USER_FUNC(unitWizzerd_SetHandDest, -2, 5, FLOAT(16.0), FLOAT(11.0), FLOAT(5.0))
    WAIT_FRM(50)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("MAH_A_2A"))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 2, PTR("MAH_S_4A"))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 3, PTR("MAH_S_4A"))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 4, PTR("MAH_S_4A"))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 5, PTR("MAH_S_4A"))
    USER_FUNC(unitWizzerd_SetHandDest, -2, 2, FLOAT(-20.0), FLOAT(40.0), FLOAT(-15.0))
    USER_FUNC(unitWizzerd_SetHandDest, -2, 3, FLOAT(20.0), FLOAT(40.0), FLOAT(15.0))
    USER_FUNC(unitWizzerd_SetHandDest, -2, 4, FLOAT(-20.0), FLOAT(0.0), FLOAT(-15.0))
    USER_FUNC(unitWizzerd_SetHandDest, -2, 5, FLOAT(20.0), FLOAT(0.0), FLOAT(15.0))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MAHO_MAGIC4"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    ADD(LW(1), 15)
    USER_FUNC(evt_eff, PTR(""), PTR("mahorn"), 0, LW(0), LW(1), LW(2), FLOAT(1.4), 2, 30, 0, 0, 0, 0, 0)
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MAHO_MAGIC4"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    ADD(LW(1), 15)
    USER_FUNC(evt_eff, PTR(""), PTR("mahorn"), 0, LW(0), LW(1), LW(2), FLOAT(1.4), 2, 30, 0, 0, 0, 0, 0)
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("MAH_A_2C"))
    RETURN()
EVT_END()

EVT_BEGIN(unitWizzerd_status_magic_event)
    RUN_CHILD_EVT(PTR(&unitWizzerd_support_common_event1))
    IF_EQUAL(LW(0), 0)
        GOTO(99)
    END_IF()
    RUN_CHILD_EVT(PTR(&unitWizzerd_support_common_event2))
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    WAIT_FRM(10)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    WAIT_FRM(30)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitWizzerd_recover_magic_event)
    RUN_CHILD_EVT(PTR(&unitWizzerd_support_common_event1))
    IF_EQUAL(LW(0), 0)
        GOTO(99)
    END_IF()
    RUN_CHILD_EVT(PTR(&unitWizzerd_support_common_event2))
    
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(0))
    IF_EQUAL(LW(0), (int32_t)BattleUnitType::WIZZERD)
        SET(LW(10), 6)
    ELSE()
        SET(LW(10), 10)
    END_IF()
    
    USER_FUNC(btlevtcmd_RecoverHp, LW(3), LW(4), LW(10))
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    ADD(LW(1), 30)
    ADD(LW(2), 10)
    USER_FUNC(evt_eff, PTR(""), PTR("recovery"), 0, LW(0), LW(1), LW(2), LW(10), 0, 0, 0, 0, 0, 0, 0)
    WAIT_FRM(10)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    WAIT_FRM(30)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitWizzerd_clone_spawn_event)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(10))
    IF_EQUAL(LW(10), (int32_t)BattleUnitType::DARK_WIZZERD)
        USER_FUNC(btlevtcmd_SpawnUnit, LW(3), PTR(&unitDarkWizzerdClone_spawn_entry), 0)
    ELSE()
        USER_FUNC(btlevtcmd_SpawnUnit, LW(3), PTR(&unitEliteWizzerdClone_spawn_entry), 0)
    END_IF()
    USER_FUNC(btlevtcmd_SetAlpha, LW(3), 1, 0)
    USER_FUNC(btlevtcmd_SetAlpha, LW(3), 2, 0)
    USER_FUNC(btlevtcmd_SetAlpha, LW(3), 3, 0)
    USER_FUNC(btlevtcmd_SetAlpha, LW(3), 4, 0)
    USER_FUNC(btlevtcmd_SetAlpha, LW(3), 5, 0)
    WAIT_FRM(2)
    USER_FUNC(btlevtcmd_SetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetHomePos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetUnitId, -2, LW(13))
    USER_FUNC(unitWizzerd_CopyStatus, LW(13), LW(3))
    USER_FUNC(btlevtcmd_OffAttribute, LW(3), 0x200'0000)
    USER_FUNC(btlevtcmd_SetUnitWork, LW(3), UW_IsClone, 1)
    USER_FUNC(btlevtcmd_SetEventAttack, LW(3), PTR(&unitWizzerdClone_attack_event))
    USER_FUNC(btlevtcmd_SetRegistStatus, LW(3), PTR(&unitWizzerdClone_status))
    USER_FUNC(btlevtcmd_OnPartsAttribute, LW(3), 1, 0x4000)
    RETURN()
EVT_END()

EVT_BEGIN(unitWizzerd_create_clones_event)
    SET(LW(3), -2)
    SET(LW(4), 1)
    RUN_CHILD_EVT(PTR(&unitWizzerd_support_common_event2))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MAHO_OFFSHOOT1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    SET(LW(0), -2)
    RUN_CHILD_EVT(PTR(&unitWizzerd_alpha_down_event))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    DO(0)
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 40)
        SET(LW(4), LW(0))
        IF_LARGE(LW(4), 0)
            SUB(LW(2), 10)
            USER_FUNC(btlevtcmd_CheckSpace, LW(3), LW(0), LW(1), LW(2), 40, 0, 0, 0)
            IF_EQUAL(LW(3), -1)
                RUN_CHILD_EVT(PTR(&unitWizzerd_clone_spawn_event))
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
                RUN_CHILD_EVT(PTR(&unitWizzerd_clone_spawn_event))
            END_IF()
        ELSE()
            DO_BREAK()
        END_IF()
    WHILE()
    USER_FUNC(unitWizzerd_ShuffleClones)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_MAHO_OFFSHOOT2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    SET(LW(0), -2)
    RUN_EVT(PTR(&unitWizzerd_alpha_up_event))
    USER_FUNC(unitWizzerd_GetClones, LW(1), LW(2), LW(3), LW(4), EVT_NULLPTR)
    SET(LW(0), LW(1))
    RUN_EVT(PTR(&unitWizzerd_alpha_up_event))
    SET(LW(0), LW(2))
    RUN_EVT(PTR(&unitWizzerd_alpha_up_event))
    SET(LW(0), LW(3))
    RUN_EVT(PTR(&unitWizzerd_alpha_up_event))
    SET(LW(0), LW(4))
    RUN_EVT(PTR(&unitWizzerd_alpha_up_event))
    WAIT_FRM(60)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitWizzerd_attack_event)
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        SET(LW(9), PTR(&unitWizzerd_weaponPunch))
        RUN_CHILD_EVT(PTR(&unitWizzerd_punch_attack_event))
        GOTO(99)
    END_IF()

    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_IsClone, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        RETURN()
    END_IF()

    USER_FUNC(btlevtcmd_EnemyItemUseCheck, -2, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(LW(0))
        USER_FUNC(btlevtcmd_StartWaitEvent, -2)
        RETURN()
    END_IF()

    USER_FUNC(unitWizzerd_GetClones, LW(1), LW(2), LW(3), LW(4), LW(0))
    IF_LARGE_EQUAL(LW(0), 1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            GOTO(99)
        END_IF()
        SET(LW(9), PTR(&unitWizzerd_weaponLaser))
        RUN_CHILD_EVT(PTR(&unitWizzerd_laser_attack_event))
        GOTO(99)
    END_IF()
        
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(10))
    IF_NOT_EQUAL(LW(10), (int32_t)BattleUnitType::WIZZERD)
        USER_FUNC(evtTot_CheckNumEnemiesRemaining, LW(0))
        IF_EQUAL(LW(0), 1)
            RUN_CHILD_EVT(PTR(&unitWizzerd_create_clones_event))
            RETURN()
        END_IF()
    END_IF()

    SWITCH(LW(10))
        CASE_EQUAL((int32_t)BattleUnitType::WIZZERD)
            USER_FUNC(btlevtcmd_DrawLots, LW(0), 9,  0, 50, 30, 10, 10, 10,  0,  0, 10)
            // Can use on self or other enemies.
            SET(LW(10), 0)
        CASE_EQUAL((int32_t)BattleUnitType::DARK_WIZZERD)
            USER_FUNC(btlevtcmd_DrawLots, LW(0), 9, 30, 60, 10,  0,  8,  8,  8,  8,  8)
            // Can only use on self.
            SET(LW(10), 1)
        CASE_ETC()
            USER_FUNC(btlevtcmd_DrawLots, LW(0), 9, 30, 60, 10,  0, 10, 10, 10, 10, 10)
            SET(LW(10), 1)
    END_SWITCH()
    
    SWITCH(LW(0))
        CASE_EQUAL(0)
            SET(LW(9), PTR(&unitWizzerd_weaponPunch))
            RUN_CHILD_EVT(PTR(&unitWizzerd_punch_attack_event))
        CASE_EQUAL(1)
            SET(LW(9), PTR(&unitWizzerd_weaponLaser))
            RUN_CHILD_EVT(PTR(&unitWizzerd_laser_attack_event))
        CASE_EQUAL(2)
            SET(LW(9), PTR(&unitWizzerd_weaponThunder))
            RUN_CHILD_EVT(PTR(&unitWizzerd_thunder_attack_event))
        CASE_EQUAL(3)
            SET(LW(9), PTR(&unitWizzerd_weaponConfuse))
            RUN_CHILD_EVT(PTR(&unitWizzerd_confuse_attack_event))
        CASE_EQUAL(4)
            SET(LW(9), PTR(&unitWizzerd_weaponHuge))
            RUN_CHILD_EVT(PTR(&unitWizzerd_status_magic_event))
        CASE_EQUAL(5)
            SET(LW(9), PTR(&unitWizzerd_weaponDef))
            RUN_CHILD_EVT(PTR(&unitWizzerd_status_magic_event))
        CASE_EQUAL(6)
            SET(LW(9), PTR(&unitWizzerd_weaponDodgy))
            RUN_CHILD_EVT(PTR(&unitWizzerd_status_magic_event))
        CASE_EQUAL(7)
            SET(LW(9), PTR(&unitWizzerd_weaponInvis))
            RUN_CHILD_EVT(PTR(&unitWizzerd_status_magic_event))
        CASE_EQUAL(8)
            SET(LW(9), PTR(&unitWizzerd_weaponRecover))
            RUN_CHILD_EVT(PTR(&unitWizzerd_recover_magic_event))
    END_SWITCH()
    
LBL(99)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitWizzerdClone_attack_event)
LBL(99)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitWizzerd_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_KillClones, 0)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_IsClone, LW(0))
    IF_EQUAL(LW(0), 0)
        USER_FUNC(unitWizzerd_GetClones, LW(1), LW(2), LW(3), LW(4), LW(0))
        IF_LARGE_EQUAL(LW(0), 1)
            USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_KillClones, 1)
        END_IF()
    ELSE()
        SET(LW(0), -2)
        RUN_CHILD_EVT(PTR(&unitWizzerd_clone_dead_event))
        RETURN()
    END_IF()
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    USER_FUNC(btlevtcmd_GetUnitId, -2, LW(0))
    IF_NOT_EQUAL(LW(0), -1)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_KillClones, LW(0))
        IF_EQUAL(LW(0), 1)
            RUN_CHILD_EVT(PTR(&unitWizzerd_kill_all_clones_event))
        END_IF()
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitWizzerd_wait_event)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_IsClone, LW(0))
    IF_EQUAL(LW(0), 0)
        USER_FUNC(unitWizzerd_GetClones, LW(1), LW(2), LW(3), LW(4), LW(0))
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
    END_IF()
    USER_FUNC(btlevtcmd_CheckStatus, -2, 9, LW(13))
    USER_FUNC(btlevtcmd_CheckStatus, -2, 1, LW(14))
    USER_FUNC(btlevtcmd_CheckStatus, -2, 2, LW(15))
    OR(LW(15), LW(13))
    OR(LW(15), LW(14))
    IF_NOT_EQUAL(LW(15), 0)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 2, PTR("MAH_Z_2"))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 3, PTR("MAH_Z_2"))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 4, PTR("MAH_Z_2"))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 5, PTR("MAH_Z_2"))
    ELSE()
        BROTHER_EVT()
LBL(10)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 2, PTR("MAH_S_4A"))
            USER_FUNC(evt_sub_random, 15, LW(0))
            ADD(LW(0), 15)
            WAIT_FRM(LW(0))
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 2, PTR("MAH_S_4B"))
            USER_FUNC(evt_sub_random, 15, LW(0))
            ADD(LW(0), 15)
            WAIT_FRM(LW(0))
            GOTO(10)
        END_BROTHER()
        BROTHER_EVT()
LBL(11)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 3, PTR("MAH_S_4A"))
            USER_FUNC(evt_sub_random, 15, LW(0))
            ADD(LW(0), 15)
            WAIT_FRM(LW(0))
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 3, PTR("MAH_S_4B"))
            USER_FUNC(evt_sub_random, 15, LW(0))
            ADD(LW(0), 15)
            WAIT_FRM(LW(0))
            GOTO(11)
        END_BROTHER()
        BROTHER_EVT()
LBL(12)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 4, PTR("MAH_S_4A"))
            USER_FUNC(evt_sub_random, 15, LW(0))
            ADD(LW(0), 15)
            WAIT_FRM(LW(0))
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 4, PTR("MAH_S_4B"))
            USER_FUNC(evt_sub_random, 15, LW(0))
            ADD(LW(0), 15)
            WAIT_FRM(LW(0))
            GOTO(12)
        END_BROTHER()
        BROTHER_EVT()
LBL(13)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 5, PTR("MAH_S_4A"))
            USER_FUNC(evt_sub_random, 15, LW(0))
            ADD(LW(0), 15)
            WAIT_FRM(LW(0))
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 5, PTR("MAH_S_4B"))
            USER_FUNC(evt_sub_random, 15, LW(0))
            ADD(LW(0), 15)
            WAIT_FRM(LW(0))
            GOTO(13)
        END_BROTHER()
    END_IF()
    BROTHER_EVT()
LBL(0)
        USER_FUNC(unitWizzerd_SetHandDest, -2, 2, FLOAT(-25.0), FLOAT(30.0), FLOAT(-10.0))
        USER_FUNC(evt_sub_random, 15, LW(0))
        ADD(LW(0), 15)
        WAIT_FRM(LW(0))
        USER_FUNC(unitWizzerd_SetHandDest, -2, 2, FLOAT(-15.0), FLOAT(40.0), FLOAT(-10.0))
        USER_FUNC(evt_sub_random, 15, LW(0))
        ADD(LW(0), 15)
        WAIT_FRM(LW(0))
        GOTO(0)
    END_BROTHER()
    BROTHER_EVT()
LBL(1)
        USER_FUNC(unitWizzerd_SetHandDest, -2, 3, FLOAT(25.0), FLOAT(30.0), FLOAT(10.0))
        USER_FUNC(evt_sub_random, 15, LW(0))
        ADD(LW(0), 15)
        WAIT_FRM(LW(0))
        USER_FUNC(unitWizzerd_SetHandDest, -2, 3, FLOAT(15.0), FLOAT(40.0), FLOAT(10.0))
        USER_FUNC(evt_sub_random, 15, LW(0))
        ADD(LW(0), 15)
        WAIT_FRM(LW(0))
        GOTO(1)
    END_BROTHER()
    BROTHER_EVT()
LBL(2)
        USER_FUNC(unitWizzerd_SetHandDest, -2, 4, FLOAT(-25.0), FLOAT(10.0), FLOAT(-10.0))
        USER_FUNC(evt_sub_random, 15, LW(0))
        ADD(LW(0), 15)
        WAIT_FRM(LW(0))
        USER_FUNC(unitWizzerd_SetHandDest, -2, 4, FLOAT(-15.0), FLOAT(0.0), FLOAT(-10.0))
        USER_FUNC(evt_sub_random, 15, LW(0))
        ADD(LW(0), 15)
        WAIT_FRM(LW(0))
        GOTO(2)
    END_BROTHER()
LBL(3)
    USER_FUNC(unitWizzerd_SetHandDest, -2, 5, FLOAT(25.0), FLOAT(10.0), FLOAT(10.0))
    USER_FUNC(evt_sub_random, 15, LW(0))
    ADD(LW(0), 15)
    WAIT_FRM(LW(0))
    USER_FUNC(unitWizzerd_SetHandDest, -2, 5, FLOAT(15.0), FLOAT(0.0), FLOAT(10.0))
    USER_FUNC(evt_sub_random, 15, LW(0))
    ADD(LW(0), 15)
    WAIT_FRM(LW(0))
    GOTO(3)
    RETURN()
EVT_END()

EVT_BEGIN(unitWizzerd_clone_dead_event)
    USER_FUNC(btlevtcmd_GetUnitId, LW(0), LW(0))
    IF_EQUAL(LW(0), -1)
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_GetUnitWork, LW(0), UW_CloneKillSignalSent, LW(1))
    IF_EQUAL(LW(1), 1)
        GOTO(10)
    END_IF()
    USER_FUNC(btlevtcmd_SetUnitWork, LW(0), UW_CloneKillSignalSent, 1)
    RUN_CHILD_EVT(PTR(&unitWizzerd_alpha_down_event))
LBL(10)
    USER_FUNC(btlevtcmd_WaitAttackEnd)
    USER_FUNC(btlevtcmd_KillUnit, LW(0), 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitWizzerd_kill_all_clones_event)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_IsClone, LW(0))
    IF_EQUAL(LW(0), 0)
        USER_FUNC(unitWizzerd_GetClones, LW(1), LW(2), LW(3), LW(4), LW(0))
        IF_LARGE_EQUAL(LW(0), 1)
            USER_FUNC(btlevtcmd_WaitAttackEnd)
            USER_FUNC(unitWizzerd_GetClones, LW(1), LW(2), LW(3), LW(4), LW(0))
            SET(LW(0), LW(1))
            RUN_EVT(PTR(&unitWizzerd_clone_dead_event))
            SET(LW(0), LW(2))
            RUN_EVT(PTR(&unitWizzerd_clone_dead_event))
            SET(LW(0), LW(3))
            RUN_EVT(PTR(&unitWizzerd_clone_dead_event))
            SET(LW(0), LW(4))
            RUN_EVT(PTR(&unitWizzerd_clone_dead_event))
            DO(0)
                USER_FUNC(unitWizzerd_GetClones, LW(1), LW(2), LW(3), LW(4), LW(0))
                IF_EQUAL(LW(0), 0)
                    DO_BREAK()
                END_IF()
                WAIT_FRM(1)
            WHILE()
        END_IF()
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitWizzerd_dead_event)
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
    RUN_CHILD_EVT(PTR(&unitWizzerd_kill_all_clones_event))
    USER_FUNC(btlevtcmd_CheckDamageCode, LW(10), 1024, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_KillUnit, LW(10), 0)
    ELSE()
        USER_FUNC(btlevtcmd_KillUnit, LW(10), 2)
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitWizzerd_gale_dead_event)
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

    // Disable coins and EXP for Gale Force.
    // USER_FUNC(btlevtcmd_GetCoin, LW(10), LW(0))
    // USER_FUNC(btlevtcmd_StoreCoin, LW(0))
    // USER_FUNC(btlevtcmd_GetExp, LW(10), LW(0))
    // USER_FUNC(btlevtcmd_StoreExp, LW(0))

    RUN_CHILD_EVT(PTR(&unitWizzerd_kill_all_clones_event))
    USER_FUNC(btlevtcmd_KillUnit, -2, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitWizzerd_incoming_miss_event)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_IsClone, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        SET(LW(0), -2)
        RUN_CHILD_EVT(PTR(&unitWizzerd_clone_dead_event))
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitWizzerd_unitWizzerd_HandMain_event)
LBL(0)
    WAIT_FRM(1)
    USER_FUNC(btlevtcmd_GetUnitId, -2, LW(0))
    IF_EQUAL(LW(0), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(unitWizzerd_HandMain, -2)
    GOTO(0)
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitWizzerd_common_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitWizzerd_wait_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitWizzerd_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitWizzerd_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitWizzerd_attack_event))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetHomePos, -2, LW(0), 20, LW(2))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(unitWizzerd_HandInit, -2)
    USER_FUNC(unitWizzerd_SetHandDest, -2, 2, FLOAT(-25.0), FLOAT(30.0), FLOAT(-10.0))
    USER_FUNC(unitWizzerd_SetHandDest, -2, 3, FLOAT(25.0), FLOAT(30.0), FLOAT(10.0))
    USER_FUNC(unitWizzerd_SetHandDest, -2, 4, FLOAT(-25.0), FLOAT(10.0), FLOAT(-10.0))
    USER_FUNC(unitWizzerd_SetHandDest, -2, 5, FLOAT(25.0), FLOAT(10.0), FLOAT(10.0))
    RUN_EVT(PTR(&unitWizzerd_unitWizzerd_HandMain_event))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_IsClone, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_CloneKillSignalSent, 0)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitWizzerd_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::WIZZERD)
    RUN_CHILD_EVT(unitWizzerd_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitDarkWizzerd_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::DARK_WIZZERD)
    RUN_CHILD_EVT(unitWizzerd_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitEliteWizzerd_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::ELITE_WIZZERD)
    RUN_CHILD_EVT(unitWizzerd_common_init_event)
    RETURN()
EVT_END()

}  // namespace mod::tot::custom