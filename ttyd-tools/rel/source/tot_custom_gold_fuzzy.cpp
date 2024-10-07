#include "tot_custom_rel.h"     // For externed unit definitions.

#include "evt_cmd.h"
#include "mod.h"
#include "tot_manager_dialogue.h"
#include "tot_state.h"

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
#include <ttyd/dispdrv.h>
#include <ttyd/evt_eff.h>
#include <ttyd/evt_ext.h>
#include <ttyd/evt_msg.h>
#include <ttyd/evt_npc.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evt_sub.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/extdrv.h>
#include <ttyd/icondrv.h>
#include <ttyd/mariost.h>
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
using namespace ::ttyd::evt_eff;
using namespace ::ttyd::evt_ext;
using namespace ::ttyd::evt_msg;
using namespace ::ttyd::evt_npc;
using namespace ::ttyd::evt_snd;
using namespace ::ttyd::evt_sub;

using ::ttyd::battle::g_BattleWork;
using ::ttyd::battle_unit::BattleWorkUnit;
using ::ttyd::battle_unit::BattleWorkUnitPart;
using ::ttyd::dispdrv::CameraId;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;
using ::ttyd::extdrv::ExtEntryData;
using ::ttyd::extdrv::ExtPoseWork;

// State for "ext" Fuzzy actors.
struct ExtFuzzyWork {
    uint8_t     flags;
    uint8_t     state;
    uint8_t     pad_0x02[2];
    gc::vec3    unk_0x04;
    float       unk_0x10;
    gc::vec3    unk_0x14;
    int32_t     unk_0x20;
    gc::vec3    unk_0x24;
};

static_assert(sizeof(ExtFuzzyWork) == 0x30);

// For sorting Fuzzies by proximity to center stage, before some of them flee.
struct FuzzyCenterDistance {
    int32_t idx;
    float distance;
};

}  // namespace

// Function / USER_FUNC declarations.

void chorogun_ext_init();
void chorogun_ext_main();
void chorogun_ext_disp(CameraId camera, void* user_data);
void DebugDispChorogunCount(CameraId camera, void* user_data);
EVT_DECLARE_USER_FUNC(chorogun_escape_all, 0)
EVT_DECLARE_USER_FUNC(chorogun_escape, 1)
EVT_DECLARE_USER_FUNC(chorogun_after_attack, 0)
EVT_DECLARE_USER_FUNC(chorogun_countered, 1)
EVT_DECLARE_USER_FUNC(chorogun_bound_target, 1)
EVT_DECLARE_USER_FUNC(chorogun_dive_target, 2)
EVT_DECLARE_USER_FUNC(chorogun_approach_target, 0)
EVT_DECLARE_USER_FUNC(chorogun_appear, 0)
EVT_DECLARE_USER_FUNC(chorogun_set_target_id, 1)
EVT_DECLARE_USER_FUNC(chorogun_get_alive_num, 1)
EVT_DECLARE_USER_FUNC(get_gold_chorobon_id, 1)

// Constants.

constexpr const int32_t UW_GoldFuzzy_SpawnedHorde = 0;
constexpr const int32_t UW_GoldFuzzy_KillHordeTrigger = 1;
constexpr const int32_t UW_GoldFuzzy_HordeId = 2;
constexpr const int32_t UW_FuzzyHorde_Unknown = 0;

// Unit data.

int8_t unitFuzzyHorde_defense[] = { 0, 0, 0, 0, 0 };
int8_t unitFuzzyHorde_defense_attr[] = { 0, 0, 0, 0, 0 };

StatusVulnerability unitFuzzyHorde_status = {
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,
};

PoseTableEntry unitFuzzyHorde_pose_table[] = {
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
    69, "CBN_S_1",
};

const ExtEntryData unitFuzzyHorde_ext_entry_data[] = {
    { "c_chorobon", "CBN_W_1", 0.0f },
    { "c_chorobon", "CBN_W_1", 2.0f },
    { "c_chorobon", "CBN_W_1", 4.0f },
    { "c_chorobon", "CBN_W_1", 6.0f },
    { "c_chorobon", "CBN_W_1", 8.0f },
    { "c_chorobon", "CBN_W_1", 10.0f },
    { "c_chorobon", "CBN_W_1", 12.0f },
    { "c_chorobon", "CBN_W_1", 14.0f },
    { "c_chorobon", "CBN_W_1", 16.0f },
    { "c_chorobon", "CBN_W_1", 18.0f },
    { "c_chorobon", "CBN_W_1", 20.0f },
    { "c_chorobon", "CBN_W_1", 22.0f },
    { "c_chorobon", "CBN_W_1", 24.0f },
    { "c_chorobon", "CBN_W_1", 26.0f },
    { "c_chorobon", "CBN_W_1", 28.0f },
    { "c_chorobon", "CBN_W_1", 30.0f },
    { "c_chorobon", "CBN_W_1", 32.0f },
    { "c_chorobon", "CBN_W_1", 34.0f },
    { "c_chorobon", "CBN_W_1", 36.0f },
    { "c_chorobon", "CBN_W_1", 38.0f },
    { nullptr, nullptr, 0.0f },
};

BattleUnitKindPart unitFuzzyHorde_parts = {
    .index = 1,
    .name = "btl_un_chorobon_gundan",
    .model_name = "c_chorobon",
    .part_offset_pos = { 0.0f, 0.0f, 0.0f },
    .part_hit_base_offset = { 0.0f, 25.0f, 0.0f },
    .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
    .unk_30 = 20,
    .unk_32 = 30,
    .base_alpha = 255,
    .defense = unitFuzzyHorde_defense,
    .defense_attr = unitFuzzyHorde_defense_attr,
    .attribute_flags = 0x0300'0009,
    .counter_attribute_flags = 0,
    .pose_table = unitFuzzyHorde_pose_table,
};

