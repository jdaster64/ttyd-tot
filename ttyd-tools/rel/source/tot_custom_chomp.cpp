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
constexpr const int32_t UW_BoomerangsThrown = 0;
constexpr const int32_t UW_TargetUnit1 = 1;
constexpr const int32_t UW_TargetUnit2 = 2;
constexpr const int32_t UW_BattleUnitType = 15;

}  // namespace

// Evt / Function declarations.

extern const int32_t unitChainChomp_init_event[];
extern const int32_t unitRedChomp_init_event[];
extern const int32_t unitChomp_common_init_event[];
extern const int32_t unitChomp_attack_event[];
extern const int32_t unitChomp_damage_event[];
extern const int32_t unitChomp_dead_event[];
extern const int32_t unitChomp_counter_damage_event[];
extern const int32_t unitChomp_wait_event[];
extern const int32_t unitChomp_chain_event[];
extern const int32_t unitChomp_wait_sound_event1[];
extern const int32_t unitChomp_wait_sound_event2[];
extern const int32_t unitChomp_sound_gensui_event[];

EVT_DECLARE_USER_FUNC(unitChomp_chain_init, 1)
EVT_DECLARE_USER_FUNC(unitChomp_chain_main, 1)

// Unit data.

int8_t unitChainChomp_defense[] = { 5, 99, 99, 5, 5 };
int8_t unitChainChomp_defense_attr[] = { 0, 2, 2, 0, 0 };

// Used for both variants.
StatusVulnerability unitChainChomp_status = {
      0,  70,  65, 100,  65, 100,   0,  70,
    100,  90, 100,  90, 100,  95,  65,  45,
     30, 100,  65, 100, 100,  90,
};

PoseTableEntry unitChainChomp_pose_table[] = {
    1, "WAN_N_1",
    2, "WAN_Y_1",
    9, "WAN_Y_1",
    5, "WAN_K_1",
    4, "WAN_X_1",
    3, "WAN_X_1",
    27, "WAN_Z_1",
    28, "WAN_S_1A",
    29, "WAN_D_1",
    30, "WAN_D_1",
    31, "WAN_A_1",
    39, "WAN_D_1",
    50, "WAN_A_1",
    42, "WAN_R_1",
    40, "WAN_W_1",
    56, "WAN_X_1",
    57, "WAN_X_1",
    65, "WAN_S_1A",
    69, "WAN_S_1A",
};

DataTableEntry unitChainChomp_data_table[] = {
    49, (void*)unitChomp_dead_event,
    25, (void*)unitChomp_counter_damage_event,
    26, (void*)unitChomp_counter_damage_event,
    27, (void*)unitChomp_counter_damage_event,
    28, (void*)unitChomp_counter_damage_event,
    29, (void*)unitChomp_counter_damage_event,
    30, (void*)unitChomp_counter_damage_event,
    31, (void*)unitChomp_counter_damage_event,
    32, (void*)unitChomp_counter_damage_event,
    33, (void*)unitChomp_counter_damage_event,
    34, (void*)unitChomp_counter_damage_event,
    35, (void*)unitChomp_counter_damage_event,
    36, (void*)unitChomp_counter_damage_event,
    48, (void*)btldefaultevt_Dummy,
    0, nullptr,
};

