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
#include <ttyd/eff_glass_n64.h>
#include <ttyd/eff_syuryou.h>
#include <ttyd/evt_eff.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evt_sub.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/mapdrv.h>
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
constexpr const int32_t UW_Main_MiniCount = 0;
constexpr const int32_t UW_Main_InBarrage = 1;
constexpr const int32_t UW_Main_BarrageShots = 2;
constexpr const int32_t UW_Main_BarrierEff = 3;
constexpr const int32_t UW_Main_MiniIds = 4;        // 4-7, one per mini
constexpr const int32_t UW_Main_MiniInOutWork = 8;  // 8-11, one per mini
constexpr const int32_t UW_Main_MaxMinis = 13;
constexpr const int32_t UW_Main_MinisPerTurn = 14;
constexpr const int32_t UW_Main_BattleUnitType = 15;

constexpr const int32_t PW_Main_BarrageTargetUnits = 0;
constexpr const int32_t PW_Main_BarrageTargetParts = 5;
constexpr const int32_t PW_Main_BarrageTargetHits = 10;
constexpr const int32_t PW_Main_ShotSfx = 15;

constexpr const int32_t UW_Mini_ParentId = 0;
constexpr const int32_t UW_Mini_SpinIdle = 1;
constexpr const int32_t UW_Mini_BattleUnitType = 15;

constexpr const int32_t PW_Mini_SplineTable = 2;

// Combined spline tables for Mini-Yuxes flying out of and into the main Yux.
// This way, you can just index the same table at a different offset rather
// than doing the original's big switch-case.
constexpr const float splineTable_1_1[30] = {
    // Out
      0.0,   0.0, -1.0,
    -35.0, -35.0, -1.0,
    -40.0,   0.0, -1.0,
     30.0,  40.0, -1.0,
     25.0,   0.0,  0.0,
    // In
     25.0,   0.0,  0.0,
     20.0, -30.0, -1.0,
     12.5, -40.0, -1.0,
      5.0, -30.0, -1.0,
      0.0,   0.0, -1.0,
};
constexpr const float splineTable_2_1[30] = {
    // Out
      0.0,   0.0, -1.0,
      5.0,  30.0, -1.0,
     12.5,  40.0, -1.0,
     20.0,  30.0, -1.0,
     25.0,   0.0,  0.0,
    // In
     25.0,   0.0,  0.0,
     20.0, -30.0, -1.0,
     12.5, -40.0, -1.0,
      5.0, -30.0, -1.0,
      0.0,   0.0, -1.0,
};
constexpr const float splineTable_2_2[30] = {
    // Out
      0.0,   0.0, -1.0,
     -5.0, -30.0, -1.0,
    -12.5, -40.0, -1.0,
    -20.0, -30.0, -1.0,
    -25.0,   0.0,  0.0,
    // In
    -25.0,   0.0,  0.0,
    -20.0,  30.0, -1.0,
    -12.5,  30.0, -1.0,
     -5.0,  30.0, -1.0,
      0.0,   0.0, -1.0,
};
constexpr const float splineTable_3_1[30] = {
    // Out
      0.0,   0.0, -1.0,
      0.0,  25.0, -1.0,
      0.0,  50.0, -1.0,
      0.0,  45.0, -1.0,
      0.0,  25.0,  0.0,
    // In
      0.0,  25.0,  0.0,
      0.0,  45.0, -1.0,
      0.0,  50.0, -1.0,
      0.0,  25.0, -1.0,
      0.0,   0.0, -1.0,
};
constexpr const float splineTable_3_2[30] = {
    // Out
      0.0,   0.0, -1.0,
     25.0, -20.0, -1.0,
     45.0, -30.0, -1.0,
     30.0, -25.0, -1.0,
     25.0, -25.0,  0.0,
    // In
     25.0, -25.0,  0.0,
     40.0, -40.0, -1.0,
     30.0, -35.0, -1.0,
     20.0, -25.0, -1.0,
      0.0,   0.0, -1.0,
};
constexpr const float splineTable_3_3[30] = {
    // Out
      0.0,   0.0, -1.0,
    -25.0, -20.0, -1.0,
    -45.0, -30.0, -1.0,
    -30.0, -25.0, -1.0,
    -25.0, -25.0,  0.0,
    // In
    -25.0, -25.0,  0.0,
    -40.0, -40.0, -1.0,
    -30.0, -35.0, -1.0,
    -20.0, -25.0, -1.0,
      0.0,   0.0, -1.0,
};
constexpr const float splineTable_4_1[30] = {
    // Out
      0.0,   0.0, -1.0,
     25.0,  20.0, -1.0,
     45.0,  30.0, -1.0,
     30.0,  25.0, -1.0,
     25.0,  25.0,  0.0,
    // In
     25.0,  25.0,  0.0,
     30.0,  35.0, -1.0,
     40.0,  45.0, -1.0,
     20.0,  25.0, -1.0,
      0.0,   0.0, -1.0,
};
constexpr const float splineTable_4_2[30] = {
    // Out
      0.0,   0.0, -1.0,
    -25.0,  20.0, -1.0,
    -45.0,  30.0, -1.0,
    -30.0,  25.0, -1.0,
    -25.0,  25.0,  0.0,
    // In
    -25.0,  25.0,  0.0,
    -30.0,  35.0, -1.0,
    -40.0,  45.0, -1.0,
    -20.0,  25.0, -1.0,
      0.0,   0.0, -1.0,
};
constexpr const float splineTable_4_3[30] = {
    // Out
      0.0,   0.0, -1.0,
     25.0, -20.0, -1.0,
     45.0, -30.0, -1.0,
     30.0, -25.0, -1.0,
     25.0, -25.0,  0.0,
    // In
     25.0, -25.0,  0.0,
     30.0, -35.0, -1.0,
     40.0, -45.0, -1.0,
     20.0, -25.0, -1.0,
      0.0,   0.0, -1.0,
};
constexpr const float splineTable_4_4[30] = {
    // Out
      0.0,   0.0, -1.0,
    -25.0, -20.0, -1.0,
    -45.0, -30.0, -1.0,
    -30.0, -25.0, -1.0,
    -25.0, -25.0,  0.0,
    // In
    -25.0, -25.0,  0.0,
    -30.0, -35.0, -1.0,
    -40.0, -45.0, -1.0,
    -20.0, -25.0, -1.0,
      0.0,   0.0, -1.0,
};

// Lookup table, to avoid unnecessary switch-case logic in the original
// spline assingment code.
constexpr const float* splineLookup[] = {
    splineTable_1_1,
    splineTable_2_1, splineTable_2_2,
    splineTable_3_1, splineTable_3_2, splineTable_3_3,
    splineTable_4_1, splineTable_4_2, splineTable_4_3, splineTable_4_4,
};

// Lookup table for initial positions of Yuxes in all possible arrangements.
constexpr const gc::vec3 kYuxPositions[] = {
    // 1 Yux
    { 25.0f, 0.0f, 0.0f },
    // 2 Yuxes
    { 25.0f, 0.0f, 0.0f }, { -25.0f, 0.0f, 0.0f },
    // 3 Yuxes
    { 0.0f, 25.0f, 0.0f }, { 25.0f, -25.0f, 0.0f }, { -25.0f, -25.0f, 0.0f },
    // 4 Yuxes
    { 25.0f, 25.0f, 0.0f }, { -25.0f, 25.0f, 0.0f }, 
    { 25.0f, -25.0f, 0.0f }, { -25.0f, -25.0f, 0.0f },
};

// Used for different layers of the above flash.
constexpr const float kAfterimageScale[] = { 1.7f, 2.4f, 3.1f, 3.8f, 4.5f };

// Timers for the flashes used by main Yux and mini-Yuxes before splitting in
// the unison phase, and before using their team attack.
//
// Note that normal Yux normally uses only 3 timers, but it'll be useful to
// share all of the following code between all three types.
//
// Shared across all instances, so that has the side effect of the flash in the
// unison phase getting sped up if there are multiple, apparently.
// This already can happen in the original with the same species, so whatever.
int32_t g_FlashTimers[5];

}  // namespace

// Evt / Function declarations.

extern const int32_t unitYux_init_event[];
extern const int32_t unitZYux_init_event[];
extern const int32_t unitXYux_init_event[];
extern const int32_t unitYux_common_init_event[];
extern const int32_t unitYux_unison_phase_event[];
extern const int32_t unitYux_attack_event[];
extern const int32_t unitYux_damage_event[];
extern const int32_t unitYux_wait_event[];
extern const int32_t unitYux_recovery_event[];
extern const int32_t unitYux_normal_attack_event[];
extern const int32_t unitYux_barrage_attack_event[];
extern const int32_t unitYux_barrage_attack_event_sub[];
extern const int32_t unitYux_barrage_attack_event_sub_nolast[];
extern const int32_t unitYux_barrage_attack_event_sub_last[];
extern const int32_t unitYux_barrage_attack_blur_event[];
extern const int32_t unitYux_attack_damage_check_event[];
extern const int32_t unitYux_attack_common_event_nohit[];
extern const int32_t unitYux_attack_common_event_hit[];
extern const int32_t unitYux_shot_miss_event[];
extern const int32_t unitYux_first_attack_pos_event[];
extern const int32_t unitYux_satellite_blur_off_event[];
extern const int32_t unitYux_satellite_blur_on_event[];

extern const int32_t unitMiniYux_init_event[];
extern const int32_t unitMiniZYux_init_event[];
extern const int32_t unitMiniXYux_init_event[];
extern const int32_t unitMiniYux_common_init_event[];
extern const int32_t unitMiniYux_unison_phase_event[];
extern const int32_t unitMiniYux_attack_event[];
extern const int32_t unitMiniYux_damage_event[];
extern const int32_t unitMiniYux_wait_event[];

EVT_DECLARE_USER_FUNC(battleGetBarriernSatelliteInitPos, 6)
EVT_DECLARE_USER_FUNC(_check_satellite, 1)
EVT_DECLARE_USER_FUNC(barriern_satellite_zanzou, 2)
EVT_DECLARE_USER_FUNC(barriern_satellite_zanzou2, 2)
EVT_DECLARE_USER_FUNC(barriern_zanzou, 1)
EVT_DECLARE_USER_FUNC(barriern_zanzou2, 1)
EVT_DECLARE_USER_FUNC(barriern_zanzou3, 1)
EVT_DECLARE_USER_FUNC(barriernSatelliteToOutside, 2)
EVT_DECLARE_USER_FUNC(barriernSatelliteToInside, 2)
EVT_DECLARE_USER_FUNC(outSplineTableSet, 2)

// Unit data.

int8_t unitYux_defense[] = { 0, 0, 0, 0, 0 };
int8_t unitYux_defense_attr[] = { 0, 0, 0, 0, 0 };
int8_t unitXYux_defense[] = { 1, 1, 1, 1, 1 };
int8_t unitXYux_defense_attr[] = { 0, 0, 0, 0, 0 };
int8_t unitMiniYux_defense[] = { 0, 0, 0, 0, 0 };
int8_t unitMiniYux_defense_attr[] = { 0, 0, 0, 0, 0 };

