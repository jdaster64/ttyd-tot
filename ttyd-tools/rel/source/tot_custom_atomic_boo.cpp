#include "tot_custom_rel.h"     // For externed unit definitions.

#include "evt_cmd.h"
#include "tot_state.h"

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
#include <ttyd/dispdrv.h>
#include <ttyd/evt_eff.h>
#include <ttyd/evt_ext.h>
#include <ttyd/evt_npc.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evt_sub.h>
#include <ttyd/evtmgr.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/extdrv.h>
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
using namespace ::ttyd::evt_ext;
using namespace ::ttyd::evt_npc;
using namespace ::ttyd::evt_snd;
using namespace ::ttyd::evt_sub;

using ::ttyd::battle::g_BattleWork;
using ::ttyd::battle_unit::BattleWorkUnit;
using ::ttyd::battle_unit::BattleWorkUnitPart;
using ::ttyd::dispdrv::CameraId;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;
using ::ttyd::extdrv::ExtEntryData;
using ::ttyd::extdrv::ExtPoseWork;

// State for "ext" Boo actors.
struct ExtBooWork {
    int8_t      state;
    bool        surprise_animation;
    int8_t      pad_0x02[2];
    int32_t     anim_timer;
    int32_t     delay;          // will not execute switch code while > 0.
    int32_t     timer;
    gc::vec3    unk_0x10;
    gc::vec3    unk_0x1c;
    gc::vec3    unk_0x28;
    int8_t      ease_mode;
    int8_t      pad_0x35[3];
} ;

static_assert(sizeof(ExtBooWork) == 0x38);

}  // namespace

// Function / USER_FUNC declarations.
void ext_boo_init();
void ext_boo_main();
EVT_DECLARE_USER_FUNC(evtTot_AtomicBoo_SetUnitId, 1)
EVT_DECLARE_USER_FUNC(evtTot_AtomicBoo_SetTarget, 2)
EVT_DECLARE_USER_FUNC(evtTot_AtomicBoo_CircleAppear, 0)
EVT_DECLARE_USER_FUNC(evtTot_AtomicBoo_CircleWait1, 0)
EVT_DECLARE_USER_FUNC(evtTot_AtomicBoo_CircleWait2, 0)
EVT_DECLARE_USER_FUNC(evtTot_AtomicBoo_BreathAppear, 0)
EVT_DECLARE_USER_FUNC(evtTot_AtomicBoo_BreathWait1, 0)
EVT_DECLARE_USER_FUNC(evtTot_AtomicBoo_BreathWait2, 0)

// Constants.

constexpr const int32_t UW_BattleUnitType = 0;
constexpr const int32_t UW_FaceHidden = 1;

constexpr const double k2Pi = 6.28318530718;
constexpr const double kPiOver4 = 0.78539816339;

// Unit data.
int8_t unitAtomicBoo_defense[] = { 0, 0, 0, 0, 0 };
int8_t unitAtomicBoo_defense_attr[] = { 0, 0, 0, 0, 0 };

StatusVulnerability unitAtomicBoo_status = {
    40,  40,  40, 100,  40, 100, 100,  30,
     0,   0, 100,   0, 100,  80,  40,   0,
    10, 100,  40, 100, 100,   5,
};

StatusVulnerability unitCosmicBoo_status = {
    30,  30,  30, 100,  30, 100, 100,  30,
     0,   0, 100,   0, 100,  80,  30,   0,
    10, 100,  30, 100, 100,   5,
};

PoseTableEntry unitAtomicBoo_pose_table[] = {
    1, "N_1",
    2, "Y_1",
    9, "Y_1",
    5, "K_1",
    4, "X_1",
    3, "X_1",
    28, "S_1",
    29, "Q_1",
    30, "Q_1",
    31, "S_1",
    39, "D_1",
    42, "R_1",
    40, "W_1",
    56, "X_1",
    57, "X_1",
    65, "T_1",
    69, "S_1",
};

PoseTableEntry unitAtomicBoo_pose_table_hidden[] = {
    1, "A_1",
    2, "A_1",
    9, "A_1",
    5, "A_1",
    4, "A_1",
    3, "A_1",
    28, "A_1",
    29, "Q_1",
    30, "Q_1",
    31, "A_1",
    39, "D_1",
    42, "A_1",
    40, "A_1",
    56, "A_1",
    57, "A_1",
    65, "T_1",
    69, "A_1",
};

DataTableEntry unitAtomicBoo_data_table[] = {
    0, nullptr,
};

const ExtEntryData unitAtomicBoo_ext_entry_data[] = {
    { "c_teresa", "S_1", 0.0f },
    { "c_teresa", "S_1", 2.0f },
    { "c_teresa", "S_1", 4.0f },
    { "c_teresa", "S_1", 6.0f },
    { "c_teresa", "S_1", 8.0f },
    { "c_teresa", "S_1", 10.0f },
    { "c_teresa", "S_1", 6.0f },
    { "c_teresa", "S_1", 7.0f },
    { "c_teresa", "A_1", 0.0f },
    { "c_teresa", "A_2", 6.0f },
    { "c_teresa", "A_2", 7.0f },
    { "c_teresa", "A_2", 8.0f },
    { "c_teresa", "A_2", 9.0f },
    { "c_teresa", "A_2", 10.0f },
    { "c_teresa", "A_2", 11.0f },
    { "c_teresa", "A_2", 12.0f },
    { nullptr, nullptr, 0.0f },
};