BattleWeapon unitChainChomp_weapon = {
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
        AttackCounterResistance_Flags::FIERY |
        AttackCounterResistance_Flags::ICY |
        AttackCounterResistance_Flags::POISON,
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

BattleUnitKindPart unitChainChomp_parts[] = {
    {
        .index = 1,
        .name = "btl_un_wanwan",
        .model_name = "c_wanwan",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 5.0f, 30.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 40.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitChainChomp_defense,
        .defense_attr = unitChainChomp_defense_attr,
        .attribute_flags = 0x0000'0009 | PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitChainChomp_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_wanwan",
        .model_name = "c_wanwan",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitChainChomp_defense,
        .defense_attr = unitChainChomp_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitChainChomp_pose_table,
    },
    {
        .index = 3,
        .name = "btl_un_wanwan",
        .model_name = "c_wanwan",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitChainChomp_defense,
        .defense_attr = unitChainChomp_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitChainChomp_pose_table,
    },
    {
        .index = 4,
        .name = "btl_un_wanwan",
        .model_name = "c_wanwan",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitChainChomp_defense,
        .defense_attr = unitChainChomp_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitChainChomp_pose_table,
    },
    {
        .index = 5,
        .name = "btl_un_wanwan",
        .model_name = "c_wanwan",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitChainChomp_defense,
        .defense_attr = unitChainChomp_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitChainChomp_pose_table,
    },
    {
        .index = 6,
        .name = "btl_un_wanwan",
        .model_name = "c_wanwan",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitChainChomp_defense,
        .defense_attr = unitChainChomp_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitChainChomp_pose_table,
    },
    {
        .index = 7,
        .name = "btl_un_wanwan",
        .model_name = "c_wanwan",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitChainChomp_defense,
        .defense_attr = unitChainChomp_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitChainChomp_pose_table,
    },
    {
        .index = 8,
        .name = "btl_un_wanwan",
        .model_name = "c_wanwan",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitChainChomp_defense,
        .defense_attr = unitChainChomp_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitChainChomp_pose_table,
    },
    {
        .index = 9,
        .name = "btl_un_wanwan",
        .model_name = "c_wanwan",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitChainChomp_defense,
        .defense_attr = unitChainChomp_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitChainChomp_pose_table,
    },
};
BattleUnitKindPart unitRedChomp_parts[] = {
    {
        .index = 1,
        .name = "btl_un_burst_wanwan",
        .model_name = "c_wanwan_a",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 5.0f, 30.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 40.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitChainChomp_defense,
        .defense_attr = unitChainChomp_defense_attr,
        .attribute_flags = 0x0000'0009 | PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitChainChomp_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_burst_wanwan",
        .model_name = "c_wanwan_a",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitChainChomp_defense,
        .defense_attr = unitChainChomp_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitChainChomp_pose_table,
    },
    {
        .index = 3,
        .name = "btl_un_burst_wanwan",
        .model_name = "c_wanwan_a",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitChainChomp_defense,
        .defense_attr = unitChainChomp_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitChainChomp_pose_table,
    },
    {
        .index = 4,
        .name = "btl_un_burst_wanwan",
        .model_name = "c_wanwan_a",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitChainChomp_defense,
        .defense_attr = unitChainChomp_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitChainChomp_pose_table,
    },
    {
        .index = 5,
        .name = "btl_un_burst_wanwan",
        .model_name = "c_wanwan_a",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitChainChomp_defense,
        .defense_attr = unitChainChomp_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitChainChomp_pose_table,
    },
    {
        .index = 6,
        .name = "btl_un_burst_wanwan",
        .model_name = "c_wanwan_a",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitChainChomp_defense,
        .defense_attr = unitChainChomp_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitChainChomp_pose_table,
    },
    {
        .index = 7,
        .name = "btl_un_burst_wanwan",
        .model_name = "c_wanwan_a",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitChainChomp_defense,
        .defense_attr = unitChainChomp_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitChainChomp_pose_table,
    },
    {
        .index = 8,
        .name = "btl_un_burst_wanwan",
        .model_name = "c_wanwan_a",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitChainChomp_defense,
        .defense_attr = unitChainChomp_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitChainChomp_pose_table,
    },
    {
        .index = 9,
        .name = "btl_un_burst_wanwan",
        .model_name = "c_wanwan_a",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitChainChomp_defense,
        .defense_attr = unitChainChomp_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitChainChomp_pose_table,
    },
};

BattleUnitKind unit_ChainChomp = {
    .unit_type = BattleUnitType::CHAIN_CHOMP,
    .unit_name = "btl_un_wanwan",
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
    .width = 34,
    .height = 36,
    .hit_offset = { 6, 44 },
    .center_offset = { 0.0f, 18.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 17.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 17.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 17.0f, 23.4f, 0.0f },
    .cut_base_offset = { 0.0f, 18.0f, 0.0f },
    .cut_width = 34.0f,
    .cut_height = 36.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_WAN_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitChainChomp_status,
    .num_parts = 9,
    .parts = unitChainChomp_parts,
    .init_evt_code = (void*)unitChainChomp_init_event,
    .data_table = unitChainChomp_data_table,
};
BattleUnitKind unit_RedChomp = {
    .unit_type = BattleUnitType::RED_CHOMP,
    .unit_name = "btl_un_burst_wanwan",
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
    .width = 34,
    .height = 36,
    .hit_offset = { 6, 44 },
    .center_offset = { 0.0f, 18.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 17.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 17.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 17.0f, 23.4f, 0.0f },
    .cut_base_offset = { 0.0f, 18.0f, 0.0f },
    .cut_width = 34.0f,
    .cut_height = 36.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_WAN_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitChainChomp_status,
    .num_parts = 9,
    .parts = unitRedChomp_parts,
    .init_evt_code = (void*)unitRedChomp_init_event,
    .data_table = unitChainChomp_data_table,
};

// Function / USER_FUNC definitions.

EVT_DEFINE_USER_FUNC(unitChomp_chain_main) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[0]));
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, unit_idx);
    if (unit == nullptr) return 0;
    
    auto* body = ttyd::battle_unit::BtlUnit_GetPartsPtr(unit, 1);
    auto* part = ttyd::battle_unit::BtlUnit_GetPartsPtr(unit, 9);
    
    gc::vec3 body_pos, part_pos;
    ttyd::battle_unit::BtlUnit_GetPartsPos(
        body, &body_pos.x, &body_pos.y, &body_pos.z);
    ttyd::battle_unit::BtlUnit_GetPartsPos(
        part, &part_pos.x, &part_pos.y, &part_pos.z);
        
    double total_distance = ttyd::_core_sqrt(
        (body_pos.x - part_pos.x) * (body_pos.x - part_pos.x) +
        (body_pos.y - part_pos.y) * (body_pos.y - part_pos.y) +
        (body_pos.z - part_pos.z) * (body_pos.z - part_pos.z));
    
    // The expected distance between each link.
    double partial_dist = total_distance * 0.125;
    if (partial_dist < 10.0f) partial_dist = 10.0f;
    
    // Parts should lose more height to gravity the closer the Chomp is to home.
    double slack = 300.0f / (partial_dist * partial_dist);
    
    if (body_pos.y > 110.0f) body_pos.y = 110.0f;
    ttyd::battle_unit::BtlUnit_SetPartsPos(
        body, body_pos.x, body_pos.y, body_pos.z);
        
    part = ttyd::battle_unit::BtlUnit_GetPartsPtr(unit, 2);
    ttyd::battle_unit::BtlUnit_SetPartsPos(
        part,
        body_pos.x - 16.0f * unit->movement_params.face_direction,
        body_pos.y + 16.0f,
        body_pos.z);
        
    // Chain parts tug on each other in both directions.
        
    for (int32_t i = 2; i < 8; ++i) {
        auto* part1 = ttyd::battle_unit::BtlUnit_GetPartsPtr(unit, i);
        auto* part2 = ttyd::battle_unit::BtlUnit_GetPartsPtr(unit, i + 1);
        
        gc::vec3 pos1, pos2;
        ttyd::battle_unit::BtlUnit_GetPartsPos(part1, &pos1.x, &pos1.y, &pos1.z);
        ttyd::battle_unit::BtlUnit_GetPartsPos(part2, &pos2.x, &pos2.y, &pos2.z);
        
        pos2.y -= slack;
        
        double x_dist = (pos1.x - pos2.x);
        double y_dist = (pos1.y - pos2.y);
        double z_dist = (pos1.z - pos2.z);
        double inter_part_distance = ttyd::_core_sqrt(
            x_dist * x_dist + y_dist * y_dist + z_dist * z_dist);
            
        double tug_strength = 1.0 - partial_dist / inter_part_distance;
        pos2.x += x_dist * tug_strength;
        pos2.y += y_dist * tug_strength;
        pos2.z += z_dist * tug_strength;
        if (pos2.y < 0.0) pos2.y = 0.0;
        
        ttyd::battle_unit::BtlUnit_SetPartsPos(part2, pos2.x, pos2.y, pos2.z);
    }
    
    for (int32_t i = 9; i > 2; --i) {
        auto* part1 = ttyd::battle_unit::BtlUnit_GetPartsPtr(unit, i);
        auto* part2 = ttyd::battle_unit::BtlUnit_GetPartsPtr(unit, i - 1);
        
        gc::vec3 pos1, pos2;
        ttyd::battle_unit::BtlUnit_GetPartsPos(part1, &pos1.x, &pos1.y, &pos1.z);
        ttyd::battle_unit::BtlUnit_GetPartsPos(part2, &pos2.x, &pos2.y, &pos2.z);
        
        pos2.y -= slack;
        
        double x_dist = (pos1.x - pos2.x);
        double y_dist = (pos1.y - pos2.y);
        double z_dist = (pos1.z - pos2.z);
        double inter_part_distance = ttyd::_core_sqrt(
            x_dist * x_dist + y_dist * y_dist + z_dist * z_dist);
            
        double tug_strength = 1.0 - partial_dist / inter_part_distance;
        pos2.x += x_dist * tug_strength;
        pos2.y += y_dist * tug_strength;
        pos2.z += z_dist * tug_strength;
        if (pos2.y < 0.0) pos2.y = 0.0;
        
        ttyd::battle_unit::BtlUnit_SetPartsPos(part2, pos2.x, pos2.y, pos2.z);
    }
    
    return 2;
}

