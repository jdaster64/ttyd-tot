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
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

// Constants (UW, etc.)
constexpr const int32_t UW_BattleUnitType = 0;

}  // namespace

// Evt / Function declarations.

extern const int32_t unitBristle_init_event[];
extern const int32_t unitDarkBristle_init_event[];
extern const int32_t unitBristle_common_init_event[];
extern const int32_t unitBristle_attack_event[];
extern const int32_t unitBristle_damage_event[];
extern const int32_t unitBristle_wait_event[];
extern const int32_t unitBristle_spiky_counter_front_event[];
extern const int32_t unitBristle_spiky_counter_top_event[];
extern const int32_t unitBristle_flip_event[];
extern const int32_t unitBristle_wakeup_event[];

// Unit data.

int8_t unitBristle_defense[] = { 4, 99, 4, 4, 4 };
int8_t unitBristle_defense_attr[] = { 0, 2, 0, 0, 0 };
int8_t unitBristle_flip_defense[] = { 0, 0, 0, 0, 0 };

StatusVulnerability unitBristle_status = {
    40, 100,  40, 100,  40, 100,   0,  60,
   100,  60, 100,  60, 100, 100,  60,  50,
    40, 100,  60, 100, 100,  95,
};
StatusVulnerability unitDarkBristle_status = {
    20,  80,  20, 100,  20, 100,   0,  40,
   100,  50, 100,  50, 100, 100,  40,  30,
    20, 100,  40, 100, 100,  30,
};

PoseTableEntry unitBristle_pose_table[] = {
    1, "TGD_N_1",
    2, "TGD_I_1",
    9, "TGD_I_1",
    5, "TGD_K_1",
    4, "TGD_X_1",
    3, "TGD_X_1",
    28, "TGD_S_4",
    29, "TGD_Q_1",
    30, "TGD_Q_1",
    31, "TGD_S_2",
    39, "TGD_D_1",
    50, "TGD_A_1",
    40, "TGD_S_4",
    42, "TGD_S_4",
    56, "TGD_X_1",
    57, "TGD_X_1",
    65, "TGD_T_1",
    69, "TGD_S_4",
};
PoseTableEntry unitBristle_flip_pose_table[] = {
    1, "TGD_N_2",
    2, "TGD_I_2",
    9, "TGD_Z_4",
    5, "TGD_K_2",
    4, "TGD_X_2",
    3, "TGD_X_2",
    28, "TGD_S_5",
    29, "TGD_Q_2",
    30, "TGD_Q_2",
    31, "TGD_S_3",
    39, "TGD_D_2",
    56, "TGD_X_2",
    57, "TGD_X_2",
    65, "TGD_S_5",
    69, "TGD_S_5",
};

DataTableEntry unitBristle_data_table[] = {
    13, (void*)unitBristle_flip_event,
    37, (void*)unitBristle_spiky_counter_top_event,
    38, (void*)unitBristle_spiky_counter_front_event,
    39, (void*)unitBristle_spiky_counter_front_event,
    48, (void*)btldefaultevt_Dummy,
    0, nullptr,
};