BattleWeapon unitFuzzyHorde_weapon = {
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
    .damage_function_params = { 7, 0, 0, 0, 0, 0, 0, 0 },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags =
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_SELECT_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::UNKNOWN_0x2 |
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags = AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::PREFER_FRONT |
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

int8_t unitGoldFuzzy_defense[] = { 0, 0, 0, 0, 0 };
int8_t unitGoldFuzzy_defense_attr[] = { 0, 0, 0, 0, 0 };

StatusVulnerability unitGoldFuzzy_status = {
      0,   0,   0, 100,   0, 100, 100,   0,
    100,   0, 100,   0, 100,   0,   0,   0,
      0, 100,   0, 100, 100,   0,
};

PoseTableEntry unitGoldFuzzy_pose_table[] = {
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

const BattleUnitSetup unitGoldFuzzy_horde_entry = {
    .unit_kind_params = &unit_FuzzyHorde,
    .alliance = 1,
    .attack_phase = 0x04000004,
    .position = { 75.0f, 0.0f, 0.0f },
    .addl_target_offset_x = 0,
    .unit_work = { 0, 0, 0, 0 },
    .item_drop_table = nullptr,
};

const PoseSoundTimingEntry unitGoldFuzzy_pose_sound_timing_table[] = {
    { "CBN_S_1", 0.5833333f, 0, "SFX_BOSS_G_CHORO_MOVE1", 1 },
    { "CBN_S_1", 0.8333333f, 0, "SFX_BOSS_G_CHORO_WAIT2", 1 },
    { "CBN_S_1", 1.5833334f, 0, "SFX_BOSS_G_CHORO_WAIT1", 1 },
    { nullptr, 0.0f, 0, nullptr, 1 },
};

BattleUnitKindPart unitGoldFuzzy_parts = {
    .index = 1,
    .name = "btl_un_gold_chorobon",
    .model_name = "c_chorobon_k",
    .part_offset_pos = { 0.0f, 0.0f, 0.0f },
    .part_hit_base_offset = { 0.0f, 25.0f, 0.0f },
    .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
    .unk_30 = 20,
    .unk_32 = 30,
    .base_alpha = 255,
    .defense = unitGoldFuzzy_defense,
    .defense_attr = unitGoldFuzzy_defense_attr,
    .attribute_flags = 0x0000'0009,
    .counter_attribute_flags = 0,
    .pose_table = unitGoldFuzzy_pose_table,
};

BattleWeapon unitGoldFuzzy_weaponTackle = {
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
    .damage_function_params = { 7, 0, 0, 0, 0, 0, 0, 0 },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags =
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_SELECT_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::JUMPLIKE |
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags = 
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::FLIPS_SHELLED |
        AttackSpecialProperty_Flags::GROUNDS_WINGED,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::PREFER_FRONT |
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

BattleWeapon unitGoldFuzzy_weaponDrainFp = {
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
    .fp_damage_function_params = { 5, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags =
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_SELECT_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::JUMPLIKE |
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
    .target_weighting_flags =
        AttackTargetWeighting_Flags::PREFER_FRONT |
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

BattleWeapon unitGoldFuzzy_weaponDrainCountered = {
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
    .damage_function = nullptr,
    .damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags =
        AttackTargetClass_Flags::SINGLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_SELECT_PARTS |
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

BattleWeapon unitGoldFuzzy_weaponMagic = {
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
    .damage_function_params = { 7, 0, 0, 0, 0, 0, 0, 0 },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags =
        AttackTargetClass_Flags::MULTIPLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_SELECT_PARTS |
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
    .sleep_chance = 70,
    .sleep_time = 2,
    
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

EVT_BEGIN(unitFuzzyHorde_death_sub_event)
    USER_FUNC(btlevtcmd_snd_se, LW(10), PTR("SFX_BTL_ENEMY_DIE1_2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetPos, LW(10), LW(0), LW(1), LW(2))
    ADD(LW(2), 10)
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
    RETURN()
EVT_END()

EVT_BEGIN(unitFuzzyHorde_escape_event)
    WAIT_MSEC(2000)
    USER_FUNC(get_gold_chorobon_id, LW(4))
    USER_FUNC(btlevtcmd_GetUnitId, -2, LW(0))
    IF_EQUAL(LW(0), -1)
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_GetUnitWork, LW(4), UW_GoldFuzzy_KillHordeTrigger, LW(0))
    IF_EQUAL(LW(0), 1)
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_SetUnitWork, LW(4), UW_GoldFuzzy_KillHordeTrigger, 1)
    USER_FUNC(btlevtcmd_GetUnitId, -2, LW(10))
    RUN_CHILD_EVT(PTR(&unitFuzzyHorde_death_sub_event))
    USER_FUNC(chorogun_escape_all)
    WAIT_MSEC(1000)
    USER_FUNC(btlevtcmd_GetUnitWork, LW(4), UW_GoldFuzzy_HordeId, LW(3))
    USER_FUNC(btlevtcmd_KillUnit, LW(3), 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitFuzzyHorde_damage_event)
    USER_FUNC(btlevtcmd_GetDamage, -2, LW(15))
    IF_SMALL(LW(15), 1)
        GOTO(99)
    END_IF()

    // See how many Fuzzies should escape, out of 1000.
    USER_FUNC(btlevtcmd_GetMaxHp, -2, LW(2))
    USER_FUNC(btlevtcmd_GetHp, -2, LW(1))
    MUL(LW(1), 1000)
    DIV(LW(1), LW(2))
    USER_FUNC(chorogun_get_alive_num, LW(0))
    SUB(LW(0), LW(1))

    USER_FUNC(chorogun_escape, LW(0))

    // If final blow, have Fuzzies flee if there are fewer than 40% of them.
    USER_FUNC(btlevtcmd_CheckDamageCode, -2, 256, LW(0))
    IF_EQUAL(LW(0), 0)
        GOTO(99)
    END_IF()
    IF_SMALL_EQUAL(LW(1), 400)
        RUN_CHILD_EVT(PTR(&unitFuzzyHorde_escape_event))
        GOTO(99)
    END_IF()

LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitFuzzyHorde_attack_event)
    USER_FUNC(btlevtcmd_EnemyItemUseCheck, -2, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(LW(0))
        USER_FUNC(btlevtcmd_StartWaitEvent, -2)
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), PTR(&unitFuzzyHorde_weapon))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, PTR(&unitFuzzyHorde_weapon), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_WeaponAftereffect, PTR(&unitFuzzyHorde_weapon))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, PTR(&unitFuzzyHorde_weapon))
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, LW(3), -1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    USER_FUNC(chorogun_set_target_id, LW(3))
    USER_FUNC(chorogun_approach_target)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), PTR(&unitFuzzyHorde_weapon), 256, LW(5))
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
    WAIT_MSEC(1000)
    GOTO(98)
    LBL(91)

    // Round the Fuzzy Horde's health up to the nearest 20%, and do HP/20% hits.
    USER_FUNC(chorogun_get_alive_num, LW(15))
    ADD(LW(15), 199)
    DIV(LW(15), 200)
    IF_SMALL_EQUAL(LW(15), 0)
        SET(LW(15), 1)
    END_IF()
    SET(LW(14), 1)
    DO(0)
        BROTHER_EVT()
            USER_FUNC(chorogun_dive_target, 30, LW(6))
            WAIT_FRM(30)
            USER_FUNC(btlevtcmd_ResultACDefence, LW(3), PTR(&unitFuzzyHorde_weapon))
            IF_EQUAL(LW(14), LW(15))
                USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&unitFuzzyHorde_weapon), 256, LW(5))
            ELSE()
                USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&unitFuzzyHorde_weapon), 0, LW(5))
            END_IF()
            IF_EQUAL(LW(5), 21)
                USER_FUNC(chorogun_countered, LW(6))
            ELSE()
                USER_FUNC(chorogun_bound_target, LW(6))
            END_IF()
        END_BROTHER()

        ADD(LW(14), 1)
        IF_SMALL_EQUAL(LW(14), LW(15))
            // Between lunges, wait 60 frames, reduced by a random interval of
            // increasingly wide range based on how low the Horde's HP is.
            // (From 0 frames at full health to 0-40 frames at <= 40% max HP)
            SET(LW(1), 1000)
            USER_FUNC(chorogun_get_alive_num, LW(0))
            SUB(LW(1), LW(0))
            DIV(LW(1), 15)
            ADD(LW(1), 1)

            USER_FUNC(evt_sub_random, 99, LW(0))
            MUL(LW(0), LW(1))
            DIV(LW(0), 100)

            SET(LW(1), 60)
            SUB(LW(1), LW(0))
            WAIT_FRM(LW(1))
        ELSE()
            // Wait a full 60 frames after the last lunge.
            WAIT_FRM(60)
            DO_BREAK()
        END_IF()
    WHILE()
    GOTO(98)
    LBL(98)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(chorogun_after_attack)
    WAIT_MSEC(1000)
    LBL(99)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitFuzzyHorde_wait_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitFuzzyHorde_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitFuzzyHorde_wait_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitFuzzyHorde_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitFuzzyHorde_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&btldefaultevt_Confuse))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_FuzzyHorde_Unknown, 0)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitFuzzyHorde_ext_entry_event)
    USER_FUNC(evt_npc_entry, PTR("ext_chorobon"), PTR("c_chorobon"))
    USER_FUNC(evt_npc_set_position, PTR("ext_chorobon"), 0, -1000, 0)
    USER_FUNC(
        evt_ext_entry, 1000, PTR(&unitFuzzyHorde_ext_entry_data),
        PTR(&chorogun_ext_init), PTR(&chorogun_ext_main), 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitGoldFuzzy_rabbit_move_event)
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
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(5), LW(6), LW(7), 0, 0)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_G_CHORO_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
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

