#include "tot_custom_rel.h"     // For externed unit declarations

#include "evt_cmd.h"
#include "mod.h"
#include "tot_gsw.h"
#include "tot_manager_dialogue.h"
#include "tot_party_bobbery.h"
#include "tot_party_flurrie.h"
#include "tot_party_goombella.h"
#include "tot_party_koops.h"
#include "tot_party_mario.h"
#include "tot_party_mowz.h"
#include "tot_party_vivian.h"
#include "tot_party_yoshi.h"
#include "tot_state.h"

#include <gc/types.h>
#include <ttyd/animdrv.h>
#include <ttyd/battle.h>
#include <ttyd/battle_camera.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_disp.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_event_default.h>
#include <ttyd/battle_event_subset.h>
#include <ttyd/battle_sub.h>
#include <ttyd/battle_unit.h>
#include <ttyd/battle_weapon_power.h>
#include <ttyd/effdrv.h>
#include <ttyd/eff_scanning.h>
#include <ttyd/evt_eff.h>
#include <ttyd/evt_map.h>
#include <ttyd/evt_msg.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evt_sub.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/mario_pouch.h>
#include <ttyd/unit_mario.h>
#include <ttyd/unit_party_christine.h>
#include <ttyd/unit_party_chuchurina.h>
#include <ttyd/unit_party_clauda.h>
#include <ttyd/unit_party_nokotarou.h>
#include <ttyd/unit_party_sanders.h>
#include <ttyd/unit_party_vivian.h>
#include <ttyd/unit_party_yoshi.h>

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
using namespace ::ttyd::evt_eff;
using namespace ::ttyd::evt_map;
using namespace ::ttyd::evt_msg;
using namespace ::ttyd::evt_snd;
using namespace ::ttyd::evt_sub;

using ::ttyd::effdrv::EffEntry;
using ::ttyd::evtmgr_cmd::evtGetFloat;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

namespace BattleUnitAttribute_Flags = ttyd::battle_unit::BattleUnitAttribute_Flags;

}  // namespace

// Unit work variable definitions; leave plenty blank for tail event internals.
constexpr const int32_t UW_Doopliss_AiState = 8;
constexpr const int32_t UW_Doopliss_FormTurnCount = 9;
constexpr const int32_t UW_Doopliss_CurrentForm = 10;
constexpr const int32_t UW_Doopliss_VivianTailEvent = 11;
constexpr const int32_t UW_Doopliss_DisabledNJNH = 12;

// Definition for currently selected weapon; exposed for ease of integration
// into party scripts.
ttyd::battle_database_common::BattleWeapon* unitDoopliss_weaponSelected = nullptr;

// Function / USER_FUNC declarations.
EVT_DECLARE_USER_FUNC(evtTot_Doopliss_MakeExtraWorkArea, 0)
EVT_DECLARE_USER_FUNC(evtTot_Doopliss_SelectWeapon, 1)
EVT_DECLARE_USER_FUNC(evtTot_Doopliss_CheckVeiled, 1)
EVT_DECLARE_USER_FUNC(evtTot_Doopliss_SpawnScanEffect, 4)
EVT_DECLARE_USER_FUNC(evtTot_Doopliss_HandleTransform, 1)
EVT_DECLARE_USER_FUNC(evtTot_Doopliss_DisableNJNH, 0)

extern DataTableEntry unitDoopliss_data_table[];

int8_t unitDoopliss_defense[] = { 0, 0, 0, 0, 0 };
int8_t unitDoopliss_defense_attr[] = { 0, 0, 0, 0, 0 };
int8_t unitDoopliss_defense_P2[] = { 1, 1, 1, 1, 1 };

StatusVulnerability unitDoopliss_status = {
     40,  40,  40, 100,  40, 100, 100,  40,
    100,  60, 100,  60, 100,  80,  80,   0,
      0, 100,  40, 100, 100,   0,
};
// Flurrie: doubly weak to Dizzy
StatusVulnerability unitDoopliss_status_P3 = {
     40,  40,  80, 100,  40, 100, 100,  40,
    100,  60, 100,  60, 100,  80,  80,   0,
      0, 100,  40, 100, 100,   0,
};
// Bobbery: doubly weak to Freeze
StatusVulnerability unitDoopliss_status_P6 = {
     40,  40,  40, 100,  40, 100, 100,  80,
    100,  60, 100,  60, 100,  80,  80,   0,
      0, 100,  40, 100, 100,   0,
};

PoseTableEntry unitDoopliss_pose_table[] = {
    1, "N_1",
    2, "Y_1",
    9, "Y_1",
    5, "K_1",
    4, "X_1",
    3, "K_1",
    28, "S_1",
    29, "Q_1",
    30, "Q_1",
    31, "S_1",
    39, "D_1",
    50, "A_2",
    42, "R_1",
    40, "W_1",
    56, "X_1",
    57, "X_1",
    65, "T_1",
    69, "S_1",
};

