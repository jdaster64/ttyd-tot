#include "tot_gon_tower.h"

#include "evt_cmd.h"
#include "tot_generate_enemy.h"
#include "tot_gon.h"

#include <ttyd/battle_database_common.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/database.h>
#include <ttyd/evt_bero.h>
#include <ttyd/evt_cam.h>
#include <ttyd/evt_case.h>
#include <ttyd/evt_eff.h>
#include <ttyd/evt_fade.h>
#include <ttyd/evt_hit.h>
#include <ttyd/evt_item.h>
#include <ttyd/evt_map.h>
#include <ttyd/evt_mario.h>
#include <ttyd/evt_mobj.h>
#include <ttyd/evt_msg.h>
#include <ttyd/evt_npc.h>
#include <ttyd/evt_party.h>
#include <ttyd/evt_pouch.h>
#include <ttyd/evt_seq.h>
#include <ttyd/evt_snd.h>
#include <ttyd/evt_sub.h>
#include <ttyd/evt_window.h>
#include <ttyd/mapdata.h>
#include <ttyd/npcdrv.h>

namespace mod::tot::gon {

namespace {

// Including entire namespaces for convenience.
using namespace ::ttyd::evt_bero;
using namespace ::ttyd::evt_cam;
using namespace ::ttyd::evt_case;
using namespace ::ttyd::evt_eff;
using namespace ::ttyd::evt_fade;
using namespace ::ttyd::evt_hit;
using namespace ::ttyd::evt_item;
using namespace ::ttyd::evt_map;
using namespace ::ttyd::evt_mario;
using namespace ::ttyd::evt_mobj;
using namespace ::ttyd::evt_msg;
using namespace ::ttyd::evt_npc;
using namespace ::ttyd::evt_party;
using namespace ::ttyd::evt_pouch;
using namespace ::ttyd::evt_seq;
using namespace ::ttyd::evt_snd;
using namespace ::ttyd::evt_sub;
using namespace ::ttyd::evt_window;

using ::ttyd::evt_bero::BeroEntry;
using ::ttyd::npcdrv::NpcSetupInfo;

namespace BeroAnimType = ::ttyd::evt_bero::BeroAnimType;
namespace BeroDirection = ::ttyd::evt_bero::BeroDirection;
namespace BeroType = ::ttyd::evt_bero::BeroType;

const char kPitNpcName[] = "\x93\x47";  // "enemy"
const char kMoverNpcName[] = "\x88\xda\x93\xae\x89\xae";  // "idouya"
const char kPiderName[] = "\x83\x70\x83\x43\x83\x5f\x81\x5b\x83\x58";
const char kArantulaName[] = 
    "\x83\x60\x83\x85\x83\x89\x83\x93\x83\x5e\x83\x89\x81\x5b";

// Info for custom NPCs.
NpcSetupInfo g_NpcSetupInfo[2];

}  // namespace

extern const BeroEntry normal_room_entry_data[3];

// Script for sign that shows current floor.
EVT_BEGIN(Tower_SignEvt)
    USER_FUNC(evt_npc_stop_for_event)
    USER_FUNC(evt_mario_key_onoff, 0)
    SET(LW(0), GSW(1321))
    ADD(LW(0), 1)
    USER_FUNC(evt_msg_print_insert, 0, PTR("msg_jon_kanban_1"), 0, 0, LW(0))
    USER_FUNC(evt_mario_key_onoff, 1)
    USER_FUNC(evt_npc_start_for_event)
    RETURN()
EVT_END()

// Setting up loading zones, etc. for regular room.
EVT_BEGIN(Tower_BeroSetupNormal)
    SET(LW(0), PTR(&normal_room_entry_data))
    USER_FUNC(evt_bero_get_info)
    RUN_CHILD_EVT(PTR(&evt_bero_info_run))
    RETURN()
EVT_END()

// Increment floor number.
EVT_BEGIN(Tower_IncrementFloor)
    ADD(GSW(1321), 1)
    SET(LW(0), 0)
    RETURN()
EVT_END()

// TODO: Implement.
EVT_BEGIN(Tower_SpawnPipe)
    USER_FUNC(evt_mario_key_onoff, 0)
    WAIT_MSEC(1000)
    WAIT_MSEC(1000)
    WAIT_MSEC(500)
    SET(LW(2), PTR("a_dokan_1"))
    SET(LW(3), PTR("dokan_1_s"))
    USER_FUNC(evt_mapobj_get_position, LW(3), LW(7), LW(8), LW(9))
    USER_FUNC(evt_snd_sfxon_3d, PTR("SFX_DOKAN_APPEAR1"), LW(7), LW(8), LW(9), 0)
    USER_FUNC(evt_hit_bind_mapobj, LW(2), LW(3))
    USER_FUNC(evt_sub_intpl_msec_init, 11, 0, 300, 500)
    LBL(0)
    USER_FUNC(evt_sub_intpl_msec_get_value)
    DIVF(LW(0), FLOAT(10.0))
    USER_FUNC(evt_mapobj_trans, 1, LW(3), 0, LW(0), 0)
    USER_FUNC(evt_hit_bind_update, LW(2))
    WAIT_FRM(1)
    IF_EQUAL(LW(1), 1)
        GOTO(0)
    END_IF()
    USER_FUNC(evt_mapobj_flag_onoff, 1, 1, PTR("dokan_1_k"), 1)
    USER_FUNC(evt_mario_key_onoff, 1)
    RETURN()
EVT_END()

// TODO: Implement, based on patches_field::EnemyNpcSetupEvt.
// (For now, just spawn a consistent fight with two Goombas).
EVT_BEGIN(Tower_NpcSetup)
    // TODO: Check for special NPCs (Mover, Charlieton, etc.)...
    
