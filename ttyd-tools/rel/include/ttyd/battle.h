#pragma once

#include <gc/types.h>
#include <ttyd/battle_database_common.h>

#include <cstdint>

namespace ttyd::battle_unit {
struct BattleWorkUnit;
struct BattleWorkUnitPart;
}
namespace ttyd::npcdrv {
struct FbatBattleInformation;
}

namespace ttyd::battle {

struct BattleWorkAlliance {
    int16_t     identifier;  // 2 for player, 1 for enemy, 0 for neutral
    int8_t      attack_direction;
    int8_t      pad_03;
    uint32_t    loss_condition_met;    
};

static_assert(sizeof(BattleWorkAlliance) == 0x8);

struct BattleWorkTarget {
    int16_t     unit_idx;
    int16_t     part_idx;  // one-indexed
    int16_t     hit_cursor_pos_x;
    int16_t     hit_cursor_pos_y;
    int16_t     hit_cursor_pos_z;
    int16_t     pad_0a;
    int32_t     final_pos_x;
    int32_t     final_pos_y;
    int32_t     final_pos_z;
    int32_t     addl_offset_x;
    int8_t      forward_distance;
    bool        fg_or_bg_layer;
    int16_t     pad_1e;
    int32_t     unk_20;
} ;

static_assert(sizeof(BattleWorkTarget) == 0x24);

struct BattleWorkWeaponTargets {
    battle_database_common::BattleWeapon* weapon;
    BattleWorkTarget    targets[74];
    int8_t              num_targets;
    int8_t              target_indices[74];
    int8_t              current_target;
    int32_t             attacker_idx;
    int32_t             attacker_enemy_belong;
    uint32_t            weapon_target_class_flags;
    uint32_t            weapon_target_property_flags;
    int32_t             attacking_direction;
} ;

static_assert(sizeof(BattleWorkWeaponTargets) == 0xacc);

struct BattleWorkCommandAction {
    uint32_t        type;
    uint32_t        enabled;
    const char*     description;
    int16_t         icon;
    int16_t         pad_0e;
    uint32_t        unk_10;  // has to do with greying out?
} ;

static_assert(sizeof(BattleWorkCommandAction) == 0x14);

struct BattleWorkCommandWeapon {
    battle_database_common::BattleWeapon* weapon;
    uint32_t        unk_04;
    const char*     name;
    int16_t         icon;
    int16_t         pad_0e;
    int32_t         index;
    uint32_t        item_id;
    uint32_t        unk_18;
} ;

static_assert(sizeof(BattleWorkCommandWeapon) == 0x1c);

struct BattleWorkCommandOperation {
    uint32_t        type;
    uint32_t        enabled;
    uint32_t        unk_08;  // has to do with greying out?
    const char*     name;
    int16_t         icon;
    int16_t         pad_12;
    const char*     help_message;
    int32_t         cost;
} ;

static_assert(sizeof(BattleWorkCommandOperation) == 0x1c);

struct BattleWorkCommandParty {
    int32_t         party_unit_kind;
    uint32_t        enabled;
    const char*     name;
    int16_t         icon;
    int16_t         unk_0e;
    const char*     help_message;
    int16_t         current_hp;
    int16_t         max_hp;
} ;

static_assert(sizeof(BattleWorkCommandParty) == 0x18);

struct BattleWorkCommandMultiItem {
    uint32_t        enabled;
    const char*     name;
    const char*     help_message;
    int16_t         icon;
    int16_t         unk_0e;
    int32_t         cost;
    uint32_t        unk_14;
} ;

static_assert(sizeof(BattleWorkCommandMultiItem) == 0x18);

struct BattleWorkCommandCursor {
    int32_t         abs_position;
    int32_t         rel_position;
    int32_t         num_options;
} ;

static_assert(sizeof(BattleWorkCommandCursor) == 0xc);
    
struct BattleWorkCommand {
    uint32_t        state;
    uint32_t        menu_type_active;
    BattleWorkCommandAction     act_class_table[6];
    BattleWorkCommandWeapon     weapon_table[21];
    BattleWorkCommandOperation  operation_table[7];
    BattleWorkCommandParty      party_table[8];
    BattleWorkCommandMultiItem  multi_item_table[3];
    BattleWorkCommandCursor     cursor_table[14];
    int8_t          unk_540[0x1c];
    void*           window_work;
    int8_t          unk_560[0x14];
} ;

static_assert(sizeof(BattleWorkCommand) == 0x574);

struct BattleWorkActionCommandManager {
    battle_unit::BattleWorkUnit*    ac_unit;
    uint32_t    unk_004;
    uint32_t    unk_008;
    uint32_t    ac_state;
    void*       ac_main_function;
    void*       ac_result_function;
    void*       ac_disp_function;
    void*       ac_delete_function;
    int32_t     ac_defense_result;
    int32_t     ac_result_count;
    uint32_t    ac_result;
    int8_t      base_ac_difficulty;  // ?
    int8_t      ac_difficulty;
    int8_t      pad_02e[2];
    uint32_t    unk_030;
    uint32_t    ac_flag;
    uint32_t    ac_params[8];
    uint32_t    ac_output_params[8];  // ?
    uint32_t    ac_gauge_params[4];
    int16_t     unk_088;
    int8_t      pad_08a[2];
    int8_t      ac_manager_pad_work[0x1fc];
    int8_t      pad_288[8];
    int8_t      ac_disp_params[0x2c];
    int8_t      extra_work[2000];
    uint32_t    stylish_current_frame;
    uint32_t    stylish_window_start;
    uint32_t    stylish_window_end;
    uint32_t    stylish_end_frame;
    uint32_t    stylish_unit_idx;
    int32_t     stylish_result;
    uint32_t    stylish_early_frames;
} ;

static_assert(sizeof(BattleWorkActionCommandManager) == 0xaa8);

struct BattleWorkActRecord {
    uint8_t mario_times_jump_moves_used;
    uint8_t mario_times_hammer_moves_used;
    uint8_t mario_times_attacking_special_moves_used;
    uint8_t mario_times_non_attacking_special_moves_used;
    uint8_t mario_damage_taken;
    uint8_t partner_damage_taken;
    uint8_t mario_damaging_hits_taken;
    uint8_t partner_damaging_hits_taken;
    uint8_t max_power_bounce_combo;
    uint8_t mario_num_times_attack_items_used;
    uint8_t mario_num_times_non_attack_items_used;
    uint8_t partner_num_times_attack_items_used;
    uint8_t partner_num_times_non_attack_items_used;
    uint8_t mario_times_changed_partner;
    uint8_t partner_times_changed_partner;
    uint8_t mario_times_attacked_audience;
    uint8_t partner_times_attacked_audience;
    uint8_t mario_times_appealed;
    uint8_t partner_times_appealed;
    uint8_t mario_fp_spent;
    uint8_t mario_times_move_used;
    uint8_t partner_fp_spent;
    uint8_t partner_times_move_used;
    uint8_t mario_times_charge_used;
    uint8_t partner_times_charge_used;
    uint8_t mario_times_super_charge_used;
    uint8_t partner_times_super_charge_used;
    uint8_t mario_times_ran_away;
    uint8_t partner_times_ran_away;
    uint8_t partner_times_attacking_moves_used;
    uint8_t partner_times_non_attacking_moves_used;
    uint8_t turns_spent;
    uint8_t num_successful_ac;      // counts to 200 instead of 100
    uint8_t num_unsuccessful_ac;    // counts to 200 instead of 100
    uint8_t pad_22[2];
};

static_assert(sizeof(BattleWorkActRecord) == 0x24);

struct BattleWorkStageHazards {
    uint32_t    unk_000;
    gc::vec3    unk_004;  // rotation?
    gc::vec3    unk_010;  // temp. rotation?
    uint8_t     stage_effects_allowed;
    uint8_t     pad_01d[3];
    uint32_t    bg_a1_or_b_falling_evt_id;
    uint32_t    bg_a1_evt_id;
    uint32_t    bg_a2_evt_id;
    
