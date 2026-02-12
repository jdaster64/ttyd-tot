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
#include <ttyd/effdrv.h>
#include <ttyd/eff_uranoko.h>
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
using ::ttyd::effdrv::EffEntry;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;

// Constants (UW, etc.)
constexpr const int32_t UW_Rotate = 0;
constexpr const int32_t UW_RotateSpeed = 1;
constexpr const int32_t UW_ShadyEffPtr = 2;
constexpr const int32_t UW_BattleUnitType = 3;

}  // namespace

// Function / USER_FUNC declarations.

EVT_DECLARE_USER_FUNC(eff_aura, 1)

extern DataTableEntry unitKoopa_data_table[3];
extern DataTableEntry unitParatroopa_data_table[3];

// Unit data.

int8_t unitKoopa_defense[] = { 1, 1, 1, 1, 1 };
int8_t unitKoopa_defense_attr[] = { 0, 0, 0, 0, 0 };
int8_t unitKoopa_flip_defense[] = { 0, 0, 0, 0, 0 };

StatusVulnerability unitKoopa_status = {
     95, 100,  90, 100,  90, 100, 100,  90,
    100,  95, 100,  95, 100,  95,  90,  95,
     90, 100,  90, 100, 100, 100,
};
StatusVulnerability unitKpKoopa_status = {
     85,  90,  80, 100,  80, 100, 100,  80,
    100,  95, 100,  95, 100,  95,  80,  85,
     80, 100,  80, 100, 100, 100,
};
StatusVulnerability unitShadyKoopa_status = {
     85,  90,  80, 100,  80, 100, 100,  80,
    100,  90, 100,  90, 100,  90,  80,  85,
     80, 100,  80, 100, 100,  95,
};
StatusVulnerability unitDarkKoopa_status = {
     75,  80,  70, 100,  70, 100, 100,  70,
    100,  85, 100,  85, 100,  85,  80,  75,
     70, 100,  70, 100, 100,  70,
};
StatusVulnerability unitParatroopa_status = {
     95, 100, 120, 100,  90, 100, 100,  90,
    100,  95, 100,  95, 100,  95,  90,  95,
    110, 100,  90, 100, 100, 100,
};
StatusVulnerability unitKpParatroopa_status = {
     85,  90, 110, 100,  80, 100, 100,  80,
    100,  95, 100,  95, 100,  95,  80,  85,
    100, 100,  90, 100, 100, 100,
};
StatusVulnerability unitShadyParatroopa_status = {
     85,  90, 110, 100,  80, 100, 100,  80,
    100,  90, 100,  90, 100,  90,  80,  85,
    100, 100,  80, 100, 100,  95,
};
StatusVulnerability unitDarkParatroopa_status = {
     75,  80, 105, 100,  70, 100, 100,  70,
    100,  85, 100,  85, 100,  85,  80,  75,
     95, 100,  70, 100, 100,  70,
};

PoseTableEntry unitKoopa_pose_table[] = {
    1, "NKT_N_1",
    2, "NKT_Y_1",
    9, "NKT_Y_1",
    5, "NKT_K_1",
    4, "NKT_X_1",
    3, "NKT_X_1",
    28, "NKT_S_1",
    29, "NKT_Q_1",
    30, "NKT_Q_1",
    31, "NKT_S_1",
    39, "NKT_D_1",
    50, "NKT_A_1",
    42, "NKT_R_1",
    40, "NKT_W_1",
    56, "NKT_X_1",
    57, "NKT_X_1",
    65, "NKT_T_1",
    69, "NKT_S_1",
};
PoseTableEntry unitKoopa_flip_pose_table[] = {
    1, "NKT_N_2",
    2, "NKT_B_1",
    9, "NKT_B_1",
    5, "NKT_K_2",
    4, "NKT_K_2",
    3, "NKT_K_2",
    28, "NKT_S_2",
    29, "NKT_Q_1",
    30, "NKT_Q_1",
    31, "NKT_D_1",
    39, "NKT_D_1",
    42, "NKT_R_1",
    40, "NKT_W_1",
    56, "NKT_K_2",
    57, "NKT_K_2",
    65, "NKT_S_2",
    69, "NKT_S_2",
};
PoseTableEntry unitParatroopa_pose_table[] = {
    1, "PTP_N_1",
    2, "PTP_Y_1",
    9, "PTP_Y_1",
    5, "PTP_K_1",
    4, "PTP_X_1",
    3, "PTP_X_1",
    28, "PTP_S_1",
    29, "PTP_Q_1",
    30, "PTP_Q_1",
    31, "PTP_S_1",
    39, "PTP_D_1",
    50, "PTP_A_1",
    42, "PTP_R_1",
    40, "PTP_W_1",
    56, "PTP_X_1",
    57, "PTP_X_1",
    65, "PTP_T_1",
    69, "PTP_S_1",
};