EVT_DEFINE_USER_FUNC(unitChomp_chain_init) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[0]));
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, unit_idx);
    
    gc::vec3 pos;
    ttyd::battle_unit::BtlUnit_GetPos(unit, &pos.x, &pos.y, &pos.z);
    
    for (int32_t i = 2; i < 8; ++i) {
        auto* part = ttyd::battle_unit::BtlUnit_GetPartsPtr(unit, i);
        ttyd::battle_unit::BtlUnit_SetPartsPos(part, pos.x, pos.y, pos.z);
        ttyd::battle_unit::BtlUnit_SetAnim(part, "WAN_S_1B");
    }
    
    auto* part = ttyd::battle_unit::BtlUnit_GetPartsPtr(unit, 8);
    ttyd::battle_unit::BtlUnit_SetPartsPos(
        part, pos.x - 20.0f * unit->movement_params.face_direction, pos.y, pos.z);
    ttyd::battle_unit::BtlUnit_SetAnim(part, "WAN_S_1B");
    
    part = ttyd::battle_unit::BtlUnit_GetPartsPtr(unit, 9);
    ttyd::battle_unit::BtlUnit_SetPartsPos(
        part, pos.x - 20.0f * unit->movement_params.face_direction, pos.y, pos.z);
    ttyd::battle_unit::BtlUnit_SetAnim(part, "WAN_Z_2");
    
    return 2;
}