    uint32_t    bg_b_scrolling_or_falling_evt_id;
    uint32_t    bg_b_rotating_evt_id;
    void*       nozzle_data;
    void*       fall_object_data;
    ttyd::battle_database_common::BattleWeapon* stage_jet_weapon_params[2];
    ttyd::battle_database_common::BattleWeapon  stage_jet_temp_weapon_params[2];
    int8_t      stage_jet_face_directions[3];
    int8_t      stage_jet_changing_face_directions[3];
    int8_t      current_stage_jet_type;
    int8_t      pad_1cb;
    uint32_t    stage_jet_change_event_id[3];
    uint32_t    stage_jet_fire_event_id;
    int32_t     fog_turn_count;
    int32_t     fog_active;
    uint32_t    wall_close_event_id;
    uint32_t    ceiling_fall_event_id;
    uint32_t    object_fall_event_id;
};

static_assert(sizeof(BattleWorkStageHazards) == 0x1f0);

struct BattleWork {
    int16_t         turn_count;
    int16_t         pad_00002;
    int32_t         battle_seq_0;
    BattleWorkAlliance alliance_information[3];
    battle_unit::BattleWorkUnit* battle_units[64];
    int32_t         move_priority_queue[64];
    int32_t         phase_evt_queue[64][2];
    int32_t         active_unit_idx;
    int32_t         unknown_unit_idx;  // BattleTransID -6
    BattleWorkWeaponTargets weapon_targets_work;
    uint32_t        battle_flags;
    uint32_t        unk_00ef8;  // flags
    uint32_t        unk_00efc;
    int32_t         stored_exp;
    int32_t         stored_exp_displayed;
    int32_t         stored_exp_displayed_inc_anim_timer;
    // BattleSeq sequence 1-7 values
    int32_t         init_seq;
    int32_t         first_act_seq;
    int32_t         turn_seq;
    int32_t         phase_seq;
    int32_t         move_seq;
    int32_t         act_seq;
    int32_t         end_seq;
    