const PoseSoundTimingEntry unitParatroopa_pose_sound_timing_table[] = {
    { "PTP_S_1", 0.0100000f, 0, "SFX_ENM_PATA_WAIT1", 1 },
    { "PTP_W_1", 0.0100000f, 0, "SFX_ENM_PATA_MOVE1", 1 },
    { "PTP_R_1", 0.0100000f, 0, "SFX_ENM_PATA_MOVE1", 1 },
    { "PTP_N_1", 0.0100000f, 0, "SFX_ENM_PATA_WAIT1", 1 },
    { "PTP_K_1", 0.0100000f, 0, "SFX_ENM_PATA_WAIT1", 1 },
    { "PTP_A_3", 0.0100000f, 0, "SFX_ENM_PATA_WAIT1", 1 },
    { "PTP_X_1", 0.0100000f, 0, "SFX_ENM_PATA_WAIT1", 1 },
    { nullptr, 0.0f, 0, nullptr, 1 },
};

BattleWeapon unitKoopa_weaponGround = {
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
        AttackTargetProperty_Flags::HAMMERLIKE |
        AttackTargetProperty_Flags::ONLY_FRONT,
    .element = AttackElement::NORMAL,
    .damage_pattern = 8,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::FREEZE_BREAK,
    .counter_resistance_flags =
        AttackCounterResistance_Flags::ALL &
        ~AttackCounterResistance_Flags::PAYBACK,
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
BattleWeapon unitKoopa_weaponPara = {
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
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
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
BattleWeapon unitKoopa_weaponFlipped = {
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
    .damage_function_params = { 6, 0, 0, 0, 0, 0, 0, 0 },
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
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR |
        AttackTargetProperty_Flags::SHELL_TOSS_LIKE |
        AttackTargetProperty_Flags::CANNOT_TARGET_CEILING,
    .element = AttackElement::NORMAL,
    .damage_pattern = 8,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::ALL_BUFFABLE |
        AttackSpecialProperty_Flags::FREEZE_BREAK,
    .counter_resistance_flags =
        AttackCounterResistance_Flags::ALL &
        ~AttackCounterResistance_Flags::PAYBACK,
    .target_weighting_flags =
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

BattleUnitKindPart unitKoopa_parts[] = {
    {
        .index = 1,
        .name = "btl_un_nokonoko",
        .model_name = "c_nokoteki",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 30.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 40.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitKoopa_defense,
        .defense_attr = unitKoopa_defense_attr,
        .attribute_flags = 0x0000'0009 | PartsAttribute_Flags::SHELLED,
        .counter_attribute_flags = 0,
        .pose_table = unitKoopa_pose_table,
    },
};
BattleUnitKindPart unitKpKoopa_parts[] = {
    {
        .index = 1,
        .name = "btl_un_nokonoko_fighter",
        .model_name = "c_touginoko",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 30.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 40.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitKoopa_defense,
        .defense_attr = unitKoopa_defense_attr,
        .attribute_flags = 0x0000'0009 | PartsAttribute_Flags::SHELLED,
        .counter_attribute_flags = 0,
        .pose_table = unitKoopa_pose_table,
    },
};
BattleUnitKindPart unitShadyKoopa_parts[] = {
    {
        .index = 1,
        .name = "btl_un_ura_noko",
        .model_name = "c_uranoko",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 30.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 40.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitKoopa_defense,
        .defense_attr = unitKoopa_defense_attr,
        .attribute_flags = 0x0000'0009 | PartsAttribute_Flags::SHELLED,
        .counter_attribute_flags = 0,
        .pose_table = unitKoopa_pose_table,
    },
};
BattleUnitKindPart unitDarkKoopa_parts[] = {
    {
        .index = 1,
        .name = "btl_un_yami_noko",
        .model_name = "c_yaminoko",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 30.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 40.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitKoopa_defense,
        .defense_attr = unitKoopa_defense_attr,
        .attribute_flags = 0x0000'0009 | PartsAttribute_Flags::SHELLED,
        .counter_attribute_flags = 0,
        .pose_table = unitKoopa_pose_table,
    },
};
BattleUnitKindPart unitParatroopa_parts[] = {
    {
        .index = 1,
        .name = "btl_un_patapata",
        .model_name = "c_nokoteki",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 30.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 40.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitKoopa_defense,
        .defense_attr = unitKoopa_defense_attr,
        .attribute_flags = 0x0000'0009 |
            PartsAttribute_Flags::WINGED |
            PartsAttribute_Flags::HAMMERLIKE_CANNOT_TARGET |
            PartsAttribute_Flags::SHELLTOSSLIKE_CANNOT_TARGET,
        .counter_attribute_flags = 0,
        .pose_table = unitParatroopa_pose_table,
    },
};
BattleUnitKindPart unitKpParatroopa_parts[] = {
    {
        .index = 1,
        .name = "btl_un_patapata_fighter",
        .model_name = "c_touginoko",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 30.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 40.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitKoopa_defense,
        .defense_attr = unitKoopa_defense_attr,
        .attribute_flags = 0x0000'0009 |
            PartsAttribute_Flags::WINGED |
            PartsAttribute_Flags::HAMMERLIKE_CANNOT_TARGET |
            PartsAttribute_Flags::SHELLTOSSLIKE_CANNOT_TARGET,
        .counter_attribute_flags = 0,
        .pose_table = unitParatroopa_pose_table,
    },
};
BattleUnitKindPart unitShadyParatroopa_parts[] = {
    {
        .index = 1,
        .name = "btl_un_ura_pata",
        .model_name = "c_uranoko",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 30.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 40.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitKoopa_defense,
        .defense_attr = unitKoopa_defense_attr,
        .attribute_flags = 0x0000'0009 |
            PartsAttribute_Flags::WINGED |
            PartsAttribute_Flags::HAMMERLIKE_CANNOT_TARGET |
            PartsAttribute_Flags::SHELLTOSSLIKE_CANNOT_TARGET,
        .counter_attribute_flags = 0,
        .pose_table = unitParatroopa_pose_table,
    },
};
BattleUnitKindPart unitDarkParatroopa_parts[] = {
    {
        .index = 1,
        .name = "btl_un_yami_pata",
        .model_name = "c_yaminoko",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 30.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 40.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitKoopa_defense,
        .defense_attr = unitKoopa_defense_attr,
        .attribute_flags = 0x0000'0009 |
            PartsAttribute_Flags::WINGED |
            PartsAttribute_Flags::HAMMERLIKE_CANNOT_TARGET |
            PartsAttribute_Flags::SHELLTOSSLIKE_CANNOT_TARGET,
        .counter_attribute_flags = 0,
        .pose_table = unitParatroopa_pose_table,
    },
};

// Evt definitions.

EVT_BEGIN(unitKoopa_flipped_attack_event)
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), PTR(&unitKoopa_weaponFlipped))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, PTR(&unitKoopa_weaponFlipped), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            RETURN()
        END_IF()
        GOTO(99)
    END_IF()
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, LW(3))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    USER_FUNC(btlevtcmd_WeaponAftereffect, PTR(&unitKoopa_weaponFlipped))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, PTR(&unitKoopa_weaponFlipped))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    WAIT_MSEC(100)
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_NOKO_JUMP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.0), FLOAT(0.5))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, -1)
    END_BROTHER()
    BROTHER_EVT()
        WAIT_FRM(27)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_NOKO_SHELL1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("NKT_A_2"))
    END_BROTHER()
    USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
    DO(36)
        USER_FUNC(btlevtcmd_AddRotate, -2, 0, 10, 0)
        WAIT_FRM(1)
    WHILE()
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Rotate, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_RotateSpeed, 5)
    BROTHER_EVT()
        DO(0)
            USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Rotate, LW(15))
            IF_EQUAL(LW(15), 1)
                USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
                DO_BREAK()
            END_IF()
            WAIT_FRM(1)
            USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_RotateSpeed, LW(14))
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, LW(14), 0)
        WHILE()
    END_BROTHER()
    WAIT_FRM(20)
    BROTHER_EVT()
        WAIT_MSEC(200)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        ADD(LW(2), 10)
        USER_FUNC(eff_aura, -2)
    END_BROTHER()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_URANOKO_MOVE1"), EVT_NULLPTR, 0, LW(15))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_RotateSpeed, 10)
    WAIT_MSEC(400)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_RotateSpeed, 20)
    WAIT_MSEC(400)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_RotateSpeed, 40)
    WAIT_MSEC(400)
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    BROTHER_EVT_ID(LW(13))
        DO(0)
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
            ADD(LW(1), 10)
            USER_FUNC(evt_eff, PTR(""), PTR("kemuri_test"), 0, LW(0), LW(1), LW(2), FLOAT(0.3), 0, 0, 0, 0, 0, 0, 0)
            WAIT_FRM(3)
        WHILE()
    END_BROTHER()
    USER_FUNC(evt_snd_sfxoff, LW(15))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_URANOKO_MOVE2"), EVT_NULLPTR, 0, LW(15))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(8.0))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetPos, -2, EVT_NULLPTR, LW(5), EVT_NULLPTR)
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(5), LW(2), 0, 0, 1)
    SET(LW(6), 0)
