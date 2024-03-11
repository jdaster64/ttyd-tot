#include "tot_gon_tower.h"

#include "evt_cmd.h"
#include "tot_generate_enemy.h"
#include "tot_generate_reward.h"
#include "tot_gon.h"

#include <gc/types.h>
#include <ttyd/battle_database_common.h>
#include <ttyd/battle_event_cmd.h>
#include <ttyd/database.h>
#include <ttyd/dispdrv.h>
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
#include <ttyd/evtmgr_cmd.h>
#include <ttyd/icondrv.h>
#include <ttyd/item_data.h>
#include <ttyd/mapdata.h>
#include <ttyd/npcdrv.h>
#include <ttyd/swdrv.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>

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

using ::ttyd::dispdrv::CameraId;
using ::ttyd::evt_bero::BeroEntry;
using ::ttyd::evtmgr_cmd::evtGetValue;
using ::ttyd::evtmgr_cmd::evtSetValue;
using ::ttyd::item_data::itemDataTable;
using ::ttyd::npcdrv::NpcSetupInfo;

namespace BeroAnimType = ::ttyd::evt_bero::BeroAnimType;
namespace BeroDirection = ::ttyd::evt_bero::BeroDirection;
namespace BeroType = ::ttyd::evt_bero::BeroType;
namespace ItemType = ::ttyd::item_data::ItemType;

const char kPitNpcName[] = "\x93\x47";  // "enemy"
const char kMoverNpcName[] = "\x88\xda\x93\xae\x89\xae";  // "idouya"
const char kPiderName[] = "\x83\x70\x83\x43\x83\x5f\x81\x5b\x83\x58";
const char kArantulaName[] = 
    "\x83\x60\x83\x85\x83\x89\x83\x93\x83\x5e\x83\x89\x81\x5b";

// Info for custom NPCs.
NpcSetupInfo g_NpcSetupInfo[2];

}  // namespace

// USER_FUNC declarations.
EVT_DECLARE_USER_FUNC(evtTot_GetUniqueItemName, 1)

extern const BeroEntry normal_room_entry_data[3];

// Script for sign that shows current floor.
EVT_BEGIN(Tower_SignEvt)
    USER_FUNC(evt_npc_stop_for_event)
    USER_FUNC(evt_mario_key_onoff, 0)
    SET(LW(0), GSW(1321))
    // TODO: Read from mod's floor number instead.
    ADD(LW(0), 1)
    USER_FUNC(evt_msg_print_insert, 0, PTR("msg_jon_kanban_1"), 0, 0, LW(0))
    USER_FUNC(evt_mario_key_onoff, 1)
    USER_FUNC(evt_npc_start_for_event)
    RETURN()
EVT_END()

// Script that runs when opening a chest.
EVT_BEGIN(Tower_ChestEvt_Core)
    // Don't allow getting a reward from more than one chest.
    IF_EQUAL(GSW(1000), 1)
        RETURN()
    END_IF()
    // Disable drawing item icons above chests.
    SET(GSW(1001), 2)
    USER_FUNC(evt_mario_key_onoff, 0)
    USER_FUNC(evtTot_GetChestData, LW(9), LW(10), LW(11), LW(12), LW(13), LW(14))
    SWITCH(LW(13))
        CASE_SMALL_EQUAL(-1)    // move, partner, coin rewards
        CASE_ETC()
            USER_FUNC(evtTot_GetUniqueItemName, LW(15))
            USER_FUNC(
                evt_item_entry, LW(15), LW(13), LW(10), LW(11), LW(12),
                17, -1, LW(14))
    END_SWITCH()
    USER_FUNC(evt_mobj_wait_animation_end, LW(8))
    SWITCH(LW(13))
        CASE_SMALL_EQUAL(-1)
            // Run special event from tot_generate_reward, then null it out
            // to have this function return control of Mario.
            RUN_CHILD_EVT(LW(14))
            SET(LW(14), 0)
        CASE_ETC()
            USER_FUNC(evt_item_get_item, LW(15))
    END_SWITCH()
    // If no special pickup script, give Mario back control and spawn pipe.
    IF_EQUAL(LW(14), 0)
        USER_FUNC(evt_mario_key_onoff, 1)
        SET(GSW(1000), 1)
    END_IF()
    RETURN()
EVT_END()

EVT_BEGIN(Tower_ChestEvt_0)
    SET(LW(8), PTR("box_0"))
    SET(LW(9), 0)
    RUN_CHILD_EVT(PTR(&Tower_ChestEvt_Core))
    RETURN()
EVT_END()

EVT_BEGIN(Tower_ChestEvt_1)
    SET(LW(8), PTR("box_1"))
    SET(LW(9), 1)
    RUN_CHILD_EVT(PTR(&Tower_ChestEvt_Core))
    RETURN()
EVT_END()

EVT_BEGIN(Tower_ChestEvt_2)
    SET(LW(8), PTR("box_2"))
    SET(LW(9), 2)
    RUN_CHILD_EVT(PTR(&Tower_ChestEvt_Core))
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
    // TODO: Increment mod's floor number instead.
    SET(LW(0), 0)
    RETURN()
EVT_END()

