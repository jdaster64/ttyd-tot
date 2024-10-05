#include "tot_custom_rel.h"     // For externed units

#include "evt_cmd.h"
#include "patches_battle.h"

#include <gc/types.h>
#include <ttyd/battle.h>
#include <ttyd/battle_camera.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_event_default.h>
#include <ttyd/battle_event_subset.h>
#include <ttyd/battle_sub.h>
#include <ttyd/battle_unit.h>
#include <ttyd/battle_weapon_power.h>
#include <ttyd/eff_vapor_n64.h>
#include <ttyd/effdrv.h>
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
using namespace ::ttyd::evt_eff;
using namespace ::ttyd::evt_snd;
using namespace ::ttyd::evt_sub;

using ::ttyd::battle::g_BattleWork;
using ::ttyd::battle_unit::BattleWorkUnit;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

// Unit work variable definitions.
constexpr const int32_t UW_BattleUnitType = 0;
constexpr const int32_t UW_Scale = 1;
constexpr const int32_t UW_ScaleAnimFrames = 2;

}  // namespace

// Function / USER_FUNC declarations.
EVT_DECLARE_USER_FUNC(evtTot_PiranhaCheckCeiling, 1)
EVT_DECLARE_USER_FUNC(evtTot_PiranhaMemoizeScale, 1)
EVT_DECLARE_USER_FUNC(evtTot_PiranhaScaleUp, 1)
EVT_DECLARE_USER_FUNC(evtTot_PiranhaScaleDown, 1)
EVT_DECLARE_USER_FUNC(evtTot_PiranhaSetAttackStatus, 2)
EVT_DECLARE_USER_FUNC(evtTot_PiranhaSpawnPoisonEffect, 1)

// Unit data.
int8_t unitPiranha_defense[] = { 0, 0, 0, 0, 0 };
int8_t unitPiranha_defense_attr[] = { 0, 0, 0, 0, 0 };
int8_t unitFrostPiranha_defense_attr[] = { 0, 1, 3, 0, 0 };

// Status (rebalanced Putrid/Frost to be more similar to each other).
StatusVulnerability unitPalePiranha_status = {
    110,  90,  60, 100, 100, 100, 100,  80,
    100,  95, 100,  95, 100,  95,  90,  30, 
     60, 100,  90, 100, 100,  95
};
StatusVulnerability unitPutridPiranha_status = {
    105,  80,  50,   0,  90, 100, 100,  70,
    100,  90, 100,  90, 100,  90,  90,  20, 
     40, 100,  80, 100, 100,  95
};
StatusVulnerability unitFrostPiranha_status = {
    105,  80,  50, 100,  90, 100, 100,   0,
    100,  90, 100,  90, 100,  90,  90,  20, 
     40, 100,  80, 100, 100,  95
};
StatusVulnerability unitFirePiranha_status = {
    100,  70,  40, 100,  80, 100, 100,  60,
    100,  85, 100,  85, 100,  85,  70,  10, 
     30, 100,  70, 100, 100,  30
};

PoseTableEntry unitPiranha_pose_table[] = {
    1, "PKF_N_1",
    2, "PKF_Y_1",
    9, "PKF_Y_1",
    5, "PKF_K_1",
    4, "PKF_X_1",
    3, "PKF_X_1",
    28, "PKF_S_1",
    29, "PKF_Q_1",
    30, "PKF_Q_1",
    31, "PKF_S_2",
    39, "PKF_D_1",
    50, "PKF_A_1B",
    40, "PKF_S_1",
    42, "PKF_S_1",
    56, "PKF_X_1",
    57, "PKF_X_1",
    65, "PKF_T_1",
    69, "PKF_S_1",
};
PoseTableEntry unitPiranha_pose_table_ceiling[] = {
    1, "PKF_N_2",
    2, "PKF_Y_2",
    9, "PKF_Y_2",
    5, "PKF_K_2",
    4, "PKF_X_2",
    3, "PKF_X_2",
    28, "PKF_S_3",
    29, "PKF_Q_2",
    30, "PKF_Q_2",
    31, "PKF_S_4",
    39, "PKF_D_2",
    50, "PKF_A_5",
    40, "PKF_S_3",
    42, "PKF_S_3",
    56, "PKF_X_2",
    57, "PKF_X_2",
    65, "PKF_S_3",
    69, "PKF_S_3",
};