    USER_FUNC(evtTot_GetGonBattleDatabasePtr, LW(0))
    SET(LW(1), PTR(g_NpcSetupInfo))
    USER_FUNC(evtTot_GetEnemyNpcInfo, 
        LW(0), LW(1), LW(2), LW(3), LW(4), LW(5), LW(6))
    
    USER_FUNC(evt_npc_entry, PTR(kPitNpcName), LW(2))
    USER_FUNC(evt_npc_set_tribe, PTR(kPitNpcName), LW(3))
    USER_FUNC(evt_npc_setup, LW(1))
    USER_FUNC(evt_npc_set_position, PTR(kPitNpcName), LW(4), LW(5), LW(6))
    
    IF_STR_EQUAL(LW(2), PTR(kPiderName))
        USER_FUNC(evt_npc_set_home_position, PTR(kPitNpcName), LW(4), LW(5), LW(6))
    END_IF()
    IF_STR_EQUAL(LW(2), PTR(kArantulaName))
        USER_FUNC(evt_npc_set_home_position, PTR(kPitNpcName), LW(4), LW(5), LW(6))
    END_IF()
    
    // TODO: Special initialization for bosses...
    
    // TODO: Swap out battle id as appropriate for special battles?
    USER_FUNC(evtTot_SetEnemyNpcBattleInfo, PTR(kPitNpcName), /* battle id */ 0)
    
    // Wait for the enemy to be defeated, then spawn pipe.
    INLINE_EVT()
        LBL(1)
        WAIT_FRM(1)
        USER_FUNC(evt_npc_status_check, PTR(kPitNpcName), 4, LW(0))
        IF_EQUAL(LW(0), 0)
            GOTO(1)
        END_IF()
        RUN_EVT(Tower_SpawnPipe)
    END_INLINE()

    RETURN()
EVT_END()

// Main room initialization event.
EVT_BEGIN(gon_01_InitEvt)
    USER_FUNC(evt_run_case_evt, 9, 1, PTR("a_kanban"), 0, PTR(Tower_SignEvt), 0)
    
    // Set up enemy NPC (TODO: or Mover, Charlieton, etc.)
    RUN_CHILD_EVT(PTR(&Tower_NpcSetup))
    
    // TODO: Handle cases for Mover / NPC rooms, boss rooms?
    RUN_CHILD_EVT(PTR(&Tower_BeroSetupNormal))
    USER_FUNC(evt_npc_check, PTR(kPitNpcName), LW(0))
    IF_NOT_EQUAL(LW(0), 1)
        USER_FUNC(evt_hit_bind_mapobj, PTR("a_dokan_1"), PTR("dokan_1_s"))
        USER_FUNC(evt_mapobj_trans, 1, PTR("dokan_1_s"), 0, 30, 0)
        USER_FUNC(evt_hit_bind_update, PTR("a_dokan_1"))
        USER_FUNC(evt_mapobj_flag_onoff, 1, 1, PTR("dokan_1_k"), 1)
    END_IF()
    
    // Always remove graffiti from wall on floor 50 map (not needed anymore?)
    // USER_FUNC(evt_sub_get_mapname, LW(0))
    // IF_STR_EQUAL(LW(0), PTR("jon_04"))
    //     USER_FUNC(evt_mapobj_flag_onoff, 1, 1, PTR("after"), 1)
    //     USER_FUNC(evt_mapobj_flag_onoff, 1, 1, PTR("s_rakugaki"), 1)
    // END_IF()
        
    // Non-boss room music / env sound:
    USER_FUNC(evt_snd_bgmon, 512, PTR("BGM_STG0_100DN1"))
    USER_FUNC(evt_snd_envoff, 512)
    USER_FUNC(evt_snd_set_rev_mode, 1)
    
    // Boss room music / env sound:
    // USER_FUNC(evt_snd_bgmoff, 512)
    // USER_FUNC(evt_snd_envon, 272, PTR("ENV_STG0_DAN1"))
    // USER_FUNC(evt_snd_set_rev_mode, 2)

    RETURN()
EVT_END()

const BeroEntry normal_room_entry_data[3] = {
    {
        .name = "dokan_2",
        .type = BeroType::PIPE,
        .sfx_id = 0,
        .direction = BeroDirection::UP,
        .center_position = { 100000, 0, 0 },
        .length = -1,
        .entry_evt_code = nullptr,
        .case_type = 6,
        .out_evt_code = nullptr,
        .target_map = nullptr,
        .target_bero = nullptr,
        .entry_anim_type = BeroAnimType::ANIMATION,
        .out_anim_type = BeroAnimType::ANIMATION,
        .entry_anim_args = nullptr,
        .out_anim_args = nullptr,
    }, {
        .name = "dokan_1",
        .type = BeroType::PIPE,
        .sfx_id = 0,
        .direction = BeroDirection::DOWN,
        .center_position = { 100000, 0, 0 },
        .length = -1,
        .entry_evt_code = nullptr,
        .case_type = 6,
        .out_evt_code = (void*)Tower_IncrementFloor,
        .target_map = "gon_01",
        .target_bero = "dokan_2",
        .entry_anim_type = BeroAnimType::ANIMATION,
        .out_anim_type = BeroAnimType::ANIMATION,
        .entry_anim_args = nullptr,
        .out_anim_args = nullptr,
    }, { /* null-terminator */ },
};

const int32_t* GetTowerInitEvt() {
    return gon_01_InitEvt;
}

}  // namespace mod::tot::gon