StatusVulnerability unitYux_status = {
    30,  80, 100,   0,  70, 100,   0,   0,
   100,  90, 100,  90, 100,  95,  70,   0,
    90, 100,  70, 100, 100,  95,
};
StatusVulnerability unitZYux_status = {
     20,  70,  95,   0,  60, 100, 100,   0,
    100,  85, 100,  85, 100,  90,  60,   0,
     80, 100,  60, 100, 100,  95,
};
StatusVulnerability unitXYux_status = {
      0,   0,   0,   0,   0,   0,   0,   0,
    100,   0, 100,   0, 100,   0,   0,   0,
      0, 100,   0, 100, 100,  10,
};
StatusVulnerability unitMiniYux_status = {
     80, 100, 100,   0,  90, 100,   0,   0,
    100,  90, 100,  90, 100,  95,  90,   0,
    100, 100,  90, 100, 100, 125,
};
StatusVulnerability unitMiniZYux_status = {
     70,  90,  95,   0,  80, 100, 100,   0,
    100,  85, 100,  85, 100,  90,  80,   0,
     95, 100,  80, 100, 100, 125,
};
StatusVulnerability unitMiniXYux_status = {
     70,  90,  90,   0,  10, 100, 100,   0,
    100,  80, 100,  80, 100,  85,  80,   0,
     90, 100,  80, 100, 100, 125,
};

PoseTableEntry unitYux_pose_table[] = {
    1, "N_1",
    2, "Y_1",
    9, "Y_1",
    5, "K_1",
    4, "X_1",
    3, "X_1",
    28, "S_1",
    29, "Q_1",
    30, "Q_1",
    31, "S_1",
    39, "D_1",
    42, "R_1",
    41, "R_1",
    40, "W_1",
    56, "X_1",
    57, "X_1",
    65, "T_1",
    69, "S_1",
};
PoseTableEntry unitMiniYux_pose_table[] = {
    1, "N_2",
    2, "Y_2",
    9, "Y_2",
    5, "K_2",
    4, "X_2",
    3, "X_2",
    28, "S_2",
    29, "Q_2",
    30, "Q_2",
    31, "S_2",
    39, "D_2",
    42, "R_2",
    41, "R_2",
    40, "W_2",
    56, "X_2",
    57, "X_2",
    69, "S_2",
};

DataTableEntry unitYux_data_table[] = {
    48, (void*)unitYux_first_attack_pos_event,
    0, nullptr,
};
DataTableEntry unitMiniYux_data_table[] = {
    0, nullptr,
};