// Evt definitions.

EVT_BEGIN(unitChomp_attack_event)
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        WAIT_MSEC(750)
        USER_FUNC(btlevtcmd_GetFirstAttackTarget, LW(3), LW(4))
    ELSE()
        USER_FUNC(btlevtcmd_EnemyItemUseCheck, -2, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(LW(0))
            USER_FUNC(btlevtcmd_StartWaitEvent, -2)
            RETURN()
        END_IF()
        USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
        USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), PTR(&unitChainChomp_weapon))
        USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, PTR(&unitChainChomp_weapon), LW(3), LW(4))
    END_IF()
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            RETURN()
        END_IF()
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_TargetUnit1, 127)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_TargetUnit1, LW(14))
    USER_FUNC(btlevtcmd_WeaponAftereffect, PTR(&unitChainChomp_weapon))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, PTR(&unitChainChomp_weapon))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangePartsFaceDirection, -2, 1, LW(15))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BoomerangsThrown, 0)
    USER_FUNC(btlevtcmd_StopWaitEvent, -2)
    USER_FUNC(evt_btl_camera_set_mode, 0, 11)
    USER_FUNC(evt_btl_camera_set_homing_unitparts, 0, -2, 1, LW(3), LW(4))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("WAN_A_1"))
    DO(2)
        USER_FUNC(btlevtcmd_GetPartsPos, -2, 1, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_JumpPartsSetting, -2, 1, 20, 0, FLOAT(0.7))
        BROTHER_EVT()
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_WAN_WAIT1"), LW(14), 0, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_WAN_WAIT3"), LW(14), 0, EVT_NULLPTR)
            WAIT_FRM(10)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_WAN_WAIT2"), LW(14), 0, EVT_NULLPTR)
        END_BROTHER()
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 1, LW(0), 0, LW(2), 20, -1)
    WHILE()
    BROTHER_EVT()
        WAIT_FRM(20)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("WAN_S_1A"))
    END_BROTHER()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_WAN_ATTACK1"), LW(14), 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetPartsPos, -2, 1, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 40)
    USER_FUNC(btlevtcmd_JumpPartsSetting, -2, 1, 30, 0, FLOAT(0.3))
    USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 1, LW(0), 0, LW(2), 30, -1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("WAN_A_1"))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_WAN_ATTACK2"), LW(14), 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    SUB(LW(1), 10)
    IF_SMALL(LW(1), 0)
        SET(LW(1), 0)
    END_IF()
    USER_FUNC(btlevtcmd_JumpPartsSetting, -2, 1, 0, FLOAT(16.0), FLOAT(0.0))
    USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 1, LW(0), LW(1), 10, 0, -1)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), PTR(&unitChainChomp_weapon), 0, LW(5))
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
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_WAN_WAIT1"), LW(14), 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("WAN_S_1A"))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPartsSetting, -2, 1, 0, FLOAT(8.0), FLOAT(0.0))
    USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 1, LW(0), LW(1), LW(2), 0, -1)
    GOTO(98)
