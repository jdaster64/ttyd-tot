#include "tot_custom_rel.h"     // For externed unit declarations

#include "evt_cmd.h"
#include "mod.h"
#include "tot_gsw.h"
#include "tot_manager_achievements.h"
#include "tot_manager_dialogue.h"
#include "tot_state.h"

#include <gc/types.h>
#include <ttyd/battle.h>
#include <ttyd/battle_camera.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_event_default.h>
#include <ttyd/battle_event_subset.h>
#include <ttyd/battle_message.h>
#include <ttyd/battle_sub.h>
#include <ttyd/battle_unit.h>
#include <ttyd/battle_weapon_power.h>
#include <ttyd/dispdrv.h>
#include <ttyd/effdrv.h>
#include <ttyd/eff_magic1_n64.h>
#include <ttyd/eff_stardust.h>
#include <ttyd/eff_thunderflare_n64.h>
#include <ttyd/evt_eff.h>
#include <ttyd/evt_map.h>
#include <ttyd/evt_msg.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evt_sub.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/icondrv.h>
#include <ttyd/system.h>

#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace mod::tot::custom {

namespace {

// Using entire namespace for convenience.
using namespace ::ttyd::battle_camera;
using namespace ::ttyd::battle_database_common;
using namespace ::ttyd::battle_event_cmd;
using namespace ::ttyd::battle_event_default;
using namespace ::ttyd::battle_event_subset;
using namespace ::ttyd::battle_message;
using namespace ::ttyd::evt_eff;
using namespace ::ttyd::evt_map;
using namespace ::ttyd::evt_msg;
using namespace ::ttyd::evt_snd;
using namespace ::ttyd::evt_sub;

using ::ttyd::dispdrv::CameraId;
using ::ttyd::effdrv::EffEntry;
using ::ttyd::evtmgr::EvtEntry;
using ::ttyd::evtmgr_cmd::evtGetFloat;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetFloat;
using ::ttyd::evtmgr_cmd::evtSetValue;

namespace BattleUnitAttribute_Flags = ttyd::battle_unit::BattleUnitAttribute_Flags;
namespace IconType = ::ttyd::icondrv::IconType;

}  // namespace

// Unit work variable definitions.
constexpr const int32_t UW_Bowser_IsLeaning = 0;
constexpr const int32_t UW_Bowser_HasTaunted = 1;
constexpr const int32_t UW_Bowser_HpThreshold = 2;
constexpr const int32_t UW_Bowser_LowHpEvent = 3;
constexpr const int32_t UW_Bowser_AiState = 4;
constexpr const int32_t UW_Bowser_Target = 5;

constexpr const int32_t UW_Kammy_IsFlying = 0;
constexpr const int32_t UW_Kammy_DamageTaken = 1;
constexpr const int32_t UW_Kammy_TurnsGrounded = 2;
constexpr const int32_t UW_Kammy_FlareEff = 3;
constexpr const int32_t UW_Kammy_MagicEff = 4;
constexpr const int32_t UW_Kammy_AiState = 5;
constexpr const int32_t UW_Kammy_NumHeals = 6;

// Handles phase changes + determines available moves.
namespace KammyAiState {
    enum e {
        PHASE_1_ATTACK = 0,
        PHASE_1_BUFF_BOWSER,
        PHASE_1_BUFF_SELF,
        PHASE_2_ATTACK = 100,
        PHASE_2_BUFF_BOWSER,
        PHASE_2_BUFF_SELF,
    };
}

// Function / USER_FUNC declarations.
EVT_DECLARE_USER_FUNC(evtTot_Bowser_CommandLossIconBounce, 9)
EVT_DECLARE_USER_FUNC(evtTot_Bowser_MarioCommandDisabled, 1)
EVT_DECLARE_USER_FUNC(evtTot_Bowser_PartyCommandDisabled, 2)
EVT_DECLARE_USER_FUNC(evtTot_Kammy_MagicSupport, 2)
EVT_DECLARE_USER_FUNC(evtTot_Kammy_GetMagicParticlesX, 2)
EVT_DECLARE_USER_FUNC(evtTot_Kammy_SpawnMagicParticles, 5)
EVT_DECLARE_USER_FUNC(evtTot_Kammy_HandleFlareParticles, 1)


int8_t unitKammy_defense[] = { 0, 0, 0, 0, 0 };
int8_t unitKammy_defense_attr[] = { 0, 0, 0, 0, 0 };

StatusVulnerability unitKammy_status = {
     40,  30,  60, 100,  40, 100, 100,  40,
    100,  60, 100,  60, 100,  80,  40,   0,
      0, 100,  40, 100, 100,   0,
};

PoseTableEntry unitKammy_pose_table[] = {
    1, "N_1",
    2, "Y_1",
    9, "Y_1",
    5, "K_1",
    4, "X_1",
    3, "X_1",
    28, "S_1",
    29, "Q_1",
    30, "Q_1",
    31, "D_1",
    39, "D_1",
    42, "R_1",
    40, "W_1",
    56, "X_1",
    57, "X_1",
    65, "T_1",
    69, "S_1",
};

PoseTableEntry unitKammy_pose_table_flying[] = {
    1, "N_2",
    2, "Y_2",
    9, "Y_2",
    5, "K_2",
    4, "X_2",
    3, "X_2",
    28, "S_2",
    29, "Q_2",
    30, "Q_2",
    31, "D_2",
    39, "D_2",
    42, "R_2",
    40, "W_2",
    56, "X_2",
    57, "X_2",
    65, "T_2",
    69, "S_2",
};

PoseTableEntry unitKammy_pose_table_dead[] = {
    28, "D_3",
    39, "D_3",
    69, "D_3",
};

BattleWeapon unitKammy_weaponBlast = {
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
        AttackSpecialProperty_Flags::DEFENSE_PIERCING |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
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

BattleWeapon unitKammy_weaponAtkMagic = {
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
        AttackTargetProperty_Flags::TARGET_SAME_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags = AttackSpecialProperty_Flags::CANNOT_MISS,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = AttackTargetWeighting_Flags::WEIGHTED_RANDOM,
        
    // status chances
    .atk_change_chance = 100,
    .atk_change_time = 3,
    .atk_change_strength = 2,
    
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

BattleWeapon unitKammy_weaponDefMagic = {
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
        AttackTargetProperty_Flags::TARGET_SAME_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags = AttackSpecialProperty_Flags::CANNOT_MISS,
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

BattleWeapon unitKammy_weaponHugeMagic = {
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
        AttackTargetProperty_Flags::TARGET_SAME_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags = AttackSpecialProperty_Flags::CANNOT_MISS,
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

BattleWeapon unitKammy_weaponElectricMagic = {
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
        AttackTargetProperty_Flags::TARGET_SAME_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags = AttackSpecialProperty_Flags::CANNOT_MISS,
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

BattleWeapon unitKammy_weaponInvisMagic = {
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
        AttackTargetProperty_Flags::TARGET_SAME_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags = AttackSpecialProperty_Flags::CANNOT_MISS,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = AttackTargetWeighting_Flags::WEIGHTED_RANDOM,
        
    // status chances
    .invisible_chance = 100,
    .invisible_time = 2,
    
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

BattleWeapon unitKammy_weaponFastMagic = {
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
        AttackTargetProperty_Flags::TARGET_SAME_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags = AttackSpecialProperty_Flags::CANNOT_MISS,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = AttackTargetWeighting_Flags::WEIGHTED_RANDOM,
        
    // status chances
    .fast_chance = 100,
    .fast_time = 4,
    
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

BattleWeapon unitKammy_weaponHealingMagic = {
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
        AttackTargetProperty_Flags::TARGET_SAME_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags = AttackSpecialProperty_Flags::CANNOT_MISS,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::PREFER_IN_PERIL |
        AttackTargetWeighting_Flags::PREFER_LESS_HEALTHY |
        AttackTargetWeighting_Flags::PREFER_LOWER_HP,
        
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

BattleUnitKindPart unitKammy_parts[] = {
    {
        .index = 1,
        .name = "btl_un_kamec_obaba",
        .model_name = "c_kamek_bb",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 30.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 40.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitKammy_defense,
        .defense_attr = unitKammy_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitKammy_pose_table_flying,
    },
    {
        .index = 2,
        .name = "btl_un_kamec_obaba",
        .model_name = "c_kamek_bb",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitKammy_defense,
        .defense_attr = unitKammy_defense_attr,
        .attribute_flags = 0x1301'0000,
        .counter_attribute_flags = 0,
        .pose_table = unitKammy_pose_table,
    },
};

EVT_BEGIN(unitKammy_camera_focus_event)
    USER_FUNC(btlevtcmd_GetUnitWork, GW(9), UW_Bowser_IsLeaning, LW(1))
    USER_FUNC(evt_btl_camera_set_mode, 0, 3)
    IF_EQUAL(LW(1), 1)
        USER_FUNC(evt_btl_camera_set_moveto, 0, 135, 106, 296, 135, 55, -402, 30, 11)
    ELSE()
        USER_FUNC(evt_btl_camera_set_moveto, 0, 140, 34, 296, 140, 50, -402, 30, 11)
    END_IF()
    WAIT_FRM(37)
    RETURN()
EVT_END()

EVT_BEGIN(unitKammy_ground_magic_common_1)
    BROTHER_EVT()
        DO(2)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_KAMEBABA_MAGIC1"), EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_FRM(40)
        WHILE()
    END_BROTHER()
    BROTHER_EVT()
        USER_FUNC(evtTot_Kammy_HandleFlareParticles, -2)
    END_BROTHER()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_1A"))
    WAIT_FRM(60)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_KAMEBABA_MAGIC4"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_1B"))
    WAIT_FRM(20)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_1C"))
    WAIT_FRM(30)
    BROTHER_EVT()
        WAIT_FRM(10)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_KAMEBABA_MAGIC8"), EVT_NULLPTR, 0, EVT_NULLPTR)
    END_BROTHER()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_1D"))
    WAIT_FRM(20)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_1E"))
    IF_NOT_EQUAL(LW(9), PTR(&unitKammy_weaponBlast))
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_KAMEBABA_MAGIC2"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(evtTot_Kammy_MagicSupport, -2, 0)
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitKammy_ground_magic_common_2)
    BROTHER_EVT()
        DO(2)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_KAMEBABA_MAGIC1"), EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_FRM(40)
        WHILE()
    END_BROTHER()
    BROTHER_EVT()
        USER_FUNC(evtTot_Kammy_HandleFlareParticles, -2)
    END_BROTHER()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_1A"))
    WAIT_FRM(60)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_KAMEBABA_MAGIC6"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_2A"))
    WAIT_FRM(40)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_KAMEBABA_MAGIC7"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_2B"))
    WAIT_FRM(20)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_2C"))
    IF_NOT_EQUAL(LW(9), PTR(&unitKammy_weaponBlast))
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_KAMEBABA_MAGIC2"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(evtTot_Kammy_MagicSupport, -2, 1)
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitKammy_sky_magic_common)
    BROTHER_EVT()
        DO(2)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_KAMEBABA_MAGIC1"), EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_FRM(40)
        WHILE()
    END_BROTHER()
    BROTHER_EVT()
        USER_FUNC(evtTot_Kammy_HandleFlareParticles, -2)
    END_BROTHER()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_3A"))
    WAIT_FRM(60)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_KAMEBABA_MAGIC8"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_3B"))
    WAIT_FRM(35)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_KAMEBABA_MAGIC7"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_3C"))
    WAIT_FRM(15)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_3D"))
    IF_NOT_EQUAL(LW(9), PTR(&unitKammy_weaponBlast))
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_KAMEBABA_MAGIC2"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(evtTot_Kammy_MagicSupport, -2, 1)
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitKammy_magic_common_event)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Kammy_IsFlying, LW(0))
    IF_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_GetUnitId, -2, LW(13))
        IF_NOT_EQUAL(LW(3), LW(13))
            RUN_CHILD_EVT(PTR(&unitKammy_ground_magic_common_1))
        ELSE()
            RUN_CHILD_EVT(PTR(&unitKammy_ground_magic_common_2))
        END_IF()
    ELSE()
        RUN_CHILD_EVT(PTR(&unitKammy_sky_magic_common))
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitKammy_start_avoid_event)
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