BattleWeapon unitYux_weaponNormal = {
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
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH |
        AttackTargetClass_Flags::ENEMY_SELECT_SIDE_HOME,
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
BattleWeapon unitZYux_weaponNormal = {
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
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH |
        AttackTargetClass_Flags::ENEMY_SELECT_SIDE_HOME,
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
BattleWeapon unitXYux_weaponNormal = {
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
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH |
        AttackTargetClass_Flags::ENEMY_SELECT_SIDE_HOME,
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
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    .stop_chance = 80,
    .stop_time = 3,
    
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
BattleWeapon unitYux_weaponBarrage = {
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
    .damage_function_params = { 1, 0, 0, 0, 0, 0, 0, 0 },
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
BattleWeapon unitZYux_weaponBarrage = {
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
BattleWeapon unitXYux_weaponBarrage = {
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
    .stop_chance = 80,
    .stop_time = 3,
    
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

BattleUnitKindPart unitYux_parts[] = {
    {
        .index = 1,
        .name = "btl_un_barriern",
        .model_name = "c_baria",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 12.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 22.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitYux_defense,
        .defense_attr = unitYux_defense_attr,
        .attribute_flags = 0x0000'0009,  // Made Hammer/Shell targetable
        .counter_attribute_flags = 0,
        .pose_table = unitYux_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_barriern",
        .model_name = "c_baria",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitYux_defense,
        .defense_attr = unitYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitYux_pose_table,
    },
    {
        .index = 3,
        .name = "btl_un_barriern",
        .model_name = "c_baria",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitYux_defense,
        .defense_attr = unitYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitYux_pose_table,
    },
    {
        .index = 4,
        .name = "btl_un_barriern",
        .model_name = "c_baria",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitYux_defense,
        .defense_attr = unitYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitYux_pose_table,
    },
    {
        .index = 5,
        .name = "btl_un_barriern",
        .model_name = "c_baria",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitYux_defense,
        .defense_attr = unitYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitYux_pose_table,
    },
    {
        .index = 6,
        .name = "btl_un_barriern",
        .model_name = "c_baria",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitYux_defense,
        .defense_attr = unitYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitYux_pose_table,
    },
    {
        .index = 7,
        .name = "btl_un_barriern",
        .model_name = "c_baria",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 10,
        .unk_32 = 10,
        .base_alpha = 255,
        .defense = unitYux_defense,
        .defense_attr = unitYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitYux_pose_table,
    },
    {
        .index = 8,
        .name = "btl_un_barriern",
        .model_name = "c_baria",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 10,
        .unk_32 = 10,
        .base_alpha = 255,
        .defense = unitYux_defense,
        .defense_attr = unitYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitYux_pose_table,
    },
    {
        .index = 9,
        .name = "btl_un_barriern",
        .model_name = "c_baria",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 10,
        .unk_32 = 10,
        .base_alpha = 255,
        .defense = unitYux_defense,
        .defense_attr = unitYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitYux_pose_table,
    },
    {
        .index = 10,
        .name = "btl_un_barriern",
        .model_name = "c_baria",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 10,
        .unk_32 = 10,
        .base_alpha = 255,
        .defense = unitYux_defense,
        .defense_attr = unitYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitYux_pose_table,
    },
    {
        .index = 11,
        .name = "btl_un_barriern",
        .model_name = "c_baria",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 10,
        .unk_32 = 10,
        .base_alpha = 255,
        .defense = unitYux_defense,
        .defense_attr = unitYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitYux_pose_table,
    },
};
BattleUnitKindPart unitZYux_parts[] = {
    {
        .index = 1,
        .name = "btl_un_barriern_z",
        .model_name = "c_baria_z",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 12.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 22.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitYux_defense,
        .defense_attr = unitYux_defense_attr,
        .attribute_flags = 0x0000'0009,  // Made Hammer/Shell targetable
        .counter_attribute_flags = 0,
        .pose_table = unitYux_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_barriern_z",
        .model_name = "c_baria_z",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitYux_defense,
        .defense_attr = unitYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitYux_pose_table,
    },
    {
        .index = 3,
        .name = "btl_un_barriern_z",
        .model_name = "c_baria_z",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitYux_defense,
        .defense_attr = unitYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitYux_pose_table,
    },
    {
        .index = 4,
        .name = "btl_un_barriern_z",
        .model_name = "c_baria_z",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitYux_defense,
        .defense_attr = unitYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitYux_pose_table,
    },
    {
        .index = 5,
        .name = "btl_un_barriern_z",
        .model_name = "c_baria_z",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitYux_defense,
        .defense_attr = unitYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitYux_pose_table,
    },
    {
        .index = 6,
        .name = "btl_un_barriern_z",
        .model_name = "c_baria_z",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitYux_defense,
        .defense_attr = unitYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitYux_pose_table,
    },
    {
        .index = 7,
        .name = "btl_un_barriern_z",
        .model_name = "c_baria_z",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 10,
        .unk_32 = 10,
        .base_alpha = 255,
        .defense = unitYux_defense,
        .defense_attr = unitYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitYux_pose_table,
    },
    {
        .index = 8,
        .name = "btl_un_barriern_z",
        .model_name = "c_baria_z",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 10,
        .unk_32 = 10,
        .base_alpha = 255,
        .defense = unitYux_defense,
        .defense_attr = unitYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitYux_pose_table,
    },
    {
        .index = 9,
        .name = "btl_un_barriern_z",
        .model_name = "c_baria_z",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 10,
        .unk_32 = 10,
        .base_alpha = 255,
        .defense = unitYux_defense,
        .defense_attr = unitYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitYux_pose_table,
    },
    {
        .index = 10,
        .name = "btl_un_barriern_z",
        .model_name = "c_baria_z",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 10,
        .unk_32 = 10,
        .base_alpha = 255,
        .defense = unitYux_defense,
        .defense_attr = unitYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitYux_pose_table,
    },
    {
        .index = 11,
        .name = "btl_un_barriern_z",
        .model_name = "c_baria_z",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 10,
        .unk_32 = 10,
        .base_alpha = 255,
        .defense = unitYux_defense,
        .defense_attr = unitYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitYux_pose_table,
    },
};
BattleUnitKindPart unitXYux_parts[] = {
    {
        .index = 1,
        .name = "btl_un_barriern_custom",
        .model_name = "c_baria_c",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 12.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 22.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitXYux_defense,
        .defense_attr = unitXYux_defense_attr,
        .attribute_flags = 0x0000'0009,  // Made Hammer/Shell targetable
        .counter_attribute_flags = 0,
        .pose_table = unitYux_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_barriern_custom",
        .model_name = "c_baria_c",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitXYux_defense,
        .defense_attr = unitXYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitYux_pose_table,
    },
    {
        .index = 3,
        .name = "btl_un_barriern_custom",
        .model_name = "c_baria_c",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitXYux_defense,
        .defense_attr = unitXYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitYux_pose_table,
    },
    {
        .index = 4,
        .name = "btl_un_barriern_custom",
        .model_name = "c_baria_c",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitXYux_defense,
        .defense_attr = unitXYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitYux_pose_table,
    },
    {
        .index = 5,
        .name = "btl_un_barriern_custom",
        .model_name = "c_baria_c",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitXYux_defense,
        .defense_attr = unitXYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitYux_pose_table,
    },
    {
        .index = 6,
        .name = "btl_un_barriern_custom",
        .model_name = "c_baria_c",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitXYux_defense,
        .defense_attr = unitXYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitYux_pose_table,
    },
    {
        .index = 7,
        .name = "btl_un_barriern_custom",
        .model_name = "c_baria_c",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 10,
        .unk_32 = 10,
        .base_alpha = 255,
        .defense = unitXYux_defense,
        .defense_attr = unitXYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitYux_pose_table,
    },
    {
        .index = 8,
        .name = "btl_un_barriern_custom",
        .model_name = "c_baria_c",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 10,
        .unk_32 = 10,
        .base_alpha = 255,
        .defense = unitXYux_defense,
        .defense_attr = unitXYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitYux_pose_table,
    },
    {
        .index = 9,
        .name = "btl_un_barriern_custom",
        .model_name = "c_baria_c",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 10,
        .unk_32 = 10,
        .base_alpha = 255,
        .defense = unitXYux_defense,
        .defense_attr = unitXYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitYux_pose_table,
    },
    {
        .index = 10,
        .name = "btl_un_barriern_custom",
        .model_name = "c_baria_c",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 10,
        .unk_32 = 10,
        .base_alpha = 255,
        .defense = unitXYux_defense,
        .defense_attr = unitXYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitYux_pose_table,
    },
    {
        .index = 11,
        .name = "btl_un_barriern_custom",
        .model_name = "c_baria_c",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 10,
        .unk_32 = 10,
        .base_alpha = 255,
        .defense = unitXYux_defense,
        .defense_attr = unitXYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitYux_pose_table,
    },
};
BattleUnitKindPart unitMiniYux_parts[] = {
    {
        .index = 1,
        .name = "btl_un_barriern_satellite",
        .model_name = "c_baria",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 5.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 10.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitMiniYux_defense,
        .defense_attr = unitMiniYux_defense_attr,
        .attribute_flags = 0x0000'0009 |
            PartsAttribute_Flags::HAMMERLIKE_CANNOT_TARGET |
            PartsAttribute_Flags::SHELLTOSSLIKE_CANNOT_TARGET,
        .counter_attribute_flags = 0,
        .pose_table = unitMiniYux_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_barriern_satellite",
        .model_name = "c_baria",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 5.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 10.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitMiniYux_defense,
        .defense_attr = unitMiniYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitMiniYux_pose_table,
    },
    {
        .index = 3,
        .name = "btl_un_barriern_satellite",
        .model_name = "c_baria",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 5.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 10.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitMiniYux_defense,
        .defense_attr = unitMiniYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitMiniYux_pose_table,
    },
    {
        .index = 4,
        .name = "btl_un_barriern_satellite",
        .model_name = "c_baria",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 5.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 10.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitMiniYux_defense,
        .defense_attr = unitMiniYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitMiniYux_pose_table,
    },
    {
        .index = 5,
        .name = "btl_un_barriern_satellite",
        .model_name = "c_baria",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 5.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 10.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitMiniYux_defense,
        .defense_attr = unitMiniYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitMiniYux_pose_table,
    },
    {
        .index = 6,
        .name = "btl_un_barriern_satellite",
        .model_name = "c_baria",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 5.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 10.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitMiniYux_defense,
        .defense_attr = unitMiniYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitMiniYux_pose_table,
    },
};
BattleUnitKindPart unitMiniZYux_parts[] = {
    {
        .index = 1,
        .name = "btl_un_barriern_satellite_z",
        .model_name = "c_baria_z",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 5.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 10.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitMiniYux_defense,
        .defense_attr = unitMiniYux_defense_attr,
        .attribute_flags = 0x0000'0009 |
            PartsAttribute_Flags::HAMMERLIKE_CANNOT_TARGET |
            PartsAttribute_Flags::SHELLTOSSLIKE_CANNOT_TARGET,
        .counter_attribute_flags = 0,
        .pose_table = unitMiniYux_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_barriern_satellite_z",
        .model_name = "c_baria_z",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 5.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 10.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitMiniYux_defense,
        .defense_attr = unitMiniYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitMiniYux_pose_table,
    },
    {
        .index = 3,
        .name = "btl_un_barriern_satellite_z",
        .model_name = "c_baria_z",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 5.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 10.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitMiniYux_defense,
        .defense_attr = unitMiniYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitMiniYux_pose_table,
    },
    {
        .index = 4,
        .name = "btl_un_barriern_satellite_z",
        .model_name = "c_baria_z",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 5.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 10.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitMiniYux_defense,
        .defense_attr = unitMiniYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitMiniYux_pose_table,
    },
    {
        .index = 5,
        .name = "btl_un_barriern_satellite_z",
        .model_name = "c_baria_z",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 5.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 10.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitMiniYux_defense,
        .defense_attr = unitMiniYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitMiniYux_pose_table,
    },
    {
        .index = 6,
        .name = "btl_un_barriern_satellite_z",
        .model_name = "c_baria_z",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 5.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 10.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitMiniYux_defense,
        .defense_attr = unitMiniYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitMiniYux_pose_table,
    },
};
BattleUnitKindPart unitMiniXYux_parts[] = {
    {
        .index = 1,
        .name = "btl_un_barriern_custom_satellite",
        .model_name = "c_baria_c",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 5.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 10.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitMiniYux_defense,
        .defense_attr = unitMiniYux_defense_attr,
        .attribute_flags = 0x0000'0009 |
            PartsAttribute_Flags::HAMMERLIKE_CANNOT_TARGET |
            PartsAttribute_Flags::SHELLTOSSLIKE_CANNOT_TARGET,
        .counter_attribute_flags = 0,
        .pose_table = unitMiniYux_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_barriern_custom_satellite",
        .model_name = "c_baria_c",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 5.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 10.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitMiniYux_defense,
        .defense_attr = unitMiniYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitMiniYux_pose_table,
    },
    {
        .index = 3,
        .name = "btl_un_barriern_custom_satellite",
        .model_name = "c_baria_c",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 5.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 10.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitMiniYux_defense,
        .defense_attr = unitMiniYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitMiniYux_pose_table,
    },
    {
        .index = 4,
        .name = "btl_un_barriern_custom_satellite",
        .model_name = "c_baria_c",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 5.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 10.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitMiniYux_defense,
        .defense_attr = unitMiniYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitMiniYux_pose_table,
    },
    {
        .index = 5,
        .name = "btl_un_barriern_custom_satellite",
        .model_name = "c_baria_c",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 5.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 10.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitMiniYux_defense,
        .defense_attr = unitMiniYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitMiniYux_pose_table,
    },
    {
        .index = 6,
        .name = "btl_un_barriern_custom_satellite",
        .model_name = "c_baria_c",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 5.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 10.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitMiniYux_defense,
        .defense_attr = unitMiniYux_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitMiniYux_pose_table,
    },
};

BattleUnitKind unit_Yux = {
    .unit_type = BattleUnitType::YUX,
    .unit_name = "btl_un_barriern",
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
    .width = 40,
    .height = 40,
    .hit_offset = { 0, 20 },
    .center_offset = { 0.0f, 0.0f, 0.0f },
    .hp_gauge_offset = { 0, -40 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 20.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 20.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 10.0f, 10.0f, 0.0f },
    .cut_base_offset = { 0.0f, 0.0f, 0.0f },
    .cut_width = 40.0f,
    .cut_height = 40.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = -1,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_BARRI_DAMAGED2",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,   // Not floating, so grounded partners can hit them
    .status_vulnerability = &unitYux_status,
    .num_parts = 11,
    .parts = unitYux_parts,
    .init_evt_code = (void*)unitYux_init_event,
    .data_table = unitYux_data_table,
};
BattleUnitKind unit_ZYux = {
    .unit_type = BattleUnitType::Z_YUX,
    .unit_name = "btl_un_barriern_z",
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
    .width = 50,
    .height = 50,
    .hit_offset = { 0, 25 },
    .center_offset = { 0.0f, 0.0f, 0.0f },
    .hp_gauge_offset = { 0, -40 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 25.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 25.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 10.0f, 10.0f, 0.0f },
    .cut_base_offset = { 0.0f, 0.0f, 0.0f },
    .cut_width = 50.0f,
    .cut_height = 50.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = -1,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_BARRI_DAMAGED2",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,   // Not floating, so grounded partners can hit them
    .status_vulnerability = &unitZYux_status,
    .num_parts = 11,
    .parts = unitZYux_parts,
    .init_evt_code = (void*)unitZYux_init_event,
    .data_table = unitYux_data_table,
};
BattleUnitKind unit_XYux = {
    .unit_type = BattleUnitType::X_YUX,
    .unit_name = "btl_un_barriern_custom",
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
    .width = 50,
    .height = 50,
    .hit_offset = { 0, 25 },
    .center_offset = { 0.0f, 0.0f, 0.0f },
    .hp_gauge_offset = { 0, -40 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 25.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 25.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 10.0f, 10.0f, 0.0f },
    .cut_base_offset = { 0.0f, 0.0f, 0.0f },
    .cut_width = 50.0f,
    .cut_height = 50.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = -1,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_BARRI_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,   // Not floating, so grounded partners can hit them
    .status_vulnerability = &unitXYux_status,
    .num_parts = 11,
    .parts = unitXYux_parts,
    .init_evt_code = (void*)unitXYux_init_event,
    .data_table = unitYux_data_table,
};
BattleUnitKind unit_MiniYux = {
    .unit_type = BattleUnitType::MINI_YUX,
    .unit_name = "btl_un_barriern_satellite",
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
    .width = 16,
    .height = 16,
    .hit_offset = { 0, 8 },
    .center_offset = { 0.0f, 0.0f, 0.0f },
    .hp_gauge_offset = { -3, -6 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 8.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 8.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 5.0f, 6.0f, 0.0f },
    .cut_base_offset = { 0.0f, 0.0f, 0.0f },
    .cut_width = 16.0f,
    .cut_height = 16.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = -1,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_BARRI_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::UNQUAKEABLE,
    .status_vulnerability = &unitMiniYux_status,
    .num_parts = 6,
    .parts = unitMiniYux_parts,
    .init_evt_code = (void*)unitMiniYux_init_event,
    .data_table = unitMiniYux_data_table,
};
BattleUnitKind unit_MiniZYux = {
    .unit_type = BattleUnitType::MINI_Z_YUX,
    .unit_name = "btl_un_barriern_satellite_z",
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
    .width = 16,
    .height = 16,
    .hit_offset = { 0, 8 },
    .center_offset = { 0.0f, 0.0f, 0.0f },
    .hp_gauge_offset = { -3, -6 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 8.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 8.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 5.0f, 6.0f, 0.0f },
    .cut_base_offset = { 0.0f, 0.0f, 0.0f },
    .cut_width = 16.0f,
    .cut_height = 16.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = -1,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_BARRI_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::UNQUAKEABLE,
    .status_vulnerability = &unitMiniZYux_status,
    .num_parts = 6,
    .parts = unitMiniZYux_parts,
    .init_evt_code = (void*)unitMiniZYux_init_event,
    .data_table = unitMiniYux_data_table,
};
BattleUnitKind unit_MiniXYux = {
    .unit_type = BattleUnitType::MINI_X_YUX,
    .unit_name = "btl_un_barriern_custom_satellite",
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
    .width = 16,
    .height = 16,
    .hit_offset = { 0, 8 },
    .center_offset = { 0.0f, 0.0f, 0.0f },
    .hp_gauge_offset = { -3, -6 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 8.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 8.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 5.0f, 6.0f, 0.0f },
    .cut_base_offset = { 0.0f, 0.0f, 0.0f },
    .cut_width = 16.0f,
    .cut_height = 16.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = -1,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_BARRI_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::UNQUAKEABLE,
    .status_vulnerability = &unitMiniXYux_status,
    .num_parts = 6,
    .parts = unitMiniXYux_parts,
    .init_evt_code = (void*)unitMiniXYux_init_event,
    .data_table = unitMiniYux_data_table,
};

const BattleUnitSetup unitMiniYux_spawn_entry = {
    .unit_kind_params = &unit_MiniYux,
    .alliance = 1,
    .attack_phase = 0x04000004,
    .position = { 1000.0f, 0.0f, 1000.0f },
    .addl_target_offset_x = 0,
    .unit_work = { 0, 0, 0, 0 },
    .item_drop_table = nullptr,
};
const BattleUnitSetup unitMiniZYux_spawn_entry = {
    .unit_kind_params = &unit_MiniZYux,
    .alliance = 1,
    .attack_phase = 0x04000004,
    .position = { 1000.0f, 0.0f, 1000.0f },
    .addl_target_offset_x = 0,
    .unit_work = { 0, 0, 0, 0 },
    .item_drop_table = nullptr,
};
const BattleUnitSetup unitMiniXYux_spawn_entry = {
    .unit_kind_params = &unit_MiniXYux,
    .alliance = 1,
    .attack_phase = 0x04000004,
    .position = { 1000.0f, 0.0f, 1000.0f },
    .addl_target_offset_x = 0,
    .unit_work = { 0, 0, 0, 0 },
    .item_drop_table = nullptr,
};

// Function / USER_FUNC definitions.

EVT_DEFINE_USER_FUNC(battleGetBarriernSatelliteInitPos) {
    
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[0]));
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, unit_idx);
    
    int32_t yux_count = evtGetValue(evt, evt->evtArguments[1]);
    int32_t yux_idx = evtGetValue(evt, evt->evtArguments[2]);
    
    int32_t yux_position_offset = yux_idx;
    for (int32_t i = 1; i < yux_count; ++i) yux_position_offset += i;
    
    const gc::vec3& yux_position = kYuxPositions[yux_position_offset];
    
    evtSetValue(evt, evt->evtArguments[3], yux_position.x * unit->movement_params.face_direction);
    evtSetValue(evt, evt->evtArguments[4], yux_position.y);
    evtSetValue(evt, evt->evtArguments[5], yux_position.z);
    
    return 2;
}

EVT_DEFINE_USER_FUNC(_check_satellite) {
    
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(evt, -2);
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, unit_idx);
    if (!unit) return 2;
    
    bool has_shield = false;
    
    for (int32_t i = 0; i < 4; ++i) {
        int32_t sub_idx = unit->unit_work[UW_Main_MiniIds + i];
        if (sub_idx != -1) {
            auto* sub = ttyd::battle::BattleGetUnitPtr(g_BattleWork, sub_idx);
            if (sub == nullptr) {
                unit->unit_work[UW_Main_MiniIds + i] = -1;
            } else {
                has_shield = true;
            }
        }
    }
    
    if (has_shield) {
        int32_t part_idx = ttyd::battle_unit::BtlUnit_GetBodyPartsId(unit);
        auto* part = ttyd::battle_unit::BtlUnit_GetPartsPtr(unit, part_idx);
        
        // If shield not already up
        if (!(part->part_attribute_flags & PartsAttribute_Flags::UNK_2000_0000)) {
            ttyd::battle_unit::BtlUnit_snd_se(unit, "SFX_ENM_BARRI_BARRIER1", EVT_NULLPTR, 0);
            
            auto* eff = ttyd::eff_syuryou::effSyuryouEntry(0.0, -1000.0, 0.0, 1, 1000);
            unit->unit_work[UW_Main_BarrierEff] = reinterpret_cast<uint32_t>(eff);
        }
        
        part->part_attribute_flags |= PartsAttribute_Flags::UNK_2000_0000;
        evtSetValue(evt, evt->evtArguments[0], 1);
        
        if (unit->unit_work[UW_Main_BarrierEff] != 0) {
            auto* eff = reinterpret_cast<EffEntry*>(unit->unit_work[UW_Main_BarrierEff]);
            void* eff_data = eff->eff_work;
            
            gc::vec3 pos;
            ttyd::battle_unit::BtlUnit_GetPos(unit, &pos.x, &pos.y, &pos.z);
            
            auto& eff_pos = *reinterpret_cast<gc::vec3*>(
                reinterpret_cast<uintptr_t>(eff_data) + 4);
            auto& eff_scale = *reinterpret_cast<float*>(
                reinterpret_cast<uintptr_t>(eff_data) + 0x10);
                
            float unit_scale = unit->true_kind == BattleUnitType::YUX ? 0.8f : 1.0f;
                
            eff_pos.x = pos.x;
            eff_pos.y = pos.y - unit_scale * 32.0f * unit->unk_scale;
            eff_pos.z = pos.z + 10.0f;
            eff_scale = unit->unk_scale * 1.8f * unit_scale;
        }
    } else {
        int32_t part_idx = ttyd::battle_unit::BtlUnit_GetBodyPartsId(unit);
        auto* part = ttyd::battle_unit::BtlUnit_GetPartsPtr(unit, part_idx);
        
        // If shield not already down
        if ((part->part_attribute_flags & PartsAttribute_Flags::UNK_2000_0000)) {
            ttyd::effdrv::effDelete(
                reinterpret_cast<EffEntry*>(unit->unit_work[UW_Main_BarrierEff]));
            unit->unit_work[UW_Main_BarrierEff] = 0;
            
            ttyd::battle_unit::BtlUnit_snd_se(unit, "SFX_ENM_BARRI_BARRIER_BREAK1", EVT_NULLPTR, 0);
            
            gc::vec3 pos;
            ttyd::battle_unit::BtlUnit_GetPos(unit, &pos.x, &pos.y, &pos.z);
            
            for (int32_t i = 0; i < 20; ++i) {
                int32_t w = ttyd::battle_unit::BtlUnit_GetWidth(unit) + 15;
                int32_t h = ttyd::battle_unit::BtlUnit_GetHeight(unit) + 15;
                ttyd::eff_glass_n64::effGlassN64Entry(
                    pos.x + ttyd::system::irand(w) - w/2,
                    pos.y + ttyd::system::irand(h) - h/2,
                    pos.z, 1.0f, 2, 60);
            }
        }
        
        part->part_attribute_flags &= ~PartsAttribute_Flags::UNK_2000_0000;
        evtSetValue(evt, evt->evtArguments[0], 0);
    }
    
    return 2;
}

int32_t barriern_satellite_zanzou_sub(
    int isFirstCall, BattleWorkUnit* unit, const float* image_scale_params, int timer_idx, int mode) {
        
    int32_t result = 0;
    int32_t interp_mode = 4;
    float image_scale_start[5];
    float image_scale_end[5];
    float main_scale_end;
    float images_z_offset;
    int32_t alpha_start, alpha_end;
    int32_t frame_count;
  
    float unit_scale = unit->true_kind == BattleUnitType::MINI_YUX ? 0.8f : 1.0f;
    
    if (mode == 0) {
        for (int32_t i = 0; i < 5; ++i) {
            image_scale_start[i] = unit_scale * image_scale_params[i];
            image_scale_end[i] = unit_scale * 0.75f;
        }
      
        frame_count = 36;
        alpha_start = 0;
        alpha_end = 64;
        images_z_offset = 0.5f;
        main_scale_end = unit_scale * 0.75f;
        interp_mode = 4;
    } else {
        for (int32_t i = 0; i < 5; ++i) {
            image_scale_start[i] = unit_scale;
            image_scale_end[i] = unit_scale * image_scale_params[i];
        }
      
        frame_count = 8;
        alpha_start = 64;
        alpha_end = 0;
        images_z_offset = -0.5f;
        main_scale_end = unit_scale;
        interp_mode = 1;
      
        if (mode == 2) frame_count = 36;
    }
  
    if (isFirstCall) {
        g_FlashTimers[timer_idx] = 0;
      
        gc::vec3 pos;
        ttyd::battle_unit::BtlUnit_GetPos(unit, &pos.x, &pos.y, &pos.z);
        auto* part = ttyd::battle_unit::BtlUnit_GetPartsPtr(unit, 1);
        ttyd::battle_unit::BtlUnit_SetPartsScale(
            part, unit_scale, unit_scale, 1.0f);
      
        for (int32_t i = 0; i < 5; ++i) {
            auto* part = ttyd::battle_unit::BtlUnit_GetPartsPtr(unit, i + 2);
            ttyd::battle_unit::BtlUnit_SetPartsPos(
                part, pos.x, pos.y, pos.z + images_z_offset);
            ttyd::battle_unit::BtlUnit_SetPartsScale(
                part, image_scale_start[i], image_scale_start[i], 1.0f);
            ttyd::battle_unit::BtlUnit_SetPartsRotate(
                part, 0.0f, 0.0f, 0.0f);
            ttyd::battle_unit::BtlUnit_SetAnim(part, "Z_2");
            part->color.a = alpha_start;
            part->part_attribute_flags &= ~0x100'0000;
        }
    }
  
    auto* part = ttyd::battle_unit::BtlUnit_GetPartsPtr(unit, 1);
    double scale = ttyd::system::intplGetValue(
        unit_scale, main_scale_end, interp_mode, g_FlashTimers[0], frame_count);
    ttyd::battle_unit::BtlUnit_SetPartsScale(part, scale, scale, 1.0f);
  
    for (int32_t i = 0; i < 5; ++i) {
        auto* part = ttyd::battle_unit::BtlUnit_GetPartsPtr(unit, i + 2);
        double scale = ttyd::system::intplGetValue(
            image_scale_start[i], image_scale_end[i], interp_mode, 
            g_FlashTimers[timer_idx], frame_count);
        ttyd::battle_unit::BtlUnit_SetPartsScale(part, scale, scale, 1.0f);
        
        part->color.a = ttyd::system::intplGetValue(
            alpha_start, alpha_end, 0, g_FlashTimers[timer_idx], frame_count);
    }
  
    if (g_FlashTimers[timer_idx] < frame_count) {
        ++g_FlashTimers[timer_idx];
        result = 0;
    } else {
        auto* part = ttyd::battle_unit::BtlUnit_GetPartsPtr(unit, 1);
        ttyd::battle_unit::BtlUnit_SetPartsScale(
            part, unit_scale, unit_scale, 1.0f);
        
        for (int32_t i = 0; i < 5; ++i) {
            auto* part = ttyd::battle_unit::BtlUnit_GetPartsPtr(unit, i + 2);
            part->part_attribute_flags |= 0x100'0000;
        }
        
        result = 2;
    }
    return result;
}

EVT_DEFINE_USER_FUNC(barriern_satellite_zanzou2) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[0]));
    int32_t yux_idx = evtGetValue(evt, evt->evtArguments[1]);
    
    auto* unit = ttyd::battle::BattleGetUnitPtr(
        g_BattleWork, unit_idx);
    auto* yux_unit = ttyd::battle::BattleGetUnitPtr(
        g_BattleWork, unit->unit_work[UW_Main_MiniIds + yux_idx]);
    return barriern_satellite_zanzou_sub(
        isFirstCall, yux_unit, kAfterimageScale, yux_idx + 1, 1);
}