LBL(10)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(8.0))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetPos, -2, EVT_NULLPTR, LW(5), EVT_NULLPTR)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), PTR(&unitKoopa_weaponFlipped), 256, LW(5))
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
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 100)
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(5), LW(2), 0, 0, 1)
    GOTO(97)
LBL(91)
    USER_FUNC(evt_btl_camera_shake_h, 0, 1, 1, 8, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_add_zoom, 0, -50)
    IF_EQUAL(LW(6), 0)
        USER_FUNC(btlevtcmd_ResultACDefence, LW(3), PTR(&unitKoopa_weaponFlipped))
        SET(LW(6), 1)
    END_IF()
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&unitKoopa_weaponFlipped), 256, LW(5))
    GOTO(97)
LBL(97)
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    IF_NOT_EQUAL(LW(3), -1)
        GOTO(10)
    END_IF()
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 500)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(8.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, 0, 1)
    WAIT_MSEC(500)
    USER_FUNC(btlevtcmd_SetPos, -2, 250, LW(1), LW(2))
LBL(98)
    USER_FUNC(evt_snd_sfxoff, LW(15))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_URANOKO_MOVE3"), EVT_NULLPTR, 0, LW(15))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(8.0))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, 0, 1)
    DELETE_EVT(LW(13))
    USER_FUNC(evt_snd_sfxoff, LW(15))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_URANOKO_MOVE4"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(10.0), FLOAT(0.4))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 20, -1)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Rotate, 1)
    WAIT_FRM(3)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitKoopa_wakeup_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_GetOverTurnCount, LW(10), LW(0))
    SUB(LW(0), 1)
    USER_FUNC(btlevtcmd_SetOverTurnCount, LW(10), LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(0))
        SWITCH(LW(0))
            CASE_OR((int32_t)BattleUnitType::SHADY_KOOPA)
            CASE_OR((int32_t)BattleUnitType::SHADY_PARATROOPA)
                RUN_CHILD_EVT(PTR(&unitKoopa_flipped_attack_event))
                CASE_END()
            CASE_ETC()
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("NKT_S_3"))
                WAIT_MSEC(400)
                USER_FUNC(btlevtcmd_StartWaitEvent, -2)
        END_SWITCH()
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_SetPartsDefenceTable, LW(10), LW(11), PTR(&unitKoopa_defense))
    USER_FUNC(btlevtcmd_OffStatusFlag, LW(10), 1)
    USER_FUNC(btlevtcmd_AnimeSetPoseTable, LW(10), LW(11), PTR(&unitKoopa_pose_table))
    USER_FUNC(btlevtcmd_AnimeChangePose, LW(10), LW(11), PTR("NKT_S_1"))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_INSIDE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(1.0))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
    USER_FUNC(btlevtcmd_SetHitOffset, -2, 1, 0, 0, 0)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitKoopa_attack_event)
    USER_FUNC(btlevtcmd_GetOverTurnCount, -2, LW(0))
    IF_LARGE_EQUAL(LW(0), 1)
        RUN_CHILD_EVT(PTR(&unitKoopa_wakeup_event))
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_GetFirstAttackTarget, LW(3), LW(4))
        WAIT_FRM(30)
    ELSE()
        USER_FUNC(btlevtcmd_EnemyItemUseCheck, -2, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(LW(0))
            USER_FUNC(btlevtcmd_StartWaitEvent, -2)
            RETURN()
        END_IF()
        USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
        USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), PTR(&unitKoopa_weaponGround))
        USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, PTR(&unitKoopa_weaponGround), LW(3), LW(4))
    END_IF()
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
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    USER_FUNC(btlevtcmd_WeaponAftereffect, PTR(&unitKoopa_weaponGround))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, PTR(&unitKoopa_weaponGround))
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    WAIT_MSEC(100)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_NOKO_JUMP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.0), FLOAT(0.7))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 0, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_NOKO_SHELL1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("NKT_A_1"))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_NOKO_MOVE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Rotate, 0)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_RotateSpeed, 0)
    BROTHER_EVT()
        DO(0)
            USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_Rotate, LW(15))
            IF_EQUAL(LW(15), 1)
                USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
                DO_BREAK()
            END_IF()
            WAIT_FRM(1)
            USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_RotateSpeed, LW(14))
            USER_FUNC(btlevtcmd_AddRotate, -2, 0, LW(14), 0)
        WHILE()
    END_BROTHER()
    WAIT_MSEC(100)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_RotateSpeed, 20)
    WAIT_MSEC(100)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_RotateSpeed, 40)
    WAIT_MSEC(100)
    BROTHER_EVT_ID(LW(13))
        DO(0)
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
            ADD(LW(1), 10)
            USER_FUNC(evt_eff, PTR(""), PTR("kemuri_test"), 0, LW(0), LW(1), LW(2), FLOAT(0.3), 0, 0, 0, 0, 0, 0, 0)
            WAIT_FRM(3)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(8.0))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetPos, -2, EVT_NULLPTR, LW(5), EVT_NULLPTR)
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(5), LW(2), 0, 0, 1)
    DELETE_EVT(LW(13))
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), PTR(&unitKoopa_weaponGround), 256, LW(5))
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
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 100)
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), 0, LW(2), 0, 0, 1)
    WAIT_MSEC(250)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_RotateSpeed, 10)
    WAIT_MSEC(250)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_NOKO_JUMP1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Rotate, 1)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 69)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(10.0), FLOAT(0.4))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 40)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 20, -1)
    GOTO(98)