const ExtEntryData unitCosmicBoo_ext_entry_data[] = {
    { "c_teresa_p", "S_1", 0.0f },
    { "c_teresa_p", "S_1", 2.0f },
    { "c_teresa_p", "S_1", 4.0f },
    { "c_teresa_p", "S_1", 6.0f },
    { "c_teresa_p", "S_1", 8.0f },
    { "c_teresa_p", "S_1", 10.0f },
    { "c_teresa_p", "S_1", 6.0f },
    { "c_teresa_p", "S_1", 7.0f },
    { "c_teresa_p", "A_1", 0.0f },
    { "c_teresa_p", "A_2", 6.0f },
    { "c_teresa_p", "A_2", 7.0f },
    { "c_teresa_p", "A_2", 8.0f },
    { "c_teresa_p", "A_2", 9.0f },
    { "c_teresa_p", "A_2", 10.0f },
    { "c_teresa_p", "A_2", 11.0f },
    { "c_teresa_p", "A_2", 12.0f },
    { nullptr, nullptr, 0.0f },
};

BattleWeapon unitAtomicBoo_weaponBodySlam = {
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
        AttackTargetClass_Flags::MULTIPLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_SELECT_PARTS |
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
        AttackSpecialProperty_Flags::FLIPS_SHELLED |
        AttackSpecialProperty_Flags::FREEZE_BREAK,
    .counter_resistance_flags = 
        AttackCounterResistance_Flags::PREEMPTIVE_SPIKY |
        AttackCounterResistance_Flags::FRONT_SPIKY |
        AttackCounterResistance_Flags::ICY |
        AttackCounterResistance_Flags::POISON,
    .target_weighting_flags = 0,
        
    // status chances
    
    .attack_evt_code = nullptr,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 50,
    .bg_a2_fall_weight = 50,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 0,
    .nozzle_turn_chance = 0,
    .nozzle_fire_chance = 0,
    .ceiling_fall_chance = 0,
    .object_fall_chance = 0,
};

BattleWeapon unitAtomicBoo_weaponCircle = {
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
    .damage_function_params = { 5, 0, 0, 0, 0, 0, 0, 0 },
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
        
    // status chances
    .stop_chance = 50,
    .stop_time = 3,
    
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

BattleWeapon unitAtomicBoo_weaponSurprise = {
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
    .damage_function_params = { 5, 0, 0, 0, 0, 0, 0, 0 },
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
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::DEFENSE_PIERCING |
        AttackSpecialProperty_Flags::DIMINISHING_BY_TARGET,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = 0,
        
    // status chances
    .confuse_chance = 50,
    .confuse_time = 3,
    
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

BattleWeapon unitAtomicBoo_weaponBreath = {
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
        AttackTargetClass_Flags::MULTIPLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_SELECT_PARTS |
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
        AttackSpecialProperty_Flags::DIMINISHING_BY_HIT |
        AttackSpecialProperty_Flags::FREEZE_BREAK,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
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

BattleUnitKindPart unitAtomicBoo_parts = {
    .index = 1,
    .name = "btl_un_atmic_teresa",
    .model_name = "c_atmic_trs",
    .part_offset_pos = { 0.0f, 0.0f, 0.0f },
    .part_hit_base_offset = { 10.0f, 80.0f, 0.0f },
    .part_hit_cursor_base_offset = { 0.0f, 110.0f, 0.0f },
    .unk_30 = 20,
    .unk_32 = 30,
    .base_alpha = 255,
    .defense = unitAtomicBoo_defense,
    .defense_attr = unitAtomicBoo_defense_attr,
    .attribute_flags = 0x0060'0009,
    .counter_attribute_flags = 0,
    .pose_table = unitAtomicBoo_pose_table,
};

BattleUnitKindPart unitCosmicBoo_parts = {
    .index = 1,
    .name = "btl_un_hyper_sinnosuke",       // Replaces Hyper Bald Cleft.
    .model_name = "c_atmic_trs_p",
    .part_offset_pos = { 0.0f, 0.0f, 0.0f },
    .part_hit_base_offset = { 10.0f, 80.0f, 0.0f },
    .part_hit_cursor_base_offset = { 0.0f, 110.0f, 0.0f },
    .unk_30 = 20,
    .unk_32 = 30,
    .base_alpha = 255,
    .defense = unitAtomicBoo_defense,
    .defense_attr = unitAtomicBoo_defense_attr,
    .attribute_flags = 0x0060'0009,
    .counter_attribute_flags = 0,
    .pose_table = unitAtomicBoo_pose_table,
};

EVT_BEGIN(unitAtomicBoo_face_hide_event)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_ATMTLSA_MOVE2"), EVT_NULLPTR, 0, LW(15))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 60)
    SUB(LW(1), 10)
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 60, 10, 4, 0, -1)
    WAIT_FRM(20)
    USER_FUNC(evt_snd_sfxoff, LW(15))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_ATMTLSA_FACE_HIDE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_1"))
    USER_FUNC(btlevtcmd_SetPartsRotate, -2, 1, 0, 180, 0)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 20, 0, 5, 0, -1)
    WAIT_FRM(10)
    DO(15)
        WAIT_FRM(1)
        USER_FUNC(btlevtcmd_AddPartsRotate, -2, 1, 0, 12, 0)
    WHILE()
    WAIT_FRM(20)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_FaceHidden, 1)
    USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitAtomicBoo_pose_table_hidden))
    RETURN()
EVT_END()

EVT_BEGIN(unitAtomicBoo_body_slam_event)
    SET(LW(12), PTR(&unitAtomicBoo_weaponBodySlam))
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(12))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(12), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            RETURN()
        END_IF()
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(12))
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, -1)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_ATMTLSA_MOVE2"), EVT_NULLPTR, 0, LW(15))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("R_1"))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    SUB(LW(2), 10)
    USER_FUNC(btlevtcmd_DivePosition, -2, -125, 100, LW(2), 120, 10, 4, 0, -1)
    USER_FUNC(evt_snd_sfxoff, LW(15))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("Z_1"))
    USER_FUNC(btlevtcmd_DivePosition, -2, -125, 0, LW(2), 30, 0, 1, 0, -1)
    USER_FUNC(evt_btl_camera_shake_h, 0, 10, 0, 60, 0)
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    SET(LW(10), 0)
LBL(0)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), LW(12), 0, LW(5))
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
            CASE_END()
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
    // DAS fixed.
    IF_EQUAL(LW(10), 0)
        USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(12))
        ADD(LW(10), 1)
    END_IF()
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 256, LW(5))
    GOTO(97)
