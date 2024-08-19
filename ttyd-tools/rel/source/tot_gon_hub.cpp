#include "tot_gon_hub.h"

#include "evt_cmd.h"
#include "mod.h"
#include "tot_gsw.h"
#include "tot_state.h"
#include "tot_window_select.h"

#include <ttyd/battle_event_cmd.h>
#include <ttyd/database.h>
#include <ttyd/evt_bero.h>
#include <ttyd/evt_cam.h>
#include <ttyd/evt_damage.h>
#include <ttyd/evt_eff.h>
#include <ttyd/evt_hit.h>
#include <ttyd/evt_map.h>
#include <ttyd/evt_mario.h>
#include <ttyd/evt_mobj.h>
#include <ttyd/evt_msg.h>
#include <ttyd/evt_npc.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evt_sub.h>
#include <ttyd/evt_window.h>
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/hitdrv.h>
#include <ttyd/mapdata.h>
#include <ttyd/npcdrv.h>

namespace mod::tot::gon {

namespace {

// Including entire namespaces for convenience.
using namespace ::ttyd::evt_bero;
using namespace ::ttyd::evt_cam;
using namespace ::ttyd::evt_eff;
using namespace ::ttyd::evt_hit;
using namespace ::ttyd::evt_map;
using namespace ::ttyd::evt_mario;
using namespace ::ttyd::evt_mobj;
using namespace ::ttyd::evt_msg;
using namespace ::ttyd::evt_npc;
using namespace ::ttyd::evt_snd;
using namespace ::ttyd::evt_sub;
using namespace ::ttyd::evt_window;

using ::ttyd::evt_bero::BeroEntry;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;
using ::ttyd::hitdrv::HitReturnPoint;
using ::ttyd::npcdrv::NpcSetupInfo;

namespace BeroAnimType = ::ttyd::evt_bero::BeroAnimType;
namespace BeroDirection = ::ttyd::evt_bero::BeroDirection;
namespace BeroType = ::ttyd::evt_bero::BeroType;

}  // namespace

extern const BeroEntry gon_10_entry_data[3];
extern const BeroEntry gon_11_entry_data[3];
extern const NpcSetupInfo gon_10_npc_data[10];
extern const NpcSetupInfo gon_11_npc_data[10];
extern HitReturnPoint gon_10_hit_return_points[10];
extern HitReturnPoint gon_11_hit_return_points[13];

EVT_BEGIN(gon_10_InitEvt)
    SET(LW(0), PTR(&gon_10_entry_data))
    USER_FUNC(evt_bero_get_info)
    USER_FUNC(evt_snd_bgmon, 512, PTR("BGM_STG1_NOK1"))
    USER_FUNC(evt_snd_envon, 272, PTR("ENV_STG1_NOK1"))

    // TODO: Add a one-time opening cutscene on first visit.
    IF_SMALL(0, 1)
        RUN_CHILD_EVT(evt_bero_info_run)
    ELSE()
        RUN_EVT(bero_case_entry)
        // RUN_EVT for first time opening cutscene event.
    END_IF()

    USER_FUNC(evt_map_playanim, PTR("S_kawa"), 1, 0)

    // TODO: Door setup.
    // SET(LW(0), PTR(&yado_data))
    // RUN_CHILD_EVT(PTR(&evt_door_setup))
    // SET(LW(0), PTR(&mise_data))
    // RUN_CHILD_EVT(PTR(&evt_door_setup))
    // SET(LW(0), PTR(&ie_data))
    // RUN_CHILD_EVT(PTR(&evt_door_setup))

    // TODO: Shop and inn setup.
    // USER_FUNC(evt_shop_setup, PTR(&obj_list), PTR(&goods_list), PTR(&shopper_data), PTR(&trade_list))
    // USER_FUNC(evt_kinopio_setup, PTR(&kino_dt))
    
    // TODO: Save block.
    // USER_FUNC(evt_mobj_save_blk, PTR("mobj_save"), 155, 60, -60, 0, 0)

