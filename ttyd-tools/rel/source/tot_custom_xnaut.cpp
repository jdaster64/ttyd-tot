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

extern const int32_t unitXNaut_init_event[];
extern const int32_t unitEliteXNaut_init_event[];
extern const int32_t unitXNaut_common_init_event[];
extern const int32_t unitXNaut_attack_event[];
extern const int32_t unitXNaut_damage_event[];
extern const int32_t unitXNaut_wait_event[];
extern const int32_t unitXNaut_ram_attack_event[];
extern const int32_t unitXNaut_jump_attack_event[];
extern const int32_t unitXNaut_attack_common_event_1[];
extern const int32_t unitXNaut_attack_common_event_2[];
extern const int32_t unitXNaut_potion_event[];

// Unit data.

int8_t unitXNaut_defense[] = { 0, 0, 0, 0, 0 };
int8_t unitXNaut_defense_attr[] = { 0, 0, 0, 0, 0 };
int8_t unitEliteXNaut_defense[] = { 1, 1, 1, 1, 1 };
int8_t unitEliteXNaut_defense_attr[] = { 0, 0, 0, 0, 0 };

StatusVulnerability unitXNaut_status = {
     90,  90,  90, 100,  90, 100, 100,  80,
    100,  95, 100,  95, 100,  95,  80, 100,
     80, 100,  80, 100, 100,  95,
};
StatusVulnerability unitEliteXNaut_status = {
     70,  70,  70, 100,  70, 100, 100,  60,
    100,  85, 100,  85, 100,  85,  60,  80,
     60, 100,  60, 100, 100,  95,
};

PoseTableEntry unitXNaut_pose_table[] = {
    1, "N_1",
    2, "Y_1",
    9, "Y_1",
    5, "K_1",
    4, "X_1",
    3, "X_1",
    27, "Z_1",
    28, "S_1",
    29, "D_1",
    30, "D_1",
    31, "S_1",
    39, "D_1",
    42, "R_1",
    40, "W_1",
    65, "T_1",
    69, "S_1",
};

DataTableEntry unitXNaut_data_table[] = {
    0, nullptr,
};