LBL(97)
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    IF_NOT_EQUAL(LW(3), -1)
        GOTO(0)
    END_IF()
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
LBL(98)
    WAIT_FRM(60)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("W_1"))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 60, 50, 6, 0, -1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_1"))
    WAIT_MSEC(250)
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitAtomicBoo_surprise_event)
    SET(LW(12), PTR(&unitAtomicBoo_weaponSurprise))
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(12))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(12), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            RETURN()
        END_IF()
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(12))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_ATMTLSA_FACE_HIDE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_SetPartsRotate, -2, 1, 0, 180, 0)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 30)
    SUB(LW(1), 10)
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 60, 10, 4, 0, -1)
    USER_FUNC(evt_btl_camera_set_mode, 0, 8)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, LW(3))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_FaceHidden, 0)
    USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitAtomicBoo_pose_table))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_2"))
    USER_FUNC(btlevtcmd_SetPartsRotate, -2, 1, 0, 0, 0)
    USER_FUNC(btlevtcmd_DivePosition, -2, 0, LW(1), LW(2), 10, 0, 5, 0, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_ATMTLSA_SURPRISED1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(evt_btl_camera_shake_w, 0, 10, 0, 60, 0)
    USER_FUNC(evt_btl_camera_shake_h, 0, 10, 0, 60, 4)
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    SET(LW(10), 0)
LBL(0)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), LW(12), 0, LW(5))
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
            CASE_END()
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
    // DAS fixed.
    IF_EQUAL(LW(10), 0)
        USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(12))
        ADD(LW(10), 1)
    END_IF()
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 256, LW(5))
    GOTO(97)
LBL(97)
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    IF_NOT_EQUAL(LW(3), -1)
        GOTO(0)
    END_IF()
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
LBL(98)
    WAIT_FRM(90)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("W_1"))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 60, -20, 6, 0, -1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_1"))
    WAIT_MSEC(250)
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitAtomicBoo_circle_event)
    SET(LW(12), PTR(&unitAtomicBoo_weaponCircle))
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(12))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(12), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            RETURN()
        END_IF()
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(12))
    USER_FUNC(evtTot_AtomicBoo_CircleAppear)
    WAIT_FRM(75)
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, LW(3), -1)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 5)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    WAIT_FRM(20)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_ATMTLSA_S_ROUND1"), EVT_NULLPTR, 0, LW(15))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_1"))
    USER_FUNC(evtTot_AtomicBoo_CircleWait1)
    USER_FUNC(evt_snd_sfxoff, LW(15))
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_ATMTLSA_SURPRISED2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 350)
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    SET(LW(10), 0)
LBL(0)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), LW(12), 0, LW(5))
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
    // DAS fixed.
    IF_EQUAL(LW(10), 0)
        USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(12))
        ADD(LW(10), 1)
    END_IF()
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 256, LW(5))
    GOTO(97)
LBL(97)
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    IF_NOT_EQUAL(LW(3), -1)
        GOTO(0)
    END_IF()
LBL(98)
    WAIT_FRM(30)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    WAIT_FRM(60)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_ATMTLSA_WHISTLE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_4"))
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_1"))
    USER_FUNC(evtTot_AtomicBoo_CircleWait2)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_ATMTLSA_UNION1"), EVT_NULLPTR, 0, LW(15))
    DO(3)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        ADD(LW(1), 40)
        ADD(LW(2), 1)
        USER_FUNC(evt_eff, PTR(""), PTR("kemuri_test"), 8, LW(0), LW(1), LW(2), FLOAT(1.25), 0, 0, 0, 0, 0, 0, 0)
        WAIT_FRM(30)
    WHILE()
    USER_FUNC(evt_snd_sfxoff, LW(15))
    WAIT_MSEC(250)
    USER_FUNC(btlevtcmd_SetScale, -2, FLOAT(1.0), FLOAT(1.0), FLOAT(1.0))
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitAtomicBoo_breath_event_sub)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), LW(12), 0, LW(5))
    SWITCH(LW(5))
        CASE_EQUAL(3)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 38)
            GOTO(90)
        CASE_EQUAL(6)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 39)
            GOTO(90)
        CASE_EQUAL(2)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 40)
            GOTO(90)
        CASE_EQUAL(1)
            GOTO(91)
    END_SWITCH()
LBL(90)
    GOTO(99)
LBL(91)
    // Use an arbitrary unused global variable to track how many checks have
    // taken place; just setting this here could cause race-condition issues,
    // but for this particular attack hits happen ~simultaneously for all
    // targets so it shouldn't be an issue.
    SET((int32_t)GSW_Battle_AtomicBoo_BreathGuardCount, 0)
    
    WAIT_FRM(15)
    IF_SMALL((int32_t)GSW_Battle_AtomicBoo_BreathGuardCount, 1)
        USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(12))
        SET((int32_t)GSW_Battle_AtomicBoo_BreathGuardCount, 1)
    END_IF()
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 0, LW(5))
        
    WAIT_FRM(30)
    IF_SMALL((int32_t)GSW_Battle_AtomicBoo_BreathGuardCount, 2)
        USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(12))
        SET((int32_t)GSW_Battle_AtomicBoo_BreathGuardCount, 2)
    END_IF()
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 0, LW(5))
    
    WAIT_FRM(30)
    IF_SMALL((int32_t)GSW_Battle_AtomicBoo_BreathGuardCount, 3)
        USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(12))
        SET((int32_t)GSW_Battle_AtomicBoo_BreathGuardCount, 3)
    END_IF()
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 0x100, LW(5))
        
