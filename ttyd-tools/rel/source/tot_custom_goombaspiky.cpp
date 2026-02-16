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
using ::ttyd::evtmgr_cmd::evtGetFloat;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

// Constants (UW, etc.)
constexpr const int32_t UW_BattleUnitType = 15;

}  // namespace

// Evt / Function declarations.

extern const int32_t unitSpikyGoomba_init_event[];
extern const int32_t unitHyperSpikyGoomba_init_event[];
extern const int32_t unitSpikyGloomba_init_event[];
extern const int32_t unitSpikyGoomba_common_init_event[];
extern const int32_t unitSpikyGoomba_attack_event[];
extern const int32_t unitSpikyGoomba_damage_event[];
extern const int32_t unitSpikyGoomba_wait_event[];
extern const int32_t unitSpikyGoomba_spiky_counter_event[];

EVT_DECLARE_USER_FUNC(unitSpikyGoomba_get_dir, 5)

// Unit data.

int8_t unitSpikyGoomba_defense[] = { 0, 0, 0, 0, 0 };
int8_t unitSpikyGoomba_defense_attr[] = { 0, 0, 0, 0, 0 };

StatusVulnerability unitSpikyGoomba_status = {
    100, 110, 100, 100, 100, 100, 100, 100,
    100, 100, 100, 100, 100, 100, 100, 110,
    110, 100, 100, 100, 100, 150,
};
StatusVulnerability unitHyperSpikyGoomba_status = {
     80,  90,  80, 100,  80, 100, 100,  80,
    100,  90, 100,  90, 100,  95,  80,  90,
     90, 100,  80, 100, 100,  95,
};
StatusVulnerability unitSpikyGloomba_status = {
     80,  90,  80, 100,  80, 100, 100,  80,
    100,  90, 100,  90, 100,  90,  80,  90,
     90, 100,  80, 100, 100,  80,
};

PoseTableEntry unitSpikyGoomba_pose_table[] = {
    1, "TGK_N_1",
    2, "TGK_Y_1",
    9, "TGK_Y_1",
    5, "TGK_K_1",
    4, "TGK_I_1",
    3, "TGK_I_1",
    28, "TGK_S_1",
    29, "TGK_Q_1",
    30, "TGK_Q_1",
    31, "TGK_S_1",
    39, "TGK_D_1",
    50, "TGK_A_1",
    41, "TGK_R_2",
    42, "TGK_R_1",
    40, "TGK_W_1",
    56, "TGK_I_1",
    57, "TGK_I_1",
    65, "TGK_T_1",
    69, "TGK_S_1",
};

DataTableEntry unitSpikyGoomba_data_table[] = {
    37, (void*)unitSpikyGoomba_spiky_counter_event,
    48, (void*)btldefaultevt_Dummy,
    0, nullptr,
};