    DO(5)
        USER_FUNC(evt_sub_random, 10, LW(0))
        USER_FUNC(evt_eff, 0, PTR("butterfly"), LW(0), -395, 20, 250, 0, 0, 0, 0, 0, 0, 0, 0)
    WHILE()
    DO(5)
        USER_FUNC(evt_sub_random, 10, LW(0))
        USER_FUNC(evt_eff, 0, PTR("butterfly"), LW(0), -80, 20, 270, 0, 0, 0, 0, 0, 0, 0, 0)
    WHILE()
    DO(2)
        USER_FUNC(evt_sub_random, 10, LW(0))
        USER_FUNC(evt_eff, 0, PTR("butterfly"), LW(0), -40, 20, 470, 0, 0, 0, 0, 0, 0, 0, 0)
    WHILE()
    DO(2)
        USER_FUNC(evt_sub_random, 10, LW(0))
        USER_FUNC(evt_eff, 0, PTR("butterfly"), LW(0), 375, 20, 250, 0, 0, 0, 0, 0, 0, 0, 0)
    WHILE()

    USER_FUNC(evt_hit_damage_return_set, PTR(&gon_10_hit_return_points))
    RUN_CHILD_EVT(ttyd::evt_damage::evt_gazigazi_entry)
    RETURN()
EVT_END()

EVT_BEGIN(gon_11_InitEvt)
    SET(LW(0), PTR(&gon_11_entry_data))
    USER_FUNC(evt_bero_get_info)
    RUN_CHILD_EVT(PTR(&evt_bero_info_run))

    USER_FUNC(evt_snd_bgmon, 512, PTR("BGM_STG1_NOK1"))
    USER_FUNC(evt_snd_envon, 272, PTR("ENV_STG1_NOK2"))
    USER_FUNC(evt_npc_setup, PTR(&gon_11_npc_data))

    // Turn off fast-travel pipe.
    USER_FUNC(evt_mapobj_flag_onoff, 1, 1, PTR("S_dakan"), 1)
    USER_FUNC(evt_hitobj_onoff, PTR("A_dokan"), 1, 0)

    // TODO: Initialize Toad sister NPCs after a couple successful runs.

    USER_FUNC(evt_map_playanim, PTR("S_kawa"), 1, 0)

    // TODO: Door setup.
    // SET(LW(0), PTR(&son_data))
    // RUN_CHILD_EVT(PTR(&evt_door_setup))
    // SET(LW(0), PTR(&taro_data))
    // RUN_CHILD_EVT(PTR(&evt_door_setup))
    // SET(LW(0), PTR(&ie_data))
    // RUN_CHILD_EVT(PTR(&evt_door_setup))

    // Not sure what these do...
    USER_FUNC(evt_map_playanim, PTR("S_mon"), 2, 0)
    USER_FUNC(evt_hitobj_onoff, PTR("A_mon"), 1, 0)

    DO(2)
        USER_FUNC(evt_sub_random, 10, LW(0))
        USER_FUNC(evt_eff, 0, PTR("butterfly"), LW(0), -395, 20, 225, 0, 0, 0, 0, 0, 0, 0, 0)
    WHILE()
    DO(5)
        USER_FUNC(evt_sub_random, 10, LW(0))
        USER_FUNC(evt_eff, 0, PTR("butterfly"), LW(0), -170, 20, 400, 0, 0, 0, 0, 0, 0, 0, 0)
    WHILE()
    DO(5)
        USER_FUNC(evt_sub_random, 10, LW(0))
        USER_FUNC(evt_eff, 0, PTR("butterfly"), LW(0), 180, 20, 400, 0, 0, 0, 0, 0, 0, 0, 0)
    WHILE()
    DO(2)
        USER_FUNC(evt_sub_random, 10, LW(0))
        USER_FUNC(evt_eff, 0, PTR("butterfly"), LW(0), 400, 20, 200, 0, 0, 0, 0, 0, 0, 0, 0)
    WHILE()