LBL(91)
    USER_FUNC(evt_btl_camera_shake_h, 0, 1, 1, 8, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_add_zoom, 0, -50)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), PTR(&unitChainChomp_weapon))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&unitChainChomp_weapon), 256, LW(5))
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_WAN_WAIT1"), LW(14), 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("WAN_S_1A"))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPartsSetting, -2, 1, 0, FLOAT(8.0), FLOAT(0.25))
    USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 1, LW(0), LW(1), LW(2), 0, -1)
    GOTO(98)
LBL(98)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    WAIT_MSEC(250)
LBL(99)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BoomerangsThrown, 1)
    USER_FUNC(btlevtcmd_ChangePartsFaceDirection, -2, 1, -1)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitChomp_counter_damage_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 39)
    USER_FUNC(btlevtcmd_SetPartsFallAccel, LW(10), 1, FLOAT(1.0))
    USER_FUNC(btlevtcmd_GetPartsPos, LW(10), 1, LW(2), LW(3), LW(4))
    USER_FUNC(btlevtcmd_PartsFaceDirectionSub, LW(10), 1, LW(2), 25)
    ADD(LW(3), 25)
    USER_FUNC(btlevtcmd_DivePartsPosition, LW(10), 1, LW(2), LW(3), LW(4), 5, 0, 0, 0, -1)
    WAIT_FRM(30)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_CheckDamageCode, -2, 0x200, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_CheckDamageCode, -2, 0x800, LW(0))
        IF_EQUAL(LW(0), 0)
            RETURN()
        END_IF()
    END_IF()
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitChomp_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BoomerangsThrown, 1)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_TargetUnit2, 2)
    USER_FUNC(btlevtcmd_OnAttribute, LW(10), 4)
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    USER_FUNC(btlevtcmd_OffAttribute, LW(10), 4)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_TargetUnit2, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitChomp_dead_event)
    SET(LW(0), 48)
    USER_FUNC(btlevtcmd_snd_se, LW(10), PTR("SFX_BTL_ENEMY_DIE1_1"), EVT_NULLPTR, 0, EVT_NULLPTR)
LBL(10)
    WAIT_FRM(1)
    SET(LW(11), 1)
    DO(9)
        USER_FUNC(btlevtcmd_AddPartsRotate, LW(10), LW(11), 0, -8, 0)
        ADD(LW(11), 1)
    WHILE()
    SUB(LW(0), 1)
    IF_LARGE(LW(0), 0)
        GOTO(10)
    END_IF()
    SET(LW(11), 1)
    DO(9)
        USER_FUNC(btlevtcmd_SetPartsRotate, LW(10), LW(11), 0, 0, 0)
        ADD(LW(11), 1)
    WHILE()
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
    USER_FUNC(evt_eff, PTR(""), PTR("star_point"), 0, LW(3), LW(1), LW(2), LW(0), 0, 0, 0, 0, 0, 0, 0)
    SET(LW(11), 1)
    DO(9)
        USER_FUNC(btlevtcmd_SetPartsRotateOffset, LW(10), LW(11), 0, 0, 0)
        ADD(LW(11), 1)
    WHILE()
    SET(LW(0), 30)