EVT_DEFINE_USER_FUNC(barriern_satellite_zanzou) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[0]));
    int32_t yux_idx = evtGetValue(evt, evt->evtArguments[1]);
    
    auto* unit = ttyd::battle::BattleGetUnitPtr(
        g_BattleWork, unit_idx);
    auto* yux_unit = ttyd::battle::BattleGetUnitPtr(
        g_BattleWork, unit->unit_work[UW_Main_MiniIds + yux_idx]);
    return barriern_satellite_zanzou_sub(
        isFirstCall, yux_unit, kAfterimageScale, yux_idx + 1, 0);
}

int32_t barriern_zanzou_sub(
    int isFirstCall, BattleWorkUnit* unit, const float* image_scale_params, int mode) {
        
    int32_t result = 0;
    int32_t interp_mode = 4;
    float image_scale_start[5];
    float image_scale_end[5];
    float main_scale_end;
    float images_z_offset;
    int32_t alpha_start, alpha_end;
    int32_t frame_count;
  
    float unit_scale = unit->true_kind == BattleUnitType::YUX ? 0.8f : 1.0f;
  
    if (mode == 0) {
        for (int32_t i = 0; i < 5; ++i) {
            image_scale_start[i] = unit_scale * image_scale_params[i];
            image_scale_end[i] = unit_scale * 0.75f;
        }
      
        frame_count = 36;
        alpha_start = 0;
        alpha_end = 64;
        images_z_offset = 0.5f;
        main_scale_end = unit_scale * 0.75f;
        interp_mode = 4;
    } else {
        for (int32_t i = 0; i < 5; ++i) {
            image_scale_start[i] = unit_scale;
            image_scale_end[i] = unit_scale * image_scale_params[i];
        }
      
        frame_count = 8;
        alpha_start = 64;
        alpha_end = 0;
        images_z_offset = -0.5f;
        main_scale_end = unit_scale;
        interp_mode = 1;
      
        if (mode == 2) frame_count = 36;
    }
  
    if (isFirstCall) {
        g_FlashTimers[0] = 0;
      
        gc::vec3 pos;
        ttyd::battle_unit::BtlUnit_GetPos(unit, &pos.x, &pos.y, &pos.z);
        auto* part = ttyd::battle_unit::BtlUnit_GetPartsPtr(unit, 1);
        ttyd::battle_unit::BtlUnit_SetPartsScale(
            part, unit_scale, unit_scale, 1.0f);
      
        for (int32_t i = 0; i < 5; ++i) {
            auto* part = ttyd::battle_unit::BtlUnit_GetPartsPtr(unit, i + 2);
            ttyd::battle_unit::BtlUnit_SetPartsPos(
                part, pos.x, pos.y, pos.z + images_z_offset);
            ttyd::battle_unit::BtlUnit_SetPartsScale(
                part, image_scale_start[i], image_scale_start[i], 1.0f);
            part->color.a = alpha_start;
            part->part_attribute_flags &= ~0x100'0000;
        }
    }
    
    auto* part = ttyd::battle_unit::BtlUnit_GetPartsPtr(unit, 1);
    double scale = ttyd::system::intplGetValue(
        unit_scale, main_scale_end, interp_mode, g_FlashTimers[0], frame_count);
    ttyd::battle_unit::BtlUnit_SetPartsScale(part, scale, scale, 1.0f);
    
    for (int32_t i = 0; i < 5; ++i) {
        auto* part = ttyd::battle_unit::BtlUnit_GetPartsPtr(unit, i + 2);
        double scale = ttyd::system::intplGetValue(
            image_scale_start[i], image_scale_end[i], 
            interp_mode, g_FlashTimers[0], frame_count);
        ttyd::battle_unit::BtlUnit_SetPartsScale(part, scale, scale, 1.0f);
        
        part->color.a = ttyd::system::intplGetValue(
            alpha_start, alpha_end, 0, g_FlashTimers[0], frame_count);
    }
    
    if (g_FlashTimers[0] < frame_count) {
        ++g_FlashTimers[0];
        result = 0;
    } else {
        auto* part = ttyd::battle_unit::BtlUnit_GetPartsPtr(unit, 1);
        ttyd::battle_unit::BtlUnit_SetPartsScale(
            part, unit_scale, unit_scale, 1.0f);
        
        for (int32_t i = 0; i < 5; ++i) {
            auto* part = ttyd::battle_unit::BtlUnit_GetPartsPtr(unit, i + 2);
            part->part_attribute_flags |= 0x100'0000;
        }
        
        result = 2;
    }
    
    return result;
}