LBL(91)
    USER_FUNC(evt_btl_camera_shake_h, 0, 1, 1, 8, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_add_zoom, 0, -50)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), PTR(&unitKoopa_weaponGround))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&unitKoopa_weaponGround), 256, LW(5))
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_RotateSpeed, 10)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(3.0), FLOAT(0.2))
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 50)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 20, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_NOKO_MOVE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.5), FLOAT(0.2))
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 75)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 20, -1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_NOKO_MOVE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_Rotate, 1)
    GOTO(98)
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

EVT_BEGIN(unitParatroopa_attack_event)
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_GetFirstAttackTarget, LW(3), LW(4))
        WAIT_FRM(50)
    ELSE()
        USER_FUNC(btlevtcmd_EnemyItemUseCheck, -2, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(LW(0))
            USER_FUNC(btlevtcmd_StartWaitEvent, -2)
            RETURN()
        END_IF()
        USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
        USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), PTR(&unitKoopa_weaponPara))
        USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, PTR(&unitKoopa_weaponPara), LW(3), LW(4))
    END_IF()
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
    USER_FUNC(btlevtcmd_WeaponAftereffect, PTR(&unitKoopa_weaponPara))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, PTR(&unitKoopa_weaponPara))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), 45)
    ADD(LW(1), 30)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(0), LW(2), 1, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.5))
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 0, -16, 0, 0, -1)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(evt_btl_camera_nomove_y_onoff, 0, 1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_PATA_MOVE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(1.0))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PTP_A_1"))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 6)
    ADD(LW(1), 6)
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 0, 0, 0, 0, 0)
    WAIT_FRM(11)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_PATA_MOVE3"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("PTP_A_4"))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    ADD(LW(1), 20)
    USER_FUNC(btlevtcmd_SetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), PTR(&unitKoopa_weaponPara), 1048832, LW(5))
    ELSE()
        USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), PTR(&unitKoopa_weaponPara), 256, LW(5))
    END_IF()
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
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(5.0))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 0, 0, 0, 0, 0)
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 25)
    SET(LW(1), 20)
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 0, 0, 0, 0, 0)
    GOTO(98)
