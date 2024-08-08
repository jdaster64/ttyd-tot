#include "tot_party_bobbery.h"

#include "evt_cmd.h"
#include "tot_manager_move.h"

#include <ttyd/battle.h>
#include <ttyd/battle_camera.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/battle_event_default.h>
#include <ttyd/battle_unit.h>
#include <ttyd/battle_weapon_power.h>
#include <ttyd/evt_audience.h>
#include <ttyd/evt_eff.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evt_sub.h>
#include <ttyd/icondrv.h>
#include <ttyd/msgdrv.h>
#include <ttyd/unit_party_sanders.h>

namespace mod::tot::party_bobbery {

namespace {
    
// Including entire namespaces for convenience.
using namespace ::ttyd::battle_camera;
using namespace ::ttyd::battle_database_common;
using namespace ::ttyd::battle_event_cmd;
using namespace ::ttyd::battle_event_default;
using namespace ::ttyd::battle_unit;
using namespace ::ttyd::battle_weapon_power;
using namespace ::ttyd::evt_audience;
using namespace ::ttyd::evt_eff;
using namespace ::ttyd::evt_snd;
using namespace ::ttyd::evt_sub;
using namespace ::ttyd::unit_party_sanders;

namespace IconType = ::ttyd::icondrv::IconType;

}  // namespace

// Declaration of weapon structs.
extern BattleWeapon customWeapon_BobberyBombFS;
extern BattleWeapon customWeapon_BobberyBomb;
extern BattleWeapon customWeapon_BobberyBombSquad;
extern BattleWeapon customWeapon_BobberyHoldFast;
extern BattleWeapon customWeapon_BobberyBobombast;
extern BattleWeapon customWeapon_BobberyPoisonBomb;
extern BattleWeapon customWeapon_BobberyMegatonBomb;

BattleWeapon* g_WeaponTable[] = {
    &customWeapon_BobberyBomb, &customWeapon_BobberyBombSquad, 
    &customWeapon_BobberyHoldFast, &customWeapon_BobberyPoisonBomb, 
    &customWeapon_BobberyBobombast, &customWeapon_BobberyMegatonBomb
};

void MakeSelectWeaponTable(
    ttyd::battle::BattleWorkCommand* command_work, int32_t* num_options) {
    for (int32_t i = 0; i < 6; ++i) {
        auto& weapon_entry = command_work->weapon_table[*num_options];
        BattleWeapon* weapon = g_WeaponTable[i];
        
        if (MoveManager::GetUnlockedLevel(MoveType::BOBBERY_BASE + i)) {
            weapon_entry.index = MoveType::BOBBERY_BASE + i;
            weapon_entry.item_id = 0;
            weapon_entry.weapon = weapon;
            weapon_entry.icon = weapon->icon;
            weapon_entry.unk_04 = 0;
            weapon_entry.unk_18 = 0;
            weapon_entry.name = ttyd::msgdrv::msgSearch(weapon->name);
            
            ++*num_options;
        }
    }
}

BattleWeapon* GetFirstAttackWeapon() {
    return &customWeapon_BobberyBombFS;
}

uint32_t GetPoisonBombPower(
    BattleWorkUnit* attacker, BattleWeapon* weapon,
    BattleWorkUnit* target, BattleWorkUnitPart* part) {
    // Deals 1 damage to bombs so they can chain, but 0 to everything else.
    return target->current_kind == BattleUnitType::BOMB_SQUAD_BOMB;
}

// Including definition of Bomb Squad bomb actor here, that way it's easier
// to add changes for variants of the Bomb Squad attack.
// TODO: There's a kind of awful hitch the first time you spawn the actor;
// see if the original game has it too / if there's a way to pre-load it once?

BattleWeapon unitBombzo_weapon = {
    .name = nullptr,
    .icon = 0,
    .item_id = 0,
    .description = nullptr,
    .base_accuracy = 100,
    .base_fp_cost = 0,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 0.0,
    .stylish_multiplier = 0,
    .unk_19 = 0,
    .bingo_card_chance = 0,
    .unk_1b = 0,
    .damage_function = (void*)weaponGetPowerDefault,
    .damage_function_params = { 3, 0, 0, 0, 0, 0, 0, 0 },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags = 
        AttackTargetClass_Flags::MULTIPLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_SELECT_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::EXPLOSION,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        // Made defense-piercing by default.
        AttackSpecialProperty_Flags::DEFENSE_PIERCING,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // .status_chances = all 0,
    
    .attack_evt_code = nullptr,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 10,
    .bg_a2_fall_weight = 10,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 30,
    .nozzle_turn_chance = 5,
    .nozzle_fire_chance = 5,
    .ceiling_fall_chance = 5,
    .object_fall_chance = 5,
};

BattleWeapon unitBombzo_weapon_Poison = {
    .name = nullptr,
    .icon = 0,
    .item_id = 0,
    .description = nullptr,
    .base_accuracy = 100,
    .base_fp_cost = 0,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 0.0,
    .stylish_multiplier = 0,
    .unk_19 = 0,
    .bingo_card_chance = 0,
    .unk_1b = 0,
    // Can only damage other Bomb Squad bombs.
    .damage_function = (void*)GetPoisonBombPower,
    .damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags = 
        AttackTargetClass_Flags::MULTIPLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_SELECT_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::EXPLOSION,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        // Made defense-piercing by default.
        AttackSpecialProperty_Flags::DEFENSE_PIERCING,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    .poison_chance = 100,
    .poison_time = 5,
    .poison_strength = 1,
    
    .attack_evt_code = nullptr,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 10,
    .bg_a2_fall_weight = 10,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 30,
    .nozzle_turn_chance = 5,
    .nozzle_fire_chance = 5,
    .ceiling_fall_chance = 5,
    .object_fall_chance = 5,
};
BattleWeapon unitBombzo_weapon_Megaton = {
    .name = nullptr,
    .icon = 0,
    .item_id = 0,
    .description = nullptr,
    .base_accuracy = 100,
    .base_fp_cost = 0,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 0.0,
    .stylish_multiplier = 0,
    .unk_19 = 0,
    .bingo_card_chance = 0,
    .unk_1b = 0,
    .damage_function = (void*)GetWeaponPowerFromUnitWorkVariable,
    .damage_function_params = { 3, 0, 0, 0, 0, 0, 0, 0 },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags = 
        AttackTargetClass_Flags::MULTIPLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_SELECT_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::EXPLOSION,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        // Made defense-piercing by default.
        AttackSpecialProperty_Flags::DEFENSE_PIERCING,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // .status_chances = all 0,
    
    .attack_evt_code = nullptr,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 50,
    .bg_a2_fall_weight = 50,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 50,
    .nozzle_turn_chance = 5,
    .nozzle_fire_chance = 5,
    .ceiling_fall_chance = 10,
    .object_fall_chance = 10,
};

PoseTableEntry unitBombzo_pose_table_1[] = {
    28, "B_4",
    31, "B_4",
    39, "B_4",
    69, "B_4",
};

PoseTableEntry unitBombzo_pose_table_2[] = {
    28, "B_3",
    31, "B_3",
    39, "B_3",
    69, "B_3",
};

PoseTableEntry unitBombzo_pose_table_3[] = {
    28, "B_2",
    31, "B_2",
    39, "B_2",
    69, "B_2",
};

EVT_BEGIN(unitBombzo_explosion_event)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, 1, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(evt_snd_sfxoff, LW(0))
        USER_FUNC(btlevtcmd_SetUnitWork, -2, 1, 0)
    END_IF()
    // Set weapon and range based on the type of bomb.
    USER_FUNC(btlevtcmd_GetUnitWork, -2, 2, LW(8))
    SWITCH(LW(8))
        CASE_EQUAL(0)
            SET(LW(8), PTR(&unitBombzo_weapon))
            SET(LW(9), 50)
            SET(LW(7), FLOAT(1.0))
        CASE_EQUAL(1)
            SET(LW(8), PTR(&unitBombzo_weapon_Poison))
            SET(LW(9), 50)
            SET(LW(7), FLOAT(1.0))
        CASE_ETC()
            SET(LW(8), PTR(&unitBombzo_weapon_Megaton))
            SET(LW(9), 500)
            SET(LW(7), FLOAT(5.0))
            // Also add weapon after-effects.
            USER_FUNC(btlevtcmd_WeaponAftereffect, LW(8))
    END_SWITCH()
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(evt_eff, PTR(""), PTR("hibashira"), 2, LW(0), LW(1), LW(2), LW(0), LW(1), LW(2), 1, LW(7), 60, 0, 0)
    USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_THUNDERS_BOMB2"), EVT_NULLPTR, 0, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_StageDispellFog)
    USER_FUNC(btlevtcmd_OnAttribute, -2, 16777216)
    USER_FUNC(btlevtcmd_OnPartsAttribute, -2, 1, 50331648)
    USER_FUNC(evt_btl_camera_shake_h, 0, 10, 0, 60, 4)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("B_2"))
    USER_FUNC(btlevtcmd_SamplingEnemy, -2, 1, LW(8))
    USER_FUNC(btlevtcmd_ChoiceSamplingEnemy, LW(8), LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    SET(LW(10), 0)
    LBL(10)
    USER_FUNC(btlevtcmd_GetUnitId, -2, LW(0))
    IF_EQUAL(LW(0), LW(3))
        GOTO(80)
    END_IF()
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), EVT_NULLPTR, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(1), EVT_NULLPTR, EVT_NULLPTR)
    SUB(LW(0), LW(1))
    IF_SMALL_EQUAL(LW(0), -1)
        MUL(LW(0), -1)
    END_IF()
    IF_LARGE(LW(0), LW(9))
        GOTO(80)
    END_IF()
    USER_FUNC(btlevtcmd_PreCheckDamage, -2, LW(3), LW(4), LW(8), 256, LW(5))
    IF_EQUAL(LW(5), 1)
        USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(8), 256, LW(5))
    END_IF()
    LBL(80)
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    IF_NOT_EQUAL(LW(3), -1)
        ADD(LW(10), 1)
        GOTO(10)
    END_IF()
    LBL(99)
    USER_FUNC(btlevtcmd_KillUnit, -2, 0)
    RETURN()