EVT_BEGIN(unitGoldFuzzy_call_horde_event)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("CBN_S_1"))
    WAIT_MSEC(1000)
    USER_FUNC(btlevtcmd_StatusWindowOnOff, 0)
    USER_FUNC(evt_btl_camera_set_prilimit, 1)
    USER_FUNC(evt_btl_camera_set_mode, 1, 3)
    USER_FUNC(evt_btl_camera_set_moveto, 1, 120, 40, 200, 120, -10, -500, 30, 0)
    WAIT_FRM(60)
    USER_FUNC(btlevtcmd_reset_move_color_lv_all)
    // Print phase 2 dialogue.
    USER_FUNC(evtTot_SetConversation, (int32_t)ConversationId::SBOSS_PHASE2)
    USER_FUNC(evtTot_GetNextMessage, LW(0), LW(1))
    USER_FUNC(evt_msg_print, 2, LW(0), 0, -2)
    USER_FUNC(btlevtcmd_StatusWindowOnOff, 1)
    USER_FUNC(evt_btl_camera_set_mode, 1, 0)
    USER_FUNC(evt_btl_camera_set_prilimit, 0)
    WAIT_MSEC(1000)
    BROTHER_EVT()
        WAIT_MSEC(500)
        USER_FUNC(btlevtcmd_AnimeChangePose, -3, 1, PTR("M_N_7"))
    END_BROTHER()
    USER_FUNC(chorogun_appear)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -3, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitGoldFuzzy_return_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 0, FLOAT(4.0), FLOAT(0.6))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(5), LW(6), LW(7))
    SET(LW(8), 40)
    RUN_CHILD_EVT(PTR(&unitGoldFuzzy_rabbit_move_event))
    RETURN()
EVT_END()

EVT_BEGIN(unitGoldFuzzy_counter_damage_event)
    USER_FUNC(btlevtcmd_SetScale, -2, FLOAT(1.0), FLOAT(1.0), FLOAT(1.0))
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_GetDamageCode, LW(10), LW(0))
    RUN_CHILD_EVT(PTR(&subsetevt_counter_damage))
    USER_FUNC(btlevtcmd_GetHp, -2, LW(0))
    IF_LARGE_EQUAL(LW(0), 1)
        RUN_CHILD_EVT(PTR(&unitGoldFuzzy_return_event))
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitGoldFuzzy_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_GoldFuzzy_HordeId, LW(0))
    USER_FUNC(btlevtcmd_GetUnitId, LW(0), LW(1))
    IF_NOT_EQUAL(LW(1), -1)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_GoldFuzzy_KillHordeTrigger, LW(0))
        IF_EQUAL(LW(0), 0)
            USER_FUNC(btlevtcmd_CheckDamageCode, -2, 512, LW(0))
            IF_NOT_EQUAL(LW(0), 0)
                USER_FUNC(btlevtcmd_CheckDamageCode, -2, 2048, LW(0))
                IF_EQUAL(LW(0), 0)
                    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_GoldFuzzy_SpawnedHorde, LW(0))
                    IF_NOT_EQUAL(LW(0), 0)
                        USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_GoldFuzzy_KillHordeTrigger, 1)
                        USER_FUNC(btlevtcmd_WaitAttackEnd)
                        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_GoldFuzzy_HordeId, LW(10))
                        RUN_CHILD_EVT(PTR(&unitFuzzyHorde_death_sub_event))
                        SET(LW(10), -2)
                        USER_FUNC(chorogun_escape_all)
                        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_GoldFuzzy_HordeId, LW(3))
                        USER_FUNC(btlevtcmd_KillUnit, LW(3), 0)
                    END_IF()
                END_IF()
            END_IF()
        END_IF()
    END_IF()
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_GoldFuzzy_SpawnedHorde, LW(0))
    IF_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_GetMaxHp, -2, LW(1))
        USER_FUNC(btlevtcmd_GetHp, -2, LW(0))
        // Spawn the horde when Gold Fuzzy reaches 2/3 of max health.
        MUL(LW(0), 100)
        DIV(LW(0), LW(1))
        IF_SMALL_EQUAL(LW(0), 66)
            IF_LARGE_EQUAL(LW(0), 1)
                USER_FUNC(btlevtcmd_WaitAttackEnd)
                USER_FUNC(btlevtcmd_SpawnUnit, LW(3), PTR(&unitGoldFuzzy_horde_entry), 0)
                USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_GoldFuzzy_HordeId, LW(3))
                RUN_CHILD_EVT(PTR(&unitGoldFuzzy_call_horde_event))
                USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_GoldFuzzy_SpawnedHorde, 1)
                GOTO(99)
            END_IF()
        END_IF()
    END_IF()
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitGoldFuzzy_magic_attack_event)
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), PTR(&unitGoldFuzzy_weaponMagic))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, PTR(&unitGoldFuzzy_weaponMagic), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            RETURN()
        END_IF()
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_WeaponAftereffect, PTR(&unitGoldFuzzy_weaponMagic))
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, PTR(&unitGoldFuzzy_weaponMagic))
    SET(LW(0), -4)
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
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), PTR(&unitGoldFuzzy_weaponMagic), 256, LW(5))
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
        USER_FUNC(btlevtcmd_ResultACDefence, LW(3), PTR(&unitGoldFuzzy_weaponMagic))
    END_IF()
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&unitGoldFuzzy_weaponMagic), 256, LW(5))
    GOTO(97)
    LBL(97)
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    IF_NOT_EQUAL(LW(3), -1)
        ADD(LW(10), 1)
        GOTO(0)
    END_IF()
    LBL(99)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitGoldFuzzy_drain_attack_event)
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), PTR(&unitGoldFuzzy_weaponDrainFp))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, PTR(&unitGoldFuzzy_weaponDrainFp), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            RETURN()
        END_IF()
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_WeaponAftereffect, PTR(&unitGoldFuzzy_weaponDrainFp))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, PTR(&unitGoldFuzzy_weaponDrainFp))
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
    RUN_CHILD_EVT(PTR(&unitGoldFuzzy_rabbit_move_event))
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
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), PTR(&unitGoldFuzzy_weaponDrainFp), 256, LW(5))
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
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_CHORO2_LANDING1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("CBN_N_1"))
    WAIT_MSEC(1000)
    GOTO(98)
    LBL(91)
    SET(LF(0), 0)
    USER_FUNC(btlevtcmd_PreCheckCounter, -2, LW(3), LW(4), PTR(&unitGoldFuzzy_weaponDrainFp), 256, LW(5))
    IF_EQUAL(LW(5), 17)
        SET(LF(0), 1)
    END_IF()
    IF_EQUAL(LW(5), 14)
        SET(LF(0), 1)
    END_IF()
    IF_EQUAL(LF(0), 1)
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
        USER_FUNC(btlevtcmd_ResultACDefence, LW(3), PTR(&unitGoldFuzzy_weaponDrainFp))
        SET(LW(9), PTR(&unitGoldFuzzy_weaponDrainCountered))
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_CHORO2_ATTACK1_1"), EVT_NULLPTR, 0, EVT_NULLPTR)
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
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_CHORO2_ATTACK1_2"), EVT_NULLPTR, 0, EVT_NULLPTR)
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
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_CHORO2_ATTACK2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), PTR(&unitGoldFuzzy_weaponDrainFp))
    SET(LW(9), PTR(&unitGoldFuzzy_weaponDrainFp))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    USER_FUNC(btlevtcmd_GetFpDamage, LW(3), LW(10))
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
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_CHORO2_LANDING1"), EVT_NULLPTR, 0, EVT_NULLPTR)
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
            USER_FUNC(btlevtcmd_AddScale, -2, FLOAT(-0.025), FLOAT(-0.025), 0)
            WAIT_FRM(1)
        WHILE()
        USER_FUNC(btlevtcmd_GetBodyId, -2, LW(1))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(5), LW(6), LW(7))
        ADD(LW(6), 45)
        ADD(LW(7), 10)
        USER_FUNC(btlevtcmd_RecoverFp, -2, LW(1), LW(10))
        USER_FUNC(evt_eff, PTR(""), PTR("recovery"), 8, LW(5), LW(6), LW(7), LW(10), 0, 0, 0, 0, 0, 0, 0)
        USER_FUNC(btlevtcmd_SetScale, -2, FLOAT(1.0), FLOAT(1.0), FLOAT(1.0))
    END_IF()
    DO(2)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        ADD(LW(1), 30)
        USER_FUNC(btlevtcmd_SetMoveSoundDataWork, -2, PTR("SFX_ENM_CHORO2_ATTACK3"), 0, 0, -1, -1, 0, 0)
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
    RUN_CHILD_EVT(PTR(&unitGoldFuzzy_return_event))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitGoldFuzzy_tackle_attack_event)
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), PTR(&unitGoldFuzzy_weaponTackle))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, PTR(&unitGoldFuzzy_weaponTackle), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            RETURN()
        END_IF()
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_WeaponAftereffect, PTR(&unitGoldFuzzy_weaponTackle))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, PTR(&unitGoldFuzzy_weaponTackle))
    USER_FUNC(evt_btl_camera_set_mode, 0, 8)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, LW(3))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 100)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 0, FLOAT(4.0), FLOAT(0.6))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(5), LW(6), LW(7))
    SET(LW(6), 0)
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(5), 50)
    SET(LW(8), 50)
    RUN_CHILD_EVT(PTR(&unitGoldFuzzy_rabbit_move_event))
    WAIT_FRM(20)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 60, 0, FLOAT(0.69921875))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    SUB(LW(2), 2)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, -1)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), PTR(&unitGoldFuzzy_weaponTackle), 256, LW(5))
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
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("CBN_N_1"))
    WAIT_MSEC(1000)
    GOTO(98)