// Weapons

BattleWeapon unitPiranha_weaponNormal = {
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
        AttackTargetClass_Flags::ONLY_TARGET_SELECT_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::HAMMERLIKE |
        AttackTargetProperty_Flags::ONLY_FRONT |
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::FREEZE_BREAK,
    .counter_resistance_flags = AttackCounterResistance_Flags::TOP_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances - fill dynamically!
    
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

BattleWeapon unitPiranha_weaponBreath = {
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
        AttackTargetClass_Flags::MULTIPLE_TARGET |
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
    .target_weighting_flags = 0,
        
    // status chances - fill dynamically!
    
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

EVT_BEGIN(unitPiranha_ceiling_fall_ready_event)
    USER_FUNC(btlevtcmd_SetEventCeilFall, -2, 0)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_PAKKUN_MOVE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    BROTHER_EVT()
        USER_FUNC(evtTot_PiranhaScaleDown, -2)
    END_BROTHER()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKF_E_4"))
    WAIT_MSEC(300)
    USER_FUNC(btlevtcmd_OnAttribute, -2, 16777216)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 1, 33554432)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetPos, -2, LW(0), 0, LW(2))
    USER_FUNC(btlevtcmd_SetHomePos, -2, LW(0), 0, LW(2))
    WAIT_MSEC(500)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_PAKKUN_MOVE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    BROTHER_EVT()
        USER_FUNC(evtTot_PiranhaScaleUp, -2)
    END_BROTHER()
    USER_FUNC(btlevtcmd_OffAttribute, -2, 16777216)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, 33554432)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKF_E_1"))
    USER_FUNC(btlevtcmd_SetHitOffset, -2, 1, 0, 0, 0)
    USER_FUNC(btlevtcmd_SetHitCursorOffset, -2, 1, 0, 50, 0)
    USER_FUNC(btlevtcmd_SetStatusIconOffset, -2, 4, 48)
    USER_FUNC(btlevtcmd_SetKissHitOffset, -2, 16, 32, 0)
    USER_FUNC(btlevtcmd_OffAttribute, -2, 2)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, 4194304)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, 2097152)
    USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitPiranha_pose_table))
    WAIT_MSEC(500)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKF_S_1"))
    RETURN()
EVT_END()