BattleWeapon unitDoopliss_weaponDefault = {
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
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::FLIPS_SHELLED |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags =
        AttackCounterResistance_Flags::FRONT_SPIKY |
        AttackCounterResistance_Flags::PREEMPTIVE_SPIKY,
    .target_weighting_flags =
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

BattleUnitKindPart unitDoopliss_parts[] = {
    {
        .index = 1,
        .name = "btl_un_rampell",
        .model_name = "c_ranpel",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 2.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitDoopliss_defense,
        .defense_attr = unitDoopliss_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitDoopliss_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_rampell",
        .model_name = "c_ranpel",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 2.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitDoopliss_defense,
        .defense_attr = unitDoopliss_defense_attr,
        .attribute_flags = 0x1301'0000,
        .counter_attribute_flags = 0,
        .pose_table = unitDoopliss_pose_table,
    },
    {
        .index = 3,
        .name = "btl_un_rampell",
        .model_name = "c_ranpel",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 2.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitDoopliss_defense,
        .defense_attr = unitDoopliss_defense_attr,
        .attribute_flags = 0x1301'0000,
        .counter_attribute_flags = 0,
        .pose_table = unitDoopliss_pose_table,
    },
    {
        .index = 4,
        .name = "btl_un_rampell",
        .model_name = "c_ranpel",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 2.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitDoopliss_defense,
        .defense_attr = unitDoopliss_defense_attr,
        .attribute_flags = 0x1301'0000,
        .counter_attribute_flags = 0,
        .pose_table = unitDoopliss_pose_table,
    },
    {
        .index = 5,
        .name = "btl_un_rampell",
        .model_name = "c_ranpel",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 2.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitDoopliss_defense,
        .defense_attr = unitDoopliss_defense_attr,
        .attribute_flags = 0x1301'0000,
        .counter_attribute_flags = 0,
        .pose_table = unitDoopliss_pose_table,
    },
    {
        .index = 6,
        .name = "btl_un_rampell",
        .model_name = "c_ranpel",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 2.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitDoopliss_defense,
        .defense_attr = unitDoopliss_defense_attr,
        .attribute_flags = 0x1301'0000,
        .counter_attribute_flags = 0,
        .pose_table = unitDoopliss_pose_table,
    },
    {
        .index = 7,
        .name = "btl_un_rampell",
        .model_name = "c_ranpel",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 2.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitDoopliss_defense,
        .defense_attr = unitDoopliss_defense_attr,
        .attribute_flags = 0x1301'0000,
        .counter_attribute_flags = 0,
        .pose_table = unitDoopliss_pose_table,
    },
};

BattleUnitKindPart unitDoopliss_parts_Mario[] = {
    {
        .index = 1,
        .name = "btl_un_rampell",
        .model_name = "b_mario",  // Actual model, might be needed for moves.
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 16.0f, 36.0f, 5.0f },
        .part_hit_cursor_base_offset = { 0.0f, 40.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 36,
        .base_alpha = 255,
        .defense = unitDoopliss_defense,
        .defense_attr = unitDoopliss_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = ttyd::unit_mario::pose_table_mario_stay,
    },
    {
        .index = 2,
        .name = "btl_un_rampell",
        .model_name = "b_mario",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 16.0f, 36.0f, 5.0f },
        .part_hit_cursor_base_offset = { 0.0f, 40.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 36,
        .base_alpha = 255,
        .defense = unitDoopliss_defense,
        .defense_attr = unitDoopliss_defense_attr,
        .attribute_flags = 0x1301'0000,
        .counter_attribute_flags = 0,
        .pose_table = ttyd::unit_mario::pose_table_object,
    },
    {
        .index = 3,
        .name = "btl_un_rampell",
        .model_name = "b_mario",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 16.0f, 36.0f, 5.0f },
        .part_hit_cursor_base_offset = { 0.0f, 40.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 36,
        .base_alpha = 255,
        .defense = unitDoopliss_defense,
        .defense_attr = unitDoopliss_defense_attr,
        .attribute_flags = 0x1301'0000,
        .counter_attribute_flags = 0,
        .pose_table = ttyd::unit_mario::pose_table_object,
    },
};

BattleUnitKindPart unitDoopliss_parts_P1[] = {
    {
        .index = 1,
        .name = "btl_un_rampell",
        .model_name = "c_pkr",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitDoopliss_defense,
        .defense_attr = unitDoopliss_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = ttyd::unit_party_christine::pose_table_christine_stay,
    },
};

BattleUnitKindPart unitDoopliss_parts_P2[] = {
    {
        .index = 1,
        .name = "btl_un_rampell",
        .model_name = "c_pnk",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitDoopliss_defense_P2,
        .defense_attr = unitDoopliss_defense_attr,
        .attribute_flags = 0x0000'1009,  // shell-flippable
        .counter_attribute_flags = 0,
        .pose_table = ttyd::unit_party_nokotarou::pose_table_nokotarou_stay,
    },
};

BattleUnitKindPart unitDoopliss_parts_P3[] = {
    {
        .index = 1,
        .name = "btl_un_rampell",
        .model_name = "c_windy",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitDoopliss_defense,
        .defense_attr = unitDoopliss_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = ttyd::unit_party_clauda::pose_table_clauda_stay,
    },
};

BattleUnitKindPart unitDoopliss_parts_P4[] = {
    {
        .index = 1,
        .name = "btl_un_rampell",
        .model_name = "c_babyyoshi",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 16.0f, 24.0f, 5.0f },
        .part_hit_cursor_base_offset = { 0.0f, 24.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 24,
        .base_alpha = 255,
        .defense = unitDoopliss_defense,
        .defense_attr = unitDoopliss_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = ttyd::unit_party_yoshi::pose_table_yoshi_stay,
    },
    {
        .index = 2,
        .name = "btl_un_rampell",
        .model_name = "c_tamago",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 16.0f, 24.0f, 5.0f },
        .part_hit_cursor_base_offset = { 0.0f, 24.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 24,
        .base_alpha = 255,
        .defense = unitDoopliss_defense,
        .defense_attr = unitDoopliss_defense_attr,
        .attribute_flags = 0x1301'0000,
        .counter_attribute_flags = 0,
        .pose_table = ttyd::unit_party_yoshi::pose_table_egg_g,
    },
    {
        .index = 3,
        .name = "btl_un_rampell",
        .model_name = "c_tamago",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 16.0f, 24.0f, 5.0f },
        .part_hit_cursor_base_offset = { 0.0f, 24.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 24,
        .base_alpha = 255,
        .defense = unitDoopliss_defense,
        .defense_attr = unitDoopliss_defense_attr,
        .attribute_flags = 0x1301'0000,
        .counter_attribute_flags = 0,
        .pose_table = ttyd::unit_party_yoshi::pose_table_egg_g,
    },
    {
        .index = 4,
        .name = "btl_un_rampell",
        .model_name = "c_tamago",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 16.0f, 24.0f, 5.0f },
        .part_hit_cursor_base_offset = { 0.0f, 24.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 24,
        .base_alpha = 255,
        .defense = unitDoopliss_defense,
        .defense_attr = unitDoopliss_defense_attr,
        .attribute_flags = 0x1301'0000,
        .counter_attribute_flags = 0,
        .pose_table = ttyd::unit_party_yoshi::pose_table_egg_g,
    },
    {
        .index = 5,
        .name = "btl_un_rampell",
        .model_name = "c_tamago",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 16.0f, 24.0f, 5.0f },
        .part_hit_cursor_base_offset = { 0.0f, 24.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 24,
        .base_alpha = 255,
        .defense = unitDoopliss_defense,
        .defense_attr = unitDoopliss_defense_attr,
        .attribute_flags = 0x1301'0000,
        .counter_attribute_flags = 0,
        .pose_table = ttyd::unit_party_yoshi::pose_table_egg_g,
    },
    {
        .index = 6,
        .name = "btl_un_rampell",
        .model_name = "c_tamago",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 16.0f, 24.0f, 5.0f },
        .part_hit_cursor_base_offset = { 0.0f, 24.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 24,
        .base_alpha = 255,
        .defense = unitDoopliss_defense,
        .defense_attr = unitDoopliss_defense_attr,
        .attribute_flags = 0x1301'0000,
        .counter_attribute_flags = 0,
        .pose_table = ttyd::unit_party_yoshi::pose_table_egg_g,
    },
    {
        .index = 7,
        .name = "btl_un_rampell",
        .model_name = "c_tamago",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 16.0f, 24.0f, 5.0f },
        .part_hit_cursor_base_offset = { 0.0f, 24.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 24,
        .base_alpha = 255,
        .defense = unitDoopliss_defense,
        .defense_attr = unitDoopliss_defense_attr,
        .attribute_flags = 0x1301'0000,
        .counter_attribute_flags = 0,
        .pose_table = ttyd::unit_party_yoshi::pose_table_egg_g,
    },
};

BattleUnitKindPart unitDoopliss_parts_P5[] = {
    {
        .index = 1,
        .name = "btl_un_rampell",
        .model_name = "c_vivian",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitDoopliss_defense,
        .defense_attr = unitDoopliss_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = ttyd::unit_party_vivian::pose_table_vivian_stay,
    },
};

BattleUnitKindPart unitDoopliss_parts_P6[] = {
    {
        .index = 1,
        .name = "btl_un_rampell",
        .model_name = "c_bomt_n",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitDoopliss_defense,
        .defense_attr = unitDoopliss_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = ttyd::unit_party_sanders::pose_table_sanders_stay,
    },
};

BattleUnitKindPart unitDoopliss_parts_P7[] = {
    {
        .index = 1,
        .name = "btl_un_rampell",
        .model_name = "c_tyutyu",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 16.0f, 24.0f, 5.0f },
        .part_hit_cursor_base_offset = { 0.0f, 24.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 24,
        .base_alpha = 255,
        .defense = unitDoopliss_defense,
        .defense_attr = unitDoopliss_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = ttyd::unit_party_chuchurina::pose_table_chuchurina_stay,
    },
    {
        .index = 2,
        .name = "btl_un_rampell",
        .model_name = "c_tyutyu",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 16.0f, 24.0f, 5.0f },
        .part_hit_cursor_base_offset = { 0.0f, 24.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 24,
        .base_alpha = 255,
        .defense = unitDoopliss_defense,
        .defense_attr = unitDoopliss_defense_attr,
        .attribute_flags = 0x1301'0000,
        .counter_attribute_flags = 0,
        .pose_table = ttyd::unit_party_chuchurina::pose_table_chuchurina_stay,
    },
    {
        .index = 3,
        .name = "btl_un_rampell",
        .model_name = "c_tyutyu",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 16.0f, 24.0f, 5.0f },
        .part_hit_cursor_base_offset = { 0.0f, 24.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 24,
        .base_alpha = 255,
        .defense = unitDoopliss_defense,
        .defense_attr = unitDoopliss_defense_attr,
        .attribute_flags = 0x1301'0000,
        .counter_attribute_flags = 0,
        .pose_table = ttyd::unit_party_chuchurina::pose_table_chuchurina_stay,
    },
    {
        .index = 4,
        .name = "btl_un_rampell",
        .model_name = "c_tyutyu",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 16.0f, 24.0f, 5.0f },
        .part_hit_cursor_base_offset = { 0.0f, 24.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 24,
        .base_alpha = 255,
        .defense = unitDoopliss_defense,
        .defense_attr = unitDoopliss_defense_attr,
        .attribute_flags = 0x1301'0000,
        .counter_attribute_flags = 0,
        .pose_table = ttyd::unit_party_chuchurina::pose_table_chuchurina_stay,
    },
};

EVT_BEGIN(unitDoopliss_self_camera_focus_event)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), EVT_NULLPTR, EVT_NULLPTR)
    USER_FUNC(evt_btl_camera_set_mode, 0, 3)
    USER_FUNC(evt_btl_camera_set_moveto, 0, LW(0), 65, 332, LW(0), 30, -340, 30, 11)
    RETURN()