// Spawn chests.
EVT_BEGIN(Tower_SpawnChests)
    USER_FUNC(evt_mario_key_onoff, 0)
    WAIT_MSEC(2000)
    USER_FUNC(evtTot_GenerateChestContents)
    
    SET(LW(10), 3)
    DO(LW(10))
        // Get contents of next chest, and break from loop if empty.
        SET(LW(11), 3)
        SUB(LW(11), LW(10))
        USER_FUNC(evtTot_GetChestData, LW(11), LW(0), LW(1), LW(2), LW(3), EVT_NULLPTR)
        IF_EQUAL(LW(3), 0)
            DO_BREAK()
        END_IF()
            
        ADD(LW(1), 50)
        USER_FUNC(
            evt_eff, PTR(""), PTR("bomb"), 0, LW(0), LW(1), LW(2), FLOAT(1.0),
            0, 0, 0, 0, 0, 0, 0)
        USER_FUNC(
            evt_snd_sfxon_3d, PTR("SFX_BOSS_RNPL_TRANSFORM4"), 
            LW(0), LW(1), LW(2), 0)
        SUB(LW(1), 25)
        
        SWITCH(LW(11))
            CASE_EQUAL(0)
                USER_FUNC(
                    evt_mobj_itembox, PTR("box_0"), LW(0), LW(1), LW(2), 1, 0, 
                    PTR(&Tower_ChestEvt_0), GSWF(5075))
            CASE_EQUAL(1)
                USER_FUNC(
                    evt_mobj_itembox, PTR("box_1"), LW(0), LW(1), LW(2), 1, 0, 
                    PTR(&Tower_ChestEvt_1), GSWF(5076))
            CASE_ETC()
                USER_FUNC(
                    evt_mobj_itembox, PTR("box_2"), LW(0), LW(1), LW(2), 1, 0, 
                    PTR(&Tower_ChestEvt_2), GSWF(5077))
        END_SWITCH()
    WHILE()

    WAIT_MSEC(1000)
    SET(GSW(1001), 1)
    USER_FUNC(evt_mario_key_onoff, 1)
    RETURN()
EVT_END()

// Spawn the exit to the floor.
EVT_BEGIN(Tower_SpawnPipe)
    USER_FUNC(evt_mario_key_onoff, 0)
    
    // Spawn pipe.
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
    
    // Delete chest objects.
    SET(LW(10), 3)
    DO(LW(10))
        SET(LW(11), 3)
        SUB(LW(11), LW(10))
        USER_FUNC(evtTot_GetChestData, LW(11), LW(0), LW(1), LW(2), LW(3), EVT_NULLPTR)
        IF_EQUAL(LW(3), 0)
            DO_CONTINUE()
        END_IF()
        ADD(LW(1), 25)
        USER_FUNC(
            evt_eff, PTR(""), PTR("bomb"), 0, LW(0), LW(1), LW(2), FLOAT(1.0),
            0, 0, 0, 0, 0, 0, 0)
    WHILE()
    USER_FUNC(evt_mobj_delete, PTR("box_0"))
    USER_FUNC(evt_mobj_delete, PTR("box_1"))
    USER_FUNC(evt_mobj_delete, PTR("box_2"))
    USER_FUNC(evt_mobj_delete, PTR("box_3"))
    
    USER_FUNC(evt_mario_key_onoff, 1)
    RETURN()
EVT_END()

// Set up the battle / enemy NPC, or other NPCs on the floor.
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
    
    // Wait for the enemy to be defeated, then spawn chests.
    INLINE_EVT()
        LBL(1)
        WAIT_FRM(1)
        USER_FUNC(evt_npc_status_check, PTR(kPitNpcName), 4, LW(0))
        IF_EQUAL(LW(0), 0)
            GOTO(1)
        END_IF()
        RUN_EVT(Tower_SpawnChests)
    END_INLINE()
    
    // Wait for a chest to be opened, then spawn pipe.
    INLINE_EVT()
        LBL(2)
        WAIT_FRM(1)
        IF_EQUAL(GSW(1000), 0)
            // TODO: Draw icons corresponding to chest contents.
            IF_LARGE_EQUAL(GSW(1001), 1)
                USER_FUNC(evtTot_DisplayChestIcons)
            END_IF()
            GOTO(2)
        END_IF()
        RUN_EVT(Tower_SpawnPipe)
    END_INLINE()

    RETURN()
EVT_END()

// Main room initialization event.
EVT_BEGIN(gon_01_InitEvt)
    // Random unused bytes, used to track chest collection and icon display.
    SET(GSW(1000), 0)
    SET(GSW(1001), 0)
    // Flags to track chest opening.
    SET(GSWF(5075), 0)
    SET(GSWF(5076), 0)
    SET(GSWF(5077), 0)

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

// Get unique names for spawning items, to avoid softlocks with full inventory.
EVT_DEFINE_USER_FUNC(evtTot_GetUniqueItemName) {
    static int32_t id = 0;
    static char name[16];
    
    id = (id + 1) % 1000;
    sprintf(name, "item_t%03" PRId32, id);
    evtSetValue(evt, evt->evtArguments[0], PTR(name));
    return 2;
}

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