BattleWeapon unitXNaut_weaponLow = {
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
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH |
        AttackTargetClass_Flags::ENEMY_SELECT_SIDE_HOME,
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
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::FLIPS_SHELLED,
    .counter_resistance_flags =
        AttackCounterResistance_Flags::TOP_SPIKY |
        // Added to make sure that front-spiky counter functions properly.
        AttackCounterResistance_Flags::PREEMPTIVE_SPIKY,
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
BattleWeapon unitXNaut_weaponJump = {
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
    .counter_resistance_flags =
        AttackCounterResistance_Flags::PREEMPTIVE_SPIKY |
        AttackCounterResistance_Flags::FRONT_SPIKY,
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
BattleWeapon unitXNaut_weaponPotion = {
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
    .size_change_chance = 100,
    .size_change_time = 3,
    .size_change_strength = 2,
    
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

BattleUnitKindPart unitXNaut_parts[] = {
    {
        .index = 1,
        .name = "btl_un_gundan_zako",
        .model_name = "c_zako_n",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 28.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 38.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitXNaut_defense,
        .defense_attr = unitXNaut_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitXNaut_pose_table,
    },
};
BattleUnitKindPart unitEliteXNaut_parts[] = {
    {
        .index = 1,
        .name = "btl_un_gundan_zako_elite",
        .model_name = "c_zako_e",
        .part_offset_pos = { 0.0f, 0.0f, 0.0f },
        .part_hit_base_offset = { 0.0f, 28.0f, 0.0f },
        .part_hit_cursor_base_offset = { 0.0f, 38.0f, 0.0f },
        .unk_30 = 20,
        .unk_32 = 30,
        .base_alpha = 255,
        .defense = unitEliteXNaut_defense,
        .defense_attr = unitEliteXNaut_defense_attr,
        .attribute_flags = 0x0000'0009,
        .counter_attribute_flags = 0,
        .pose_table = unitXNaut_pose_table,
    },
};

BattleUnitKind unit_XNaut = {
    .unit_type = BattleUnitType::X_NAUT,
    .unit_name = "btl_un_gundan_zako",
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
    .width = 30,
    .height = 36,
    .hit_offset = { 0, 36 },
    .center_offset = { 0.0f, 18.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 15.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 15.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 12.0f, 27.0f, 0.0f },
    .cut_base_offset = { 0.0f, 18.0f, 0.0f },
    .cut_width = 30.0f,
    .cut_height = 36.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_3RD1_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitXNaut_status,
    .num_parts = 1,
    .parts = unitXNaut_parts,
    .init_evt_code = (void*)unitXNaut_init_event,
    .data_table = unitXNaut_data_table,
};
BattleUnitKind unit_EliteXNaut = {
    .unit_type = BattleUnitType::ELITE_X_NAUT,
    .unit_name = "btl_un_gundan_zako_elite",
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
    .width = 30,
    .height = 36,
    .hit_offset = { 0, 36 },
    .center_offset = { 0.0f, 18.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 15.0f, 0.0f, -10.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 15.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 12.0f, 27.0f, 0.0f },
    .cut_base_offset = { 0.0f, 18.0f, 0.0f },
    .cut_width = 30.0f,
    .cut_height = 36.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = 0,
    .swallow_attributes = 2,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_ENM_3RD1_DAMAGED1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0,
    .status_vulnerability = &unitEliteXNaut_status,
    .num_parts = 1,
    .parts = unitEliteXNaut_parts,
    .init_evt_code = (void*)unitEliteXNaut_init_event,
    .data_table = unitXNaut_data_table,
};

// Evt definitions.

EVT_BEGIN(unitXNaut_attack_common_event_1)
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(9))
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(9))
    USER_FUNC(evt_btl_camera_set_mode, 0, 8)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, LW(3))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 250)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(14))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(14))
    BROTHER_EVT()
        DO(9)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD1_MOVE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_FRM(5)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD1_MOVE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_FRM(5)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("R_2"))
    DO(9)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 10)
        ADD(LW(2), 1)
        USER_FUNC(evt_eff, PTR(""), PTR("kemuri_test"), 0, LW(0), LW(1), LW(2), FLOAT(0.5), 10, 0, 0, 0, 0, 0, 0)
        WAIT_FRM(10)
    WHILE()
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 10)
    USER_FUNC(evt_eff, PTR(""), PTR("kemuri_test"), 0, LW(0), LW(1), LW(2), FLOAT(0.5), 10, 0, 0, 0, 0, 0, 0)
    BROTHER_EVT_ID(LW(15))
        DO(0)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD1_MOVE3"), EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_FRM(5)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_1"))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 90)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(4.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), 0, LW(2), 0, -1, 0)
    DELETE_EVT(LW(15))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 10)
    ADD(LW(2), 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitXNaut_attack_common_event_2)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("R_1"))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(7.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitXNaut_attack_event)
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(PTR(&unitXNaut_ram_attack_event))
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_EnemyItemUseCheck, -2, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        RUN_CHILD_EVT(LW(0))
        USER_FUNC(btlevtcmd_StartWaitEvent, -2)
        RETURN()
    END_IF()
LBL(10)
    SET(LW(0), 0)
    ADD(LW(0), 50)
    ADD(LW(0), 30)
    ADD(LW(0), 20)
    SUB(LW(0), 1)
    USER_FUNC(evt_sub_random, LW(0), LW(1))
    SET(LW(0), 50)
    IF_SMALL(LW(1), LW(0))
        RUN_CHILD_EVT(PTR(&unitXNaut_ram_attack_event))
        GOTO(99)
    END_IF()
    ADD(LW(0), 30)
    IF_SMALL(LW(1), LW(0))
        RUN_CHILD_EVT(PTR(&unitXNaut_jump_attack_event))
        GOTO(99)
    END_IF()
    ADD(LW(0), 20)
    IF_SMALL(LW(1), LW(0))
        USER_FUNC(btlevtcmd_CheckStatus, -2, 10, LW(2))
        IF_EQUAL(LW(2), 0)
            RUN_CHILD_EVT(PTR(&unitXNaut_potion_event))
            GOTO(99)
        END_IF()
    END_IF()
    GOTO(10)
