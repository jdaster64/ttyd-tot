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
#include <ttyd/evt_map.h>
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
using namespace ::ttyd::evt_map;
using namespace ::ttyd::evt_snd;
using namespace ::ttyd::evt_sub;

using ::ttyd::battle::g_BattleWork;
using ::ttyd::battle_unit::BattleWorkUnit;
using ::ttyd::battle_unit::BattleWorkUnitPart;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

// Constants (UW, etc.)
constexpr const int32_t UW_BattleUnitType = 0;

}  // namespace

// Evt declarations.

extern const int32_t unitCrazeeDayzee_init_event[];
extern const int32_t unitAmazyDayzee_init_event[];
extern const int32_t unitDayzee_common_init_event[];
extern const int32_t unitDayzee_attack_event[];
extern const int32_t unitDayzee_damage_event[];
extern const int32_t unitDayzee_wait_event[];
extern const int32_t unitDayzee_song_event[];
extern const int32_t unitDayzee_flee_event[];
extern const int32_t unitDayzee_twinkle_event[];

// Unit data.

int8_t unitCrazeeDayzee_defense[] = { 0, 0, 0, 0, 0 };
int8_t unitCrazeeDayzee_defense_attr[] = { 0, 0, 0, 0, 0 };
int8_t unitAmazyDayzee_defense[] = { 1, 1, 1, 1, 1 };
int8_t unitAmazyDayzee_defense_attr[] = { 0, 0, 0, 0, 0 };

StatusVulnerability unitCrazeeDayzee_status = {
     95,  30,  30, 100,  50, 100, 100,  30,
    100,  30, 100,  30, 100,  30,  30, 100,
     80, 100,  30, 100, 100,  30,
};
StatusVulnerability unitAmazyDayzee_status = {
     50,  10,  10, 100,  30, 100, 100,  10,
    100,  10, 100,  10, 100,  10,  10, 100,
     60, 100,  10, 100, 100,  10,
};

PoseTableEntry unitCrazeeDayzee_pose_table[] = {
    1, "PAN_N_1",
    2, "PAN_Y_1",
    9, "PAN_Y_1",
    5, "PAN_K_1",
    4, "PAN_X_1",
    3, "PAN_X_1",
    28, "PAN_S_1",
    29, "PAN_Q_1",
    30, "PAN_Q_1",
    31, "PAN_S_1",
    39, "PAN_D_1",
    50, "PAN_A_1",
    42, "PAN_R_1",
    40, "PAN_W_1",
    56, "PAN_X_1",
    57, "PAN_X_1",
    65, "PAN_T_1",
    69, "PAN_S_1",
};

DataTableEntry unitCrazeeDayzee_data_table[] = {
    48, (void*)btldefaultevt_Dummy,
    0, nullptr,
};