EVT_DEFINE_USER_FUNC(barriern_zanzou3) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[0]));
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, unit_idx);
    return barriern_zanzou_sub(isFirstCall, unit, kAfterimageScale, 2);
}

EVT_DEFINE_USER_FUNC(barriern_zanzou2) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[0]));
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, unit_idx);
    return barriern_zanzou_sub(isFirstCall, unit, kAfterimageScale, 1);
}

EVT_DEFINE_USER_FUNC(barriern_zanzou) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[0]));
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, unit_idx);
    return barriern_zanzou_sub(isFirstCall, unit, kAfterimageScale, 0);
}

struct SatelliteInOutWork {
    int32_t unk_00;
    const float* spline_table;
    float spline_work_1[5];
    float spline_work_2[15];
    int32_t interp_timer;
};

static_assert(sizeof(SatelliteInOutWork) == 0x5c);

// Shared logic for handling interpolation when mini-Yuxes move in/out.
int32_t barriernSatelliteTo_Main(BattleWorkUnit* unit) {
    bool still_working = false;
    for (int32_t i = 0; i < 4; ++i) {
        auto* work = reinterpret_cast<SatelliteInOutWork*>(unit->unit_work[UW_Main_MiniInOutWork + i]);
        if (work == nullptr) {
            continue;
        } else if (work->interp_timer < 60) {
            double spline_dist = ttyd::system::intplGetValue(
                0.0, 1.0, 4, work->interp_timer, 60);
            gc::vec3 spline_result;
            ttyd::mapdrv::spline_getvalue(
                spline_dist, &spline_result, work->unk_00, work->spline_table, 
                &work->spline_work_1[0], &work->spline_work_2[0]);
                
            auto* yux = ttyd::battle::BattleGetUnitPtr(
                g_BattleWork, unit->unit_work[UW_Main_MiniIds + i]);
            if (yux != nullptr) {
                gc::vec3 pos;
                ttyd::battle_unit::BtlUnit_GetPos(unit, &pos.x, &pos.y, &pos.z);
                
                ttyd::battle_unit::BtlUnit_SetPos(yux,
                    pos.x + spline_result.x * unit->movement_params.face_direction,
                    pos.y + spline_result.y,
                    pos.z + spline_result.z);
                ttyd::battle_unit::BtlUnit_SetHomePos(yux,
                    pos.x + spline_result.x * unit->movement_params.face_direction,
                    pos.y + spline_result.y,
                    pos.z + spline_result.z);
                    
                ++work->interp_timer;
                still_working = true;
            }
        } else {
            ttyd::battle::BattleFree(work);
            unit->unit_work[UW_Main_MiniInOutWork + i] = 0;
        }
    }
    return still_working ? 0 : 2;
}

EVT_DEFINE_USER_FUNC(barriernSatelliteToOutside) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[0]));
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, unit_idx);
    
    int32_t yux_count = evtGetValue(evt, evt->evtArguments[1]);
    int32_t lookup_offset = 0;
    for (int32_t i = 1; i < yux_count; ++i) lookup_offset += i;
    
    if (isFirstCall) {
        // Nothing to do.
        if (yux_count == 0 || yux_count > 4) return 2;
        
        for (int32_t i = 0; i < 4; ++i) {
            int32_t yux_ent_idx = unit->unit_work[UW_Main_MiniIds + i];
            if (yux_ent_idx == -1) continue;
            
            auto* work = reinterpret_cast<SatelliteInOutWork*>(
                ttyd::battle::BattleAlloc(sizeof(SatelliteInOutWork)));
            unit->unit_work[UW_Main_MiniInOutWork + i] = reinterpret_cast<uint32_t>(work);
            
            work->unk_00 = 5;
            work->spline_table = splineLookup[lookup_offset + i];
            work->interp_timer = 0;
            ttyd::mapdrv::spline_maketable(
                5, work->spline_table, &work->spline_work_1[0], &work->spline_work_2[0]);
                
            auto* yux = ttyd::battle::BattleGetUnitPtr(g_BattleWork, yux_ent_idx);
            auto* part = ttyd::battle_unit::BtlUnit_GetPartsPtr(yux, 1);
            part->parts_work[PW_Mini_SplineTable] = reinterpret_cast<uint32_t>(work->spline_table);
        }
    }
    
    return barriernSatelliteTo_Main(unit);
}

EVT_DEFINE_USER_FUNC(barriernSatelliteToInside) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[0]));
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, unit_idx);
    
    int32_t yux_count = evtGetValue(evt, evt->evtArguments[1]);
    int32_t lookup_offset = 0;
    for (int32_t i = 1; i < yux_count; ++i) lookup_offset += i;
    
    if (isFirstCall) {
        for (int32_t i = 0; i < 4; ++i) {
            int32_t yux_ent_idx = unit->unit_work[UW_Main_MiniIds + i];
            if (yux_ent_idx == -1) continue;
            
            auto* yux = ttyd::battle::BattleGetUnitPtr(g_BattleWork, yux_ent_idx);
            auto* part = ttyd::battle_unit::BtlUnit_GetPartsPtr(yux, 1);
            
            auto* work = reinterpret_cast<SatelliteInOutWork*>(
                ttyd::battle::BattleAlloc(sizeof(SatelliteInOutWork)));
            unit->unit_work[UW_Main_MiniInOutWork + i] = reinterpret_cast<uint32_t>(work);
            
            work->unk_00 = 5;
            // Use the "in" spline table corresponding to the "out" one.
            work->spline_table = reinterpret_cast<float*>(part->parts_work[PW_Mini_SplineTable]) + 15;
            work->interp_timer = 0;
            ttyd::mapdrv::spline_maketable(
                5, work->spline_table, &work->spline_work_1[0], &work->spline_work_2[0]);
                
            part->parts_work[PW_Mini_SplineTable] = reinterpret_cast<uint32_t>(work->spline_table);
        }
    }
    
    return barriernSatelliteTo_Main(unit);
}

EVT_DEFINE_USER_FUNC(outSplineTableSet) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[0]));
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, unit_idx);
    
    int32_t yux_count = evtGetValue(evt, evt->evtArguments[1]);
    int32_t lookup_offset = 0;
    for (int32_t i = 1; i < yux_count; ++i) lookup_offset += i;
    
    for (int32_t i = 0; i < yux_count; ++i) {
        auto* yux = ttyd::battle::BattleGetUnitPtr(g_BattleWork, unit->unit_work[UW_Main_MiniIds + i]);
        auto* part = ttyd::battle_unit::BtlUnit_GetPartsPtr(yux, 1);
        part->parts_work[PW_Mini_SplineTable] = reinterpret_cast<uint32_t>(splineLookup[lookup_offset + i]);
    }
    return 2;
}

// Evt definitions.

// Mini-Yux events

EVT_BEGIN(unitMiniYux_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    RETURN()
EVT_END()

EVT_BEGIN(unitMiniYux_attack_event)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitMiniYux_unison_phase_event)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitMiniYux_wait_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
LBL(0)
    USER_FUNC(evt_sub_random, 120, LW(0))
    ADD(LW(0), 30)
    WAIT_FRM(LW(0))
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Mini_SpinIdle, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_CheckActStatus, -2, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            USER_FUNC(btlevtcmd_CheckStatus, -2, 5, LW(0))
            IF_EQUAL(LW(0), 0)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_4"))
                WAIT_FRM(180)
                USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
            END_IF()
        END_IF()
    END_IF()
    GOTO(0)
    RETURN()
EVT_END()