BattleWeapon unitBristle_weapon = {
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
        AttackTargetProperty_Flags::ONLY_FRONT |
        AttackTargetProperty_Flags::HAMMERLIKE,
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
        AttackCounterResistance_Flags::PREEMPTIVE_SPIKY |
        AttackCounterResistance_Flags::FRONT_SPIKY |
        AttackCounterResistance_Flags::FIERY,
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

BattleUnitKindPart unitBristle_parts[] = {
    {
        .index = 1,
        .name = "btl_un_togedaruma",
        .model_name = "c_togedaruma",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 35.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 45.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitBristle_defense,
        .defense_attr = unitBristle_defense_attr,
        .attribute_flags = 0x0000'0009 | PartsAttribute_Flags::BOMB_FLIPPABLE,
        .counter_attribute_flags =
            PartsCounterAttribute_Flags::TOP_SPIKY |
            PartsCounterAttribute_Flags::PREEMPTIVE_SPIKY |
            PartsCounterAttribute_Flags::FRONT_SPIKY,
        .pose_table = unitBristle_pose_table,
    },
};
BattleUnitKindPart unitDarkBristle_parts[] = {
    {
        .index = 1,
        .name = "btl_un_yamitogedaruma",
        .model_name = "c_togedaruma_b",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 35.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 45.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitBristle_defense,
        .defense_attr = unitBristle_defense_attr,
        .attribute_flags = 0x0000'0009 | PartsAttribute_Flags::BOMB_FLIPPABLE,
        .counter_attribute_flags =
            PartsCounterAttribute_Flags::TOP_SPIKY |
            PartsCounterAttribute_Flags::PREEMPTIVE_SPIKY |
            PartsCounterAttribute_Flags::FRONT_SPIKY,
        .pose_table = unitBristle_pose_table,
    },
};


BattleUnitKind unit_Bristle = {
    .unit_type = BattleUnitType::BRISTLE,
    .unit_name = "btl_un_togedaruma",
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
    .width = 46,
    .height = 40,
    .hit_offset = { 5, 40 },
    .center_offset = { 0.0f, 20.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 23.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 23.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 20.0f, 26.0f, 0.0f },
    .cut_base_offset = { 0.0f, 20.0f, 0.0f },
    .cut_width = 46.0f,
    .cut_height = 40.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_TOGEDA_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitBristle_status,
    .num_parts = 1,
    .parts = unitBristle_parts,
    .init_evt_code = (void*)unitBristle_init_event,
    .data_table = unitBristle_data_table,
};
BattleUnitKind unit_DarkBristle = {
    .unit_type = BattleUnitType::DARK_BRISTLE,
    .unit_name = "btl_un_yamitogedaruma",
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
    .width = 46,
    .height = 40,
    .hit_offset = { 5, 40 },
    .center_offset = { 0.0f, 20.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 23.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 23.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 20.0f, 26.0f, 0.0f },
    .cut_base_offset = { 0.0f, 20.0f, 0.0f },
    .cut_width = 46.0f,
    .cut_height = 40.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_TOGEDA_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitDarkBristle_status,
    .num_parts = 1,
    .parts = unitDarkBristle_parts,
    .init_evt_code = (void*)unitDarkBristle_init_event,
    .data_table = unitBristle_data_table,
};

// Evt definitions.

EVT_BEGIN(unitBristle_attack_event)
    USER_FUNC(btlevtcmd_GetOverTurnCount, -2, LW(0))
    IF_LARGE_EQUAL(LW(0), 1)
        RUN_CHILD_EVT(PTR(&unitBristle_wakeup_event))
        RETURN()
    END_IF()
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
        USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), PTR(&unitBristle_weapon))
        USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, PTR(&unitBristle_weapon), LW(3), LW(4))
    END_IF()
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            RETURN()
        END_IF()
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_WeaponAftereffect, PTR(&unitBristle_weapon))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, PTR(&unitBristle_weapon))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGEDA_MOVE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("TGD_J_1"))
    WAIT_FRM(10)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGEDA_MOVE2"), EVT_NULLPTR, 0, LW(15))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("TGD_A_1"))
    WAIT_MSEC(400)
    USER_FUNC(evt_btl_camera_set_mode, 0, 8)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, LW(3))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(7.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), PTR(&unitBristle_weapon), 256, LW(5))
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
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("TGD_A_1"))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    ADD(LW(2), 5)
    USER_FUNC(evt_snd_sfxoff, LW(15))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGEDA_MOVE3"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 0, 0, FLOAT(0.1))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), FLOAT(60.0), -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGEDA_MOVE4"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("TGD_N_1"))
    WAIT_MSEC(1500)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("TGD_J_1"))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGEDA_MOVE5"), EVT_NULLPTR, 0, EVT_NULLPTR)
    GOTO(98)
LBL(91)
    USER_FUNC(evt_btl_camera_shake_h, 0, 1, 1, 8, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_add_zoom, 0, -50)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), PTR(&unitBristle_weapon))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&unitBristle_weapon), 256, LW(5))
    USER_FUNC(evt_snd_sfxoff, LW(15))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGEDA_MOVE3"), EVT_NULLPTR, 0, EVT_NULLPTR)
    GOTO(98)
LBL(98)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpSetting, -2, 0, FLOAT(5.0), FLOAT(0.2))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 0, -1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("TGD_J_2"))
    WAIT_FRM(15)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("TGD_S_4"))
    WAIT_MSEC(400)
LBL(99)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitBristle_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    RETURN()
EVT_END()

EVT_BEGIN(unitBristle_spiky_counter_front_event)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_TOGEDA_MOVE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_CheckStatus, -2, 17, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("TGD_J_3"))
            WAIT_FRM(60)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("TGD_J_4"))
        ELSE()
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("TGD_J_1"))
            WAIT_FRM(60)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("TGD_J_2"))
        END_IF()
    END_BROTHER()
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_CheckStatus, -2, 10, LW(3))
    IF_NOT_EQUAL(LW(3), 0)
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 53)
        ADD(LW(1), 26)
        GOTO(90)
    END_IF()
    USER_FUNC(btlevtcmd_CheckStatus, -2, 11, LW(3))
    IF_NOT_EQUAL(LW(3), 0)
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 33)
        ADD(LW(1), 16)
        GOTO(90)
    END_IF()
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 43)
    ADD(LW(1), 21)
    GOTO(90)
LBL(90)
    USER_FUNC(evt_eff, PTR(""), PTR("toge_flush"), 0, LW(0), LW(1), LW(2), 60, 0, 0, 0, 0, 0, 0, 0)
    WAIT_FRM(60)
    RETURN()
EVT_END()

