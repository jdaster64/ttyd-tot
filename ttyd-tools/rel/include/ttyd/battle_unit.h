#pragma once

#include "battle_database_common.h"

#include <gc/types.h>

#include <cstdint>

namespace ttyd::battle_unit {

namespace BattleUnitAttribute_Flags {
    enum e {
        UNK_1                       = 0x1U,
        OUT_OF_REACH                = 0x2U,     // usually on the ceiling
        UNQUAKEABLE                 = 0x4U,     // aerial or hovering
        UNK_8                       = 0x8U,     // invisible-like state?
        VEILED                      = 0x10U,
        SHELL_SHIELDED              = 0x20U,
        UNTARGETABLE_40             = 0x40U,
        UNK_100                     = 0x100U,
        DISABLE_ZERO_G_FLOATING     = 0x1000U,
        DISABLE_ZERO_G_IMMOBILITY   = 0x2000U,
        PREVENT_SPIN_KNOCKBACK      = 0x4000U,
        UNDEAD                      = 0x10000U, // always set Red/Dry/Dark Bones
        INACTIVE                    = 0x20000U,
        UNK_40000                   = 0x40000U,
        UNK_80000                   = 0x80000U,
        UNK_20_0000                 = 0x20'0000U,
        UNK_40_0000                 = 0x40'0000U,
        UNK_100_0000                = 0x100'0000U,
        UNK_200_0000                = 0x200'0000U,
        UNK_400_0000                = 0x400'0000U,
        UNK_1000_0000               = 0x1000'0000U,
        UNK_2000_0000               = 0x2000'0000U,
        UNK_4000_0000               = 0x4000'0000U,  // prevents damage?
    };
}

namespace PartsAttribute_Flags {
    enum e {
        // Different tiers of preference for moves that target one actor part:
        // - Freely-selectable attacks like jump can target anything regardless
        //   of whether any of 0x1, 0x2, 0x4 are set
        // - Forced-selection attacks like Shade Fist or Shell Toss will target
        //   parts w/ 0x1 set if reachable, then 0x2 parts otherwise
        // - Bomb Squad targets parts with at least one of 0x1, 0x2, 0x4 set
        MAIN_TARGET                 = 0x1U,
        PREFERRED_TARGET            = 0x2U,
        SELECT_TARGET               = 0x4U,
        
        UNK_8                       = 0x8U,
        UNK_10                      = 0x10U,
        UNK_40                      = 0x40U,
        WEAK_TO_ATTACK_FX_R         = 0x80U,
        WEAK_TO_ICE_POWER           = 0x100U,
        WINGED                      = 0x800U,
        SHELLED                     = 0x1000U,
        BOMB_FLIPPABLE              = 0x2000U,
        UNK_MISS_4000               = 0x4000U,
        UNK_8000                    = 0x8000U,
        NEVER_TARGETABLE            = 0x10000U,
        UNK_20000                   = 0x20000U,
        UNK_40000                   = 0x40000U,
        UNTATTLEABLE                = 0x80000U,
        JUMPLIKE_CANNOT_TARGET      = 0x10'0000U,
        HAMMERLIKE_CANNOT_TARGET    = 0x20'0000U,
        SHELLTOSSLIKE_CANNOT_TARGET = 0x40'0000U,
        DISABLE_HP_REDUCTION        = 0x80'0000U,  // damage still visible
        UNK_100_0000                = 0x100'0000U,
        UNK_200_0000                = 0x200'0000U,
        UNK_400_0000                = 0x400'0000U,
        UNK_800_0000                = 0x800'0000U,
        UNK_1000_0000               = 0x1000'0000U,
        UNK_2000_0000               = 0x2000'0000U,  // immune to damage/status?
        UNK_4000_0000               = 0x4000'0000U,  // immune to ohko?
        UNK_8000_0000               = 0x8000'0000U,  // immune to status?
    };
}

namespace PartsCounterAttribute_Flags {
    enum e {
        TOP_SPIKY               = 0x1U,
        PREEMPTIVE_SPIKY        = 0x2U,
        FRONT_SPIKY             = 0x4U,
        FIERY                   = 0x10U,
        FIERY_STATUS            = 0x20U,
        ICY                     = 0x40U,
        ICY_STATUS              = 0x80U,
        POISON                  = 0x100U,
        POISON_STATUS           = 0x200U,
        ELECTRIC                = 0x400U,
        ELECTRIC_STATUS         = 0x800U,
        EXPLOSIVE               = 0x1000U,
        VOLATILE_EXPLOSIVE      = 0x2000U,    
    };
}

namespace BattleUnitToken_Flags {
    enum e {
        UNK_1                   = 0x1U,
        UNK_2                   = 0x2U,
        UNK_4                   = 0x4U,
        CHARGE_EXPENDED         = 0x8U,
        CONFUSE_PROC            = 0x10U,
        ACTIVE_MOVE_STATE       = 0x1000U,
        HUSTLE_MOVE_STATE       = 0x2000U,  // Rally Wink
        UNK_8000_0000           = 0x8000'0000U,
    };
}

namespace BattleUnitStatus_Flags {
    enum e {
        CAN_ACT                 = 0x1U,
        DEFENDING               = 0x100'0000U,  // 'Defend' state
        HAS_MOVES_REMAINING     = 0x200'0000U,
        IN_DANGER               = 0x1000'0000U,
        IN_PERIL                = 0x2000'0000U,
        
