#include "tot_custom_rel.h"     // For externed units

#include "evt_cmd.h"

#include <gc/types.h>
#include <ttyd/battle_camera.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_event_default.h>
#include <ttyd/battle_event_subset.h>
#include <ttyd/battle_weapon_power.h>
#include <ttyd/evt_eff.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evt_sub.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>

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

using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

}  // namespace

// Unit work variable definitions.
constexpr const int32_t UW_BattleUnitType = 0;

// Unit data.
int8_t unitCleft_defense[] = { 1, 99, 1, 1, 1 };
int8_t unitCleft_defense_turn[] = { 0, 0, 0, 0, 0 };
int8_t unitCleft_defense_attr[] = { 0, 2, 0, 0, 0 };
// Immune to fire/ice/elec, only takes 1 damage from other attacks.
int8_t unitIronCleft_defense_attr[] = { 4, 2, 2, 4, 2 };

StatusVulnerability unitCleft_status = {
     50, 100,  50, 100,  50, 100,   0,  70, 
    100,  70, 100,  70, 100, 100,  70,  50, 
     70, 100,  70, 100, 100,  95,
};

StatusVulnerability unitHyperCleft_status = {
     40,  90,  40, 100,  40, 100,   0,  60, 
    100,  65, 100,  65, 100, 100,  60,  40, 
     30, 100,  70, 100, 100,  95,
};

StatusVulnerability unitMoonCleft_status = {
     30,  80,  30, 100,  30, 100,   0,  50, 
    100,  60, 100,  60, 100, 100,  50,  30, 
     20, 100,  50, 100, 100,  95,
};

// Immune to all negative statuses except OHKO (50%).
StatusVulnerability unitIronCleft_status = {
      0,   0,   0,   0,   0, 100,   0,   0,
    100,   0, 100,   0, 100,   0,   0,   0,
      0, 100,   0, 100, 100,  50,
};

PoseTableEntry unitCleft_pose_table[] = {
    1, "SIN_N_1",
    2, "SIN_Y_1",
    9, "SIN_Y_1",
    5, "SIN_K_1",
    4, "SIN_I_1",
    3, "SIN_I_1",
    28, "SIN_S_1",
    29, "SIN_Q_1",
    30, "SIN_Q_1",
    31, "SIN_A_1a",
    39, "SIN_D_1",
    50, "SIN_A_1a",
    42, "SIN_R_1",
    40, "SIN_W_1",
    56, "SIN_I_1",
    57, "SIN_I_1",
    65, "SIN_T_1",
    69, "SIN_S_1",
};

PoseTableEntry unitCleft_pose_table_turn[] = {
    1, "SIN_N_2",
    2, "SIN_Y_2",
    9, "SIN_Y_2",
    5, "SIN_K_2",
    4, "SIN_K_2",
    3, "SIN_K_2",
    28, "SIN_D_2",
    29, "SIN_D_2",
    30, "SIN_D_2",
    31, "SIN_D_2",
    39, "SIN_D_2",
    56, "SIN_K_2",
    57, "SIN_K_2",
    65, "SIN_D_2",
    69, "SIN_D_2",
};

