#pragma once

#include <cstdint>

#include "evtmgr.h"

namespace ttyd::evt_door {

struct DoorSubmapInfo {
    uint32_t type;          // ?
    int32_t in_pos;         // ?
    const char* in_hitobj;
    int32_t out_pos;        // ?
    const char* out_hitobj;
    const char* anim_1;
    const char* anim_2;
    const char* anim_3;
    const char* anim_4;
    const char* outside_group;
    const char* door_group;
    const char* inside_group_s;
    const char* inside_group_a;
    const char* init_inside_group_s;
    const char* init_inside_group_a;
    const char** npc_list;
    const char** map_group_list;
    const char** mobj_list;
    const char** item_obj_list;
};
static_assert(sizeof(DoorSubmapInfo) == 0x4c);

extern "C" {

// .text
EVT_DECLARE_USER_FUNC(evt_door_end_wait, 0)
EVT_DECLARE_USER_FUNC(evt_door_set_param, 3)
EVT_DECLARE_USER_FUNC(evt_door_data_copy, 2)
EVT_DECLARE_USER_FUNC(npc_hide_onoff, 2)
EVT_DECLARE_USER_FUNC(evt_door_load_mapflag, 1)
EVT_DECLARE_USER_FUNC(evt_door_save_mapflag, 1)
EVT_DECLARE_USER_FUNC(evt_door_param, 4)
EVT_DECLARE_USER_FUNC(door_dark_flag, 2)
// door_dark_flag_sub
// door_position_sub
EVT_DECLARE_USER_FUNC(get_noclip_map, 3)
EVT_DECLARE_USER_FUNC(npc_light, 1)
EVT_DECLARE_USER_FUNC(npc_dark, 1)
EVT_DECLARE_USER_FUNC(animation, 5)
EVT_DECLARE_USER_FUNC(init_inside_group, 3)
EVT_DECLARE_USER_FUNC(inside_group, 3)
EVT_DECLARE_USER_FUNC(door_group, 2)
EVT_DECLARE_USER_FUNC(outside_group, 2)
EVT_DECLARE_USER_FUNC(in_pos, 3)
EVT_DECLARE_USER_FUNC(out_pos, 3)
EVT_DECLARE_USER_FUNC(inout_hit, 5)
EVT_DECLARE_USER_FUNC(door_entry, 2)
EVT_DECLARE_USER_FUNC(snd_door_out, 0)
EVT_DECLARE_USER_FUNC(snd_door_in, 0)

// .data
extern int32_t evt_door_setup[1];

}

}