#include "tot_custom_rel.h"     // For externed unit declarations

#include "evt_cmd.h"

#include <gc/types.h>
#include <ttyd/battle.h>
#include <ttyd/battle_camera.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_event_default.h>
#include <ttyd/battle_sub.h>
#include <ttyd/battle_unit.h>
#include <ttyd/battle_weapon_power.h>
#include <ttyd/eff_kemuri9_n64.h>
#include <ttyd/evt_eff.h>
#include <ttyd/evt_map.h>
#include <ttyd/evt_msg.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evt_sub.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>

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
using namespace ::ttyd::evt_eff;
using namespace ::ttyd::evt_map;
using namespace ::ttyd::evt_msg;
using namespace ::ttyd::evt_snd;
using namespace ::ttyd::evt_sub;

using ::ttyd::evtmgr_cmd::evtGetFloat;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

}  // namespace

// Unit work variable definitions.
constexpr const int32_t UW_AiState = 0;
constexpr const int32_t UW_LowHealthMsg = 1;
constexpr const int32_t UW_NumHeals = 2;
constexpr const int32_t UW_DragonType = 3;

// Handles phase changes + determines available moves.
namespace DragonAiState {
    enum e {
        PHASE_1_1 = 0,
        PHASE_1_2,
        PHASE_1_RANDOM,
        PHASE_2_1,
        PHASE_2_2,
        PHASE_2_RANDOM,
        PHASE_3_1,
        PHASE_3_2,
        PHASE_3_RANDOM,
    };
}

// Determines what type of attack to use.
namespace AttackType {
    enum e {
        STOMP = 0,
        BITE,
        BREATH,
        MEGABREATH,
        QUAKE,
        CHARGE,
        RECOVER
    };
}

// Determines the visual / status properties of breath attacks.
// Matches the order of types in "gonbaba_breath" evt_eff call.
namespace BreathType {
    enum e {
        FIRE = 0,
        POISON,
        MEGABREATH,
        FREEZE,
        CONFUSE,
        SLEEP,
        TINY,
    };
}

// Determines what dialogue tree to use for a particular situation.
namespace ConversationType {
    enum e {
        BATTLE_ENTRY = 0,
        PHASE_2_START,
        PHASE_3_START,
        LOW_HEALTH,
        MEGABREATH,
        HEAL,
        DEAD,
    };
}

// Function / USER_FUNC declarations.
EVT_DECLARE_USER_FUNC(evtTot_Dragon_StompEffect, 3)
EVT_DECLARE_USER_FUNC(evtTot_Dragon_SetBreathParams, 1)
EVT_DECLARE_USER_FUNC(evtTot_Dragon_GetPhaseHpThresholds, 4)
EVT_DECLARE_USER_FUNC(evtTot_Dragon_GetAttackWeights, 10)
EVT_DECLARE_USER_FUNC(evtTot_Dragon_GetBreathWeights, 9)
EVT_DECLARE_USER_FUNC(evtTot_Dragon_SetupConversation, 2)
EVT_DECLARE_USER_FUNC(evtTot_Dragon_GetNextDialogue, 2)

// Unit data.
int8_t unitHooktail_defense[] = { 1, 1, 1, 1, 1 };
int8_t unitGloomtail_defense[] = { 2, 2, 2, 2, 2 };
int8_t unitBonetail_defense[] = { 2, 2, 2, 2, 2 };

int8_t unitDragon_defense_attr[] = { 0, 0, 0, 0, 0 };

// Note: some statuses were changed to be more consistent with other bosses.
StatusVulnerability unitHooktail_status  = {
    30,  30,  50, 100,  50, 100, 100,   0,
     0,   0, 100,   0, 100,  75,  75,   0,
     0, 100,  30,   0,   0,   0,
};
StatusVulnerability unitGloomtail_status  = {
    30,  30,  30, 100,  30, 100, 100,   0,
     0,   0, 100,   0, 100,  70,  30,   0,
     0, 100,  30,   0,   0,   0,
};
StatusVulnerability unitBonetail_status  = {
     0,   0,   0, 100,   0, 100, 100,   0, 
     0,   0, 100,   0, 100,   0,   0,   0,
     0, 100,   0,   0,   0,   0,
};

PoseTableEntry unitDragon_pose_table[] = {
    1, "GNB_N_1",
    5, "GNB_X_1",
    2, "GNB_Z_1",
    3, "GNB_X_1",
    28, "GNB_S_3",
    65, "GNB_T_3",
    31, "GNB_S_3",
    39, "GNB_D_1",
    69, "GNB_S_3",
};

PoseTableEntry unitDragon_pose_table_weak[] = {
    1, "GNB_N_1",
    5, "GNB_X_1",
    2, "GNB_N_1",
    3, "GNB_X_1",
    28, "GNB_S_4",
    65, "GNB_T_4",
    31, "GNB_S_4",
    39, "GNB_V_1",
    69, "GNB_S_4",
};

BattleWeapon unitDragon_weaponStomp = {
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
    .damage_pattern = 0xb,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags = AttackSpecialProperty_Flags::ALL_BUFFABLE,
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
    .object_fall_chance = 10,
};