LBL(20)
    WAIT_FRM(1)
    SET(LW(11), 1)
    DO(9)
        USER_FUNC(btlevtcmd_AddPartsRotate, LW(10), LW(11), 3, 0, 0)
        ADD(LW(11), 1)
    WHILE()
    SUB(LW(0), 1)
    IF_LARGE(LW(0), 0)
        GOTO(20)
    END_IF()
    USER_FUNC(btlevtcmd_GetBackItem, LW(10))
    USER_FUNC(btlevtcmd_CheckDamageCode, LW(10), 0x400, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_KillUnit, LW(10), 0)
    ELSE()
        USER_FUNC(btlevtcmd_KillUnit, LW(10), 2)
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitChomp_wait_sound_event1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_WAN_WAIT1"), LW(11), 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_WAN_WAIT3"), LW(11), 0, EVT_NULLPTR)
    WAIT_FRM(LW(10))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_WAN_WAIT2"), LW(11), 0, EVT_NULLPTR)
    RETURN()
EVT_END()

EVT_BEGIN(unitChomp_wait_sound_event2)
    WAIT_FRM(LW(10))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_WAN_WAIT4"), LW(11), 0, EVT_NULLPTR)
    RETURN()
EVT_END()

EVT_BEGIN(unitChomp_sound_gensui_event)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_TargetUnit1, LW(11))
    MULF(LW(11), FLOAT(0.7))
    IF_SMALL(LW(11), 32)
        SET(LW(11), 32)
    END_IF()
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_TargetUnit1, LW(11))
    RETURN()
EVT_END()

EVT_BEGIN(unitChomp_wait_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_TargetUnit2, 0)
    USER_FUNC(btlevtcmd_CheckStatus, -2, 1, LW(15))
    IF_NOT_EQUAL(LW(15), 0)
        USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_TargetUnit2, 2)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("WAN_N_1"))
        USER_FUNC(btlevtcmd_SetPartsRotateOffset, -2, 1, 0, 10, 0)
        BROTHER_EVT()
LBL(1)
            RUN_CHILD_EVT(PTR(&unitChomp_sound_gensui_event))
            USER_FUNC(btlevtcmd_SetPartsRotate, -2, 1, 0, 0, 0)
            WAIT_FRM(20)
            DO(10)
                WAIT_FRM(4)
                USER_FUNC(btlevtcmd_AddPartsRotate, -2, 1, 0, 0, 1)
            WHILE()
            DO(10)
                WAIT_FRM(4)
                USER_FUNC(btlevtcmd_AddPartsRotate, -2, 1, 0, 0, -1)
            WHILE()
            GOTO(1)
        END_BROTHER()
        GOTO(98)
    END_IF()
    USER_FUNC(btlevtcmd_CheckStatus, -2, 2, LW(14))
    USER_FUNC(btlevtcmd_CheckStatus, -2, 9, LW(15))
    OR(LW(15), LW(14))
    IF_NOT_EQUAL(LW(15), 0)
        USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_TargetUnit2, 2)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("WAN_Y_1"))
        BROTHER_EVT()
LBL(5)
            RUN_CHILD_EVT(PTR(&unitChomp_sound_gensui_event))
            USER_FUNC(evt_sub_random, 20, LW(0))
            WAIT_FRM(LW(0))
            GOTO(5)
        END_BROTHER()
        GOTO(98)
    END_IF()
    USER_FUNC(btlevtcmd_CheckStatus, -2, 5, LW(13))
    USER_FUNC(btlevtcmd_CheckStatus, -2, 4, LW(14))
    USER_FUNC(btlevtcmd_CheckStatus, -2, 3, LW(15))
    OR(LW(15), LW(13))
    OR(LW(15), LW(14))
    IF_NOT_EQUAL(LW(15), 0)
        BROTHER_EVT()