BattleWeapon unitCrazeeDayzee_weapon = {
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
        AttackSpecialProperty_Flags::DEFENSE_PIERCING,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = AttackTargetWeighting_Flags::WEIGHTED_RANDOM,
        
    // status chances
    .sleep_chance = 50,
    .sleep_time = 5,
    
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
BattleWeapon unitAmazyDayzee_weapon = {
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
    .damage_function_params = { 20, 0, 0, 0, 0, 0, 0, 0 },
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
        AttackSpecialProperty_Flags::DEFENSE_PIERCING,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = AttackTargetWeighting_Flags::WEIGHTED_RANDOM,
        
    // status chances
    .sleep_chance = 75,
    .sleep_time = 5,
    
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

BattleUnitKindPart unitCrazeeDayzee_parts[] = {
    {
        .index = 1,
        .name = "btl_un_pansy",
        .model_name = "c_pansy",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 29.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 34.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitCrazeeDayzee_defense,
        .defense_attr = unitCrazeeDayzee_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitCrazeeDayzee_pose_table,
    },
};
BattleUnitKindPart unitAmazyDayzee_parts[] = {
    {
        .index = 1,
        .name = "btl_un_twinkling_pansy",
        .model_name = "c_kpansy",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 29.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 34.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitAmazyDayzee_defense,
        .defense_attr = unitAmazyDayzee_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitCrazeeDayzee_pose_table,
    },
};

BattleUnitKind unit_CrazeeDayzee = {
    .unit_type = BattleUnitType::CRAZEE_DAYZEE,
    .unit_name = "btl_un_pansy",
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
    .width = 22,
    .height = 32,
    .hit_offset = { 5, 32 },
    .center_offset = { 0.0f, 16.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 11.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 11.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 6.0f, 20.8f, 0.0f },
    .cut_base_offset = { 0.0f, 16.0f, 0.0f },
    .cut_width = 22.0f,
    .cut_height = 32.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_PANSY_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitCrazeeDayzee_status,
    .num_parts = 1,
    .parts = unitCrazeeDayzee_parts,
    .init_evt_code = (void*)unitCrazeeDayzee_init_event,
    .data_table = unitCrazeeDayzee_data_table,
};
BattleUnitKind unit_AmazyDayzee = {
    .unit_type = BattleUnitType::AMAZY_DAYZEE,
    .unit_name = "btl_un_twinkling_pansy",
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
    .width = 22,
    .height = 32,
    .hit_offset = { 5, 32 },
    .center_offset = { 0.0f, 16.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 11.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 11.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 6.0f, 20.8f, 0.0f },
    .cut_base_offset = { 0.0f, 16.0f, 0.0f },
    .cut_width = 22.0f,
    .cut_height = 32.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_PANSY_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitAmazyDayzee_status,
    .num_parts = 1,
    .parts = unitAmazyDayzee_parts,
    .init_evt_code = (void*)unitAmazyDayzee_init_event,
    .data_table = unitCrazeeDayzee_data_table,
};

// Evt definitions.

EVT_BEGIN(unitDayzee_song_event)
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
    SET(LW(0), 255)
    SET(LW(1), 255)
    DO(0)
        IF_SMALL(LW(0), 192)
            DO_BREAK()
        END_IF()
        // Blend background color to blue or cyan based on attacker.
        IF_EQUAL(LW(10), 0)
            USER_FUNC(evt_map_set_blend, 0, LW(0), LW(0), LW(1), 255)
        ELSE()
            USER_FUNC(evt_map_set_blend, 0, LW(0), LW(1), LW(1), 255)
        END_IF()
        SUB(LW(0), 2)
        WAIT_FRM(1)
    WHILE()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_PANSY_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 50)
    BROTHER_EVT()
        DO(10)
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
            IF_SMALL(LW(15), 0)
                USER_FUNC(evt_eff64, PTR(""), PTR("onpu_n64"), 1, LW(0), LW(1), LW(2), 0, 0, 0, 0, 0, 0, 0, 0)
            ELSE()
                USER_FUNC(evt_eff64, PTR(""), PTR("onpu_n64"), 2, LW(0), LW(1), LW(2), 0, 0, 0, 0, 0, 0, 0, 0)
            END_IF()
            WAIT_FRM(12)
        WHILE()
    END_BROTHER()
    WAIT_FRM(120)
    SET(LW(6), 0)
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
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
            GOTO(97)
            CASE_END()
    END_SWITCH()
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
    SET(LW(0), 192)
    SET(LW(1), 255)
    DO(0)
        IF_LARGE_EQUAL(LW(0), 256)
            DO_BREAK()
        END_IF()
        // Blend background color to blue or cyan based on attacker.
        IF_EQUAL(LW(10), 0)
            USER_FUNC(evt_map_set_blend, 0, LW(0), LW(0), LW(1), 255)
        ELSE()
            USER_FUNC(evt_map_set_blend, 0, LW(0), LW(1), LW(1), 255)
        END_IF()
        ADD(LW(0), 2)
        WAIT_FRM(1)
    WHILE()
    USER_FUNC(evt_map_blend_off, 0)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 69)
    WAIT_MSEC(500)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitDayzee_flee_event)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_PANSY_ESCAPE1_1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PAN_E_1"))
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 180, 0)
    WAIT_MSEC(250)
    USER_FUNC(btlevtcmd_SetPartsBlur, -2, 1, 255, 255, 255, 255, 100, 100, 100, 100, 0)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 1, 0x400'0000)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_PANSY_ESCAPE1_2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.5))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), FLOAT(20.0), -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_PANSY_ESCAPE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(9.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, 300, LW(1), LW(2), 0, -1, 0)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, 0x400'0000)
    WAIT_MSEC(500)
    USER_FUNC(btlevtcmd_KillUnit, -2, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitDayzee_attack_event)
    // Changed; both Crazee and Amazy Dayzees do 1-HP check before item check.
    USER_FUNC(btlevtcmd_GetHp, -2, LW(0))
    IF_SMALL_EQUAL(LW(0), 1)
        GOTO(10)
    END_IF()

    USER_FUNC(btlevtcmd_EnemyItemUseCheck, -2, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(LW(0))
        USER_FUNC(btlevtcmd_StartWaitEvent, -2)
        RETURN()
    END_IF()

    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(0))
    IF_EQUAL(LW(0), (int32_t)BattleUnitType::CRAZEE_DAYZEE)
        USER_FUNC(btlevtcmd_DrawLots, LW(0), 2, 70, 30)
        SET(LW(9), PTR(&unitCrazeeDayzee_weapon))
        SET(LW(10), 0)
    ELSE()
        USER_FUNC(btlevtcmd_DrawLots, LW(0), 2, 20, 80)
        SET(LW(9), PTR(&unitAmazyDayzee_weapon))
        SET(LW(10), 1)
    END_IF()

    IF_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(PTR(&unitDayzee_song_event))
        GOTO(99)
    ELSE()
LBL(10)
        RUN_CHILD_EVT(PTR(&unitDayzee_flee_event))
        GOTO(99)
    END_IF()

LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitDayzee_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    RETURN()
EVT_END()

EVT_BEGIN(unitDayzee_wait_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitDayzee_twinkle_event)
LBL(0)
    WAIT_FRM(40)
    USER_FUNC(btlevtcmd_GetUnitId, -2, LW(0))
    IF_EQUAL(LW(0), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    ADD(LW(1), 20)
    USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(3))
    MULF(LW(1), LW(3))
    ADD(LW(2), 15)
    USER_FUNC(evt_eff64, PTR(""), PTR("stardust_n64"), 2, LW(0), LW(1), LW(2), 0, 0, 0, 0, 0, 0, 0, 0)
    END_IF()
    GOTO(0)
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitDayzee_common_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitDayzee_wait_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitDayzee_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitDayzee_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitDayzee_attack_event))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitCrazeeDayzee_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::CRAZEE_DAYZEE)
    RUN_CHILD_EVT(unitDayzee_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitAmazyDayzee_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::AMAZY_DAYZEE)
    RUN_EVT(PTR(&unitDayzee_twinkle_event))
    RUN_CHILD_EVT(unitDayzee_common_init_event)
    RETURN()
EVT_END()

}  // namespace mod::tot::custom