LBL(91)
    DO(4)
        USER_FUNC(btlevtcmd_AddScale, -2, FLOAT(0.08), FLOAT(-0.12), 0)
        WAIT_FRM(1)
    WHILE()
    USER_FUNC(evt_btl_camera_shake_h, 0, 1, 1, 8, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_add_zoom, 0, -50)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), PTR(&unitGoldFuzzy_weaponTackle))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&unitGoldFuzzy_weaponTackle), 256, LW(5))
    DO(4)
        USER_FUNC(btlevtcmd_AddScale, -2, FLOAT(-0.08), FLOAT(0.12), 0)
        WAIT_FRM(1)
    WHILE()
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 40)
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.7))
    USER_FUNC(btlevtcmd_SetScale, -2, FLOAT(1.0), FLOAT(1.0), FLOAT(1.0))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 20, -1)
    DO(2)
        USER_FUNC(btlevtcmd_AddScale, -2, FLOAT(0.16), FLOAT(-0.24), 0)
        WAIT_FRM(1)
    WHILE()
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 20)
    USER_FUNC(btlevtcmd_SetScale, -2, FLOAT(1.0), FLOAT(1.0), FLOAT(1.0))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 10, -1)
    DO(2)
        USER_FUNC(btlevtcmd_AddScale, -2, FLOAT(0.16), FLOAT(-0.24), 0)
        WAIT_FRM(1)
    WHILE()
    DO(2)
        USER_FUNC(btlevtcmd_AddScale, -2, FLOAT(-0.16), FLOAT(0.24), 0)
        WAIT_FRM(1)
    WHILE()
    DO(2)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("CBN_A_1"))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        ADD(LW(1), 30)
        USER_FUNC(btlevtcmd_SetScale, -2, FLOAT(1.0), FLOAT(1.0), FLOAT(1.0))
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 6, -1)
        WAIT_FRM(16)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 6, -1)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("CBN_S_1"))
        USER_FUNC(btlevtcmd_AddScale, -2, FLOAT(0.16), FLOAT(-0.24), 0)
        WAIT_FRM(1)
        USER_FUNC(btlevtcmd_AddScale, -2, FLOAT(0.16), FLOAT(-0.24), 0)
        WAIT_FRM(1)
    WHILE()
    DO(2)
        USER_FUNC(btlevtcmd_AddScale, -2, FLOAT(-0.16), FLOAT(0.24), 0)
        WAIT_FRM(1)
    WHILE()
    WAIT_FRM(10)
    GOTO(98)
LBL(98)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_nomove_y_onoff, 0, 0)
    RUN_CHILD_EVT(PTR(&unitGoldFuzzy_return_event))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
LBL(99)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitGoldFuzzy_attack_event)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_GoldFuzzy_SpawnedHorde, LW(0))
    IF_EQUAL(LW(0), 0)
        GOTO(90)
    END_IF()
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_GoldFuzzy_KillHordeTrigger, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        // If horde is dead, 50% chance to attack normally.
        USER_FUNC(evt_sub_random, 99, LW(0))
        IF_SMALL(LW(0), 50)
            GOTO(90)
        END_IF()
    END_IF()

    // If horde is spawned, use Flower Fuzzy moveset.
    USER_FUNC(btlevtcmd_GetFp, -2, LW(1))
    IF_LARGE_EQUAL(LW(1), 4)
        // If current FP >= 4, FP x 10% chance to use magic attack.
        USER_FUNC(evt_sub_random, 9, LW(0))
        IF_SMALL(LW(0), LW(1))
            RUN_CHILD_EVT(&unitGoldFuzzy_magic_attack_event)
            GOTO(99)
        END_IF()
    END_IF()
    // If magic attack failed and Mario has FP, use FP draining move.
    USER_FUNC(btlevtcmd_GetFp, -3, LW(1))
    IF_LARGE(LW(1), 0)
        RUN_CHILD_EVT(&unitGoldFuzzy_drain_attack_event)
        GOTO(99)
    END_IF()

LBL(90)
    // Otherwise, use regular tackle attack.
    RUN_CHILD_EVT(&unitGoldFuzzy_tackle_attack_event)

LBL(99)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitGoldFuzzy_phase_event)
    // If first phase, and Fuzzy Horde is spawned and not killed...
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_GoldFuzzy_SpawnedHorde, LW(0))
    IF_EQUAL(LW(0), 0)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_GoldFuzzy_KillHordeTrigger, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_CheckPhase, LW(0), 0x400'0001)
    IF_EQUAL(LW(0), 0)
        GOTO(99)
    END_IF()

    // Get the Horde's current HP, divided by 10.
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_GoldFuzzy_HordeId, LW(1))
    USER_FUNC(btlevtcmd_GetHp, LW(1), LW(0))
    DIV(LW(0), 10)
    IF_SMALL(LW(0), 1)
        SET(LW(0), 1)
    END_IF()
    // Restore that much health.
    USER_FUNC(btlevtcmd_RecoverHp, -2, 1, LW(0))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(5), LW(6), LW(7))
    ADD(LW(6), 45)
    ADD(LW(7), 10)
    USER_FUNC(evt_eff, PTR(""), PTR("recovery"), 7, LW(5), LW(6), LW(7), LW(0), 0, 0, 0, 0, 0, 0, 0)
    WAIT_FRM(40)

LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitGoldFuzzy_wait_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitGoldFuzzy_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitGoldFuzzy_wait_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitGoldFuzzy_attack_event))
    USER_FUNC(btlevtcmd_SetEventPhase, -2, PTR(&unitGoldFuzzy_phase_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitGoldFuzzy_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&btldefaultevt_Confuse))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RUN_CHILD_EVT(PTR(&unitFuzzyHorde_ext_entry_event))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_GoldFuzzy_SpawnedHorde, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_GoldFuzzy_KillHordeTrigger, 0)
    USER_FUNC(btlevtcmd_AnimeSetPoseSoundTable, -2, 1, PTR(&unitGoldFuzzy_pose_sound_timing_table))
    USER_FUNC(btlevtcmd_SetJumpSound, -2, PTR("SFX_BOSS_G_CHORO_MOVE1"), PTR("SFX_BOSS_G_CHORO_LANDING1"))
    USER_FUNC(btlevtcmd_SetMaxFp, -2, 10)
    USER_FUNC(btlevtcmd_SetFp, -2, 0)
    RETURN()
EVT_END()

DataTableEntry unitFuzzyHorde_data_table[] = {
    48, (void*)btldefaultevt_Dummy,
    0, nullptr,
};

BattleUnitKind unit_FuzzyHorde = {
    .unit_type = BattleUnitType::FUZZY_HORDE,
    .unit_name = "btl_un_chorobon_gundan",
    .max_hp = 20,
    .max_fp = 0,
    .danger_hp = 1,
    .peril_hp = 1,
    .level = 19,
    .bonus_exp = 0,
    .bonus_coin = 0,
    .bonus_coin_rate = 100,
    .base_coin = 0,
    .run_rate = 50,
    .pb_soft_cap = 5,
    .width = 260,
    .height = 50,
    .hit_offset = { 0, 50 },
    .center_offset = { 0.0f, 0.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 130.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 130.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 130.0f, 32.5f, 0.0f },
    .cut_base_offset = { 0.0f, 25.0f, 0.0f },
    .cut_width = 260.0f,
    .cut_height = 50.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = -1,
    .swallow_attributes = 0,
    .hammer_knockback_chance = 0,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_BOSS_TEAM_CHORO_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0x0024'0000,
    .status_vulnerability = &unitFuzzyHorde_status,
    .num_parts = 1,
    .parts = &unitFuzzyHorde_parts,
    .init_evt_code = (void*)unitFuzzyHorde_init_event,
    .data_table = unitFuzzyHorde_data_table,
};

DataTableEntry unitGoldFuzzy_data_table[] = {
    24, (void*)unitGoldFuzzy_counter_damage_event,
    25, (void*)unitGoldFuzzy_counter_damage_event,
    26, (void*)unitGoldFuzzy_counter_damage_event,
    27, (void*)unitGoldFuzzy_counter_damage_event,
    28, (void*)unitGoldFuzzy_counter_damage_event,
    29, (void*)unitGoldFuzzy_counter_damage_event,
    30, (void*)unitGoldFuzzy_counter_damage_event,
    31, (void*)unitGoldFuzzy_counter_damage_event,
    32, (void*)unitGoldFuzzy_counter_damage_event,
    33, (void*)unitGoldFuzzy_counter_damage_event,
    34, (void*)unitGoldFuzzy_counter_damage_event,
    35, (void*)unitGoldFuzzy_counter_damage_event,
    36, (void*)unitGoldFuzzy_counter_damage_event,
    48, (void*)btldefaultevt_Dummy,
    0, nullptr,
};

BattleUnitKind unit_GoldFuzzy = {
    .unit_type = BattleUnitType::GOLD_FUZZY,
    .unit_name = "btl_un_gold_chorobon",
    .max_hp = 20,
    .max_fp = 0,
    .danger_hp = 1,
    .peril_hp = 1,
    .level = 27,
    .bonus_exp = 0,
    .bonus_coin = 0,
    .bonus_coin_rate = 100,
    .base_coin = 0,
    .run_rate = 50,
    .pb_soft_cap = 5,
    .width = 28,
    .height = 32,
    .hit_offset = { 5, 20 },
    .center_offset = { 0.0f, 16.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 14.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 14.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 14.0f, 20.8f, 0.0f },
    .cut_base_offset = { 0.0f, 16.0f, 0.0f },
    .cut_width = 28.0f,
    .cut_height = 32.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = -1,
    .swallow_attributes = 0,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_BOSS_G_CHORO_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0x0004'0000,
    .status_vulnerability = &unitGoldFuzzy_status,
    .num_parts = 1,
    .parts = &unitGoldFuzzy_parts,
    .init_evt_code = (void*)unitGoldFuzzy_init_event,
    .data_table = unitGoldFuzzy_data_table,
};

// Global state.

ExtFuzzyWork* g_ChorogunWork;
int32_t g_ChorogunTargetId;

EVT_DEFINE_USER_FUNC(chorogun_escape_all) {
    for (int32_t i = 0; i < ttyd::extdrv::extGetPoseNum(); ++i) {
        if (g_ChorogunWork[i].flags & 1) {
            g_ChorogunWork[i].flags &= ~1;
            g_ChorogunWork[i].state = 60;
        }
    }
    return 2;
}

EVT_DEFINE_USER_FUNC(chorogun_escape) {
    // Changed: Made this a parameter of the event function.
    int32_t num_to_escape = evtGetValue(evt, evt->evtArguments[0]);
    
    // Array of candidate fleeing Fuzzies' ids and distances from center.
    FuzzyCenterDistance dist[1000];
  
    BattleWorkUnit* fuzzy_unit = nullptr;
    for (int32_t i = 0; i < 0x40; ++i) {
        auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, i);
        if (unit && unit->true_kind == BattleUnitType::FUZZY_HORDE) {
            fuzzy_unit = unit;
            break;
        }
    }
    
    auto* chorogunWork = g_ChorogunWork;
    auto* poseWork = ttyd::extdrv::extGetPosePtr();
    
    int32_t num_candidates = 0;
    for (int32_t i = 0; i < ttyd::extdrv::extGetPoseNum(); ++i) {
        if (chorogunWork->flags & 1) {
            double dx = fuzzy_unit->home_position.x - poseWork->mtx->m[0][3];
            double dz = fuzzy_unit->home_position.z - poseWork->mtx->m[2][3];
            double distance_sq = dx * dx + dz * dz;
            
            // Find candidates to flee, taking the first N candidates, and all
            // others under a maximum squared distance from the center.
            if (num_candidates < num_to_escape || distance_sq <= 2500.0f) {
                dist[num_candidates].idx = i;
                dist[num_candidates].distance = distance_sq;
                ++num_candidates;
            }
        }
        
        ++chorogunWork;
        ++poseWork;
    }
    
    // Sort by minimum distance, using selection sort to pick smallest N.
    for (int32_t i = 0; i < num_candidates; ++i) {
        if (i >= num_to_escape) break;
        
        int32_t min_idx = i;
        for (int32_t j = i+1; j < num_candidates; ++j) {
            if (dist[j].distance < dist[min_idx].distance)
                min_idx = j;
        }
        
        if (min_idx != i) {
            int32_t tmp_index = dist[i].idx;
            int32_t tmp_distance = dist[i].distance;
            dist[i].idx = dist[min_idx].idx;
            dist[i].distance = dist[min_idx].distance;
            dist[min_idx].idx = tmp_index;
            dist[min_idx].distance = tmp_distance;
        }
    }
    if (num_candidates > num_to_escape) num_candidates = num_to_escape;
    
    for (int32_t i = 0; i < num_candidates; ++i) {
        auto& chorogunWork = g_ChorogunWork[dist[i].idx];
        chorogunWork.flags &= ~1;
        chorogunWork.state = 60;
    }
    
    return 2;
}

