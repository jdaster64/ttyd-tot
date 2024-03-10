#pragma once

#include <gc/types.h>

#include <cstdint>

namespace ttyd::party {

struct PartyEntry {
    uint32_t    flags;
    uint32_t    disp_flags;
    uint32_t    flags_3;
    int32_t     paper_anim_group_id;
    int32_t     unk_0x010;
    int32_t     paper_pose_id;
    char*       anim_name;
    char*       paper_anim_name;
    int16_t     pose_time;
    int8_t      unk_0x022[2];
    int32_t     motion_timer;
    int16_t     force_move_timer;
    int16_t     invincible_timer;
    int8_t      unk_0x02c;
    int8_t      unk_0x02d;
    uint8_t     dokan_val;
    int8_t      current_slot_id;
    int8_t      controller_num;
    int8_t      current_member_id;
    bool        is_flying;
    int8_t      fbat_hit_check_member_id;
    int8_t      run_mode_id;
    int8_t      prev_run_mode_id;
    int8_t      walk_motion_id;
    int8_t      paper_state_id;
    int8_t      sub_motion_id;
    int8_t      use_motion_id;
    int8_t      slit_motion_id;
    int8_t      move_mode;
    int8_t      unk_0x03c[0x10];
    int8_t      hazard_timer;
    int8_t      unk_0x04d[0xb];
    gc::vec3    position;
    gc::vec3    unk_0x064;
    gc::vec3    scale;
    gc::vec3    model_position;
    gc::vec3    unk_0x088;
    gc::vec3    dest_position;
    float       unk_0x0a0;
    int8_t      unk_0x0a4[4];
    float       unk_0x0a8;
    gc::vec3    position_correction;
    gc::vec3    shadow_position;
    gc::vec3    unk_0x0c4;
    int8_t      unk_0x0d0[0x10];
    float       unk_0x0e0;
    float       ground_height;
    float       unk_0x0e8;
    float       unk_0x0ec;
    float       height;
    float       width;
    float       view_direction_forward_world;
    float       direction_world;
    float       direction_view;
    float       force_move_velocity;
    int8_t      unk_0x108[4];
    float       current_disp_dir;
    float       target_disp_dir;
    float       velocity_y;
    float       unk_0x118;
    float       vertical_movement_param;
    float       step_height;
    int8_t      unk_0x124[4];
    float       jump_distance;
    void*       hitobj_wall;
    void*       hitobj_activate;
    void*       hitobj_wall_move;
    void*       hitobj_stand_on;
    void*       unk_0x13c;
    void*       hitobj_head;
    int8_t      unk_0x144[0xc];
    uint16_t    pause_button_buffer;
    uint16_t    buttons_pressed;
    uint16_t    buttons_held;
    uint16_t    buttons_released;
    uint8_t     stick_dir_x;
    uint8_t     stick_dir_y;
    uint8_t     substick_dir_x;
    uint8_t     substick_dir_y;
    uint8_t     left_trigger;
    uint8_t     right_trigger;
    uint8_t     unused_input_1;
    uint8_t     unused_input_2;
    void*       player;
    int32_t     camera_id;
    int32_t     yoshi_psnd_sfx_id;
    int32_t     unk_0x16c;
    void*       user_data;
    uint32_t    misc_work[5];
} __attribute__((__packed__));

static_assert(sizeof(PartyEntry) == 0x188);

extern "C" {

// .text
// partyMain
// unk_800bc660
// N_partyForceChgRunMode
// partyChgRunMode
// N__partyChgRunMode
// partyUsePost
// partyReInit
// partyInit
// partyEntry2Pos
// partyEntry2Hello
// partyEntry2
// partyEntryMain
// partyGetHeight
// yoshiSetColor
// partyKill2
// partyKill
// partyGoodbye
// partyEntryPos
// partyEntry
// partyEntryHello
// partySetForceMove
// partyStop
// partyRun
// partyCtrlOn
// partyCtrlOff
// partyChgPaper
void partyChgPose(PartyEntry* party, const char* pose_name);
// partyChgPoseId
// partyPaperLightOff
// partyPaperOff
// partyPaperOn
// anotherPartyGetPtr
PartyEntry* partyGetPtr(int32_t slot_id);

// .data
extern int32_t partyCtrlNo;

}

}