LBL(3)
            RUN_CHILD_EVT(PTR(&unitChomp_sound_gensui_event))
            USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
            USER_FUNC(evt_sub_random, 20, LW(0))
            WAIT_FRM(LW(0))
            SET(LW(10), 15)
            RUN_EVT(PTR(&unitChomp_wait_sound_event1))
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_JumpPartsSetting, -2, 1, 30, 0, FLOAT(0.25))
            USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 1, LW(0), 0, LW(2), 30, 0)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_WAN_LANDING1"), LW(11), 0, EVT_NULLPTR)
            SET(LW(10), 10)
            RUN_EVT(PTR(&unitChomp_wait_sound_event2))
            USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 5)
            USER_FUNC(btlevtcmd_JumpPartsSetting, -2, 1, 20, 0, FLOAT(0.2))
            USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 1, LW(0), 0, LW(2), 20, 0)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_WAN_LANDING1"), LW(11), 0, EVT_NULLPTR)
            SET(LW(10), 7)
            RUN_EVT(PTR(&unitChomp_wait_sound_event2))
            USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 20)
            USER_FUNC(btlevtcmd_JumpPartsSetting, -2, 1, 15, 0, FLOAT(0.4))
            USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 1, LW(0), 0, LW(2), 15, 0)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_WAN_LANDING1"), LW(11), 0, EVT_NULLPTR)
            SET(LW(10), 7)
            RUN_EVT(PTR(&unitChomp_wait_sound_event2))
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_JumpPartsSetting, -2, 1, 15, 0, FLOAT(0.2))
            USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 1, LW(0), LW(1), LW(2), 15, 0)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_WAN_LANDING1"), LW(11), 0, EVT_NULLPTR)
            GOTO(3)
        END_BROTHER()
        GOTO(98)
    END_IF()
    BROTHER_EVT()
LBL(4)
        RUN_CHILD_EVT(PTR(&unitChomp_sound_gensui_event))
        USER_FUNC(evt_sub_random, 20, LW(0))
        WAIT_FRM(LW(0))
        SET(LW(10), 15)
        RUN_EVT(PTR(&unitChomp_wait_sound_event1))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("WAN_A_1"))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_JumpPartsSetting, -2, 1, 30, 0, FLOAT(0.25))
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 1, LW(0), 0, LW(2), 30, 0)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_WAN_LANDING1"), LW(11), 0, EVT_NULLPTR)
        SET(LW(10), 10)
        RUN_EVT(PTR(&unitChomp_wait_sound_event2))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("WAN_S_1A"))
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 5)
        USER_FUNC(btlevtcmd_JumpPartsSetting, -2, 1, 20, 0, FLOAT(0.2))
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 1, LW(0), 0, LW(2), 20, 0)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_WAN_LANDING1"), LW(11), 0, EVT_NULLPTR)
        SET(LW(10), 7)
        RUN_EVT(PTR(&unitChomp_wait_sound_event2))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("WAN_A_1"))
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 20)
        USER_FUNC(btlevtcmd_JumpPartsSetting, -2, 1, 15, 0, FLOAT(0.4))
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 1, LW(0), 0, LW(2), 15, 0)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_WAN_LANDING1"), LW(11), 0, EVT_NULLPTR)
        SET(LW(10), 7)
        RUN_EVT(PTR(&unitChomp_wait_sound_event2))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("WAN_S_1A"))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_JumpPartsSetting, -2, 1, 15, 0, FLOAT(0.2))
        USER_FUNC(btlevtcmd_JumpPartsPosition, -2, 1, LW(0), LW(1), LW(2), 15, 0)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_WAN_LANDING1"), LW(11), 0, EVT_NULLPTR)
        GOTO(4)
    END_BROTHER()
    GOTO(98)
LBL(98)
LBL(99)
    WAIT_FRM(1)
    GOTO(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitChomp_chain_event)
LBL(0)
    WAIT_FRM(1)
    USER_FUNC(btlevtcmd_GetUnitId, -2, LW(0))
    IF_EQUAL(LW(0), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(unitChomp_chain_main, -2)
    GOTO(0)
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitChomp_common_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitChomp_wait_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitChomp_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitChomp_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitChomp_attack_event))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BoomerangsThrown, 1)
    USER_FUNC(unitChomp_chain_init, -2)
    RUN_EVT(PTR(&unitChomp_chain_event))
    USER_FUNC(btlevtcmd_SetJumpSound, -2, PTR("0"), PTR("SFX_ENM_WAN_LANDING1"))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_TargetUnit1, 127)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitChainChomp_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::CHAIN_CHOMP)
    RUN_CHILD_EVT(unitChomp_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitRedChomp_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::RED_CHOMP)
    RUN_CHILD_EVT(unitChomp_common_init_event)
    RETURN()
EVT_END()

}  // namespace mod::tot::custom