EVT_DEFINE_USER_FUNC(chorogun_after_attack) {
    for (int32_t i = 0; i < ttyd::extdrv::extGetPoseNum(); ++i) {
        if (g_ChorogunWork[i].flags & 1)
            g_ChorogunWork[i].state = 50;
    }
    return 2;
}

EVT_DEFINE_USER_FUNC(chorogun_countered) {
    int32_t idx = evtGetValue(evt, evt->evtArguments[0]);
    g_ChorogunWork[idx].state = 48;
    return 2;
}

EVT_DEFINE_USER_FUNC(chorogun_bound_target) {
    int32_t idx = evtGetValue(evt, evt->evtArguments[0]);
    g_ChorogunWork[idx].state = 46;
    return 2;
}

EVT_DEFINE_USER_FUNC(chorogun_dive_target) {
    int32_t candidates[1000];
    
    int32_t num_candidates = 0;
    for (int32_t i = 0; i < ttyd::extdrv::extGetPoseNum(); ++i) {
        // Changed: also ignore Fuzzies currently in 'countered' state.
        if ((g_ChorogunWork[i].flags & 1) &&
             g_ChorogunWork[i].state != 48 && g_ChorogunWork[i].state != 49) {
            candidates[num_candidates] = i;
            ++num_candidates;
        }
    }
    
    int32_t idx = candidates[ttyd::system::irand(num_candidates)];
    auto* chorogunWork = g_ChorogunWork + idx;
    auto* poseWork = ttyd::extdrv::extGetPosePtr() + idx;
    
    int32_t num_frames = evtGetValue(evt, evt->evtArguments[0]);
    
    chorogunWork->state = 45;
    chorogunWork->unk_0x20 = num_frames;
    
    gc::vec3 target_pos;
    auto* target = ttyd::battle::BattleGetUnitPtr(g_BattleWork, g_ChorogunTargetId);
    ttyd::battle_unit::BtlUnit_GetPos(
        target, &target_pos.x, &target_pos.y, &target_pos.z);
    
    chorogunWork->unk_0x04.x = 
        (target_pos.x - poseWork->mtx->m[0][3]) / (float)num_frames;
    chorogunWork->unk_0x04.z = 
        (target_pos.z - poseWork->mtx->m[2][3]) / (float)num_frames;
    // TODO: Possibly a vanilla bug, maybe should use m[1][3] instead?
    chorogunWork->unk_0x04.y = (
            2.0f * (target_pos.y - poseWork->mtx->m[2][3])
          + 0.5f * num_frames * num_frames
        ) / (2.0f * num_frames);
  
    ttyd::evtmgr_cmd::evtSetValue(evt, evt->evtArguments[1], idx);
    
    BattleWorkUnit* fuzzy_unit = nullptr;
    for (int32_t i = 0; i < 0x40; ++i) {
        auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, i);
        if (unit && unit->true_kind == BattleUnitType::FUZZY_HORDE) {
            fuzzy_unit = unit;
            break;
        }
    }
    gc::vec3 sound_pos = {
        poseWork->mtx->m[0][3],
        poseWork->mtx->m[1][3],
        poseWork->mtx->m[2][3]
    };
    ttyd::battle_unit::BtlUnit_snd_se_pos(
        fuzzy_unit, "SFX_BOSS_TEAM_CHORO_ATTACK1", 0x80, 0, &sound_pos);

    return 2;
}

EVT_DEFINE_USER_FUNC(chorogun_approach_target) {
    if (isFirstCall) {
        for (int32_t i = 0; i < ttyd::extdrv::extGetPoseNum(); ++i) {
            if (g_ChorogunWork[i].flags & 1)
                g_ChorogunWork[i].state = 30;
        }
    }
    
    for (int32_t i = 0; i < ttyd::extdrv::extGetPoseNum(); ++i) {
        if ((g_ChorogunWork[i].flags & 1) &&
             g_ChorogunWork[i].state != 40) return 0;
    }
    
    return 2;
}

EVT_DEFINE_USER_FUNC(chorogun_appear) {
    if (isFirstCall) {
        for (int32_t i = 0; i < ttyd::extdrv::extGetPoseNum(); ++i) {
            g_ChorogunWork[i].flags |= 1;
            g_ChorogunWork[i].state = 1;
        }
    }
    
    for (int32_t i = 0; i < ttyd::extdrv::extGetPoseNum(); ++i) {
        if (g_ChorogunWork[i].state != 21) return 0;
    }
    
    return 2;
}

EVT_DEFINE_USER_FUNC(chorogun_set_target_id) {
    g_ChorogunTargetId = evtGetValue(evt, evt->evtArguments[0]);
    return 2;
}

EVT_DEFINE_USER_FUNC(chorogun_get_alive_num) {
    int32_t live_num = 0;
    for (int32_t i = 0; i < ttyd::extdrv::extGetPoseNum(); ++i) {
        if (g_ChorogunWork[i].flags & 1) ++live_num;
    }
    evtSetValue(evt, evt->evtArguments[0], live_num);
    return 2;
}

void chorogun_ext_disp(CameraId camera, void* user_data) {
    ttyd::extdrv::extLoadRenderMode();
    ttyd::extdrv::extLoadVertex();
    ttyd::extdrv::extLoadTexture();
    ttyd::extdrv::extLoadTev();
    ttyd::extdrv::extDraw();
    ttyd::extdrv::extLoadTextureExit();
}

void DebugDispChorogunCount(CameraId camera, void* user_data) {
    // Only display in debug mode.
    if (!g_Mod->state_.CheckOptionValue(OPTVAL_DEBUG_MODE_ENABLED)) return;

    int32_t live_num = 0;
    for (int32_t i = 0; i < ttyd::extdrv::extGetPoseNum(); ++i) {
        if (g_ChorogunWork[i].flags & 1) ++live_num;
    }
    gc::mtx34 mtx;
    gc::mtx::PSMTXTrans(&mtx, 250.0f, -180.0f, 0.0f);
    uint32_t color = 0xFFFFFFFFU;
    if (live_num)
        ttyd::icondrv::iconNumberDispGx(&mtx, live_num, true, &color);
}