BattleWeapon unitCleft_weapon = {
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
        AttackTargetClass_Flags::ONLY_TARGET_SELECT_PARTS |
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
        AttackCounterResistance_Flags::PREEMPTIVE_SPIKY |
        AttackCounterResistance_Flags::FRONT_SPIKY |
        AttackCounterResistance_Flags::FIERY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::PREFER_FRONT |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000,
        
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

BattleWeapon unitIronCleft_weapon = {
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
        AttackTargetClass_Flags::ONLY_TARGET_SELECT_PARTS |
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
        AttackSpecialProperty_Flags::DEFENSE_PIERCING |
        AttackSpecialProperty_Flags::FREEZE_BREAK,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::PREFER_FRONT |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000,
        
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

BattleWeapon unitCleft_weaponCharge = {
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
        AttackTargetProperty_Flags::TARGET_SAME_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::IGNORES_STATUS_CHANCE |
        AttackSpecialProperty_Flags::CANNOT_MISS |
        // To prevent storing guards / Superguards if Infatuated.
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

// EVTs...

EVT_BEGIN(unitCleft_flip_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_SetPartsDefenceTable, LW(10), LW(11), PTR(&unitCleft_defense_turn))
    USER_FUNC(btlevtcmd_AnimeSetPoseTable, LW(10), LW(11), PTR(&unitCleft_pose_table_turn))
    USER_FUNC(btlevtcmd_OnStatusFlag, LW(10), 1)
    USER_FUNC(btlevtcmd_SetOverTurnCount, LW(10), 2)
    WAIT_FRM(2)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_INSIDE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, LW(10), LW(11), PTR("SIN_D_2"))
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.7))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 15, -1)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 10, -1)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetPos, -2, LW(0), 7, LW(2))
    USER_FUNC(btlevtcmd_SetHomePos, -2, LW(0), 7, LW(2))
    USER_FUNC(btlevtcmd_SetHitOffset, LW(10), LW(11), 0, -10, 0)
    USER_FUNC(btlevtcmd_OffPartsCounterAttribute, -2, 1, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitCleft_spike_counter_event)
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_CheckStatus, -2, 10, LW(3))
    IF_NOT_EQUAL(LW(3), 0)
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 15)
        ADD(LW(1), 45)
        GOTO(90)
    END_IF()
    USER_FUNC(btlevtcmd_CheckStatus, -2, 11, LW(3))
    IF_NOT_EQUAL(LW(3), 0)
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 7)
        ADD(LW(1), 25)
        GOTO(90)
    END_IF()
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 10)
    ADD(LW(1), 35)
LBL(90)
    USER_FUNC(evt_eff, PTR(""), PTR("toge_flush"), 0, LW(0), LW(1), LW(2), 60, 0, 0, 0, 0, 0, 0, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitCleft_damage_event)
    USER_FUNC(btlevtcmd_GetDamage, -2, LW(15))
    IF_LARGE_EQUAL(LW(15), 1)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("SIN_D_1"))
    END_IF()
    USER_FUNC(btlevtcmd_GetOverTurnCount, -2, LW(0))
    IF_LARGE_EQUAL(LW(0), 1)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("SIN_D_2"))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 10, -1)
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 5, -1)
    END_IF()
    SET(LW(10), -2)
    SET(LW(11), 1)
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    RETURN()
EVT_END()

EVT_BEGIN(unitCleft_wakeup_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_GetOverTurnCount, LW(10), LW(0))
    SUB(LW(0), 1)
    USER_FUNC(btlevtcmd_SetOverTurnCount, LW(10), LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_StartWaitEvent, -2)
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_SetPartsDefenceTable, LW(10), LW(11), PTR(&unitCleft_defense))
    USER_FUNC(btlevtcmd_OffStatusFlag, LW(10), 1)
    USER_FUNC(btlevtcmd_AnimeSetPoseTable, LW(10), LW(11), PTR(&unitCleft_pose_table))
    USER_FUNC(btlevtcmd_AnimeChangePose, LW(10), LW(11), PTR("SIN_S_1"))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetPos, -2, LW(0), 0, LW(2))
    USER_FUNC(btlevtcmd_SetHomePos, -2, LW(0), 0, LW(2))
    USER_FUNC(btlevtcmd_SetHitOffset, LW(10), LW(11), 0, 0, 0)
    USER_FUNC(btlevtcmd_OnPartsCounterAttribute, -2, 1, 1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_INSIDE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(1.0))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitCleft_charge_event)
    SET(LW(9), PTR(&unitCleft_weaponCharge))
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
EVT_END()

EVT_BEGIN(unitCleft_normal_attack_event)
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
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), 100)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(0), LW(2), 1, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(4.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), 0, LW(2), 0, -1, 1)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    BROTHER_EVT_ID(LW(15))
        DO(0)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_EMON_MOVE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_FRM(7)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_EMON_MOVE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_FRM(7)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_EMON_MOVE3"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 50)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    WAIT_MSEC(750)
    USER_FUNC(evt_btl_camera_set_mode, 0, 3)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_EMON_DASH1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(5.0))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetPos, -2, EVT_NULLPTR, LW(5), EVT_NULLPTR)
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(5), LW(2), 0, -1, 0)
    DELETE_EVT(LW(15))
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
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 50)
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), 0, LW(2), 0, -1, 0)
    WAIT_MSEC(500)
    GOTO(98)