// Different attacks...

EVT_BEGIN(unitKammy_blast_magic_event)
    // Pre-select multiple targets for the attack.
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(9))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(9), 
        (int32_t)GSW_Boss_Target1_Unit, (int32_t)GSW_Boss_Target1_Part)
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(9))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(9), 
        (int32_t)GSW_Boss_Target2_Unit, (int32_t)GSW_Boss_Target2_Part)

    SET(LW(3), (int32_t)GSW_Boss_Target1_Unit)
    SET(LW(4), (int32_t)GSW_Boss_Target1_Part)
    SET(LW(13), 0)
    SET(LW(14), 0)

    // If either target is invalid, skip the attack entirely.
    IF_EQUAL((int32_t)GSW_Boss_Target1_Unit, -1)
        GOTO(4)
    END_IF()
    IF_EQUAL((int32_t)GSW_Boss_Target2_Unit, -1)
        GOTO(4)
    END_IF()
    GOTO(5)

LBL(4)
    USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
        RETURN()
    END_IF()
    GOTO(99)

LBL(5)
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))

LBL(10)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    RUN_CHILD_EVT(PTR(&unitKammy_magic_common_event))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_KAMEBABA_MAGIC3_2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))

    // Hit forced if targeting the same character twice, and hit last time.
    IF_EQUAL(LW(13), 1)
        GOTO(91)
    END_IF()

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
    BROTHER_EVT()
        WAIT_FRM(1)
        DO(0)
            USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(0))
            IF_EQUAL(LW(0), -1)
                USER_FUNC(evtTot_Kammy_GetMagicParticlesX, -2, LW(0))
                USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(6), LW(7), LW(8))
                IF_SMALL_EQUAL(LW(0), LW(6))
                    RUN_CHILD_EVT(PTR(&unitKammy_start_avoid_event))
                    DO_BREAK()
                END_IF()
            ELSE()
                USER_FUNC(evtTot_Kammy_GetMagicParticlesX, -2, LW(0))
                USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(6), LW(7), LW(8))
                IF_LARGE_EQUAL(LW(0), LW(6))
                    RUN_CHILD_EVT(PTR(&unitKammy_start_avoid_event))
                    DO_BREAK()
                END_IF()
            END_IF()
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    ADD(LW(0), -57)
    ADD(LW(1), 10)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(10), LW(11), LW(12))
    USER_FUNC(btlevtcmd_GetStageSize, LW(10), EVT_NULLPTR, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(0))
    MUL(LW(10), LW(0))
    SUB(LW(11), 5)
    USER_FUNC(evtTot_Kammy_SpawnMagicParticles, -2, LW(10), LW(11), LW(12), 60)
    GOTO(92)

LBL(91)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    ADD(LW(0), -57)
    ADD(LW(1), 10)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(10), LW(11), LW(12))
    USER_FUNC(evtTot_Kammy_SpawnMagicParticles, -2, LW(10), LW(11), LW(12), 45)

    IF_EQUAL(LW(14), 0)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Kammy_AiState, LW(0))
        IF_LARGE_EQUAL(LW(0), (int32_t)KammyAiState::PHASE_2_ATTACK)
            IF_EQUAL((int32_t)GSW_Boss_Target1_Unit, (int32_t)GSW_Boss_Target2_Unit)
                // If targeting the same character twice...
                // Hit with a non-lethal attack, then force a lethal hit next.
                BROTHER_EVT()
                    WAIT_FRM(45)
                    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
                    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 0, LW(5))
                END_BROTHER()
                SET(LW(13), 1)
                GOTO(92)
            END_IF()
        END_IF()
    END_IF()

    BROTHER_EVT()
        WAIT_FRM(45)
        USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    END_BROTHER()

LBL(92)
    // Attack multiple targets when at low health.
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Kammy_AiState, LW(0))
    IF_LARGE_EQUAL(LW(0), (int32_t)KammyAiState::PHASE_2_ATTACK)
        IF_SMALL(LW(14), 1)
            SET(LW(3), (int32_t)GSW_Boss_Target2_Unit)
            SET(LW(4), (int32_t)GSW_Boss_Target2_Part)
            ADD(LW(14), 1)
            GOTO(10)
        END_IF()
    END_IF()
    
    WAIT_FRM(45)
LBL(98)
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 43)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitKammy_status_magic_event)
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
    RUN_CHILD_EVT(PTR(&unitKammy_magic_common_event))
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    WAIT_FRM(30)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitKammy_recover_magic_event)
    USER_FUNC(btlevtcmd_AddUnitWork, -2, UW_Kammy_NumHeals, 1)
    
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    RUN_CHILD_EVT(PTR(&unitKammy_magic_common_event))
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    WAIT_FRM(30)

    // Recover 20% of max HP (or 99 max).
    USER_FUNC(btlevtcmd_GetMaxHp, LW(3), LW(10))
    MUL(LW(10), 20)
    DIV(LW(10), 100)
    IF_LARGE(LW(10), 99)
        SET(LW(10), 99)
    END_IF()  
    USER_FUNC(btlevtcmd_RecoverHp, LW(3), LW(4), LW(10))
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    IF_EQUAL(LW(3), GW(8))
        ADD(LW(0), -30)
        ADD(LW(1), 100)
        ADD(LW(2), 10)
    ELSE()
        ADD(LW(1), 50)
        ADD(LW(2), 10)
    END_IF()
    USER_FUNC(evt_eff, PTR(""), PTR("recovery"), 0, LW(0), LW(1), LW(2), LW(10), 0, 0, 0, 0, 0, 0, 0)
    WAIT_FRM(40)

LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitKammy_attack_event)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Kammy_AiState, LW(15))
    SWITCH(LW(15))
        CASE_OR((int32_t)KammyAiState::PHASE_1_BUFF_BOWSER)
        CASE_OR((int32_t)KammyAiState::PHASE_2_BUFF_BOWSER)
            USER_FUNC(btlevtcmd_AddUnitWork, -2, UW_Kammy_AiState, 1)
            
            // If Bowser is already KO'd, advance to self-buff phase.
            IF_LARGE(GW(10), 0)
                GOTO(50)
            END_IF()
            
            SET(LW(3), GW(8))
            SET(LW(4), 1)
            SET(LW(5), 30)      // ATK buff weight
            SET(LW(6), 30)      // DEF buff
            SET(LW(7), 10)      // Electric buff
            SET(LW(8), 10)      // Invis buff
            SET(LW(9), 50)      // Huge buff
            SET(LW(10), 0)      // Fast buff
            SET(LW(11), 0)      // Healing
            
            GOTO(80)
            CASE_END()

        CASE_OR((int32_t)KammyAiState::PHASE_1_BUFF_SELF)
        CASE_OR((int32_t)KammyAiState::PHASE_2_BUFF_SELF)
LBL(50)
            USER_FUNC(btlevtcmd_AddUnitWork, -2, UW_Kammy_AiState, -2)
            
            SET(LW(3), -2)
            SET(LW(4), 1)
            SET(LW(5), 10)      // ATK buff weight
            SET(LW(6), 10)      // DEF buff
            SET(LW(7), 30)      // Electric buff
            SET(LW(8), 30)      // Invis buff
            SET(LW(9), 0)       // Huge buff
            SET(LW(10), 50)     // Fast buff
            SET(LW(11), 0)      // Healing

LBL(80)
            
            // Don't re-up statuses that are already active.
            USER_FUNC(btlevtcmd_CheckStatus, LW(3), (int32_t)StatusEffectType::ATTACK_UP, LW(0))
            IF_LARGE(LW(0), 0)
                SET(LW(5), 0)
            END_IF()
            
            USER_FUNC(btlevtcmd_CheckStatus, LW(3), (int32_t)StatusEffectType::DEFENSE_UP, LW(0))
            IF_LARGE(LW(0), 0)
                SET(LW(6), 0)
            END_IF()
            
            USER_FUNC(btlevtcmd_CheckStatus, LW(3), (int32_t)StatusEffectType::ELECTRIC, LW(0))
            IF_LARGE(LW(0), 0)
                SET(LW(7), 0)
            END_IF()
            
            USER_FUNC(btlevtcmd_CheckStatus, LW(3), (int32_t)StatusEffectType::INVISIBLE, LW(0))
            IF_LARGE(LW(0), 0)
                SET(LW(8), 0)
            END_IF()

            USER_FUNC(btlevtcmd_CheckStatus, LW(3), (int32_t)StatusEffectType::HUGE, LW(0))
            IF_LARGE(LW(0), 0)
                SET(LW(9), 0)
            END_IF()

            USER_FUNC(btlevtcmd_CheckStatus, LW(3), (int32_t)StatusEffectType::FAST, LW(0))
            IF_LARGE(LW(0), 0)
                SET(LW(10), 0)
            END_IF()

            // Only use Huge / Fast statuses once in low-health phase.
            IF_SMALL(LW(15), (int32_t)KammyAiState::PHASE_2_ATTACK)
                SET(LW(9), 0)
                SET(LW(10), 0)
            END_IF()
                
            USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Kammy_NumHeals, LW(0))
            IF_SMALL(LW(0), 3)
                USER_FUNC(btlevtcmd_GetHp, LW(3), LW(13))
                USER_FUNC(btlevtcmd_GetMaxHp, LW(3), LW(14))
                MUL(LW(13), 100)
                DIV(LW(13), LW(14))
                
                // Weight for healing = 0 - 100, from 50% - 0% of max HP.
                SUB(LW(13), 50)
                MUL(LW(13), -2)
                IF_LARGE(LW(13), 0)
                    SET(LW(11), LW(13))
                END_IF()
            END_IF()

            USER_FUNC(btlevtcmd_DrawLots, LW(0), 7, LW(5), LW(6), LW(7), LW(8), LW(9), LW(10), LW(11))
            SWITCH(LW(0))
                CASE_EQUAL(0)
                    SET(LW(9), PTR(&unitKammy_weaponAtkMagic))
                CASE_EQUAL(1)
                    SET(LW(9), PTR(&unitKammy_weaponDefMagic))
                CASE_EQUAL(2)
                    SET(LW(9), PTR(&unitKammy_weaponElectricMagic))
                CASE_EQUAL(3)
                    SET(LW(9), PTR(&unitKammy_weaponInvisMagic))
                CASE_EQUAL(4)
                    SET(LW(9), PTR(&unitKammy_weaponHugeMagic))
                CASE_EQUAL(5)
                    SET(LW(9), PTR(&unitKammy_weaponFastMagic))
                CASE_ETC()
                    SET(LW(9), PTR(&unitKammy_weaponHealingMagic))
                    RUN_CHILD_EVT(PTR(&unitKammy_recover_magic_event))
                    GOTO(99)
            END_SWITCH()
            
            RUN_CHILD_EVT(PTR(&unitKammy_status_magic_event))
            GOTO(99)

            CASE_END()
        CASE_ETC()
            USER_FUNC(btlevtcmd_AddUnitWork, -2, UW_Kammy_AiState, 1)
            SET(LW(9), PTR(&unitKammy_weaponBlast))
            RUN_CHILD_EVT(PTR(&unitKammy_blast_magic_event))
            GOTO(99)
    END_SWITCH()
    
LBL(99)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitKammy_phase_event)
    // Advance to phase 2 AI when at less than half health.
    USER_FUNC(btlevtcmd_CheckPhase, LW(0), 0x4000003)
    IF_EQUAL(LW(0), 1)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Kammy_AiState, LW(0))
        IF_LARGE_EQUAL(LW(0), (int32_t)KammyAiState::PHASE_2_ATTACK)
            GOTO(10)
        END_IF()
        USER_FUNC(btlevtcmd_GetHp, -2, LW(1))
        USER_FUNC(btlevtcmd_GetMaxHp, -2, LW(2))
        MUL(LW(1), 100)
        DIV(LW(1), LW(2))
        IF_LARGE_EQUAL(LW(1), 50)
            GOTO(10)
        END_IF()

        // TODO: Maybe add dialogue.

        USER_FUNC(btlevtcmd_AddUnitWork, -2, UW_Kammy_AiState, (int32_t)KammyAiState::PHASE_2_ATTACK)
    END_IF()