    USER_FUNC(evt_hit_damage_return_set, PTR(&gon_11_hit_return_points))
    RUN_CHILD_EVT(ttyd::evt_damage::evt_gazigazi_entry)
    RETURN()
EVT_END()

const BeroEntry gon_10_entry_data[3] = {
    {
        .name = "w_bero",
        .type = BeroType::ROAD,
        .sfx_id = 0,
        .direction = BeroDirection::AUTO,
        .center_position = { 100000, 0, 0 },
        .length = -1,
        .entry_evt_code = nullptr,
        .case_type = -1,
        .out_evt_code = nullptr,
        .target_map = "gon_00",
        .target_bero = "w_bero",
        .entry_anim_type = BeroAnimType::ANIMATION,
        .out_anim_type = BeroAnimType::ANIMATION,
        .entry_anim_args = nullptr,
        .out_anim_args = nullptr,
    },{
        .name = "e_bero",
        .type = BeroType::ROAD,
        .sfx_id = 0,
        .direction = BeroDirection::AUTO,
        .center_position = { 100000, 0, 0 },
        .length = -1,
        .entry_evt_code = nullptr,
        .case_type = -1,
        .out_evt_code = nullptr,
        .target_map = "gon_11",
        .target_bero = "w_bero",
        .entry_anim_type = BeroAnimType::ANIMATION,
        .out_anim_type = BeroAnimType::ANIMATION,
        .entry_anim_args = nullptr,
        .out_anim_args = nullptr,
    },
    { /* null-terminator */ },
};

const BeroEntry gon_11_entry_data[3] = {
    {
        .name = "w_bero",
        .type = BeroType::ROAD,
        .sfx_id = 0,
        .direction = BeroDirection::AUTO,
        .center_position = { 100000, 0, 0 },
        .length = -1,
        .entry_evt_code = nullptr,
        .case_type = -1,
        .out_evt_code = nullptr,
        .target_map = "gon_10",
        .target_bero = "e_bero",
        .entry_anim_type = BeroAnimType::ANIMATION,
        .out_anim_type = BeroAnimType::ANIMATION,
        .entry_anim_args = nullptr,
        .out_anim_args = nullptr,
    },{
        .name = "e_bero",
        .type = BeroType::ROAD,
        .sfx_id = 0,
        .direction = BeroDirection::AUTO,
        .center_position = { 100000, 0, 0 },
        .length = -1,
        .entry_evt_code = nullptr,
        .case_type = -1,
        .out_evt_code = nullptr,
        .target_map = "gon_00",
        .target_bero = "w_bero",
        .entry_anim_type = BeroAnimType::ANIMATION,
        .out_anim_type = BeroAnimType::ANIMATION,
        .entry_anim_args = nullptr,
        .out_anim_args = nullptr,
    },
    { /* null-terminator */ },
};

const NpcSetupInfo gon_10_npc_data[10] = {
    {
        .name = "\x83\x6d\x83\x52\x83\x6d\x83\x52" "A",  // nokonokoA
        .flags = 0,
        .initEvtCode = nullptr,     // (void*)nokonoko_A_init
        .regularEvtCode = nullptr,  // (void*)nokonoko_A_regl
        .talkEvtCode = nullptr,     // (void*)nokonoko_A_talk
    },
    {
        .name = "\x83\x6d\x83\x52\x83\x6d\x83\x52" "B",  // nokonokoB
        .flags = 0,
        .initEvtCode = npc_init_evt,
        .regularEvtCode = nullptr,  // (void*)nokonoko_B_regl
        .talkEvtCode = nullptr,     // (void*)nokonoko_B_talk
        .deadEvtCode = nullptr,     // (void*)nokonoko_B_damage
    },
    {
        .name = "\x83\x6d\x83\x52\x83\x6d\x83\x52" "C",  // nokonokoC
        .flags = 0,
        .initEvtCode = npc_init_evt,
        .regularEvtCode = nullptr,  // (void*)nokonoko_C_regl
        .talkEvtCode = nullptr,     // (void*)nokonoko_C_talk
    },
    {
        .name = "\x83\x6d\x83\x52\x83\x6d\x83\x52" "D",  // nokonokoD
        .flags = 0,
        .initEvtCode = npc_init_evt,
        .talkEvtCode = nullptr,     // (void*)nokonoko_D_talk
    },
    {
        .name = "\x83\x6d\x83\x52\x83\x6d\x83\x52" "F",  // nokonokoF
        .flags = 0,
        .initEvtCode = npc_init_evt,
        .talkEvtCode = nullptr,     // (void*)nokonoko_F_talk
    },
    {
        .name = "\x8f\x68\x93\x58\x88\xf5",             // hotel staff
        .flags = 0,
        .initEvtCode = npc_init_evt,
    },
    {
        .name = "\x93\x58\x88\xf5",                     // store clerk
        .flags = 0,
        .initEvtCode = npc_init_evt,
    },
    {
        .name = "\x83\x6f\x83\x6f",                     // Hooktail??
        .flags = 0,
        .initEvtCode = npc_init_evt,
    },
    {
        .name = "\x83\x7a\x83\x8f\x83\x43\x83\x67",     // General White
        .flags = 0x4000'0600,
        .initEvtCode = nullptr,     // (void*)init_white
        .talkEvtCode = nullptr,     // (void*)talk_white
    },
};

const NpcSetupInfo gon_11_npc_data[10] = {
    {
        .name = "\x83\x6d\x83\x52\x83\x6d\x83\x52" "G",  // nokonokoG
        .flags = 0,
        .initEvtCode = npc_init_evt,
        .regularEvtCode = nullptr,  // (void*)nokonoko_G_regl
        .talkEvtCode = nullptr,     // (void*)nokonoko_G_talk
    },
    {
        .name = "\x83\x6d\x83\x52\x83\x6d\x83\x52" "H",  // nokonokoH
        .flags = 0,
        .initEvtCode = npc_init_evt,
        .regularEvtCode = nullptr,  // (void*)nokonoko_H_regl
        .talkEvtCode = nullptr,     // (void*)nokonoko_H_talk
    },
    {
        .name = "\x83\x6d\x83\x52\x83\x6d\x83\x52" "I",  // nokonokoI
        .flags = 0,
        .initEvtCode = npc_init_evt,
        .regularEvtCode = nullptr,  // (void*)nokonoko_I_regl
        .talkEvtCode = nullptr,     // (void*)nokonoko_I_talk
    },
    {
        .name = "\x83\x6d\x83\x52\x83\x6d\x83\x52" "K",  // nokonokoK
        .flags = 0,
        .initEvtCode = npc_init_evt,
        .talkEvtCode = nullptr,     // (void*)nokonoko_K_talk
    },
    {
        .name = "\x91\xba\x92\xb7",                     // Mayor Kroop
        .flags = 0x4000'0600,
        .initEvtCode = npc_init_evt,
        .talkEvtCode = nullptr,     // (void*)boss_noko_talk
    },
    {
        .name = "\x96\xe5\x94\xd4\x83\x6d\x83\x52\x83\x6d\x83\x52",  // Gatekeeper
        .flags = 0x4000'0600,
        .initEvtCode = nullptr,     // (void*)monban_init
        .talkEvtCode = nullptr,     // (void*)monban_talk
    },
    {
        .name = "\x83\x6d\x83\x52\x83\x5e\x83\x8d\x83\x45",          // Koops
        .flags = 0x4000'0600,
        .initEvtCode = nullptr,     // (void*)nokotarou_init
        .talkEvtCode = nullptr,     // (void*)nokotarou_talk
    },
    {
        .name = "\x83\x6d\x83\x52\x83\x5e\x83\x8d\x83\x45\x95\x83",  // Koopley
        .flags = 0x4000'0600,
        .initEvtCode = nullptr,     // (void*)nokotarou_init
        .talkEvtCode = nullptr,     // (void*)nokotarou_talk
    },
    {
        .name = "\x83\x6d\x83\x52\x83\x8a\x83\x93",         // Koopie Koo
        .flags = 0x4000'0600,
        .initEvtCode = nullptr,     // (void*)nokorin_init
        .talkEvtCode = nullptr,     // (void*)nokorin_talk
    },
};

HitReturnPoint gon_10_hit_return_points[10] = {
    { "mod_00", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_01", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_02", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_04", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_05", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_06", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_07", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_08", { 0.0f, -1000.0f, 0.0f }, },
    { "A_hasi04", { 0.0f, -1000.0f, 0.0f }, },
};

HitReturnPoint gon_11_hit_return_points[13] = {
    { "mod_00", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_01", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_02", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_03", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_04", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_05", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_06", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_07", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_08", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_09", { 0.0f, -1000.0f, 0.0f }, },
    { "mod_010", { 0.0f, -1000.0f, 0.0f }, },
    { "A_hasi1", { 0.0f, -1000.0f, 0.0f }, },
};

const int32_t* GetWestSideInitEvt() {
    return gon_10_InitEvt;
}

const int32_t* GetEastSideInitEvt() {
    return gon_11_InitEvt;
}

}  // namespace mod::tot::gon