EVT_BEGIN(unitPiranha_return_event)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evtTot_PiranhaCheckCeiling, LW(0))
    IF_EQUAL(LW(0), 1)
        USER_FUNC(evt_sub_random, 1, LW(15))
    ELSE()
        SET(LW(15), 0)
    END_IF()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_PAKKUN_MOVE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    BROTHER_EVT()
        USER_FUNC(evtTot_PiranhaScaleDown, -2)
    END_BROTHER()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKF_E_2"))
    WAIT_MSEC(300)
    USER_FUNC(btlevtcmd_OnAttribute, -2, 16777216)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 1, 33554432)
    IF_EQUAL(LW(15), 0)
        USER_FUNC(btlevtcmd_SetEventCeilFall, -2, 0)
        USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_SetPos, -2, LW(0), 0, LW(2))
        USER_FUNC(btlevtcmd_SetHomePos, -2, LW(0), 0, LW(2))
        WAIT_MSEC(500)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_PAKKUN_MOVE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
        BROTHER_EVT()
            USER_FUNC(evtTot_PiranhaScaleUp, -2)
        END_BROTHER()
        USER_FUNC(btlevtcmd_OffAttribute, -2, 16777216)
        USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, 33554432)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKF_E_1"))
        USER_FUNC(btlevtcmd_SetHitOffset, -2, 1, 0, 0, 0)
        USER_FUNC(btlevtcmd_SetHitCursorOffset, -2, 1, 0, 50, 0)
        USER_FUNC(btlevtcmd_SetStatusIconOffset, -2, 4, 48)
        USER_FUNC(btlevtcmd_SetKissHitOffset, -2, 16, 32, 0)
        USER_FUNC(btlevtcmd_OffAttribute, -2, 2)
        USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, 4194304)
        USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, 2097152)
        USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitPiranha_pose_table))
        WAIT_MSEC(500)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKF_S_1"))
    ELSE()
        USER_FUNC(btlevtcmd_SetEventCeilFall, -2, PTR(&unitPiranha_ceiling_fall_ready_event))
        USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_SetPos, -2, LW(0), 130, LW(2))
        USER_FUNC(btlevtcmd_SetHomePos, -2, LW(0), 130, LW(2))
        WAIT_MSEC(500)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_PAKKUN_MOVE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
        BROTHER_EVT()
            USER_FUNC(evtTot_PiranhaScaleUp, -2)
        END_BROTHER()
        USER_FUNC(btlevtcmd_OffAttribute, -2, 16777216)
        USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, 33554432)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKF_E_3"))
        USER_FUNC(btlevtcmd_SetHitOffset, -2, 1, 0, -80, 0)
        USER_FUNC(btlevtcmd_SetHitCursorOffset, -2, 1, 0, -50, 0)
        USER_FUNC(btlevtcmd_SetStatusIconOffset, -2, 4, 10)
        USER_FUNC(btlevtcmd_SetKissHitOffset, -2, 16, -32, 0)
        USER_FUNC(btlevtcmd_OnAttribute, -2, 2)
        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 1, 4194304)
        USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 1, 2097152)
        USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitPiranha_pose_table_ceiling))
        WAIT_MSEC(500)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKF_S_3"))
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitPiranha_spike_counter_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 31)
    WAIT_MSEC(250)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKF_S_1"))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_CheckStatus, -2, 10, LW(3))
    IF_NOT_EQUAL(LW(3), 0)
        ADD(LW(1), 63)
        GOTO(90)
    END_IF()
    USER_FUNC(btlevtcmd_CheckStatus, -2, 11, LW(3))
    IF_NOT_EQUAL(LW(3), 0)
        ADD(LW(1), 37)
        GOTO(90)
    END_IF()
    ADD(LW(1), 50)
    GOTO(90)
LBL(90)
    USER_FUNC(evt_eff, PTR(""), PTR("toge_flush"), 0, LW(0), LW(1), LW(2), 60, 0, 0, 0, 0, 0, 0, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitPiranha_counter_damage_event)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKF_D_1"))
    WAIT_MSEC(1000)
    USER_FUNC(btlevtcmd_CheckDamageCode, -2, 512, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_CheckDamageCode, -2, 2048, LW(0))
        IF_EQUAL(LW(0), 0)
            RETURN()
        END_IF()
    END_IF()
    RUN_CHILD_EVT(unitPiranha_return_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitPiranha_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    RETURN()
EVT_END()

EVT_BEGIN(unitPiranha_normal_attack_event)
    // Set status effects on weapon based on unit type.
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(0))
    USER_FUNC(evtTot_PiranhaSetAttackStatus, LW(0), 0)

    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        WAIT_MSEC(750)
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
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_PAKKUN_MOVE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    BROTHER_EVT()
        USER_FUNC(evtTot_PiranhaScaleDown, -2)
    END_BROTHER()
    USER_FUNC(btlevtcmd_CheckAttribute, -2, 2, LW(15))
    IF_EQUAL(LW(15), 0)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKF_E_2"))
    ELSE()
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKF_E_4"))
    END_IF()
    WAIT_MSEC(300)
    USER_FUNC(btlevtcmd_OnAttribute, -2, 16777216)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 1, 33554432)
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), 45)
    USER_FUNC(btlevtcmd_SetPos, -2, LW(0), 0, LW(2))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    WAIT_MSEC(500)
    USER_FUNC(evt_btl_camera_set_mode, 0, 8)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, LW(3))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    USER_FUNC(btlevtcmd_SetHitOffset, -2, 1, 0, 0, 0)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_PAKKUN_MOVE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    BROTHER_EVT()
        USER_FUNC(evtTot_PiranhaScaleUp, -2)
    END_BROTHER()
    USER_FUNC(btlevtcmd_OffAttribute, -2, 16777216)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, 33554432)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKF_E_1"))
    WAIT_MSEC(150)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKF_A_1A"))
    WAIT_MSEC(200)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_PAKKUN_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKF_A_1B"))
    WAIT_MSEC(200)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKF_A_1C"))
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
    GOTO(98)