void chorogun_ext_main() {
    auto* chorogunWork = g_ChorogunWork;
    auto* poseWork = ttyd::extdrv::extGetPosePtr();
    
    BattleWorkUnit* fuzzy_unit = nullptr;
    for (int32_t i = 0; i < 0x40; ++i) {
        auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, i);
        if (unit && unit->true_kind == BattleUnitType::FUZZY_HORDE) {
            fuzzy_unit = unit;
            break;
        }
    }
  
    int32_t num_active_fuzzies = 0;
    for (int32_t i = 0; i < ttyd::extdrv::extGetPoseNum(); ++i) {
        if (chorogunWork[i].flags & 1)
            ++num_active_fuzzies;
    }
  
    for (int32_t i = 0; i < ttyd::extdrv::extGetPoseNum(); ++i) {
        poseWork->anim_frame = (uint32_t)(
            ttyd::mariost::g_MarioSt->currentRetraceCount + i*i) % 20;
        
        poseWork->mtx->m[0][3] += chorogunWork->unk_0x04.x;
        poseWork->mtx->m[1][3] += chorogunWork->unk_0x04.y;
        poseWork->mtx->m[2][3] += chorogunWork->unk_0x04.z;
        
        switch (chorogunWork->state) {
            case 1: {
                if (--chorogunWork->unk_0x20 < 1) {
                    poseWork->mtx->m[0][3] =
                        ttyd::system::irand(32768) / 32767.0f * 500.0f - 250.0f;
                    poseWork->mtx->m[1][3] =
                        ttyd::system::irand(32768) / 32767.0f * 100.0f + 250.0f;
                    poseWork->mtx->m[2][3] =
                        ttyd::system::irand(32768) / 32767.0f * 800.0f - 100.0f;
                    chorogunWork->unk_0x04.x = 0.0f;
                    chorogunWork->unk_0x04.y = 0.0f;
                    chorogunWork->unk_0x04.z = 0.0f;
                    chorogunWork->unk_0x10 = 3.0f;
                    chorogunWork->state = 2;
                    chorogunWork->unk_0x20 = 0;
                    if (i == 0) {
                        gc::vec3 sound_pos = { 0.0f, 200.f, 200.f };
                        ttyd::battle_unit::BtlUnit_snd_se_pos(
                            fuzzy_unit, "SFX_BOSS_TEAM_CHORO_FALL2", 0x7f, 0, &sound_pos);
                    }
                }
                break;
            }
            case 2: {
                if (poseWork->mtx->m[1][3] > 0.0f) {
                    chorogunWork->unk_0x04.y -= 0.25f;
                } else {
                    if (i % 100 == 0) {
                        gc::vec3 sound_pos = {
                            poseWork->mtx->m[0][3],
                            poseWork->mtx->m[1][3],
                            poseWork->mtx->m[2][3]
                        };
                        ttyd::battle_unit::BtlUnit_snd_se_pos(
                            fuzzy_unit, "SFX_BOSS_TEAM_CHORO_FALL1", 0x7f, 0, &sound_pos);
                    }
                    chorogunWork->state = 10;
                }
                
                break;
            }
            case 10: {
                auto* mtx = poseWork->mtx;
                
                if (mtx->m[1][3] > 0.0f) {
                    chorogunWork->unk_0x04.y -= 0.5f;
                } else {
                    double dx = chorogunWork->unk_0x14.x - mtx->m[0][3];
                    double dz = chorogunWork->unk_0x14.z - mtx->m[2][3];
                    double distance = ttyd::_core_sqrt(dx * dx + dz * dz);
                    
                    if (i % 100 == 0) {
                        gc::vec3 sound_pos = {
                            poseWork->mtx->m[0][3],
                            poseWork->mtx->m[1][3],
                            poseWork->mtx->m[2][3]
                        };
                        ttyd::battle_unit::BtlUnit_snd_se_pos(
                            fuzzy_unit, "SFX_BOSS_TEAM_CHORO_TOGETHER1", 0x7f, 0, &sound_pos);
                    }
                    
                    if (distance > 5.0f) {
                        if (distance <= 20.0f) {
                            chorogunWork->unk_0x10 *= 0.9f;
                        }
                        chorogunWork->unk_0x04.x = chorogunWork->unk_0x10 * dx / distance;
                        chorogunWork->unk_0x04.y = chorogunWork->unk_0x10;
                        chorogunWork->unk_0x04.z = chorogunWork->unk_0x10 * dz / distance;
                        poseWork->mtx->m[1][3] = 0.0f;
                    } else {
                        chorogunWork->state = 20;
                    }
                }
                
                break;
            }
            case 20: {
                poseWork->mtx->m[1][3] = 0.0f;
                chorogunWork->unk_0x04.x = 0.0f;
                chorogunWork->unk_0x04.y = chorogunWork->unk_0x10;
                chorogunWork->unk_0x04.z = 0.0f;
                chorogunWork->state = 21;
                break;
            }
            case 21: {
                if (fuzzy_unit->position.y < poseWork->mtx->m[1][3]) {
                    chorogunWork->unk_0x04.y -= 0.5f;
                } else {
                    double dx = ttyd::system::intplGetValue(
                        chorogunWork->unk_0x14.x, fuzzy_unit->home_position.x,
                        0, 1000 - num_active_fuzzies, 1000);
                    dx = fuzzy_unit->position.x - fuzzy_unit->home_position.x 
                        + dx - poseWork->mtx->m[0][3];
                    
                    double dz = ttyd::system::intplGetValue(
                        chorogunWork->unk_0x14.z, fuzzy_unit->home_position.z,
                        0, 1000 - num_active_fuzzies, 1000);
                    dz = fuzzy_unit->position.z - fuzzy_unit->home_position.z 
                        + dz - poseWork->mtx->m[2][3];
                    
                    double distance = ttyd::_core_sqrt(dx * dx + dz * dz);
                    
                    if (i % 100 == 0) {
                        gc::vec3 sound_pos = {
                            poseWork->mtx->m[0][3],
                            poseWork->mtx->m[1][3],
                            poseWork->mtx->m[2][3]
                        };
                        ttyd::battle_unit::BtlUnit_snd_se_pos(
                            fuzzy_unit, "SFX_BOSS_TEAM_CHORO_WAIT1", 0x60, 0, &sound_pos);
                    }
                    
                    chorogunWork->unk_0x10 =
                        3.0f + (0.5f * ttyd::system::irand(32768)) / 32767.0f;
                    chorogunWork->unk_0x04.x = chorogunWork->unk_0x10 * dx / distance;
                    chorogunWork->unk_0x04.y = chorogunWork->unk_0x10;
                    chorogunWork->unk_0x04.z = chorogunWork->unk_0x10 * dz / distance;
                    poseWork->mtx->m[1][3] = fuzzy_unit->position.y;
                }
                
                break;
            }
            case 30: {
                chorogunWork->state = 31;
                chorogunWork->unk_0x20 = 0;
                
                auto* target = ttyd::battle::BattleGetUnitPtr(
                    g_BattleWork, g_ChorogunTargetId);
                gc::vec3 target_pos;
                ttyd::battle_unit::BtlUnit_GetPos(
                    target, &target_pos.x, &target_pos.y, &target_pos.z);
                    
                target_pos.x +=
                    ttyd::system::irand(32768) / 32767.0f * 50.0f - 25.0f;
                chorogunWork->unk_0x24.x = target_pos.x;
                target_pos.z +=
                    ttyd::system::irand(32768) / 32767.0f * 50.0f - 25.0f;
                chorogunWork->unk_0x24.z = target_pos.z;
                
                // intentional fallthrough!
            }
            case 31: {
                auto* mtx = poseWork->mtx;
                
                if (mtx->m[1][3] > 0.0f) {
                    chorogunWork->unk_0x04.y -= 0.5f;
                } else {
                    double dx = chorogunWork->unk_0x24.x - mtx->m[0][3];
                    double dz = chorogunWork->unk_0x24.z - mtx->m[2][3];
                    double distance = ttyd::_core_sqrt(dx * dx + dz * dz);
                    
                    if (i % 100 == 0) {
                        gc::vec3 sound_pos = {
                            poseWork->mtx->m[0][3],
                            poseWork->mtx->m[1][3],
                            poseWork->mtx->m[2][3]
                        };
                        ttyd::battle_unit::BtlUnit_snd_se_pos(
                            fuzzy_unit, "SFX_BOSS_TEAM_CHORO_MOVE1", 0x60, 0, &sound_pos);
                    }
                    
                    if (distance <= 30.0f || chorogunWork->unk_0x20 >= 240) {
                        chorogunWork->state = 40;
                    } else {
                        chorogunWork->unk_0x04.x = chorogunWork->unk_0x10 * dx / distance;
                        chorogunWork->unk_0x04.y = chorogunWork->unk_0x10;
                        chorogunWork->unk_0x04.z = chorogunWork->unk_0x10 * dz / distance;
                        poseWork->mtx->m[1][3] = 0.0f;
                        ++chorogunWork->unk_0x20;
                    }
                }
                
                break;
            }
            case 40: {
                if (0.0f < poseWork->mtx->m[1][3]) {
                    chorogunWork->unk_0x04.y -= 0.5f;
                } else {
                    poseWork->mtx->m[1][3] = 0.0f;
                    chorogunWork->unk_0x04.x = 0.0f;
                    chorogunWork->unk_0x04.y = chorogunWork->unk_0x10;
                    chorogunWork->unk_0x04.z = 0.0f;
                }
                
                break;
            }
            case 45: {
                if (--chorogunWork->unk_0x20 < 1) {
                    chorogunWork->unk_0x20 = 0;
                    chorogunWork->unk_0x04.y = 0.0f;
                } else {
                    chorogunWork->unk_0x04.y -= 0.5f;
                }
                
                break;
            }
            case 46: {
                chorogunWork->state = 47;
                chorogunWork->unk_0x20 = 0;
                chorogunWork->unk_0x04.y = 6.0f;
                if (poseWork->mtx->m[1][3] <= 0.0f) {
                    poseWork->mtx->m[1][3] = 0.1f;
                }
                
                // intentional fallthrough
            }
            case 47: {
                if (0.0f < poseWork->mtx->m[1][3]) {
                    chorogunWork->unk_0x04.y -= 0.5f;
                } else {
                    poseWork->mtx->m[1][3] = 0.0f;
                    chorogunWork->unk_0x04.x = 0.0f;
                    chorogunWork->unk_0x04.y = chorogunWork->unk_0x10;
                    chorogunWork->unk_0x04.z = 0.0f;
                    chorogunWork->state = 40;
                }
                
                break;
            }
            case 48: {
                chorogunWork->state = 49;
                
                chorogunWork->unk_0x20 = 0;
                chorogunWork->unk_0x04.x = -4.0f;
                chorogunWork->unk_0x04.y = 10.0f;
                if (poseWork->mtx->m[1][3] <= 0.0f) {
                    poseWork->mtx->m[1][3] = 0.1f;
                }
                
                // intentional fallthrough
            }
            case 49: {
                if (0.0f < poseWork->mtx->m[1][3]) {
                    chorogunWork->unk_0x04.y -= 0.5f;
                }
                else {
                    chorogunWork->state = 40;
                    
                    poseWork->mtx->m[1][3] = 0.0f;
                    chorogunWork->unk_0x04.x = 0.0f;
                    chorogunWork->unk_0x04.y = chorogunWork->unk_0x10;
                    chorogunWork->unk_0x04.z = 0.0f;
                }
                
                break;
            }
            case 50: {
                chorogunWork->state = 20;
                break;
            }
            case 60: {
                chorogunWork->state = 61;
                break;
            }
            case 61: {
                if (poseWork->mtx->m[1][3] > 0.0f) {
                    chorogunWork->unk_0x04.y -= 0.5f;
                } else {
                    if (i % 100 == 0) {
                        gc::vec3 sound_pos = {
                            poseWork->mtx->m[0][3],
                            poseWork->mtx->m[1][3],
                            poseWork->mtx->m[2][3]
                        };
                        ttyd::battle_unit::BtlUnit_snd_se_pos(
                            fuzzy_unit, "SFX_BOSS_TEAM_CHORO_MOVE1", 0x60, 0, &sound_pos);
                    }
                    
                    poseWork->mtx->m[1][3] = 0.0f;
                    chorogunWork->unk_0x04.x = 5.0f;
                    chorogunWork->unk_0x04.y =
                        chorogunWork->unk_0x10 + ttyd::system::irand(5);
                    chorogunWork->unk_0x04.z = 5.0f;
                }
                
                if (poseWork->mtx->m[0][3] >= 250.0f) {
                    chorogunWork->state = 99;
                }
                
                break;
            }
            case 99: {
                poseWork->mtx->m[0][3] = 0.0f;
                poseWork->mtx->m[1][3] = -1000.0f;
                poseWork->mtx->m[2][3] = 0.0f;
                break;
            }
        }
        
        ++poseWork;
        ++chorogunWork;
    }
  
    ttyd::dispdrv::dispEntry(CameraId::k3d, 1, 0.0f, chorogun_ext_disp, nullptr);
    ttyd::dispdrv::dispEntry(CameraId::k2d, 1, 902.0f, DebugDispChorogunCount, nullptr);
}