        // Custom statuses for ToT.
        SCOPED                  = 0x400U,
        SCOPED_PLUS             = 0x800U,
        MIDBOSS                 = 0x1000U,
    };
}

struct MovementSoundControl {
    const char*     sound_name_left;
    const char*     sound_name_right;
    int16_t         unk_08;
    int16_t         unk_0a;
    int16_t         initial_wait_timer;
    int16_t         repeat_wait_timer_left;
    int16_t         repeat_wait_timer_right;
    int16_t         current_timer;
    int16_t         sound_to_play_next;
    int16_t         pad_16;
    int32_t         psnd_sfx_idx;
} ;

static_assert(sizeof(MovementSoundControl) == 0x1c);
    
struct MovementSoundData {
    const char*     jump_sound_left;
    const char*     jump_sound_right;
    const char*     run_sound_left;
    const char*     run_sound_right;
    int16_t         run_initial_wait_timer;
    int16_t         run_repeat_wait_timer_left;
    int16_t         run_repeat_wait_timer_right;
    int16_t         pad_16;
    const char*     walk_sound_left;
    const char*     walk_sound_right;
    int16_t         walk_initial_wait_timer;
    int16_t         walk_repeat_wait_timer_left;
    int16_t         walk_repeat_wait_timer_right;
    int16_t         pad_26;
    const char*     dive_sound_left;
    const char*     dive_sound_right;
    int16_t         dive_initial_wait_timer;
    int16_t         dive_repeat_wait_timer_left;
    int16_t         dive_repeat_wait_timer_right;
    int16_t         pad_36;
} ;

static_assert(sizeof(MovementSoundData) == 0x38);

// See jda_ttyd_utils Github (BattleWorkUnit.0x148 onward) for more details.
struct MovementParams {
    gc::vec3        move_start_position;
    gc::vec3        move_current_position;
    gc::vec3        move_target_position;
    int32_t         move_frame_count;
    float           move_speed_xz;
    float           fall_accel;
    float           move_speed_y;
    float           move_angle_xz;
    float           move_dist_xz;
    float           jump_offset_height;
    int8_t          move_direction;
    int8_t          face_direction;
    int8_t          pad_42[2];
    MovementSoundControl sound_control_work;
    // Jump parameters used in btlevtcmd_MarioJumpPosition.
    gc::vec3        jump_move_per_frame;
    float           jump_angle_perturbance_angle;
    float           jump_angle_perturbance_angle_delta;
    float           jump_current_position_y;  // ?
    int8_t          unk_78[4];
    MovementSoundData sound_data_table;
} ;

static_assert(sizeof(MovementParams) == 0xb4);

struct BadgesEquipped {
    int8_t      close_call;
    int8_t      pretty_lucky;
    int8_t      lucky_day;
    int8_t      unk_03;     // Probably unused
    int8_t      power_plus;
    int8_t      p_up_d_down;
    int8_t      all_or_nothing;
    int8_t      mega_rush;
    