LBL(91)
    USER_FUNC(evt_btl_camera_shake_h, 0, 1, 1, 8, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_add_zoom, 0, -50)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    GOTO(98)
LBL(98)
    WAIT_MSEC(500)
    RUN_CHILD_EVT(PTR(&unitPiranha_return_event))
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitPiranha_breath_attack_event)
    // Set status effects on weapon based on unit type.
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(11))
    USER_FUNC(evtTot_PiranhaSetAttackStatus, LW(11), 1)
    
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
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
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_PAKKUN_MOVE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    BROTHER_EVT()
        USER_FUNC(evtTot_PiranhaScaleDown, -2)
    END_BROTHER()
    USER_FUNC(btlevtcmd_CheckAttribute, -2, 2, LW(15))
    IF_EQUAL(LW(15), 0)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKF_E_2"))
    ELSE()
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKF_E_4"))
    END_IF()
    WAIT_MSEC(300)
    USER_FUNC(btlevtcmd_OnAttribute, -2, 16777216)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 1, 33554432)
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetPos, -2, -40, 0, LW(2))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    WAIT_MSEC(500)
    USER_FUNC(evt_btl_camera_set_mode, 0, 8)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, LW(3))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    USER_FUNC(btlevtcmd_SetHitOffset, -2, 1, 0, 0, 0)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_PAKKUN_MOVE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    BROTHER_EVT()
        USER_FUNC(evtTot_PiranhaScaleUp, -2)
    END_BROTHER()
    USER_FUNC(btlevtcmd_OffAttribute, -2, 16777216)
    USER_FUNC(btlevtcmd_OffPartsAttribute, -2, 1, 33554432)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKF_E_1"))
    WAIT_MSEC(500)
    BROTHER_EVT()
        DO(3)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_PAKKUN_MOUTH1"), EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_FRM(20)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_CheckAttribute, -2, 2, LW(15))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKF_S_2"))
    WAIT_MSEC(1000)

    // Sound effects.
    SET(LW(14), -1)
    SWITCH(LW(11))
        CASE_EQUAL((int32_t)BattleUnitType::PUTRID_PIRANHA)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_PAKKUN_POISON1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        CASE_EQUAL((int32_t)BattleUnitType::FROST_PIRANHA)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ITEM_ICE_WIND1"), EVT_NULLPTR, 0, LW(14))
        CASE_EQUAL((int32_t)BattleUnitType::PIRANHA_PLANT)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_KOOPA_FIRE_BREATH1"), EVT_NULLPTR, 0, LW(14))
    END_SWITCH()
    BROTHER_EVT()
        WAIT_MSEC(1500)
        USER_FUNC(evt_snd_sfxoff, LW(14))
    END_BROTHER()

    // Visual effects.
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_GetEnemyBelong, LW(3), LW(10))
        SWITCH(LW(11))
            CASE_EQUAL((int32_t)BattleUnitType::PUTRID_PIRANHA)
                WAIT_MSEC(600)
                IF_EQUAL(LW(10), 0)
                    DO(20)
                        USER_FUNC(evtTot_PiranhaSpawnPoisonEffect, 1)
                        WAIT_FRM(3)
                    WHILE()
                ELSE()
                    DO(20)
                        USER_FUNC(evtTot_PiranhaSpawnPoisonEffect, 0)
                        WAIT_FRM(3)
                    WHILE()
                END_IF()
            CASE_EQUAL((int32_t)BattleUnitType::FROST_PIRANHA)
                USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(3))
                USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(4))
                
                SET(LW(5), LW(3))
                MUL(LW(5), 10)
                ADD(LW(1), LW(5))
                MUL(LW(5), LW(4))
                MUL(LW(5), 1)
                ADD(LW(0), LW(5))
                ADD(LW(2), 20)

                // Ice effect.
                IF_LARGE(LW(4), 0)
                    SET(LW(5), 0)
                ELSE()
                    SET(LW(5), 1)
                END_IF()
                USER_FUNC(
                    evt_eff, 0, PTR("ibuki"),
                    LW(5), LW(0), LW(1), LW(2), 110, 0, 0, 0, 0, 0, 0, 0)
            CASE_EQUAL((int32_t)BattleUnitType::PIRANHA_PLANT)
                WAIT_MSEC(300)
                USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(5))
                SETF(LW(6), FLOAT(40.0))
                MULF(LW(6), LW(5))
                USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), LW(6))
                SETF(LW(6), FLOAT(22.5))
                MULF(LW(6), LW(5))
                ADDF(LW(1), LW(6))
                SUB(LW(2), 5)

                // Reverse effect if facing right.
                USER_FUNC(btlevtcmd_GetFaceDirection, -2, LW(4))
                IF_EQUAL(LW(4), 1)
                    USER_FUNC(patch::battle::evtTot_SetGonbabaBreathDir, 1)
                END_IF()

                // Fire effect.
                MULF(LW(5), FLOAT(3.5))
                USER_FUNC(
                    evt_eff, PTR(""), PTR("gonbaba_breath"), 7,
                    LW(0), LW(1), LW(2), LW(5), 75, 0, 0, 0, 0, 0, 0)

                // Set effect to left-facing by default, after attack.
                WAIT_FRM(100)
                USER_FUNC(patch::battle::evtTot_SetGonbabaBreathDir, 0)
        END_SWITCH()
    END_BROTHER()

    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKF_A_3A"))
    WAIT_MSEC(300)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKF_A_3B"))
    WAIT_MSEC(300)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)

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
    END_IF()
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    GOTO(97)
    LBL(97)
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    IF_NOT_EQUAL(LW(3), -1)
        ADD(LW(10), 1)
        GOTO(10)
    END_IF()
    WAIT_MSEC(1000)
    USER_FUNC(btlevtcmd_CheckAttribute, -2, 2, LW(15))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PKF_A_3C"))
    WAIT_MSEC(200)
    RUN_CHILD_EVT(PTR(&unitPiranha_return_event))
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitPiranha_attack_event)
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_EnemyItemUseCheck, -2, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(LW(0))
            USER_FUNC(btlevtcmd_StartWaitEvent, -2)
            RETURN()
        END_IF()

        // Pale Piranhas only have a standard attack.
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(0))
        IF_EQUAL(LW(0), (int32_t)BattleUnitType::PALE_PIRANHA)
            GOTO(50)
        END_IF()

        USER_FUNC(evt_sub_random, 99, LW(1))
        IF_LARGE_EQUAL(LW(1), 60)
            SET(LW(9), PTR(&unitPiranha_weaponBreath))
            RUN_CHILD_EVT(PTR(&unitPiranha_breath_attack_event))
            GOTO(99)
        END_IF()
    END_IF()