void chorogun_ext_init() {
    g_ChorogunWork = 
        (ExtFuzzyWork*)ttyd::battle::BattleAlloc(sizeof(ExtFuzzyWork) * 1000);
  
    auto* chorogunWork = g_ChorogunWork;
    auto* poseWork = ttyd::extdrv::extGetPosePtr();
  
    for (int32_t i = 0; i < ttyd::extdrv::extGetPoseNum(); ++i) {
        poseWork->flags = 1;
        
        gc::mtx::PSMTXIdentity(poseWork->mtx);
        poseWork->mtx->m[0][3] = 0.0f;
        poseWork->mtx->m[1][3] = -1000.0f;
        poseWork->mtx->m[2][3] = 0.0f;
        poseWork->facing_dir = 90.0f;
        
        chorogunWork->flags = 0;
        chorogunWork->state = 0;
        chorogunWork->unk_0x04.x = 0.0f;
        chorogunWork->unk_0x04.y = 0.0f;
        chorogunWork->unk_0x04.z = 0.0f;
        if (i == 0) {
            chorogunWork->unk_0x20 = 22;
        } else {
            chorogunWork->unk_0x20 = ttyd::system::irand(45);
        }
        
        chorogunWork->unk_0x14.x =
            (5.0f * (i % 50) - 50.0f) + 
            ttyd::system::irand(32768) / 32767.0f * 10.0f - 5.0f;
        chorogunWork->unk_0x14.y = 0.0f;
        chorogunWork->unk_0x14.z =
            (7.0f * (i / 50) - 70.0f) + 
            ttyd::system::irand(32768) / 32767.0f * 5.0f - 2.5f;
        
        ++chorogunWork;
        ++poseWork;
    }
    
    for (int32_t i = 0; i < 500; ++i) {
        auto& a = g_ChorogunWork[ttyd::system::irand(ttyd::extdrv::extGetPoseNum())];
        auto& b = g_ChorogunWork[ttyd::system::irand(ttyd::extdrv::extGetPoseNum())];
        gc::vec3 tmp = { a.unk_0x14.x, a.unk_0x14.y, a.unk_0x14.z };
        a.unk_0x14.x = b.unk_0x14.x;
        a.unk_0x14.y = b.unk_0x14.y;
        a.unk_0x14.z = b.unk_0x14.z;
        b.unk_0x14.x = tmp.x;
        b.unk_0x14.y = tmp.y;
        b.unk_0x14.z = tmp.z;
    }
}

EVT_DEFINE_USER_FUNC(get_gold_chorobon_id) {
    for (int32_t i = 0; i < 0x40; ++i) {
        auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, i);
        if (unit && unit->true_kind == BattleUnitType::GOLD_FUZZY) {
            evtSetValue(evt, evt->evtArguments[0], i);
            return 2;
        }
    }
    evtSetValue(evt, evt->evtArguments[0], -1);
    return 2;
}

}  // namespace mod::tot::custom