LBL(10)
    USER_FUNC(btlevtcmd_CheckPhase, LW(0), 0x4000001)
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
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Kammy_IsFlying, LW(0))
    IF_EQUAL(LW(0), 1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_AddUnitWork, -2, UW_Kammy_TurnsGrounded, 1)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Kammy_TurnsGrounded, LW(0))
    IF_SMALL(LW(0), 3)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, 1)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(5.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, 300, LW(1), LW(2), 0, -1, 0)
    WAIT_FRM(60)
    USER_FUNC(btlevtcmd_OnAttribute, -2, 4)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 1, 2097152)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 1, 4194304)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    WAIT_MSEC(500)
    USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitKammy_pose_table_flying))
    USER_FUNC(btlevtcmd_SetTalkPoseType, -2, 65)
    USER_FUNC(btlevtcmd_SetStayPoseType, -2, 43)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    BROTHER_EVT_ID(LW(15))
        DO(0)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_KAMEBABA_MOVE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_MSEC(166)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, -1)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetPos, -2, LW(0), 40, LW(2))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), 40, LW(2), 60, 0, 4, 0, -1)
    USER_FUNC(btlevtcmd_SetHomePos, -2, LW(0), 40, LW(2))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 69)
    DELETE_EVT(LW(15))
    WAIT_MSEC(500)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Kammy_IsFlying, 1)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Kammy_DamageTaken, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Kammy_TurnsGrounded, 0)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitKammy_damage_fall_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_OffAttribute, -2, 4)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, 2097152)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, 4194304)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("D_2"))
    WAIT_MSEC(500)
    USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitKammy_pose_table))
    USER_FUNC(btlevtcmd_SetTalkPoseType, -2, 65)
    USER_FUNC(btlevtcmd_SetStayPoseType, -2, 43)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 39)
    INLINE_EVT()
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_SetPartsPos, -2, 2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 2, PTR("H_1"))
        USER_FUNC(btlevtcmd_SetAlpha, -2, 2, 255)
        USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 2, 16777216)
        WAIT_FRM(80)
        USER_FUNC(evt_sub_intpl_init, 0, 255, 0, 30)
        DO(30)
            USER_FUNC(evt_sub_intpl_get_value_para, LW(1), LW(2))
            USER_FUNC(btlevtcmd_SetAlpha, -2, 2, LW(1))
            WAIT_FRM(1)
        WHILE()
        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 2, 16777216)
    END_INLINE()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_FALL1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(1.0))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 10, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_KAMEBABA_DOWN1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetHomePos, -2, LW(0), 0, LW(2))
    WAIT_MSEC(500)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Kammy_IsFlying, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitKammy_dead_event)
    SET(GW(11), 1)

    // Check for GW(10) and GW(11) being set to 1 both at once.
    IF_EQUAL(GW(10), 1)
        USER_FUNC(evtTot_MarkCompletedAchievement,
            (int32_t)AchievementId::V3_RUN_BOSS_SIMULTANEOUS)
    END_IF()

    WAIT_FRM(1)
    USER_FUNC(btlevtcmd_WaitAttackEnd)
    USER_FUNC(btlevtcmd_GetUnitWork, LW(10), UW_Kammy_IsFlying, LW(0))
    IF_EQUAL(LW(0), 1)
        RUN_CHILD_EVT(PTR(&unitKammy_damage_fall_event))
    END_IF()
    SWITCH(GW(10))
        CASE_EQUAL(0)
            USER_FUNC(btlevtcmd_StatusWindowOnOff, 0)
            RUN_CHILD_EVT(PTR(&unitKammy_camera_focus_event))
            USER_FUNC(btlevtcmd_SetTalkPose, -2, PTR("T_3"))
            USER_FUNC(btlevtcmd_SetStayPose, -2, PTR("D_3"))

            USER_FUNC(evtTot_SetConversation, (int32_t)ConversationId::BOSS_2B_DEATH_A)
            USER_FUNC(evtTot_GetNextMessage, LW(0), LW(1))
            USER_FUNC(evt_msg_print, 2, LW(0), 0, -2)

            WAIT_MSEC(200)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("D_3"))
            USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitKammy_pose_table_dead))
            SET(GW(11), 2)
            SET(LW(10), -2)
            RUN_CHILD_EVT(PTR(&subsetevt_dead_core_nospin_norotate))
            USER_FUNC(btlevtcmd_StatusWindowOnOff, 1)
        CASE_EQUAL(1)
            DO(0)
                IF_EQUAL(GW(10), 2)
                    DO_BREAK()
                END_IF()
                WAIT_FRM(1)
            WHILE()
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("D_3"))
            USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitKammy_pose_table_dead))
            SET(GW(11), 2)
            SET(LW(10), -2)
            RUN_CHILD_EVT(PTR(&subsetevt_dead_core_nospin_norotate))
        CASE_EQUAL(2)
            USER_FUNC(btlevtcmd_StatusWindowOnOff, 0)
            RUN_CHILD_EVT(PTR(&unitKammy_camera_focus_event))
            USER_FUNC(btlevtcmd_SetTalkPose, -2, PTR("D_1"))
            USER_FUNC(btlevtcmd_SetStayPose, -2, PTR("D_1"))

            USER_FUNC(evtTot_SetConversation, (int32_t)ConversationId::BOSS_2B_DEATH_B)
            USER_FUNC(evtTot_GetNextMessage, LW(0), LW(1))
            USER_FUNC(evt_msg_print, 2, LW(0), 0, -2)

            WAIT_MSEC(200)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("D_3"))
            USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitKammy_pose_table_dead))
            SET(GW(11), 2)
            SET(LW(10), -2)
            RUN_CHILD_EVT(PTR(&subsetevt_dead_core_nospin_norotate))
            USER_FUNC(btlevtcmd_StatusWindowOnOff, 1)
    END_SWITCH()
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitKammy_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    USER_FUNC(btlevtcmd_CheckStatus, -2, 27, LW(0))
    IF_EQUAL(LW(0), 1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_CheckDamageCode, -2, 256, LW(0))
    IF_EQUAL(LW(0), 0)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_GetUnitWork, LW(10), UW_Kammy_IsFlying, LW(0))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(btlevtcmd_GetTotalDamage, LW(10), LW(1))
        USER_FUNC(btlevtcmd_GetUnitWork, LW(10), UW_Kammy_DamageTaken, LW(0))
        ADD(LW(0), LW(1))
        USER_FUNC(btlevtcmd_SetUnitWork, LW(10), UW_Kammy_DamageTaken, LW(0))
        // Require a total of 10 damage to be dealt to ground Kammy.
        IF_LARGE_EQUAL(LW(0), 10)
            USER_FUNC(btlevtcmd_WaitAttackEnd)
            USER_FUNC(btlevtcmd_StopWaitEvent, -2)
            RUN_CHILD_EVT(PTR(&unitKammy_damage_fall_event))
            USER_FUNC(btlevtcmd_StartWaitEvent, -2)
        END_IF()
    END_IF()
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitKammy_wait_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitKammy_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitKammy_wait_event))
    USER_FUNC(btlevtcmd_SetEventPhase, -2, PTR(&unitKammy_phase_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitKammy_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitKammy_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&btldefaultevt_Confuse))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Kammy_IsFlying, 1)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Kammy_DamageTaken, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Kammy_TurnsGrounded, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Kammy_NumHeals, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Kammy_AiState, (int32_t)KammyAiState::PHASE_1_ATTACK)
    USER_FUNC(btlevtcmd_SetWalkSound, -2, PTR("SFX_BOSS_KAMEBABA_MOVE1L"), PTR("SFX_BOSS_KAMEBABA_MOVE1R"), 0, 15, 15)
    USER_FUNC(btlevtcmd_SetRunSound, -2, PTR("SFX_BOSS_KAMEBABA_MOVE1L"), PTR("SFX_BOSS_KAMEBABA_MOVE1R"), 0, 8, 8)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

DataTableEntry unitKammy_data_table[] = {
    49, (void*)unitKammy_dead_event,
    0, nullptr,
};

BattleUnitKind unit_Kammy = {
    .unit_type = BattleUnitType::KAMMY_KOOPA,
    .unit_name = "btl_un_kamec_obaba",
    .max_hp = 50,
    .max_fp = 0,
    .danger_hp = 1,
    .peril_hp = 1,
    .level = 48,
    .bonus_exp = 0,
    .bonus_coin = 0,
    .bonus_coin_rate = 100,
    .base_coin = 0,
    .run_rate = 50,
    .pb_soft_cap = 7,
    .width = 42,
    .height = 42,
    .hit_offset = { 7, 42 },
    .center_offset = { 0.0f, 21.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 20.0f, 20.0f, 0.0f },
    .held_item_base_offset = { 21.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 18.0f, 0.0f, 0.0f },
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
    .damage_sfx_name = "SFX_BOSS_KAMEBABA_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::UNQUAKEABLE,
    .status_vulnerability = &unitKammy_status,
    .num_parts = 2,
    .parts = unitKammy_parts,
    .init_evt_code = (void*)unitKammy_init_event,
    .data_table = unitKammy_data_table,
};

const BattleUnitSetup unitKammy_battle_setup = {
    .unit_kind_params = &unit_Kammy,
    .alliance = 1,
    .attack_phase = 0x04000004,
    .position = { 1000.0f, 0.0f, 1000.0f },
    .addl_target_offset_x = 0,
    .unit_work = { 0, 0, 0, 0 },
    .item_drop_table = nullptr,
};

///////////////////////////////////////

int8_t unitBowser_defense[] = { 2, 2, 2, 2, 2 };
int8_t unitBowser_defense_attr[] = { 0, 0, 0, 0, 0 };

StatusVulnerability unitBowser_status = {
     30,  30,  30, 100,  40, 100, 100,  25,
    100,  70, 100,  70, 100,  80,  30,   0,
      0, 100,  30, 100, 100,   0,
};

PoseTableEntry unitBowser_pose_table[] = {
    1, "KPA_N_3",
    2, "KPA_Z_6",
    9, "KPA_Z_6",
    5, "KPA_K_2",
    4, "KPA_S_2",
    3, "KPA_S_2",
    28, "KPA_S_1",
    29, "KPA_Q_2",
    30, "KPA_Q_2",
    31, "KPA_D_1",
    39, "KPA_D_1",
    50, "KPA_S_1",
    42, "KPA_R_1",
    40, "KPA_W_1",
    56, "KPA_S_1",
    57, "KPA_S_1",
    65, "KPA_T_1",
    69, "KPA_S_1",
};

PoseTableEntry unitBowser_pose_table_leaning[] = {
    1, "KPA_N_2",
    2, "KPA_Z_5",
    9, "KPA_Z_5",
    5, "KPA_K_1",
    4, "KPA_X_1",
    3, "KPA_X_1",
    28, "KPA_S_8",
    29, "KPA_D_4",
    30, "KPA_D_4",
    31, "KPA_D_4",
    39, "KPA_D_4",
    50, "KPA_S_8",
    42, "KPA_R_1",
    40, "KPA_W_1",
    56, "KPA_S_8",
    57, "KPA_S_8",
    65, "KPA_T_9",
    69, "KPA_S_8",
};

PoseTableEntry unitBowser_pose_table_dead[] = {
    28, "KPA_Z_3",
    39, "KPA_Z_3",
    69, "KPA_Z_3",
};

BattleWeapon unitBowser_weaponStomp = {
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
    .damage_function_params = { 8, 0, 0, 0, 0, 0, 0, 0 },
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
        AttackTargetProperty_Flags::JUMPLIKE |
        AttackTargetProperty_Flags::CANNOT_TARGET_CEILING,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0xa,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::FLIPS_SHELLED |
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags =
        AttackCounterResistance_Flags::FRONT_SPIKY |
        AttackCounterResistance_Flags::PREEMPTIVE_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM,
        
    // status chances
    
    .attack_evt_code = nullptr,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 10,
    .bg_a2_fall_weight = 10,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 10,
    .nozzle_turn_chance = 0,
    .nozzle_fire_chance = 0,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 10,
};