BattleWeapon unitDragon_weaponBite = {
    .name = nullptr,
    .icon = 0,
    .item_id = 0,
    .description = nullptr,
    .base_accuracy = 100,
    .base_fp_cost = 0,
    .base_sp_cost = 0,
    .superguards_allowed = 1,   // Contact-superguardable!
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
    .special_property_flags = AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = 0,
        
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

BattleWeapon unitDragon_weaponBreathBase = {
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
        AttackTargetClass_Flags::ONLY_TARGET_SELECT_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SELF |
        AttackTargetClass_Flags::CANNOT_TARGET_SAME_ALLIANCE |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,  // Set dynamically
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = 0,

    // status chances - set dynamically
    
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

BattleWeapon unitDragon_weaponQuake = {
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
    .damage_function_params = { 10, 0, 0, 0, 0, 0, 0, 0 },
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
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::CANNOT_TARGET_FLOATING,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::FLIPS_SHELLED |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = 0,

    // status chances
    
    .attack_evt_code = nullptr,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 30,
    .bg_a2_fall_weight = 30,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 0,
    .nozzle_fire_chance = 0,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

BattleWeapon unitDragon_weaponCharge = {
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
    .special_property_flags = AttackSpecialProperty_Flags::CANNOT_MISS,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = 0,

    // status chances
    .charge_strength = 8,
    
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

BattleWeapon unitDragon_weaponRecover = {
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
    .special_property_flags = AttackSpecialProperty_Flags::CANNOT_MISS,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = 0,

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

// EVTs.

// Custom, drives all dragon / party dialogue at phase transitions, etc.
// LW(0) = Conversation type, LW(1) = Dragon type.
// Note that the camera does not change, at least currently.
EVT_BEGIN(unitDragon_conversation_event)
    USER_FUNC(btlevtcmd_StatusWindowOnOff, 0)
    USER_FUNC(evtTot_Dragon_SetupConversation, LW(1), LW(0))
    USER_FUNC(evtTot_Dragon_GetNextDialogue, LW(14), LW(15))
    
    DO(0)
        IF_EQUAL(LW(14), 1)
            USER_FUNC(evt_msg_print, 2, LW(15), 0, -2)
        ELSE()
            USER_FUNC(evt_msg_print_battle_party, LW(15))
        END_IF()
            
        USER_FUNC(evtTot_Dragon_GetNextDialogue, LW(14), LW(15))
        IF_EQUAL(LW(14), -1)
            DO_BREAK()
        END_IF()
            
        WAIT_MSEC(300)
    WHILE()

    USER_FUNC(btlevtcmd_StatusWindowOnOff, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitDragon_stomp_event)
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(9))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(9), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("GNB_W_2"))
    WAIT_MSEC(950)
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_snd_se_offset, -2, PTR("SFX_BOSS_GNB_MOVE1"), EVT_NULLPTR, 0, -300, 0, -30, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        ADD(LW(0), -300)
        ADD(LW(2), -30)
        USER_FUNC(evtTot_Dragon_StompEffect, LW(0), LW(1), LW(2))
    END_BROTHER()
    USER_FUNC(evt_btl_camera_shake_h, 0, 2, 0, 10, 13)
    WAIT_MSEC(766)
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_snd_se_offset, -2, PTR("SFX_BOSS_GNB_MOVE1"), EVT_NULLPTR, 0, -300, 0, 30, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        ADD(LW(0), -300)
        ADD(LW(2), 30)
        USER_FUNC(evtTot_Dragon_StompEffect, LW(0), LW(1), LW(2))
    END_BROTHER()
    USER_FUNC(evt_btl_camera_shake_h, 0, 2, 0, 10, 13)
    WAIT_MSEC(16)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("GNB_A_3"))
    INLINE_EVT()
        USER_FUNC(btlevtcmd_snd_se_offset, -2, PTR("SFX_BOSS_GNB_ARM_UP1"), EVT_NULLPTR, 0, -320, 30, 30, EVT_NULLPTR)
        WAIT_MSEC(700)
        USER_FUNC(btlevtcmd_snd_se_offset, -2, PTR("SFX_BOSS_GNB_ARM_DOWN1"), EVT_NULLPTR, 0, -360, 50, 30, EVT_NULLPTR)
    END_INLINE()
    WAIT_MSEC(750)
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
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    GOTO(97)
LBL(97)
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_snd_se_offset, -2, PTR("SFX_BOSS_GNB_MOVE1"), EVT_NULLPTR, 0, -400, 0, -5, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        ADD(LW(0), -400)
        ADD(LW(2), -5)
        USER_FUNC(evtTot_Dragon_StompEffect, LW(0), LW(1), LW(2))
    END_BROTHER()
    WAIT_MSEC(50)
    USER_FUNC(evt_btl_camera_shake_h, 0, 2, 0, 10, 13)
    WAIT_MSEC(116)
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(evt_btl_camera_shake_h, 0, 8, 0, 30, 13)
    WAIT_MSEC(583)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("GNB_W_3"))
    WAIT_MSEC(850)
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_snd_se_offset, -2, PTR("SFX_BOSS_GNB_MOVE1"), EVT_NULLPTR, 0, -180, 0, 30, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        ADD(LW(0), -180)
        ADD(LW(2), 30)
        USER_FUNC(evtTot_Dragon_StompEffect, LW(0), LW(1), LW(2))
    END_BROTHER()
    USER_FUNC(evt_btl_camera_shake_h, 0, 2, 0, 10, 13)
    WAIT_MSEC(766)
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_snd_se_offset, -2, PTR("SFX_BOSS_GNB_MOVE1"), EVT_NULLPTR, 0, -100, 0, 30, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        ADD(LW(0), -100)
        ADD(LW(2), -30)
        USER_FUNC(evtTot_Dragon_StompEffect, LW(0), LW(1), LW(2))
    END_BROTHER()
    USER_FUNC(evt_btl_camera_shake_h, 0, 2, 0, 10, 13)
    WAIT_MSEC(116)
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitDragon_bite_event)
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(9))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(9), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    INLINE_EVT()
        USER_FUNC(btlevtcmd_snd_se_offset, -2, PTR("SFX_BOSS_GNB_BITE1"), EVT_NULLPTR, 0, -220, 80, 0, EVT_NULLPTR)
        WAIT_MSEC(250)
        USER_FUNC(btlevtcmd_snd_se_offset, -2, PTR("SFX_BOSS_GNB_BITE2"), EVT_NULLPTR, 0, -260, 40, 0, EVT_NULLPTR)
    END_INLINE()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("GNB_A_2"))
    WAIT_MSEC(333)
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
            GOTO(99)
            CASE_END()
    END_SWITCH()
LBL(90)
    GOTO(97)
LBL(91)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
LBL(97)
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
LBL(99)
    WAIT_MSEC(333)
    RETURN()
EVT_END()