LBL(50)
    SET(LW(9), PTR(&unitPiranha_weaponNormal))
    RUN_CHILD_EVT(PTR(&unitPiranha_normal_attack_event))
LBL(99)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitPiranha_wait_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitPiranha_common_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitPiranha_wait_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitPiranha_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitPiranha_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitPiranha_attack_event))
    USER_FUNC(btlevtcmd_SetHitCursorOffset, -2, 1, 0, 50, 0)
    USER_FUNC(evtTot_PiranhaMemoizeScale, -2)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitPalePiranha_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType,
        (int32_t)BattleUnitType::PALE_PIRANHA)
    RUN_CHILD_EVT(unitPiranha_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitPutridPiranha_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType,
        (int32_t)BattleUnitType::PUTRID_PIRANHA)
    RUN_CHILD_EVT(unitPiranha_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitFrostPiranha_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType,
        (int32_t)BattleUnitType::FROST_PIRANHA)
    RUN_CHILD_EVT(unitPiranha_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitFirePiranha_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType,
        (int32_t)BattleUnitType::PIRANHA_PLANT)
    RUN_CHILD_EVT(unitPiranha_common_init_event)
    RETURN()
EVT_END()

DataTableEntry unitPiranha_data_table[] = {
    25, (void*)unitPiranha_counter_damage_event,
    26, (void*)unitPiranha_counter_damage_event,
    27, (void*)unitPiranha_counter_damage_event,
    28, (void*)unitPiranha_counter_damage_event,
    29, (void*)unitPiranha_counter_damage_event,
    30, (void*)unitPiranha_counter_damage_event,
    31, (void*)unitPiranha_counter_damage_event,
    32, (void*)unitPiranha_counter_damage_event,
    33, (void*)unitPiranha_counter_damage_event,
    34, (void*)unitPiranha_counter_damage_event,
    35, (void*)unitPiranha_counter_damage_event,
    36, (void*)unitPiranha_counter_damage_event,
    37, (void*)unitPiranha_spike_counter_event,
    48, (void*)btldefaultevt_Dummy,
    0, nullptr,
};