BattleWeapon unitSpikyGoomba_weapon = {
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
    .damage_function_params = { 3, 0, 0, 0, 0, 0, 0, 0 },
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
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::FLIPS_SHELLED |
        AttackSpecialProperty_Flags::GROUNDS_WINGED,
    .counter_resistance_flags = 0,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
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
BattleWeapon unitSpikyGoomba_weaponCharge = {
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
    .charge_strength = 6,
    
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

BattleUnitKindPart unitSpikyGoomba_parts[] = {
    {
        .index = 1,
        .name = "btl_un_togekuri",
        .model_name = "c_kuribo",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitSpikyGoomba_defense,
        .defense_attr = unitSpikyGoomba_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = PartsCounterAttribute_Flags::TOP_SPIKY,
        .pose_table = unitSpikyGoomba_pose_table,
    },
};
BattleUnitKindPart unitHyperSpikyGoomba_parts[] = {
    {
        .index = 1,
        .name = "btl_un_hyper_togekuri",
        .model_name = "c_kuribo_h",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitSpikyGoomba_defense,
        .defense_attr = unitSpikyGoomba_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = PartsCounterAttribute_Flags::TOP_SPIKY,
        .pose_table = unitSpikyGoomba_pose_table,
    },
};
BattleUnitKindPart unitSpikyGloomba_parts[] = {
    {
        .index = 1,
        .name = "btl_un_yami_togekuri",
        .model_name = "c_kuribo_y",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitSpikyGoomba_defense,
        .defense_attr = unitSpikyGoomba_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = PartsCounterAttribute_Flags::TOP_SPIKY,
        .pose_table = unitSpikyGoomba_pose_table,
    },
};

BattleUnitKind unit_SpikyGoomba = {
    .unit_type = BattleUnitType::SPIKY_GOOMBA,
    .unit_name = "btl_un_togekuri",
    .max_hp = 5,
    .max_fp = 0,
    .danger_hp = 1,
    .peril_hp = 1,
    .level = 1,
    .bonus_exp = 0,
    .bonus_coin = 0,
    .bonus_coin_rate = 100,
    .base_coin = 0,
    .run_rate = 70,
    .pb_soft_cap = 9999,
    .width = 23,
    .height = 30,
    .hit_offset = { 5, 30 },
    .center_offset = { 0.0f, 15.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 12.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 11.5f, 0.0f, 0.0f },
    .kiss_hit_offset = { 11.5f, 18.0f, 0.0f },
    .cut_base_offset = { 0.0f, 15.0f, 0.0f },
    .cut_width = 23.0f,
    .cut_height = 30.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_TOGEKURI_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitSpikyGoomba_status,
    .num_parts = 1,
    .parts = unitSpikyGoomba_parts,
    .init_evt_code = (void*)unitSpikyGoomba_init_event,
    .data_table = unitSpikyGoomba_data_table,
};
BattleUnitKind unit_HyperSpikyGoomba = {
    .unit_type = BattleUnitType::HYPER_SPIKY_GOOMBA,
    .unit_name = "btl_un_hyper_togekuri",
    .max_hp = 5,
    .max_fp = 0,
    .danger_hp = 1,
    .peril_hp = 1,
    .level = 1,
    .bonus_exp = 0,
    .bonus_coin = 0,
    .bonus_coin_rate = 100,
    .base_coin = 0,
    .run_rate = 70,
    .pb_soft_cap = 9999,
    .width = 23,
    .height = 30,
    .hit_offset = { 5, 30 },
    .center_offset = { 0.0f, 15.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 12.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 11.5f, 0.0f, 0.0f },
    .kiss_hit_offset = { 11.5f, 18.0f, 0.0f },
    .cut_base_offset = { 0.0f, 15.0f, 0.0f },
    .cut_width = 23.0f,
    .cut_height = 30.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_TOGEKURI_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitHyperSpikyGoomba_status,
    .num_parts = 1,
    .parts = unitHyperSpikyGoomba_parts,
    .init_evt_code = (void*)unitHyperSpikyGoomba_init_event,
    .data_table = unitSpikyGoomba_data_table,
};
BattleUnitKind unit_SpikyGloomba = {
    .unit_type = BattleUnitType::SPIKY_GLOOMBA,
    .unit_name = "btl_un_yami_togekuri",
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
    .width = 23,
    .height = 30,
    .hit_offset = { 5, 30 },
    .center_offset = { 0.0f, 15.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 12.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 11.5f, 0.0f, 0.0f },
    .kiss_hit_offset = { 11.5f, 18.0f, 0.0f },
    .cut_base_offset = { 0.0f, 15.0f, 0.0f },
    .cut_width = 23.0f,
    .cut_height = 30.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_TOGEKURI_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitSpikyGloomba_status,
    .num_parts = 1,
    .parts = unitSpikyGloomba_parts,
    .init_evt_code = (void*)unitSpikyGloomba_init_event,
    .data_table = unitSpikyGoomba_data_table,
};

// Function / USER_FUNC definitions.

EVT_DEFINE_USER_FUNC(unitSpikyGoomba_get_dir) {
    const float v0 = evtGetFloat(evt, evt->evtArguments[0]);
    const float v1 = evtGetFloat(evt, evt->evtArguments[1]);
    const float v2 = evtGetFloat(evt, evt->evtArguments[2]);
    const float v3 = evtGetFloat(evt, evt->evtArguments[3]);
    const int32_t v4 = evtGetValue(evt, evt->evtArguments[4]);
    
    if (v2 - v0 == 0.0f && v3 - v1 == 0.0f) {
        evtSetValue(evt, evt->evtArguments[4], v4);
    } else {
        evtSetValue(evt, evt->evtArguments[4], 
            static_cast<int32_t>(
                ttyd::system::angleABf(v2 - v0, v3 - v1, 0.0f, 0.0f)));
    }
    
    return 2;
}

// Evt definitions.

EVT_BEGIN(unitSpikyGoomba_attack_event)
    // Always attack if Charged (even if from an item).
    USER_FUNC(btlevtcmd_CheckStatus, -2, StatusEffectType::CHARGE, LW(0))
    IF_EQUAL(LW(0), 1)
        GOTO(10)
    END_IF()
        
    USER_FUNC(btlevtcmd_EnemyItemUseCheck, -2, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(LW(0))
        USER_FUNC(btlevtcmd_StartWaitEvent, -2)
        RETURN()
    END_IF()

    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(0))
    IF_NOT_EQUAL(LW(0), (int32_t)BattleUnitType::HYPER_SPIKY_GOOMBA)
        GOTO(10)
    END_IF()

    USER_FUNC(btlevtcmd_get_turn, LW(0))
    IF_SMALL_EQUAL(LW(0), 1)
        USER_FUNC(btlevtcmd_DrawLots, LW(0), 2, 25, 75)
    ELSE()
        USER_FUNC(btlevtcmd_DrawLots, LW(0), 2, 25, 25)
    END_IF()
    IF_EQUAL(LW(0), 1)
        SET(LW(9), PTR(&unitSpikyGoomba_weaponCharge))
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
    END_IF()

LBL(10)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.0), FLOAT(0.6))
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), PTR(&unitSpikyGoomba_weapon))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, PTR(&unitSpikyGoomba_weapon), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            RETURN()
        END_IF()
        GOTO(99)
    END_IF()
    USER_FUNC(evt_btl_camera_set_mode, 0, 8)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, LW(3))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    USER_FUNC(btlevtcmd_WeaponAftereffect, PTR(&unitSpikyGoomba_weapon))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, PTR(&unitSpikyGoomba_weapon))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), 80)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(0), LW(2), 1, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(4.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), 0, LW(2), 0, -1, 1)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 43)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    WAIT_MSEC(170)
    USER_FUNC(evt_btl_camera_set_mode, 0, 3)
    USER_FUNC(evt_btl_camera_nomove_y_onoff, 0, 1)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 50)
    USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 15, 0)
    INLINE_EVT()
        USER_FUNC(btlevtcmd_GetPosFloat, -2, LW(5), LW(6), LW(7))
        SET(LW(8), 0)
        DO(28)
            USER_FUNC(btlevtcmd_GetPosFloat, -2, LW(9), LW(10), LW(11))
            USER_FUNC(unitSpikyGoomba_get_dir, LW(5), LW(6), LW(9), LW(10), LW(8))
            USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, LW(8))
            SETF(LW(5), LW(9))
            SETF(LW(6), LW(10))
            WAIT_FRM(1)
        WHILE()
    END_INLINE()
    INLINE_EVT()
        DO(12)
            WAIT_FRM(1)
        WHILE()
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 51)
    END_INLINE()
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    ADD(LW(2), 2)
    SUB(LW(1), 4)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 28, -1)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), PTR(&unitSpikyGoomba_weapon), 256, LW(5))
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
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_KURI_LANDING_MISS1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetDispOffset, -2, 0, FLOAT(-12.0), 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("TGK_D_1"))
    WAIT_FRM(40)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("TGK_A_2"))
    USER_FUNC(btlevtcmd_SetDispOffset, -2, 0, 0, 0)
    USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 12, 0)
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_GetRotate, -2, EVT_NULLPTR, EVT_NULLPTR, LW(6))
        DIV(LW(6), 10)
        MUL(LW(6), -1)
        SET(LW(7), 10)