// LW(14) = Animation style (Hooktail = 0, Gloomtail = 1, random = 2).
// LW(15) = Type of breath.
EVT_BEGIN(unitDragon_breath_event)
    // Pick sound effect to play based on breath type.
    SWITCH(LW(15))
        CASE_EQUAL((int32_t)BreathType::FREEZE)
            SET(LW(13), PTR("SFX_BOSS_GNB_BREATH_COLD1"))
        CASE_EQUAL((int32_t)BreathType::POISON)
            SET(LW(13), PTR("SFX_BOSS_GNB_BREATH_POISON1"))
        CASE_EQUAL((int32_t)BreathType::FIRE)
            SET(LW(13), PTR("SFX_BOSS_GNB_BREATH_FIRE1"))
        CASE_ETC()
            SET(LW(13), PTR("SFX_BOSS_GNB_BREATH_CHANGE1"))
    END_SWITCH()
    // Pick animation to use at random.
    IF_LARGE_EQUAL(LW(14), 2)
        USER_FUNC(evt_sub_random, LW(14), 1)
    END_IF()
    
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(9))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(9), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    IF_EQUAL(LW(14), 0)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("GNB_A_1"))
    ELSE()
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("GNB_A2_1"))
        WAIT_MSEC(833)
        USER_FUNC(btlevtcmd_snd_se_offset, -2, PTR("SFX_BOSS_GNB_NECK_UP1"), EVT_NULLPTR, 0, -250, 0, 0, EVT_NULLPTR)
        WAIT_MSEC(1000)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("GNB_A2_3"))
        USER_FUNC(btlevtcmd_snd_se_offset, -2, LW(13), EVT_NULLPTR, 0, -300, 0, 0, EVT_NULLPTR)
    END_IF()
    SET(LW(10), 120)
    SET(LW(11), 30)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(6), LW(7), LW(8))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(6), 360)
    ADD(LW(7), 30)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(12), EVT_NULLPTR, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(12), 360)
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(12), LW(10))
    IF_EQUAL(LW(14), 0)
        INLINE_EVT()
            WAIT_MSEC(500)
            USER_FUNC(btlevtcmd_snd_se_offset, -2, PTR("SFX_BOSS_GNB_NECK_UP1"), EVT_NULLPTR, 0, -250, 0, 0, EVT_NULLPTR)
        END_INLINE()
    END_IF()
    INLINE_EVT()
        IF_EQUAL(LW(14), 0)
            WAIT_FRM(105)
            USER_FUNC(btlevtcmd_snd_se_offset, -2, LW(13), EVT_NULLPTR, 0, -300, 0, 0, EVT_NULLPTR)
        ELSE()
            WAIT_FRM(20)
        END_IF()
        ADD(LW(11), 45)
        SUB(LW(8), 10)
        USER_FUNC(evt_eff, 0, PTR("gonbaba_breath"), LW(15), LW(6), LW(7), LW(8), FLOAT(3.0), LW(11), 0, 0, 0, 0, 0, 0)
    END_INLINE()
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(12), EVT_NULLPTR, EVT_NULLPTR)
    SUB(LW(12), LW(6))
    IF_SMALL(LW(12), 0)
        MUL(LW(12), -1)
    END_IF()
    ADD(LW(12), 20)
    MUL(LW(12), LW(11))
    DIV(LW(12), LW(10))
    IF_EQUAL(LW(14), 0)
        ADD(LW(12), 105)
    ELSE()
        ADD(LW(12), 20)
    END_IF()
    WAIT_FRM(LW(12))
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
    // DAS fixed (need to hit back guard with delayed timing, though).
    IF_EQUAL(LW(10), 0)
        USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
        ADD(LW(10), 1)
    END_IF()
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    GOTO(97)
LBL(97)
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    IF_NOT_EQUAL(LW(3), -1)
        WAIT_FRM(20)
        GOTO(10)
    END_IF()
    WAIT_FRM(84)
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitDragon_megabreath_event)
    SET(LW(0), (int32_t)ConversationType::MEGABREATH)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_DragonType, LW(1))
    RUN_CHILD_EVT(unitDragon_conversation_event)
    
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(9))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(9), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("GNB_A_1"))
    SET(LW(10), 120)
    SET(LW(11), 30)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(6), LW(7), LW(8))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(6), 360)
    ADD(LW(7), 30)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(12), EVT_NULLPTR, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(12), 360)
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(12), LW(10))
    INLINE_EVT()
        WAIT_MSEC(500)
        USER_FUNC(btlevtcmd_snd_se_offset, -2, PTR("SFX_BOSS_GNB_NECK_UP1"), EVT_NULLPTR, 0, -250, 0, 0, EVT_NULLPTR)
    END_INLINE()
    INLINE_EVT()
        WAIT_MSEC(1750)
        USER_FUNC(btlevtcmd_snd_se_offset, -2, PTR("SFX_BOSS_GNB_MEGABREATH1"), EVT_NULLPTR, 0, -300, 0, 0, EVT_NULLPTR)
        ADD(LW(11), 45)
        SUB(LW(8), 10)
        USER_FUNC(evt_eff, 0, PTR("gonbaba_breath"), 2, LW(6), LW(7), LW(8), FLOAT(3.0), LW(11), 0, 0, 0, 0, 0, 0)
    END_INLINE()
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(12), EVT_NULLPTR, EVT_NULLPTR)
    SUB(LW(12), LW(6))
    IF_SMALL(LW(12), 0)
        MUL(LW(12), -1)
    END_IF()
    ADD(LW(12), 20)
    MUL(LW(12), LW(11))
    DIV(LW(12), LW(10))
    ADD(LW(12), 105)
    WAIT_FRM(LW(12))
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
    // DAS fixed (need to hit back guard with delayed timing, though).
    IF_EQUAL(LW(10), 0)
        USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
        ADD(LW(10), 1)
    END_IF()
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    GOTO(97)
LBL(97)
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    IF_NOT_EQUAL(LW(3), -1)
        WAIT_MSEC(333)
        GOTO(10)
    END_IF()
    WAIT_MSEC(1400)
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitDragon_quake_event)
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(9))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(9), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("GNB_A_5"))
    WAIT_MSEC(666)
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BOSS_GNB_NECK_UP1"), 0)
    WAIT_MSEC(1000)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("GNB_A_6"))
    WAIT_MSEC(233)
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BOSS_GNB_DOWN3"), 0)
    WAIT_MSEC(50)
    USER_FUNC(evt_btl_camera_shake_h, 0, 2, 0, 10, 13)
    WAIT_MSEC(250)
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
            GOTO(99)
            CASE_END()
    END_SWITCH()
LBL(90)
    GOTO(97)
LBL(91)
    // DAS fixed.
    IF_EQUAL(LW(10), 0)
        USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
        ADD(LW(10), 1)
    END_IF()
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
    GOTO(97)
LBL(97)
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    IF_NOT_EQUAL(LW(3), -1)
        GOTO(10)
    END_IF()
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(evt_btl_camera_shake_h, 0, 8, 0, 30, 13)
    WAIT_MSEC(583)
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitDragon_charge_event)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(btlevtcmd_snd_se_offset, -2, PTR("SFX_BOSS_GNB_CHARGE1"), EVT_NULLPTR, 0, -250, 0, 0, LW(15))
    USER_FUNC(btlevtcmd_GetHitPos, -2, 2, LW(0), LW(1), LW(2))
    ADD(LW(0), 20)
    ADD(LW(1), -25)
    USER_FUNC(evt_eff, PTR(""), PTR("charge"), 0, LW(0), LW(1), LW(2), FLOAT(2.0), 60, 0, 0, 0, 0, 0, 0)
    WAIT_MSEC(1000)
    USER_FUNC(btlevtcmd_CheckDamage, -2, -2, 1, LW(9), 256, LW(5))
    RETURN()
EVT_END()

EVT_BEGIN(unitDragon_recover_event)
    SET(LW(0), (int32_t)ConversationType::HEAL)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_DragonType, LW(1))
    RUN_CHILD_EVT(unitDragon_conversation_event)
    
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_NumHeals, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        SUB(LW(0), 1)
        USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_NumHeals, LW(0))
    END_IF()
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(btlevtcmd_RecoverHp, -2, 1, 20)
    USER_FUNC(btlevtcmd_GetHitPos, -2, 1, LW(0), LW(1), LW(2))
    ADD(LW(0), -27)
    ADD(LW(1), 15)
    ADD(LW(2), 10)
    USER_FUNC(evt_eff, PTR(""), PTR("recovery"), 0, LW(0), LW(1), LW(2), 20, 0, 0, 0, 0, 0, 0, 0)
    WAIT_FRM(40)
    RETURN()
EVT_END()