LBL(91)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(6.0))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 0, 0, 0, 0, 0)
    USER_FUNC(evt_btl_camera_shake_h, 0, 1, 1, 8, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_add_zoom, 0, -50)
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), PTR(&unitKoopa_weaponPara))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&unitKoopa_weaponPara), 1048832, LW(5))
    ELSE()
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&unitKoopa_weaponPara), 256, LW(5))
    END_IF()
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_PATA_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 40)
    ADD(LW(1), 30)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(3.0))
    INLINE_EVT()
        SET(LW(0), 0)
        DO(24)
            SUB(LW(0), 45)
            IF_SMALL(LW(0), 0)
                ADD(LW(0), 360)
            END_IF()
            USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, LW(0))
            WAIT_FRM(1)
        WHILE()
        USER_FUNC(btlevtcmd_SetRotate, -2, 0, 0, 0)
        USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 43)
        WAIT_FRM(1)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        SUB(LW(1), 20)
        USER_FUNC(btlevtcmd_SetPos, -2, LW(0), LW(1), LW(2))
    END_INLINE()
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 0, 30, 0, 0, 0)
    WAIT_FRM(10)
    GOTO(98)
LBL(98)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 42)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(4.0))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_DivePosition, -2, LW(0), LW(1), LW(2), 0, -10, 0, 0, -1)
LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitKoopa_wait_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitKoopa_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    RETURN()
EVT_END()

EVT_BEGIN(unitKoopa_flip_event)
    USER_FUNC(btlevtcmd_SetHitOffset, -2, 1, 0, -15, 0)
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_SetPartsDefenceTable, LW(10), LW(11), PTR(&unitKoopa_flip_defense))
    USER_FUNC(btlevtcmd_AnimeSetPoseTable, LW(10), LW(11), PTR(&unitKoopa_flip_pose_table))
    USER_FUNC(btlevtcmd_OnStatusFlag, LW(10), 1)
    USER_FUNC(btlevtcmd_SetOverTurnCount, LW(10), 2)
    USER_FUNC(btlevtcmd_AnimeChangePose, LW(10), LW(11), PTR("NKT_D_1"))
    WAIT_FRM(2)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_INSIDE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.7))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 15, -1)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 10, -1)
    USER_FUNC(btlevtcmd_AnimeChangePose, LW(10), LW(11), PTR("NKT_S_2"))
    RETURN()
EVT_END()