LBL(10)
        USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, LW(6))
        SUB(LW(7), 1)
        WAIT_FRM(1)
        IF_LARGE_EQUAL(LW(7), 1)
            GOTO(10)
        END_IF()
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("TGK_K_1"))
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    END_BROTHER()
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MoveDirectionAdd, -2, LW(0), -30)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, -1)
    USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 0, 0)
    WAIT_FRM(40)
    GOTO(95)
LBL(91)
    USER_FUNC(evt_btl_camera_shake_h, 0, 1, 1, 8, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_add_zoom, 0, -50)
    DO(4)
        USER_FUNC(btlevtcmd_AddScale, -2, FLOAT(0.08), FLOAT(-0.12), 0)
        WAIT_FRM(1)
    WHILE()
    USER_FUNC(btlevtcmd_SetScale, -2, FLOAT(1.0), FLOAT(1.0), FLOAT(1.0))
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), PTR(&unitSpikyGoomba_weapon))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&unitSpikyGoomba_weapon), 256, LW(5))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 43)
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_GetRotate, -2, EVT_NULLPTR, EVT_NULLPTR, LW(6))
        DIV(LW(6), 10)
        MUL(LW(6), -1)
        SET(LW(7), 10)
LBL(20)
        USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, LW(6))
        SUB(LW(7), 1)
        WAIT_FRM(1)
        IF_LARGE_EQUAL(LW(7), 1)
            GOTO(20)
        END_IF()
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    END_BROTHER()
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    SET(LW(1), 0)
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 40)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 28, -1)
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 20)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 10)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 6, -1)
    GOTO(95)