EVT_BEGIN(unitDragon_attack_event)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_DragonType, LW(0))
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_AiState, LW(1))
    USER_FUNC(
        evtTot_Dragon_GetAttackWeights, 
        LW(0), LW(1),                                       // inputs
        LW(2), LW(3), LW(4), LW(5), LW(6), LW(7), LW(8),    // weights
        LW(10))                                             // next ai state
        
    // Disable healing weight if out of healing uses.
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_NumHeals, LW(13))
    IF_SMALL_EQUAL(LW(13), 0)
        SET(LW(8), 0)
    END_IF()
    
    // Pick attack at random.
    USER_FUNC(btlevtcmd_DrawLots, LW(13), 7, LW(2), LW(3), LW(4), LW(5), LW(6), LW(7), LW(8))
    SWITCH(LW(13))
        CASE_EQUAL((int32_t)AttackType::STOMP)
            SET(LW(9), PTR(&unitDragon_weaponStomp))
            SET(LW(13), PTR(&unitDragon_stomp_event))
            
        CASE_EQUAL((int32_t)AttackType::BITE)
            SET(LW(9), PTR(&unitDragon_weaponBite))
            SET(LW(13), PTR(&unitDragon_bite_event))
            
        CASE_EQUAL((int32_t)AttackType::QUAKE)
            SET(LW(9), PTR(&unitDragon_weaponQuake))
            SET(LW(13), PTR(&unitDragon_quake_event))
            
        CASE_EQUAL((int32_t)AttackType::RECOVER)
            SET(LW(9), PTR(&unitDragon_weaponRecover))
            SET(LW(13), PTR(&unitDragon_recover_event))
            
        CASE_EQUAL((int32_t)AttackType::CHARGE)
            SET(LW(9), PTR(&unitDragon_weaponCharge))
            // Override next AI type, to always follow up with Megabreath.
            SET(LW(10), (int32_t)DragonAiState::PHASE_3_2)
            SET(LW(13), PTR(&unitDragon_charge_event))
            
        CASE_EQUAL((int32_t)AttackType::MEGABREATH)
            SET(LW(9), PTR(&unitDragon_weaponBreathBase))
            USER_FUNC(evtTot_Dragon_SetBreathParams, (int32_t)BreathType::MEGABREATH)
            SET(LW(13), PTR(&unitDragon_megabreath_event))
            
        CASE_EQUAL((int32_t)AttackType::BREATH)
            GOTO(50)
    END_SWITCH()
    GOTO(99)
    
LBL(50)
    // Pick breath type at random.
    USER_FUNC(
        evtTot_Dragon_GetBreathWeights,
        LW(0), LW(1),                                       // inputs
        LW(2), LW(3), LW(4), LW(5), LW(6), LW(7), LW(8))    // weights
    
    SET(LW(14), LW(0))
    USER_FUNC(btlevtcmd_DrawLots, LW(15), 7, LW(2), LW(3), LW(4), LW(5), LW(6), LW(7), LW(8))
    SET(LW(9), PTR(&unitDragon_weaponBreathBase))
    USER_FUNC(evtTot_Dragon_SetBreathParams, LW(15))
    SET(LW(13), PTR(&unitDragon_breath_event))
    
LBL(99)
    // Update AI state for next attack, then perform attack.
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_AiState, LW(10))
    RUN_CHILD_EVT(LW(13))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitDragon_phase_event)
    USER_FUNC(btlevtcmd_CheckPhase, LW(0), 0x400'0004)
    IF_EQUAL(LW(0), 0)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_CheckActStatus, -2, LW(0))
    IF_EQUAL(LW(0), 0)
        GOTO(99)
    END_IF()
    // Get current HP, dragon type, and current AI phase.
    USER_FUNC(btlevtcmd_GetHp, -2, LW(0))
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_DragonType, LW(1))
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_AiState, LW(2))
    // Get target HP for event thresholds.
    USER_FUNC(evtTot_Dragon_GetPhaseHpThresholds, -2, LW(3), LW(4), LW(5))
    
    // Check for phase 3 (skip phase 2 if it's reached early).
    IF_SMALL_EQUAL(LW(0), LW(4))
        IF_SMALL(LW(2), (int32_t)DragonAiState::PHASE_3_1)
            USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_AiState, (int32_t)DragonAiState::PHASE_3_1)
            SET(LW(0), (int32_t)ConversationType::PHASE_3_START)
            RUN_CHILD_EVT(unitDragon_conversation_event)
            GOTO(99)
        END_IF()
    END_IF()
    // Check for phase 2.
    IF_SMALL_EQUAL(LW(0), LW(3))
        IF_SMALL(LW(2), (int32_t)DragonAiState::PHASE_2_1)
            USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_AiState, (int32_t)DragonAiState::PHASE_2_1)
            SET(LW(0), (int32_t)ConversationType::PHASE_2_START)
            RUN_CHILD_EVT(unitDragon_conversation_event)
            GOTO(99)
        END_IF()
    END_IF()
    // Check for low-health message.
    IF_SMALL_EQUAL(LW(0), LW(5))
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_LowHealthMsg, LW(2))
        IF_NOT_EQUAL(LW(2), 1)
            SET(LW(0), (int32_t)ConversationType::LOW_HEALTH)
            RUN_CHILD_EVT(unitDragon_conversation_event)
            USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_LowHealthMsg, 1)
            USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitDragon_pose_table_weak))
        END_IF()
    END_IF()
    
LBL(99)
    RETURN()
EVT_END()