EVT_BEGIN(unitParatroopa_fall_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_OffAttribute, LW(10), 4)
    USER_FUNC(btlevtcmd_OffPartsAttribute, LW(10), LW(11), 0x800)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(10), LW(11), 39)
    WAIT_MSEC(500)
    USER_FUNC(btlevtcmd_AnimeSetPoseTable, LW(10), LW(11), PTR(&unitKoopa_pose_table))
    USER_FUNC(btlevtcmd_ChangeDataTable, LW(10), PTR(&unitKoopa_data_table))
    USER_FUNC(btlevtcmd_AnimeChangePoseDirect, LW(10), LW(11), PTR("NKT_D_1"))

    USER_FUNC(btlevtcmd_GetUnitWork, -2, UW_BattleUnitType, LW(0))
    SWITCH(LW(0))
        CASE_EQUAL((int32_t)BattleUnitType::PARATROOPA)
            SET(LW(0), (int32_t)BattleUnitType::KOOPA_TROOPA)
            USER_FUNC(btlevtcmd_ReplaceParts, LW(10), LW(11), PTR(&unitKoopa_parts), 1)
            USER_FUNC(btlevtcmd_SetRegistStatus, LW(10), PTR(&unitKoopa_status))
        CASE_EQUAL((int32_t)BattleUnitType::KP_PARATROOPA)
            SET(LW(0), (int32_t)BattleUnitType::KP_KOOPA)
            USER_FUNC(btlevtcmd_ReplaceParts, LW(10), LW(11), PTR(&unitKpKoopa_parts), 1)
            USER_FUNC(btlevtcmd_SetRegistStatus, LW(10), PTR(&unitKpKoopa_status))
        CASE_EQUAL((int32_t)BattleUnitType::SHADY_PARATROOPA)
            SET(LW(0), (int32_t)BattleUnitType::SHADY_KOOPA)
            USER_FUNC(btlevtcmd_ReplaceParts, LW(10), LW(11), PTR(&unitShadyKoopa_parts), 1)
            USER_FUNC(btlevtcmd_SetRegistStatus, LW(10), PTR(&unitShadyKoopa_status))
        CASE_EQUAL((int32_t)BattleUnitType::DARK_PARATROOPA)
            SET(LW(0), (int32_t)BattleUnitType::DARK_KOOPA)
            USER_FUNC(btlevtcmd_ReplaceParts, LW(10), LW(11), PTR(&unitDarkKoopa_parts), 1)
            USER_FUNC(btlevtcmd_SetRegistStatus, LW(10), PTR(&unitDarkKoopa_status))
    END_SWITCH()
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, LW(0))
    USER_FUNC(btlevtcmd_ChangeKind, LW(10), LW(0))
    USER_FUNC(btlevtcmd_OnUnitFlag, LW(10), 4)

    SET(LW(11), 1)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(10), LW(11), 39)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_FALL1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetPos, LW(10), LW(0), LW(1), LW(2))
    SET(LW(1), 0)
    USER_FUNC(btlevtcmd_SetFallAccel, LW(10), FLOAT(1.0))
    USER_FUNC(btlevtcmd_JumpPosition, LW(10), LW(0), LW(1), LW(2), 10, -1)
    USER_FUNC(btlevtcmd_SetHomePos, LW(10), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHpDamageCount, LW(10), LW(0))
    IF_SMALL_EQUAL(LW(0), 1)
        WAIT_FRM(30)
        USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitKoopa_wait_event))
        USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitKoopa_attack_event))
        USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitKoopa_damage_event))
        USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitKoopa_attack_event))
        USER_FUNC(btlevtcmd_SetRunSound, -2, PTR("SFX_ENM_NOKO_MOVE3L"), PTR("SFX_ENM_NOKO_MOVE3R"), 0, 3, 3)
        USER_FUNC(btlevtcmd_SetWalkSound, -2, PTR("SFX_ENM_NOKO_MOVE3L"), PTR("SFX_ENM_NOKO_MOVE3L"), 0, 6, 6)
        USER_FUNC(btlevtcmd_SetOverTurnCount, -2, 0)
        USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(10), LW(11), 39)
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitKoopa_wait_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitKoopa_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitKoopa_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitKoopa_attack_event))
    USER_FUNC(btlevtcmd_SetRunSound, -2, PTR("SFX_ENM_NOKO_MOVE3L"), PTR("SFX_ENM_NOKO_MOVE3R"), 0, 3, 3)
    USER_FUNC(btlevtcmd_SetWalkSound, -2, PTR("SFX_ENM_NOKO_MOVE3L"), PTR("SFX_ENM_NOKO_MOVE3L"), 0, 6, 6)
    USER_FUNC(btlevtcmd_SetOverTurnCount, -2, 0)
    USER_FUNC(btlevtcmd_AnimeChangePoseType, LW(10), LW(11), 39)
    RUN_CHILD_EVT(PTR(&unitKoopa_flip_event))
    RETURN()
EVT_END()

EVT_BEGIN(unitParatroopa_first_attack_pos)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    SET(LW(1), 10)
    USER_FUNC(btlevtcmd_SetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_WaitAttackEnd)
    RETURN()
EVT_END()