LBL(99)
    RETURN()
EVT_END()

EVT_BEGIN(unitAtomicBoo_breath_event)
    SET(LW(12), PTR(&unitAtomicBoo_weaponBreath))
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(12))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(12), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            RETURN()
        END_IF()
        GOTO(99)
    END_IF()
    USER_FUNC(evtTot_AtomicBoo_SetTarget, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(12))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_ATMTLSA_FACE_HIDE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_SetPartsRotate, -2, 1, 0, 180, 0)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 30)
    SUB(LW(1), 10)
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 60, 10, 4, 0, -1)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_FaceHidden, 0)
    USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitAtomicBoo_pose_table))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_3"))
    USER_FUNC(btlevtcmd_SetPartsRotate, -2, 1, 0, 0, 0)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 10, 0, 5, 0, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_ATMTLSA_BLOW2"), EVT_NULLPTR, 0, LW(15))
    USER_FUNC(evtTot_AtomicBoo_BreathAppear)
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
LBL(10)
    IF_EQUAL(LW(3), -1)
        GOTO(20)
    END_IF()
    RUN_EVT(PTR(&unitAtomicBoo_breath_event_sub))
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    GOTO(10)
LBL(20)
    WAIT_FRM(120)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evtTot_AtomicBoo_BreathWait1)
    USER_FUNC(evt_snd_sfxoff, LW(15))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_ATMTLSA_WHISTLE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_4"))
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_1"))
    USER_FUNC(evtTot_AtomicBoo_BreathWait2)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BOSS_ATMTLSA_UNION1"), EVT_NULLPTR, 0, LW(15))
    DO(3)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        ADD(LW(1), 40)
        ADD(LW(2), 1)
        USER_FUNC(evt_eff, PTR(""), PTR("kemuri_test"), 8, LW(0), LW(1), LW(2), FLOAT(1.25), 0, 0, 0, 0, 0, 0, 0)
        WAIT_FRM(30)
    WHILE()
    USER_FUNC(evt_snd_sfxoff, LW(15))
    WAIT_MSEC(1500)
    USER_FUNC(btlevtcmd_SetScale, -2, FLOAT(1.0), FLOAT(1.0), FLOAT(1.0))
    RETURN()
EVT_END()

EVT_BEGIN(unitAtomicBoo_attack_event)
    USER_FUNC(btlevtcmd_EnemyItemUseCheck, -2, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(LW(0))
        USER_FUNC(btlevtcmd_StartWaitEvent, -2)
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_FaceHidden, LW(0))
    IF_EQUAL(LW(0), 0)
        SET(LW(1), 30)
        ADD(LW(1), 20)
        ADD(LW(1), 20)
        SUB(LW(1), 1)
        USER_FUNC(evt_sub_random, LW(1), LW(0))
        SET(LW(1), 30)
        IF_SMALL(LW(0), LW(1))
            RUN_CHILD_EVT(PTR(&unitAtomicBoo_face_hide_event))
            USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(5))
            IF_EQUAL(LW(5), (int32_t)BattleUnitType::ATOMIC_BOO)
                GOTO(99)
            ELSE()
                // Cosmic Boo can charge and attack on the same turn.
                WAIT_MSEC(400)
                GOTO(50)
            END_IF()
        END_IF()
        ADD(LW(1), 20)
        IF_SMALL(LW(0), LW(1))
            RUN_CHILD_EVT(PTR(&unitAtomicBoo_body_slam_event))
            GOTO(99)
        END_IF()
        RUN_CHILD_EVT(PTR(&unitAtomicBoo_circle_event))
        GOTO(99)
    ELSE()
LBL(50)
        SET(LW(1), 50)
        ADD(LW(1), 50)
        SUB(LW(1), 1)
        USER_FUNC(evt_sub_random, LW(1), LW(0))
        SET(LW(1), 50)
        IF_SMALL(LW(0), LW(1))
            RUN_CHILD_EVT(PTR(&unitAtomicBoo_surprise_event))
            GOTO(99)
        END_IF()
        RUN_CHILD_EVT(PTR(&unitAtomicBoo_breath_event))
        GOTO(99)
    END_IF()
LBL(99)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitAtomicBoo_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    RETURN()
EVT_END()

EVT_BEGIN(unitAtomicBoo_wait_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitAtomicBoo_init_ext_event)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(0))
    IF_EQUAL(LW(0), (int32_t)BattleUnitType::ATOMIC_BOO)
        USER_FUNC(evt_npc_entry, PTR("ext_teresa"), PTR("c_teresa"))
        USER_FUNC(evt_npc_set_position, PTR("ext_teresa"), 0, -1000, 0)
        USER_FUNC(
            evt_ext_entry, 30, PTR(&unitAtomicBoo_ext_entry_data), 
            PTR(&ext_boo_init), PTR(&ext_boo_main), 0)
    ELSE()
        USER_FUNC(evt_npc_entry, PTR("ext_teresa"), PTR("c_teresa_p"))
        USER_FUNC(evt_npc_set_position, PTR("ext_teresa"), 0, -1000, 0)
        USER_FUNC(
            evt_ext_entry, 30, PTR(&unitCosmicBoo_ext_entry_data), 
            PTR(&ext_boo_init), PTR(&ext_boo_main), 0)
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitAtomicBoo_common_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitAtomicBoo_wait_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitAtomicBoo_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitAtomicBoo_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&btldefaultevt_Confuse))
    USER_FUNC(evtTot_AtomicBoo_SetUnitId, -2)
    RUN_CHILD_EVT(PTR(&unitAtomicBoo_init_ext_event))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitAtomicBoo_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType,
        (int32_t)BattleUnitType::ATOMIC_BOO)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_FaceHidden, 0)
    RUN_CHILD_EVT(PTR(&unitAtomicBoo_common_init_event))
    RETURN()