EVT_END()

EVT_BEGIN(unitBombzo_phase_event)
    // Tick down 3-phase timer on phase 1 and 3.
    USER_FUNC(btlevtcmd_CheckPhase, LW(0), 0x4000001)
    USER_FUNC(btlevtcmd_CheckPhase, LW(1), 0x4000003)
    ADD(LW(0), LW(1))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(btlevtcmd_GetUnitWork, -2, 0, LW(0))
        SUB(LW(0), 1)
        USER_FUNC(btlevtcmd_SetUnitWork, -2, 0, LW(0))
        SWITCH(LW(0))
            CASE_EQUAL(1)
                // Start flashing red on the start of the turn after spawn.
                USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitBombzo_pose_table_1))
                USER_FUNC(btlevtcmd_StartWaitEvent, -2)
            CASE_EQUAL(0)
                // Explode after the player phase the turn after spawn.
                USER_FUNC(btlevtcmd_PhaseEventStartDeclare, -2)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("B_2"))
                WAIT_FRM(36)
                RUN_CHILD_EVT(PTR(&unitBombzo_explosion_event))
            // Otherwise, do nothing.
        END_SWITCH()
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(unitBombzo_wait_event)
    USER_FUNC(btlevtcmd_AnimeChangePoseFromTable, -2, 1)
    LBL(0)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, 0, LW(0))
    IF_LARGE(LW(0), 1)
        WAIT_FRM(1)
        GOTO(0)
    END_IF()
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    ADD(LW(1), 10)
    USER_FUNC(evt_eff64, 0, PTR("sweat_n64"), 1, LW(0), LW(1), LW(2), 15, -45, 15, 0, 0, 0, 0, 0)
    USER_FUNC(evt_sub_random, 45, LW(0))
    ADD(LW(0), 45)
    WAIT_FRM(LW(0))
    GOTO(0)
    RETURN()
EVT_END()

EVT_BEGIN(unitBombzo_damage_event)
    SET(LW(10), -2)
    SET(LW(11), 1)
    RUN_CHILD_EVT(PTR(&btldefaultevt_Damage))
    RETURN()
EVT_END()

EVT_BEGIN(unitBombzo_attack_event)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(unitBombzo_init_event)
    USER_FUNC(btlevtcmd_SetEventWait, -2, PTR(&unitBombzo_wait_event))
    USER_FUNC(btlevtcmd_SetEventPhase, -2, PTR(&unitBombzo_phase_event))
    USER_FUNC(btlevtcmd_SetEventAttack, -2, PTR(&unitBombzo_attack_event))
    USER_FUNC(btlevtcmd_SetEventDamage, -2, PTR(&unitBombzo_damage_event))
    USER_FUNC(btlevtcmd_SetEventConfusion, -2, PTR(&btldefaultevt_Confuse))
    // Start phase transition timer at 3 instead of 2 (1.5 turns).
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 0, 3)
    USER_FUNC(btlevtcmd_SetUnitWork, -2, 1, 0)
    USER_FUNC(btlevtcmd_SetMaxMoveCount, -2, 0)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, 0, LW(0))
    SWITCH(LW(0))
        CASE_LARGE_EQUAL(3)
            USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitBombzo_pose_table_3))
        CASE_LARGE_EQUAL(2)
            USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitBombzo_pose_table_2))
        CASE_LARGE_EQUAL(1)
            USER_FUNC(btlevtcmd_AnimeSetPoseTable, -2, 1, PTR(&unitBombzo_pose_table_1))
    END_SWITCH()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("B_1"))
    RETURN()
EVT_END()

EVT_BEGIN(unitBombzo_dead_event)
    USER_FUNC(btlevtcmd_AfterReactionEntry, -2, 53)
    RETURN()
EVT_END()

EVT_BEGIN(unitBombzo_end_battle_evt)
    USER_FUNC(btlevtcmd_GetUnitWork, -2, 1, LW(0))
    IF_NOT_EQUAL(LW(0), 0)
        USER_FUNC(evt_snd_sfxoff, LW(0))
        USER_FUNC(btlevtcmd_SetUnitWork, -2, 1, 0)
    END_IF()
    RETURN()
EVT_END()

int8_t unitBombzo_defense[] = { 0, 0, 0, 0, 0 };
int8_t unitBombzo_defense_attr[] = { 0, 0, 0, 0, 0 };

StatusVulnerability unitBombzo_status = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,
};
        
DataTableEntry unitBombzo_data_table[] = {
    49, (void*)unitBombzo_dead_event,
    53, (void*)unitBombzo_explosion_event,
    63, (void*)unitBombzo_end_battle_evt,
    0, nullptr,
};

BattleUnitKindPart unitBombzo_parts = {
    .index = 1,
    .name = "btl_un_bomzou",
    .model_name = "c_bakudan",
    .part_offset_pos = { 0.0f, 0.0f, 0.0f },
    .part_hit_base_offset = { 15.0f, 25.0f, 0.0f },
    .part_hit_cursor_base_offset = { 0.0f, 30.0f, 0.0f },
    .unk_30 = 20,
    .unk_32 = 30,
    .base_alpha = 255,
    .defense = unitBombzo_defense,
    .defense_attr = unitBombzo_defense_attr,
    .attribute_flags = 0x00080009,
    .counter_attribute_flags = 0,
    .pose_table = unitBombzo_pose_table_3,
};

BattleUnitKind unitBombzo_unit = {
    .unit_type = BattleUnitType::BOMB_SQUAD_BOMB,
    .unit_name = "btl_un_bomzou",
    .max_hp = 1,
    .max_fp = 0,
    .danger_hp = 1,
    .peril_hp = 1,
    .level = 1,
    .bonus_exp = 0,
    .bonus_coin = 0,
    .bonus_coin_rate = 0,
    .base_coin = 0,
    .run_rate = 64,
    .pb_soft_cap = 9,
    .width = 24,
    .height = 24,
    .hit_offset = { 0, 20 },
    .center_offset = { 0.0f, 0.0f, 0.0f },
    .hp_gauge_offset = { 0, 0 },
    .talk_toge_base_offset = { 0.0f, 0.0f, 0.0f },
    .held_item_base_offset = { 0.0f, 0.0f, 0.0f },
    .burn_flame_offset = { 0.0f, 0.0f, 0.0f },
    .binta_hit_offset = { 12.0f, 0.0f, 0.0f },
    .kiss_hit_offset = { 12.0f, 15.6f, 0.0f },
    .cut_base_offset = { 0.0f, 0.0f, 0.0f },
    .cut_width = 0.0f,
    .cut_height = 0.0f,
    .turn_order = 0,
    .turn_order_variance = 0,
    .swallow_chance = -1,
    .swallow_attributes = 0,
    .hammer_knockback_chance = 100,
    .itemsteal_param = 20,
    .star_point_disp_offset = { 0.0f, 0.0f, 0.0f },
    .damage_sfx_name = "SFX_BTL_DAMAGE1",
    .fire_damage_sfx_name = "SFX_BTL_DAMAGE_FIRE1",
    .ice_damage_sfx_name = "SFX_BTL_DAMAGE_ICE1",
    .explosion_damage_sfx_name = "SFX_BTL_DAMAGE_BIRIBIRI1",
    .attribute_flags = 0x12400000,
    .status_vulnerability = &unitBombzo_status,
    .num_parts = 1,
    .parts = &unitBombzo_parts,
    .init_evt_code = (void*)unitBombzo_init_event,
    .data_table = unitBombzo_data_table,
};

BattleUnitSetup unitBombzo_entry = {
    .unit_kind_params = &unitBombzo_unit,
    .alliance = 2,
    .attack_phase = 0x04000001,
    .position = { 1000.0f, 0.0f, 1000.0f },
    .addl_target_offset_x = 0,
    .unit_work = { 0, 0, 0, 0 },
    .item_drop_table = nullptr,
};