EVT_END()

EVT_BEGIN(unitDoopliss_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    
    USER_FUNC(btlevtcmd_GetUnitKind, -2, LW(0))
    IF_EQUAL(LW(0), (int32_t)BattleUnitType::DOOPLISS_CH_8_VIVIAN)
        // Have the tail resume tracking the body, in case it was off before.
        USER_FUNC(btlevtcmd_SetUnitWork, -2, 3, 0)
    END_IF()

    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    RETURN()
EVT_END()

EVT_BEGIN(unitDoopliss_normal_attack_event)
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
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(evt_btl_camera_set_mode, 0, 8)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, LW(3))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("R_1"))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 20)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(2.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_1A"))
    WAIT_MSEC(250)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_1B"))
    WAIT_MSEC(500)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_2"))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    ADD(LW(0), 120)
    ADD(LW(1), 70)
    USER_FUNC(btlevtcmd_AnimeSetMotionSpeed, -2, 1, FLOAT(2.0))
    BROTHER_EVT()
        WAIT_FRM(10)
        USER_FUNC(btlevtcmd_AnimeSetMotionSpeed, -2, 1, FLOAT(1.5))
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_RNPL_FLY1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        WAIT_FRM(15)
        USER_FUNC(btlevtcmd_AnimeSetMotionSpeed, -2, 1, FLOAT(1.0))
    END_BROTHER()
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 50, 7, 4, 0, -1)
    USER_FUNC(btlevtcmd_AnimeSetMotionSpeed, -2, 1, FLOAT(0.75))
    WAIT_MSEC(200)
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 15, 0)
        USER_FUNC(evt_sub_intpl_msec_init, 1, 0, 135, 200)
        DO(0)
            USER_FUNC(evt_sub_intpl_msec_get_value_para, LW(0), LW(1))
            USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, LW(0))
            IF_EQUAL(LW(1), 0)
                DO_BREAK()
            END_IF()
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_RNPL_FALL1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    WAIT_MSEC(80)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(0), LW(2), 1, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_AnimeSetMotionSpeed, -2, 1, FLOAT(2.0))
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.01))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 15, -1)
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
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_RNPL_DOWN1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    USER_FUNC(btlevtcmd_AnimeSetMotionSpeed, -2, 1, FLOAT(2.0))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("D_2"))
    WAIT_MSEC(200)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("O_1"))
    WAIT_MSEC(800)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("R_2"))
    GOTO(98)
LBL(91)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
    USER_FUNC(btlevtcmd_AnimeSetMotionSpeed, -2, 1, FLOAT(1.0))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    USER_FUNC(btlevtcmd_AnimeSetMotionSpeed, -2, 1, FLOAT(2.0))
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 15, 0)
        USER_FUNC(evt_sub_intpl_init, 4, 135, 0, 20)
        DO(0)
            USER_FUNC(evt_sub_intpl_get_value_para, LW(0), LW(1))
            USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, LW(0))
            IF_EQUAL(LW(1), 0)
                DO_BREAK()
            END_IF()
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_AnimeSetMotionSpeed, -2, 1, FLOAT(1.0))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("J_1B"))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    ADD(LW(0), 60)
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.75))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 20, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_RNPL_LANDING1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_1A"))
    WAIT_MSEC(200)
    USER_FUNC(btlevtcmd_AnimeSetMotionSpeed, -2, 1, FLOAT(2.0))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("R_1"))
LBL(98)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(4.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    USER_FUNC(btlevtcmd_AnimeSetMotionSpeed, -2, 1, FLOAT(1.0))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_1"))
    RETURN()
EVT_END()

EVT_BEGIN(unitDoopliss_attack_event)
    SET((int32_t)GSW_Battle_DooplissMove, 1)
    SET(LW(15), 0)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Doopliss_CurrentForm, LW(0))
    SWITCH(LW(0))
        CASE_EQUAL(0)
            SET(LW(9), PTR(&unitDoopliss_weaponDefault))
            RUN_CHILD_EVT(PTR(&unitDoopliss_normal_attack_event))
            GOTO(99)
        CASE_ETC()
            // Select a weapon randomly based on the transformed character.
            USER_FUNC(evtTot_Doopliss_SelectWeapon, LW(15))
    END_SWITCH()

    IF_NOT_EQUAL(LW(15), 0)
        // If a move script was selected, run it, otherwise just idle.
        RUN_CHILD_EVT(LW(15))
    ELSE()
        // No valid moves; Mario must have equipped both Jumpman + Hammerman.
        USER_FUNC(btlevtcmd_StatusWindowOnOff, 0)
        USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Doopliss_DisabledNJNH, 1)

        USER_FUNC(evtTot_SetConversation, (int32_t)ConversationId::BOSS_3_DISABLE)
        USER_FUNC(evtTot_GetNextMessage, LW(0), LW(1))
        USER_FUNC(evt_msg_print, 2, LW(0), 0, -2)

        USER_FUNC(btlevtcmd_StatusWindowOnOff, 1)
        USER_FUNC(evtTot_Doopliss_DisableNJNH)
    END_IF()