    int8_t      power_rush;
    int8_t      p_down_d_up;
    int8_t      double_pain;
    int8_t      last_stand;
    int8_t      defend_plus;
    int8_t      damage_dodge;
    int8_t      happy_heart;
    int8_t      happy_flower;
    
    int8_t      return_postage;
    int8_t      hp_plus;
    int8_t      fp_plus;
    int8_t      double_dip;
    int8_t      triple_dip;
    int8_t      flower_saver;
    int8_t      feeling_fine;
    int8_t      zap_tap;
    
    int8_t      pity_flower;
    int8_t      hp_drain;
    int8_t      fp_drain;
    int8_t      refund;
    int8_t      charge;
    int8_t      super_charge;
    int8_t      unused_defend_command_badge;
    int8_t      jumpman;
    
    int8_t      hammerman;
    int8_t      ice_power;
    int8_t      spike_shield;
    int8_t      super_appeal;
    int8_t      lucky_start;
    int8_t      simplifier;
    int8_t      unsimplifier;
    int8_t      auto_command_badge;
} ;

static_assert(sizeof(BadgesEquipped) == 0x28);

struct BattleWorkUnit;
struct BattleWorkUnitPart;
    
struct BattleWorkUnitPart {
    BattleWorkUnitPart* next_part;
    battle_database_common::BattleUnitKindPart* kind_part_params;
    const char*         part_name;
    gc::vec3            home_position;
    gc::vec3            position;
    gc::vec3            position_offset;
    gc::vec3            display_offset;
    gc::vec3            base_rotation;
    gc::vec3            rotation;
    gc::vec3            rotation_offset;
    gc::vec3            scale;
    gc::vec3            base_scale;
    float               unk_078;  // some sort of scale
    
    MovementParams      movement_params;
    uint32_t            parts_work[16];  // length unknown; at least 3
    gc::vec3            hit_base_position;  // ?
    gc::vec3            hit_offset;
    gc::vec3            hit_cursor_base_position;  // ?
    gc::vec3            hit_cursor_offset;
    int16_t             addl_target_offset_x;
    int16_t             unk_1a2;
    int16_t             unk_1a4;
    int8_t              unk_1a6[6];
    
    uint32_t            part_attribute_flags;
    uint32_t            counter_attribute_flags;
    int8_t*             defense;
    int8_t*             defense_attr;
    
    void*               pose_table;
    int32_t             anim_pose_type;
    char                anim_pose_name[64];  // length unknown
    uint32_t            unk_204;  // anim-related flags
    float               unk_208;
    float               anim_motion_speed;
    void*               unk_210;  // some sort of anim-related callback fn
    int8_t              unk_214;
    int8_t              unk_215;  // flags related to anim
    int8_t              pad_216[2];
    gc::color4          base_blur_color;
    uint32_t            blur_flags;
    gc::color4          blur_colors[2];
    int8_t              blur_params[0x44 * 10];
    int8_t              pose_sound_params[0x18];
    