EVT_BEGIN(partySandersAttack_FirstAttack)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.0), FLOAT(0.70))
    USER_FUNC(btlevtcmd_GetFirstAttackTarget, LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    SET(LW(12), PTR(&customWeapon_BobberyBombFS))
    USER_FUNC(evt_btl_camera_set_mode, 0, 11)
    USER_FUNC(evt_btl_camera_set_homing_unitparts, 0, -2, 1, LW(3), LW(4))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionSub, -2, LW(0), 250)
    USER_FUNC(btlevtcmd_SetPos, -2, LW(0), LW(1), LW(2))
    WAIT_FRM(50)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("R_2"))
    USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(6))
    MULF(LW(6), 20)
    USER_FUNC(_get_bomb_hit_position, -2, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), LW(6))
    USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetMoveSoundDataWork, -2, PTR("SFX_BTL_THUNDERS_ATTACK1"), 0, 0, -1, -1, 0, 0)
    ADD(LW(2), 5)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(5.0))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), 0, LW(2), 0, -2, 1)
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_THUNDERS_ATTACK2"), LW(14))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_3"))
    WAIT_FRM(60)
    USER_FUNC(evt_snd_sfxoff, LW(14))
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_THUNDERS_ATTACK3"), 0)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHeight, -2, LW(6))
    DIV(LW(6), 2)
    ADD(LW(1), LW(6))
    USER_FUNC(evt_eff64, PTR(""), PTR("bomb_n64"), 1, LW(0), LW(1), LW(2), 0, 0, 0, 0, 0, 0, 0, 0)
    USER_FUNC(evt_btl_camera_shake_h, 0, 5, 0, 10, 13)
    USER_FUNC(btlevtcmd_StageDispellFog)
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 131328, LW(5))
    LBL(90)
    USER_FUNC(evt_audience_ap_recovery)
    USER_FUNC(btlevtcmd_InviteApInfoReport)
    LBL(92)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("J_1A"))
    BROTHER_EVT()
        WAIT_FRM(30)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("J_1B"))
    END_BROTHER()
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.5))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 40)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 90, -1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("J_1A"))
    BROTHER_EVT()
        WAIT_FRM(8)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("J_1B"))
    END_BROTHER()
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.70))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 20)
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 16, -1)
    WAIT_FRM(20)
    LBL(95)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(6.0))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RUN_CHILD_EVT(PTR(&btldefaultevt_SuitoruBadgeEffect))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(partySandersAttack_NormalAttack)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.0), FLOAT(0.70))
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    USER_FUNC(btlevtcmd_AttackDeclare, -2, LW(3), LW(4))
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_CommandPayWeaponCost, -2)
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    USER_FUNC(btlevtcmd_CalculateFaceDirection, -2, -1, LW(3), LW(4), 16, LW(15))
    USER_FUNC(btlevtcmd_ChangeFaceDirection, -2, LW(15))
    USER_FUNC(evt_btl_camera_set_mode, 0, 11)
    USER_FUNC(evt_btl_camera_set_homing_unitparts, 0, -2, 1, LW(3), LW(4))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
    USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
    USER_FUNC(btlevtcmd_AcGetDifficulty, LW(0))
    SUB(LW(0), 3)
    SWITCH(LW(0))
        CASE_SMALL_EQUAL(-3)
            SET(LW(0), 120)
            SET(LW(1), 60)
        CASE_EQUAL(-2)
            SET(LW(0), 120)
            SET(LW(1), 45)
        CASE_EQUAL(-1)
            SET(LW(0), 120)
            SET(LW(1), 30)
        CASE_EQUAL(0)
            SET(LW(0), 120)
            SET(LW(1), 25)
        CASE_EQUAL(1)
            SET(LW(0), 120)
            SET(LW(1), 20)
        CASE_EQUAL(2)
            SET(LW(0), 120)
            SET(LW(1), 12)
        CASE_ETC()
            SET(LW(0), 120)
            SET(LW(1), 6)
    END_SWITCH()
    USER_FUNC(btlevtcmd_AcSetParamAll, 2, LW(0), LW(1), 0, 100, EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_AcSetFlag, 15)
    USER_FUNC(btlevtcmd_SetupAC, -2, 15, 1, 0)
    WAIT_FRM(22)
    BROTHER_EVT_ID(LW(15))
        SET(LW(0), 5)
        DO(LW(0))
            USER_FUNC(btlevtcmd_AcGetOutputParam, 0, LW(1))
            IF_EQUAL(LW(1), 0)
                SET(LW(0), 5)
            ELSE()
                DO_BREAK()
            END_IF()
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_StartAC, 1)
    USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
    USER_FUNC(btlevtcmd_AcGetOutputParam, 0, LW(1))
    IF_EQUAL(LW(1), 2)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 20)
        USER_FUNC(btlevtcmd_SetMoveSoundDataWork, -2, PTR("SFX_BTL_THUNDERS_ATTACK1"), 0, 0, -1, -1, 0, 0)
        USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 20, -2, 1)
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("F_1"))
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_THUNDERS_DOWN1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_FaceDirectionAdd, -2, LW(0), 20)
        USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.5))
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 12, -1)
        USER_FUNC(btlevtcmd_StopAC)
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), 0)
        WAIT_FRM(60)
        USER_FUNC(evt_audience_ap_recovery)
        USER_FUNC(btlevtcmd_InviteApInfoReport)
        GOTO(95)
    END_IF()
    USER_FUNC(btlevtcmd_CommandPreCheckDamage, -2, LW(3), LW(4), 256, LW(5))
    IF_EQUAL(LW(5), 5)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("R_2"))
        USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(5))
        MULF(LW(5), 35)
        USER_FUNC(_get_bomb_hit_position, -2, LW(3), LW(4), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), LW(5))
        USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_SetMoveSoundDataWork, -2, PTR("SFX_BTL_THUNDERS_ATTACK1"), 0, 0, -1, -1, 0, 0)
        USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), 0, LW(2), 60, -2, 1)
        USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 256, LW(5))
    END_IF()
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("R_2"))
    USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(6))
    MULF(LW(6), 20)
    USER_FUNC(_get_bomb_hit_position, -2, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), LW(6))
    USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetMoveSoundDataWork, -2, PTR("SFX_BTL_THUNDERS_ATTACK1"), 0, 0, -1, -1, 0, 0)
    ADD(LW(2), 5)
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), 0, LW(2), 90, -2, 1)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 300)
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_THUNDERS_ATTACK2"), LW(14))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_3"))
    WAIT_FRM(60)
    SWITCH(LW(5))
        CASE_OR(2)
        CASE_OR(3)
        CASE_OR(6)
        CASE_OR(4)
            CASE_END()
        CASE_EQUAL(1)
        CASE_ETC()
            WAIT_MSEC(1000)
    END_SWITCH()
    USER_FUNC(btlevtcmd_ResultAC)
    USER_FUNC(evt_snd_sfxoff, LW(14))
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_THUNDERS_ATTACK3"), 0)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetHeight, -2, LW(6))
    DIV(LW(6), 2)
    ADD(LW(1), LW(6))
    USER_FUNC(btlevtcmd_GetResultAC, LW(6))
    IF_FLAG(LW(6), 0x2)
        USER_FUNC(evt_eff64, PTR(""), PTR("bomb_n64"), 1, LW(0), LW(1), LW(2), 0, 0, 0, 0, 0, 0, 0, 0)
    ELSE()
        USER_FUNC(evt_eff64, PTR(""), PTR("bomb_n64"), 0, LW(0), LW(1), LW(2), 0, 0, 0, 0, 0, 0, 0, 0)
    END_IF()
    USER_FUNC(evt_btl_camera_shake_h, 0, 5, 0, 10, 13)
    USER_FUNC(btlevtcmd_StageDispellFog)
    IF_NOT_EQUAL(LW(5), 1)
        IF_EQUAL(LW(5), 3)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 38)
        END_IF()
        IF_EQUAL(LW(5), 6)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 39)
        END_IF()
        IF_EQUAL(LW(5), 2)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 40)
        END_IF()
        GOTO(90)
    END_IF()
    USER_FUNC(btlevtcmd_GetResultAC, LW(6))
    IF_FLAG(LW(6), 0x2)
        USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 131328, LW(5))
        USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), 0, LW(6))
        USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), LW(6))
    ELSE()
        USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 256, LW(5))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), -1)
    END_IF()
    LBL(90)
    USER_FUNC(btlevtcmd_StopAC)
    LBL(92)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("J_1A"))
    BROTHER_EVT()
        WAIT_FRM(30)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("J_1B"))
    END_BROTHER()
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.40))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    BROTHER_EVT_ID(LW(15))
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 90, -1)
    END_BROTHER()
    WAIT_FRM(60)
    DELETE_EVT(LW(15))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(5), LW(6), LW(7))
    USER_FUNC(btlevtcmd_GetStageSize, EVT_NULLPTR, LW(8), EVT_NULLPTR)
    ADD(LW(6), LW(8))
    DIV(LW(6), 2)
    USER_FUNC(btlevtcmd_SetPos, -2, LW(5), LW(6), LW(7))
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.25))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 30, -1)
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.70))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    BROTHER_EVT_ID(LW(15))
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 16, -1)
    END_BROTHER()
    USER_FUNC(btlevtcmd_ACRStart, -2, 0, 15, 15, 15)
    USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
    USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
    IF_LARGE_EQUAL(LW(6), 2)
        USER_FUNC(evtTot_LogActiveMoveStylish, 0)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("Y_1"))
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
        USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(0), 1, 0, 0, 0)
        BROTHER_EVT_ID(LW(15))
            USER_FUNC(evt_sub_intpl_init_float, 11, FLOAT(1.0), FLOAT(0.60), 8)
            DO(8)
                USER_FUNC(evt_sub_intpl_get_float, LW(5), EVT_NULLPTR)
                USER_FUNC(btlevtcmd_GetScale, -2, LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_SetScale, -2, LW(0), LW(5), LW(2))
                WAIT_FRM(1)
            WHILE()
            USER_FUNC(evt_sub_intpl_init_float, 11, FLOAT(0.60), FLOAT(1.20), 8)
            DO(8)
                USER_FUNC(evt_sub_intpl_get_float, LW(5), EVT_NULLPTR)
                USER_FUNC(btlevtcmd_GetScale, -2, LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_SetScale, -2, LW(0), LW(5), LW(2))
                WAIT_FRM(1)
            WHILE()
            USER_FUNC(evt_sub_intpl_init_float, 11, FLOAT(1.20), FLOAT(1.0), 6)
            DO(6)
                USER_FUNC(evt_sub_intpl_get_float, LW(5), EVT_NULLPTR)
                USER_FUNC(btlevtcmd_GetScale, -2, LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_SetScale, -2, LW(0), LW(5), LW(2))
                WAIT_FRM(1)
            WHILE()
        END_BROTHER()
        BROTHER_EVT()
            USER_FUNC(evt_sub_intpl_init_float, 11, FLOAT(1.0), FLOAT(1.5), 8)
            DO(8)
                USER_FUNC(evt_sub_intpl_get_float, LW(5), EVT_NULLPTR)
                USER_FUNC(btlevtcmd_GetScale, -2, LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_SetScale, -2, LW(5), LW(1), LW(2))
                WAIT_FRM(1)
            WHILE()
            USER_FUNC(evt_sub_intpl_init_float, 11, FLOAT(1.5), FLOAT(0.60), 8)
            DO(8)
                USER_FUNC(evt_sub_intpl_get_float, LW(5), EVT_NULLPTR)
                USER_FUNC(btlevtcmd_GetScale, -2, LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_SetScale, -2, LW(5), LW(1), LW(2))
                WAIT_FRM(1)
            WHILE()
            USER_FUNC(evt_sub_intpl_init_float, 11, FLOAT(0.60), FLOAT(1.0), 6)
            DO(6)
                USER_FUNC(evt_sub_intpl_get_float, LW(5), EVT_NULLPTR)
                USER_FUNC(btlevtcmd_GetScale, -2, LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_SetScale, -2, LW(5), LW(1), LW(2))
                WAIT_FRM(1)
            WHILE()
        END_BROTHER()
        USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
        WAIT_FRM(40)
    ELSE()
        USER_FUNC(evt_audience_acrobat_notry)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("F_1"))
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_THUNDERS_DOWN1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        WAIT_FRM(40)
    END_IF()
    USER_FUNC(evt_audience_ap_recovery)
    USER_FUNC(btlevtcmd_InviteApInfoReport)
    LBL(95)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(6.0))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RUN_CHILD_EVT(PTR(&btldefaultevt_SuitoruBadgeEffect))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(_shot_bomb_event)
    USER_FUNC(_shot_move, LW(3), LW(0), LW(1), LW(0), LW(1), LW(2), LW(5))
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    SETF(LW(6), LW(5))
    MULF(LW(6), FLOAT(6.40))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), LW(6))
    USER_FUNC(btlevtcmd_SetFallAccel, LW(3), FLOAT(0.5))
    USER_FUNC(btlevtcmd_JumpPosition, LW(3), LW(0), LW(1), LW(2), 16, -1)
    SETF(LW(6), LW(5))
    MULF(LW(6), FLOAT(3.60))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), LW(6))
    USER_FUNC(btlevtcmd_JumpPosition, LW(3), LW(0), LW(1), LW(2), 12, -1)
    SETF(LW(6), LW(5))
    MULF(LW(6), FLOAT(4.0))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), LW(6))
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_GetFaceDirection, LW(3), LW(0))
        MUL(LW(0), -18)
        USER_FUNC(btlevtcmd_SetRotateOffset, LW(3), 0, 7, 0)
        DO(20)
            USER_FUNC(btlevtcmd_AddRotate, LW(3), 0, 0, LW(0))
            WAIT_FRM(1)
        WHILE()
        USER_FUNC(btlevtcmd_SetRotate, LW(3), 0, 0, 0)
    END_BROTHER()
    USER_FUNC(btlevtcmd_MovePosition, LW(3), LW(0), LW(1), LW(2), 20, -1, 0)
    USER_FUNC(_judge_on_stage, LW(3), LW(5))
    IF_EQUAL(LW(5), 0)
        USER_FUNC(btlevtcmd_KillUnit, LW(3), 0)
        RETURN()
    END_IF()
    USER_FUNC(btlevtcmd_GetPos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetHomePos, LW(3), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_AnimeChangePose, LW(3), 1, PTR("B_2"))
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_THUNDERS_BOMB1"), LW(0))
    USER_FUNC(btlevtcmd_SetUnitWork, LW(3), 1, LW(0))
    WAIT_MSEC(400)
    USER_FUNC(btlevtcmd_StartWaitEvent, LW(3))
    RETURN()