LBL(99)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitDoopliss_transform_event)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Doopliss_CurrentForm, LW(0))
    SWITCH(LW(0))
        CASE_EQUAL(0)
            // If in default form, transform to Mario/partner randomly,
            // or Mario if no partner exists.
            USER_FUNC(btlevtcmd_GetUnitId, -4, LW(15))
            IF_EQUAL(LW(15), -1)
                SET(LW(5), 0)
            ELSE()
                USER_FUNC(evt_sub_random, 1, LW(5))
            END_IF()
        CASE_EQUAL(1)
            // If no partner exists, can't transform any more.
            USER_FUNC(btlevtcmd_GetUnitId, -4, LW(15))
            IF_EQUAL(LW(15), -1)
                GOTO(99)
            END_IF()
            SET(LW(5), 1)
        CASE_ETC()
            // If in form of partner, choose randomly between Mario and
            // partner if the partner is different, or just Mario otherwise.
            USER_FUNC(btlevtcmd_GetUnitKind, -4, LW(2))
            SWITCH(LW(2))
                CASE_EQUAL((int32_t)BattleUnitType::GOOMBELLA)
                    SET(LW(1), 2)
                CASE_EQUAL((int32_t)BattleUnitType::KOOPS)
                    SET(LW(1), 3)
                CASE_EQUAL((int32_t)BattleUnitType::FLURRIE)
                    SET(LW(1), 4)
                CASE_EQUAL((int32_t)BattleUnitType::YOSHI)
                    SET(LW(1), 5)
                CASE_EQUAL((int32_t)BattleUnitType::VIVIAN)
                    SET(LW(1), 6)
                CASE_EQUAL((int32_t)BattleUnitType::BOBBERY)
                    SET(LW(1), 7)
                CASE_EQUAL((int32_t)BattleUnitType::MS_MOWZ)
                    SET(LW(1), 8)
            END_SWITCH()
            IF_EQUAL(LW(0), LW(1))
                SET(LW(5), 0)
            ELSE()
                USER_FUNC(evt_sub_random, 1, LW(5))
            END_IF()
    END_SWITCH()

    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Doopliss_VivianTailEvent, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Doopliss_VivianTailEvent, 0)
        DELETE_EVT(LW(0))
    END_IF()
        
    USER_FUNC(evt_btl_camera_set_mode, 0, 3)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(10), EVT_NULLPTR, EVT_NULLPTR)
    USER_FUNC(evt_btl_camera_set_moveto, 0, LW(10), 65, 332, LW(10), 30, -340, 30, 11)
    WAIT_FRM(37)
    
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Doopliss_CurrentForm, LW(0))
    IF_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_RNPL_ARM_UP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_3A"))
        WAIT_MSEC(600)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_RNPL_ARM_UP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_3A"))
        WAIT_MSEC(600)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_RNPL_ARM_UP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_3A"))
        WAIT_MSEC(200)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_RNPL_LAUGH1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_3B"))
        WAIT_MSEC(800)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_RNPL_EYE_SHINE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(6))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        SETF(LW(7), FLOAT(-6.0))
        MULF(LW(7), LW(6))
        ADDF(LW(0), LW(7))
        SETF(LW(7), FLOAT(33.0))
        MULF(LW(7), LW(6))
        ADDF(LW(1), LW(7))
        USER_FUNC(evt_eff, PTR(""), PTR("toge_flush"), 3, LW(0), LW(1), LW(2), 60, 0, 0, 0, 0, 0, 0, 0)
        SETF(LW(7), FLOAT(7.0))
        MULF(LW(7), LW(6))
        ADDF(LW(0), LW(7))
        SETF(LW(7), FLOAT(-1.0))
        MULF(LW(7), LW(6))
        ADDF(LW(1), LW(7))
        USER_FUNC(evt_eff, PTR(""), PTR("toge_flush"), 3, LW(0), LW(1), LW(2), 60, 0, 0, 0, 0, 0, 0, 0)
        WAIT_MSEC(1000)
    END_IF()

    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_RNPL_TRANSFORM4"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    ADD(LW(2), 5)
    USER_FUNC(evt_eff, PTR(""), PTR("kemuri_test"), 7, LW(0), LW(1), LW(2), FLOAT(2.0), 0, 0, 0, 0, 0, 0, 0)
    WAIT_MSEC(200)
    USER_FUNC(btlevtcmd_SetPos, -2, -1000, -1000, 0)
    WAIT_MSEC(500)
    USER_FUNC(evt_btl_camera_set_mode, 0, 3)
    USER_FUNC(evt_btl_camera_set_moveto, 0, -110, 65, 332, -110, 30, -340, 30, 11)
    WAIT_FRM(37)
    
    SWITCH(LW(5))
        CASE_EQUAL(0)
            USER_FUNC(btlevtcmd_GetPos, -3, LW(0), LW(1), LW(2))
            ADD(LW(0), 2)
            ADD(LW(1), -5)
            ADD(LW(2), 5)
            USER_FUNC(evtTot_Doopliss_SpawnScanEffect, -3, LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_snd_se, -3, PTR("SFX_BOSS_RNPL_TRANSFORM2"), EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_MSEC(200)
            USER_FUNC(btlevtcmd_snd_se, -3, PTR("SFX_BOSS_RNPL_TRANSFORM1"), EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_MSEC(1300)
            USER_FUNC(btlevtcmd_snd_se, -3, PTR("SFX_BOSS_RNPL_TRANSFORM3"), EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_MSEC(500)
            USER_FUNC(evt_btl_camera_set_mode, 0, 3)
            USER_FUNC(evt_btl_camera_set_moveto, 0, LW(10), 65, 332, LW(10), 30, -340, 30, 11)
            WAIT_FRM(37)
            
            // Handle initializing poses, parts, etc.
            USER_FUNC(evtTot_Doopliss_HandleTransform, (int32_t)BattleUnitType::MARIO)
        CASE_EQUAL(1)
            USER_FUNC(btlevtcmd_GetPos, -4, LW(0), LW(1), LW(2))
            ADD(LW(0), 2)
            ADD(LW(1), -5)
            ADD(LW(2), 5)
            USER_FUNC(evtTot_Doopliss_SpawnScanEffect, -4, LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_snd_se, -3, PTR("SFX_BOSS_RNPL_TRANSFORM2"), EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_MSEC(200)
            USER_FUNC(btlevtcmd_snd_se, -3, PTR("SFX_BOSS_RNPL_TRANSFORM1"), EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_MSEC(1300)
            USER_FUNC(btlevtcmd_snd_se, -3, PTR("SFX_BOSS_RNPL_TRANSFORM3"), EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_MSEC(500)
            USER_FUNC(evt_btl_camera_set_mode, 0, 3)
            USER_FUNC(evt_btl_camera_set_moveto, 0, LW(10), 65, 332, LW(10), 30, -340, 30, 11)
            WAIT_FRM(37)
            
            // Handle initializing poses, parts, etc.
            USER_FUNC(btlevtcmd_GetUnitKind, -4, LW(0))
            USER_FUNC(evtTot_Doopliss_HandleTransform, LW(0))
            
            // Additional setup as necessary.
            SWITCH(LW(0))
                CASE_EQUAL((int32_t)BattleUnitType::YOSHI)
                    USER_FUNC(ttyd::unit_party_yoshi::yoshi_original_color_anim_set, -2)
                CASE_EQUAL((int32_t)BattleUnitType::VIVIAN)
                    RUN_EVT_ID(PTR(&ttyd::unit_party_vivian::vivian_shadow_tail_event), LW(0))
                    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Doopliss_VivianTailEvent, LW(0))
            END_SWITCH()
    END_SWITCH()
    
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(5), LW(6), LW(7))
    USER_FUNC(evt_sub_intpl_msec_init, 11, -50, 0, 1000)
    DO(0)
        USER_FUNC(evt_sub_intpl_msec_get_value_para, LW(0), LW(1))
        USER_FUNC(btlevtcmd_SetPos, -2, LW(5), LW(0), LW(7))
        IF_EQUAL(LW(1), 0)
            DO_BREAK()
        END_IF()
        WAIT_FRM(1)
    WHILE()

    // Mario doesn't have a talking animation; it's probably fine.
    USER_FUNC(btlevtcmd_SetTalkPoseType, -2, 65)
    USER_FUNC(btlevtcmd_SetStayPoseType, -2, 43)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitDoopliss_P2_flip_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_SetPartsDefenceTable, LW(10), LW(11), PTR(&unitDoopliss_defense))
    USER_FUNC(btlevtcmd_AnimeSetPoseTable, LW(10), LW(11), PTR(&ttyd::unit_party_nokotarou::pose_table_nokotarou_turn))
    USER_FUNC(btlevtcmd_SetOverTurnCount, LW(10), 2)
    USER_FUNC(btlevtcmd_AnimeChangePose, LW(10), LW(11), PTR("PNK_D_1"))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_INSIDE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(1.0))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 6, -1)
    RETURN()
EVT_END()

EVT_BEGIN(unitDoopliss_P2_wakeup_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_SetPartsDefenceTable, LW(10), LW(11), PTR(&unitDoopliss_defense_P2))
    USER_FUNC(btlevtcmd_AnimeSetPoseTable, LW(10), LW(11), PTR(&ttyd::unit_party_nokotarou::pose_table_nokotarou_stay))
    USER_FUNC(btlevtcmd_SetOverTurnCount, LW(10), 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, LW(10), LW(11), PTR("PNK_S_1"))
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(1.0))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
    RETURN()