EVT_BEGIN(unitMiniYux_common_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitMiniYux_wait_event))
    USER_FUNC(btlevtcmd_SetEventUnisonPhase, -2, PTR(&unitMiniYux_unison_phase_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitMiniYux_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitMiniYux_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&btldefaultevt_Confuse))
    USER_FUNC(btlevtcmd_OnUnitFlag, -2, 0x30'0000)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitMiniYux_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Mini_BattleUnitType, (int32_t)BattleUnitType::MINI_YUX)
    RUN_CHILD_EVT(unitMiniYux_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitMiniZYux_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Mini_BattleUnitType, (int32_t)BattleUnitType::MINI_Z_YUX)
    RUN_CHILD_EVT(unitMiniYux_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitMiniXYux_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Mini_BattleUnitType, (int32_t)BattleUnitType::MINI_X_YUX)
    RUN_CHILD_EVT(unitMiniYux_common_init_event)
    RETURN()
EVT_END()


// main Yux events

EVT_BEGIN(unitYux_first_attack_pos_event)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    SET(LW(1), 20)
    USER_FUNC(btlevtcmd_SetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_WaitAttackEnd)
    RETURN()
EVT_END()

EVT_BEGIN(unitYux_shot_miss_event)
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

EVT_BEGIN(unitYux_damage_event)
    USER_FUNC(btlevtcmd_SetRGB, -2, 1, 255, 255, 255)
    SET(LW(10), -2)
    SET(LW(11), 1)
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    RETURN()
EVT_END()

EVT_BEGIN(unitYux_recovery_event)
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, -1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 300)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_3"))
    WAIT_FRM(120)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_RecoverHp, -2, 1, 2)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(5), LW(6), LW(7))
    ADD(LW(6), 30)
    ADD(LW(7), 10)
    USER_FUNC(evt_eff, PTR(""), PTR("recovery"), 0, LW(5), LW(6), LW(7), 2, 0, 0, 0, 0, 0, 0, 0)
    WAIT_FRM(30)
    RETURN()
EVT_END()

EVT_BEGIN(unitYux_attack_common_event_hit)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Main_InBarrage, LW(6))
    IF_EQUAL(LW(6), 0)
        USER_FUNC(btlevtcmd_OffPartsAttribute, -2, LW(15), 0x400'0000)
    ELSE()
        USER_FUNC(btlevtcmd_SetPartsBlur, -2, LW(15), 255, 255, 255, 255, 255, 255, 255, 100, 0)
        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, LW(15), 0x400'0000)
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, LW(15), PTR("Z_3"))
    USER_FUNC(btlevtcmd_GetPos, LW(12), LW(6), LW(7), LW(8))
    ADD(LW(8), 1)
    USER_FUNC(btlevtcmd_SetPartsPos, -2, LW(15), LW(6), LW(7), LW(8))
    USER_FUNC(btlevtcmd_CheckPartsAttribute, -2, LW(15), 0x100'0000, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_OffPartsAttribute, -2, LW(15), 0x100'0000)
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_5"))
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Main_InBarrage, LW(6))
    IF_EQUAL(LW(6), 0)
        USER_FUNC(btlevtcmd_SetPartsScale, -2, LW(15), 1, 0, 1)
        BROTHER_EVT()
            DO(0)
                DO(10)
                    USER_FUNC(btlevtcmd_AddPartsScale, -2, LW(15), 0, FLOAT(0.125), 0)
                    WAIT_FRM(1)
                WHILE()
                DO(10)
                    USER_FUNC(btlevtcmd_AddPartsScale, -2, LW(15), 0, FLOAT(-0.125), 0)
                    WAIT_FRM(1)
                WHILE()
            WHILE()
        END_BROTHER()
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(6), LW(7), LW(8))
        USER_FUNC(btlevtcmd_JumpPartsSetting, -2, LW(15), 0, 4, 0)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(15), LW(6), LW(7), LW(8), 0, -1)
    ELSE()
        USER_FUNC(btlevtcmd_SetPartsScale, -2, LW(15), 1, 0, 1)
        BROTHER_EVT()
            DO(0)
                DO(5)
                    USER_FUNC(btlevtcmd_AddPartsScale, -2, LW(15), 0, FLOAT(0.25), 0)
                    WAIT_FRM(1)
                WHILE()
                DO(5)
                    USER_FUNC(btlevtcmd_AddPartsScale, -2, LW(15), 0, FLOAT(-0.25), 0)
                    WAIT_FRM(1)
                WHILE()
            WHILE()
        END_BROTHER()
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(6), LW(7), LW(8))
        USER_FUNC(btlevtcmd_JumpPartsSetting, -2, LW(15), 0, 7, 0)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(15), LW(6), LW(7), LW(8), 0, -1)
        USER_FUNC(btlevtcmd_SetPartsPos, -2, LW(15), 0, -1000, 0)
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitYux_attack_common_event_nohit)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Main_InBarrage, LW(6))
    IF_EQUAL(LW(6), 0)
        USER_FUNC(btlevtcmd_OffPartsAttribute, -2, LW(15), 0x400'0000)
    ELSE()
        USER_FUNC(btlevtcmd_SetPartsBlur, -2, LW(15), 255, 255, 255, 255, 255, 255, 255, 100, 0)
        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, LW(15), 0x400'0000)
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, LW(15), PTR("Z_3"))
    USER_FUNC(btlevtcmd_GetPos, LW(12), LW(6), LW(7), LW(8))
    ADD(LW(8), 1)
    USER_FUNC(btlevtcmd_SetPartsPos, -2, LW(15), LW(6), LW(7), LW(8))
    USER_FUNC(btlevtcmd_CheckPartsAttribute, -2, LW(15), 0x100'0000, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_OffPartsAttribute, -2, LW(15), 0x100'0000)
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_5"))
    BROTHER_EVT()
        DO(0)
            USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(0))
            IF_EQUAL(LW(0), -1)
                USER_FUNC(btlevtcmd_GetPartsPos, -2, LW(15), LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(6), LW(7), LW(8))
                IF_SMALL_EQUAL(LW(0), LW(6))
                    RUN_CHILD_EVT(PTR(&unitYux_shot_miss_event))
                    DO_BREAK()
                END_IF()
            ELSE()
                USER_FUNC(btlevtcmd_GetPartsPos, -2, LW(15), LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(6), LW(7), LW(8))
                IF_LARGE_EQUAL(LW(0), LW(6))
                    RUN_CHILD_EVT(PTR(&unitYux_shot_miss_event))
                    DO_BREAK()
                END_IF()
            END_IF()
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Main_InBarrage, LW(6))
    IF_EQUAL(LW(6), 0)
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(6), LW(7), LW(8))
        USER_FUNC(btlevtcmd_GetStageSize, LW(6), EVT_NULLPTR, EVT_NULLPTR)
        MUL(LW(6), 1)
        USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(0))
        MUL(LW(6), LW(0))
        SUB(LW(7), 5)
        USER_FUNC(btlevtcmd_SetPartsScale, -2, LW(15), 1, 0, 1)
        BROTHER_EVT()
            DO(0)
                DO(10)
                    USER_FUNC(btlevtcmd_AddPartsScale, -2, LW(15), 0, FLOAT(0.125), 0)
                    WAIT_FRM(1)
                WHILE()
                DO(10)
                    USER_FUNC(btlevtcmd_AddPartsScale, -2, LW(15), 0, FLOAT(-0.125), 0)
                    WAIT_FRM(1)
                WHILE()
            WHILE()
        END_BROTHER()
        USER_FUNC(btlevtcmd_JumpPartsSetting, -2, LW(15), 0, 4, 0)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(15), LW(6), LW(7), LW(8), 0, -1)
    ELSE()
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(6), LW(7), LW(8))
        USER_FUNC(btlevtcmd_GetStageSize, LW(6), EVT_NULLPTR, EVT_NULLPTR)
        MUL(LW(6), 1)
        USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(0))
        MUL(LW(6), LW(0))
        SUB(LW(7), 5)
        USER_FUNC(btlevtcmd_SetPartsScale, -2, LW(15), 1, 0, 1)
        BROTHER_EVT()
            DO(0)
                DO(5)
                    USER_FUNC(btlevtcmd_AddPartsScale, -2, LW(15), 0, FLOAT(0.25), 0)
                    WAIT_FRM(1)
                WHILE()
                DO(5)
                    USER_FUNC(btlevtcmd_AddPartsScale, -2, LW(15), 0, FLOAT(-0.25), 0)
                    WAIT_FRM(1)
                WHILE()
            WHILE()
        END_BROTHER()
        USER_FUNC(btlevtcmd_JumpPartsSetting, -2, LW(15), 0, 7, 0)
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, LW(15), LW(6), LW(7), LW(8), 0, -1)
        USER_FUNC(btlevtcmd_SetPartsPos, -2, LW(15), 0, -1000, 0)
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitYux_attack_damage_check_event)
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
    END_SWITCH()
LBL(90)
    RUN_CHILD_EVT(PTR(&unitYux_attack_common_event_nohit))
    GOTO(98)
LBL(91)
    SET(LW(13), PW_Main_BarrageTargetUnits)
    SET(LW(14), PW_Main_BarrageTargetParts)
    DO(0)
        USER_FUNC(btlevtcmd_GetPartsWork, -2, 1, LW(13), LW(5))
        USER_FUNC(btlevtcmd_GetPartsWork, -2, 1, LW(14), LW(6))
        IF_EQUAL(LW(3), LW(5))
            IF_EQUAL(LW(4), LW(6))
                ADD(LW(13), 10)  // Corresponding hit success var
                USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, LW(13), 1)
                DO_BREAK()
            END_IF()
        END_IF()
        ADD(LW(13), 1)
        ADD(LW(14), 1)
    WHILE()
    RUN_CHILD_EVT(PTR(&unitYux_attack_common_event_hit))
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
    IF_EQUAL(LW(10), 1)
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    ELSE()
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 0, LW(5))
    END_IF()
    GOTO(98)
LBL(98)
    USER_FUNC(btlevtcmd_SetPartsPos, -2, LW(15), 0, -1000, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitYux_normal_attack_event)
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
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Main_InBarrage, 0)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, PW_Main_BarrageTargetUnits, LW(3))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, PW_Main_BarrageTargetParts, LW(4))
    SET(LW(10), 1)
    SET(LW(12), -2)
    SET(LW(15), 7)
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, -1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(14))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(14))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_1"))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BARRI_ATTACK2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(barriern_zanzou, -2)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BARRI_ATTACK9"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(barriern_zanzou2, -2)
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    SET(LW(0), 6)
    USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(14))
    MUL(LW(0), LW(14))
    DO(10)
        WAIT_FRM(1)
        USER_FUNC(btlevtcmd_AddRotate, -2, 0, LW(0), 0)
    WHILE()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BARRI_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_1"))
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_2"))
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_3"))
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    WAIT_FRM(10)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_4"))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BARRI_ATTACK3"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BARRI_ATTACK7"), EVT_NULLPTR, 0, LW(0))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 7, PW_Main_ShotSfx, LW(0))
    RUN_CHILD_EVT(PTR(&unitYux_attack_damage_check_event))
    USER_FUNC(btlevtcmd_GetPartsWork, -2, 7, PW_Main_ShotSfx, LW(0))
    USER_FUNC(evt_snd_sfxoff, LW(0))
    SET(LW(0), -6)
    USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(14))
    MUL(LW(0), LW(14))
    DO(10)
        WAIT_FRM(1)
        USER_FUNC(btlevtcmd_AddRotate, -2, 0, LW(0), 0)
    WHILE()
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitYux_barrage_attack_blur_event)
    USER_FUNC(barriern_satellite_zanzou, -2, LW(0))
    USER_FUNC(barriern_satellite_zanzou2, -2, LW(0))
    RETURN()
EVT_END()

