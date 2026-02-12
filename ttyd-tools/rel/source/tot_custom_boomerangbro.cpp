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
constexpr const int32_t UW_TargetPart1 = 3;
constexpr const int32_t UW_TargetPart2 = 4;
constexpr const int32_t UW_Target1Hit = 5;
constexpr const int32_t UW_Target2Hit = 6;
constexpr const int32_t UW_GuardChecked = 7;

}  // namespace

// Evt declarations.

extern const int32_t unitBoomerangBro_init_event[];
extern const int32_t unitBoomerangBro_attack_event[];
extern const int32_t unitBoomerangBro_damage_event[];
extern const int32_t unitBoomerangBro_wait_event[];
extern const int32_t unitBoomerangBro_normal_attack_event[];
extern const int32_t unitBoomerangBro_normal_attack_event_noaim[];
extern const int32_t unitBoomerangBro_double_attack_event[];
extern const int32_t unitBoomerangBro_double_attack_event_noaim[];
extern const int32_t unitBoomerangBro_noanim_throw_common_event1[];
extern const int32_t unitBoomerangBro_noanim_throw_common_event2[];
extern const int32_t unitBoomerangBro_boomerang_move_event[];
extern const int32_t unitBoomerangBro_damage_check_event[];
extern const int32_t unitBoomerangBro_damage_check_last_event[];
extern const int32_t unitBoomerangBro_damage_check_nolast_event[];

// Unit data.

int8_t unitBoomerangBro_defense[] = { 1, 1, 1, 1, 1 };
int8_t unitBoomerangBro_defense_attr[] = { 0, 0, 0, 0, 0 };

StatusVulnerability unitBoomerangBro_status = {
    80,  80,  70, 100,  80, 100, 100,  70,
   100,  80, 100,  80, 100,  95,  80,  70,
    60, 100,  80, 100, 100,  95,
};

PoseTableEntry unitBoomerangBro_pose_table[] = {
    1, "BRO_N_1",
    2, "BRO_Y_1",
    9, "BRO_Y_1",
    5, "BRO_K_1",
    4, "BRO_X_1",
    3, "BRO_X_1",
    27, "BRO_D_1",
    28, "BRO_S_1",
    29, "BRO_Q_1",
    30, "BRO_Q_1",
    31, "BRO_S_1",
    39, "BRO_D_1",
    42, "BRO_R_1",
    40, "BRO_W_1",
    56, "BRO_X_1",
    57, "BRO_X_1",
    65, "BRO_T_1",
    69, "BRO_S_1",
};

DataTableEntry unitBoomerangBro_data_table[] = {
    0, nullptr,
};

BattleWeapon unitBoomerangBro_weapon = {
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
        AttackSpecialProperty_Flags::FREEZE_BREAK,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
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

BattleUnitKindPart unitBoomerangBro_parts[] = {
    {
        .index = 1,
        .name = "btl_un_boomerang_bros",
        .model_name = "c_burosu_b",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 35.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 45.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitBoomerangBro_defense,
        .defense_attr = unitBoomerangBro_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitBoomerangBro_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_boomerang_bros",
        .model_name = "c_burosu_b",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitBoomerangBro_defense,
        .defense_attr = unitBoomerangBro_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitBoomerangBro_pose_table,
    },
    {
        .index = 3,
        .name = "btl_un_boomerang_bros",
        .model_name = "c_burosu_b",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitBoomerangBro_defense,
        .defense_attr = unitBoomerangBro_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitBoomerangBro_pose_table,
    },
    {
        .index = 4,
        .name = "btl_un_boomerang_bros",
        .model_name = "c_burosu_b",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitBoomerangBro_defense,
        .defense_attr = unitBoomerangBro_defense_attr,
        .attribute_flags =
            PartsAttribute_Flags::NEVER_TARGETABLE |
            PartsAttribute_Flags::UNK_100_0000 |
            PartsAttribute_Flags::UNK_200_0000 |
            PartsAttribute_Flags::UNK_1000_0000,
        .counter_attribute_flags = 0,
        .pose_table = unitBoomerangBro_pose_table,
    },
};

BattleUnitKind unit_BoomerangBro = {
    .unit_type = BattleUnitType::BOOMERANG_BRO,
    .unit_name = "btl_un_boomerang_bros",
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
    .width = 36,
    .height = 44,
    .hit_offset = { 3, 44 },
    .center_offset = { 0.0f, 22.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 18.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 15.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 10.0f, 28.0f, 0.0f },
    .cut_base_offset = { 0.0f, 22.0f, 0.0f },
    .cut_width = 36.0f,
    .cut_height = 44.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_BOOME_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitBoomerangBro_status,
    .num_parts = 4,
    .parts = unitBoomerangBro_parts,
    .init_evt_code = (void*)unitBoomerangBro_init_event,
    .data_table = unitBoomerangBro_data_table,
};

// Evt definitions.

EVT_BEGIN(unitBoomerangBro_damage_check_event)
    SET(LW(0), 0)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), LW(6), 0, LW(5))
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
    GOTO(99)