BattleUnitKindPart unitPalePiranha_parts = {
    .index = 1,
    .name = "btl_un_monochrome_pakkun",
    .model_name = "c_pakflwr_t",
    .part_offset_pos = { 0.0f, 0.0f, 0.0f },
    .part_hit_base_offset = { 0.0f, 40.0f, 0.0f },
    .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
    .unk_30 = 20,
    .unk_32 = 30,
    .base_alpha = 255,
    .defense = unitPiranha_defense,
    .defense_attr = unitPiranha_defense_attr,
    .attribute_flags = 0x0000'0009,
    .counter_attribute_flags = 0x0000'0001,
    .pose_table = unitPiranha_pose_table,
};

BattleUnitKindPart unitPutridPiranha_parts = {
    .index = 1,
    .name = "btl_un_poison_pakkun",
    .model_name = "c_pakflwr_p",
    .part_offset_pos = { 0.0f, 0.0f, 0.0f },
    .part_hit_base_offset = { 0.0f, 40.0f, 0.0f },
    .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
    .unk_30 = 20,
    .unk_32 = 30,
    .base_alpha = 255,
    .defense = unitPiranha_defense,
    .defense_attr = unitPiranha_defense_attr,
    .attribute_flags = 0x0000'0009,
    .counter_attribute_flags = 0x0000'0001,
    .pose_table = unitPiranha_pose_table,
};

BattleUnitKindPart unitFrostPiranha_parts = {
    .index = 1,
    .name = "btl_un_ice_pakkun",
    .model_name = "c_pakflwr_a",
    .part_offset_pos = { 0.0f, 0.0f, 0.0f },
    .part_hit_base_offset = { 0.0f, 40.0f, 0.0f },
    .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
    .unk_30 = 20,
    .unk_32 = 30,
    .base_alpha = 255,
    .defense = unitPiranha_defense,
    .defense_attr = unitFrostPiranha_defense_attr,
    .attribute_flags = 0x0000'0009,
    .counter_attribute_flags = 0x0000'0001,
    .pose_table = unitPiranha_pose_table,
};

BattleUnitKindPart unitFirePiranha_parts = {
    .index = 1,
    .name = "btl_un_pakkun_flower",
    .model_name = "c_pakflwr",
    .part_offset_pos = { 0.0f, 0.0f, 0.0f },
    .part_hit_base_offset = { 0.0f, 40.0f, 0.0f },
    .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
    .unk_30 = 20,
    .unk_32 = 30,
    .base_alpha = 255,
    .defense = unitPiranha_defense,
    .defense_attr = unitPiranha_defense_attr,
    .attribute_flags = 0x0000'0009,
    .counter_attribute_flags = 0x0000'0001,
    .pose_table = unitPiranha_pose_table,
};

BattleUnitKind unit_PalePiranha = {
    .unit_type = BattleUnitType::PALE_PIRANHA,
    .unit_name = "btl_un_monochrome_pakkun",
    .max_hp = 4,
    .max_fp = 0,
    .danger_hp = 1,
    .peril_hp = 1,
    .level = 11,
    .bonus_exp = 0,
    .bonus_coin = 0,
    .bonus_coin_rate = 100,
    .base_coin = 1,
    .run_rate = 60,
    .pb_soft_cap = 9999,
    .width = 40,
    .height = 48,
    .hit_offset = { 4, 48 },
    .center_offset = { 0.0f, 24.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 20.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 20.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 16.0f, 32.0f, 0.0f },
    .cut_base_offset = { 0.0f, 24.0f, 0.0f },
    .cut_width = 40.0f,
    .cut_height = 48.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_PAKKUN_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitPalePiranha_status,
    .num_parts = 1,
    .parts = &unitPalePiranha_parts,
    .init_evt_code = (void*)unitPalePiranha_init_event,
    .data_table = unitPiranha_data_table,
};