EVT_BEGIN(unitYux_barrage_attack_event_sub_nolast)
    SET(LW(10), 0)
    RUN_CHILD_EVT(PTR(&unitYux_attack_damage_check_event))
    RETURN()
EVT_END()

EVT_BEGIN(unitYux_barrage_attack_event_sub_last)
    SET(LW(13), PW_Main_BarrageTargetUnits)
    SET(LW(14), PW_Main_BarrageTargetParts)
    DO(0)
        USER_FUNC(btlevtcmd_GetPartsWork, -2, 1, LW(13), LW(5))
        USER_FUNC(btlevtcmd_GetPartsWork, -2, 1, LW(14), LW(6))
        IF_EQUAL(LW(3), LW(5))
            IF_EQUAL(LW(4), LW(6))
                ADD(LW(13), 10)  // Corresponding hit success var
                USER_FUNC(btlevtcmd_GetPartsWork, -2, 1, LW(13), LW(0))
                DO_BREAK()
            END_IF()
        END_IF()
        ADD(LW(13), 1)
        ADD(LW(14), 1)
    WHILE()
    IF_EQUAL(LW(0), 1)
        RUN_CHILD_EVT(PTR(&unitYux_attack_common_event_hit))
        USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
        USER_FUNC(btlevtcmd_SetPartsPos, -2, LW(15), 0, -1000, 0)
        WAIT_FRM(20)
        USER_FUNC(btlevtcmd_OffPartsAttribute, -2, LW(15), 0x400'0000)
        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, LW(15), 0x100'0000)
    ELSE()
        SET(LW(10), 1)
        RUN_CHILD_EVT(PTR(&unitYux_attack_damage_check_event))
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitYux_barrage_attack_event_sub)
    USER_FUNC(btlevtcmd_GetPartsWork, -2, 1, LW(13), LW(3))
    USER_FUNC(btlevtcmd_GetPartsWork, -2, 1, LW(14), LW(4))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BARRI_ATTACK4"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BARRI_ATTACK8"), EVT_NULLPTR, 0, LW(0))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, LW(15), PW_Main_ShotSfx, LW(0))
    DO(0)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Main_MaxMinis, LW(5))
        SUB(LW(5), 1)
        IF_LARGE_EQUAL(LW(13), LW(5))
            RUN_CHILD_EVT(PTR(&unitYux_barrage_attack_event_sub_last))
            DO_BREAK()
        END_IF()
        ADD(LW(13), 1)
        ADD(LW(14), 1)
        USER_FUNC(btlevtcmd_GetPartsWork, -2, 1, LW(13), LW(5))
        USER_FUNC(btlevtcmd_GetPartsWork, -2, 1, LW(14), LW(6))
        IF_EQUAL(LW(3), LW(5))
            IF_EQUAL(LW(4), LW(6))
                RUN_CHILD_EVT(PTR(&unitYux_barrage_attack_event_sub_nolast))
                DO_BREAK()
            END_IF()
        END_IF()
    WHILE()
    USER_FUNC(btlevtcmd_AddUnitWork, -2, UW_Main_BarrageShots, 1)
    USER_FUNC(btlevtcmd_GetPartsWork, -2, LW(15), PW_Main_ShotSfx, LW(0))
    USER_FUNC(evt_snd_sfxoff, LW(0))
    RETURN()
EVT_END()

EVT_BEGIN(unitYux_barrage_attack_event)
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
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Main_InBarrage, 1)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Main_BarrageShots, 0)
    USER_FUNC(evtTot_SelectMultihitTargetsX5, LW(0), LW(1), LW(2), LW(3), LW(4), LW(5), LW(6), LW(7), LW(8), LW(11))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, PW_Main_BarrageTargetUnits,     LW(0))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, PW_Main_BarrageTargetUnits + 1, LW(2))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, PW_Main_BarrageTargetUnits + 2, LW(4))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, PW_Main_BarrageTargetUnits + 3, LW(6))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, PW_Main_BarrageTargetUnits + 4, LW(8))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, PW_Main_BarrageTargetParts,     LW(1))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, PW_Main_BarrageTargetParts + 1, LW(3))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, PW_Main_BarrageTargetParts + 2, LW(5))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, PW_Main_BarrageTargetParts + 3, LW(7))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, PW_Main_BarrageTargetParts + 4, LW(11))
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, PW_Main_BarrageTargetHits,     0)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, PW_Main_BarrageTargetHits + 1, 0)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, PW_Main_BarrageTargetHits + 2, 0)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, PW_Main_BarrageTargetHits + 3, 0)
    USER_FUNC(btlevtcmd_SetPartsWork, -2, 1, PW_Main_BarrageTargetHits + 4, 0)
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, -1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(btlevtcmd_GetPartsWork, -2, 1, PW_Main_BarrageTargetUnits, LW(3))
    USER_FUNC(btlevtcmd_GetPartsWork, -2, 1, PW_Main_BarrageTargetParts, LW(4))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BARRI_ATTACK5"), EVT_NULLPTR, 0, EVT_NULLPTR)
    SET(LW(0), 0)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Main_MaxMinis, LW(11))
    DO(0)
        IF_LARGE_EQUAL(LW(0), LW(11))
            DO_BREAK()
        END_IF()
        RUN_EVT(PTR(&unitYux_barrage_attack_blur_event))
        ADD(LW(0), 1)
        WAIT_FRM(10)
    WHILE()
    USER_FUNC(barriern_zanzou, -2)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BARRI_ATTACK10"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(barriern_zanzou2, -2)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BARRI_ATTACK6"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("F_1"))
    WAIT_FRM(20)
    SET(LW(11), UW_Main_MiniIds)
    DO(0)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Main_MaxMinis, LW(12))
        ADD(LW(12), UW_Main_MiniIds)
        IF_LARGE_EQUAL(LW(11), LW(12))
            DO_BREAK()
        END_IF()
        USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(11), LW(12))
        USER_FUNC(btlevtcmd_GetBodyId, LW(12), LW(13))
        USER_FUNC(btlevtcmd_AnimeChangePose, LW(12), LW(13), PTR("F_2"))
        ADD(LW(11), 1)
    WHILE()
    WAIT_FRM(20)
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    SET(LW(11), UW_Main_MiniIds)
    DO(0)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Main_MaxMinis, LW(12))
        ADD(LW(12), UW_Main_MiniIds)
        IF_LARGE_EQUAL(LW(11), LW(12))
            DO_BREAK()
        END_IF()
        USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(11), LW(12))
        USER_FUNC(btlevtcmd_SetRotate, LW(12), 0, 0, 0)
        ADD(LW(11), 1)
    WHILE()
    SET(LW(0), 6)
    USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(14))
    MUL(LW(0), LW(14))
    DO(10)
        WAIT_FRM(1)
        USER_FUNC(btlevtcmd_AddRotate, -2, 0, LW(0), 0)
        SET(LW(11), UW_Main_MiniIds)
        DO(0)
            USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Main_MaxMinis, LW(12))
            ADD(LW(12), UW_Main_MiniIds)
            IF_LARGE_EQUAL(LW(11), LW(12))
                DO_BREAK()
            END_IF()
            USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(11), LW(12))
            USER_FUNC(btlevtcmd_AddRotate, LW(12), 0, LW(0), 0)
            ADD(LW(11), 1)
        WHILE()
    WHILE()
    SET(LW(11), UW_Main_MiniIds)
    DO(0)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Main_MaxMinis, LW(12))
        ADD(LW(12), UW_Main_MiniIds)
        IF_LARGE_EQUAL(LW(11), LW(12))
            DO_BREAK()
        END_IF()
        USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(11), LW(12))
        USER_FUNC(btlevtcmd_GetBodyId, LW(12), LW(13))
        USER_FUNC(btlevtcmd_AnimeChangePose, LW(12), LW(13), PTR("S_2"))
        ADD(LW(11), 1)
    WHILE()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BARRI_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_1"))
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_2"))
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_3"))
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    WAIT_FRM(10)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_4"))
    SET(LW(12), -2)
    SET(LW(13), 0)
    SET(LW(14), 5)
    SET(LW(15), 7)
    RUN_EVT(PTR(&unitYux_barrage_attack_event_sub))
    WAIT_FRM(30)
    SET(LW(11), UW_Main_MiniIds)
    ADD(LW(13), 1)
    ADD(LW(14), 1)
    ADD(LW(15), 1)
    DO(0)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Main_MaxMinis, LW(12))
        ADD(LW(12), UW_Main_MiniIds)
        IF_LARGE_EQUAL(LW(11), LW(12))
            DO_BREAK()
        END_IF()
        USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(11), LW(12))
        RUN_EVT(PTR(&unitYux_barrage_attack_event_sub))
        WAIT_FRM(20)
        ADD(LW(11), 1)
        ADD(LW(13), 1)
        ADD(LW(14), 1)
        ADD(LW(15), 1)
    WHILE()
    DO(0)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Main_BarrageShots, LW(0))
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Main_MaxMinis, LW(1))
        ADD(LW(1), 1)
        IF_LARGE_EQUAL(LW(0), LW(1))
            DO_BREAK()
        END_IF()
        WAIT_FRM(1)
    WHILE()
    WAIT_FRM(10)
    USER_FUNC(_check_satellite, LW(0))
    SET(LW(0), -6)
    USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(14))
    MUL(LW(0), LW(14))
    DO(10)
        WAIT_FRM(1)
        USER_FUNC(btlevtcmd_AddRotate, -2, 0, LW(0), 0)
        SET(LW(11), UW_Main_MiniIds)
        DO(0)
            USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Main_MaxMinis, LW(12))
            ADD(LW(12), UW_Main_MiniIds)
            IF_LARGE_EQUAL(LW(11), LW(12))
                DO_BREAK()
            END_IF()
            USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(11), LW(12))
            USER_FUNC(btlevtcmd_AddRotate, LW(12), 0, LW(0), 0)
            ADD(LW(11), 1)
        WHILE()
    WHILE()
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitYux_attack_event)
    // Set weapons and chance of using recovery move.
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Main_BattleUnitType, LW(15))
    SWITCH(LW(15))
        CASE_EQUAL((int32_t)BattleUnitType::YUX)
            SET(LW(9), PTR(&unitYux_weaponNormal))
            SET(LW(10), PTR(&unitYux_weaponBarrage))
            SET(LW(11), 0)
        CASE_EQUAL((int32_t)BattleUnitType::Z_YUX)
            SET(LW(9), PTR(&unitZYux_weaponNormal))
            SET(LW(10), PTR(&unitZYux_weaponBarrage))
            SET(LW(11), 30)
        CASE_ETC()
            SET(LW(9), PTR(&unitXYux_weaponNormal))
            SET(LW(10), PTR(&unitXYux_weaponBarrage))
            SET(LW(11), 0)
    END_SWITCH()

    USER_FUNC(btlevtcmd_SetRGB, -2, 1, 255, 255, 255)
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(PTR(&unitYux_normal_attack_event))
        RETURN()
    END_IF()

    USER_FUNC(btlevtcmd_EnemyItemUseCheck, -2, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(LW(0))
        USER_FUNC(btlevtcmd_StartWaitEvent, -2)
        RETURN()
    END_IF()

    USER_FUNC(_check_satellite, LW(0))
    SET(LW(0), 0)
    SET(LW(1), UW_Main_MiniIds)
    DO(4)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(1), LW(3))
        IF_NOT_EQUAL(LW(3), -1)
            USER_FUNC(btlevtcmd_CheckActStatus, LW(3), LW(2))
            IF_NOT_EQUAL(LW(2), 0)
                USER_FUNC(btlevtcmd_CheckStatus, LW(3), 5, LW(2))
                IF_EQUAL(LW(2), 0)
                    ADD(LW(0), 1)
                END_IF()
            END_IF()
        END_IF()
        ADD(LW(1), 1)
    WHILE()

    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Main_MaxMinis, LW(1))
    IF_EQUAL(LW(0), LW(1))
        SET(LW(9), LW(10))
        RUN_CHILD_EVT(PTR(&unitYux_barrage_attack_event))
    ELSE()
        USER_FUNC(evt_sub_random, 99, LW(0))
        IF_SMALL(LW(0), LW(11))
            RUN_CHILD_EVT(PTR(&unitYux_recovery_event))
        ELSE()
            RUN_CHILD_EVT(PTR(&unitYux_normal_attack_event))
        END_IF()
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitYux_satellite_blur_off_event)
    SET(LW(2), UW_Main_MiniIds)
    DO(4)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(2), LW(3))
        IF_NOT_EQUAL(LW(3), -1)
            USER_FUNC(btlevtcmd_GetBodyId, LW(3), LW(4))
            USER_FUNC(btlevtcmd_OffPartsAttribute, LW(3), LW(4), 0x400'0000)
        END_IF()
        ADD(LW(2), 1)
    WHILE()
    RETURN()
EVT_END()

EVT_BEGIN(unitYux_satellite_blur_on_event)
    SET(LW(2), UW_Main_MiniIds)
    DO(4)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(2), LW(3))
        IF_NOT_EQUAL(LW(3), -1)
            USER_FUNC(btlevtcmd_GetBodyId, LW(3), LW(4))
            USER_FUNC(btlevtcmd_SetPartsBlur, LW(3), LW(4), 255, 255, 255, 255, 255, 255, 255, 100, 0)
            USER_FUNC(btlevtcmd_OnPartsAttribute, LW(3), LW(4), 0x400'0000)
        END_IF()
        ADD(LW(2), 1)
    WHILE()
    RETURN()
EVT_END()

EVT_BEGIN(unitYux_unison_phase_event)
    USER_FUNC(btlevtcmd_StopWaitEvent, -2)
    USER_FUNC(btlevtcmd_CheckPhase, LW(0), 0x400'0001)
    IF_EQUAL(LW(0), 0)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_get_turn, LW(0))
    IF_SMALL_EQUAL(LW(0), 1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_CheckActStatus, -2, LW(0))
    IF_EQUAL(LW(0), 0)
        GOTO(99)
    END_IF()
    USER_FUNC(evt_sub_random, 10, LW(0))
    WAIT_FRM(LW(0))

    SET(LW(0), 0)
    SET(LW(1), UW_Main_MiniIds)
    DO(4)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(1), LW(3))
        IF_NOT_EQUAL(LW(3), -1)
            ADD(LW(0), 1)
        END_IF()
        ADD(LW(1), 1)
    WHILE()
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Main_MaxMinis, LW(1))
    IF_LARGE_EQUAL(LW(0), LW(1))
        GOTO(99)
    END_IF()

    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BARRI_ATTACK9"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(barriern_zanzou3, -2)
    RUN_CHILD_EVT(PTR(&unitYux_satellite_blur_on_event))
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Main_MiniCount, LW(1))
    IF_LARGE_EQUAL(LW(1), 1)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BARRI_SPLIT1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    END_IF()
    USER_FUNC(barriernSatelliteToInside, -2, LW(1))
    RUN_CHILD_EVT(PTR(&unitYux_satellite_blur_off_event))
    USER_FUNC(btlevtcmd_GetUnitId, -2, LW(4))
    SET(LW(1), UW_Main_MiniIds)
    SET(LW(5), 0)
    DO(0)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Main_MaxMinis, LW(11))
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Main_MinisPerTurn, LW(12))

        // Out of spaces to spawn new mini-Yuxes.
        ADD(LW(11), UW_Main_MiniIds)
        IF_LARGE_EQUAL(LW(1), LW(11))
            DO_BREAK()
        END_IF()

        USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(1), LW(2))
        IF_EQUAL(LW(2), -1)

            USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Main_BattleUnitType, LW(13))
            SWITCH(LW(13))
                CASE_EQUAL((int32_t)BattleUnitType::YUX)
                    USER_FUNC(btlevtcmd_SpawnUnit, LW(3), PTR(&unitMiniYux_spawn_entry), 0)
                CASE_EQUAL((int32_t)BattleUnitType::Z_YUX)
                    USER_FUNC(btlevtcmd_SpawnUnit, LW(3), PTR(&unitMiniZYux_spawn_entry), 0)
                CASE_ETC()
                    USER_FUNC(btlevtcmd_SpawnUnit, LW(3), PTR(&unitMiniXYux_spawn_entry), 0)
            END_SWITCH()

            USER_FUNC(btlevtcmd_AddPos, LW(3), 0, 0, -1)
            USER_FUNC(btlevtcmd_SetUnitWork, -2, LW(1), LW(3))
            USER_FUNC(btlevtcmd_SetUnitWork, LW(3), UW_Mini_ParentId, LW(4))
            ADD(LW(0), 1)
            ADD(LW(5), 1)
            // Spawned the maximum mini-Yuxes this turn.
            IF_LARGE_EQUAL(LW(5), LW(12))
                DO_BREAK()
            END_IF()
        END_IF()
        ADD(LW(1), 1)
    WHILE()
    WAIT_FRM(5)
    RUN_CHILD_EVT(PTR(&unitYux_satellite_blur_on_event))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BARRI_SPLIT2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(barriernSatelliteToOutside, -2, LW(0))
    WAIT_FRM(60)
    RUN_CHILD_EVT(PTR(&unitYux_satellite_blur_off_event))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Main_MiniCount, LW(0))