LBL(91)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_GuardChecked, LW(8))
    IF_EQUAL(LW(8), 0)
        USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(6))
        USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_GuardChecked, 1)
    END_IF()
    IF_EQUAL(LW(7), 1)
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(6), 256, LW(5))
    ELSE()
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(6), 0, LW(5))
    END_IF()
    SET(LW(0), 1)
    GOTO(99)
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitBoomerangBro_damage_check_last_event)
    IF_EQUAL(LW(0), 1)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_GuardChecked, LW(8))
        IF_EQUAL(LW(8), 0)
            USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(6))
            USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_GuardChecked, 1)
        END_IF()
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(6), 256, LW(5))
    ELSE()
        SET(LW(7), 1)
        RUN_CHILD_EVT(PTR(&unitBoomerangBro_damage_check_event))
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitBoomerangBro_damage_check_nolast_event)
    SET(LW(7), 0)
    RUN_CHILD_EVT(PTR(&unitBoomerangBro_damage_check_event))
    RETURN()
EVT_END()

EVT_BEGIN(unitBoomerangBro_boomerang_move_event)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BOOME_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, LW(15), PTR("BRO_A_1B"))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    ADD(LW(1), 20)
    USER_FUNC(btlevtcmd_SetPartsPos, -2, LW(15), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetPartsRotate, -2, LW(15), 0, 0, -180)
    USER_FUNC(btlevtcmd_SetPartsRotateOffset, -2, LW(15), 0, 0, 0)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, LW(15), 0x100'0000)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, LW(15), 0x200'0000)
    SET(LW(9), 0)
    BROTHER_EVT()
        DO(0)
            WAIT_FRM(1)
            USER_FUNC(btlevtcmd_GetPartsPos, -2, LW(15), LW(10), LW(12), LW(13))
            IF_EQUAL(LW(9), 2)
                USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_GuardChecked, 0)
            END_IF()
            SWITCH(LW(9))
                CASE_EQUAL(0)
                    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_TargetUnit1, LW(3))
                    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_TargetPart1, LW(4))
                    IF_EQUAL(LW(3), -1)
                        SET(LW(9), 1)
                        SWITCH_BREAK()
                    END_IF()
                    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(11), LW(12), LW(13))
                    IF_LARGE_EQUAL(LW(10), LW(11))
                        SWITCH_BREAK()
                    END_IF()
                    RUN_CHILD_EVT(PTR(&unitBoomerangBro_damage_check_nolast_event))
                    SET(LW(9), 1)
                    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Target1Hit, LW(0))
                    CASE_END()
                CASE_EQUAL(1)
                    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_TargetUnit2, LW(3))
                    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_TargetPart2, LW(4))
                    IF_EQUAL(LW(3), -1)
                        SET(LW(9), 2)
                        SWITCH_BREAK()
                    END_IF()
                    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(11), LW(12), LW(13))
                    IF_LARGE_EQUAL(LW(10), LW(11))
                        SWITCH_BREAK()
                    END_IF()
                    RUN_CHILD_EVT(PTR(&unitBoomerangBro_damage_check_nolast_event))
                    SET(LW(9), 2)
                    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Target2Hit, LW(0))
                    CASE_END()
                CASE_EQUAL(2)
                    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_TargetUnit2, LW(3))
                    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_TargetPart2, LW(4))
                    IF_EQUAL(LW(3), -1)
                        SET(LW(9), 3)
                        SWITCH_BREAK()
                    END_IF()
                    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(11), LW(12), LW(13))
                    IF_SMALL(LW(10), LW(11))
                        SWITCH_BREAK()
                    END_IF()
                    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Target2Hit, LW(0))
                    IF_EQUAL(LW(14), 0)
                        RUN_CHILD_EVT(PTR(&unitBoomerangBro_damage_check_nolast_event))
                    ELSE()
                        RUN_CHILD_EVT(PTR(&unitBoomerangBro_damage_check_last_event))
                    END_IF()
                    SET(LW(9), 3)
                    CASE_END()
                CASE_EQUAL(3)
                    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_TargetUnit1, LW(3))
                    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_TargetPart1, LW(4))
                    IF_EQUAL(LW(3), -1)
                        SET(LW(9), 4)
                        SWITCH_BREAK()
                    END_IF()
                    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(11), LW(12), LW(13))
                    IF_SMALL(LW(10), LW(11))
                        SWITCH_BREAK()
                    END_IF()
                    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Target1Hit, LW(0))
                    IF_EQUAL(LW(14), 0)
                        RUN_CHILD_EVT(PTR(&unitBoomerangBro_damage_check_nolast_event))
                    ELSE()
                        RUN_CHILD_EVT(PTR(&unitBoomerangBro_damage_check_last_event))
                    END_IF()
                    SET(LW(9), 4)
                    CASE_END()
                CASE_EQUAL(4)
                    DO_BREAK()
                    CASE_END()
            END_SWITCH()