// TODO: Add support for partnerless version of script.
EVT_BEGIN(unitDragon_battle_entry_event)
    USER_FUNC(evt_btl_camera_set_prilimit, 1)
    USER_FUNC(evt_map_replayanim, 0, PTR("dontyo"))
    USER_FUNC(evt_btl_camera_set_mode, 1, 3)
    USER_FUNC(evt_btl_camera_set_moveto, 1, 0, 110, 1080, 0, 93, -2, 1, 0)
    USER_FUNC(btlevtcmd_GetStageSize, LW(6), EVT_NULLPTR, EVT_NULLPTR)
    MUL(LW(6), -1)
    USER_FUNC(btlevtcmd_GetHomePos, -3, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHomePos, -4, LW(3), LW(4), LW(5))
    USER_FUNC(btlevtcmd_SetPos, -3, LW(6), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetPos, -4, LW(6), LW(4), LW(5))
    USER_FUNC(btlevtcmd_GetStageSize, EVT_NULLPTR, LW(3), EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    ADD(LW(1), LW(3))
    USER_FUNC(btlevtcmd_SetPos, -2, LW(0), LW(1), LW(2))
    WAIT_MSEC(1000)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -3, 1, 42)
    USER_FUNC(btlevtcmd_GetBodyId, -4, LW(0))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -4, LW(0), 42)
    USER_FUNC(btlevtcmd_GetHomePos, -3, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHomePos, -4, LW(3), LW(4), LW(5))
    SET(LW(8), LW(0))
    SUB(LW(8), LW(3))
    IF_SMALL(LW(8), 0)
        MUL(LW(8), -1)
    END_IF()
    USER_FUNC(btlevtcmd_GetStageSize, LW(6), EVT_NULLPTR, EVT_NULLPTR)
    SET(LW(7), LW(6))
    ADD(LW(7), LW(8))
    MUL(LW(6), -1)
    MUL(LW(7), -1)
    USER_FUNC(btlevtcmd_SetPos, -3, LW(6), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetPos, -4, LW(7), LW(4), LW(5))
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_SetMoveSpeed, -4, FLOAT(6.0))
        USER_FUNC(btlevtcmd_MovePosition, -4, LW(3), LW(4), LW(5), 0, -1, 0)
        USER_FUNC(btlevtcmd_GetBodyId, -4, LW(0))
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -4, LW(0), 43)
    END_BROTHER()
    USER_FUNC(btlevtcmd_SetMoveSpeed, -3, FLOAT(6.0))
    USER_FUNC(btlevtcmd_MovePosition, -3, LW(0), LW(1), LW(2), 0, -1, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -3, 1, PTR("M_I_Y"))
    WAIT_MSEC(33)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("GNB_F_3"))
    WAIT_FRM(1)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetPos, -2, LW(0), LW(1), LW(2))
    WAIT_MSEC(166)
    INLINE_EVT()
        WAIT_MSEC(83)
        USER_FUNC(btlevtcmd_AnimeChangePose, -3, 1, PTR("M_I_O"))
        USER_FUNC(btlevtcmd_SetFallAccel, -3, FLOAT(0.3))
        USER_FUNC(btlevtcmd_GetPos, -3, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_snd_se, -3, PTR("SFX_VOICE_MARIO_SURPRISED2_2"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_JumpPosition, -3, LW(0), LW(1), LW(2), 25, -1)
    END_INLINE()
    INLINE_EVT()
        WAIT_MSEC(83)
        USER_FUNC(btlevtcmd_SetFallAccel, -4, FLOAT(0.3))
        USER_FUNC(btlevtcmd_GetPos, -4, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_JumpPosition, -4, LW(0), LW(1), LW(2), 25, -1)
        USER_FUNC(btlevtcmd_GetBodyId, -4, LW(0))
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -4, LW(0), 5)
    END_INLINE()
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BOSS_GNB_APPEAR1"), 0)
    USER_FUNC(evt_btl_camera_shake_h, 1, 8, 0, 20, 13)
    WAIT_MSEC(500)
    USER_FUNC(btlevtcmd_AnimeChangePose, -3, 1, PTR("M_I_Y"))
    USER_FUNC(btlevtcmd_GetBodyId, -4, LW(0))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -4, LW(0), 43)
    USER_FUNC(evt_btl_camera_set_mode, 1, 3)
    USER_FUNC(evt_btl_camera_set_moveto, 1, -233, 45, 452, 56, 125, 37, 60, 0)
    WAIT_MSEC(1000)
    USER_FUNC(btlevtcmd_SetTalkPose, -2, PTR("GNB_T_3"))
    USER_FUNC(btlevtcmd_SetStayPose, -2, PTR("GNB_S_3"))
    SET(LW(0), (int32_t)ConversationType::BATTLE_ENTRY)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_DragonType, LW(1))
    RUN_CHILD_EVT(unitDragon_conversation_event)
    USER_FUNC(btlevtcmd_StartWaitEvent, -3)
    USER_FUNC(btlevtcmd_StartWaitEvent, -4)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    USER_FUNC(evt_btl_camera_set_prilimit, 0)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 3)
    RETURN()
EVT_END()

EVT_BEGIN(unitDragon_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    RETURN()
EVT_END()

EVT_BEGIN(unitDragon_damage_sub_event)
    USER_FUNC(btlevtcmd_GetDamagePartsId, -2, LW(11))
    IF_EQUAL(LW(11), 1)
        USER_FUNC(btlevtcmd_AnimeChangePose, LW(10), 1, PTR("GNB_D_2"))
    ELSE()
        USER_FUNC(btlevtcmd_AnimeChangePose, LW(10), 1, PTR("GNB_D_1"))
    END_IF()
    USER_FUNC(btlevtcmd_GetDamage, LW(10), LW(0))
    IF_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_GetHitPos, LW(10), LW(11), LW(0), LW(1), LW(2))
        USER_FUNC(evt_snd_sfxon_3d, PTR("SFX_BTL_ATTACK_MISS2"), LW(0), LW(1), LW(2), 0)
    END_IF()
    WAIT_MSEC(1000)
    RETURN()
EVT_END()

EVT_BEGIN(unitDragon_dead_event)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("GNB_X_1"))
    USER_FUNC(btlevtcmd_snd_se_offset, -2, PTR("SFX_BOSS_GNB_DOWN1"), EVT_NULLPTR, 0, -250, 0, 0, LW(13))
    USER_FUNC(btlevtcmd_WaitAttackEnd)
    USER_FUNC(evt_btl_camera_set_prilimit, 1)
    USER_FUNC(evt_btl_camera_wait_move_end)
    USER_FUNC(evt_btl_camera_set_mode, 1, 3)
    SET(LW(1), 160)
    SET(LW(2), 536)
    USER_FUNC(btlevtcmd_GetStageAudienceAreaSize, EVT_NULLPTR, LW(5), EVT_NULLPTR)
    ADD(LW(1), LW(5))
    USER_FUNC(btlevtcmd_GetStageSize, EVT_NULLPTR, EVT_NULLPTR, LW(5))
    ADD(LW(2), LW(5))
    ADD(LW(2), 80)
    SET(LW(5), LW(1))
    SUB(LW(5), 37)
    SET(LW(6), LW(2))
    SUB(LW(6), 511)
    USER_FUNC(evt_btl_camera_set_moveto, 1, 93, LW(1), LW(2), 93, LW(5), LW(6), 30, 0)
    WAIT_MSEC(1500)
    USER_FUNC(btlevtcmd_SetTalkPose, -2, PTR("GNB_X_1"))
    USER_FUNC(btlevtcmd_SetStayPose, -2, PTR("GNB_X_1"))
    SET(LW(0), (int32_t)ConversationType::DEAD)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_DragonType, LW(1))
    RUN_CHILD_EVT(unitDragon_conversation_event)
    
    USER_FUNC(evt_btl_camera_set_mode, 1, 3)
    USER_FUNC(evt_btl_camera_set_moveto, 1, -38, 33, 1005, -38, 92, 13, 60, 0)
    USER_FUNC(evt_snd_sfxoff, LW(13))
    USER_FUNC(btlevtcmd_snd_se_offset, -2, PTR("SFX_BOSS_GNB_DOWN2"), EVT_NULLPTR, 0, -250, 0, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("GNB_D_3"))
    WAIT_MSEC(700)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("GNB_H_1"))
    WAIT_FRM(1)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    ADD(LW(0), 25)
    USER_FUNC(btlevtcmd_SetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("GNB_H_1"))
    INLINE_EVT()
        SET(LW(5), 100)
        SET(LW(4), 120)
        USER_FUNC(btlevtcmd_SetRotateOffset, -2, 0, LW(5), 0)
        DO(40)
            USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, LW(4))
            SUB(LW(4), 3)
            WAIT_FRM(1)
        WHILE()
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    END_INLINE()
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.2))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    SUB(LW(0), 20)
    SUB(LW(2), 20)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 90, -1)
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BOSS_GNB_DOWN3"), 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("GNB_H_2"))
    USER_FUNC(evt_btl_camera_shake_h, 1, 8, 0, 30, 13)
    WAIT_MSEC(166)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("GNB_H_3"))
    USER_FUNC(btlevtcmd_GetExp, LW(10), LW(0))
    USER_FUNC(btlevtcmd_StoreExp, LW(0))
    IF_LARGE_EQUAL(LW(0), 1)
        DO(LW(0))
        WHILE()
    END_IF()
    WAIT_MSEC(1000)
    USER_FUNC(btlevtcmd_KillUnit, -2, 1)
    USER_FUNC(evt_btl_camera_set_prilimit, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitDragon_wait_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitDragon_init_event)
    USER_FUNC(btlevtcmd_OnUnitFlag, -2, 16777216)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitDragon_wait_event))
    USER_FUNC(btlevtcmd_SetEventPhase, -2, PTR(&unitDragon_phase_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitDragon_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitDragon_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&btldefaultevt_Confuse))
    USER_FUNC(btlevtcmd_SetTalkBodyId, -2, 1)
    USER_FUNC(btlevtcmd_SetBaseScale, -2, FLOAT(1.3), FLOAT(1.3), FLOAT(0.5))
    USER_FUNC(btlevtcmd_SetEventEntry, -2, PTR(&unitDragon_battle_entry_event))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitHooktail_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_AiState, (int32_t)DragonAiState::PHASE_1_1)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_LowHealthMsg, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_DragonType, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_NumHeals, 0)
    RUN_CHILD_EVT(unitDragon_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitGloomtail_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_AiState, (int32_t)DragonAiState::PHASE_1_1)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_LowHealthMsg, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_DragonType, 1)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_NumHeals, 0)
    RUN_CHILD_EVT(unitDragon_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitBonetail_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_AiState, (int32_t)DragonAiState::PHASE_1_1)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_LowHealthMsg, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_DragonType, 2)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_NumHeals, 2)
    RUN_CHILD_EVT(unitDragon_init_event)
    RETURN()