LBL(99)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitYux_wait_event)
    USER_FUNC(_check_satellite, LW(0))
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    SET(LW(11), UW_Main_MiniIds)
    DO(0)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Main_MaxMinis, LW(12))
        ADD(LW(12), UW_Main_MiniIds)
        IF_LARGE_EQUAL(LW(11), LW(12))
            DO_BREAK()
        END_IF()
        USER_FUNC(btlevtcmd_GetUnitWork, -2, LW(11), LW(12))
        IF_NOT_EQUAL(LW(12), -1)
            USER_FUNC(btlevtcmd_SetUnitWork, LW(12), UW_Mini_SpinIdle, 1)
        END_IF()
        ADD(LW(11), 1)
    WHILE()
    BROTHER_EVT()
        SET(LW(1), 255)
        SET(LW(2), 255)
        SET(LW(3), 255)
        SET(LW(4), -8)
LBL(0)
        USER_FUNC(_check_satellite, LW(0))
        WAIT_FRM(1)
        GOTO(0)
    END_BROTHER()
LBL(1)
    USER_FUNC(evt_sub_random, 120, LW(0))
    ADD(LW(0), 30)
    WAIT_FRM(LW(0))
    USER_FUNC(btlevtcmd_CheckActStatus, -2, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_CheckStatus, -2, 5, LW(0))
        USER_FUNC(btlevtcmd_CheckStatus, -2, 3, LW(1))
        USER_FUNC(btlevtcmd_CheckStatus, -2, 4, LW(2))
        OR(LW(0), LW(1))
        OR(LW(0), LW(2))
        IF_EQUAL(LW(0), 0)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_3"))
            WAIT_FRM(180)
            USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
        END_IF()
    END_IF()
    GOTO(1)
    RETURN()
EVT_END()

EVT_BEGIN(unitYux_common_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitYux_wait_event))
    USER_FUNC(btlevtcmd_SetEventUnisonPhase, -2, PTR(&unitYux_unison_phase_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitYux_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitYux_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitYux_attack_event))
    USER_FUNC(btlevtcmd_SetPartsScale, -2, 1, LW(9), LW(9), FLOAT(1.0))
    USER_FUNC(btlevtcmd_OnUnitFlag, -2, 0x30'0000)
    SET(LW(0), UW_Main_MiniIds)
    DO(4)
        USER_FUNC(btlevtcmd_SetUnitWork, -2, LW(0), -1)
        ADD(LW(0), 1)
    WHILE()
    SET(LW(6), 0)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Main_MiniCount, LW(7))
LBL(0)
    IF_SMALL(LW(6), LW(7))
        USER_FUNC(btlevtcmd_GetUnitId, -2, LW(5))
        SET(LW(8), LW(6))
        ADD(LW(8), UW_Main_MiniIds)

        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Main_BattleUnitType, LW(10))
        SWITCH(LW(10))
            CASE_EQUAL((int32_t)BattleUnitType::YUX)
                USER_FUNC(btlevtcmd_SpawnUnit, LW(3), PTR(&unitMiniYux_spawn_entry), 0)
            CASE_EQUAL((int32_t)BattleUnitType::Z_YUX)
                USER_FUNC(btlevtcmd_SpawnUnit, LW(3), PTR(&unitMiniZYux_spawn_entry), 0)
            CASE_ETC()
                USER_FUNC(btlevtcmd_SpawnUnit, LW(3), PTR(&unitMiniXYux_spawn_entry), 0)
        END_SWITCH()
        
        USER_FUNC(btlevtcmd_SetUnitWork, -2, LW(8), LW(3))
        USER_FUNC(btlevtcmd_SetUnitWork, LW(3), UW_Mini_ParentId, LW(5))
        USER_FUNC(btlevtcmd_SetUnitWork, LW(3), UW_Mini_SpinIdle, 1)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(battleGetBarriernSatelliteInitPos, -2, LW(7), LW(6), LW(10), LW(11), LW(12))
        ADD(LW(0), LW(10))
        ADD(LW(1), LW(11))
        ADD(LW(2), LW(12))
        USER_FUNC(btlevtcmd_SetPos, LW(3), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_SetHomePos, LW(3), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_SetScale, LW(3), LW(9), LW(9), FLOAT(1.0))
        ADD(LW(6), 1)
        GOTO(0)
    END_IF()
LBL(10)
    USER_FUNC(outSplineTableSet, -2, LW(7))
    RETURN()
EVT_END()

EVT_BEGIN(unitYux_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Main_BattleUnitType, (int32_t)BattleUnitType::YUX)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Main_MaxMinis, 2)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Main_MinisPerTurn, 1)
    SETF(LW(9), FLOAT(0.8))
    RUN_CHILD_EVT(unitYux_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitZYux_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Main_BattleUnitType, (int32_t)BattleUnitType::Z_YUX)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Main_MaxMinis, 4)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Main_MinisPerTurn, 1)
    SETF(LW(9), FLOAT(1.0))
    RUN_CHILD_EVT(unitYux_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitXYux_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Main_BattleUnitType, (int32_t)BattleUnitType::X_YUX)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Main_MaxMinis, 4)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Main_MinisPerTurn, 2)
    SETF(LW(9), FLOAT(1.0))
    RUN_CHILD_EVT(unitYux_common_init_event)
    RETURN()
EVT_END()

}  // namespace mod::tot::custom