LBL(10)
        WHILE()
    END_BROTHER()
    BROTHER_EVT()
        DO(0)
            WAIT_FRM(1)
            USER_FUNC(btlevtcmd_AddPartsRotate, -2, LW(15), 0, 0, 64)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BOOME_ATTACK2"), EVT_NULLPTR, 0, LW(8))
    USER_FUNC(btlevtcmd_DivePartsPosition, -2, LW(15), -200, LW(1), LW(2), 40, FLOAT(-20.0), 4, 0, -1)
    USER_FUNC(evt_snd_sfxoff, LW(8))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BOOME_ATTACK3"), EVT_NULLPTR, 0, LW(8))
    USER_FUNC(btlevtcmd_DivePartsPosition, -2, LW(15), LW(0), LW(1), LW(2), 40, FLOAT(20.0), 1, 0, -1)
    USER_FUNC(evt_snd_sfxoff, LW(8))
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, LW(15), 0x100'0000)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, LW(15), 0x200'0000)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BOOME_ATTACK4"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AddUnitWork, -2, UW_BoomerangsThrown, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitBoomerangBro_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    RETURN()
EVT_END()

EVT_BEGIN(unitBoomerangBro_normal_attack_event)
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_GetFirstAttackTarget, LW(3), LW(4))
    ELSE()
        USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
        USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), PTR(&unitBoomerangBro_weapon))
        USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, PTR(&unitBoomerangBro_weapon), LW(3), LW(4))
    END_IF()
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            RETURN()
        END_IF()
        GOTO(99)
    END_IF()
    USER_FUNC(evtTot_CheckSpeciesIsEnemy, LW(3), LW(0))
    IF_EQUAL(LW(0), 1)
        RUN_CHILD_EVT(PTR(&unitBoomerangBro_normal_attack_event_noaim))
        GOTO(99)
    END_IF()
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_WeaponAftereffect, PTR(&unitBoomerangBro_weapon))
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, PTR(&unitBoomerangBro_weapon))
    SET(LW(0), UW_TargetUnit1)
    SET(LW(1), UW_TargetPart1)
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Target1Hit, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Target2Hit, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BoomerangsThrown, 0)
LBL(10)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, LW(0), LW(3))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, LW(1), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(20)
    END_IF()
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    ADD(LW(0), 1)
    ADD(LW(1), 1)
    GOTO(10)