BattleWeapon unitBowser_weaponBite = {
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
    .damage_function_params = { 8, 0, 0, 0, 0, 0, 0, 0 },
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
        AttackTargetProperty_Flags::HAMMERLIKE,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags =
        AttackCounterResistance_Flags::TOP_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    .poison_chance = 50,
    .poison_time = 10,
    .poison_strength = 1,
    
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

BattleWeapon unitBowser_weaponCriticalBite = {
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
    .damage_function_params = { 8, 0, 0, 0, 0, 0, 0, 0 },
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
        AttackTargetProperty_Flags::HAMMERLIKE,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::TOT_CRITICAL_HIT,
    .counter_resistance_flags =
        AttackCounterResistance_Flags::TOP_SPIKY,
    .target_weighting_flags =
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

BattleWeapon unitBowser_weaponBreath = {
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
    .damage_function_params = { 8, 0, 0, 0, 0, 0, 0, 0 },
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
    .element = AttackElement::FIRE,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::DEFENSE_PIERCING |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = 0,
        
    // status chances
    .burn_chance = 80,
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

BattleUnitKindPart unitBowser_parts = {
    .index = 1,
    .name = "btl_un_koopa",
    .model_name = "c_koopa",
    .part_offset_pos = { 0.0f, 0.0f, 0.0f },
    .part_hit_base_offset = { 34.0f, 62.0f, 0.0f },
    .part_hit_cursor_base_offset = { 22.0f, 78.0f, 0.0f },
    .unk_30 = 70,
    .unk_32 = 80,
    .base_alpha = 255,
    .defense = unitBowser_defense,
    .defense_attr = unitBowser_defense_attr,
    .attribute_flags = 0x0000'0009,
    .counter_attribute_flags = 0,
    .pose_table = unitBowser_pose_table,
};

EVT_BEGIN(unitBowser_camera_focus_event)
    USER_FUNC(evt_btl_camera_set_mode, 0, 3)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(evt_btl_camera_set_moveto, 0, LW(0), 95, 350, LW(0), 45, -300, 30, 11)
    WAIT_FRM(37)
    RETURN()
EVT_END()

EVT_BEGIN(unitBowser_first_damage_event)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Bowser_HasTaunted, LW(0))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Bowser_HasTaunted, 2)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_S_1"))
        USER_FUNC(btlevtcmd_StatusWindowOnOff, 0)
        RUN_CHILD_EVT(PTR(&unitBowser_camera_focus_event))
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Bowser_IsLeaning, LW(0))
        IF_EQUAL(LW(0), 1)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_U_3"))
            WAIT_MSEC(200)
        END_IF()
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_VOICE_KOOPA_LAUGH2_1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_Y_2"))
        WAIT_MSEC(1000)
        USER_FUNC(btlevtcmd_SetTalkPose, -2, PTR("KPA_T_2"))
        USER_FUNC(btlevtcmd_SetStayPose, -2, PTR("KPA_S_3"))

        USER_FUNC(evtTot_SetConversation, (int32_t)ConversationId::BOSS_2A_TAUNT)
        USER_FUNC(evtTot_GetNextMessage, LW(0), LW(1))
        USER_FUNC(evt_msg_print, 2, LW(0), 0, -2)
        
        WAIT_MSEC(300)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_S_1"))
        IF_EQUAL(GW(11), 0)
            RUN_CHILD_EVT(PTR(&unitKammy_camera_focus_event))

            USER_FUNC(evtTot_SetConversation, (int32_t)ConversationId::BOSS_2B_TAUNT)
            USER_FUNC(evtTot_GetNextMessage, LW(0), LW(1))
            USER_FUNC(evt_msg_print, 2, LW(0), 0, GW(9))

            WAIT_MSEC(300)
        END_IF()
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Bowser_IsLeaning, LW(0))
        IF_EQUAL(LW(0), 1)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_U_2"))
            WAIT_MSEC(200)
        END_IF()
        USER_FUNC(btlevtcmd_StatusWindowOnOff, 1)
        USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitBowser_first_damage_check_event)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Bowser_HasTaunted, LW(0))
    IF_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Bowser_Target, LW(0))
        IF_EQUAL(LW(0), (int32_t)BattleUnitType::MARIO)
            USER_FUNC(btlevtcmd_GetTotalDamage, LW(3), LW(1))
            IF_NOT_EQUAL(LW(1), 0)
                USER_FUNC(btlevtcmd_SetUnitWork, -2, 1, 1)
            END_IF()
        END_IF()
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitBowser_pound_attack_event)
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(9))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(9), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
        END_IF()
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_GetUnitKind, LW(3), LW(0))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Bowser_Target, LW(0))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 40)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(2.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_KOOPA_JUMP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_A_2A"))
    WAIT_MSEC(300)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_A_2B"))
    BROTHER_EVT()
        WAIT_FRM(25)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_A_2C"))
        WAIT_FRM(15)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_A_2D"))
    END_BROTHER()
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.25))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    SUB(LW(2), 20)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 50, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_KOOPA_HIP_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
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
    USER_FUNC(btlevtcmd_JumpContinue, -2)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_A_2E"))
    WAIT_MSEC(500)
    GOTO(98)
LBL(91)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    USER_FUNC(evt_btl_camera_shake_h, 0, 3, 0, 50, 13)
    USER_FUNC(btlevtcmd_JumpContinue, -2)
    WAIT_MSEC(500)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_A_2E"))
    WAIT_MSEC(300)
    USER_FUNC(btlevtcmd_GetResultACDefence, LW(7))
    IF_SMALL_EQUAL(LW(7), 3)
        SET(LW(15), 0)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Bowser_Target, LW(0))
        SWITCH(LW(0))
            CASE_EQUAL(222)
                USER_FUNC(evtTot_Bowser_MarioCommandDisabled, LW(15))
            CASE_BETWEEN(224, 230)
                USER_FUNC(evtTot_Bowser_PartyCommandDisabled, LW(15), LW(3))
            CASE_ETC()
                SET(LW(0), -1)
        END_SWITCH()
        IF_NOT_EQUAL(LW(0), -1)
            IF_NOT_EQUAL(LW(15), 0)
                INLINE_EVT()
                    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
                    USER_FUNC(btlevtcmd_GetPos, -2, LW(5), LW(6), LW(7))
                    USER_FUNC(btlevtcmd_FaceDirectionSub, LW(3), LW(5), 60)
                    USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(5), LW(6), LW(7))
                    USER_FUNC(evtTot_Bowser_CommandLossIconBounce, LW(15), LW(0), LW(1), LW(2), LW(5), LW(6), LW(7), FLOAT(0.5), 30)
                    SET(LW(0), LW(5))
                    SET(LW(1), LW(6))
                    SET(LW(2), LW(7))
                    USER_FUNC(btlevtcmd_FaceDirectionSub, LW(3), LW(5), 40)
                    USER_FUNC(evtTot_Bowser_CommandLossIconBounce, LW(15), LW(0), LW(1), LW(2), LW(5), LW(6), LW(7), FLOAT(0.5), 20)
                    SET(LW(0), LW(5))
                    SET(LW(1), LW(6))
                    SET(LW(2), LW(7))
                    USER_FUNC(btlevtcmd_FaceDirectionSub, LW(3), LW(5), 25)
                    USER_FUNC(evtTot_Bowser_CommandLossIconBounce, LW(15), LW(0), LW(1), LW(2), LW(5), LW(6), LW(7), FLOAT(0.5), 12)
                END_INLINE()
            END_IF()
            BROTHER_EVT()
                USER_FUNC(btlevtcmd_AnnounceMessage, 0, 0, 0, LW(0), 120)
            END_BROTHER()
        END_IF()
    END_IF()
LBL(98)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    WAIT_MSEC(250)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_R_1"))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(4.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_S_1"))
    RUN_CHILD_EVT(PTR(&unitBowser_first_damage_check_event))
    RETURN()
EVT_END()

EVT_BEGIN(unitBowser_breath_attack_event)
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(9))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(9), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
        END_IF()
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_GetUnitKind, LW(3), LW(0))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Bowser_Target, LW(0))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_R_1"))
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), 120)
    USER_FUNC(evt_btl_camera_set_mode, 0, 8)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, LW(3))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(0), LW(2), 1, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(4.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), 0, LW(2), 0, -1, 1)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    WAIT_MSEC(170)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_A_1A"))
    WAIT_MSEC(300)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_VOICE_KOOPA_FIRE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_KOOPA_FIRE_BREATH1"), EVT_NULLPTR, 0, LW(15))
    INLINE_EVT()
        WAIT_MSEC(1500)
        USER_FUNC(evt_snd_sfxoff, LW(15))
    END_INLINE()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_A_1B"))
    WAIT_MSEC(300)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(5))
    SETF(LW(6), FLOAT(60.0))
    MULF(LW(6), LW(5))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), LW(6))
    SETF(LW(6), FLOAT(22.5))
    MULF(LW(6), LW(5))
    ADDF(LW(1), LW(6))
    SUB(LW(2), 5)
    MULF(LW(5), FLOAT(3.5))
    USER_FUNC(evt_eff, PTR(""), PTR("gonbaba_breath"), 7, LW(0), LW(1), LW(2), LW(5), 90, 0, 0, 0, 0, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_A_1C"))
    WAIT_MSEC(300)
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    SET(LW(10), 0)
LBL(10)
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
    GOTO(97)
LBL(91)
    IF_EQUAL(LW(10), 0)
        USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
        ADD(LW(10), 1)
    END_IF()
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
LBL(97)
    RUN_CHILD_EVT(PTR(&unitBowser_first_damage_check_event))
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    IF_NOT_EQUAL(LW(3), -1)
        GOTO(10)
    END_IF()
LBL(98)
    WAIT_MSEC(1200)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_A_1D"))
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    WAIT_MSEC(250)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_R_1"))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(4.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitBowser_bite_attack_event)
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(9))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(9), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
        END_IF()
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_GetUnitKind, LW(3), LW(0))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Bowser_Target, LW(0))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(evt_btl_camera_set_mode, 0, 8)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, LW(3))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_R_1"))
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), 65)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(0), LW(2), 1, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(4.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), 0, LW(2), 0, -1, 1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_A_3A"))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_KOOPA_BITE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    WAIT_MSEC(170)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_A_3B"))
    WAIT_MSEC(150)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_KOOPA_BITE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
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
    WAIT_MSEC(100)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_A_3C"))
    WAIT_MSEC(150)
    GOTO(98)