    int32_t             unk_4e8;
    BattleWorkUnit*     unit_owner;
    gc::color4          color;
    gc::color4          blended_color;  // with base alpha, invis status, etc.
    float               unk_4f8;  // some sort of z-offset?  Used in btlDispMain
    int8_t              unk_4fc;  // axis order to apply rotations?
    int8_t              unk_4fd[3];
} ;

static_assert(sizeof(BattleWorkUnitPart) == 0x500);

struct BattleWorkUnit {
    int32_t         unit_id;
    int32_t         true_kind;      // BattleUnitType::e
    int32_t         current_kind;   // BattleUnitType::e
    int8_t          alliance;
    int8_t          level;
    int8_t          group_index;
    int8_t          pad_00f;
    battle_database_common::BattleUnitKind* unit_kind_params;
    BattleWorkUnitPart* parts;
    void*           data_table;
    uint32_t        unit_flags;
    int8_t          move_state;  // ?
    int8_t          max_move_count;
    int8_t          moves_remaining;
    int8_t          max_moves_this_turn;
    int8_t          active_turns;  // ? Used to make Slow status skip turns
    int8_t          swallow_chance;
    int8_t          swallow_attributes;
    int8_t          pad_027;
    int32_t         move_priority;
    int32_t         attack_phase;
    
    gc::vec3        home_position;
    gc::vec3        position;
    gc::vec3        offset_position;
    gc::vec3        display_offset;
    gc::vec3        base_rotation;
    gc::vec3        rotation;
    gc::vec3        rotation_offset;
    gc::vec3        center_offset;
    gc::vec3        scale;
    gc::vec3        base_scale;
    gc::vec3        toge_offset;  // Used for speech bubble spike?
    gc::vec3        held_item_base_offset;
    gc::vec3        possession_item_offset;
    int16_t         width;
    int16_t         height;
    int16_t         status_icon_offset[2];
    int16_t         hp_gauge_offset[2];
    gc::vec3        cut_base_offset;
    float           cut_width;
    float           cut_height;
    gc::vec3        binta_hit_offset;
    gc::vec3        kiss_hit_offset;
    
    uint32_t        attribute_flags;
    int16_t         max_hp;
    int16_t         base_max_hp;  // no badges
    int16_t         current_hp;
    int16_t         max_fp;
    int16_t         base_max_fp;  // no badges
    int16_t         current_fp;
    float           unk_scale;
    
    int8_t          sleep_turns;
    int8_t          unk_119;  // something to do with waking up from sleep?
    int8_t          stop_turns;
    int8_t          dizzy_turns;
    int8_t          poison_turns;
    int8_t          poison_strength;
    int8_t          confusion_turns;
    int8_t          electric_turns;
    
    int8_t          dodgy_turns;
    int8_t          burn_turns;
    int8_t          freeze_turns;
    int8_t          size_change_turns;
    int8_t          size_change_strength;
    int8_t          attack_change_turns;
    int8_t          attack_change_strength;
    int8_t          defense_change_turns;
    
    int8_t          defense_change_strength;
    int8_t          charge_strength;
    int8_t          allergic_turns;
    int8_t          flipped_turns;      // for shelled enemies, etc.
    int8_t          invisible_turns;
    int8_t          payback_turns;
    int8_t          hold_fast_turns;
    int8_t          fast_turns;
    
    int8_t          slow_turns;
    int8_t          hp_regen_turns;
    int8_t          hp_regen_strength;
    int8_t          fp_regen_turns;
    int8_t          fp_regen_strength;
    int8_t          knocked_out;        // OHKO strength?
    int8_t          unk_136;
    int8_t          unk_137;
    
    uint32_t        status_flags;
    int8_t          unk_13c[4];
    uint32_t        protect_unit_idx;   // related to Shell Shield?
    battle_database_common::StatusVulnerability* status_vulnerability;
    MovementParams  movement_params;
    int8_t          hp_gauge_params[0x1c];
    uint32_t        unit_work[16];
    
    // Parameters used during a single "act" (attack).
    BattleWorkUnitPart* currently_targeted_part;
    uint32_t        unk_25c;  // unreferenced?
    int16_t         hp_damaging_hits_dealt;  // used for Power Bounce decay, etc.
    int16_t         pad_262;
    int32_t         total_hp_damage_taken;
    int32_t         total_fp_damage_taken;
    int32_t         total_fp_lost;
    int8_t          hp_damage_taken;
    int8_t          fp_damage_taken;
    int8_t          fp_lost;
    int8_t          hits_taken;
    uint32_t        damage_pattern;
    uint32_t        damage_code;
    uint32_t        token_flags;
    int32_t         hits_dealt_this_attack;  // Used for Fire Drive decay, etc.
    int32_t         total_damage_dealt_this_attack;  // Used for Drain badges
    