LBL(20)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("BRO_A_2A"))
    WAIT_FRM(27)
    SET(LW(15), 2)
    SET(LW(14), 1)
    SET(LW(6), PTR(&unitBoomerangBro_weapon))
    RUN_EVT(PTR(&unitBoomerangBro_boomerang_move_event))
    DO(0)
        WAIT_FRM(1)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BoomerangsThrown, LW(0))
        IF_LARGE_EQUAL(LW(0), 1)
            DO_BREAK()
        END_IF()
    WHILE()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("BRO_S_1"))
    WAIT_MSEC(500)
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitBoomerangBro_noanim_throw_common_event1)
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, 1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("BRO_A_2A"))
    WAIT_FRM(27)
    SET(LW(15), 2)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BOOME_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, LW(15), PTR("BRO_A_1B"))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    ADD(LW(1), 20)
    USER_FUNC(btlevtcmd_SetPartsPos, -2, LW(15), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetPartsRotate, -2, LW(15), 0, 0, -180)
    USER_FUNC(btlevtcmd_SetPartsRotateOffset, -2, LW(15), 0, 0, 0)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, LW(15), 0x100'0000)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, LW(15), 0x200'0000)
    BROTHER_EVT()
        DO(0)
            WAIT_FRM(1)
            USER_FUNC(btlevtcmd_AddPartsRotate, -2, LW(15), 0, 0, 64)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BOOME_ATTACK2"), EVT_NULLPTR, 0, LW(8))
    USER_FUNC(btlevtcmd_DivePartsPosition, -2, LW(15), 250, 40, 0, 15, FLOAT(-20.0), 4, 0, -1)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    USER_FUNC(btlevtcmd_DivePartsPosition, -2, LW(15), -30, 40, 0, 15, FLOAT(20.0), 1, 0, -1)
    USER_FUNC(evt_snd_sfxoff, LW(8))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BOOME_ATTACK3"), EVT_NULLPTR, 0, LW(8))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    ADD(LW(1), 20)
    USER_FUNC(btlevtcmd_DivePartsPosition, -2, LW(15), LW(0), LW(1), LW(2), 15, FLOAT(-10.0), 1, 0, -1)
    USER_FUNC(evt_snd_sfxoff, LW(8))
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, LW(15), 0x100'0000)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, LW(15), 0x200'0000)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_BOOME_ATTACK4"), EVT_NULLPTR, 0, EVT_NULLPTR)
    RETURN()
EVT_END()

// LW(15) should be set to the number of hits; 2 for single toss, 4 for double.
EVT_BEGIN(unitBoomerangBro_noanim_throw_common_event2)
    SET(LW(14), 1)
    DO(0)
        IF_LARGE(LW(14), LW(15))
            DO_BREAK()
        END_IF()

        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_GuardChecked, LW(13))
        IF_SMALL(LW(13), LW(14))
            USER_FUNC(btlevtcmd_ResultACDefence, LW(3), PTR(&unitBoomerangBro_weapon))
            USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_GuardChecked, LW(14))
        END_IF()

        // Last hit should be marked as lethal.
        IF_EQUAL(LW(14), LW(15))
            USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&unitBoomerangBro_weapon), 256, LW(5))
        ELSE()
            USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&unitBoomerangBro_weapon), 0, LW(5))
        END_IF()

        ADD(LW(14), 1)
        IF_EQUAL(LW(14), 3)
            WAIT_FRM(32 + 28)
        ELSE()
            WAIT_FRM(32)
        END_IF()
    WHILE()
    RETURN()
EVT_END()

EVT_BEGIN(unitBoomerangBro_normal_attack_event_noaim)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_WeaponAftereffect, PTR(&unitBoomerangBro_weapon))
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, PTR(&unitBoomerangBro_weapon))
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    SET(LW(10), 0)
    RUN_CHILD_EVT(unitBoomerangBro_noanim_throw_common_event1)
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("BRO_S_1"))
    END_BROTHER()
LBL(0)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), PTR(&unitBoomerangBro_weapon), 256, LW(5))
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
    // Reworked to check guards on each hit separately, and at consistent timing.
    BROTHER_EVT()
        SET(LW(15), 2)
        RUN_CHILD_EVT(unitBoomerangBro_noanim_throw_common_event2)
    END_BROTHER()
    GOTO(97)
LBL(97)
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    IF_NOT_EQUAL(LW(3), -1)
        GOTO(0)
    END_IF()