EVT_END()

DataTableEntry unitDragon_data_table[] = {
    9, (void*)unitDragon_damage_sub_event,
    49, (void*)unitDragon_dead_event,
    0, nullptr,
};

BattleUnitKindPart unitHooktail_parts[] = {
    {
        .index = 1,
        .name = "btl_un_gonbaba",
        .model_name = "c_gonbaba",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 230.0f, 36.0f, 60.0f },
        .part_hit_cursor_base_offset = { 230.0f, 36.0f, 60.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitHooktail_defense,
        .defense_attr = unitDragon_defense_attr,
        .attribute_flags = 0x0000'0002,
        .counter_attribute_flags = 0,
        .pose_table = unitDragon_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_gonbaba",
        .model_name = "c_gonbaba",
        .part_offset_pos = { 100.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 250.0f, 110.0f, 25.0f },
        .part_hit_cursor_base_offset = { 250.0f, 110.0f, 25.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitHooktail_defense,
        .defense_attr = unitDragon_defense_attr,
        .attribute_flags = 0x0160'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitDragon_pose_table,
    },
    {
        .index = 3,
        .name = "btl_un_gonbaba",
        .model_name = "c_gonbaba4",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitHooktail_defense,
        .defense_attr = unitDragon_defense_attr,
        .attribute_flags = 0x0101'0000,
        .counter_attribute_flags = 0,
        .pose_table = unitDragon_pose_table,
    },
};

BattleUnitKindPart unitGloomtail_parts[] = {
    {
        .index = 1,
        .name = "btl_un_bunbaba",
        .model_name = "c_gonbaba_b",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 230.0f, 36.0f, 60.0f },
        .part_hit_cursor_base_offset = { 230.0f, 36.0f, 60.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitGloomtail_defense,
        .defense_attr = unitDragon_defense_attr,
        .attribute_flags = 0x0000'0002,
        .counter_attribute_flags = 0,
        .pose_table = unitDragon_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_bunbaba",
        .model_name = "c_gonbaba_b",
        .part_offset_pos = { 100.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 250.0f, 110.0f, 25.0f },
        .part_hit_cursor_base_offset = { 250.0f, 110.0f, 25.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitGloomtail_defense,
        .defense_attr = unitDragon_defense_attr,
        .attribute_flags = 0x0160'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitDragon_pose_table,
    },
    {
        .index = 3,
        .name = "btl_un_bunbaba",
        .model_name = "c_gonbaba4",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitGloomtail_defense,
        .defense_attr = unitDragon_defense_attr,
        .attribute_flags = 0x0101'0000,
        .counter_attribute_flags = 0,
        .pose_table = unitDragon_pose_table,
    },
};

BattleUnitKindPart unitBonetail_parts[] = {
    {
        .index = 1,
        .name = "btl_un_zonbaba",
        .model_name = "c_gonbaba_z",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 230.0f, 36.0f, 60.0f },
        .part_hit_cursor_base_offset = { 230.0f, 36.0f, 60.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitHooktail_defense,
        .defense_attr = unitDragon_defense_attr,
        .attribute_flags = 0x0000'0002,
        .counter_attribute_flags = 0,
        .pose_table = unitDragon_pose_table,
    },
    {
        .index = 2,
        .name = "btl_un_zonbaba",
        .model_name = "c_gonbaba_z",
        .part_offset_pos = { 100.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 250.0f, 110.0f, 25.0f },
        .part_hit_cursor_base_offset = { 250.0f, 110.0f, 25.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitHooktail_defense,
        .defense_attr = unitDragon_defense_attr,
        .attribute_flags = 0x0160'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitDragon_pose_table,
    },
    {
        .index = 3,
        .name = "btl_un_zonbaba",
        .model_name = "c_gonbaba4",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 0.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 0.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitBonetail_defense,
        .defense_attr = unitDragon_defense_attr,
        .attribute_flags = 0x0101'0000,
        .counter_attribute_flags = 0,
        .pose_table = unitDragon_pose_table,
    },
};

BattleUnitKind unit_Hooktail = {
    .unit_type = BattleUnitType::HOOKTAIL,
    .unit_name = "btl_un_gonbaba",
    .max_hp = 20,
    .max_fp = 0,
    .danger_hp = 1,
    .peril_hp = 1,
    .level = 65,
    .bonus_exp = 0,
    .bonus_coin = 0,
    .bonus_coin_rate = 100,
    .base_coin = 0,
    .run_rate = 50,
    .pb_soft_cap = 9,
    .width = 230,
    .height = 170,
    .hit_offset = { -210, 45 },
    .center_offset = { 0.0f, 0.0f, 0.0f },
    .hp_gauge_offset = { -250, 0 },
    .talk_toge_base_offset = { 280.0f, 40.0f, 0.0f },
    .held_item_base_offset = { 115.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 115.0f, 0.0f, 0.0f },
    .unk_54 = 20.0f,
    .binta_hit_offset = { 250.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 380.0f, 110.0f, 50.0f },
    .cut_base_offset = { -290.0f, 85.0f, 0.0f },
    .cut_width = 230.0f,
    .cut_height = 170.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = -1,
    .swallow_attributes = 0,
    .hammer_knockback_chance = 0,
    .itemsteal_param = 0,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_BOSS_GNB_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0x0020'0000,
    .status_vulnerability = &unitHooktail_status,
    .num_parts = 3,
    .parts = unitHooktail_parts,
    .init_evt_code = (void*)unitHooktail_init_event,
    .data_table = unitDragon_data_table,
};

BattleUnitKind unit_Gloomtail = {
    .unit_type = BattleUnitType::GLOOMTAIL,
    .unit_name = "btl_un_bunbaba",
    .max_hp = 80,
    .max_fp = 0,
    .danger_hp = 1,
    .peril_hp = 1,
    .level = 76,
    .bonus_exp = 0,
    .bonus_coin = 0,
    .bonus_coin_rate = 100,
    .base_coin = 0,
    .run_rate = 50,
    .pb_soft_cap = 7,
    .width = 230,
    .height = 170,
    .hit_offset = { -210, 45 },
    .center_offset = { 0.0f, 0.0f, 0.0f },
    .hp_gauge_offset = { -250, 0 },
    .talk_toge_base_offset = { 280.0f, 40.0f, 0.0f },
    .held_item_base_offset = { 115.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 115.0f, 0.0f, 0.0f },
    .unk_54 = 20.0f,
    .binta_hit_offset = { 250.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 380.0f, 110.0f, 50.0f },
    .cut_base_offset = { -290.0f, 85.0f, 0.0f },
    .cut_width = 230.0f,
    .cut_height = 170.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = -1,
    .swallow_attributes = 0,
    .hammer_knockback_chance = 0,
    .itemsteal_param = 0,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_BOSS_GNB_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0x0020'0000,
    .status_vulnerability = &unitGloomtail_status,
    .num_parts = 3,
    .parts = unitGloomtail_parts,
    .init_evt_code = (void*)unitGloomtail_init_event,
    .data_table = unitDragon_data_table,
};

BattleUnitKind unit_Bonetail = {
    .unit_type = BattleUnitType::BONETAIL,
    .unit_name = "btl_un_zonbaba",
    .max_hp = 200,
    .max_fp = 0,
    .danger_hp = 1,
    .peril_hp = 1,
    .level = 99,
    .bonus_exp = 0,
    .bonus_coin = 0,
    .bonus_coin_rate = 100,
    .base_coin = 0,
    .run_rate = 50,
    .pb_soft_cap = 5,
    .width = 230,
    .height = 170,
    .hit_offset = { -210, 45 },
    .center_offset = { 0.0f, 0.0f, 0.0f },
    .hp_gauge_offset = { -250, 0 },
    .talk_toge_base_offset = { 280.0f, 40.0f, 0.0f },
    .held_item_base_offset = { 115.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 115.0f, 0.0f, 0.0f },
    .unk_54 = 20.0f,
    .binta_hit_offset = { 250.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 380.0f, 110.0f, 50.0f },
    .cut_base_offset = { -290.0f, 85.0f, 0.0f },
    .cut_width = 230.0f,
    .cut_height = 170.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = -1,
    .swallow_attributes = 0,
    .hammer_knockback_chance = 0,
    .itemsteal_param = 0,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_BOSS_GNB_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0x0020'0000,
    .status_vulnerability = &unitBonetail_status,
    .num_parts = 3,
    .parts = unitBonetail_parts,
    .init_evt_code = (void*)unitBonetail_init_event,
    .data_table = unitDragon_data_table,
};

// arg0-2: x, y, z position of effect
EVT_DEFINE_USER_FUNC(evtTot_Dragon_StompEffect) {
    float x = evtGetFloat(evt, evt->evtArguments[0]);
    float y = evtGetFloat(evt, evt->evtArguments[1]);
    float z = evtGetFloat(evt, evt->evtArguments[2]);
    ttyd::eff_kemuri9_n64::effKemuri9N64Entry(x, y, z, 60.0, 0.0, 1, 18, 90);
    return 2;
}

// arg0: Type of breath
EVT_DEFINE_USER_FUNC(evtTot_Dragon_SetBreathParams) {
    auto& weapon = unitDragon_weaponBreathBase;
    // Clear all status parameters.
    memset(&weapon.sleep_chance, 0, 0x30);
    weapon.element = AttackElement::NORMAL;
    
    switch (evtGetValue(evt, evt->evtArguments[0])) {
        case BreathType::FIRE:
            weapon.element = AttackElement::FIRE;
            // Why not, y'know
            weapon.burn_chance = 50;
            weapon.burn_time = 3;
            break;
        case BreathType::POISON:
            weapon.poison_chance = 50;
            weapon.poison_time = 10;
            weapon.poison_strength = 1;
            break;
        case BreathType::FREEZE:
            // Increased chance since it's easier to break out of.
            weapon.freeze_chance = 50;
            weapon.freeze_time = 3;
            break;
        case BreathType::CONFUSE:
            weapon.confuse_chance = 30;
            weapon.confuse_time = 3;
            break;
        case BreathType::SLEEP:
            weapon.sleep_chance = 30;
            weapon.sleep_time = 3;
            break;
        case BreathType::TINY:
            weapon.size_change_chance = 30;
            weapon.size_change_time = 3;
            weapon.size_change_strength = -2;
            break;
        case BreathType::MEGABREATH:
            // No special properties; even its attack power is default, lol.
            break;
    }
    
    return 2;
}

// arg0: dragon unit id, arg1~3: out thresholds for phase 2, phase 3, low health
EVT_DEFINE_USER_FUNC(evtTot_Dragon_GetPhaseHpThresholds) {
    auto* battleWork = ttyd::battle::g_BattleWork;
    int32_t unit_id = evtGetValue(evt, evt->evtArguments[0]);
    unit_id = ttyd::battle_sub::BattleTransID(evt, unit_id);
    auto* unit = ttyd::battle::BattleGetUnitPtr(battleWork, unit_id);
    int32_t hp = unit->max_hp;
    
    switch (unit->current_kind) {
        case BattleUnitType::HOOKTAIL:
            evtSetValue(evt, evt->evtArguments[1], hp * 50 / 100);
            evtSetValue(evt, evt->evtArguments[2], hp * 0 / 100);
            evtSetValue(evt, evt->evtArguments[3], hp * 25 / 100);
            break;
        case BattleUnitType::GLOOMTAIL:
            evtSetValue(evt, evt->evtArguments[1], hp * 60 / 100);
            evtSetValue(evt, evt->evtArguments[2], hp * 40 / 100);
            evtSetValue(evt, evt->evtArguments[3], hp * 20 / 100);
            break;
        case BattleUnitType::BONETAIL:
            evtSetValue(evt, evt->evtArguments[1], hp * 50 / 100);
            evtSetValue(evt, evt->evtArguments[2], hp * 0 / 100);
            evtSetValue(evt, evt->evtArguments[3], hp * 25 / 100);
            break;
    }
    
    return 2;
}

// arg0: dragon type, arg1: ai state
// out arg2~8: Weights for attacks (in AttackType order)
// out arg9: Next ai state
EVT_DEFINE_USER_FUNC(evtTot_Dragon_GetAttackWeights) {
    int32_t dragon_type = evtGetValue(evt, evt->evtArguments[0]);
    int32_t ai_type = evtGetValue(evt, evt->evtArguments[1]);
    int32_t weights[7] = { 0, 0, 0, 0, 0, 0, 0 };
    int32_t next_ai_type = ai_type;
    
    switch (dragon_type) {
        case 0:
            switch(ai_type) {
                case DragonAiState::PHASE_1_1:
                    weights[AttackType::BREATH] = 10;
                    next_ai_type = DragonAiState::PHASE_1_2;
                    break;
                case DragonAiState::PHASE_1_2:
                    weights[AttackType::STOMP] = 10;
                    next_ai_type = DragonAiState::PHASE_1_RANDOM;
                    break;
                case DragonAiState::PHASE_1_RANDOM:
                    weights[AttackType::BREATH] = 10;
                    weights[AttackType::STOMP] = 10;
                    break;
                case DragonAiState::PHASE_2_1:
                case DragonAiState::PHASE_2_2:
                    weights[AttackType::BITE] = 10;
                    next_ai_type = DragonAiState::PHASE_2_RANDOM;
                    break;
                case DragonAiState::PHASE_2_RANDOM:
                    weights[AttackType::STOMP] = 10;
                    weights[AttackType::BREATH] = 15;
                    weights[AttackType::BITE] = 5;
                    break;
            }
            break;
        case 1:
            switch(ai_type) {
                case DragonAiState::PHASE_1_1:
                case DragonAiState::PHASE_1_2:
                    weights[AttackType::BREATH] = 10;
                    next_ai_type = DragonAiState::PHASE_1_RANDOM;
                    break;
                case DragonAiState::PHASE_1_RANDOM:
                    weights[AttackType::BREATH] = 20;
                    weights[AttackType::STOMP] = 10;
                    weights[AttackType::BITE] = 10;
                    next_ai_type = DragonAiState::PHASE_1_RANDOM;
                    break;
                case DragonAiState::PHASE_2_1:
                case DragonAiState::PHASE_2_2:
                    weights[AttackType::QUAKE] = 10;
                    next_ai_type = DragonAiState::PHASE_2_RANDOM;
                    break;
                case DragonAiState::PHASE_2_RANDOM:
                    weights[AttackType::BREATH] = 20;
                    weights[AttackType::QUAKE] = 15;
                    weights[AttackType::STOMP] = 10;
                    weights[AttackType::BITE] = 10;
                    break;
                case DragonAiState::PHASE_3_1:
                    weights[AttackType::CHARGE] = 10;
                    next_ai_type = DragonAiState::PHASE_3_2;
                    break;
                case DragonAiState::PHASE_3_2:
                    weights[AttackType::MEGABREATH] = 10;
                    next_ai_type = DragonAiState::PHASE_3_RANDOM;
                    break;
                case DragonAiState::PHASE_3_RANDOM:
                    weights[AttackType::BREATH] = 20;
                    weights[AttackType::CHARGE] = 20;
                    weights[AttackType::QUAKE] = 10;
                    weights[AttackType::STOMP] = 10;
                    weights[AttackType::BITE] = 10;
                    break;
            }
            break;
        case 2:
            switch(ai_type) {
                case DragonAiState::PHASE_1_1:
                    weights[AttackType::BREATH] = 10;
                    next_ai_type = DragonAiState::PHASE_1_RANDOM;
                    break;
                case DragonAiState::PHASE_2_1:
                    weights[AttackType::BREATH] = 10;
                    next_ai_type = DragonAiState::PHASE_2_RANDOM;
                    break;
                case DragonAiState::PHASE_2_RANDOM:
                    weights[AttackType::RECOVER] = 20;
                    // fallthrough
                default:
                    weights[AttackType::BREATH] = 40;
                    weights[AttackType::STOMP] = 25;
                    weights[AttackType::BITE] = 15;
                    break;
            }
            break;
    }
    
    for (int32_t i = 0; i < 7; ++i) {
        evtSetValue(evt, evt->evtArguments[2 + i], weights[i]);
    }
    evtSetValue(evt, evt->evtArguments[9], next_ai_type);
    
    return 2;
}

// arg0: dragon type, arg1: ai state
// out arg2~8: Weights for breaths (in BreathType order)
EVT_DEFINE_USER_FUNC(evtTot_Dragon_GetBreathWeights) {
    int32_t dragon_type = evtGetValue(evt, evt->evtArguments[0]);
    int32_t ai_type = evtGetValue(evt, evt->evtArguments[1]);
    int32_t weights[7] = { 0, 0, 0, 0, 0, 0, 0 };
    
    switch (dragon_type) {
        case 0:
            weights[BreathType::FIRE] = 10;
            break;
        case 1:
            weights[BreathType::POISON] = 10;
            if (ai_type > DragonAiState::PHASE_1_1) {
                weights[BreathType::FIRE] = 10;
            }
            break;
        case 2:
            if (ai_type == DragonAiState::PHASE_2_1) {
                weights[BreathType::FREEZE] = 10;
                break;
            }
            weights[BreathType::SLEEP] = 10;
            weights[BreathType::CONFUSE] = 10;
            weights[BreathType::TINY] = 10;
            weights[BreathType::POISON] = 10;
            if (ai_type > DragonAiState::PHASE_1_RANDOM) {
                weights[BreathType::FREEZE] = 20;
            }
            break;
    }
    
    for (int32_t i = 0; i < 7; ++i) {
        evtSetValue(evt, evt->evtArguments[2 + i], weights[i]);
    }
    
    return 2;
}

struct DialogueEntry {
    int32_t speaker;    // 1 for dragon, 2 for partners
    char msg[16];
};
DialogueEntry g_DialogueList[16];
int32_t g_DialoguePos = 0;

// arg0 = dragon, arg1 = conversation type
EVT_DEFINE_USER_FUNC(evtTot_Dragon_SetupConversation) {
    int32_t dragon_type = evtGetValue(evt, evt->evtArguments[0]);
    int32_t conversation_type = evtGetValue(evt, evt->evtArguments[1]);
    
    // For now, assume every message is a single dragon dialogue box.
    g_DialogueList[0].speaker = 1;
    sprintf(
        g_DialogueList[0].msg, "tot_dragon%02" PRId32 "_%02" PRId32,
        dragon_type, conversation_type);
    
    g_DialogueList[1].speaker = -1;
    g_DialoguePos = 0;
    
    return 2;
}

// out arg0 = speaker, arg1 = message id
EVT_DEFINE_USER_FUNC(evtTot_Dragon_GetNextDialogue) {
    evtSetValue(
        evt, evt->evtArguments[0], g_DialogueList[g_DialoguePos].speaker);
    evtSetValue(
        evt, evt->evtArguments[1], PTR(g_DialogueList[g_DialoguePos].msg));
    ++g_DialoguePos;
    return 2;
}

}  // namespace mod::tot::custom