LBL(91)
    USER_FUNC(evt_btl_camera_shake_h, 0, 1, 1, 8, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_add_zoom, 0, -50)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(1.0), FLOAT(0.2))
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 75)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 20, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_EMON_MOVE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.5), FLOAT(0.2))
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 100)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 20, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_EMON_MOVE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
LBL(98)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(4.0))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitCleft_slow_attack_event)
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
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    BROTHER_EVT_ID(LW(14))
        DO(0)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_ISIN_MOVE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_FRM(7)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_ISIN_MOVE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_FRM(7)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_ISIN_MOVE3"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 50)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 300)
    WAIT_MSEC(750)
    DELETE_EVT(LW(14))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.0))
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    SUB(LW(2), FLOAT(30.0))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), 25)
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), 0, LW(2), 0, -1, 0)
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
    USER_FUNC(btlevtcmd_FaceDirectionSub, LW(3), LW(0), 75)
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), 0, LW(2), 0, -1, 0)
    WAIT_MSEC(500)
    GOTO(98)
LBL(91)
    USER_FUNC(evt_btl_camera_noshake, 0)
    USER_FUNC(evt_btl_camera_shake_h, 0, 1, 1, 8, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_add_zoom, 0, -50)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(1.0), FLOAT(0.4))
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), 75)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 20, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_ISIN_MOVE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(evt_btl_camera_shake_h, 0, 5, 0, 20, 0)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.5), FLOAT(0.4))
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), 100)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 20, -1)
    USER_FUNC(evt_btl_camera_shake_h, 0, 5, 0, 20, 0)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_ISIN_MOVE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
LBL(98)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(4.0))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    USER_FUNC(evt_btl_camera_noshake, 0)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitCleft_attack_event)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(5))
    USER_FUNC(btlevtcmd_GetOverTurnCount, -2, LW(0))
    IF_LARGE_EQUAL(LW(0), 1)
        RUN_CHILD_EVT(PTR(&unitCleft_wakeup_event))
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_GetFirstAttackTarget, LW(3), LW(4))
        USER_FUNC(evt_btl_camera_set_mode, 0, 8)
        USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 3)
        WAIT_MSEC(800)
    ELSE()
        USER_FUNC(btlevtcmd_CheckStatus, -2, 16, LW(0))
        IF_EQUAL(LW(0), 1)
            GOTO(10)
        END_IF()
        USER_FUNC(btlevtcmd_EnemyItemUseCheck, -2, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(LW(0))
            USER_FUNC(btlevtcmd_StartWaitEvent, -2)
            RETURN()
        END_IF()
        // More likely to charge turn 1.
        USER_FUNC(btlevtcmd_get_turn, LW(0))
        IF_SMALL_EQUAL(LW(0), 1)
            USER_FUNC(btlevtcmd_DrawLots, LW(0), 2, 25, 75)
        ELSE()
            USER_FUNC(btlevtcmd_DrawLots, LW(0), 2, 25, 25)
        END_IF()
        IF_EQUAL(LW(0), 1)
            IF_EQUAL(LW(5), (int32_t)BattleUnitType::HYPER_CLEFT)
                RUN_CHILD_EVT(PTR(&unitCleft_charge_event))
                RETURN()
            END_IF()
        END_IF()
LBL(10)
        SET(LW(9), PTR(&unitCleft_weapon))
        USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
        USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(9))
        USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(9), LW(3), LW(4))
    END_IF()
    IF_EQUAL(LW(5), (int32_t)BattleUnitType::IRON_CLEFT_RED)
        SET(LW(9), PTR(&unitIronCleft_weapon))
        RUN_CHILD_EVT(&unitCleft_slow_attack_event)
    ELSE()
        SET(LW(9), PTR(&unitCleft_weapon))
        RUN_CHILD_EVT(&unitCleft_normal_attack_event)
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitCleft_wait_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitCleft_common_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitCleft_wait_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitCleft_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitCleft_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitCleft_attack_event))
    USER_FUNC(btlevtcmd_SetWalkSound, -2, PTR("SFX_ENM_EMON_MOVE1"), PTR("SFX_ENM_EMON_MOVE2"), 0, 10, 10)
    USER_FUNC(btlevtcmd_SetRunSound, -2, PTR("SFX_ENM_EMON_MOVE1"), PTR("SFX_ENM_EMON_MOVE2"), 0, 7, 7)
    USER_FUNC(btlevtcmd_SetJumpSound, -2, PTR("SFX_ENM_EMON_JUMP1"), PTR("SFX_ENM_EMON_LANDING1"))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitCleft_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType,
        (int32_t)BattleUnitType::CLEFT)
    RUN_CHILD_EVT(unitCleft_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitHyperCleft_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType,
        (int32_t)BattleUnitType::HYPER_CLEFT)
    RUN_CHILD_EVT(unitCleft_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitMoonCleft_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, 
        (int32_t)BattleUnitType::MOON_CLEFT)
    RUN_CHILD_EVT(unitCleft_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitIronCleft_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType,
        (int32_t)BattleUnitType::IRON_CLEFT_RED)
    RUN_CHILD_EVT(unitCleft_common_init_event)
    // Override movement sounds.
    USER_FUNC(btlevtcmd_SetWalkSound, -2, PTR("SFX_BOSS_ISIN_MOVE1"), PTR("SFX_BOSS_ISIN_MOVE2"), 0, 10, 10)
    USER_FUNC(btlevtcmd_SetRunSound, -2, PTR("SFX_BOSS_ISIN_MOVE1"), PTR("SFX_BOSS_ISIN_MOVE2"), 0, 7, 7)
    RETURN()