LBL(99)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitXNaut_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    RETURN()
EVT_END()

EVT_BEGIN(unitXNaut_potion_event)
    SET(LW(3), -2)
    SET(LW(4), 1)
    USER_FUNC(btlevtcmd_WeaponAftereffect, PTR(&unitXNaut_weaponPotion))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, PTR(&unitXNaut_weaponPotion))
    USER_FUNC(evt_btl_camera_set_mode, 0, 7)
    USER_FUNC(evt_btl_camera_set_homing_unit, 0, -2, -1)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 300)
    WAIT_FRM(60)
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD1_BOTTLE1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        WAIT_FRM(40)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD1_BOTTLE2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    END_BROTHER()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_4A"))
    WAIT_FRM(80)
    BROTHER_EVT()
        WAIT_FRM(10)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD1_BOTTLE3"), EVT_NULLPTR, 0, EVT_NULLPTR)
        WAIT_FRM(30)
        DO(3)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD1_BOTTLE4"), EVT_NULLPTR, 0, EVT_NULLPTR)
            WAIT_FRM(20)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_4B"))
    WAIT_FRM(120)
    INLINE_EVT()
        WAIT_FRM(5)
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD1_BOTTLE5"), EVT_NULLPTR, 0, EVT_NULLPTR)
        WAIT_FRM(55)
        USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    END_INLINE()
    USER_FUNC(btlevtcmd_ResultACDefence, LW(3), PTR(&unitXNaut_weaponPotion))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), PTR(&unitXNaut_weaponPotion), 256, LW(5))
    RETURN()
EVT_END()

EVT_BEGIN(unitXNaut_jump_attack_event)
    SET(LW(9), PTR(&unitXNaut_weaponJump))
    USER_FUNC(btlevtcmd_GetEnemyBelong, -2, LW(0))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, LW(0), LW(9))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(9), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        USER_FUNC(btlevtcmd_CheckToken, -2, 16, LW(0))
        IF_NOT_EQUAL(LW(0), 0)
            RUN_CHILD_EVT(PTR(&subsetevt_confuse_flustered))
            RETURN()
        END_IF()
        GOTO(98)
    END_IF()
    RUN_CHILD_EVT(PTR(&unitXNaut_attack_common_event_1))
    USER_FUNC(evt_btl_camera_nomove_y_onoff, 0, 1)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD1_ATTACK2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_2"))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 10)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 0, FLOAT(1.75), FLOAT(0.2))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 0, 0)
    USER_FUNC(evt_btl_camera_nomove_y_onoff, 0, 0)
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), LW(9), 0, LW(5))
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
LBL(90)
            USER_FUNC(btlevtcmd_JumpContinue, -2)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD1_MISS1"), EVT_NULLPTR, 0, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
            USER_FUNC(evt_eff64, PTR(""), PTR("kemuri1_n64"), 4, LW(0), LW(1), LW(2), 0, 0, 0, 0, 0, 0, 0, 0)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("D_2"))
            USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 15)
            USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(0.5))
            USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, 0, 1)
            WAIT_FRM(30)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("D_3"))
            WAIT_FRM(10)
            GOTO(98)
LBL(91)
            USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
            USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
            USER_FUNC(btlevtcmd_JumpContinue, -2)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD1_ATTACK3"), EVT_NULLPTR, 0, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
            USER_FUNC(evt_eff64, PTR(""), PTR("kemuri1_n64"), 4, LW(0), LW(1), LW(2), 0, 0, 0, 0, 0, 0, 0, 0)
            USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 10)
            USER_FUNC(btlevtcmd_JumpSetting, -2, 0, FLOAT(0.5), FLOAT(0.3))
            USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 0, 0)
            USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 5)
            USER_FUNC(btlevtcmd_JumpSetting, -2, 0, FLOAT(0.5), FLOAT(0.2))
            USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 0, 0)
            USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 3)
            USER_FUNC(btlevtcmd_JumpSetting, -2, 0, FLOAT(0.5), FLOAT(0.1))
            USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 0, 0)
            WAIT_FRM(15)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_1"))
            WAIT_FRM(10)
            GOTO(98)