EVT_END()

EVT_BEGIN(unitDoopliss_phase_event)
    USER_FUNC(btlevtcmd_CheckPhase, LW(0), 0x4000003)
    IF_EQUAL(LW(0), 0)
        GOTO(99)
    END_IF()

    USER_FUNC(btlevtcmd_CheckActStatus, -2, LW(0))
    IF_EQUAL(LW(0), 0)
        GOTO(99)
    END_IF()

    // Check for Withdraw status.
    USER_FUNC(btlevtcmd_CheckPartsAttribute, -2, 1, int(0xe0000000), LW(0))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(1.0))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
        USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, int(0xe0000000))
        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 1, int(0x1000))
    END_IF()
    // Check for flipped status.
    USER_FUNC(btlevtcmd_GetOverTurnCount, -2, LW(0))
    IF_LARGE(LW(0), 0)
        RUN_CHILD_EVT(unitDoopliss_P2_wakeup_event)
    END_IF()

    // Check for low-health events.
    USER_FUNC(btlevtcmd_GetHp, -2, LW(11))
    USER_FUNC(btlevtcmd_GetMaxHp, -2, LW(12))
    MUL(LW(11), 100)
    DIV(LW(11), LW(12))
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Doopliss_AiState, LW(0))
    
    IF_SMALL(LW(11), 40)
        IF_SMALL(LW(0), 3)
            USER_FUNC(btlevtcmd_StatusWindowOnOff, 0)
            USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Doopliss_AiState, 3)

            USER_FUNC(evtTot_SetConversation, (int32_t)ConversationId::BOSS_3_P3)
            USER_FUNC(evtTot_GetNextMessage, LW(0), LW(1))
            USER_FUNC(evt_msg_print, 2, LW(0), 0, -2)

            USER_FUNC(btlevtcmd_StatusWindowOnOff, 1)
        END_IF()
    ELSE()
        IF_SMALL(LW(11), 70)
            IF_SMALL(LW(0), 2)
                USER_FUNC(btlevtcmd_StatusWindowOnOff, 0)
                USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Doopliss_AiState, 2)

                USER_FUNC(evtTot_SetConversation, (int32_t)ConversationId::BOSS_3_P2)
                USER_FUNC(evtTot_GetNextMessage, LW(0), LW(1))
                USER_FUNC(evt_msg_print, 2, LW(0), 0, -2)

                USER_FUNC(btlevtcmd_StatusWindowOnOff, 1)
            END_IF()
        END_IF()
    END_IF()

    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Doopliss_FormTurnCount, LW(0))
    IF_LARGE(LW(0), 0)
        USER_FUNC(btlevtcmd_AddUnitWork, -2, UW_Doopliss_FormTurnCount, -1)
    ELSE()
        USER_FUNC(evtTot_Doopliss_CheckVeiled, LW(0))
        IF_EQUAL(LW(0), 0)
            USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Doopliss_FormTurnCount, 2)
            USER_FUNC(btlevtcmd_PhaseEventStartDeclare, -2)
            RUN_CHILD_EVT(PTR(&unitDoopliss_transform_event))
            USER_FUNC(btlevtcmd_StartWaitEvent, -2)
        END_IF()
    END_IF()
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitDoopliss_wait_event)
    SET((int32_t)GSW_Battle_DooplissMove, 0)

    // Handle Koops' withdraw state, otherwise use default idle pose.
    USER_FUNC(btlevtcmd_CheckPartsAttribute, -2, 1, int(0xe0000000), LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PNK_A_1"))
    ELSE()
        USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    END_IF()

    RETURN()
EVT_END()

EVT_BEGIN(unitDoopliss_battle_entry_event)
    USER_FUNC(btlevtcmd_StatusWindowOnOff, 0)
    WAIT_FRM(60)
    WAIT_FRM(60)
    RUN_CHILD_EVT(PTR(&unitDoopliss_self_camera_focus_event))

    USER_FUNC(evtTot_SetConversation, (int32_t)ConversationId::BOSS_3_INTRO)
    USER_FUNC(evtTot_GetNextMessage, LW(0), LW(1))
    USER_FUNC(evt_msg_print, 2, LW(0), 0, -2)
    
    WAIT_MSEC(300)
    USER_FUNC(btlevtcmd_StatusWindowOnOff, 1)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitDoopliss_dead_event)
    SET((int32_t)GSW_Battle_DooplissMove, 0)
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
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
        USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
        USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    END_IF()
    USER_FUNC(btlevtcmd_WaitAttackEnd)
    WAIT_FRM(1)
    USER_FUNC(btlevtcmd_GetUnitKind, -2, LW(0))
    IF_NOT_EQUAL(LW(0), (int32_t)BattleUnitType::DOOPLISS_CH_8)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        ADD(LW(2), 5)
        USER_FUNC(evt_eff, PTR(""), PTR("kemuri_test"), 7, LW(0), LW(1), LW(2), FLOAT(2.0), 0, 0, 0, 0, 0, 0, 0)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_RNPL_TRANSFORM4"), EVT_NULLPTR, 0, EVT_NULLPTR)
        WAIT_MSEC(300)
        // Transform back to Doopliss.
        USER_FUNC(evtTot_Doopliss_HandleTransform, (int32_t)BattleUnitType::DOOPLISS_CH_8)
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("D_2"))
    USER_FUNC(btlevtcmd_SetTalkPose, -2, PTR("D_2"))
    USER_FUNC(btlevtcmd_SetStayPose, -2, PTR("D_2"))
    USER_FUNC(btlevtcmd_WaitAttackEnd)
    USER_FUNC(btlevtcmd_StatusWindowOnOff, 0)
    RUN_CHILD_EVT(PTR(&unitDoopliss_self_camera_focus_event))

    USER_FUNC(evtTot_SetConversation, (int32_t)ConversationId::BOSS_3_DEATH)
    USER_FUNC(evtTot_GetNextMessage, LW(0), LW(1))
    USER_FUNC(evt_msg_print, 2, LW(0), 0, -2)
    
    USER_FUNC(btlevtcmd_StatusWindowOnOff, 1)
LBL(90)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
LBL(99)
    SET(LW(10), -2)
    RUN_CHILD_EVT(PTR(&subsetevt_dead_core_nospin_norotate))
    RETURN()
EVT_END()

EVT_BEGIN(unitDoopliss_Mario_dead_event)
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
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
        USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
        USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_I_S"))
    WAIT_FRM(152)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_NM_DOWN2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("M_D_2"))
    WAIT_MSEC(500)
    RUN_CHILD_EVT(PTR(&unitDoopliss_dead_event))
    RETURN()