EVT_END()

EVT_BEGIN(partySandersAttack_TimeBombSet)
    USER_FUNC(btlevtcmd_CommandPayWeaponCost, -2)
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(7))
    
    // Spawn a different number of bombs (2 - 4) based on move level.
    IF_EQUAL(LW(7), PTR(&customWeapon_BobberyBombSquad))
        USER_FUNC(evtTot_GetMoveSelectedLevel, MoveType::BOBBERY_BOMB_SQUAD, LW(4))
    ELSE()
        USER_FUNC(evtTot_GetMoveSelectedLevel, MoveType::BOBBERY_POISON_BOMB, LW(4))
    END_IF()
    ADD(LW(4), 1)
    SET(LW(5), 0)
    SET(LW(6), LW(4))
    DO(LW(6))
        USER_FUNC(btlevtcmd_SpawnUnit, LW(3), PTR(&unitBombzo_entry), 0)
        USER_FUNC(btlevtcmd_OnAttribute, LW(3), 16777216)
        // Switch what kind of bomb to deploy based on the move used.
        IF_EQUAL(LW(7), PTR(&customWeapon_BobberyPoisonBomb))
            USER_FUNC(btlevtcmd_SetUnitWork, LW(3), 2, 1)
            USER_FUNC(
                btlevtcmd_OnOffStatus, LW(3), StatusEffectType::POISON,
                /* turns */ 100, /* strength */ 0, /* enabled */ 1)
        END_IF()
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_SetPos, LW(3), LW(0), LW(1), LW(2))
        ADD(LW(5), 1)
        SWITCH(LW(5))
            CASE_EQUAL(1)
                SET(LW(9), LW(3))
            CASE_EQUAL(2)
                SET(LW(10), LW(3))
            CASE_EQUAL(3)
                SET(LW(11), LW(3))
            CASE_ETC()
                SET(LW(12), LW(3))
        END_SWITCH()
    WHILE()
    USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
    USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
    USER_FUNC(btlevtcmd_AcGetDifficulty, LW(0))
    SUB(LW(0), 3)
    SWITCH(LW(0))
        CASE_SMALL_EQUAL(-3)
            SET(LW(1), 80)
            SET(LW(2), 12)
        CASE_EQUAL(-2)
            SET(LW(1), 80)
            SET(LW(2), 15)
        CASE_EQUAL(-1)
            SET(LW(1), 80)
            SET(LW(2), 20)
        CASE_EQUAL(0)
            SET(LW(1), 80)
            SET(LW(2), 22)
        CASE_EQUAL(1)
            SET(LW(1), 80)
            SET(LW(2), 32)
        CASE_EQUAL(2)
            SET(LW(1), 90)
            SET(LW(2), 50)
        CASE_ETC()
            SET(LW(1), 120)
            SET(LW(2), 80)
    END_SWITCH()
    USER_FUNC(btlevtcmd_GetUnitId, -2, LW(0))
    USER_FUNC(btlevtcmd_AcSetParamAll, LW(0), LW(4), LW(1), LW(1), 0, 90, LW(2), EVT_NULLPTR)
    USER_FUNC(btlevtcmd_SetupAC, -2, 7, 1, 0)
    USER_FUNC(btlevtcmd_AcSetFlag, 1)
    WAIT_FRM(20)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_4A"))
    USER_FUNC(btlevtcmd_StartAC, 1)
    BROTHER_EVT_ID(LW(15))
        SET(LW(13), 0)
        SET(LW(14), 5)
        DO(LW(14))
            USER_FUNC(btlevtcmd_AcGetOutputParam, 0, LW(0))
            IF_NOT_EQUAL(LW(0), 0)
                SWITCH(LW(13))
                    CASE_EQUAL(0)
                        SET(LW(3), LW(9))
                    CASE_EQUAL(1)
                        SET(LW(3), LW(10))
                    CASE_EQUAL(2)
                        SET(LW(3), LW(11))
                    CASE_ETC()
                        SET(LW(3), LW(12))
                END_SWITCH()
                USER_FUNC(btlevtcmd_OffAttribute, LW(3), 16777216)
                USER_FUNC(btlevtcmd_AcGetOutputParam, 2, LW(0))
                USER_FUNC(btlevtcmd_AcGetOutputParam, 1, LW(1))
                ADD(LW(1), 20)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_4B"))
                RUN_EVT_ID(PTR(&_shot_bomb_event), LW(8))
                ADD(LW(13), 1)
                IF_LARGE_EQUAL(LW(13), LW(4))
                    USER_FUNC(btlevtcmd_AnimeWaitPlayComplete, -2, 1)
                    WAIT_FRM(10)
                    USER_FUNC(btlevtcmd_ACRStart, -2, 0, 12, 12, 10)
                    USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
                    SWITCH(LW(6))
                        CASE_LARGE_EQUAL(2)
                            USER_FUNC(evtTot_LogActiveMoveStylish, 0)
                            USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
                            USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(0), 1, 0, 0, 0)
                            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("Y_1"))
                        CASE_ETC()
                            USER_FUNC(evt_audience_acrobat_notry)
                    END_SWITCH()
                    USER_FUNC(btlevtcmd_WaitEventEnd, LW(8))
                    DO_BREAK()
                END_IF()
                USER_FUNC(btlevtcmd_AnimeWaitPlayComplete, -2, 1)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_4A"))
                USER_FUNC(btlevtcmd_AcSetOutputParam, 0, 0)
            END_IF()
            WAIT_FRM(1)
            SET(LW(14), 5)
        WHILE()
    END_BROTHER()
    USER_FUNC(btlevtcmd_ResultAC)
    WAIT_FRM(60)
    USER_FUNC(btlevtcmd_StopAC)
    USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
    LBL(90)
    USER_FUNC(evt_audience_ap_recovery)
    USER_FUNC(btlevtcmd_InviteApInfoReport)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(partySandersAttack_CounterSet)
    USER_FUNC(btlevtcmd_JumpSetting, -2, 20, FLOAT(0.0), FLOAT(0.70))
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    SET(LW(12), PTR(&customWeapon_BobberyHoldFast))
    USER_FUNC(btlevtcmd_PayWeaponCost, -2, LW(12))
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    WAIT_FRM(20)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), EVT_NULLPTR, EVT_NULLPTR)
    USER_FUNC(btlevtcmd_GetPos, -3, LW(1), EVT_NULLPTR, EVT_NULLPTR)
    ADD(LW(0), LW(1))
    DIV(LW(0), 2)
    USER_FUNC(evt_btl_camera_set_mode, 0, 3)
    USER_FUNC(evt_btl_camera_set_moveto, 1, LW(0), 73, 400, LW(0), 43, 0, 30, 5)
    USER_FUNC(btlevtcmd_GetBodyId, -2, LW(9))
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, LW(9), PTR("O_1"))
    USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
    USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
    USER_FUNC(btlevtcmd_AcSetParamAll, 3, 0, 40, 1, 0, 1, 2, 0)
    USER_FUNC(btlevtcmd_AcSetFlag, 8)
    USER_FUNC(btlevtcmd_SetupAC, -2, 18, 1, 0)
    WAIT_FRM(52)
    USER_FUNC(btlevtcmd_AnimeWaitPlayComplete, -2, 1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_2"))
    USER_FUNC(btlevtcmd_StartAC, 1)
    USER_FUNC(btlevtcmd_ResultAC)
    USER_FUNC(btlevtcmd_GetResultAC, LW(0))
    USER_FUNC(btlevtcmd_StopAC)
    IF_NOT_FLAG(LW(0), 0x2)
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), -1)
    ELSE()
        USER_FUNC(btlevtcmd_AcGetOutputParam, 0, LW(0))
        SUB(LW(0), 1)
        USER_FUNC(btlevtcmd_GetResultPrizeLv, -1, LW(0), LW(6))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), LW(6))
    END_IF()
    USER_FUNC(btlevtcmd_ACRStart, -2, 0, 12, 12, 0)
    USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
    SWITCH(LW(6))
        CASE_LARGE_EQUAL(2)
            USER_FUNC(evtTot_LogActiveMoveStylish, 0)
            USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
            USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(0), 1, 0, 0, 0)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("Y_1"))
            WAIT_FRM(30)
        CASE_ETC()
            USER_FUNC(evt_audience_acrobat_notry)
    END_SWITCH()
    USER_FUNC(btlevtcmd_AcGetOutputParam, 0, LW(0))
    ADD(LW(0), 1)
    USER_FUNC(_make_counterset_weapon, LW(12), LW(0))
    WAIT_FRM(20)
    INLINE_EVT()
        WAIT_FRM(30)
        USER_FUNC(evt_btl_camera_set_mode, 0, 0)
        USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    END_INLINE()
    USER_FUNC(evt_audience_ap_recovery)
    USER_FUNC(btlevtcmd_InviteApInfoReport)
    USER_FUNC(btlevtcmd_CheckDamage, -2, LW(3), LW(4), LW(12), 256, LW(5))
    WAIT_FRM(80)
    LBL(99)
    USER_FUNC(evt_audience_ap_recovery)
    USER_FUNC(btlevtcmd_InviteApInfoReport)
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(partySandersAttack_SuperBombAttack)
    USER_FUNC(btlevtcmd_GetSelectEnemy, LW(3), LW(4))
    IF_EQUAL(LW(3), -1)
        GOTO(99)
    END_IF()
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    USER_FUNC(btlevtcmd_WeaponAftereffect, LW(12))
    USER_FUNC(btlevtcmd_AttackDeclareAll, -2)
    USER_FUNC(btlevtcmd_WaitGuardMove)
    USER_FUNC(btlevtcmd_CommandPayWeaponCost, -2)
    USER_FUNC(btlevtcmd_RunDataEventChild, -2, 7)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("Y_1"))
    USER_FUNC(evt_btl_camera_set_mode, 0, 11)
    USER_FUNC(evt_btl_camera_set_homing_unitparts, 0, -2, 1, LW(3), LW(4))
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 200)
    USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(0))
    USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(0))
    USER_FUNC(btlevtcmd_AcGetDifficulty, LW(0))
    SUB(LW(0), 3)
    SWITCH(LW(0))
        CASE_SMALL_EQUAL(-3)
            SET(LW(0), 300)
            SET(LW(1), 20)
            SET(LW(2), 20)
        CASE_EQUAL(-2)
            SET(LW(0), 270)
            SET(LW(1), 18)
            SET(LW(2), 22)
        CASE_EQUAL(-1)
            SET(LW(0), 255)
            SET(LW(1), 17)
            SET(LW(2), 23)
        CASE_EQUAL(0)
            SET(LW(0), 225)
            SET(LW(1), 16)
            SET(LW(2), 24)
        CASE_EQUAL(1)
            SET(LW(0), 210)
            SET(LW(1), 15)
            SET(LW(2), 26)
        CASE_EQUAL(2)
            SET(LW(0), 180)
            SET(LW(1), 14)
            SET(LW(2), 28)
        CASE_ETC()
            SET(LW(0), 150)
            SET(LW(1), 13)
            SET(LW(2), 28)
    END_SWITCH()
    // Change gauge parameters and difficulty based on move level.
    USER_FUNC(evtTot_GetMoveSelectedLevel, MoveType::BOBBERY_BOBOMBAST, LW(5))
    SWITCH(LW(5))
        CASE_EQUAL(1)
            ADD(LW(0), 2)
            SUB(LW(1), 2)
            USER_FUNC(btlevtcmd_AcSetParamAll, 1, LW(0), 178, LW(1), LW(2), 25, 100, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_AcSetGaugeParam, 100, 100, 100, 100)
        CASE_EQUAL(2)
            ADD(LW(0), 1)
            SUB(LW(1), 1)
            USER_FUNC(btlevtcmd_AcSetParamAll, 1, LW(0), 178, LW(1), LW(2), 17, 69, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_AcSetGaugeParam, 68, 100, 100, 100)
        CASE_ETC()
            USER_FUNC(btlevtcmd_AcSetParamAll, 1, LW(0), 178, LW(1), LW(2), 13, 53, EVT_NULLPTR)
            USER_FUNC(btlevtcmd_AcSetGaugeParam, 52, 78, 100, 100)
    END_SWITCH()
    USER_FUNC(btlevtcmd_AcSetFlag, 64)
    USER_FUNC(btlevtcmd_SetupAC, -2, 6, 1, 0)
    WAIT_FRM(22)
    USER_FUNC(btlevtcmd_StartAC, 1)
    USER_FUNC(btlevtcmd_CommandPreCheckDamage, -2, LW(3), LW(4), 256, LW(5))
    IF_EQUAL(LW(5), 5)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("R_2"))
        USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(6))
        MULF(LW(6), 35)
        USER_FUNC(_get_bomb_hit_position, -2, LW(3), LW(4), LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), LW(6))
        USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_SetMoveSoundDataWork, -2, PTR("SFX_BTL_THUNDERS_ATTACK1"), 0, 0, -1, -1, 0, 0)
        USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), 0, LW(2), 60, -2, 1)
        USER_FUNC(btlevtcmd_SetScale, -2, FLOAT(1.0), FLOAT(1.0), FLOAT(1.0))
        USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 256, LW(5))
        RETURN()
    END_IF()
    BROTHER_EVT()
        SUB(LW(0), 85)
        WAIT_FRM(LW(0))
        USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
        USER_FUNC(evt_eff, PTR(""), PTR("sandars"), 5, LW(0), LW(1), LW(2), 60, 0, 0, 0, 0, 0, 0, 0)
        WAIT_FRM(15)
        USER_FUNC(evt_eff, PTR(""), PTR("sandars"), 5, LW(0), LW(1), LW(2), 150, 0, 0, 0, 0, 0, 0, 0)
        WAIT_FRM(15)
        USER_FUNC(evt_eff, PTR(""), PTR("sandars"), 5, LW(0), LW(1), LW(2), 30, 0, 0, 0, 0, 0, 0, 0)
        WAIT_FRM(15)
        USER_FUNC(evt_eff, PTR(""), PTR("sandars"), 5, LW(0), LW(1), LW(2), 120, 0, 0, 0, 0, 0, 0, 0)
        WAIT_FRM(40)
    END_BROTHER()
    BROTHER_EVT()
        SUB(LW(0), 85)
        WAIT_FRM(LW(0))
        USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_THUNDERS_ATTACK2"), LW(14))
        SET(LW(0), 0)
        DO(80)
            ADD(LW(0), 1)
            IF_LARGE(LW(0), 80)
                SET(LW(0), 80)
            END_IF()
            SET(LW(1), 4096)
            MUL(LW(1), LW(0))
            DIV(LW(1), 80)
            USER_FUNC(evt_snd_sfx_pit, LW(14), LW(1))
            WAIT_FRM(1)
        WHILE()
        WAIT_FRM(3)
        USER_FUNC(evt_snd_sfxoff, LW(14))
    END_BROTHER()
    BROTHER_EVT()
        DO(LW(0))
            USER_FUNC(btlevtcmd_AcGetOutputParam, 1, LW(0))
            ADD(LW(0), 100)
            MULF(LW(0), FLOAT(0.01))
            USER_FUNC(btlevtcmd_SetScale, -2, LW(0), LW(0), FLOAT(1.0))
            WAIT_FRM(1)
        WHILE()
    END_BROTHER()
    SET(LW(13), LW(0))
    SUB(LW(13), 85)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("R_2"))
    USER_FUNC(btlevtcmd_GetStatusMg, -2, LW(6))
    MULF(LW(6), 20)
    USER_FUNC(_get_bomb_hit_position, -2, LW(3), LW(4), LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_FaceDirectionAdd, LW(3), LW(0), LW(6))
    USER_FUNC(btlevtcmd_TransStageFloorPosition, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_SetMoveSoundDataWork, -2, PTR("SFX_BTL_THUNDERS_ATTACK1"), 0, 0, -1, -1, 0, 0)
    ADD(LW(2), 5)
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), LW(13), -2, 1)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    USER_FUNC(evt_btl_camera_set_zoom, 0, 300)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("A_3"))
    USER_FUNC(btlevtcmd_ResultAC)
    USER_FUNC(btlevtcmd_StopAC)
    SWITCH(LW(5))
        CASE_OR(2)
        CASE_OR(3)
        CASE_OR(6)
        CASE_OR(4)
            CASE_END()
        CASE_EQUAL(1)
        CASE_ETC()
            WAIT_MSEC(1000)
    END_SWITCH()
    USER_FUNC(btlevtcmd_ResultAC)
    USER_FUNC(btlevtcmd_SetScale, -2, FLOAT(1.0), FLOAT(1.0), FLOAT(1.0))
    USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_THUNDERS_ATTACK4"), 0)
    USER_FUNC(btlevtcmd_GetPos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_GetResultAC, LW(6))
    IF_FLAG(LW(6), 0x2)
        USER_FUNC(evt_eff, PTR(""), PTR("sandars"), 3, LW(0), LW(1), LW(2), 0, 0, 0, 0, 0, 0, 0, 0)
    ELSE()
        USER_FUNC(evt_eff, PTR(""), PTR("sandars"), 4, LW(0), LW(1), LW(2), 0, 0, 0, 0, 0, 0, 0, 0)
    END_IF()
    USER_FUNC(btlevtcmd_StageDispellFog)
    USER_FUNC(evt_btl_camera_shake_h, 0, 10, 0, 20, 13)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 2)
    SET(LW(10), 0)
    LBL(10)
    USER_FUNC(btlevtcmd_CommandPreCheckDamage, -2, LW(3), LW(4), 256, LW(6))
    IF_NOT_EQUAL(LW(6), 1)
        IF_EQUAL(LW(6), 3)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 38)
        END_IF()
        IF_EQUAL(LW(6), 6)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 39)
        END_IF()
        IF_EQUAL(LW(6), 2)
            USER_FUNC(btlevtcmd_StartAvoid, LW(3), 40)
        END_IF()
        GOTO(50)
    END_IF()
    USER_FUNC(btlevtcmd_GetResultAC, LW(0))
    IF_FLAG(LW(0), 0x2)
        USER_FUNC(btlevtcmd_AcGetOutputParam, 2, LW(0))
        SWITCH(LW(0))
            CASE_SMALL_EQUAL(4)
                SET(LW(0), -1)
            CASE_SMALL_EQUAL(6)
                SET(LW(0), 0)
            CASE_SMALL_EQUAL(8)
                SET(LW(0), 1)
            CASE_ETC()
                SET(LW(0), 2)
        END_SWITCH()
        IF_LARGE_EQUAL(LW(0), 0)
            USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 131328, LW(5))
            USER_FUNC(btlevtcmd_GetResultPrizeLv, LW(3), LW(0), LW(6))
            USER_FUNC(btlevtcmd_GetHitPos, LW(3), LW(4), LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), LW(0), LW(1), LW(2))
            USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), LW(6))
        ELSE()
            USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 256, LW(5))
            USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), -1)
        END_IF()
    ELSE()
        USER_FUNC(btlevtcmd_CommandCheckDamage, -2, LW(3), LW(4), 256, LW(5))
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
        USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(0), -1)
    END_IF()
    LBL(50)
    USER_FUNC(btlevtcmd_GetSelectNextEnemy, LW(3), LW(4))
    IF_NOT_EQUAL(LW(3), -1)
        ADD(LW(10), 1)
        GOTO(10)
    END_IF()
    LBL(90)
    USER_FUNC(btlevtcmd_StopAC)
    LBL(92)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("J_1A"))
    BROTHER_EVT()
        WAIT_FRM(30)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("J_1B"))
        USER_FUNC(btlevtcmd_SetScale, -2, FLOAT(1.0), FLOAT(1.0), FLOAT(1.0))
    END_BROTHER()
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.40))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    BROTHER_EVT_ID(LW(15))
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 90, -1)
    END_BROTHER()
    WAIT_FRM(60)
    DELETE_EVT(LW(15))
    USER_FUNC(btlevtcmd_GetPos, -2, LW(5), LW(6), LW(7))
    USER_FUNC(btlevtcmd_GetStageSize, EVT_NULLPTR, LW(8), EVT_NULLPTR)
    ADD(LW(6), LW(8))
    DIV(LW(6), 2)
    USER_FUNC(btlevtcmd_SetPos, -2, LW(5), LW(6), LW(7))
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.25))
    USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 30, -1)
    USER_FUNC(btlevtcmd_SetFallAccel, -2, FLOAT(0.70))
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    BROTHER_EVT_ID(LW(15))
        USER_FUNC(btlevtcmd_JumpPosition, -2, LW(0), LW(1), LW(2), 16, -1)
    END_BROTHER()
    USER_FUNC(btlevtcmd_ACRStart, -2, 0, 15, 15, 15)
    USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
    USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
    IF_LARGE_EQUAL(LW(6), 2)
        USER_FUNC(evtTot_LogActiveMoveStylish, 0)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("Y_1"))
        USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
        USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(0), 1, 0, 0, 0)
        BROTHER_EVT_ID(LW(15))
            USER_FUNC(evt_sub_intpl_init_float, 11, FLOAT(1.0), FLOAT(0.60), 8)
            DO(8)
                USER_FUNC(evt_sub_intpl_get_float, LW(5), EVT_NULLPTR)
                USER_FUNC(btlevtcmd_GetScale, -2, LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_SetScale, -2, LW(0), LW(5), LW(2))
                WAIT_FRM(1)
            WHILE()
            USER_FUNC(evt_sub_intpl_init_float, 11, FLOAT(0.60), FLOAT(1.20), 8)
            DO(8)
                USER_FUNC(evt_sub_intpl_get_float, LW(5), EVT_NULLPTR)
                USER_FUNC(btlevtcmd_GetScale, -2, LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_SetScale, -2, LW(0), LW(5), LW(2))
                WAIT_FRM(1)
            WHILE()
            USER_FUNC(evt_sub_intpl_init_float, 11, FLOAT(1.20), FLOAT(1.0), 6)
            DO(6)
                USER_FUNC(evt_sub_intpl_get_float, LW(5), EVT_NULLPTR)
                USER_FUNC(btlevtcmd_GetScale, -2, LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_SetScale, -2, LW(0), LW(5), LW(2))
                WAIT_FRM(1)
            WHILE()
        END_BROTHER()
        BROTHER_EVT()
            USER_FUNC(evt_sub_intpl_init_float, 11, FLOAT(1.0), FLOAT(1.5), 8)
            DO(8)
                USER_FUNC(evt_sub_intpl_get_float, LW(5), EVT_NULLPTR)
                USER_FUNC(btlevtcmd_GetScale, -2, LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_SetScale, -2, LW(5), LW(1), LW(2))
                WAIT_FRM(1)
            WHILE()
            USER_FUNC(evt_sub_intpl_init_float, 11, FLOAT(1.5), FLOAT(0.60), 8)
            DO(8)
                USER_FUNC(evt_sub_intpl_get_float, LW(5), EVT_NULLPTR)
                USER_FUNC(btlevtcmd_GetScale, -2, LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_SetScale, -2, LW(5), LW(1), LW(2))
                WAIT_FRM(1)
            WHILE()
            USER_FUNC(evt_sub_intpl_init_float, 11, FLOAT(0.60), FLOAT(1.0), 6)
            DO(6)
                USER_FUNC(evt_sub_intpl_get_float, LW(5), EVT_NULLPTR)
                USER_FUNC(btlevtcmd_GetScale, -2, LW(0), LW(1), LW(2))
                USER_FUNC(btlevtcmd_SetScale, -2, LW(5), LW(1), LW(2))
                WAIT_FRM(1)
            WHILE()
        END_BROTHER()
        USER_FUNC(btlevtcmd_WaitEventEnd, LW(15))
        WAIT_FRM(40)
    ELSE()
        USER_FUNC(evt_audience_acrobat_notry)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("F_1"))
        USER_FUNC(btlevtcmd_snd_se, -2, PTR("SFX_BTL_THUNDERS_DOWN1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        WAIT_FRM(40)
    END_IF()
    LBL(95)
    USER_FUNC(evt_audience_ap_recovery)
    USER_FUNC(btlevtcmd_InviteApInfoReport)
    USER_FUNC(evt_btl_camera_set_mode, 0, 0)
    USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
    USER_FUNC(btlevtcmd_SetMoveSpeed, -2, FLOAT(6.0))
    USER_FUNC(btlevtcmd_AnimeChangePoseType, -2, 1, 40)
    USER_FUNC(btlevtcmd_GetHomePos, -2, LW(0), LW(1), LW(2))
    USER_FUNC(btlevtcmd_MovePosition, -2, LW(0), LW(1), LW(2), 0, -1, 0)
    LBL(99)
    USER_FUNC(btlevtcmd_ResetFaceDirection, -2)
    RUN_CHILD_EVT(PTR(&btldefaultevt_SuitoruBadgeEffect))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

EVT_BEGIN(customAttack_MegatonBomb)
    USER_FUNC(btlevtcmd_CommandPayWeaponCost, -2)
    USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(12))
    
    USER_FUNC(btlevtcmd_SpawnUnit, LW(9), PTR(&unitBombzo_entry), 0)
    USER_FUNC(btlevtcmd_SetUnitWork, LW(9), 2, 2)
    USER_FUNC(btlevtcmd_SetScale, LW(9), FLOAT(3.0), FLOAT(3.0), FLOAT(3.0))
    USER_FUNC(btlevtcmd_SetPos, LW(9), FLOAT(0.0), FLOAT(400.0), FLOAT(-30.0))

    // Play falling sound, set up Stylish command right as the bomb lands.
    BROTHER_EVT()
        WAIT_FRM(45)
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_1"))
        USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_SAC_TIME3"), LW(6))
        WAIT_FRM(39)
        USER_FUNC(evt_snd_sfxoff, LW(6))
        USER_FUNC(btlevtcmd_ACRStart, -2, 0, 12, 12, 0)
        USER_FUNC(btlevtcmd_ACRGetResult, LW(6), LW(7))
        SWITCH(LW(6))
            CASE_LARGE_EQUAL(2)
                USER_FUNC(evtTot_LogActiveMoveStylish, 0)
                USER_FUNC(btlevtcmd_CommandGetWeaponAddress, -2, LW(0))
                USER_FUNC(btlevtcmd_AudienceDeclareAcrobatResult, LW(0), 1, 0, 0, 0)
                USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("Y_1"))
                WAIT_FRM(50)
            CASE_ETC()
                USER_FUNC(evt_audience_acrobat_notry)
        END_SWITCH()
    END_BROTHER()
    
    USER_FUNC(btlevtcmd_SetFallAccel, LW(9), FLOAT(0.4))
    USER_FUNC(btlevtcmd_FallPosition, LW(9), FLOAT(0.0), FLOAT(0.0), FLOAT(-30.0), 90)
    USER_FUNC(evt_btl_camera_shake_h, 0, 10, 0, 60, 4)
    USER_FUNC(btlevtcmd_snd_se, LW(9), PTR("SFX_BOSS_GNB_APPEAR1"), EVT_NULLPTR, 0, EVT_NULLPTR)
    
    // Mario gets jumpscared as the bomb lands.
    BROTHER_EVT()
        USER_FUNC(btlevtcmd_AnimeChangePose, -3, 1, PTR("M_B_3"))
        USER_FUNC(btlevtcmd_snd_se, -3, PTR("SFX_VOICE_MARIO_SURPRISED2_2"), EVT_NULLPTR, 0, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_JumpSetting, -3, 24, FLOAT(0.0), FLOAT(0.7))
        USER_FUNC(btlevtcmd_GetPos, -3, LW(0), LW(1), LW(2))
        USER_FUNC(btlevtcmd_JumpPosition, -3, LW(0), LW(1), LW(2), 0, -1)
        USER_FUNC(btlevtcmd_AnimeChangePose, -3, 1, PTR("M_S_1"))
    END_BROTHER()

    WAIT_FRM(45)
    
    // Do Kiss Thief command 4 times with progressively tighter timing.
    USER_FUNC(btlevtcmd_CommandGetWeaponActionLv, LW(14))
    USER_FUNC(btlevtcmd_AcSetDifficulty, -2, LW(14))
    USER_FUNC(btlevtcmd_AcGetDifficulty, LW(14))
    // Difficulty parameter (based on Kiss Thief _get_itemsteal_param)
    MUL(LW(14), -20)
    ADD(LW(14), 140)
    // Initial success threshold (fullness%)
    SET(LW(15), 72)
    SET(LW(11), -1)
    SET(LW(13), 4)
    DO(LW(13))
        USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_1"))
        IF_EQUAL(LW(11), 1)
            // Have bomb fuse light and Mario start panicking on the third bar.
            USER_FUNC(btlevtcmd_AnimeChangePose, LW(9), 1, PTR("B_2"))
            USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_THUNDERS_BOMB1"), LW(0))
            USER_FUNC(btlevtcmd_SetUnitWork, LW(9), 1, LW(0))
            
            USER_FUNC(btlevtcmd_AnimeChangePose, -3, 1, PTR("M_N_7"))
            USER_FUNC(btlevtcmd_snd_se, -3, PTR("SFX_VOICE_MARIO_SURPRISED1"), EVT_NULLPTR, 0, EVT_NULLPTR)
        END_IF()
        USER_FUNC(btlevtcmd_AcSetParamAll, 1, LW(14), 2, 2, LW(15), EVT_NULLPTR, EVT_NULLPTR, EVT_NULLPTR)
        USER_FUNC(btlevtcmd_AcSetFlag, 0)
        USER_FUNC(btlevtcmd_SetupAC, -2, 15, 1, 0)
        WAIT_FRM(22)
        USER_FUNC(btlevtcmd_StartAC, 1)
        USER_FUNC(btlevtcmd_ResultAC)
        USER_FUNC(btlevtcmd_GetResultAC, LW(6))
        IF_FLAG(LW(6), 0x2)
            USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("Y_1"))
            ADD(LW(15), 8)
            ADD(LW(11), 1)
            USER_FUNC(btlevtcmd_GetResultPrizeLv, -1, LW(11), LW(6))
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), LW(6))
            USER_FUNC(btlevtcmd_ACSuccessEffect, LW(6), FLOAT(20.0), FLOAT(30.0), FLOAT(0.0))
            USER_FUNC(btlevtcmd_snd_se, LW(9), PTR("SFX_CONDITION_DEKADEKA1"), EVT_NULLPTR, 0, EVT_NULLPTR)
            DO(50)
                USER_FUNC(btlevtcmd_AddScale, LW(9), FLOAT(0.01), FLOAT(0.01), FLOAT(0.01))
                WAIT_FRM(1)
            WHILE()
        ELSE()
            USER_FUNC(btlevtcmd_InviteApInfoReport)
            USER_FUNC(btlevtcmd_AudienceDeclareACResult, LW(12), -1)
            USER_FUNC(evt_btl_camera_set_mode, 0, 0)
            USER_FUNC(evt_btl_camera_set_moveSpeedLv, 0, 1)
            DO_BREAK()
        END_IF()
    WHILE()
        
    USER_FUNC(btlevtcmd_AnimeChangePose, -3, 1, PTR("M_S_1"))
    IF_SMALL(LW(11), 1)
        // Have bomb fuse light if not already lit.
        USER_FUNC(btlevtcmd_AnimeChangePose, LW(9), 1, PTR("B_2"))
        USER_FUNC(evt_snd_sfxon, PTR("SFX_BTL_THUNDERS_BOMB1"), LW(0))
        USER_FUNC(btlevtcmd_SetUnitWork, LW(9), 1, LW(0))
    END_IF()
    IF_EQUAL(LW(11), 3)
        USER_FUNC(btlevtcmd_InviteApInfoReport)
    END_IF()
    WAIT_FRM(30)
    USER_FUNC(btlevtcmd_AnimeChangePose, -2, 1, PTR("S_1"))
    
    // Set final damage value - 4 x (number of completed bars + 1).
    ADD(LW(11), 2)
    MUL(LW(11), 4)
    USER_FUNC(btlevtcmd_SetUnitWork, LW(9), 3, LW(11))
    USER_FUNC(btlevtcmd_StartWaitEvent, -2)
    RETURN()
