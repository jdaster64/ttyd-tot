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
    int32_t         current_cursor_type;
    int32_t         unk_544;
    int32_t         current_operation_type;  // When in "Tactics" menu
    int8_t          unk_54c[0x10];
    void*           window_work;
    int8_t          unk_560[0x4];
    int32_t         selection_error_msg;
    int8_t          unk_568[0xc];
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

struct SpBonusInfo {
    battle_database_common::BattleWeapon* weapon;
    float       ac_success_multiplier;
    int8_t      stylish_multiplier;
    int8_t      unk_from_weapon_0x19;
    int8_t      bingo_slot_chance;
    int8_t      unk_from_weapon_0x1b;
};
static_assert(sizeof(SpBonusInfo) == 0xc);

struct BattleWorkAudienceMember {
    uint32_t    flags;
    int32_t     unk_0x004;  // substate?
    int32_t     unk_0x008;  // substate / timer?
    float       unk_0x00c;
    float       unk_0x010;
    float       unk_0x014;
    uint8_t     unk_0x018;
    int8_t      status;     // BattleAudienceCtrlProcessKinopio subroutine type
    uint8_t     tpl_idx;    // ?    
    int8_t      member_kind;
    int16_t     item_idx;   // index into BattleWorkAudienceItem array
    int16_t     pad_0x1e;
    
    void*       anim_data;
    void*       unk_anim_related;
    void*       unk_current_anim_command;
    uint32_t    unk_anim_command_timer;
    uint32_t    unk_0x030;
    uint32_t    unk_0x034[3];
    uint8_t     unk_0x040;  // index into 0x034, anim-command related
    uint8_t     unk_0x041;
    uint8_t     unk_0x042;
    uint8_t     unk_0x043;
    uint8_t     unk_0x044;
    uint8_t     pad_0x045[3];
    
    gc::vec3    position;
    gc::vec3    final_scale;
    gc::vec3    scale;
    gc::vec3    skew_stretch;   // ?
    gc::vec3    velocity;
    gc::vec3    acceleration;
    gc::vec3    facing_dir;     // ?
    gc::vec3    rotation;
    gc::vec3    rotation_offset;
    gc::vec3    home_position;
    gc::vec3    disp_position;
    gc::vec3    relative_position;
    gc::vec3    relative_velocity;
    float       relative_accel_y;
    float       unk_dir;
    battle_unit::BattleWorkUnit* target_unit;
    int32_t     audience_eat_idx;
    int32_t     sleep_turns;
    
    int32_t     evt_move_frames_remaining;
    int32_t     evt_move_frames_total;
    float       evt_move_start_pos_x;
    float       evt_move_target_pos_x;
    float       evt_move_start_pos_y;
    float       evt_move_target_pos_y;
    float       evt_move_start_pos_z;
    float       evt_move_target_pos_z;
    gc::vec3    evt_move_delta_pos;
    float       evt_move_gravity_y;
    uint32_t    evt_move_interpolation_type;
    
    // Various purposes based on member's type.
    uint32_t    audience_work;
    float       boo_float_displacement;
};

static_assert(sizeof(BattleWorkAudienceMember) == 0x134);

struct BattleWorkAudienceItem {
    uint32_t    flags;
    int32_t     state;
    int32_t     state_timer;
    uint32_t    audience_owner_idx;
    int32_t     item_type;
    gc::vec3    position;
    float       uniform_scale;
    float       rotation_deg_z;
    gc::vec3    velocity;
    gc::vec3    acceleration;
    battle_unit::BattleWorkUnit* target;
    uint32_t    unk_interactable;
};

static_assert(sizeof(BattleWorkAudienceItem) == 0x48);

struct BattleWorkAudienceApSrc {
    uint32_t    flags;
    int32_t     state;
    gc::vec3    position;
    gc::vec3    unk_0x14;   // unused?
    gc::vec3    move_start_pos;
    gc::vec3    move_target_pos;
    gc::vec3    scale;
    int32_t     move_timer;
    int32_t     move_total_time;
    float       dist_current_to_status_bar;
    float       dist_start_to_status_bar;
    float       angle_to_status_bar;
    float       delta_angle_to_status_bar;
    uint8_t     alpha;
    uint8_t     pad_0x5d[3];
};

static_assert(sizeof(BattleWorkAudienceApSrc) == 0x60);

struct BattleWorkAudienceSound {
    uint32_t    flags;
    uint32_t    psndsfx_idx;
    uint8_t     sound_level;
    uint8_t     pad_0x09[3];
    int32_t     sound_length;
    int32_t     fadeout_length;
    int32_t     force_fade_timer;
    int32_t     force_fade_length;
    uint8_t     base_volume;
    uint8_t     current_base_volume;
    uint8_t     volume_multiplier;
    uint8_t     fade_start_volume_multiplier;
    uint8_t     fade_end_volume_multiplier;
    uint8_t     pad_0x21[3];
};

static_assert(sizeof(BattleWorkAudienceSound) == 0x24);

struct BattleWorkAudienceWin {
    uint8_t     enable;
    uint8_t     unk_0x01[7];
    gc::vec3    position;
    int32_t     slide_in_timer;
    float       audience_count_disp;    // lags behind actual count
};