EVT_END()

EVT_BEGIN(unitCosmicBoo_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType,
        (int32_t)BattleUnitType::TOT_COSMIC_BOO)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_FaceHidden, 0)
    RUN_CHILD_EVT(PTR(&unitAtomicBoo_common_init_event))
    RETURN()
EVT_END()

BattleUnitKind unit_AtomicBoo = {
    .unit_type = BattleUnitType::ATOMIC_BOO,
    .unit_name = "btl_un_atmic_teresa",
    .max_hp = 40,
    .max_fp = 0,
    .danger_hp = 1,
    .peril_hp = 1,
    .level = 52,
    .bonus_exp = 0,
    .bonus_coin = 1,
    .bonus_coin_rate = 100,
    .base_coin = 0,
    .run_rate = 50,
    // Intentionally raised from 6 to make Power Bounce / Multibonk more viable.
    .pb_soft_cap = 9,
    .width = 120,
    .height = 110,
    .hit_offset = { 30, 105 },
    .center_offset = { 0.0f, 55.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 60.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 60.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 46.0f, 71.5f, 10.0f },
    .cut_base_offset = { 0.0f, 55.0f, 0.0f },
    .cut_width = 120.0f,
    .cut_height = 110.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = -1,
    .swallow_attributes = 0,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_BOSS_ATMTLSA_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0x0000'0004,
    .status_vulnerability = &unitAtomicBoo_status,
    .num_parts = 1,
    .parts = &unitAtomicBoo_parts,
    .init_evt_code = (void*)unitAtomicBoo_init_event,
    .data_table = unitAtomicBoo_data_table,
};

BattleUnitKind unit_CosmicBoo = {
    .unit_type = BattleUnitType::TOT_COSMIC_BOO,
    .unit_name = "btl_un_hyper_sinnosuke",  // Replaces Hyper Bald Cleft.
    .max_hp = 40,
    .max_fp = 0,
    .danger_hp = 1,
    .peril_hp = 1,
    .level = 52,
    .bonus_exp = 0,
    .bonus_coin = 1,
    .bonus_coin_rate = 100,
    .base_coin = 0,
    .run_rate = 50,
    .pb_soft_cap = 7,
    .width = 120,
    .height = 110,
    .hit_offset = { 30, 105 },
    .center_offset = { 0.0f, 55.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 60.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 60.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 46.0f, 71.5f, 10.0f },
    .cut_base_offset = { 0.0f, 55.0f, 0.0f },
    .cut_width = 120.0f,
    .cut_height = 110.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = -1,
    .swallow_attributes = 0,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_BOSS_ATMTLSA_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0x0000'0004,
    .status_vulnerability = &unitCosmicBoo_status,
    .num_parts = 1,
    .parts = &unitCosmicBoo_parts,
    .init_evt_code = (void*)unitCosmicBoo_init_event,
    .data_table = unitAtomicBoo_data_table,
};

// Global state.
ExtBooWork* g_BooWork;
int32_t g_AtomicBooUnitId;
int32_t g_TargetUnitId;
int32_t g_TargetPartId;
int32_t g_BooCircleAngle;
float   g_AtomicBooScale = 1.0f;

void ext_boo_disp(CameraId camera, void* user_data) {
    ttyd::extdrv::extLoadRenderMode();
    ttyd::extdrv::extLoadVertex();
    ttyd::extdrv::extLoadTexture();
    ttyd::extdrv::extLoadTev();
    ttyd::extdrv::extDraw();
    ttyd::extdrv::extLoadTextureExit();
}

void ext_boo_init() {
    g_BooWork = (ExtBooWork*)ttyd::battle::BattleAlloc(sizeof(ExtBooWork) * 30);
    auto* poseWork = (ExtPoseWork*)ttyd::extdrv::extGetPosePtr();
    
    for (int32_t i = 0; i < ttyd::extdrv::extGetPoseNum(); ++i) {
        poseWork[i].flags = 1;
        poseWork[i].facing_dir = 90.0f;
        
        gc::mtx::PSMTXIdentity(poseWork[i].mtx);
        poseWork[i].mtx->m[0][3] = 0.0f;
        poseWork[i].mtx->m[1][3] = -1000.0f;
        poseWork[i].mtx->m[2][3] = 0.0f;
        
        g_BooWork[i].state = 0;
        g_BooWork[i].surprise_animation = false;
        g_BooWork[i].anim_timer = 0;
        
        g_BooWork[i].unk_0x10.x = 0.0f;
        g_BooWork[i].unk_0x10.y = 0.0f;
        g_BooWork[i].unk_0x10.z = 0.0f;
    }
}