LBL(99)
    WAIT_FRM(100)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitBoomerangBro_double_attack_event)
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), PTR(&unitBoomerangBro_weapon))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, PTR(&unitBoomerangBro_weapon), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            RETURN()
        END_IF()
        GOTO(99)
    END_IF()
    USER_FUNC(evtTot_CheckSpeciesIsEnemy, LW(3), LW(0))
    IF_EQUAL(LW(0), 1)
        RUN_CHILD_EVT(PTR(&unitBoomerangBro_double_attack_event_noaim))
        GOTO(99)
    END_IF()
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_WeaponAftereffect, PTR(&unitBoomerangBro_weapon))
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, PTR(&unitBoomerangBro_weapon))
    SET(LW(0), UW_TargetUnit1)
    SET(LW(1), UW_TargetPart1)
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Target1Hit, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Target2Hit, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BoomerangsThrown, 0)
LBL(10)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, LW(0), LW(3))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, LW(1), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(20)
    END_IF()
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    ADD(LW(0), 1)
    ADD(LW(1), 1)
    GOTO(10)
LBL(20)
    SET(LW(6), PTR(&unitBoomerangBro_weapon))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("BRO_A_2A"))
    WAIT_FRM(27)
    SET(LW(15), 2)
    SET(LW(14), 0)
    RUN_EVT(PTR(&unitBoomerangBro_boomerang_move_event))
    WAIT_MSEC(750)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("BRO_A_2A"))
    WAIT_FRM(27)
    SET(LW(15), 4)
    SET(LW(14), 1)
    RUN_EVT(PTR(&unitBoomerangBro_boomerang_move_event))
    DO(0)
        WAIT_FRM(1)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BoomerangsThrown, LW(0))
        IF_LARGE_EQUAL(LW(0), 2)
            DO_BREAK()
        END_IF()
    WHILE()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("BRO_S_1"))
    WAIT_MSEC(500)
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitBoomerangBro_double_attack_event_noaim)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_WeaponAftereffect, PTR(&unitBoomerangBro_weapon))
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, PTR(&unitBoomerangBro_weapon))
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    SET(LW(10), 0)
    // Run once synchronously, then once asynchronously so guard timing
    // makes sense with the double throw animation.
    RUN_CHILD_EVT(unitBoomerangBro_noanim_throw_common_event1)
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("BRO_S_1"))
        WAIT_FRM(28)
        RUN_CHILD_EVT(unitBoomerangBro_noanim_throw_common_event1)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("BRO_S_1"))
    END_BROTHER()
LBL(0)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), PTR(&unitBoomerangBro_weapon), 256, LW(5))
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
    // Reworked to check guards on each hit separately, and at consistent timing.
    BROTHER_EVT()
        SET(LW(15), 4)
        RUN_CHILD_EVT(unitBoomerangBro_noanim_throw_common_event2)
    END_BROTHER()
    GOTO(97)
LBL(97)
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    IF_NOT_EQUAL(LW(3), -1)
        GOTO(0)
    END_IF()
LBL(99)
    WAIT_FRM(160)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitBoomerangBro_attack_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_GuardChecked, 0)
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(PTR(&unitBoomerangBro_normal_attack_event))
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_EnemyItemUseCheck, -2, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(LW(0))
        USER_FUNC(btlevtcmd_StartWaitEvent, -2)
        RETURN()
    END_IF()
    USER_FUNC(evtTot_GetPercentOfMaxHP, -2, LW(0))
    IF_SMALL_EQUAL(LW(0), 50)
        USER_FUNC(evt_sub_random, 99, LW(1))
        IF_SMALL(LW(1), 30)
            RUN_CHILD_EVT(PTR(&unitBoomerangBro_double_attack_event))
            GOTO(99)
        END_IF()
    END_IF()
    RUN_CHILD_EVT(PTR(&unitBoomerangBro_normal_attack_event))
    GOTO(99)
LBL(99)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitBoomerangBro_wait_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitBoomerangBro_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitBoomerangBro_wait_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitBoomerangBro_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitBoomerangBro_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitBoomerangBro_attack_event))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

}  // namespace mod::tot::custom