BattleUnitKind unit_PutridPiranha = {
    .unit_type = BattleUnitType::PUTRID_PIRANHA,
    .unit_name = "btl_un_poison_pakkun",
    .max_hp = 8,
    .max_fp = 0,
    .danger_hp = 1,
    .peril_hp = 1,
    .level = 20,
    .bonus_exp = 0,
    .bonus_coin = 0,
    .bonus_coin_rate = 100,
    .base_coin = 1,
    .run_rate = 60,
    .pb_soft_cap = 9999,
    .width = 40,
    .height = 48,
    .hit_offset = { 4, 48 },
    .center_offset = { 0.0f, 24.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 20.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 20.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 16.0f, 32.0f, 0.0f },
    .cut_base_offset = { 0.0f, 24.0f, 0.0f },
    .cut_width = 40.0f,
    .cut_height = 48.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_PAKKUN_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitPutridPiranha_status,
    .num_parts = 1,
    .parts = &unitPutridPiranha_parts,
    .init_evt_code = (void*)unitPutridPiranha_init_event,
    .data_table = unitPiranha_data_table,
};

BattleUnitKind unit_FrostPiranha = {
    .unit_type = BattleUnitType::FROST_PIRANHA,
    .unit_name = "btl_un_ice_pakkun",
    .max_hp = 10,
    .max_fp = 0,
    .danger_hp = 1,
    .peril_hp = 1,
    .level = 26,
    .bonus_exp = 0,
    .bonus_coin = 0,
    .bonus_coin_rate = 100,
    .base_coin = 1,
    .run_rate = 50,
    .pb_soft_cap = 9999,
    .width = 40,
    .height = 48,
    .hit_offset = { 4, 48 },
    .center_offset = { 0.0f, 24.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 20.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 20.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 16.0f, 32.0f, 0.0f },
    .cut_base_offset = { 0.0f, 24.0f, 0.0f },
    .cut_width = 40.0f,
    .cut_height = 48.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_PAKKUN_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitFrostPiranha_status,
    .num_parts = 1,
    .parts = &unitFrostPiranha_parts,
    .init_evt_code = (void*)unitFrostPiranha_init_event,
    .data_table = unitPiranha_data_table,
};

BattleUnitKind unit_PiranhaPlant = {
    .unit_type = BattleUnitType::PIRANHA_PLANT,
    .unit_name = "btl_un_pakkun_flower",
    .max_hp = 15,
    .max_fp = 0,
    .danger_hp = 1,
    .peril_hp = 1,
    .level = 33,
    .bonus_exp = 0,
    .bonus_coin = 0,
    .bonus_coin_rate = 100,
    .base_coin = 1,
    .run_rate = 50,
    .pb_soft_cap = 9999,
    .width = 40,
    .height = 48,
    .hit_offset = { 4, 48 },
    .center_offset = { 0.0f, 24.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 20.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 20.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 16.0f, 32.0f, 0.0f },
    .cut_base_offset = { 0.0f, 24.0f, 0.0f },
    .cut_width = 40.0f,
    .cut_height = 48.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_PAKKUN_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitFirePiranha_status,
    .num_parts = 1,
    .parts = &unitFirePiranha_parts,
    .init_evt_code = (void*)unitFirePiranha_init_event,
    .data_table = unitPiranha_data_table,
};