void ext_boo_main() {
    auto* extPoseWork = (ExtPoseWork*)ttyd::extdrv::extGetPosePtr();
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, g_AtomicBooUnitId);
    auto* target = ttyd::battle::BattleGetUnitPtr(g_BattleWork, g_TargetUnitId);
    BattleWorkUnitPart* part = nullptr;
    
    // Delays for individual Boos spawning / retreating for circle attack.
    const int32_t kDelayFrames[] = { 0, 2, 5, 7, 10, 12, 15, 17 };
    // X,Y offsets from target hitpos for breath attack.
    float kBreathOffsetsXY[] = {
        -30.0,   8.0, -28.0,  10.0, -26.0,   6.0, -24.0,  30.0,
        -22.0,  16.0, -20.0,  50.0, -18.0,  14.0, -16.0,  18.0,
        -14.0,  60.0, -12.0,  34.0, -10.0,  66.0,  -8.0,   8.0,
         -6.0,  28.0,  -4.0,  40.0,  -2.0,  46.0,   0.0,  20.0,
          2.0,  26.0,   4.0,  52.0,   6.0,  74.0,   8.0,  68.0,
         10.0,   0.0,  12.0,  28.0,  14.0,  44.0,  16.0,  72.0,
         18.0,  42.0,  20.0,  76.0,  22.0,  56.0,  24.0,  32.0,
         26.0,   4.0,  28.0,   2.0 
    };
    // Easing modes for breath attack; randomly selected per Boo.
    int8_t kEaseModes[] = { 0, 1, 2, 3, 4, 5, 6 };
    
    if (++g_BooCircleAngle >= 360) g_BooCircleAngle -= 360;
    
    // Atomic Boo's current position.
    gc::vec3 pos;
    // The target's hit position.
    gc::vec3 hitpos;
    
    if (unit) {
        ttyd::battle_unit::BtlUnit_GetPos(unit, &pos.x, &pos.y, &pos.z);
        pos.y += 40.0f;
        
        // Interpolate Atomic Boo's size smoothly.
        gc::vec3 scale;
        ttyd::battle_unit::BtlUnit_GetScale(unit, &scale.x, &scale.y, &scale.z);
        if (g_AtomicBooScale < scale.x) {
            scale.x -= 0.005f;
            if (scale.x <= g_AtomicBooScale) scale.x = g_AtomicBooScale;
            ttyd::battle_unit::BtlUnit_SetScale(unit, scale.x, scale.x, scale.z);
        } else if (g_AtomicBooScale > scale.x) {
            scale.x += 0.005f;
            if (scale.x >= g_AtomicBooScale) scale.x = g_AtomicBooScale;
            ttyd::battle_unit::BtlUnit_SetScale(unit, scale.x, scale.x, scale.z);
        }
    }
    
    if (target) {
        part = ttyd::battle_unit::BtlUnit_GetPartsPtr(target, g_TargetPartId);
        ttyd::battle_unit::BtlUnit_GetHitPos(
            target, part, &hitpos.x, &hitpos.y, &hitpos.z);
    }
    
    for (int32_t i = 0; i < ttyd::extdrv::extGetPoseNum(); ++i) {
        auto& booWork = g_BooWork[i];
        auto& poseWork = extPoseWork[i];
        
        if (booWork.surprise_animation) {
            int32_t val = ++booWork.anim_timer;
            if (val < 31) {
                poseWork.anim_frame = 8;
            } else if (val < 44) {
                poseWork.anim_frame = 9 + (val - 30) / 2;
            } else {
                poseWork.anim_frame = 15;
            }
        } else {
            if (++booWork.anim_timer >= 24) booWork.anim_timer -= 24;
            poseWork.anim_frame = booWork.anim_timer / 3;
        }
        
        if (booWork.delay >= 1) {
            --booWork.delay;
        } else {
            auto* mtx = poseWork.mtx;
            
            mtx->m[0][3] += booWork.unk_0x10.x;
            mtx->m[1][3] += booWork.unk_0x10.y;
            mtx->m[2][3] += booWork.unk_0x10.z;
            
            if (mtx->m[1][3] < 0.0f) mtx->m[1][3] = 0.0f;
            
            switch (booWork.state) {
                case 0: {
                    mtx->m[0][3] = 0.0f;
                    mtx->m[1][3] = -1000.0f;
                    mtx->m[2][3] = 0.0f;
                    
                    poseWork.facing_dir = 90.0f;
                    
                    mtx->m[0][0] = 1.0f;
                    mtx->m[1][1] = 1.0f;
                    break;
                }
                case 1: {
                    poseWork.flags &= ~2;
                    
                    mtx->m[0][3] = pos.x;
                    mtx->m[1][3] = pos.y;
                    mtx->m[2][3] = pos.z - 5.0f;
                    
                    mtx->m[0][0] = 0.0f;
                    mtx->m[1][1] = 0.0f;
                    
                    double angle = kPiOver4 * i;
                    booWork.unk_0x10.x = 2.0f * ttyd::_core_cos(angle);
                    booWork.unk_0x10.y = 2.0f * ttyd::_core_sin(angle);
                    booWork.unk_0x10.z = 0.0f;
                    
                    booWork.delay = kDelayFrames[i];
                    booWork.timer = 50;
                    booWork.state = 5;
                    break;
                }
                case 5: {
                    if (--booWork.timer == 25) {
                        g_AtomicBooScale -= 0.05f;
                        if (i & 1) {
                            gc::vec3 my_pos = { mtx->m[0][3], mtx->m[1][3], mtx->m[2][3] };
                            ttyd::battle_unit::BtlUnit_snd_se_pos(
                                unit, "SFX_BOSS_ATMTLSA_DIVIDE1", EVT_NULLPTR, 0, &my_pos);
                        }
                    }
                    
                    mtx->m[0][0] += 0.02f;
                    mtx->m[1][1] += 0.02f;
                    
                    if (booWork.timer < 1) {
                        poseWork.flags |= 2;
                        
                        booWork.state = 10;
                        booWork.timer = 150;
                        booWork.unk_0x10.x = 0.0f;
                        booWork.unk_0x10.y = 0.0f;
                        booWork.unk_0x10.z = 0.0f;
                        
                        if (i & 1) {
                            gc::vec3 my_pos = { mtx->m[0][3], mtx->m[1][3], mtx->m[2][3] };
                            ttyd::battle_unit::BtlUnit_snd_se_pos(
                                unit, "SFX_BOSS_ATMTLSA_S_MOVE1", EVT_NULLPTR, 0, &my_pos);
                        }
                    }
                    break;
                }
                case 10: {
                    double angle = k2Pi * (0.125 * i + 2.0 * g_BooCircleAngle / 360.0);
                    
                    booWork.unk_0x28.x = 90.0 * ttyd::_core_cos(angle) - 100.0;
                    booWork.unk_0x28.y = 10.0f;
                    booWork.unk_0x28.z = 90.0 * ttyd::_core_sin(angle);
                    
                    if (mtx->m[0][3] < -100.0f) {
                        poseWork.facing_dir = 270.0f;
                    } else {
                        poseWork.facing_dir = 90.0f;
                    }
                    
                    booWork.unk_0x10.x = (booWork.unk_0x28.x - mtx->m[0][3]) / 20.0;
                    booWork.unk_0x10.y = (booWork.unk_0x28.y - mtx->m[1][3]) / 20.0;
                    booWork.unk_0x10.z = (booWork.unk_0x28.z - mtx->m[2][3]) / 20.0;
                    
                    if (booWork.timer-- < 1) {
                        booWork.state = 15;
                        booWork.unk_0x10.x = 0.0f;
                        booWork.unk_0x10.y = 0.0f;
                        booWork.unk_0x10.z = 0.0f;
                    }
                    break;
                }
                case 15: {
                    double angle = k2Pi * (0.125 * i + 2.0 * g_BooCircleAngle / 360.0);
                    
                    booWork.unk_0x28.x = 90.0 * ttyd::_core_cos(angle) - 100.0;
                    booWork.unk_0x28.y = 10.0f;
                    booWork.unk_0x28.z = 90.0 * ttyd::_core_sin(angle);
                    
                    if (mtx->m[0][3] < -100.0f) {
                        poseWork.facing_dir = 270.0f;
                    } else {
                        poseWork.facing_dir = 90.0f;
                    }
                    
                    booWork.unk_0x10.x = (booWork.unk_0x28.x - mtx->m[0][3]) / 20.0;
                    booWork.unk_0x10.y = (booWork.unk_0x28.y - mtx->m[1][3]) / 20.0;
                    booWork.unk_0x10.z = (booWork.unk_0x28.z - mtx->m[2][3]) / 20.0;
                    
                    // Wait for all Boos to reach state, then transition to 20.
                    int32_t i = 0;
                    for (; i < 8; ++i) {
                        if (g_BooWork[i].state != 15) break;
                    }
                    if (i == 8) {
                        for (int32_t i = 0; i < 8; ++i) {
                            g_BooWork[i].state = 20;
                            g_BooWork[i].surprise_animation = true;
                            g_BooWork[i].anim_timer = 0;
                            g_BooWork[i].unk_0x10.x = 0.0f;
                            g_BooWork[i].unk_0x10.y = 0.0f;
                            g_BooWork[i].unk_0x10.z = 0.0f;
                        }
                    }                    
                    
                    break;
                }
                case 20: {
                    if (booWork.anim_timer >= 180) {
                        booWork.state = 23;
                        booWork.surprise_animation = false;
                        booWork.anim_timer = 0;
                        booWork.delay = kDelayFrames[i];
                        booWork.timer = 60;
                        
                        poseWork.facing_dir = 90.0f;
                    }
                    break;
                }
                case 23: {
                    booWork.unk_0x28.x = pos.x - 35.0f + ttyd::system::irand(90);
                    booWork.unk_0x28.y = pos.y - 35.0f + ttyd::system::irand(90);
                    booWork.unk_0x28.z = pos.z - 30.0f - 0.05 * i;
                
                    booWork.state = 25;
                    booWork.timer = 60;
                    
                    if (i & 1) {
                        gc::vec3 my_pos = { mtx->m[0][3], mtx->m[1][3], mtx->m[2][3] };
                        ttyd::battle_unit::BtlUnit_snd_se_pos(
                            unit, "SFX_BOSS_ATMTLSA_S_MOVE1", EVT_NULLPTR, 0, &my_pos);
                    }
                    
                    break;
                }
                case 25: {
                    booWork.unk_0x10.x = (booWork.unk_0x28.x - mtx->m[0][3]) / 20.0;
                    booWork.unk_0x10.y = (booWork.unk_0x28.y - mtx->m[1][3]) / 20.0;
                    booWork.unk_0x10.z = (booWork.unk_0x28.z - mtx->m[2][3]) / 20.0;
                    
                    if (booWork.timer-- < 1) {
                        booWork.state = 30;
                        booWork.unk_0x10.x = 0.0f;
                        booWork.unk_0x10.y = 0.0f;
                        booWork.unk_0x10.z = 0.0f;
                        
                        poseWork.flags &= ~2;
                    }
                    
                    break;
                }
                case 31: {
                    mtx->m[0][0] -= 0.0125;
                    mtx->m[1][1] = mtx->m[0][0];
                    if (mtx->m[0][0] <= 0.0f) booWork.state = 0;
                    
                    break;
                }
                case 51: {
                    poseWork.flags &= ~2;
                    
                    mtx->m[0][3] = pos.x - 15.0f;
                    mtx->m[1][3] = pos.y - 5.0f;
                    mtx->m[2][3] = pos.z + 5.0f;
                    mtx->m[0][0] = 0.0f;
                    mtx->m[1][1] = 0.0f;
                    
                    booWork.unk_0x10.x = 0.0f;
                    booWork.unk_0x10.y = 0.0f;
                    booWork.unk_0x10.z = 0.0f;
                    booWork.unk_0x1c.x = mtx->m[0][3];
                    booWork.unk_0x1c.y = mtx->m[1][3];
                    booWork.unk_0x1c.z = mtx->m[2][3];
                    booWork.unk_0x28.x =
                        hitpos.x + kBreathOffsetsXY[2 * i] - 200.0f;
                    booWork.unk_0x28.y =
                        hitpos.y + kBreathOffsetsXY[2 * i + 1] - 50.0f;
                    booWork.unk_0x28.z = hitpos.z - 10.0f - 0.05 * i;
                    
                    booWork.delay = i * 3;
                    booWork.state = 53;
                    booWork.timer = 0;
                    
                    booWork.ease_mode = kEaseModes[ttyd::system::irand(7)];
                    
                    break;
                }
                case 53: {
                    g_AtomicBooScale -= 0.015;
                    booWork.state = 55;
                    break;
                }
                case 55: {
                    mtx->m[0][0] += 0.04;
                    if (mtx->m[0][0] >= 1.0) mtx->m[0][0] = 1.0f;
                    mtx->m[1][1] = mtx->m[0][0];
                    
                    mtx->m[0][3] = ttyd::system::intplGetValue(
                        booWork.unk_0x1c.x, booWork.unk_0x28.x,
                        booWork.ease_mode, booWork.timer, 30);
                    mtx->m[1][3] = ttyd::system::intplGetValue(
                        booWork.unk_0x1c.y, booWork.unk_0x28.y,
                        booWork.ease_mode, booWork.timer, 30);
                    mtx->m[2][3] = ttyd::system::intplGetValue(
                        booWork.unk_0x1c.z, booWork.unk_0x28.z,
                        booWork.ease_mode, booWork.timer, 30);
                        
                    if (++booWork.timer > 30) booWork.state = 60;
                    
                    break;
                }
                case 63: {
                    booWork.unk_0x28.x = pos.x - 35.0f + ttyd::system::irand(90);
                    booWork.unk_0x28.y = pos.y - 35.0f + ttyd::system::irand(90);
                    booWork.unk_0x28.z = pos.z - 30.0f - 0.05 * i;
                
                    booWork.state = 65;
                    booWork.timer = 60;
                    
                    if (i & 1) {
                        gc::vec3 my_pos = { mtx->m[0][3], mtx->m[1][3], mtx->m[2][3] };
                        ttyd::battle_unit::BtlUnit_snd_se_pos(
                            unit, "SFX_BOSS_ATMTLSA_S_MOVE1", EVT_NULLPTR, 0, &my_pos);
                    }
                    
                    break;
                }
                case 65: {
                    booWork.unk_0x10.x = (booWork.unk_0x28.x - mtx->m[0][3]) / 15.0;
                    booWork.unk_0x10.y = (booWork.unk_0x28.y - mtx->m[1][3]) / 15.0;
                    booWork.unk_0x10.z = (booWork.unk_0x28.z - mtx->m[2][3]) / 15.0;
                    
                    if (booWork.timer-- < 1) {
                        booWork.state = 70;
                        booWork.unk_0x10.x = 0.0f;
                        booWork.unk_0x10.y = 0.0f;
                        booWork.unk_0x10.z = 0.0f;
                    }
                    
                    break;
                }
                case 71: {
                    mtx->m[0][0] -= 0.0125;
                    mtx->m[1][1] = mtx->m[0][0];
                    if (mtx->m[0][0] <= 0.0f) booWork.state = 0;
                    
                    break;
                }
            }
        }
    }
    
    ttyd::dispdrv::dispEntry(CameraId::k3d, 1, 0.0f, ext_boo_disp, nullptr);
}