    int32_t         init_evt_tid;
    void*           wait_evt_code;
    int32_t         wait_evt_tid;
    void*           unison_phase_evt_code;
    void*           phase_evt_code;
    int32_t         phase_evt_tid;
    void*           attack_evt_code;
    void*           confuse_evt_code;
    int32_t         attack_evt_tid;
    uint32_t        battle_menu_state;
    void*           damage_evt_code;
    int32_t         damage_evt_tid;
    void*           entry_evt_code;
    int32_t         entry_evt_tid;
    void*           ceiling_fall_evt_code;
    int32_t         ceiling_fall_evt_tid;
    void*           unknown_evt_code;
    int32_t         unknown_evt_tid;
    void*           hit_evt_code;  // ? BattleRunHitEventDirect
    uint32_t        talk_body_part_id;
    const char*     talk_pose_name;
    const char*     stay_pose_name;
    
    BadgesEquipped  badges_equipped;
    int32_t         held_item;
    battle_database_common::ItemDropData* held_item_table;
    
    int8_t          misc_310[0x824];
} ;

static_assert(sizeof(BattleWorkUnit) == 0xb34);

extern "C" {

// BtlUnit_CheckShadowGuard
// BtlUnit_EnemyItemCanUseCheck
// BtlUnit_HpGaugeMain
// BtlUnit_HpGaugeInit
int32_t BtlUnit_snd_se_pos(
    BattleWorkUnit* unit, const char* sfx, int32_t vol, int32_t pit, gc::vec3* pos);
// BtlUnit_snd_se
// BtlUnit_ControlPoseSoundMain
// BtlUnit_PoseSoundInit
// BtlUnit_SetCommandAnimPose
// BtlUnit_SetSeMode
// BtlUnit_LoadSeMode
// BtlUnit_ResetMoveStatus
// BtlUnit_GetGuardKouraPtr
void BtlUnit_PayWeaponCost(
    BattleWorkUnit* unit, battle_database_common::BattleWeapon* weapon);
// BtlUnit_CheckWeaponCost
int32_t BtlUnit_GetWeaponCost(
    BattleWorkUnit* unit, battle_database_common::BattleWeapon* weapon);
// BtlUnit_SetMaxFp
// BtlUnit_GetMaxFp
// BtlUnit_RecoverFp
// BtlUnit_RecoverHp
void BtlUnit_SetFp(BattleWorkUnit* unit, int32_t fp);
int32_t BtlUnit_GetFp(BattleWorkUnit* unit);
int32_t BtlUnit_GetCoin(BattleWorkUnit* unit);
// BtlUnit_GetExp
// BtlUnit_CheckPinchStatus
// BtlUnit_SetParamToPouch
// BtlUnit_ReviseHpFp
// BtlUnit_SetParamFromPouch
// BtlUnit_CanActStatus
// BtlUnit_CanGuardStatus
// BtlUnit_CheckData
uint32_t BtlUnit_GetData(BattleWorkUnit* unit, int32_t data_type);
// BtlUnit_GetACPossibility
// BtlUnit_SetTotalHitDamage
// BtlUnit_GetTotalHitDamage
// BtlUnit_GetHitDamage
// BtlUnit_GetEnemyBelong
// BtlUnit_GetTalkTogePos
// BtlUnit_ChangeStayAnim
// BtlUnit_ChangeTalkAnim
// BtlUnit_SetBodyAnim
// BtlUnit_SetBodyAnimType
// BtlUnit_SetAnim
// BtlUnit_SetAnimType
// BtlUnit_GetPoseNameFromType
void BtlUnit_OffUnitFlag(BattleWorkUnit* unit, uint32_t flag);
void BtlUnit_OnUnitFlag(BattleWorkUnit* unit, uint32_t flag);
uint32_t BtlUnit_CheckUnitFlag(BattleWorkUnit* unit, uint32_t flag);
// BtlUnit_OffStatusFlag
void BtlUnit_OnStatusFlag(BattleWorkUnit* unit, uint32_t flag);
// BtlUnit_CheckStatusFlag
uint32_t BtlUnit_CheckStatus(BattleWorkUnit* unit, int8_t status_type);
uint32_t BtlUnit_CheckRecoveryStatus(BattleWorkUnit* unit, int8_t status_type);
// BtlUnit_ClearStatus
uint32_t BtlUnit_SetStatus(
    BattleWorkUnit* unit, int8_t status_type, int8_t turns, int8_t strength);
void BtlUnit_GetStatus(
    BattleWorkUnit* unit, int8_t status_type, int8_t* turns, int8_t* strength);
// _CheckMoveCount
// BtlUnit_GetBelong
// BtlUnit_SetJumpSpeed
// BtlUnit_SetPartsMoveSpeed
// BtlUnit_SetMoveSpeed
// BtlUnit_SetPartsFallAccel
// BtlUnit_SetFallAccel
// BtlUnit_SetPartsMoveTargetPos
// BtlUnit_SetMoveTargetPos
// BtlUnit_SetPartsMoveCurrentPos
// BtlUnit_SetMoveCurrentPos
// BtlUnit_SetPartsMoveStartPos
// BtlUnit_SetMoveStartPos
// BtlUnit_AddPartsDispOffset
// BtlUnit_SetPartsDispOffset
// BtlUnit_SetDispOffset
// BtlUnit_AddPartsOffsetPos
// BtlUnit_SetPartsOffsetPos
// BtlUnit_GetPartsOffsetPos
// BtlUnit_SetOffsetPos
// BtlUnit_SetHeight
int32_t BtlUnit_GetHeight(BattleWorkUnit* unit);
// BtlUnit_GetWidth
// BtlUnit_AddPartsScale
// BtlUnit_SetPartsScale
// BtlUnit_SetPartsBaseScale
// BtlUnit_AddScale
void BtlUnit_SetScale(BattleWorkUnit* unit, float x, float y, float z);
void BtlUnit_GetScale(BattleWorkUnit* unit, float* x, float* y, float* z);
// BtlUnit_SetBaseScale
// BtlUnit_AddPartsRotateOffset
// BtlUnit_SetPartsRotateOffset
// BtlUnit_SetRotateOffset
// BtlUnit_GetPartsBaseRotate
// BtlUnit_SetPartsBaseRotate
// BtlUnit_GetBaseRotate
// BtlUnit_SetBaseRotate
// BtlUnit_AddPartsRotate
// BtlUnit_GetPartsRotate
// BtlUnit_SetPartsRotate
// BtlUnit_AddRotate
// BtlUnit_GetRotate
// BtlUnit_SetRotate
// BtlUnit_SetPartsHomePos
// BtlUnit_AddHomePos
// BtlUnit_SetHomePos
// BtlUnit_GetHomePos
// BtlUnit_SetHitCursorOffset
// BtlUnit_SetHitOffset
void BtlUnit_GetHitPos(
    BattleWorkUnit* unit, BattleWorkUnitPart* part, float* x, float* y, float* z);
// BtlUnit_GetPartsWorldPos
// BtlUnit_AddPartsPos
// BtlUnit_SetPartsPos
// BtlUnit_GetPartsPos
// BtlUnit_AddPos
// BtlUnit_SetPos
void BtlUnit_GetPos(BattleWorkUnit* unit, float* x, float* y, float* z);
int32_t BtlUnit_GetBodyPartsId(BattleWorkUnit* unit);
BattleWorkUnitPart* BtlUnit_GetPartsPtr(BattleWorkUnit* unit, int part_idx);
// BtlUnit_GetUnitId
// BtlUnit_Spawn
// BtlUnit_Delete
BattleWorkUnit* BtlUnit_Entry(
    battle_database_common::BattleUnitSetup* unit_setup);
// BtlUnit_Init

}

}