EVT_END()

BattleWeapon customWeapon_BobberyBomb = {
    .name = "btl_wn_pbm_normal",
    .icon = IconType::PARTNER_MOVE_0,
    .item_id = 0,
    .description = "msg_pbm_bakuhatsu",
    .base_accuracy = 100,
    .base_fp_cost = 0,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 1,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)GetWeaponPowerFromSelectedLevel,
    .damage_function_params = { 2, 3, 3, 5, 4, 8, 0, MoveType::BOBBERY_BASE },
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
    .element = AttackElement::EXPLOSION,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_bakuhatsu",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::USABLE_IF_CONFUSED |
        AttackSpecialProperty_Flags::GROUNDS_WINGED |
        AttackSpecialProperty_Flags::FLIPS_BOMB |
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags =
        AttackCounterResistance_Flags::ALL &
        ~AttackCounterResistance_Flags::PREEMPTIVE_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)partySandersAttack_NormalAttack,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 10,
    .bg_a2_fall_weight = 10,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 10,
    .nozzle_turn_chance = 10,
    .nozzle_fire_chance = 10,
    .ceiling_fall_chance = 5,
    .object_fall_chance = 10,
};

BattleWeapon customWeapon_BobberyBombFS = {
    .name = nullptr,
    .icon = IconType::PARTNER_MOVE_0,
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
    .unk_1b = 50,
    .damage_function = (void*)GetWeaponPowerFromMaxLevel,
    .damage_function_params = { 3, 3, 3, 3, 3, 3, 0, 0 },
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
    .element = AttackElement::EXPLOSION,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = nullptr,
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::GROUNDS_WINGED |
        AttackSpecialProperty_Flags::FLIPS_BOMB |
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags =
        AttackCounterResistance_Flags::ALL &
        ~AttackCounterResistance_Flags::PREEMPTIVE_SPIKY,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)partySandersAttack_FirstAttack,
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