LBL(98)
            USER_FUNC(evt_btl_camera_set_mode, 0, 0)
            RUN_CHILD_EVT(PTR(&unitXNaut_attack_common_event_2))
            RETURN()
EVT_END()

EVT_BEGIN(unitXNaut_ram_attack_event)
    SET(LW(9), PTR(&unitXNaut_weaponLow))
    USER_FUNC(btlevtcmd_check_battleflag, LW(0), 2)
    IF_NOT_EQUAL(LW(0), 0)
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
        GOTO(98)
    END_IF()
    RUN_CHILD_EVT(PTR(&unitXNaut_attack_common_event_1))
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD1_ATTACK1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_3"))
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 20)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 0, FLOAT(4.0), FLOAT(0.2))
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 0, 0)
    END_BROTHER()
    DO(0)
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(10), EVT_NULLPTR, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), EVT_NULLPTR, EVT_NULLPTR)
        IF_EQUAL(LW(14), -1)
            IF_SMALL_EQUAL(LW(0), LW(10))
                DO_BREAK()
            END_IF()
        ELSE()
            IF_LARGE_EQUAL(LW(0), LW(10))
                DO_BREAK()
            END_IF()
        END_IF()
        WAIT_FRM(1)
    WHILE()
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), LW(9), 0, LW(5))
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
LBL(90)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD1_MISS1"), EVT_NULLPTR, 0, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_JumpContinue, -2)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("D_2"))
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 15)
            USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(0.5))
            USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, 0, 1)
            WAIT_FRM(30)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("D_3"))
            WAIT_FRM(10)
            GOTO(98)
LBL(91)
            USER_FUNC(evt_btl_camera_shake_h, 0, 1, 1, 8, 0)
            USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
            USER_FUNC(evt_btl_camera_add_zoom, 0, -50)
            USER_FUNC(btlevtcmd_ResultACDefence, LW(3), LW(9))
            USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(9), 256, LW(5))
            USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 48)
            USER_FUNC(btlevtcmd_JumpSetting, -2, 0, FLOAT(2.0), FLOAT(0.1))
            USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), 0, LW(2), 0, 0)
            USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_ENM_3RD1_LANDING1"), EVT_NULLPTR, 0, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("J_1B"))
            WAIT_FRM(5)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_1"))
            WAIT_FRM(20)
            GOTO(98)
LBL(98)
            RUN_CHILD_EVT(PTR(&unitXNaut_attack_common_event_2))
            RETURN()
EVT_END()

EVT_BEGIN(unitXNaut_wait_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    RETURN()
EVT_END()

EVT_BEGIN(unitXNaut_common_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitXNaut_wait_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitXNaut_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitXNaut_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&unitXNaut_attack_event))
    USER_FUNC(btlevtcmd_SetRunSound, -2, PTR("SFX_ENM_3RD1_MOVE1"), PTR("SFX_ENM_3RD1_MOVE2"), 0, 5, 5)
    USER_FUNC(btlevtcmd_SetWalkSound, -2, PTR("SFX_ENM_3RD1_MOVE1"), PTR("SFX_ENM_3RD1_MOVE2"), 0, 10, 10)
    USER_FUNC(btlevtcmd_SetJumpSound, -2, 0, PTR("SFX_ENM_3RD1_LANDING1"))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitXNaut_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::X_NAUT)
    RUN_CHILD_EVT(unitXNaut_common_init_event)
    RETURN()
EVT_END()

EVT_BEGIN(unitEliteXNaut_init_event)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, UW_BattleUnitType, (int32_t)BattleUnitType::ELITE_X_NAUT)
    RUN_CHILD_EVT(unitXNaut_common_init_event)
    RETURN()
EVT_END()

}  // namespace mod::tot::custom