static_assert(sizeof(BattleWorkAudienceWin) == 0x1c);

struct BattleWorkAudience {
    uint32_t    flags;
    void*       current_evt_id;
    uint32_t    ap_src_sfx_idx;
    void*       normal_audience_tpl;
    void*       guest_audience_tpls[2];
    uint8_t     guest_audience_kinds[2];
    
    uint8_t     pad_0x0001a[0x1bc - 0x1a];
    
    BattleWorkAudienceMember members[200];
    BattleWorkAudienceItem items[100];
    BattleWorkAudienceApSrc sp_stars[100];
    BattleWorkAudienceSound sounds[24];
    BattleWorkAudienceWin window_work;
    
    // Read from battle_audience.o 0x802f9eac (US), indexed by Mario's level.
    float       base_target_audience;
    float       target_audience_value;
    float       bonus_audience_value;
    int32_t     current_audience_count_int;
    int32_t     current_audience_count_int_left;
    int32_t     current_audience_count_int_right;
    int32_t     max_audience_for_stage_rank;
    int32_t     new_audience_member_weights[12];
    int32_t     impending_star_power;               // In increments of 0.01 SP.
    SpBonusInfo* impending_bonus_info;
    int32_t     crowd_pleasing_events_streak;       // max of 2
    int32_t     crowd_displeasing_events_streak;    // max of 5
    int32_t     num_stylishes_performed;            // max of 5, per attack
    int32_t     unk_0x137d8;
    uint32_t    check_phase_reaction_state;
    uint32_t    check_phase_reaction_substate;
    
    int32_t     present_item_kind;
    int32_t     present_item_class;                 // 0 for good, 1 for bad
    int32_t     present_item_target_unit_idx;
    int32_t     items_spawned_this_turn;
    int32_t     max_items_this_turn;
    int32_t     items_thrown_this_burst;
    battle_database_common::BattleWeapon item_throw_weapon;
    int32_t     item_on_member_idx;
    gc::vec3    item_on_member_pos;
    
    uint32_t    possible_phase_event_types[14];
    int32_t     num_possible_phase_events;
    int32_t     turn_end_phase_event_trigger_chance;
    int32_t     audience_joy_level;
    int32_t     audience_excited;       // set to 1 during bingos?, levelups
};

static_assert(sizeof(BattleWorkAudience) == 0x13914);

struct BattleWorkBreakSlotReel {
    int32_t     index;
    int32_t     flags;
    int32_t     state;
    int32_t     unk_0x0c;  // substate / timer?
    int32_t     icon;
    int32_t*    possible_icons;
    int32_t     num_possible_icons;
    int32_t     possible_icons_idx;
    gc::vec3    icon_position;
    gc::vec3    icon_rotation;
    gc::vec3    icon_scale;
    gc::vec3    unk_0x44;  // some kind of translation offset
    float       unk_0x50;
    float       unk_0x54;
    uint8_t     unk_0x58;
    uint8_t     unk_0x59[3];
};

static_assert(sizeof(BattleWorkBreakSlotReel) == 0x5c);

struct BattleWorkBreakSlot {
    uint32_t    flags;
    int32_t     state;
    int32_t     unk_0x008;  // substate / timer?
    int32_t     slot;
    int32_t     active_bingo_turn_count;
    float       active_bingo_sp_multiplier;
    BattleWorkBreakSlotReel reels[3];
    int32_t     psndsfx_idx;
    void*       evt_entry;    
};

static_assert(sizeof(BattleWorkBreakSlot) == 0x134);

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
    BattleWorkAudience audience_work;
    BattleWorkBreakSlot bingo_work;
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

    int16_t         announce_msg_width;     // ?
    int16_t         announce_msg_height;    // ?
    int32_t         announce_msg_item_type;
    int8_t          unk_18c94[0xc];
    char            announce_msg_buf[0x100];

    int8_t          status_change_msg_work[0x258];
    int8_t          unk_18ff8;
    int8_t          impending_merlee_spell_type;
    uint16_t        unk_18ffa;  // frame counter for something in btlseqFirstAct

    // These fields comprise a SpBonusInfo struct:
    battle_database_common::BattleWeapon* impending_bonus_weapon;
    float           impending_sp_ac_success_multiplier;
    int8_t          impending_sp_stylish_multiplier;
    int8_t          unk_19005;
    int8_t          impending_sp_bingo_card_chance;
    int8_t          unk_19007;

    const char*     weapon_ac_help_msg;
    uint32_t        battle_ac_help_disp_type;
    int8_t          unk_19010[0x40];
    int32_t         mario_jump_disabled_turns;
    int32_t         mario_hammer_disabled_turns;
    int32_t         mario_items_disabled_turns;
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
void BattleFree(void* alloc);
void* BattleAlloc(int32_t size);
void BattleIncSeq(BattleWork* battleWork, int32_t level);
int32_t BattleGetSeq(BattleWork* battleWork, int32_t level);
void BattleSetSeq(BattleWork* battleWork, int32_t level, int32_t state);
// BattleSetMarioParamToFieldBattle
void Btl_UnitSetup(BattleWork* battleWork);
// BattleEnd
// BattleMain
// BattleInit
// battleSeqEndCheck
// battleMain

 // .data
extern BattleWork* g_BattleWork; 

}

}