EVT_BEGIN(unitKoopa_common_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitKoopa_wait_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitKoopa_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitKoopa_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitKoopa_attack_event))
    USER_FUNC(btlevtcmd_SetOverTurnCount, -2, 0)
    USER_FUNC(btlevtcmd_SetRunSound, -2, PTR("SFX_ENM_NOKO_MOVE3L"), PTR("SFX_ENM_NOKO_MOVE3R"), 0, 3, 3)
    USER_FUNC(btlevtcmd_SetWalkSound, -2, PTR("SFX_ENM_NOKO_MOVE3L"), PTR("SFX_ENM_NOKO_MOVE3L"), 0, 6, 6)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitParatroopa_common_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitKoopa_wait_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitParatroopa_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitKoopa_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitParatroopa_attack_event))
    USER_FUNC(btlevtcmd_AnimeSetPoseSoundTable, -2, 1, PTR(&unitParatroopa_pose_sound_timing_table))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitKoopa_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType,
        (int32_t)BattleUnitType::KOOPA_TROOPA)
    RUN_CHILD_EVT(unitKoopa_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitKpKoopa_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType,
        (int32_t)BattleUnitType::KP_KOOPA)
    RUN_CHILD_EVT(unitKoopa_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitShadyKoopa_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType,
        (int32_t)BattleUnitType::SHADY_KOOPA)
    RUN_CHILD_EVT(unitKoopa_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitDarkKoopa_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType,
        (int32_t)BattleUnitType::DARK_KOOPA)
    RUN_CHILD_EVT(unitKoopa_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitParatroopa_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType,
        (int32_t)BattleUnitType::KOOPA_TROOPA)
    RUN_CHILD_EVT(unitParatroopa_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitKpParatroopa_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType,
        (int32_t)BattleUnitType::KP_PARATROOPA)
    RUN_CHILD_EVT(unitParatroopa_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitShadyParatroopa_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType,
        (int32_t)BattleUnitType::SHADY_PARATROOPA)
    RUN_CHILD_EVT(unitParatroopa_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitDarkParatroopa_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType,
        (int32_t)BattleUnitType::DARK_PARATROOPA)
    RUN_CHILD_EVT(unitParatroopa_common_init_event)
    RETURN()
EVT_END()

// BattleUnitKind, Setup structs.

DataTableEntry unitKoopa_data_table[3] = {
    13, (void*)unitKoopa_flip_event,
    48, (void*)btldefaultevt_Dummy,
    0, nullptr,
};
DataTableEntry unitParatroopa_data_table[3] = {
    14, (void*)unitParatroopa_fall_event,
    48, (void*)unitParatroopa_first_attack_pos,
    0, nullptr,
};