LBL(91)
    WAIT_MSEC(100)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_A_3C"))
    WAIT_MSEC(150)
LBL(98)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    WAIT_MSEC(250)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_R_1"))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(4.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_S_1"))
    RUN_CHILD_EVT(PTR(&unitBowser_first_damage_check_event))
    RETURN()
EVT_END()

EVT_BEGIN(unitBowser_critical_bite_attack_event)
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(9))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(9), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
        END_IF()
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_GetUnitKind, LW(3), LW(0))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Bowser_Target, LW(0))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_R_1"))
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), 65)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(0), LW(2), 1, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(4.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), 0, LW(2), 0, -1, 1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_A_3A"))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_KOOPA_BITE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))

    // Improve sound effect syncing with animation.
    BROTHER_EVT_ID(LW(15))
        WAIT_FRM(15)
        DO(8)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_KOOPA_BITE3"), EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_FRM(12)
        WHILE()
    END_BROTHER()

    WAIT_MSEC(170)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_A_3B"))
    WAIT_MSEC(150)
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
    DELETE_EVT(LW(15))
    SET(LW(15), -1)
    WAIT_MSEC(100)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_A_3C"))
    WAIT_MSEC(150)
    GOTO(98)
LBL(91)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_A_4"))
    WAIT_MSEC(1500)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_A_3C"))
    WAIT_MSEC(150)
    SET(LW(15), -1)
LBL(98)
    IF_NOT_EQUAL(LW(15), -1)
        DELETE_EVT(LW(15))
    END_IF()
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    WAIT_MSEC(250)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_R_1"))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(4.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    RUN_CHILD_EVT(PTR(&unitBowser_first_damage_check_event))
    RETURN()
EVT_END()

EVT_BEGIN(unitBowser_check_target_hp)
    SET(LW(9), PTR(&unitBowser_weaponCriticalBite))
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(9))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(9), LW(3), LW(4))

    SET(LW(1), 0)
    
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()

    // Verify that the target is Mario or a partner directly.
    USER_FUNC(btlevtcmd_GetUnitKind, LW(3), LW(0))
    IF_SMALL(LW(0), (int32_t)BattleUnitType::MARIO)
        GOTO(99)
    END_IF()
    IF_EQUAL(LW(0), (int32_t)BattleUnitType::SHELL_SHIELD)
        GOTO(99)
    END_IF()
    
    // If target has 2+ max HP and at least 50% of max currently, set
    // weight of critical bite to be non-zero.
    USER_FUNC(btlevtcmd_GetHp, LW(3), LW(11))
    USER_FUNC(btlevtcmd_GetMaxHp, LW(3), LW(12))
    MUL(LW(11), 100)
    DIV(LW(11), LW(12))

    IF_LARGE_EQUAL(LW(11), 50)
        IF_LARGE(LW(12), 1)
            SET(LW(1), 20)
        END_IF()
    END_IF()

LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitBowser_attack_event)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Bowser_AiState, LW(0))
    SWITCH(LW(0))
        CASE_EQUAL(0)
            USER_FUNC(btlevtcmd_AddUnitWork, -2, UW_Bowser_AiState, 1)
            SET(LW(9), PTR(&unitBowser_weaponBreath))
            RUN_CHILD_EVT(PTR(&unitBowser_breath_attack_event))
        CASE_EQUAL(1)
            USER_FUNC(btlevtcmd_AddUnitWork, -2, UW_Bowser_AiState, 1)
            SET(LW(9), PTR(&unitBowser_weaponBite))
            RUN_CHILD_EVT(PTR(&unitBowser_bite_attack_event))
        CASE_EQUAL(2)
            USER_FUNC(btlevtcmd_AddUnitWork, -2, UW_Bowser_AiState, 1)
            SET(LW(9), PTR(&unitBowser_weaponStomp))
            RUN_CHILD_EVT(PTR(&unitBowser_pound_attack_event))
        CASE_EQUAL(3)
            // When Bowser reaches low HP, force critical bite if possible.
            // If not there yet, or target is too low on HP, use random move.
            USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Bowser_LowHpEvent, LW(1))
            IF_EQUAL(LW(1), 0)
                GOTO(50)
            END_IF()
            RUN_CHILD_EVT(PTR(&unitBowser_check_target_hp))
            IF_EQUAL(LW(1), 0)
                GOTO(50)
            END_IF()

            USER_FUNC(btlevtcmd_AddUnitWork, -2, UW_Bowser_AiState, 1)
            SET(LW(9), PTR(&unitBowser_weaponCriticalBite))
            RUN_CHILD_EVT(PTR(&unitBowser_critical_bite_attack_event))
        
        CASE_ETC()
            RUN_CHILD_EVT(PTR(&unitBowser_check_target_hp))
LBL(50)
            USER_FUNC(btlevtcmd_DrawLots, LW(0), 4, 30, 20, 20, LW(1))
            SWITCH(LW(0))
                CASE_EQUAL(0)
                    SET(LW(9), PTR(&unitBowser_weaponBreath))
                    RUN_CHILD_EVT(PTR(&unitBowser_breath_attack_event))
                CASE_EQUAL(1)
                    SET(LW(9), PTR(&unitBowser_weaponBite))
                    RUN_CHILD_EVT(PTR(&unitBowser_bite_attack_event))
                CASE_EQUAL(2)
                    SET(LW(9), PTR(&unitBowser_weaponStomp))
                    RUN_CHILD_EVT(PTR(&unitBowser_pound_attack_event))
                CASE_EQUAL(3)
                    SET(LW(9), PTR(&unitBowser_weaponCriticalBite))
                    RUN_CHILD_EVT(PTR(&unitBowser_critical_bite_attack_event))
            END_SWITCH()
    END_SWITCH()