// arg0 - in unit id
EVT_DEFINE_USER_FUNC(evtTot_AtomicBoo_SetUnitId) {
    int32_t unit_id = evtGetValue(evt, evt->evtArguments[0]);
    g_AtomicBooUnitId = ttyd::battle_sub::BattleTransID(evt, unit_id);
    return 2;
}

// arg0, arg1 - in unit id, part id
EVT_DEFINE_USER_FUNC(evtTot_AtomicBoo_SetTarget) {
    int32_t unit_id = evtGetValue(evt, evt->evtArguments[0]);
    g_TargetUnitId = ttyd::battle_sub::BattleTransID(evt, unit_id);
    g_TargetPartId = evtGetValue(evt, evt->evtArguments[1]);
    return 2;
}

// no args
EVT_DEFINE_USER_FUNC(evtTot_AtomicBoo_CircleAppear) {
    for (int32_t i = 0; i < 8; ++i) g_BooWork[i].state = 1;
    return 2;
}

// no args
EVT_DEFINE_USER_FUNC(evtTot_AtomicBoo_CircleWait1) {
    for (int32_t i = 0; i < 8; ++i) {
        if (g_BooWork[i].state != 15) return 0;
    }
    return 2;
}

// no args
EVT_DEFINE_USER_FUNC(evtTot_AtomicBoo_CircleWait2) {
    for (int32_t i = 0; i < 8; ++i) {
        if (g_BooWork[i].state != 30) return 0;
    }
    for (int32_t i = 0; i < 8; ++i) g_BooWork[i].state = 31;
    g_AtomicBooScale = 1.0f;
    return 2;
}

// no args (original uses constant 0)
EVT_DEFINE_USER_FUNC(evtTot_AtomicBoo_BreathAppear) {
    for (int32_t i = 0; i < ttyd::extdrv::extGetPoseNum(); ++i) {
        g_BooWork[i].state = 51;
    }
    return 2;
}

// no args
EVT_DEFINE_USER_FUNC(evtTot_AtomicBoo_BreathWait1) {
    for (int32_t i = 0; i < 30; ++i) {
        if (g_BooWork[i].state != 60) return 0;
    }
    for (int32_t i = 0; i < 30; ++i) {
        g_BooWork[i].state = 63;
        g_BooWork[i].delay = 30;
    }
    return 2;
}

// no args
EVT_DEFINE_USER_FUNC(evtTot_AtomicBoo_BreathWait2) {
    for (int32_t i = 0; i < 30; ++i) {
        if (g_BooWork[i].state != 70) return 0;
    }
    for (int32_t i = 0; i < 30; ++i) g_BooWork[i].state = 71;
    g_AtomicBooScale = 1.0f;
    return 2;
}

}  // namespace mod::tot::custom