BattleWeapon customWeapon_BobberyBombSquad = {
    .name = "btl_wn_pbm_lv1",
    .icon = IconType::PARTNER_MOVE_1,
    .item_id = 0,
    .description = "msg_pbm_jigen_bakudan",
    .base_accuracy = 100,
    .base_fp_cost = 3,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)weaponGetPowerDefault,  // for treating as attack
    .damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
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
    .ac_help_msg = "msg_ac_jigen_bakudan",
    .special_property_flags = AttackSpecialProperty_Flags::UNGUARDABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)partySandersAttack_TimeBombSet,
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

BattleWeapon customWeapon_BobberyHoldFast = {
    .name = "btl_wn_pbm_lv2",
    .icon = IconType::PARTNER_MOVE_1,
    .item_id = 0,
    .description = "msg_pbm_counter",
    .base_accuracy = 100,
    .base_fp_cost = 4,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
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
    .ac_help_msg = "msg_ac_bom_counter",
    .special_property_flags =
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::IGNORES_STATUS_CHANCE |
        AttackSpecialProperty_Flags::CANNOT_MISS,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags = 0,
        
    // status chances
    .hold_fast_time = 1,
    
    .attack_evt_code = (void*)partySandersAttack_CounterSet,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 5,
    .bg_a2_fall_weight = 5,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 20,
    .nozzle_turn_chance = 10,
    .nozzle_fire_chance = 5,
    .ceiling_fall_chance = 5,
    .object_fall_chance = 5,
};