    void*           battle_end_work;  // ptr to struct of length 0x2ac
    int8_t          pad_work[0x1fc * 4];
    BattleWorkCommand command_work;
    BattleWorkActionCommandManager ac_manager_work;
    npcdrv::FbatBattleInformation* fbat_info;
    int8_t          status_window_related[0x14];
    int8_t          unk_02750[4];
    int8_t          camera_work[0x104];
    int8_t          audience_work[0x13914];
    int8_t          bingo_work[0x134];
    int8_t          party_info_work[0x2c * 7];
    
    uint32_t        tattled_unit_type_flags[8];
    uint32_t        badge_equipped_flags;
    int8_t          unk_163f8[4];
    
    int8_t          stage_work[0xb3c];
    BattleWorkActRecord act_record_work;
    int8_t          after_reaction_queue[0x8 * 64];
    int8_t          stage_object_work[0x7c * 32];
    BattleWorkStageHazards stage_hazard_work;
    int8_t          icon_work[0x9c * 16];
    int8_t          unk_18c8c[0x114];
    int8_t          status_change_msg_work[0x258];
    
    int8_t          unk_18ff8;
    int8_t          impending_merlee_spell_type;
    uint16_t        unk_18ffa;  // frame counter for something in btlseqFirstAct
    battle_database_common::BattleWeapon* impending_bonus_weapon;
    float           impending_sp_ac_success_multiplier;
    int8_t          impending_sp_stylish_multiplier;
    int8_t          unk_19005;
    int8_t          impending_sp_bingo_card_chance;
    int8_t          unk_19007;
    const char*     weapon_ac_help_msg;
    uint32_t        battle_ac_help_disp_type;
    
    int8_t          unk_19010[0x4c];
    int32_t         lucky_start_evt_tid;
    int32_t         reserve_items[4];
    int32_t         curtain_sfx_entry_idx;
    bool            last_ac_successful;
    int8_t          pad_19075[3];

    uint32_t        debug_event_trigger_flags;
    int8_t          unk_1907c[8];
    int32_t         debug_audience_count;
    int32_t         debug_audience_monotype_kind;
    int32_t         debug_force_bingo_slot_type;
    int8_t          unk_19090[8];
} ;

static_assert(sizeof(BattleWork) == 0x19098);

extern "C" {

// .text
// BattleConsumeReserveItem
// BattleStatusWindowCheck
// BattleStatusWindowSystemOff
// BattleStatusWindowEventOff
// BattleStatusWindowSystemOn
// BattleStatusWindowEventOn
// BattleStatusWindowAPRecoveryOff
// BattleStatusWindowAPRecoveryOn
// BattleMajinaiEndCheck
// BattleMajinaiDone
// BattleMajinaiCheck
// battleDisableHResetCheck
// BattleAfterReactionMain
// BattleAfterReactionRelease
// BattleAfterReactionEntry
// BattleAfterReactionQueueInit
// BattleCheckUnitBroken
// BattleGetFloorHeight
// BattleGetStockExp
void BattleStoreExp(BattleWork* battleWork, int32_t added_exp);
// BattleStoreCoin
// BattlePartyInfoWorkInit
void _EquipItem(
    battle_unit::BattleWorkUnit* unit, uint32_t unk0, int32_t item);
void BtlUnit_EquipItem(
    battle_unit::BattleWorkUnit* unit, uint32_t unk0, int32_t item);
// BattleTransPartyIdToUnitKind
// BattleTransPartyId
// BattleChangeParty
// BattlePartyAnimeLoad
// BattleGetPartnerPtr
battle_unit::BattleWorkUnit* BattleGetPartyPtr(BattleWork* battleWork);
battle_unit::BattleWorkUnit* BattleGetMarioPtr(BattleWork* battleWork);
// BattleGetSystemPtr
battle_unit::BattleWorkUnitPart* BattleGetUnitPartsPtr(
    int32_t unit_idx, int32_t part_idx);
// BattleSetUnitPtr
battle_unit::BattleWorkUnit* BattleGetUnitPtr(
    BattleWork* battleWork, int32_t idx);
// BattleFree
void* BattleAlloc(int32_t size);
// BattleIncSeq
int32_t BattleGetSeq(BattleWork* battleWork, int32_t level);
// BattleSetSeq
// BattleSetMarioParamToFieldBattle
// Btl_UnitSetup
// BattleEnd
// BattleMain
// BattleInit
// battleSeqEndCheck
// battleMain

 // .data
extern BattleWork* g_BattleWork; 

}

}