EVT_END()

EVT_BEGIN(unitDoopliss_Party_dead_event)
    USER_FUNC(btlevtcmd_GetUnitKind, -2, LW(7))
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
        IF_EQUAL(LW(7), (int32_t)BattleUnitType::DOOPLISS_CH_8_KOOPS)
            USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
        ELSE()
            USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
        END_IF()
        USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
        USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    END_IF()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_NM_DOWN3"), EVT_NULLPTR, 0, EVT_NULLPTR)
    SWITCH(LW(7))
        CASE_EQUAL((int32_t)BattleUnitType::DOOPLISS_CH_8_GOOMBELLA)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKR_D_3"))
        CASE_EQUAL((int32_t)BattleUnitType::DOOPLISS_CH_8_KOOPS)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PNK_D_3"))
        CASE_EQUAL((int32_t)BattleUnitType::DOOPLISS_CH_8_FLURRIE)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PWD_D_3"))
        CASE_EQUAL((int32_t)BattleUnitType::DOOPLISS_CH_8_YOSHI)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PYS_D_3"))
        CASE_EQUAL((int32_t)BattleUnitType::DOOPLISS_CH_8_VIVIAN)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PTR_D_3"))
        CASE_EQUAL((int32_t)BattleUnitType::DOOPLISS_CH_8_BOBBERY)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("D_3"))
        CASE_EQUAL((int32_t)BattleUnitType::DOOPLISS_CH_8_MS_MOWZ)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PCH_D_3"))
    END_SWITCH()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_PARTY_BATTLE_DIE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    WAIT_MSEC(500)
    RUN_CHILD_EVT(PTR(&unitDoopliss_dead_event))
    RETURN()
EVT_END()

EVT_BEGIN(unitDoopliss_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitDoopliss_wait_event))
    USER_FUNC(btlevtcmd_SetEventPhase, -2, PTR(&unitDoopliss_phase_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitDoopliss_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitDoopliss_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&btldefaultevt_Confuse))
    USER_FUNC(btlevtcmd_SetEventEntry, -2, PTR(&unitDoopliss_battle_entry_event))
    
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Doopliss_AiState, 1)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Doopliss_FormTurnCount, 1)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Doopliss_CurrentForm, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Doopliss_VivianTailEvent, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Doopliss_DisabledNJNH, 0)

    // Necessary for some attacks that have variable success (e.g. Tease).
    USER_FUNC(evtTot_Doopliss_MakeExtraWorkArea)
    
    USER_FUNC(btlevtcmd_SetWalkSound, -2, PTR("SFX_BOSS_RNPL_MOVE1L"), PTR("SFX_BOSS_RNPL_MOVE1R"), 0, 15, 15)
    USER_FUNC(btlevtcmd_SetRunSound, -2, PTR("SFX_BOSS_RNPL_MOVE1L"), PTR("SFX_BOSS_RNPL_MOVE1R"), 0, 8, 8)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

DataTableEntry unitDoopliss_data_table[] = {
    49, (void*)unitDoopliss_dead_event,
    0, nullptr,
};

DataTableEntry unitDoopliss_data_table_Mario[] = {
    49, (void*)unitDoopliss_Mario_dead_event,
    0, nullptr,
};

DataTableEntry unitDoopliss_data_table_P1[] = {
    49, (void*)unitDoopliss_Party_dead_event,
    0, nullptr,
};

DataTableEntry unitDoopliss_data_table_P2[] = {
    49, (void*)unitDoopliss_Party_dead_event,
    13, (void*)unitDoopliss_P2_flip_event,
    0, nullptr,
};

DataTableEntry unitDoopliss_data_table_P3[] = {
    49, (void*)unitDoopliss_Party_dead_event,
    0, nullptr,
};

DataTableEntry unitDoopliss_data_table_P4[] = {
    49, (void*)unitDoopliss_Party_dead_event,
    0, nullptr,
};

DataTableEntry unitDoopliss_data_table_P5[] = {
    49, (void*)unitDoopliss_Party_dead_event,
    0, nullptr,
};

DataTableEntry unitDoopliss_data_table_P6[] = {
    49, (void*)unitDoopliss_Party_dead_event,
    0, nullptr,
};

DataTableEntry unitDoopliss_data_table_P7[] = {
    49, (void*)unitDoopliss_Party_dead_event,
    0, nullptr,
};

BattleUnitKind unit_Doopliss = {
    .unit_type = BattleUnitType::DOOPLISS_CH_8,
    .unit_name = "btl_un_rampell",
    .max_hp = 40,
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
    .width = 50,
    .height = 48,
    .hit_offset = { 4, 48 },
    .center_offset = { 0.0f, 22.0f, 0.0f },
    .hp_gauge_offset = { 0, -3 },
    .talk_toge_base_offset = { 0.0f, 40.0f, 0.0f },
    .held_item_base_offset = { 25.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 18.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 12.0f, 32.0f, 0.0f },
    .cut_base_offset = { 7.0f, 20.0f, 0.0f },
    .cut_width = 50.0f,
    .cut_height = 48.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_BOSS_RNPL_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitDoopliss_status,
    .num_parts = 7,
    .parts = unitDoopliss_parts,
    .init_evt_code = (void*)unitDoopliss_init_event,
    .data_table = unitDoopliss_data_table,
};