EVT_DEFINE_USER_FUNC(evtTot_PiranhaCheckCeiling) {
    // Hardcoded to return 0 for now, since ceiling is never enabled.
    evtSetValue(evt, evt->evtArguments[0], 0);
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_PiranhaMemoizeScale) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[0]));
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, unit_idx);
    auto* parts = ttyd::battle_unit::BtlUnit_GetPartsPtr(unit, 1);
    unit->unit_work[UW_Scale] = parts->unk_078;
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_PiranhaScaleUp) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[0]));
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, unit_idx);
    auto* parts = ttyd::battle_unit::BtlUnit_GetPartsPtr(unit, 1);

    if (isFirstCall) {
        unit->unit_work[UW_ScaleAnimFrames] = 0;
    }
    parts->unk_078 = ttyd::system::intplGetValue(
        0.0f, unit->unit_work[UW_Scale], 0, unit->unit_work[UW_ScaleAnimFrames], 15);

    if (unit->unit_work[UW_ScaleAnimFrames]++ < 15) return 0;

    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_PiranhaScaleDown) {
    int32_t unit_idx = ttyd::battle_sub::BattleTransID(
        evt, evtGetValue(evt, evt->evtArguments[0]));
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, unit_idx);
    auto* parts = ttyd::battle_unit::BtlUnit_GetPartsPtr(unit, 1);

    if (isFirstCall) {
        unit->unit_work[UW_ScaleAnimFrames] = 0;
    }
    parts->unk_078 = ttyd::system::intplGetValue(
        unit->unit_work[UW_Scale], 0.0f, 0, unit->unit_work[UW_ScaleAnimFrames], 15);

    if (unit->unit_work[UW_ScaleAnimFrames]++ < 15) return 0;
    
    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_PiranhaSetAttackStatus) {
    int32_t unit_type = evtGetValue(evt, evt->evtArguments[0]);
    int32_t attack_type = evtGetValue(evt, evt->evtArguments[1]);
    BattleWeapon* attack = attack_type == 0
        ? &unitPiranha_weaponNormal : &unitPiranha_weaponBreath;

    attack->element = AttackElement::NORMAL;
    attack->poison_chance = 0;
    attack->poison_strength = 0;
    attack->poison_time = 0;
    attack->freeze_chance = 0;
    attack->freeze_time = 0;
    attack->burn_chance = 0;
    attack->burn_time = 0;
    
    if (attack_type == 1) {
        switch (unit_type) {
            case BattleUnitType::PUTRID_PIRANHA:
                attack->poison_chance = 50;
                attack->poison_strength = 1;
                attack->poison_time = 10;
                break;
            case BattleUnitType::FROST_PIRANHA:
                attack->freeze_chance = 50;
                attack->freeze_time = 2;
                attack->element = AttackElement::ICE;
                break;
            case BattleUnitType::PIRANHA_PLANT:
                attack->burn_chance = 75;
                attack->burn_time = 3;
                attack->element = AttackElement::FIRE;
                break;
        }
    } else if (unit_type == BattleUnitType::FROST_PIRANHA) {
        attack->freeze_chance = 30;
        attack->freeze_time = 2;
    }

    return 2;
}

EVT_DEFINE_USER_FUNC(evtTot_PiranhaSpawnPoisonEffect) {
    const float x_base = evtGetValue(evt, evt->evtArguments[0]) ? 125.0f : -125.0f;
    const float x = x_base + ttyd::system::irand(150) - 75;
    const float y = ttyd::system::irand(70);
    const float z = ttyd::system::irand(80) - 40;
    const float size = 2.5f + ttyd::system::irand(32768) / 32767.0f;

    auto* eff = ttyd::eff_vapor_n64::effVaporN64Entry(x, y, z, size, 0, 15);
    const uint32_t color = 0x207364ffU;
    *(int32_t*)((uintptr_t)eff->eff_work + 0x18) = (color >> 24) & 0xff;
    *(int32_t*)((uintptr_t)eff->eff_work + 0x1c) = (color >> 16) & 0xff;
    *(int32_t*)((uintptr_t)eff->eff_work + 0x20) = (color >> 8)  & 0xff;
    *(int32_t*)((uintptr_t)eff->eff_work + 0x24) = (color >> 0)  & 0xff;
    *(int32_t*)((uintptr_t)eff->eff_work + 0x28) = (color >> 24) & 0xff;
    *(int32_t*)((uintptr_t)eff->eff_work + 0x2c) = (color >> 16) & 0xff;
    *(int32_t*)((uintptr_t)eff->eff_work + 0x30) = (color >> 8)  & 0xff;
    *(int32_t*)((uintptr_t)eff->eff_work + 0x34) = (color >> 0)  & 0xff;

    return 2;
}

}  // namespace mod::tot::custom