BattleWeapon customWeapon_BobberyBobombast = {
    .name = "btl_wn_pbm_lv3",
    .icon = IconType::PARTNER_MOVE_2,
    .item_id = 0,
    .description = "msg_pbm_sungoi_bakuhatsu",
    .base_accuracy = 100,
    .base_fp_cost = 9,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)weaponGetACOutputParam,
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
    .element = AttackElement::EXPLOSION,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_super_bakuhatsu",
    .special_property_flags = 
        AttackSpecialProperty_Flags::UNGUARDABLE |
        AttackSpecialProperty_Flags::GROUNDS_WINGED |
        AttackSpecialProperty_Flags::FLIPS_BOMB |
        AttackSpecialProperty_Flags::FREEZE_BREAK |
        AttackSpecialProperty_Flags::ALL_BUFFABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)partySandersAttack_SuperBombAttack,
    .bg_a1_a2_fall_weight = 0,
    .bg_a1_fall_weight = 50,
    .bg_a2_fall_weight = 50,
    .bg_no_a_fall_weight = 100,
    .bg_b_fall_weight = 20,
    .nozzle_turn_chance = 25,
    .nozzle_fire_chance = 25,
    .ceiling_fall_chance = 10,
    .object_fall_chance = 10,
};

BattleWeapon customWeapon_BobberyPoisonBomb = {
    .name = "tot_ptr6_poisonbomb",
    .icon = IconType::PARTNER_MOVE_2,
    .item_id = 0,
    .description = "tot_ptr6_poisonbomb_desc",
    .base_accuracy = 100,
    .base_fp_cost = 3,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)weaponGetPowerDefault,  // for treating as attack
    .damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
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
    .ac_help_msg = "msg_ac_jigen_bakudan",
    .special_property_flags = AttackSpecialProperty_Flags::UNGUARDABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)partySandersAttack_TimeBombSet,
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