EVT_BEGIN(unitBristle_spiky_counter_top_event)
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_CheckStatus, -2, 10, LW(3))
    IF_NOT_EQUAL(LW(3), 0)
        ADD(LW(1), 58)
        GOTO(90)
    END_IF()
    USER_FUNC(btlevtcmd_CheckStatus, -2, 11, LW(3))
    IF_NOT_EQUAL(LW(3), 0)
        ADD(LW(1), 32)
        GOTO(90)
    END_IF()
    ADD(LW(1), 45)
    GOTO(90)
LBL(90)
    USER_FUNC(evt_eff, PTR(""), PTR("toge_flush"), 0, LW(0), LW(1), LW(2), 60, 0, 0, 0, 0, 0, 0, 0)
    WAIT_FRM(60)
    RETURN()
EVT_END()

EVT_BEGIN(unitBristle_flip_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_SetPartsDefenceTable, LW(10), LW(11), PTR(&unitBristle_flip_defense))
    USER_FUNC(btlevtcmd_AnimeSetPoseTable, LW(10), LW(11), PTR(&unitBristle_flip_pose_table))
    USER_FUNC(btlevtcmd_OnStatusFlag, LW(10), 1)
    USER_FUNC(btlevtcmd_GetOverTurnCount, LW(10), LW(12))
    USER_FUNC(btlevtcmd_SetOverTurnCount, LW(10), 2)
    WAIT_FRM(2)
    USER_FUNC(btlevtcmd_snd_se, LW(10), PTR("SFX_ENM_INSIDE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, LW(10), LW(11), PTR("TGD_S_1"))
    IF_SMALL_EQUAL(LW(12), 0)
        SET(LW(12), 0)
        USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 10, 0)
        BROTHER_EVT()
            DO(0)
                IF_LARGE_EQUAL(LW(12), 10)
                    DO_BREAK()
                END_IF()
                ADD(LW(12), 1)
                WAIT_FRM(1)
                USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, -18)
            WHILE()
        END_BROTHER()
    END_IF()
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.7))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 25, -1)
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, LW(10), LW(11), PTR("TGD_S_5"))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetPos, -2, LW(0), 10, LW(2))
    USER_FUNC(btlevtcmd_SetHomePos, -2, LW(0), 10, LW(2))
    RETURN()
EVT_END()

EVT_BEGIN(unitBristle_wakeup_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_GetOverTurnCount, LW(10), LW(0))
    SUB(LW(0), 1)
    USER_FUNC(btlevtcmd_SetOverTurnCount, LW(10), LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_StartWaitEvent, -2)
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_SetPartsDefenceTable, LW(10), LW(11), PTR(&unitBristle_defense))
    USER_FUNC(btlevtcmd_OffStatusFlag, LW(10), 1)
    USER_FUNC(btlevtcmd_AnimeSetPoseTable, LW(10), LW(11), PTR(&unitBristle_pose_table))
    SET(LW(12), 0)
    USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 10, 0)
    BROTHER_EVT()
        DO(0)
            IF_LARGE_EQUAL(LW(12), 10)
                DO_BREAK()
            END_IF()
            ADD(LW(12), 1)
            WAIT_FRM(1)
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, 0, -18)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_snd_se, LW(10), PTR("SFX_ENM_TOGEDA_MOVE6"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(1.0))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 18, -1)
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, LW(10), LW(11), PTR("TGD_S_4"))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetPos, -2, LW(0), 0, LW(2))
    USER_FUNC(btlevtcmd_SetHomePos, -2, LW(0), 0, LW(2))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitBristle_common_wait_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
LBL(0)
    SET(LW(15), 0)
    USER_FUNC(btlevtcmd_CheckStatus, -2, 1, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        ADD(LW(15), 1)
    END_IF()
    USER_FUNC(btlevtcmd_CheckStatus, -2, 2, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        ADD(LW(15), 1)
    END_IF()
    USER_FUNC(btlevtcmd_CheckStatus, -2, 3, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        ADD(LW(15), 1)
    END_IF()
    USER_FUNC(btlevtcmd_CheckStatus, -2, 4, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        ADD(LW(15), 1)
    END_IF()
    USER_FUNC(btlevtcmd_CheckStatus, -2, 5, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        ADD(LW(15), 1)
    END_IF()
    USER_FUNC(btlevtcmd_CheckStatus, -2, 9, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        ADD(LW(15), 1)
    END_IF()
    IF_NOT_EQUAL(LW(15), 0)
        USER_FUNC(btlevtcmd_OffPartsCounterAttribute, -2, 1, 2)
    ELSE()
        USER_FUNC(btlevtcmd_OnPartsCounterAttribute, -2, 1, 2)
    END_IF()
    WAIT_FRM(1)
    GOTO(0)
    RETURN()
EVT_END()

EVT_BEGIN(unitBristle_common_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitBristle_common_wait_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitBristle_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitBristle_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitBristle_attack_event))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitBristle_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::BRISTLE)
    RUN_CHILD_EVT(unitBristle_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitDarkBristle_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::DARK_BRISTLE)
    RUN_CHILD_EVT(unitBristle_common_init_event)
    RETURN()
EVT_END()

}  // namespace mod::tot::custom