EVT_END()

DataTableEntry unitCleft_data_table[] = {
    13, (void*)unitCleft_flip_event,
    37, (void*)unitCleft_spike_counter_event,
    48, (void*)btldefaultevt_Dummy,
    0, nullptr,
};

BattleUnitKindPart unitCleft_parts = {
    .index = 1,
    .name = "btl_un_monochrome_sinemon",
    .model_name = "c_sinemon_w",
    .part_offset_pos = { 0.0f, 0.0f, 0.0f },
    .part_hit_base_offset = { 0.0f, 25.0f, 0.0f },
    .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
    .unk_30 = 20,
    .unk_32 = 30,
    .base_alpha = 255,
    .defense = unitCleft_defense,
    .defense_attr = unitCleft_defense_attr,
    .attribute_flags = 0x0000'2009,
    .counter_attribute_flags = 0x0000'0001,
    .pose_table = unitCleft_pose_table,
};

BattleUnitKindPart unitHyperCleft_parts = {
    .index = 1,
    .name = "btl_un_hyper_sinemon",
    .model_name = "c_sinemon_h",
    .part_offset_pos = { 0.0f, 0.0f, 0.0f },
    .part_hit_base_offset = { 0.0f, 25.0f, 0.0f },
    .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
    .unk_30 = 20,
    .unk_32 = 30,
    .base_alpha = 255,
    .defense = unitCleft_defense,
    .defense_attr = unitCleft_defense_attr,
    .attribute_flags = 0x0000'2009,
    .counter_attribute_flags = 0x0000'0001,
    .pose_table = unitCleft_pose_table,
};

BattleUnitKindPart unitMoonCleft_parts = {
    .index = 1,
    .name = "btl_un_sinemon",
    .model_name = "c_sinemon",
    .part_offset_pos = { 0.0f, 0.0f, 0.0f },
    .part_hit_base_offset = { 0.0f, 25.0f, 0.0f },
    .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
    .unk_30 = 20,
    .unk_32 = 30,
    .base_alpha = 255,
    .defense = unitCleft_defense,
    .defense_attr = unitCleft_defense_attr,
    .attribute_flags = 0x0000'2009,
    .counter_attribute_flags = 0x0000'0001,
    .pose_table = unitCleft_pose_table,
};

BattleUnitKindPart unitIronCleft_parts = {
    .index = 1,
    .name = "btl_un_iron_sinemon",
    .model_name = "c_sinemon_a",
    .part_offset_pos = { 0.0f, 0.0f, 0.0f },
    .part_hit_base_offset = { 0.0f, 25.0f, 0.0f },
    .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
    .unk_30 = 30,
    .unk_32 = 30,
    .base_alpha = 255,
    .defense = unitCleft_defense,
    .defense_attr = unitIronCleft_defense_attr,
    .attribute_flags = 0x0000'0009,     // Not bomb-flippable
    .counter_attribute_flags = 0x0000'0001,
    .pose_table = unitCleft_pose_table,
};