LBL(90)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RUN_CHILD_EVT(PTR(&unitBowser_first_damage_event))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitBowser_half_hp_event)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Bowser_LowHpEvent, LW(1))
    IF_EQUAL(LW(1), 0)
        USER_FUNC(btlevtcmd_GetHp, -2, LW(0))
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Bowser_HpThreshold, LW(1))
        IF_SMALL(LW(0), LW(1))
            USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Bowser_LowHpEvent, 1)
            USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Bowser_IsLeaning, LW(0))
            IF_EQUAL(LW(0), 1)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_U_3"))
                WAIT_MSEC(200)
            END_IF()
            USER_FUNC(btlevtcmd_StatusWindowOnOff, 0)
            RUN_CHILD_EVT(PTR(&unitBowser_camera_focus_event))
            USER_FUNC(btlevtcmd_SetTalkPose, -2, PTR("KPA_T_3"))
            USER_FUNC(btlevtcmd_SetStayPose, -2, PTR("KPA_S_4"))

            USER_FUNC(evtTot_SetConversation, (int32_t)ConversationId::BOSS_2A_P2)
            USER_FUNC(evtTot_GetNextMessage, LW(0), LW(1))
            USER_FUNC(evt_msg_print, 2, LW(0), 0, -2)
            
            WAIT_MSEC(300)
            USER_FUNC(btlevtcmd_StatusWindowOnOff, 1)
            USER_FUNC(evt_btl_camera_set_mode, 0, 0)
            USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Bowser_IsLeaning, LW(0))
            IF_EQUAL(LW(0), 1)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_U_2"))
                WAIT_MSEC(200)
            END_IF()
            USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 43)
        END_IF()
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitBowser_phase_event)
    USER_FUNC(btlevtcmd_CheckStatus, -2, 27, LW(0))
    IF_EQUAL(LW(0), 1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_get_turn, LW(0))
    IF_EQUAL(LW(0), 1)
        GOTO(90)
    END_IF()
    USER_FUNC(btlevtcmd_CheckPhase, LW(0), 0x4000001)
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_CheckActStatus, -2, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            USER_FUNC(btlevtcmd_PhaseEventStartDeclare, -2)
            USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Bowser_IsLeaning, LW(0))
            IF_EQUAL(LW(0), 0)
                // TODO: Incorporate spiky status into AI if Kammy is KO'd?
                USER_FUNC(evt_sub_random, 99, LW(1))
                IF_SMALL(LW(1), 0)
                    USER_FUNC(btlevtcmd_SetUnitWork, -2, 0, 1)
                    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_U_2"))
                    WAIT_MSEC(200)
                    USER_FUNC(btlevtcmd_OnPartsCounterAttribute, -2, 1, 1)
                    USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitBowser_pose_table_leaning))
                END_IF()
            ELSE()
                USER_FUNC(evt_sub_random, 99, LW(1))
                IF_SMALL(LW(1), 100)
                    USER_FUNC(btlevtcmd_SetUnitWork, -2, 0, 0)
                    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_U_3"))
                    WAIT_MSEC(200)
                    USER_FUNC(btlevtcmd_OffPartsCounterAttribute, -2, 1, 1)
                    USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitBowser_pose_table))
                END_IF()
            END_IF()
            USER_FUNC(btlevtcmd_StartWaitEvent, -2)
        END_IF()
    END_IF()
    LBL(90)
    USER_FUNC(btlevtcmd_CheckPhase, LW(0), 0x4000003)
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_PhaseEventStartDeclare, -2)
        RUN_CHILD_EVT(PTR(&unitBowser_half_hp_event))
        USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    END_IF()
    LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitBowser_dead_event)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(3), LW(4), LW(5))
    SET(LW(6), 1)
    IF_EQUAL(LW(0), LW(3))
        IF_EQUAL(LW(1), LW(4))
            IF_EQUAL(LW(2), LW(5))
                SET(LW(6), 0)
            END_IF()
        END_IF()
    END_IF()
    IF_EQUAL(LW(6), 1)
        USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(6.0))
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
        USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 43)
        USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    END_IF()
    WAIT_FRM(1)
    USER_FUNC(btlevtcmd_WaitAttackEnd)
    USER_FUNC(btlevtcmd_WaitAttackEnd)
    USER_FUNC(btlevtcmd_StatusWindowOnOff, 0)
    RUN_CHILD_EVT(PTR(&unitBowser_camera_focus_event))
    SWITCH(GW(11))
        CASE_EQUAL(0)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_VOICE_KOOPA_SATIATED2_2"), EVT_NULLPTR, 0, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_D_2"))
            WAIT_MSEC(200)
            USER_FUNC(btlevtcmd_SetTalkPose, -2, PTR("KPA_T_8"))
            USER_FUNC(btlevtcmd_SetStayPose, -2, PTR("KPA_Z_3"))

            USER_FUNC(evtTot_SetConversation, (int32_t)ConversationId::BOSS_2A_DEATH_A)
            USER_FUNC(evtTot_GetNextMessage, LW(0), LW(1))
            USER_FUNC(evt_msg_print, 2, LW(0), 0, -2)

            WAIT_MSEC(300)
            USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitBowser_pose_table_dead))
            SET(GW(10), 2)
            SET(LW(10), -2)
            RUN_CHILD_EVT(PTR(&subsetevt_dead_core_nospin_norotate))
        CASE_ETC()
            USER_FUNC(btlevtcmd_SetTalkPose, -2, PTR("KPA_D_1"))
            USER_FUNC(btlevtcmd_SetStayPose, -2, PTR("KPA_D_1"))

            USER_FUNC(evtTot_SetConversation, (int32_t)ConversationId::BOSS_2A_DEATH_B)
            USER_FUNC(evtTot_GetNextMessage, LW(0), LW(1))
            USER_FUNC(evt_msg_print, 2, LW(0), 0, -2)

            WAIT_MSEC(300)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_VOICE_KOOPA_SATIATED2_2"), EVT_NULLPTR, 0, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_D_2"))
            WAIT_MSEC(700)
            USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitBowser_pose_table_dead))
            SET(GW(10), 2)
            SET(LW(10), -2)
            RUN_CHILD_EVT(PTR(&subsetevt_dead_core_nospin_norotate))
    END_SWITCH()
    USER_FUNC(btlevtcmd_StatusWindowOnOff, 1)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitBowser_damage_event)
    USER_FUNC(btlevtcmd_CheckDamageCode, -2, 1024, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        SET(GW(10), 1)

        // Check for GW(10) and GW(11) being set to 1 both at once.
        IF_EQUAL(GW(11), 1)
            USER_FUNC(evtTot_MarkCompletedAchievement,
                (int32_t)AchievementId::V3_RUN_BOSS_SIMULTANEOUS)
        END_IF()
    END_IF()
    SET(LW(10), -2)
    SET(LW(11), 1)
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    RETURN()
EVT_END()

EVT_BEGIN(unitBowser_battle_entry_event)
    USER_FUNC(btlevtcmd_StatusWindowOnOff, 0)
    WAIT_FRM(60)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_S_1"))
    WAIT_FRM(60)
    RUN_CHILD_EVT(PTR(&unitBowser_camera_focus_event))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_VOICE_KOOPA_LAUGH2_1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("KPA_Y_2"))
    WAIT_MSEC(1000)

    USER_FUNC(evtTot_SetConversation, (int32_t)ConversationId::BOSS_2A_INTRO)
    USER_FUNC(evtTot_GetNextMessage, LW(0), LW(1))
    USER_FUNC(evt_msg_print, 2, LW(0), 0, -2)
    WAIT_MSEC(300)

    RUN_CHILD_EVT(PTR(&unitKammy_camera_focus_event))
    USER_FUNC(evtTot_SetConversation, (int32_t)ConversationId::BOSS_2B_INTRO)
    USER_FUNC(evtTot_GetNextMessage, LW(0), LW(1))
    USER_FUNC(evt_msg_print, 2, LW(0), 0, GW(9))
    WAIT_MSEC(300)

    USER_FUNC(btlevtcmd_StatusWindowOnOff, 1)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitBowser_wait_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitBowser_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitBowser_wait_event))
    USER_FUNC(btlevtcmd_SetEventPhase, -2, PTR(&unitBowser_phase_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitBowser_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitBowser_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&btldefaultevt_Confuse))
    USER_FUNC(btlevtcmd_SetEventEntry, -2, PTR(&unitBowser_battle_entry_event))
    // Death states for Bowser and Kammy.
    SET(GW(10), 0)
    SET(GW(11), 0)
    // Unit ids for Bowser and Kammy.
    USER_FUNC(btlevtcmd_GetUnitId, -2, LW(0))
    SET(GW(8), LW(0))
    USER_FUNC(btlevtcmd_SpawnUnit, GW(9), PTR(&unitKammy_battle_setup), 0)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 60)
    ADD(LW(1), 40)
    ADD(LW(2), 10)
    USER_FUNC(btlevtcmd_SetPos, GW(9), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetHomePos, GW(9), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Bowser_IsLeaning, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Bowser_HasTaunted, 0)
    USER_FUNC(btlevtcmd_GetMaxHp, -2, LW(0))
    DIV(LW(0), 2)
    
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Bowser_HpThreshold, LW(0))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Bowser_LowHpEvent, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Bowser_AiState, 0)

    USER_FUNC(btlevtcmd_SetWalkSound, -2, PTR("SFX_BOSS_KOOPA_MOVE1L"), PTR("SFX_BOSS_KOOPA_MOVE1R"), 0, 15, 15)
    USER_FUNC(btlevtcmd_SetRunSound, -2, PTR("SFX_BOSS_KOOPA_MOVE1L"), PTR("SFX_BOSS_KOOPA_MOVE1R"), 0, 8, 8)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

DataTableEntry unitBowser_data_table[] = {
    49, (void*)unitBowser_dead_event,
    0, nullptr,
};

BattleUnitKind unit_Bowser = {
    .unit_type = BattleUnitType::BOWSER_CH_8,
    .unit_name = "btl_un_koopa",
    .max_hp = 70,
    .max_fp = 0,
    .danger_hp = 1,
    .peril_hp = 1,
    .level = 68,
    .bonus_exp = 0,
    .bonus_coin = 0,
    .bonus_coin_rate = 100,
    .base_coin = 0,
    .run_rate = 50,
    .pb_soft_cap = 7,
    .width = 86,
    .height = 80,
    .hit_offset = { 0, 70 },
    .center_offset = { 0.0f, 40.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 40.0f, 40.0f, 0.0f },
    .held_item_base_offset = { 43.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 21.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 36.0f, 72.0f, 0.0f },
    .cut_base_offset = { 0.0f, 40.0f, 0.0f },
    .cut_width = 86.0f,
    .cut_height = 80.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_BOSS_KOOPA_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitBowser_status,
    .num_parts = 1,
    .parts = &unitBowser_parts,
    .init_evt_code = (void*)unitBowser_init_event,
    .data_table = unitBowser_data_table,
};

struct CommandLossIconAnimWork {
    gc::vec3 start;
    gc::vec3 cur;
    gc::vec3 end;
    float frames;
    float vel_xz;
    float accel_y;
    float vel_y;
};