EVT_DEFINE_USER_FUNC(evtTot_Doopliss_MakeExtraWorkArea) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(evt, -2);
    auto* battleWork = ttyd::battle::g_BattleWork;
    auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, unit_idx);
    unit->extra_work = ttyd::battle::BattleAlloc(sizeof(BattleWeapon));

    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_Doopliss_SelectWeapon) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(evt, -2);
    auto* battleWork = ttyd::battle::g_BattleWork;
    auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, unit_idx);

    int32_t ai_phase = unit->unit_work[UW_Doopliss_AiState];
    int32_t current_form = unit->unit_work[UW_Doopliss_CurrentForm];

    int32_t move_ids[16];
    int32_t num_attacks = 0;

    if (current_form == 1) {
        if (unit->badges_equipped.hammerman < 1) {
            for (int32_t i = 0; i < 8; ++i) {
                int32_t move = MoveType::JUMP_BASE + i;
                int32_t unlocked_level = MoveManager::GetUnlockedLevel(move);
                const auto* data = MoveManager::GetMoveData(move);
                if (unlocked_level && (ai_phase == 3 || data->move_tier < ai_phase)) {
                    move_ids[num_attacks] = move;
                    ++num_attacks;
                }
            }
        }
        if (unit->badges_equipped.jumpman < 1) {
            for (int32_t i = 0; i < 8; ++i) {
                int32_t move = MoveType::HAMMER_BASE + i;
                int32_t unlocked_level = MoveManager::GetUnlockedLevel(move);
                const auto* data = MoveManager::GetMoveData(move);
                if (unlocked_level && (ai_phase == 3 || data->move_tier < ai_phase)) {
                    move_ids[num_attacks] = move;
                    ++num_attacks;
                }
            }
        }
    } else {
        int32_t move_start = MoveType::GOOMBELLA_BASE + (current_form - 2) * 6;

        for (int32_t move = move_start; move < move_start + 6; ++move) {
            switch (move) {
                case MoveType::GOOMBELLA_BASE:
                case MoveType::GOOMBELLA_MULTIBONK:
                case MoveType::GOOMBELLA_IRONBONK:
                
                case MoveType::KOOPS_BASE:
                case MoveType::KOOPS_POWER_SHELL:
                case MoveType::KOOPS_WITHDRAW:
                case MoveType::KOOPS_BULK_UP:
                case MoveType::KOOPS_SHELL_SLAM:

                case MoveType::FLURRIE_BASE:
                case MoveType::FLURRIE_LIP_LOCK:
                case MoveType::FLURRIE_BLIZZARD:
                case MoveType::FLURRIE_THUNDER_STORM:

                case MoveType::YOSHI_BASE:
                case MoveType::YOSHI_GULP:
                case MoveType::YOSHI_EGG_BARRAGE:
                case MoveType::YOSHI_MINI_EGG:
                case MoveType::YOSHI_STAMPEDE:

                case MoveType::VIVIAN_BASE:
                case MoveType::VIVIAN_CURSE:
                case MoveType::VIVIAN_NEUTRALIZE:
                case MoveType::VIVIAN_FIERY_JINX:
                
                case MoveType::BOBBERY_BASE:
                case MoveType::BOBBERY_BOMB_SQUAD:
                case MoveType::BOBBERY_POISON_BOMB:
                case MoveType::BOBBERY_BOBOMBAST:
                // This is probably a little too mean, given Bobbery's already
                // got two unguardable moves and you'd have one turn to react.
                // case MoveType::BOBBERY_MEGATON_BOMB:

                case MoveType::MOWZ_BASE:
                case MoveType::MOWZ_TEASE:
                case MoveType::MOWZ_SMOKE_BOMB: {
                    int32_t unlocked_level = MoveManager::GetUnlockedLevel(move);
                    const auto* data = MoveManager::GetMoveData(move);
                    if (unlocked_level && data->move_tier <= ai_phase) {
                        move_ids[num_attacks] = move;
                        ++num_attacks;
                    }
                    break;
                }
            }
        }
    }

    if (num_attacks == 0) {
        evtSetValue(evt, evt->evtArguments[0], 0);
        return 2;
    }

    int32_t move = move_ids[g_Mod->state_.Rand(num_attacks)];
    int32_t base_move = MoveManager::GetBaseMoveType(move);

    BattleWeapon* weapon = nullptr;
    switch (base_move) {
        case MoveType::JUMP_BASE:
            weapon = party_mario::g_CustomJumpWeapons[move - base_move];
            break;
        case MoveType::HAMMER_BASE:
            if (unit->badges_equipped.hammerman == 0) {
                weapon = party_mario::g_CustomHammerWeapons[move - base_move];
            } else {
                weapon = party_mario::g_CustomHammerThrowWeapons[move - base_move];
            }
            break;
        case MoveType::GOOMBELLA_BASE:
            weapon = party_goombella::g_WeaponTable[move - base_move];
            break;
        case MoveType::KOOPS_BASE:
            weapon = party_koops::g_WeaponTable[move - base_move];
            break;
        case MoveType::FLURRIE_BASE:
            weapon = party_flurrie::g_WeaponTable[move - base_move];
            break;
        case MoveType::YOSHI_BASE:
            weapon = party_yoshi::g_WeaponTable[move - base_move];
            break;
        case MoveType::VIVIAN_BASE:
            weapon = party_vivian::g_WeaponTable[move - base_move];
            break;
        case MoveType::BOBBERY_BASE:
            weapon = party_bobbery::g_WeaponTable[move - base_move];
            break;
        case MoveType::MOWZ_BASE:
            weapon = party_mowz::g_WeaponTable[move - base_move];
            break;
    }

    if (!weapon) {
        evtSetValue(evt, evt->evtArguments[0], 0);
        return 2;
    }

    int32_t level = ai_phase;
    if (move == MoveType::VIVIAN_CURSE || move == MoveType::VIVIAN_NEUTRALIZE) {
        level = 1;
    }

    MoveManager::ResetSelectedLevels();
    MoveManager::ChangeSelectedLevel(move, level - 1);
    evtSetValue(evt, evt->evtArguments[0], PTR(weapon->attack_evt_code));
    unitDoopliss_weaponSelected = weapon;

    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_Doopliss_CheckVeiled) {
    auto* unit = ttyd::battle::BattleGetMarioPtr(ttyd::battle::g_BattleWork);
    if (ttyd::battle_unit::BtlUnit_CheckShadowGuard(unit)) {
        evtSetValue(evt, evt->evtArguments[0], 1);
    } else {
        evtSetValue(evt, evt->evtArguments[0], 0);
    }
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_Doopliss_SpawnScanEffect) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    int32_t id = evtGetValue(evt, evt->evtArguments[0]);
    id = ttyd::battle_sub::BattleTransID(evt, id);
    auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, id);
    
    EffEntry* eff = ttyd::eff_scanning::effScanningEntry(
        evtGetFloat(evt, evt->evtArguments[1]),
        evtGetFloat(evt, evt->evtArguments[2]),
        evtGetFloat(evt, evt->evtArguments[3]), 0);
    *reinterpret_cast<float*>(
        reinterpret_cast<intptr_t>(eff->eff_work) + 0x10) = unit->unk_scale;
    
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_Doopliss_HandleTransform) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    int32_t id = ttyd::battle_sub::BattleTransID(evt, -2);
    auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, id);
    auto* part = ttyd::battle::BattleGetUnitPartsPtr(id, 1);
    auto& snd_table = unit->movement_params.sound_data_table;
    
    BattleUnitKindPart* kind_parts = nullptr;
    int32_t num_parts = 1;

    unit->status_vulnerability = &unitDoopliss_status;
    
    switch (evtGetValue(evt, evt->evtArguments[0])) {
        case BattleUnitType::DOOPLISS_CH_8: {
            kind_parts = unitDoopliss_parts;
            num_parts = 7;

            unit->current_kind = BattleUnitType::DOOPLISS_CH_8;
            unit->data_table = (void*)unitDoopliss_data_table;
            unit->unit_work[UW_Doopliss_CurrentForm] = 0;

            ttyd::battle_unit::BtlUnit_ClearStatus(unit);

            break;
        }
        case BattleUnitType::MARIO: {
            kind_parts = unitDoopliss_parts_Mario;
            num_parts = 3;
            
            unit->current_kind = BattleUnitType::DOOPLISS_CH_8_FAKE_MARIO;
            unit->data_table = (void*)unitDoopliss_data_table_Mario;
            unit->unit_work[UW_Doopliss_CurrentForm] = 1;
            
            snd_table.walk_sound_left = "SFX_BOSS_NM_MOVE1L";
            snd_table.walk_sound_right = "SFX_BOSS_NM_MOVE1R";
            snd_table.walk_initial_wait_timer = 0;
            snd_table.walk_repeat_wait_timer_left = 20;
            snd_table.walk_repeat_wait_timer_right = 20;
            snd_table.run_sound_left = "SFX_BOSS_NM_MOVE1L";
            snd_table.run_sound_right = "SFX_BOSS_NM_MOVE1R";
            snd_table.run_initial_wait_timer = 0;
            snd_table.run_repeat_wait_timer_left = 10;
            snd_table.run_repeat_wait_timer_right = 10;
            
            break;
        }
        case BattleUnitType::GOOMBELLA: {
            kind_parts = unitDoopliss_parts_P1;
            
            unit->current_kind = BattleUnitType::DOOPLISS_CH_8_GOOMBELLA;
            unit->data_table = (void*)unitDoopliss_data_table_P1;
            unit->unit_work[UW_Doopliss_CurrentForm] = 2;
            
            break;
        }
        case BattleUnitType::KOOPS: {
            kind_parts = unitDoopliss_parts_P2;
            
            unit->current_kind = BattleUnitType::DOOPLISS_CH_8_KOOPS;
            unit->data_table = (void*)unitDoopliss_data_table_P2;
            unit->unit_work[UW_Doopliss_CurrentForm] = 3;
            
            break;
        }
        case BattleUnitType::FLURRIE: {
            kind_parts = unitDoopliss_parts_P3;
            
            unit->current_kind = BattleUnitType::DOOPLISS_CH_8_FLURRIE;
            unit->status_vulnerability = &unitDoopliss_status_P3;
            unit->data_table = (void*)unitDoopliss_data_table_P3;
            unit->unit_work[UW_Doopliss_CurrentForm] = 4;
            
            snd_table.walk_sound_left = "SFX_PARTY_BATTLE_HANG1";
            snd_table.walk_sound_right = "SFX_PARTY_BATTLE_HANG1";
            snd_table.walk_initial_wait_timer = 6;
            snd_table.walk_repeat_wait_timer_left = 12;
            snd_table.walk_repeat_wait_timer_right = 16;
            snd_table.run_sound_left = "SFX_PARTY_BATTLE_HANG1";
            snd_table.run_sound_right = "SFX_PARTY_BATTLE_HANG1";
            snd_table.run_initial_wait_timer = 0;
            snd_table.run_repeat_wait_timer_left = 9;
            snd_table.run_repeat_wait_timer_right = 3;
            snd_table.dive_sound_left = "SFX_PARTY_BATTLE_HANG1";
            snd_table.dive_sound_right = "SFX_PARTY_BATTLE_HANG1";
            snd_table.dive_initial_wait_timer = 0;
            snd_table.dive_repeat_wait_timer_left = 9;
            snd_table.dive_repeat_wait_timer_right = 3;
            snd_table.jump_sound_left = "SFX_PARTY_BATTLE_JUMP1";
            snd_table.jump_sound_right = "SFX_PARTY_BATTLE_LANDING1";
            
            break;
        }
        case BattleUnitType::YOSHI: {
            kind_parts = unitDoopliss_parts_P4;
            num_parts = 7;
            
            unit->current_kind = BattleUnitType::DOOPLISS_CH_8_YOSHI;
            unit->data_table = (void*)unitDoopliss_data_table_P4;
            unit->unit_work[UW_Doopliss_CurrentForm] = 5;
            
            break;
        }
        case BattleUnitType::VIVIAN: {
            kind_parts = unitDoopliss_parts_P5;
            
            unit->current_kind = BattleUnitType::DOOPLISS_CH_8_VIVIAN;
            unit->data_table = (void*)unitDoopliss_data_table_P5;
            unit->unit_work[UW_Doopliss_CurrentForm] = 6;
            
            break;
        }
        case BattleUnitType::BOBBERY: {
            kind_parts = unitDoopliss_parts_P6;
            
            unit->current_kind = BattleUnitType::DOOPLISS_CH_8_BOBBERY;
            unit->status_vulnerability = &unitDoopliss_status_P6;
            unit->data_table = (void*)unitDoopliss_data_table_P6;
            unit->unit_work[UW_Doopliss_CurrentForm] = 7;
            
            break;
        }
        case BattleUnitType::MS_MOWZ: {
            kind_parts = unitDoopliss_parts_P7;
            num_parts = 4;
            
            unit->current_kind = BattleUnitType::DOOPLISS_CH_8_MS_MOWZ;
            unit->data_table = (void*)unitDoopliss_data_table_P7;
            unit->unit_work[UW_Doopliss_CurrentForm] = 8;
            
            break;
        }
    }
    
    if (id != BattleUnitType::MARIO && id != BattleUnitType::FLURRIE) {
        snd_table.walk_sound_left = "SFX_PARTY_BATTLE_MOVE1L";
        snd_table.walk_sound_right = "SFX_PARTY_BATTLE_MOVE1R";
        snd_table.walk_initial_wait_timer = 6;
        snd_table.walk_repeat_wait_timer_left = 12;
        snd_table.walk_repeat_wait_timer_right = 16;
        snd_table.run_sound_left = "SFX_PARTY_BATTLE_MOVE1L";
        snd_table.run_sound_right = "SFX_PARTY_BATTLE_MOVE1R";
        snd_table.run_initial_wait_timer = 0;
        snd_table.run_repeat_wait_timer_left = 9;
        snd_table.run_repeat_wait_timer_right = 3;
        snd_table.jump_sound_left = "SFX_PARTY_BATTLE_JUMP1";
        snd_table.jump_sound_right = "SFX_PARTY_BATTLE_LANDING1";
    }
    
    for (int32_t i = 0; i < 7; ++i) {
        // Fill with dummy information if there are no more valid parts.
        auto* current_kind_parts = kind_parts;
        uint32_t attribute_flags = 0x1301'0000;
        if (i < num_parts) {
            current_kind_parts = kind_parts + i;
            attribute_flags = current_kind_parts->attribute_flags;
        }
        
        // Replace parts and pose tables.
        part->kind_part_params = current_kind_parts;
        part->part_name = part->kind_part_params->name;
        part->pose_table = part->kind_part_params->pose_table;
        part->part_attribute_flags = attribute_flags;
        part->counter_attribute_flags = part->kind_part_params->counter_attribute_flags;
        part->defense = part->kind_part_params->defense;
        part->defense_attr = part->kind_part_params->defense_attr;
        part->position_offset.x = part->kind_part_params->part_offset_pos.x;
        part->position_offset.y = part->kind_part_params->part_offset_pos.y;
        part->position_offset.z = part->kind_part_params->part_offset_pos.z;
        part->hit_base_position.x = part->kind_part_params->part_hit_base_offset.x;
        part->hit_base_position.y = part->kind_part_params->part_hit_base_offset.y;
        part->hit_offset.x = 0.0f;
        part->hit_offset.y = 0.0f;
        part->addl_target_offset_x = 0;
        ttyd::battle_unit::BtlUnit_SetPartsRotate(part, 0.0f, 0.0f, 0.0f);
        ttyd::battle_unit::BtlUnit_SetPartsRotateOffset(part, 0.0f, 0.0f, 0.0f);

        // Release the existing animPose.
        ttyd::animdrv::animPoseRelease(part->anim_pose_id);
        
        part = part->next_part;
    }
    
    ttyd::battle_disp::btlDispEntAnime(unit);
    
    unit->unit_flags |= 4;

    // Equip copies of Mario or partner's current badge loadout.
    memset(&unit->badges_equipped, 0, sizeof(ttyd::battle_unit::BadgesEquipped));
    if (unit->current_kind != BattleUnitType::DOOPLISS_CH_8) {
        auto* pouch = ttyd::mario_pouch::pouchGetPtr();
        uint32_t equip_flags = unit->current_kind == BattleUnitType::MARIO ? 2 : 4;
        for (int32_t i = 0; i < 200; ++i) {
            ttyd::battle::_EquipItem(unit, equip_flags, pouch->equipped_badges[i]);
        }
        // If Doopliss already remarked on having Jumpman and Hammerman both on,
        // ignore those badges for the rest of the fight.
        if (unit->unit_work[UW_Doopliss_DisabledNJNH] == 1) {
            unit->badges_equipped.jumpman = 0;
            unit->badges_equipped.hammerman = 0;
        }
    }
    
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_Doopliss_DisableNJNH) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    int32_t id = ttyd::battle_sub::BattleTransID(evt, -2);
    auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, id);

    unit->badges_equipped.jumpman = 0;
    unit->badges_equipped.hammerman = 0;

    return 2;
}

} // namespace mod::tot::custom