BattleUnitKind unit_Cleft = {
    .unit_type = BattleUnitType::CLEFT,
    .unit_name = "btl_un_monochrome_sinemon",
    .max_hp = 2,
    .max_fp = 0,
    .danger_hp = 1,
    .peril_hp = 1,
    .level = 12,
    .bonus_exp = 0,
    .bonus_coin = 0,
    .bonus_coin_rate = 100,
    .base_coin = 0,
    .run_rate = 70,
    .pb_soft_cap = 9999,
    .width = 26,
    .height = 32,
    .hit_offset = { 4, 32 },
    .center_offset = { 0.0f, 16.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 13.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 13.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 10.0f, 24.0f, 10.0f },
    .cut_base_offset = { 0.0f, 16.0f, 0.0f },
    .cut_width = 26.0f,
    .cut_height = 32.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_EMON_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0x0004'0000,
    .status_vulnerability = &unitCleft_status,
    .num_parts = 1,
    .parts = &unitCleft_parts,
    .init_evt_code = (void*)unitCleft_init_event,
    .data_table = unitCleft_data_table,
};

BattleUnitKind unit_HyperCleft = {
    .unit_type = BattleUnitType::HYPER_CLEFT,
    .unit_name = "btl_un_hyper_sinemon",
    .max_hp = 4,
    .max_fp = 0,
    .danger_hp = 1,
    .peril_hp = 1,
    .level = 17,
    .bonus_exp = 0,
    .bonus_coin = 0,
    .bonus_coin_rate = 100,
    .base_coin = 0,
    .run_rate = 70,
    .pb_soft_cap = 9999,
    .width = 26,
    .height = 32,
    .hit_offset = { 4, 32 },
    .center_offset = { 0.0f, 16.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 13.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 13.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 10.0f, 24.0f, 10.0f },
    .cut_base_offset = { 0.0f, 16.0f, 0.0f },
    .cut_width = 26.0f,
    .cut_height = 32.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_EMON_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0x0004'0000,
    .status_vulnerability = &unitHyperCleft_status,
    .num_parts = 1,
    .parts = &unitHyperCleft_parts,
    .init_evt_code = (void*)unitHyperCleft_init_event,
    .data_table = unitCleft_data_table,
};

BattleUnitKind unit_MoonCleft = {
    .unit_type = BattleUnitType::MOON_CLEFT,
    .unit_name = "btl_un_sinemon",
    .max_hp = 6,
    .max_fp = 0,
    .danger_hp = 1,
    .peril_hp = 1,
    .level = 26,
    .bonus_exp = 0,
    .bonus_coin = 0,
    .bonus_coin_rate = 100,
    .base_coin = 0,
    .run_rate = 70,
    .pb_soft_cap = 9999,
    .width = 26,
    .height = 32,
    .hit_offset = { 4, 32 },
    .center_offset = { 0.0f, 16.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 13.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 13.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 10.0f, 24.0f, 10.0f },
    .cut_base_offset = { 0.0f, 16.0f, 0.0f },
    .cut_width = 26.0f,
    .cut_height = 32.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_EMON_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0x0004'0000,
    .status_vulnerability = &unitMoonCleft_status,
    .num_parts = 1,
    .parts = &unitMoonCleft_parts,
    .init_evt_code = (void*)unitMoonCleft_init_event,
    .data_table = unitCleft_data_table,
};

BattleUnitKind unit_IronCleft = {
    .unit_type = BattleUnitType::IRON_CLEFT_RED,
    .unit_name = "btl_un_iron_sinemon",
    .max_hp = 6,
    .max_fp = 0,
    .danger_hp = 1,
    .peril_hp = 1,
    .level = 18,
    .bonus_exp = 0,
    .bonus_coin = 0,
    .bonus_coin_rate = 100,
    .base_coin = 0,
    .run_rate = 70,
    .pb_soft_cap = 5,
    // Double-check that base size is the same if not scaled up.
    .width = 26,
    .height = 32,
    .hit_offset = { 4, 32 },
    .center_offset = { 0.0f, 16.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 13.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 13.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 10.0f, 24.0f, 10.0f },
    .cut_base_offset = { 0.0f, 16.0f, 0.0f },
    .cut_width = 26.0f,
    .cut_height = 32.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = -1,
    .swallow_attributes = 0,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_BOSS_ISIN_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0x0004'0000,
    .status_vulnerability = &unitIronCleft_status,
    .num_parts = 1,
    .parts = &unitIronCleft_parts,
    .init_evt_code = (void*)unitIronCleft_init_event,
    .data_table = unitCleft_data_table,
};

}  // namespace mod::tot::custom