BattleWeapon customWeapon_BobberyMegatonBomb = {
    .name = "tot_ptr6_megatonbomb",
    .icon = IconType::PARTNER_MOVE_3,
    .item_id = 0,
    .description = "tot_ptr6_megatonbomb_desc",
    .base_accuracy = 100,
    .base_fp_cost = 3,
    .base_sp_cost = 0,
    .superguards_allowed = 0,
    .unk_14 = 1.0,
    .stylish_multiplier = 1,
    .unk_19 = 5,
    .bingo_card_chance = 100,
    .unk_1b = 50,
    .damage_function = (void*)weaponGetPowerDefault,  // for treating as attack
    .damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .fp_damage_function = nullptr,
    .fp_damage_function_params = { 0, 0, 0, 0, 0, 0, 0, 0 },
    .target_class_flags =
        AttackTargetClass_Flags::MULTIPLE_TARGET |
        AttackTargetClass_Flags::ONLY_TARGET_PREFERRED_PARTS |
        AttackTargetClass_Flags::CANNOT_TARGET_SYSTEM_UNITS |
        AttackTargetClass_Flags::CANNOT_TARGET_TREE_OR_SWITCH,
    .target_property_flags =
        AttackTargetProperty_Flags::TARGET_OPPOSING_ALLIANCE_DIR,
    .element = AttackElement::NORMAL,
    .damage_pattern = 0,
    .weapon_ac_level = 3,
    .unk_6f = 2,
    .ac_help_msg = "msg_ac_heart_catch",
    .special_property_flags = AttackSpecialProperty_Flags::UNGUARDABLE,
    .counter_resistance_flags = AttackCounterResistance_Flags::ALL,
    .target_weighting_flags =
        AttackTargetWeighting_Flags::WEIGHTED_RANDOM |
        AttackTargetWeighting_Flags::UNKNOWN_0x2000 |
        AttackTargetWeighting_Flags::PREFER_FRONT,
        
    // status chances
    
    .attack_evt_code = (void*)customAttack_MegatonBomb,
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

}  // namespace mod::tot::party_bobbery