void _DisplayCommandLossIcon(CameraId camera, void* user_data) {
    auto* evt = reinterpret_cast<EvtEntry*>(user_data);
    auto* work = *reinterpret_cast<CommandLossIconAnimWork**>(evt->userData);
    ttyd::icondrv::iconDispGx(
        1.0f, &work->cur, 0, *reinterpret_cast<uint32_t*>(&evt->userData[4]));
}

EVT_DEFINE_USER_FUNC(evtTot_Bowser_CommandLossIconBounce) {
    // Read parameters out every call for ease of access.
    int icon_id = evtGetValue(evt, evt->evtArguments[0]);
    gc::vec3 start_pos = {
        evtGetFloat(evt, evt->evtArguments[1]),
        evtGetFloat(evt, evt->evtArguments[2]),
        evtGetFloat(evt, evt->evtArguments[3])
    };
    gc::vec3 end_pos = {
        evtGetFloat(evt, evt->evtArguments[4]),
        evtGetFloat(evt, evt->evtArguments[5]),
        evtGetFloat(evt, evt->evtArguments[6])
    };
    float gravity = evtGetFloat(evt, evt->evtArguments[7]);
    float frames = evtGetValue(evt, evt->evtArguments[8]);
    
    if (isFirstCall) {
        auto* work = reinterpret_cast<CommandLossIconAnimWork*>(
            ttyd::battle::BattleAlloc(sizeof(CommandLossIconAnimWork)));
        *reinterpret_cast<void**>(&evt->userData[0]) = work;
        
        work->start.x = start_pos.x;
        work->start.y = start_pos.y;
        work->start.z = start_pos.z;
        work->cur.x = start_pos.x;
        work->cur.y = start_pos.y;
        work->cur.z = start_pos.z;
        work->end.x = end_pos.x;
        work->end.y = end_pos.y;
        work->end.z = end_pos.z;
        work->frames = frames;
        work->vel_xz = ttyd::system::distABf(
            work->start.x, work->start.z, work->end.x, work->end.z) / work->frames;
        work->accel_y = gravity;
        work->vel_y = 
            work->accel_y * work->frames * 0.5 + 
            (work->end.y - work->start.y) / work->frames;

        *reinterpret_cast<uint32_t*>(&evt->userData[4]) = icon_id;
        *reinterpret_cast<int32_t*>(&evt->userData[8]) = 0;
    }
    
    auto* work = *reinterpret_cast<CommandLossIconAnimWork**>(evt->userData);
    if (*reinterpret_cast<int32_t*>(&evt->userData[8]) != 0) {
        ttyd::battle::BattleFree(work);
        return 2;
    }
    
    work->cur.y += work->vel_y;
    work->vel_y -= work->accel_y;
    if (work->vel_y < 0.0f && work->cur.y < work->end.y) {
        work->cur.y = work->end.y;
    }
    
    float angle = ttyd::system::angleABf(
        work->start.x, work->start.z, work->end.x, work->end.z);
    ttyd::battle_sub::btlMovePos(work->vel_xz, angle, &work->cur.x, &work->cur.z);
    
    if (work->frames -= 1; work->frames < 1) {
        *reinterpret_cast<int32_t*>(&evt->userData[8]) = 1;
        work->cur.x = work->end.x;
        work->cur.y = work->end.y;
        work->cur.z = work->end.z;
    }
    
    ttyd::dispdrv::dispEntry(
        CameraId::k3d, 1, 900.0f, _DisplayCommandLossIcon, evt); 
        
    return 0;
}

EVT_DEFINE_USER_FUNC(evtTot_Bowser_MarioCommandDisabled) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    
    static const char* strings[] = {
        "btl_jump_cmd_disable",
        "btl_hammer_cmd_disable",
        "btl_item_cmd_disable",
    };
    
    int32_t r = ttyd::system::irand(3);
    int32_t icon_id;
    if (r == 0) {
        battleWork->mario_jump_disabled_turns = 3;
        icon_id = IconType::BOOT_ICON;
    } else if (r == 1) {
        battleWork->mario_hammer_disabled_turns = 3;
        icon_id = IconType::HAMMER_ICON;
    } else {
        battleWork->mario_items_disabled_turns = 3;
        icon_id = IconType::ITEM_ICON;
    }
    
    evt->lwData[0] = PTR(strings[r]);
    evtSetValue(evt, evt->evtArguments[0], icon_id);
    
    return 2;    
}

EVT_DEFINE_USER_FUNC(evtTot_Bowser_PartyCommandDisabled) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    int32_t id = evtGetValue(evt, evt->evtArguments[1]);
    id = ttyd::battle_sub::BattleTransID(evt, id);
    auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, id);
    
    static const char* strings[] = {
        "btl_party_tech_cmd_disable",
        "btl_party_item_cmd_disable",
    };
    
    int32_t r = ttyd::system::irand(2);
    int32_t icon_id;
    if (r == 0) {
        unit->party_moves_disabled_turns = 3;
        icon_id = IconType::PARTNER_MOVE_0;
    } else {
        unit->party_items_disabled_turns = 3;
        icon_id = IconType::ITEM_ICON;
    }
    
    evt->lwData[0] = PTR(strings[r]);
    evtSetValue(evt, evt->evtArguments[0], icon_id);
    
    return 2;    
}

EVT_DEFINE_USER_FUNC(evtTot_Kammy_MagicSupport) {  
    auto* battleWork = ttyd::battle::g_BattleWork;
    int32_t id = evtGetValue(evt, evt->evtArguments[0]);
    id = ttyd::battle_sub::BattleTransID(evt, id);
    auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, id);
    int32_t type = evtGetValue(evt, evt->evtArguments[1]);
    
    float x, y, z;
    ttyd::battle_unit::BtlUnit_GetPos(unit, &x, &y, &z);

    if (type == 1) {
        x += 30.0f * unit->unk_scale * unit->movement_params.face_direction;
        y += 50.0f * unit->unk_scale;
        z += 5.0f;
        ttyd::eff_stardust::effStardustEntry(x, y, z, 40.0f, 40.0f, 6, 5, 30);
    } else if (type == 0) {
        x += 30.0f * unit->unk_scale * unit->movement_params.face_direction;
        y += 10.0f * unit->unk_scale;
        z += 5.0f;
        ttyd::eff_stardust::effStardustEntry(x, y, z, 40.0f, 40.0f, 6, 5, 30);
    }

    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_Kammy_GetMagicParticlesX) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    int32_t id = evtGetValue(evt, evt->evtArguments[0]);
    id = ttyd::battle_sub::BattleTransID(evt, id);
    auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, id);
    
    evtSetFloat(
        evt, evt->evtArguments[1],
        *reinterpret_cast<float*>(reinterpret_cast<intptr_t>(
            reinterpret_cast<EffEntry*>(
                unit->unit_work[UW_Kammy_MagicEff])->eff_work) + 0x10));
    
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_Kammy_SpawnMagicParticles) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    int32_t id = evtGetValue(evt, evt->evtArguments[0]);
    id = ttyd::battle_sub::BattleTransID(evt, id);
    auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, id);
    
    float x, y, z;
    ttyd::battle_unit::BtlUnit_GetPos(unit, &x, &y, &z);
    
    EffEntry* eff = ttyd::eff_magic1_n64::effMagic1N64Entry(
        x + 28.0f * unit->unk_scale * unit->movement_params.face_direction,
        y + 20.0f * unit->unk_scale,
        z + 5.0f,
        evtGetFloat(evt, evt->evtArguments[1]),
        evtGetFloat(evt, evt->evtArguments[2]),
        evtGetFloat(evt, evt->evtArguments[3]),
        0,
        evtGetValue(evt, evt->evtArguments[4]));
        
    unit->unit_work[UW_Kammy_MagicEff] = reinterpret_cast<uint32_t>(eff);
    *reinterpret_cast<float*>(
        reinterpret_cast<intptr_t>(eff->eff_work) + 0x28) = unit->unk_scale;

    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_Kammy_HandleFlareParticles) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    int32_t id = evtGetValue(evt, evt->evtArguments[0]);
    id = ttyd::battle_sub::BattleTransID(evt, id);
    auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, id);
    
    if (isFirstCall) {
        EffEntry* eff = ttyd::eff_thunderflare_n64::effThunderflareN64Entry(
            0.0f, -1000.0f, 0.0f, 1.0f, 0, 60);
        unit->unit_work[UW_Kammy_FlareEff] = reinterpret_cast<uint32_t>(eff);
    }
    
    auto* eff = reinterpret_cast<EffEntry*>(unit->unit_work[UW_Kammy_FlareEff]);
    if (eff->flags & 1) {
        return 2;
    }
    
    float x, y, z;
    ttyd::battle_unit::BtlUnit_GetPos(unit, &x, &y, &z);
    
    intptr_t work = reinterpret_cast<intptr_t>(eff->eff_work);
    
    *reinterpret_cast<float*>(work + 0x08) = 
        x + 30.0f * unit->unk_scale * unit->movement_params.face_direction;
    *reinterpret_cast<float*>(work + 0x0c) = y + 35.0f * unit->unk_scale;
    *reinterpret_cast<float*>(work + 0x10) = z + 5.0f;
    *reinterpret_cast<float*>(work + 0x28) = unit->unk_scale;
    
    return 0;
}

}  // namespace mod::tot::custom