LBL(95)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_SetMoveSoundDataWork, -2, PTR("SFX_ENM_KURI_MOVE1"), PTR("SFX_ENM_KURI_MOVE2"), 0, 3, 3, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 41)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(5.0))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -2, 0)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitSpikyGoomba_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    RETURN()
EVT_END()

EVT_BEGIN(unitSpikyGoomba_spiky_counter_event)
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_CheckStatus, -2, 10, LW(3))
    IF_NOT_EQUAL(LW(3), 0)
        ADD(LW(1), 50)
        GOTO(90)
    END_IF()
    USER_FUNC(btlevtcmd_CheckStatus, -2, 11, LW(3))
    IF_NOT_EQUAL(LW(3), 0)
        ADD(LW(1), 30)
        GOTO(90)
    END_IF()
    ADD(LW(1), 40)
    GOTO(90)
LBL(90)
    USER_FUNC(evt_eff, PTR(""), PTR("toge_flush"), 0, LW(0), LW(1), LW(2), 60, 0, 0, 0, 0, 0, 0, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitSpikyGoomba_wait_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitSpikyGoomba_common_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitSpikyGoomba_wait_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitSpikyGoomba_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitSpikyGoomba_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitSpikyGoomba_attack_event))
    USER_FUNC(btlevtcmd_SetRunSound, -2, PTR("SFX_ENM_TOGEKURI_MOVE1"), PTR("SFX_ENM_TOGEKURI_MOVE2"), 0, 6, 6)
    USER_FUNC(btlevtcmd_SetWalkSound, -2, PTR("SFX_ENM_TOGEKURI_MOVE1"), PTR("SFX_ENM_TOGEKURI_MOVE2"), 0, 6, 6)
    USER_FUNC(btlevtcmd_SetJumpSound, -2, PTR("SFX_ENM_KURI_JUMP1"), PTR("SFX_ENM_KURI_LANDING1"))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitSpikyGoomba_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::SPIKY_GOOMBA)
    RUN_CHILD_EVT(unitSpikyGoomba_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitHyperSpikyGoomba_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::HYPER_SPIKY_GOOMBA)
    RUN_CHILD_EVT(unitSpikyGoomba_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitSpikyGloomba_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::SPIKY_GLOOMBA)
    RUN_CHILD_EVT(unitSpikyGoomba_common_init_event)
    RETURN()
EVT_END()

}  // namespace mod::tot::custom