BattleUnitKind unit_KoopaTroopa = {
    .unit_type = BattleUnitType::KOOPA_TROOPA,
    .unit_name = "btl_un_nokonoko",
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
    .width = 36,
    .height = 40,
    .hit_offset = { -4, 40 },
    .center_offset = { 0.0f, 20.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 9.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 18.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 11.0f, 24.0f, 0.0f },
    .cut_base_offset = { 0.0f, 20.0f, 0.0f },
    .cut_width = 36.0f,
    .cut_height = 40.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_NOKO_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitKoopa_status,
    .num_parts = 1,
    .parts = unitKoopa_parts,
    .init_evt_code = (void*)unitKoopa_init_event,
    .data_table = unitKoopa_data_table,
};
BattleUnitKind unit_KpKoopa = {
    .unit_type = BattleUnitType::KP_KOOPA,
    .unit_name = "btl_un_nokonoko_fighter",
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
    .height = 40,
    .hit_offset = { -4, 40 },
    .center_offset = { 0.0f, 20.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 9.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 18.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 11.0f, 24.0f, 0.0f },
    .cut_base_offset = { 0.0f, 20.0f, 0.0f },
    .cut_width = 36.0f,
    .cut_height = 40.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_NOKO_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitKpKoopa_status,
    .num_parts = 1,
    .parts = unitKpKoopa_parts,
    .init_evt_code = (void*)unitKpKoopa_init_event,
    .data_table = unitKoopa_data_table,
};
BattleUnitKind unit_ShadyKoopa = {
    .unit_type = BattleUnitType::SHADY_KOOPA,
    .unit_name = "btl_un_ura_noko",
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
    .height = 40,
    .hit_offset = { -4, 40 },
    .center_offset = { 0.0f, 20.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 9.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 18.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 11.0f, 24.0f, 0.0f },
    .cut_base_offset = { 0.0f, 20.0f, 0.0f },
    .cut_width = 36.0f,
    .cut_height = 40.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_NOKO_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitShadyKoopa_status,
    .num_parts = 1,
    .parts = unitShadyKoopa_parts,
    .init_evt_code = (void*)unitShadyKoopa_init_event,
    .data_table = unitKoopa_data_table,
};
BattleUnitKind unit_DarkKoopa = {
    .unit_type = BattleUnitType::DARK_KOOPA,
    .unit_name = "btl_un_yami_noko",
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
    .height = 40,
    .hit_offset = { -4, 40 },
    .center_offset = { 0.0f, 20.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 9.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 18.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 11.0f, 24.0f, 0.0f },
    .cut_base_offset = { 0.0f, 20.0f, 0.0f },
    .cut_width = 36.0f,
    .cut_height = 40.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_NOKO_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitDarkKoopa_status,
    .num_parts = 1,
    .parts = unitDarkKoopa_parts,
    .init_evt_code = (void*)unitDarkKoopa_init_event,
    .data_table = unitKoopa_data_table,
};
BattleUnitKind unit_Paratroopa = {
    .unit_type = BattleUnitType::PARATROOPA,
    .unit_name = "btl_un_patapata",
    .max_hp = 5,
    .max_fp = 0,
    .danger_hp = 1,
    .peril_hp = 1,
    .level = 1,
    .bonus_exp = 0,
    .bonus_coin = 0,
    .bonus_coin_rate = 100,
    .base_coin = 0,
    .run_rate = 60,
    .pb_soft_cap = 9999,
    .width = 36,
    .height = 40,
    .hit_offset = { -4, 40 },
    .center_offset = { 0.0f, 24.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 9.0f, 6.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 18.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 11.0f, 24.0f, 0.0f },
    .cut_base_offset = { 0.0f, 24.0f, 0.0f },
    .cut_width = 36.0f,
    .cut_height = 40.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_PATA_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::UNQUAKEABLE,
    .status_vulnerability = &unitParatroopa_status,
    .num_parts = 1,
    .parts = unitParatroopa_parts,
    .init_evt_code = (void*)unitParatroopa_init_event,
    .data_table = unitParatroopa_data_table,
};
BattleUnitKind unit_KpParatroopa = {
    .unit_type = BattleUnitType::KP_PARATROOPA,
    .unit_name = "btl_un_patapata_fighter",
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
    .height = 40,
    .hit_offset = { -4, 40 },
    .center_offset = { 0.0f, 24.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 9.0f, 6.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 18.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 11.0f, 24.0f, 0.0f },
    .cut_base_offset = { 0.0f, 24.0f, 0.0f },
    .cut_width = 36.0f,
    .cut_height = 40.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_PATA_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::UNQUAKEABLE,
    .status_vulnerability = &unitKpParatroopa_status,
    .num_parts = 1,
    .parts = unitKpParatroopa_parts,
    .init_evt_code = (void*)unitKpParatroopa_init_event,
    .data_table = unitParatroopa_data_table,
};
BattleUnitKind unit_ShadyParatroopa = {
    .unit_type = BattleUnitType::SHADY_PARATROOPA,
    .unit_name = "btl_un_ura_pata",
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
    .height = 40,
    .hit_offset = { -4, 40 },
    .center_offset = { 0.0f, 24.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 9.0f, 6.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 18.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 11.0f, 24.0f, 0.0f },
    .cut_base_offset = { 0.0f, 24.0f, 0.0f },
    .cut_width = 36.0f,
    .cut_height = 40.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_PATA_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::UNQUAKEABLE,
    .status_vulnerability = &unitShadyParatroopa_status,
    .num_parts = 1,
    .parts = unitShadyParatroopa_parts,
    .init_evt_code = (void*)unitShadyParatroopa_init_event,
    .data_table = unitParatroopa_data_table,
};
BattleUnitKind unit_DarkParatroopa = {
    .unit_type = BattleUnitType::DARK_PARATROOPA,
    .unit_name = "btl_un_yami_pata",
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
    .height = 40,
    .hit_offset = { -4, 40 },
    .center_offset = { 0.0f, 24.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 9.0f, 6.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 18.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 11.0f, 24.0f, 0.0f },
    .cut_base_offset = { 0.0f, 24.0f, 0.0f },
    .cut_width = 36.0f,
    .cut_height = 40.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_PATA_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = BattleUnitAttribute_Flags::UNQUAKEABLE,
    .status_vulnerability = &unitDarkParatroopa_status,
    .num_parts = 1,
    .parts = unitDarkParatroopa_parts,
    .init_evt_code = (void*)unitDarkParatroopa_init_event,
    .data_table = unitParatroopa_data_table,
};

// Function / USER_FUNC definitions.

EVT_DEFINE_USER_FUNC(eff_aura) {
    int32_t id = evtGetValue(evt, evt->evtArguments[0]);
    id = ttyd::battle_sub::BattleTransID(evt, id);
    auto* unit = ttyd::battle::BattleGetUnitPtr(g_BattleWork, id);
    
    if (isFirstCall) {
        unit->unit_work[UW_ShadyEffPtr] = 
            reinterpret_cast<uint32_t>(
                ttyd::eff_uranoko::effUranokoEntry(0.0f, -1000.0f, 0.0f, 0, 120));
    }

    auto* eff = reinterpret_cast<EffEntry*>(unit->unit_work[UW_ShadyEffPtr]);
    
    if ((eff->flags & 1) == 0) {
        return 2;
    } else {
        gc::vec3 pos;
        ttyd::battle_unit::BtlUnit_GetPos(unit, &pos.x, &pos.y, &pos.z);
        
        intptr_t work = reinterpret_cast<intptr_t>(eff->eff_work);
        
        *reinterpret_cast<float*>(work + 0x04) = pos.x;
        *reinterpret_cast<float*>(work + 0x08) = pos.y;
        *reinterpret_cast<float*>(work + 0x0c) = pos.z;
        *reinterpret_cast<float*>(work + 0x14) = unit->unk_scale;
    }
    return 0;
}

}  // namespace mod::tot::custom